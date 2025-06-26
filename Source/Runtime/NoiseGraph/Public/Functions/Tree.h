// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_FUNCTIONS_TREE_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_FUNCTIONS_TREE_SIMD_H_
#undef NOISEGRAPH_FUNCTIONS_TREE_SIMD_H_
#else
#define NOISEGRAPH_FUNCTIONS_TREE_SIMD_H_
#endif

#include "hwy/highway.h"

#include "AlignedArray.h"
#include "OperationsSIMD.h"
#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"
#include "Numerics/FixedPointSIMD.h"
#include "Numerics/MathVector.h"
#include "Mathematics/Indexing.h"
#include "Mathematics/IndexingSIMD.h"
#include "NoiseTypeTraits.h"
#include "NoiseSamplingParameters.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{

	// *********************************************************************************************
	// HELPER DATA
	// *********************************************************************************************
	// Class that holds all allocated objects needed for caching
	template <size_t B, size_t F, size_t Dim>
	struct TreeCacheAllocPool
	{
		using fp = FixedPoint<B, F>;

		struct DepthTree
		{
			AlignedArray<T<B, F>> Tree;
			MathVector<fp, Dim> Begin;	// World position at array 0 index
			MathVector<int, Dim> Size;	// Size of each axis
		};

		std::vector<DepthTree> Trees;
		
		AlignedArray<T<B, F>> Values;
	};

	// Depth-specific variables
	template <size_t B, size_t F, size_t Dim>
	struct TreeCacheDepthProperties
	{
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;

		const fp Interval; // Interval between cells
		const fp IntervalInverse; 
		const int Padding;	// 7x7 for depth 0 (3 on each side), 5x5 for depth n (2 on each side). 

		const MathVector<fp, Dim> Begin;	// World pos at index 0 of the depth tree array
		const MathVector<fp, Dim> End;		// World pos at max axis index of the depth tree array
		const MathVector<int, Dim> Size;
		const int TotalSize;				// Number of total cells in the depth
		const int PaddedSize;				// SIMD-adjusted size of the depth tree array

		TreeCacheDepthProperties(int depth, MathVector<fp, Dim> start, MathVector<fp, Dim> end) :
			Interval(fpc::One >> depth),
			IntervalInverse(fpc::One << depth),
			Padding((depth == 0) ? 3 : 2),
			Begin(start.Apply<fp>([&](fp val) -> fp { 
				return Floor(val, depth) - (Interval * Padding); 
			})),
			End(start.Apply<fp>([&](fp val) -> fp {
			return Ceil(val, depth) + (Interval * Padding);
				})),
			// ((Ceil(end, depth) - Floor(start, depth)) * IntervalInverse + Padding).ToInt()
			Size(
				(
					(
						end.Apply<fp>([&](fp val) { return Ceil(val, depth); }) -
						start.Apply<fp>([&](fp val) { return Floor(val, depth); })
					) * IntervalInverse + (Padding * 2)
				).Apply<int>([](fp val) { return val.ToInt(); })
			),
			TotalSize(Size.Product()),
			PaddedSize(TotalSize + GetPadding(TotalSize, hn::Lanes(D<B, F>()))) { }

	};

	// *********************************************************************************************
	// HELPER FUNCTIONS
	// *********************************************************************************************
	// Retrieves the index for the cell at a specified point, on a specified depth. 
	// 
	// Index is determined by (xn - x0) / interval, where:
	//	x0 is the depth's actual starting value.
	//	xn is a value in the cell that we want the index of. 
	//	interval is the depth's interval. 
	// 
	// This can be rearranged to (xn - x0) * (1 / interval)
	template <size_t B, size_t F>
	inline V<B, F> GetCellIndex(V<B, F> position, V<B, F> start, int depth, bool log = false) {
		using vec = V<B, F>;
		using fpc = FixedPointConstant<B, F>;

		vec top = sn::Sub(FPFloor<B, F>(position, depth), start);

		// The structure of the DepthTree is the start is at indices (0, 0), and all other elements
		// have positions greater than the start. Trying to find a position lower than the start
		// is not possible since it's not in the DepthTree. 
		assert(hn::AllFalse(D<B, F>(), hn::IsNegative(top)));

		vec invInterval = FPBroadcast<B, F>(fpc::One << depth);
		return FPToInt<B, F>(FPMul<B, F>(top, invInterval));
	}

	// Uses the cell's world position to generate a point in the cell.
	// Shifts it towards the center using the regularity. 
	template <size_t B, size_t F>
	inline void GenerateCellPoint(
		int i, FixedPoint<B, F> seed, FixedPoint<B, F> regularity,
		TreeCacheDepthProperties<B, F, 2>& dp,
		V<B, F>& pointX, V<B, F>& pointY, V<B, F>& worldX, V<B, F>& worldY
	) {
		using vec = V<B, F>;
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;

		// Cell array indices
		vec arrIdX, arrIdY;
		VUnravel(i, dp.Size.Get(0), dp.Size.Get(1), arrIdX, arrIdY);

		// Cell world coordinates
		worldX = FPAdd<B, F>(dp.Begin.Get(0), FPMul<B, F>(hn::ShiftLeft<F>(arrIdX), dp.Interval));
		worldY = FPAdd<B, F>(dp.Begin.Get(1), FPMul<B, F>(hn::ShiftLeft<F>(arrIdY), dp.Interval));

		// Cell's randomly generated point
		// Generates the local offset of the point where x and y are between [0, interval], 
		// then uses the regularity to squeeze it towards the center. 
		vec pointLocalX = Random<B, F>(worldX, worldY, FPBroadcast<B, F>(fpc::Sqrt2), seed);
		vec pointLocalY = Random<B, F>(worldX, worldY, FPBroadcast<B, F>(fpc::Sqrt3), seed);

		// Regularity: 0 is no change. 1 is at the center of the grid. 
		// Squeeze towards center is done by scaling down the local offset to the proper
		// size, and then shifting it towards the center (half-interval). 
		pointLocalX = FPAdd<B, F>(
			FPMul<B, F>(pointLocalX, 1 - regularity), regularity * (dp.Interval >> 1));
		pointLocalY = FPAdd<B, F>(
			FPMul<B, F>(pointLocalY, 1 - regularity), regularity * (dp.Interval >> 1));

		pointX = worldX + pointLocalX;
		pointY = worldY + pointLocalY;
	}

	// *********************************************************************************************
	// CACHING
	// *********************************************************************************************
	/// <summary>
	/// Generates all possible cell branches at all depths for the bounding box. 
	/// For each tree array, each cell will have 4 T items, 2 for the cell point, and 2 for the 
	/// branch's point.
	/// 
	/// std::vector<Aligned> depthTree - Stores pointers to the trees array for all depths
	/// std::vector<int> treeSizeX - The count of cells on the x axis
	/// std::vector<MathVector<fp, 2>> treeStart - The (x, y) of the first cell
	/// </summary>
	template <size_t B, size_t F, size_t Dim, typename Func> requires Noise<B, F, Dim, Func>
	inline constexpr void GetTreeCache(
		MathVector<FixedPoint<B, F>, Dim> start, MathVector<FixedPoint<B, F>, Dim> end, 
		Func& func, FixedPoint<B, F> seed, unsigned int depth, FixedPoint<B, F> regularity,
		TreeCacheAllocPool<B, F, Dim>& pool
	) {
		namespace hn = hwy::HWY_NAMESPACE;
		using vec = V<B, F>;
		using mask = M<B, F>;
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		const D<B, F> dc;
		const int laneCount = hn::Lanes(dc);

		// Check to make sure the cache is not going to be too big. 
		// TODO

		// Ensures the pooled allocations are correct size.
		// Allows pooled objects to used with array-like syntax. 
		pool.Trees.resize(depth + 1);

		// When writing to the TreeDepth with consecutive cells we want to store each cell 4 items 
		// apart, since TreeDepth has 4 elements per cell (pointX, pointY, branchX, branchY). 
		vec scatter = Mul(hn::Iota(dc, 0), 4);
		vec scatterOffset = Mul(scatter, sizeof(T<B, F>));

		// *****************************************************************************************
		// Caching each depth sequentially
		for (int d = 0; d <= int(depth); ++d) {

			TreeCacheDepthProperties<B, F, 2> dp = TreeCacheDepthProperties<B, F, 2>(
				d, start, end);

			// Ensures the depth's allocations are large enough for the current pass. 
			// Reallocates if not. 
			pool.Trees[d].Tree.EnsureSize(dp.PaddedSize * 4);
			pool.Trees[d].Begin = dp.Begin;
			pool.Trees[d].Size = dp.Size;

			// *************************************************************************************
			// Depth 0, special case where:
			//	- For a sample, the 7x7 grid around it has to be evaluated
			//	- Branches are connected by the Func sampler rather than by distance

			if (d == 0) {
				// Values array allocations
				pool.Values.EnsureSize(dp.PaddedSize);

				// Gets the random point for each cell, samples func(), and stores the value. 
				for (int i = 0; i < dp.PaddedSize; i += laneCount) {
					vec pointX, pointY, _, __;
					GenerateCellPoint<B, F>(i, seed, regularity, dp, pointX, pointY, _, __);

					T<B, F>* ptr = pool.Trees[0].Tree.GetPtr() + (i * 4);
					hn::ScatterOffset(pointX, dc, ptr + 0, scatterOffset);
					hn::ScatterOffset(pointY, dc, ptr + 1, scatterOffset);
					hn::Store(func(pointX, pointY), dc, pool.Values.GetPtr() + i);
				}

				// Branches are based on the lowest valued moore's neighbour (3x3). 
				for (int i = 0; i < dp.PaddedSize; i += laneCount) {
					// Stores the point with the minimum value
					// If this is a local minima, then there is no branch. 
					// For this case, the branch connects to itself.
					vec ind = hn::Iota(dc, i);
					vec branch = ind;
					vec branchValue = hn::Load(dc, pool.Values.GetPtr() + i);

					// Loops through the 3x3 moore neighbourhood and connects using the func value. 
					for (int dy = -1; dy <= 1; ++dy) {
						for (int dx = -1; dx <= 1; ++dx) {

							// Skip out of bounds neighbours (Outside of the values 2d array)
							mask isInRangeX = hn::Not(hn::MaskFalse(D<B, F>()));
							mask isInRangeY = isInRangeX;
							vec indX = Mod(ind, dp.Size.Get(0));

							if (dx < 0) {
								isInRangeX = hn::Gt(indX, Zero<vec>());
							}
							else if (dx > 0) {
								isInRangeX = hn::Lt(indX, Broadcast<vec>(dp.Size.Get(0) - 1));
							}
							if (dy < 0) {
								isInRangeY = hn::Ge(ind, Broadcast<vec>(dp.Size.Get(0)));
							}
							else if (dy > 0) {
								isInRangeY = hn::Lt(ind, 
									Broadcast<vec>(dp.TotalSize - dp.Size.Get(0)));
							}
							mask isInRange = hn::And(isInRangeX, isInRangeY);

							// Checks for minima
							int compIndex = Adjust(i, dp.Size.Get(0), dx, dy);

							// Note: Even though comparison index is technically contiguous, we 
							// still need to go through the mask and then load values by index. 
							// This is because the comparison index can have out-of-bounds indices 
							// if we do a contiguous load.
							// 
							// E.g. at i = 0, dx = dy = -1, the neighbourIndex is negative, which is
							// not a valid place to start loading. 
							//
							// Ind is used as a temporary indices because those indices are 
							// definitely in range. The values of the indices will be ignored. 
							vec nInd = hn::IfThenElse(isInRange, hn::Iota(dc, compIndex), ind);

							vec values = hn::GatherIndex(dc, pool.Values.GetPtr(), nInd);
							mask newMinima = hn::And(isInRange, hn::Lt(values, branchValue));

							// Update local minima variables
							branch = hn::IfThenElse(newMinima, hn::Iota(dc, compIndex), branch);
							branchValue = hn::IfThenElse(newMinima, values, branchValue);
						}
					}

					// Gathers the values for the closest branch for each cell by index. 
					// Then saves them into the consecutive cells' branchX and branchY slots. 
					T<B, F>* ptr = pool.Trees[0].Tree.GetPtr();
					vec branchX = hn::GatherIndex(dc, ptr + 0, Mul(branch, 4));
					vec branchY = hn::GatherIndex(dc, ptr + 1, Mul(branch, 4));
					hn::ScatterOffset(branchX, dc, ptr + (i * 4) + 2, scatterOffset);
					hn::ScatterOffset(branchY, dc, ptr + (i * 4) + 3, scatterOffset);
				}

				continue; // To next depth
			}

			// *************************************************************************************
			// Depth N
			// For each cell, the 5x5 has to be evaluate at this depth
			// Cells should use their parent cell's point if it is within the cell.
			// Branches are based on distance to any branch in any of the previous depths

			// Common-Functionality Lambda
			auto GetCellIndexLambda = [&](vec posX, vec posY, vec& indX, vec& indY, int depth) {
				indX = GetCellIndex<B, F>(
					posX, FPBroadcast<B, F>(pool.Trees[depth].Begin.Get(0)), depth);
				indY = GetCellIndex<B, F>(
					posY, FPBroadcast<B, F>(pool.Trees[depth].Begin.Get(1)), depth);
				};

			for (int i = 0; i < dp.PaddedSize; i += laneCount) {

				// DEFAULT CASE - New point
				// Case where the parent cell's point is not in the current cell.  
				// Points are randomly generated, and the branch is the closest distance to 
				// all existing branches in previous depths neighbourhood (7x7 for 0, 5x5 for N). 
				vec pointX, pointY, worldX, worldY;
				GenerateCellPoint<B, F>(i, seed, regularity, dp, pointX, pointY, worldX, worldY);

				// Loop thorugh all previous depths and find minima
				vec branchX = pointX;
				vec branchY = pointY;
				vec closestDist = FPBroadcast<B, F>(fpc::Max);
				
				for (int dn = 0; dn < d; ++dn) {
					TreeCacheDepthProperties<B, F, 2> ddp = TreeCacheDepthProperties<B, F, 2>(
						dn, start, end);

					for (int dy = -ddp.Padding; dy <= ddp.Padding; ++dy) {
						for (int dx = -ddp.Padding; dx <= ddp.Padding; ++dx) {

							// Get the array indices of the comparison cell. 
							// Note: The tree algorithm guarantees all sampling points will be 
							// surrounded by the final 7x7 or 5x5 padding. However this doesn't 
							// hold true here during the cache. Some points won't have the padding 
							// available on a previous depth. 
							//
							// For instance:
							// 
							//	d0 =-3	d0 = -2	d0 = -1	d0 = 0
							//							d1 = 0
							//	+---+---+---+---+---+---+---+---+
							//	|	 	|		| n	|	|	|	|
							//	+   x   +	x	+---+---+---+---+
							//	|		|		|	|	|	|	|
							//	+---+---+---+---+---+---+---+---+
							//
							// Consider cell n on depth 1. It is part of the 5x5 padding for x = 0. 
							// But the cell does not have a 7x7 padding on depth 0. So we need to
							// only try and find minima where the previous depth's cells actually
							// exist. 

							vec compX = FPAdd<B, F>(pointX, dx * ddp.Interval);
							vec compY = FPAdd<B, F>(pointY, dy * ddp.Interval);

							vec compIndX, compIndY;
							GetCellIndexLambda(compX, compY, compIndX, compIndY, dn);

							// Skip out of bounds neighbours (Outside of the values 2d array)
							mask isInRangeX = hn::And(
								hn::Ge(compIndX, Zero<vec>()),
								hn::Lt(compIndX, Broadcast<vec>(ddp.Size.Get(0)))
							);
							mask isInRangeY = hn::And(
								hn::Ge(compIndY, Zero<vec>()),
								hn::Lt(compIndY, Broadcast<vec>(ddp.Size.Get(1)))
							);

							mask isInRange = hn::And(isInRangeX, isInRangeY);


							vec compInd = VFlatten(compIndX, compIndY, pool.Trees[dn].Size.Get(0));
							compInd = hn::IfThenElseZero(isInRange, compInd);
							vec compInd4 = Mul(compInd, 4);

							// Finds the point on the line between the comparison's point and
							// branch that minimizes distance to the current cell's point. 
							vec compPointX, compPointY;
							T<B, F>* ptr = pool.Trees[dn].Tree.GetPtr();
							FPMinimumDistancePoint<B, F>(
								hn::GatherIndex(dc, ptr + 0, compInd4),
								hn::GatherIndex(dc, ptr + 1, compInd4),
								hn::GatherIndex(dc, ptr + 2, compInd4),
								hn::GatherIndex(dc, ptr + 3, compInd4),
								pointX, pointY, compPointX, compPointY
							);

							// Save that minimal point if it's closer than the currently held one
							vec dist = sn::Add(
								FPSquare<B, F>(compPointX - pointX),
								FPSquare<B, F>(compPointY - pointY)
							);

							mask newMinima = hn::And(hn::Lt(dist, closestDist), isInRange);
							closestDist = hn::IfThenElse(newMinima, dist, closestDist);
							branchX = hn::IfThenElse(newMinima, compPointX, branchX);
							branchY = hn::IfThenElse(newMinima, compPointY, branchY);
						}
					}
				}
				
				// ALTERNATE CASE - Reuse point
				// Case where the parent cell's point is in the area of the current cell. 
				// This can be checked by getting the coordinates and then point of the parent
				// cell, and then checking if that point is in the area of the current cell. 
				
				// Get the array indices of the parent cell. 
				vec parentIndX, parentIndY;
				GetCellIndexLambda(worldX, worldY, parentIndX, parentIndY, d - 1);
				vec parentInd = VFlatten(parentIndX, parentIndY, pool.Trees[d - 1].Size.Get(0));
				parentInd = Mul(parentInd, 4); // pre-multiply by 4 to use on depthTree.

				// Retrieve the parent's point and branch
				T<B, F>* pptr = pool.Trees[d - 1].Tree.GetPtr();
				vec pPointX = hn::GatherIndex(dc, pptr + 1, parentInd);
				vec pPointY = hn::GatherIndex(dc, pptr + 1, parentInd);
				vec pBranchX = hn::GatherIndex(dc, pptr + 2, parentInd);
				vec pBranchY = hn::GatherIndex(dc, pptr + 3, parentInd);

				// Checks to see if the parent's point is in the current cell
				mask usePreviousPoint = hn::And(
					hn::And(
						hn::Ge(pPointX, worldX), 
						hn::Ge(pPointY, worldY)
					),
					hn::And(
						hn::Lt(pPointX, FPAdd<B, F>(worldX, dp.Interval)),
						hn::Lt(pPointY, FPAdd<B, F>(worldY, dp.Interval))
					)
				);

				// FINALLY - Choose case and store values
				// Either use old or new point
				pointX = hn::IfThenElse(usePreviousPoint, pPointX, pointX);
				pointY = hn::IfThenElse(usePreviousPoint, pPointY, pointY);
				branchX = hn::IfThenElse(usePreviousPoint, pBranchX, branchX);
				branchY = hn::IfThenElse(usePreviousPoint, pBranchY, branchY);
				
				// Stores the values into the depthtree collection
				T<B, F>* ptr = pool.Trees[d].Tree.GetPtr() + (i * 4);
				hn::ScatterOffset(pointX, dc, ptr + 0, scatterOffset);
				hn::ScatterOffset(pointY, dc, ptr + 1, scatterOffset);
				hn::ScatterOffset(branchX, dc, ptr + 2, scatterOffset);
				hn::ScatterOffset(branchY, dc, ptr + 3, scatterOffset);
			} // i loop

		} // Depth loop
	} // Cache function


	// *********************************************************************************************
	// NOISE
	// *********************************************************************************************
	// Dendry Noise.
	// See: https://dl.acm.org/doi/pdf/10.1145/3306131.3317020
	// For this implementation, the tree cache is required.
	// This is because there in basically all cases it's a performance boost to cache the tree
	// (And implementing the non-cache version is basically just the same code duplicated)
	template <size_t B, size_t F, typename Func> requires Noise<B, F, 2, Func>
	inline constexpr V<B, F> Tree(
		V<B, F> x, V<B, F> y, unsigned int depth, TreeCacheAllocPool<B, F, 2>& pool
	) {
		using vec = V<B, F>;
		using mask = M<B, F>;
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		const D<B, F> dc;
		
		// Loop thorugh all depths and find minima
		vec closestDist = FPBroadcast<B, F>(fpc::Max);

		for (int dn = 0; dn <= int(depth); ++dn) {
			fp dninterval = fpc::One >> dn;
			int dnPadding = (dn == 0) ? 3 : 2;

			for (int dy = -dnPadding; dy <= dnPadding; ++dy) {
				for (int dx = -dnPadding; dx <= dnPadding; ++dx) {

					// Get the array indices of the comparison cell. 
					vec compIndX = GetCellIndex<B, F>(
						FPAdd<B, F>(x, dx * dninterval),
						FPBroadcast<B, F>(pool.Trees[dn].Begin[0]), dn);
					vec compIndY = GetCellIndex<B, F>(
						FPAdd<B, F>(y, dy * dninterval),
						FPBroadcast<B, F>(pool.Trees[dn].Begin[1]), dn);

					vec compInd = VFlatten(compIndX, compIndY, pool.Trees[dn].Size[0]);
					vec compInd4 = Mul(compInd, 4);

					// Finds the point on the line between the comparison's point and
					// branch that minimizes distance to the current cell's point. 
					vec compPointX, compPointY;
					T<B, F>* ptr = pool.Trees[dn].Tree.GetPtr();
					FPMinimumDistancePoint<B, F>(
						hn::GatherIndex(dc, ptr + 0, compInd4),
						hn::GatherIndex(dc, ptr + 1, compInd4),
						hn::GatherIndex(dc, ptr + 2, compInd4),
						hn::GatherIndex(dc, ptr + 3, compInd4),
						x, y, compPointX, compPointY
					);

					// Save that minimal point if it's closer than the currently held one
					vec dist = sn::Add(
						FPSquare<B, F>(compPointX - x),
						FPSquare<B, F>(compPointY - y)
					);

					closestDist = sn::Min(dist, closestDist);
				}
			}
		}

		return FPSqrt<B, F>(closestDist);
	}

}
HWY_AFTER_NAMESPACE();


#endif  // include guard