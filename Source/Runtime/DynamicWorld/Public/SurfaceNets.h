// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(DYNAMICWORLD_CHUNKING_SURFACENETS_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef DYNAMICWORLD_CHUNKING_SURFACENETS_H_
#undef DYNAMICWORLD_CHUNKING_SURFACENETS_H_
#else
#define DYNAMICWORLD_CHUNKING_SURFACENETS_H_
#endif

#include "hwy/highway.h"

#include "OperationsSIMD.h"
#include "AlignedArray.h"
#include "Mathematics/IndexingSIMD.h"
#include "Numerics/MathVector.h"
#include "TypeTraits/IntSelector.h"

// EdgeMaskTable is an array lookup that takes an 8 bit corner mask where each corner is 1 or 0 
// depending on if it's inside or outside the isosurface, and returns a 12 bit edge mask where 
// the bit is 1 for edge crossing and 0 for no crossing. 
//
// Corner mask layout:
// Art from: https://gist.github.com/dwilliamson/c041e3454a713e58baf6e4f8e5fffecd
// 
//      z
//      |     y
//      |   /
//      | /
//      +----- x
// 
//            5             7
//            +-------------+               +-----3-------+   
//          / |           / |             / |            /|   
//        /   |         /   |           5   9          7  11
//    1 +-----+-------+  3  |         +-----+1------+     |   
//      |   4 +-------+-----+ 6       |     +-----2-+-----+   
//      |   /         |   /           8   4         5  10
//      | /           | /             | /           | /       
//    0 +-------------+ 2             +------0------+         

/*
Note - Evaluating the edge mask table in constexpr leads to failure by exceeding the step limit.
Instead, here is a python script that generates the values of the edge mask table in hexadecimal:

EdgeMaskVertices = [
	# X-axis
	0, 2, 1, 3, 4, 6, 5, 7,
	# Y-axis
	0, 4, 1, 5, 2, 6, 3, 7,
	# Z-axis
	0, 1, 2, 3, 4, 5, 6, 7
]

def generate_edge_mask_table():
	edge_mask_table = []

	for i in range(256):
		edge_mask = 0

		# For each edge, compare the two cornermask bits (Cornermask is the index of the table).
		# If they are different, then set bit to 1. Else to 0.
		for e in range(12):
			a = i >> EdgeMaskVertices[(e * 2)]
			b = i >> EdgeMaskVertices[(e * 2) + 1]

			if ((a ^ b) & 1) != 0:
				edge_mask |= 1 << e

		# Append the 12-bit edge mask as a hexadecimal string
		edge_mask_table.append(f"0x{edge_mask:03X}")

	return edge_mask_table

if __name__ == "__main__":
	edge_mask_table = generate_edge_mask_table()

	# Print the result in a format that can be copied directly into C++
	for i in range(0, len(edge_mask_table), 8):
		# Join 8 values per row, add a comma after the last value in each row
		print(', '.join(edge_mask_table[i:i+8]) + ',')  
*/
#if HWY_ONCE
static constexpr std::array<int, 24> EdgeMaskVertices{
	// X - axis
	0, 2, 1, 3, 4, 6, 5, 7,
	// Y - axis
	0, 4, 1, 5, 2, 6, 3, 7,
	// Z - axis
	0, 1, 2, 3, 4, 5, 6, 7
};

