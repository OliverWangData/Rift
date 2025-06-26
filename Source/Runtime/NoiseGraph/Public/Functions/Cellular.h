// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_FUNCTIONS_CELLULAR_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_FUNCTIONS_CELLULAR_SIMD_H_
#undef NOISEGRAPH_FUNCTIONS_CELLULAR_SIMD_H_
#else
#define NOISEGRAPH_FUNCTIONS_CELLULAR_SIMD_H_
#endif

#include "hwy/highway.h"
#include "hwy/aligned_allocator.h"

#include "OperationsSIMD.h"
#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"
#include "Numerics/FixedPointSIMD.h"
#include "Mathematics/Indexing.h"

#include "Random.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	/// <summary>
	/// Features:
	/// 0: Return distance to closest cell point
	/// 1: Return cell's random value
	/// 2: Return dist between cell's two closest points
	/// </summary>
	template <size_t B, size_t F, size_t Feature>
	inline constexpr auto Cellular(
		V<B, F> x, V<B, F> y,
		FixedPoint<B, F> seed, unsigned int maxPointsPerGrid = 1
	) {
		static_assert(Feature >= 0 && Feature <= 2, "Feature not defined.");
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		using vec = V<B, F>;
		using mask = M<B, F>;

		// Grid coordinates
		vec x0 = FPFloor<B, F>(x);
		vec y0 = FPFloor<B, F>(y);

		vec minDist = FPBroadcast<B, F>(FixedPointConstant<B, F>::Max);
		vec minX = x;
		vec minY = y;
		vec minId = Zero<vec>();
		vec secondMinX = minX;
		vec secondMinY = minY;

		// Loops through moore neighbourhood
		for (int dy = -1; dy <= 1; dy++) {
			for (int dx = -1; dx <= 1; dx++) {

				vec gridX = FPAdd<B, F>(x0, dx);
				vec gridY = FPAdd<B, F>(y0, dy);
				vec gridhash = Random<B, F>(gridX, gridY, seed);

				// Formula to uniformly determines the number of points in a grid cell
				vec points = And(
					FPAdd<B, F>(FPMul<B, F>(gridhash, maxPointsPerGrid), 1),
					FixedPoint<B, F>::IntegerMask
				);
				int maxPointCount = fp::FromBase(ReduceMax(points)).ToInt();

				// Loops through each point in the cell
				for (int i = 0; i < maxPointCount; i++) {
					vec pointX = FPAdd<B, F>(
						gridX, 
						Random<B, F>(gridhash, FPBroadcast<B, F>(fpc::Sqrt2), fp::FromBase(i))
					);
					vec pointY = FPAdd<B, F>(
						gridY, 
						Random<B, F>(gridhash, FPBroadcast<B, F>(fpc::Sqrt3), fp::FromBase(i))
					);

					vec dist = FPAdd<B, F>(
						FPSquare<B, F>(FPSub<B, F>(pointX, x)),
						FPSquare<B, F>(FPSub<B, F>(pointY, y))
					);

					// Updates the min variables if new point is closest point
					mask validPointMask = hn::Lt(FPBroadcast<B, F>(i), points);
					mask isMinMask = hn::Lt(dist, minDist);
					mask newValueMask = hn::And(isMinMask, validPointMask);
					minDist = hn::IfThenElse(newValueMask, dist, minDist);

					if constexpr (Feature == 2) {
						secondMinX = hn::IfThenElse(newValueMask, minX, secondMinX);
						secondMinY = hn::IfThenElse(newValueMask, minY, secondMinY);
					}

					if constexpr (Feature == 1 || Feature == 2) {
						minX = hn::IfThenElse(newValueMask, pointX, minX);
						minY = hn::IfThenElse(newValueMask, pointY, minY);
						minId = hn::IfThenElse(newValueMask, Broadcast<vec>(i), minId);
					}

				}
			}
		}
		if constexpr (Feature == 0) {
			return FPSqrt<B, F>(minDist);
		}
		else if constexpr (Feature == 1) {
			return Random<B, F>(minX, minY, minId, seed);
		}
		else if constexpr (Feature == 2) {
			return FPSqrt<B, F>(FPAdd<B, F>(
				FPSquare<B, F>(FPSub<B, F>(minX, secondMinX)),
				FPSquare<B, F>(FPSub<B, F>(minY, secondMinY))
			));
		}
	}


	/// <summary>
	/// Features:
	/// 0: Return distance to closest cell point
	/// 1: Return cell's random value
	/// 2: Return dist between cell's two closest points
	/// </summary>
	template <size_t B, size_t F, size_t Feature>
	inline constexpr auto Cellular(
		V<B, F> x, V<B, F> y, V<B, F> z, 
		FixedPoint<B, F> seed, unsigned int maxPointsPerGrid = 1
	) {
		static_assert(Feature >= 0 && Feature <= 2, "Feature not defined.");
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		using vec = V<B, F>;
		using mask = M<B, F>;

		// Grid coordinates
		vec x0 = FPFloor<B, F>(x);
		vec y0 = FPFloor<B, F>(y);
		vec z0 = FPFloor<B, F>(z);

		vec minDist = FPBroadcast<B, F>(FixedPointConstant<B, F>::Max);
		vec minX = x;
		vec minY = y;
		vec minZ = z;
		vec minId = Zero<vec>();
		vec secondMinX = minX;
		vec secondMinY = minY;
		vec secondMinZ = minZ;

		// Loops through moore neighbourhood
		for (int dz = -1; dz <= 1; dz++) {
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {

					vec gridX = FPAdd<B, F>(x0, dx);
					vec gridY = FPAdd<B, F>(y0, dy);
					vec gridZ = FPAdd<B, F>(z0, dz);
					vec gridhash = Random<B, F>(gridX, gridY, gridZ, seed);

					// Formula to uniformly determines the number of points in a grid cell
					vec points = And(
						FPAdd<B, F>(FPMul<B, F>(gridhash, maxPointsPerGrid), 1),
						fp::IntegerMask
					);
					int maxPointCount = fp::FromBase(ReduceMax(points)).ToInt();

					// Loops through each point in the cell
					for (int i = 0; i < maxPointCount; i++) {
						vec pointX = FPAdd<B, F>(
							gridX,
							Random<B, F>(gridhash, FPBroadcast<B, F>(fpc::Sqrt2), fp::FromBase(i))
						);
						vec pointY = FPAdd<B, F>(
							gridY,
							Random<B, F>(gridhash, FPBroadcast<B, F>(fpc::Sqrt3), fp::FromBase(i))
						);
						vec pointZ = FPAdd<B, F>(gridZ, Random<B, F>(
							gridhash, FPBroadcast<B, F>(fpc::InvSqrt3), fp::FromBase(i)
						));

						vec dist = FPAdd<B, F>(
							FPAdd<B, F>(
								FPSquare<B, F>(FPSub<B, F>(pointX, x)),
								FPSquare<B, F>(FPSub<B, F>(pointY, y))
							),
							FPSquare<B, F>(FPSub<B, F>(pointZ, z))
						);

						// Updates the min variables if new point is closest point
						mask validPointMask = hn::Lt(FPBroadcast<B, F>(i), points);
						mask isMinMask = hn::Lt(dist, minDist);
						mask newValueMask = hn::And(isMinMask, validPointMask);
						minDist = hn::IfThenElse(newValueMask, dist, minDist);

						if constexpr (Feature == 2) {
							secondMinX = hn::IfThenElse(newValueMask, minX, secondMinX);
							secondMinY = hn::IfThenElse(newValueMask, minY, secondMinY);
							secondMinZ = hn::IfThenElse(newValueMask, minZ, secondMinZ);
						}

						if constexpr (Feature == 1 || Feature == 2) {
							minX = hn::IfThenElse(newValueMask, pointX, minX);
							minY = hn::IfThenElse(newValueMask, pointY, minY);
							minZ = hn::IfThenElse(newValueMask, pointZ, minZ);
							minId = hn::IfThenElse(newValueMask, Broadcast<vec>(i), minId);
						}
					}
				}
			}
		}

		if constexpr (Feature == 0) {
			return FPSqrt<B, F, 3>(minDist);
		}
		else if constexpr (Feature == 1) {
			return Random<B, F>(minX, minY, minZ, minId, seed);
		}
		else if constexpr (Feature == 2) {
			return FPSqrt<B, F, 3>(FPAdd<B, F>(
				FPAdd<B, F>(
					FPSquare<B, F>(FPSub<B, F>(minX, secondMinX)),
					FPSquare<B, F>(FPSub<B, F>(minY, secondMinY))
				),
				FPSquare<B, F>(FPSub<B, F>(minZ, secondMinZ))
			));
		}
	}



}
HWY_AFTER_NAMESPACE();


#endif  // include guard