// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_FUNCTIONS_WARP_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_FUNCTIONS_WARP_SIMD_H_
#undef NOISEGRAPH_FUNCTIONS_WARP_SIMD_H_
#else
#define NOISEGRAPH_FUNCTIONS_WARP_SIMD_H_
#endif

#include "hwy/highway.h"
#include "OperationsSIMD.h"
#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"
#include "Numerics/FixedPointSIMD.h"
#include "NoiseTypeTraits.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F, typename Func> requires Noise<B, F, 2, Func>
	inline constexpr V<B, F> Warp(
		V<B, F> x, V<B, F> y, Func& base, Func& shift,
		unsigned int layers = 1, FixedPoint<B, F> strength = FixedPoint<B, F>(0.5)
	) {
		using vec = V<B, F>;
		using fpc = FixedPointConstant<B, F>;

		// Rescales the values from [0, 1] to [-strength, strength]
		auto RescaleRangeLambda = [&](vec value) {
			return FPMul<B, F>(strength, FPSub<B, F>(hn::ShiftLeft<1>(value), 1));
		};

		for (unsigned int i = 0; i < layers; i++) {
			vec shiftX = shift(x, y);

			// Constant offset of 0.5
			vec shiftY = shift(
				FPAdd<B, F>(x, fpc::One >> 1),
				FPAdd<B, F>(y, fpc::One >> 1)
			);

			x = FPAdd<B, F>(x, RescaleRangeLambda(shiftX));
			y = FPAdd<B, F>(y, RescaleRangeLambda(shiftY));
		}

		return base(x, y);
	}

	template <size_t B, size_t F, typename Func> requires Noise<B, F, 3, Func>
	inline constexpr V<B, F> Warp(
		V<B, F> x, V<B, F> y, V<B, F> z, Func& base, Func& shift,
		unsigned int layers = 1, FixedPoint<B, F> strength = FixedPoint<B, F>(0.5)
	) {
		using vec = V<B, F>;
		using fpc = FixedPointConstant<B, F>;

		// Rescales the values from [0, 1] to [-strength, strength]
		auto RescaleRangeLambda = [&](vec value) {
			return FPMul<B, F>(strength, FPSub<B, F>(hn::ShiftLeft<1>(value), 1));
			};

		for (unsigned int i = 0; i < layers; i++) {

			vec shiftX = shift(x, y, z);
			// constant offset of 0.5
			vec shiftY = shift(
				FPAdd<B, F>(x, fpc::One >> 1),
				FPAdd<B, F>(y, fpc::One >> 1),
				FPAdd<B, F>(y, fpc::One >> 1)
			);
			// Constant offset of 0.375
			vec shiftZ = shift(
				FPAdd<B, F>(x, (fpc::One >> 2) | (fpc::One >> 3)),
				FPAdd<B, F>(y, (fpc::One >> 2) | (fpc::One >> 3)),
				FPAdd<B, F>(y, (fpc::One >> 2) | (fpc::One >> 3))
			);

			x = FPAdd<B, F>(x, RescaleRangeLambda(shiftX));
			y = FPAdd<B, F>(y, RescaleRangeLambda(shiftY));
			y = FPAdd<B, F>(y, RescaleRangeLambda(shiftZ));
		}

		return base(x, y, z);
	}
}
HWY_AFTER_NAMESPACE();


#endif  // include guard