static constexpr std::array<int, 256> EdgeMaskTable{
	0x000, 0x111, 0x122, 0x033, 0x241, 0x350, 0x363, 0x272,
	0x282, 0x393, 0x3A0, 0x2B1, 0x0C3, 0x1D2, 0x1E1, 0x0F0,
	0x414, 0x505, 0x536, 0x427, 0x655, 0x744, 0x777, 0x666,
	0x696, 0x787, 0x7B4, 0x6A5, 0x4D7, 0x5C6, 0x5F5, 0x4E4,
	0x428, 0x539, 0x50A, 0x41B, 0x669, 0x778, 0x74B, 0x65A,
	0x6AA, 0x7BB, 0x788, 0x699, 0x4EB, 0x5FA, 0x5C9, 0x4D8,
	0x03C, 0x12D, 0x11E, 0x00F, 0x27D, 0x36C, 0x35F, 0x24E,
	0x2BE, 0x3AF, 0x39C, 0x28D, 0x0FF, 0x1EE, 0x1DD, 0x0CC,
	0x844, 0x955, 0x966, 0x877, 0xA05, 0xB14, 0xB27, 0xA36,
	0xAC6, 0xBD7, 0xBE4, 0xAF5, 0x887, 0x996, 0x9A5, 0x8B4,
	0xC50, 0xD41, 0xD72, 0xC63, 0xE11, 0xF00, 0xF33, 0xE22,
	0xED2, 0xFC3, 0xFF0, 0xEE1, 0xC93, 0xD82, 0xDB1, 0xCA0,
	0xC6C, 0xD7D, 0xD4E, 0xC5F, 0xE2D, 0xF3C, 0xF0F, 0xE1E,
	0xEEE, 0xFFF, 0xFCC, 0xEDD, 0xCAF, 0xDBE, 0xD8D, 0xC9C,
	0x878, 0x969, 0x95A, 0x84B, 0xA39, 0xB28, 0xB1B, 0xA0A,
	0xAFA, 0xBEB, 0xBD8, 0xAC9, 0x8BB, 0x9AA, 0x999, 0x888,
	0x888, 0x999, 0x9AA, 0x8BB, 0xAC9, 0xBD8, 0xBEB, 0xAFA,
	0xA0A, 0xB1B, 0xB28, 0xA39, 0x84B, 0x95A, 0x969, 0x878,
	0xC9C, 0xD8D, 0xDBE, 0xCAF, 0xEDD, 0xFCC, 0xFFF, 0xEEE,
	0xE1E, 0xF0F, 0xF3C, 0xE2D, 0xC5F, 0xD4E, 0xD7D, 0xC6C,
	0xCA0, 0xDB1, 0xD82, 0xC93, 0xEE1, 0xFF0, 0xFC3, 0xED2,
	0xE22, 0xF33, 0xF00, 0xE11, 0xC63, 0xD72, 0xD41, 0xC50,
	0x8B4, 0x9A5, 0x996, 0x887, 0xAF5, 0xBE4, 0xBD7, 0xAC6,
	0xA36, 0xB27, 0xB14, 0xA05, 0x877, 0x966, 0x955, 0x844,
	0x0CC, 0x1DD, 0x1EE, 0x0FF, 0x28D, 0x39C, 0x3AF, 0x2BE,
	0x24E, 0x35F, 0x36C, 0x27D, 0x00F, 0x11E, 0x12D, 0x03C,
	0x4D8, 0x5C9, 0x5FA, 0x4EB, 0x699, 0x788, 0x7BB, 0x6AA,
	0x65A, 0x74B, 0x778, 0x669, 0x41B, 0x50A, 0x539, 0x428,
	0x4E4, 0x5F5, 0x5C6, 0x4D7, 0x6A5, 0x7B4, 0x787, 0x696,
	0x666, 0x777, 0x744, 0x655, 0x427, 0x536, 0x505, 0x414,
	0x0F0, 0x1E1, 0x1D2, 0x0C3, 0x2B1, 0x3A0, 0x393, 0x282,
	0x272, 0x363, 0x350, 0x241, 0x033, 0x122, 0x111, 0x000
};

// Represents the vector from the center of the voxel, to the lower-valued corner of each edge
// in the edgemask. 
//					
//		b-----2-----d		Example in 2D:						Edgemask
//		|			|		a = -0.5,-0.5	lowest for x, y		(0, 3)
//		3	  x		1		b = -0.5, 0.5	lowest for y		(2)
//		|			|		c = 0.5, -0.5	lowest for x		(1)
//		a-----0-----c		d = n/a			never lowest value
// 
//		So the final edgemask offsets would be a-c-b-a. 
//
// Remark: This means we can simply do -0.5 + t, where t is a / (a - b), for the axis that has the 
// intersection. 
static std::array<FVector3f, 12> edgeOriginOffset = {
	// X-axis edges
	FVector3f(-0.5, -0.5, -0.5),
	FVector3f(-0.5, -0.5, 0.5),
	FVector3f(-0.5, 0.5, -0.5),
	FVector3f(-0.5, 0.5, 0.5),
	// Y-axis edges
	FVector3f(-0.5, -0.5, -0.5),
	FVector3f(0.5, -0.5, -0.5),
	FVector3f(-0.5, 0.5, -0.5),
	FVector3f(0.5, 0.5, -0.5),
	// Z-axis edges
	FVector3f(-0.5, -0.5, -0.5),
	FVector3f(-0.5, -0.5, 0.5),
	FVector3f(0.5, -0.5, -0.5),
	FVector3f(0.5, -0.5, 0.5)
};
#endif

namespace SIMD::HWY_NAMESPACE
{
	// *********************************************************************************************
	// HELPER DATA
	// *********************************************************************************************
	template <typename TVertexIndex>
	struct SurfaceNetsAllocPool
	{
		AlignedArray<FVector3f> VertexPositions;
		AlignedArray<FVector3f> VertexNormals;
		AlignedArray<TVertexIndex> Triangles;
		AlignedArray<TVertexIndex> LatticeToBufferIndices;
		AlignedArray<TVertexIndex> BufferToLatticeIndices;
	};

	// *********************************************************************************************
	// ALGORITHM
	// *********************************************************************************************
	// How this surface net implementation works is that it loops through each voxel of each X-axis
	// row, checks if there is an isosurface / edge crossing / intersection, and if so, it uses the
	// voxel's corners and edges to determine vertex position, vertex normal, and triangles. 
	template <
		typename TDensity, typename TVertexIndex,
		int SizeX, int SizeY, int SizeZ
	>
		requires std::is_unsigned_v<TDensity> && std::is_unsigned_v<TVertexIndex>
	constexpr void SurfaceNet(
			TDensity* HWY_RESTRICT density, 
			SurfaceNetsAllocPool<TVertexIndex>& pool
		) {
		using dd = hn::ScalableTag<TDensity>;
		using vec = hn::Vec<dd>;
		using mask = hn::Mask<dd>;
		constexpr int dLaneCount = hn::Lanes(dd());
		using bitmask = IntSelector<dLaneCount>::unsigned_type;

		// *****************************************************************************************
		// Lambdas

		// Extracts the Msb of each density lane into an int with the process below:
		//	1) Shifts lane value so the msb is on the right
		//	2) Converts that into a mask with true for set and false for unset values.
		//	3) Use highway's BitsFromMask to convert that set/unset mask into an integer.
		// 
		//  Lane Val			Shifted Val			Mask Val	BitsFromMask
		//	1001_0110		->	0000_0001		->	true
		//	0101_1000		->	0000_0000		->	false
		//	0001_0000		->	0000_0000		->	false		-> 100
		auto extractDensityMsbLambda = [](vec v) -> bitmask {
			return hn::detail::BitsFromMask(
				hn::Ne(hn::ShiftRight<sizeof(TDensity) * 8 - 1>(v), Zero<vec>())
			);
		};

		// Checks if the bitmasks a and b are homogenous where mask is 1. 
		// E.g. If mask = 0001_1111, then a and b are only homogenous when:
		// a = b = 0001_1111 or 0000_0000.
		// This ensures a is homogenous, b is homogenous, and and is homogenous with b (All edges 
		// of the cube). 
		auto isHomogenousLambda = [](bitmask a, bitmask b, bitmask mask) {
			return ((a & mask) == (b & mask)) && (((a & mask) == mask) || (a == 0));
		};

		// The voxel position is determined by the average of the 
		// intersection points. To avoid division, all the intersection
		// points are offset such that the center of the voxel is at 
		// the origin. 
		//
		//	For example, for these intersection points where x is the 
		// origin:
		// 
		//	+-----------+
		//	|			o
		//	|	  		|
		//	|			|
		//	x------o----+
		//
		// Instead of having this and dividing:
		//
		//	+			+
		//		     __-o
		//		 __--
		//	 __--
		//	x------o	+
		// 
		// We can sum these:
		// 
		//	+			+
		//			 ___o
		//		  x--
		//		  |
		//	+	   o	+
		//
		auto calculateVertPositionLambda = [](TDensity* values, int edgemask) {
			// These are used to scale the density values from [min, max] to [-1, 1]
			constexpr float offset = std::numeric_limits<TDensity>::max() / 2;
			constexpr float inv = 1 / offset;

			FVector3f pos(0, 0, 0);

			for (int e = 0; e < 12; ++e) {
				// Edge has a crossing
				if (((edgemask >> e) & 1) != 0) {
					// Find the intersection
					// Since the density values fully saturate the type (Aka range is between 
					// [min TDensity, max TDensity], we want to convert to and offset the center 
					// to 0 before doing intersection calculations. 
					float a = (values[EdgeMaskVertices[(e * 2)]] - offset) * inv;
					float b = (values[EdgeMaskVertices[(e * 2) + 1]] - offset) * inv;
					float t = a / (a - b);

					// Creating and adding the vector from voxel center to intersection
					if (e < 4) { // X-Axis
						pos += FVector3f(t, 0, 0) + edgeOriginOffset[e];
					}
					else if (e < 8) { // Y-Axis
						pos += FVector3f(0, t, 0) + edgeOriginOffset[e];
					}
					else { // Z-Axis
						pos += FVector3f(0, 0, t) + edgeOriginOffset[e];
					}
				}
			}

			return pos;
		};

		// Takes in voxel lattice indices, converts them into buffer indices, and appends them 
		// to the triangles array in the right order. Allows flipping triangles. 
		auto createTrianglesLambda = [&](
			TVertexIndex a, TVertexIndex b, TVertexIndex c, TVertexIndex d, bool flip
			) {
				TVertexIndex* ptr = pool.LatticeToBufferIndices.GetPtr();
				TVertexIndex v0 = ptr[a];
				TVertexIndex v1 = ptr[b];
				TVertexIndex v2 = ptr[c];
				TVertexIndex v3 = ptr[d];

				if (flip) {
					std::swap(v1, v2);
				}

				pool.Triangles.Add(v0);
				pool.Triangles.Add(v1);
				pool.Triangles.Add(v2);
				pool.Triangles.Add(v2);
				pool.Triangles.Add(v1);
				pool.Triangles.Add(v3);
		};

		// *****************************************************************************************
		
		// Ensures that TVertexIndex has enough indices for all the voxels
		constexpr int voxelCount = Length(SizeX - 1, SizeY - 1, SizeZ - 1);
		static_assert(voxelCount <= std::numeric_limits<TVertexIndex>::max());

		// Allocations
		pool.LatticeToBufferIndices.EnsureSize(voxelCount);
		pool.LatticeToBufferIndices.Fill(std::numeric_limits<TVertexIndex>::max());
		pool.LatticeToBufferIndices.Clear();
		pool.BuferToLatticeIndices.Clear();
		pool.VertexPositions.Clear();
		pool.VertexNormals.Clear();
		pool.Triangles.Clear();

		// Each simd will overlap at the end densities to cover all voxels. 
		//
		//	+---+---x---+---x---+
		//	|	|	|	|	|	|	x are densities shared by two simd
		//	+---+---x---+---x---+
		//	|-simd1-|-simd2-|-simd3-
		//
		//	If this is not done, some voxels won't be processed.
		//
		//	+---+---+---+---+---+
		//	|	|	| x	|	|	|	No shared densities, voxel x is ignored by homogenity checks
		//	+---+---+---+---+---+
		//	|-simd1-|	|-simd2-|
		constexpr int simdIncrement = (dLaneCount / 2) - 1;

		// Division to find simd per row. Rounds up. 
		// I.e. 25 densities per row, 8 increment, gives 3.125 = 4 simd per row. 
		constexpr int simdMaxCount = (SizeX + simdIncrement - 1) / simdIncrement;

		// Cache of extracted msb. It essentially has 1 bit for every lane of a row simd.
		HWY_ALIGN bitmask msbCache[simdMaxCount];

		// Cache of interleaved row densities.
		HWY_ALIGN TDensity rowCache[SizeX * 2]; 


		// Z starts from 1 since the bottom densities do not form a voxel on their own
		for (int z = 1; z < SizeZ; ++z)
		{
			// Y starts from 0 because we want to pre-cache the first row into rowCache.
			for (int y = 0; y < SizeY; ++y)
			{
				// Loops through the row, simd by simd. Simds overlap at the ends. 
				// Loops until SizeX - 1 because there aren't enough densities for a voxel if 
				// x = SizeX - 1
				//
				//	+---+---+---+
				//	|	|	|	|		a = SizeX - 2
				//	+---+---a---b		b = SizeX - 1
				//	|--simd 1---|-- simd 2-
				//
				// Even though simd 2 has density b, there are no voxels starting at b. The last 
				// voxel starts at SizeX - 2, so x goes up to SizeX - 2.
				int simdCount = 0;
				for (int x = 0; x < SizeX - 1; x += simdIncrement)
				{
					// Number of new densities that can actually be loaded. This is usually 
					// LaneCount / 2, except for the last simd of the row which may be smaller. 
					//
					//	+---+---+---+---+---+---+---+
					//	|	|	|	|	|	|	|	|
					//	1---2---3---4---2---3---4---2
					//	|---simd1---|---simd2---|---simd3---|
					//
					//	Simd1 can load 4. Simd2 can load 4. Simd3 can load 2. 
					int loadHalfCount = std::min(dLaneCount / 2, SizeX - x);
					int loadCount = loadHalfCount * 2;

					// Load and interleave row densities, and then extracts the msb.
					//
					//	|----------simd z1----------|
					//	z
					//	|
					//	a---b---c---d
					//	|	|	|	|
					//	0---1---2---3 ---x
					// 
					//	|----------simd z0----------|
					//
					// 
					//	0---a---1---b---2---c---3---d
					// 
					//	|---------simd comb---------|
					// 
					//	Simd z0 is 0123. Simdz1 is abcd. And the interleaved simd is 0a1b2c3d. 
					int i = Flatten(x, y, z, SizeX, SizeY);
					vec loadedZ0 = hn::LoadN(dd(), density + i - (SizeX * SizeY), loadHalfCount);
					vec loadedZ1 = hn::LoadN(dd(), density + i, loadHalfCount);
					vec loaded = hn::InterleaveWholeLower(dd(), loadedZ0, loadedZ1);
					bitmask loadedMsb = extractDensityMsbLambda(loaded);

					// Skip voxels when y = 0. 
					// This is when we only want to pre-cache the first row. 
					if (y != 0)
					{
						bitmask cachedMsb = msbCache[simdCount];

						// The homogenity mask will be all 1s most of the time, except for
						// on the last simd where it excludes the bits that are not part of loaded
						// densities. E.g.
						//
						//	+---+---+---+---+---+---+---+
						//	|	|	|	|	|	|	|	|
						//	1---2---3---4---2---3---4---2
						//	|---simd1---|---simd2---|---simd3---|
						//
						// Simd1 = 1111_1111	Simd2 = 1111_1111	Simd3 = 0000_1111
						bitmask homogenityMask = (loadCount == dLaneCount) ? ~0 : 
							(1 << loadCount) - 1;

						// Check if the msbs are homogenous. 
						// If they are, there are no isosurfaces so we can skip. 
						if (!isHomogenousLambda(cachedMsb, loadedMsb, homogenityMask))
						{
							// Caching to a stack array to reduce lane access overhead
							HWY_ALIGN TDensity valuesLoaded[dLaneCount];
							hn::Store(loaded, dd(), valuesLoaded);

							// Loop through each voxel
							for (int v = 0; v < loadHalfCount - 1; ++v)
							{
								// Builds the 8 bit cornermask of the voxel with the layout
								// of the EdgeMaskTable index
								bitmask loadedVoxelMsb = (loadedMsb >> (v * 2)) & 15;
								bitmask cachedVoxelMsb = (cachedMsb >> (v * 2)) & 15;
								uint8_t cornerMask = loadedVoxelMsb << 4 | cachedVoxelMsb;

								// Check if current voxel has an isosurface
								if (cornerMask != 0 && cornerMask != 255)
								{
									// Save the voxel's buffer position into the lattice array map.
									// Triangles use voxel lattice indices since buffer indices are 
									// not available in the triangle step. That gets processed 
									// aftwards to the when all buffer indices are determined.
									int latIndex = Flatten(x + v, y - 1, z - 1, 
										SizeX - 1, SizeY - 1);
									int bufIndex = pool.VertexPositions->Count();
									(*(pool.LatticeToBufferIndices))[latIndex] = bufIndex;
									pool.BufferToLatticeIndices->Add(latIndex);

									// Edgemask is 1 or 0 depending on if edge of voxel is a
									// crossing or not. 
									int edgeMask = EdgeMaskTable[cornerMask];

									// Array of just the 8 corners
									HWY_ALIGN TDensity corners[8];
									corners[0] = rowCache[x + (v * 2) + 0];
									corners[1] = rowCache[x + (v * 2) + 1];
									corners[2] = rowCache[x + (v * 2) + 2];
									corners[3] = rowCache[x + (v * 2) + 3];
									corners[4] = valuesLoaded[(v * 2) + 0];
									corners[5] = valuesLoaded[(v * 2) + 1];
									corners[6] = valuesLoaded[(v * 2) + 2];
									corners[7] = valuesLoaded[(v * 2) + 3];

									// *************************************************************
									// Voxel position
									FVector3f pos = calculateVertPositionLambda(corners, edgeMask);
									FVector3f voxPos = FVector3f(x + v, y - 1, z - 1);
									pool.VertexPositions.Add(pos + voxPos);

									// *************************************************************
									// Voxel normal
									// This is an approximte normal calculation that doesn't use
									// trigonometry cross products. How its done is by summing the
									// gradients created by the corners for each axis. 
									float normalX = corners[2] - corners[0]
										+ corners[3] - corners[1]
										+ corners[6] - corners[4]
										+ corners[7] - corners[5];

									float normalY = corners[4] - corners[0]
										+ corners[5] - corners[1]
										+ corners[6] - corners[2]
										+ corners[7] - corners[3];

									float normalZ = corners[1] - corners[0]
										+ corners[3] - corners[2]
										+ corners[5] - corners[4]
										+ corners[7] - corners[6];

									FVector3f normal(normalX, normalY, normalZ);
									normal.Normalize();
									pool.VertexNormals.Add(normal);

									// *************************************************************
									// Triangles
									// Each voxel owns edges 0, 4, 8 so will make quads for those. 
									// These edges touch vertices that have already been added to
									// the buffer and are spatially close to the current vertex.
									// This allows us to not need a second pass to translate from
									// lattice to buffer. 

									// Offsets for lattice indices
									constexpr TVertexIndex planeOffset = (SizeX - 1) * (SizeY - 1);
									constexpr TVertexIndex rowOffset = SizeY - 1;

									// x is the start of the simd, and v is the index of the voxel
									// inside the simd, so the voxel's index is x + v. 
									// It has to be bigger than 0 since the voxel at x = 0 is 
									// the first voxel of the row, which doesn't have a prev voxel.
									bool hasPrevXVoxel = (x + v) >= 1;

									// Has to be bigger than 1 since y = 0 is only the first row
									// of densities, y = 1 is the first row of voxels (2 rows of 
									// densities), and y = 2 is the second row of voxels. 
									bool hasPrevYVoxel = y > 1;

									// Same concept as y. z = 1 is the first plane of voxels, z = 2
									// is the second. 
									bool hasPrevZVoxel = z > 1;

									// Construct the lattice index of neighbouring voxels, and then
									// use the latticeToBufferIndices array to get the buffer index
									TVertexIndex iX0Y1Z0 = latIndex - 1 - planeOffset;
									TVertexIndex iX0Y0Z1 = latIndex - 1 - rowOffset;
									TVertexIndex iX0Y1Z1 = latIndex - 1;
									TVertexIndex iX1Y0Z0 = latIndex - rowOffset - planeOffset;
									TVertexIndex iX1Y0Z1 = latIndex - rowOffset;
									TVertexIndex iX1Y1Z0 = latIndex - planeOffset;
									TVertexIndex iX1Y1Z1 = latIndex;

									// For each quad, the default case is the lower valued corner
									// on the axis is lower density and inside the isosurface. 
									// If that's not the case, then we can flip the order of the 
									// indexes to draw the triangle accordingly. 
									
									// Edge 0 - YZ quad
									if ((edgeMask & 1) != 0 && hasPrevYVoxel && hasPrevZVoxel)
									{
										createTrianglesLambda(
											iX1Y0Z0, iX1Y0Z1, iX1Y1Z0, iX1Y1Z1, 
											corners[0] > corners[2]
										);
									}

									// Edge 4 - XZ quad
									if ((edgeMask >> 4) != 0 && hasPrevXVoxel && hasPrevZVoxel)
									{
										createTrianglesLambda(
											iX1Y1Z0, iX1Y1Z1, iX0Y1Z0, iX0Y1Z1,
											corners[0] > corners[4]
										);
									}

									// Edge 8 - XY quad
									if ((edgeMask >> 8) != 0 && hasPrevXVoxel && hasPrevYVoxel)
									{
										createTrianglesLambda(
											iX0Y0Z1, iX0Y1Z1, iX1Y0Z1, iX1Y1Z1,
											corners[0] > corners[1]
										);
									}

								} // Voxel not homogenous
							} // Voxel loop
						} // SIMD not homogenous
					} // y != 0

					// Store into cache
					hn::StoreN(loaded, dd(), rowCache + x, loadCount);
					msbCache[simdCount] = loadedMsb;
					++simdCount;

				} // row simd loop
			} // y loop
		} // z loop
	};
}


#endif  // include guard