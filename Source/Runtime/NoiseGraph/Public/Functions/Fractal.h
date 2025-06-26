// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_FUNCTIONS_FRACTAL_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_FUNCTIONS_FRACTAL_SIMD_H_
#undef NOISEGRAPH_FUNCTIONS_FRACTAL_SIMD_H_
#else
#define NOISEGRAPH_FUNCTIONS_FRACTAL_SIMD_H_
#endif

#include "hwy/highway.h"
#include "OperationsSIMD.h"
#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"
#include "Numerics/FixedPointSIMD.h"
#include "NoiseTypeTraits.h"

#include "Diagnostics/DebugLog.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F, typename Func> requires Noise<B, F, 2, Func>
	inline constexpr V<B, F> Fractal(
		V<B, F> x, V<B, F> y, Func& func, 
		unsigned int octaves = 4, FixedPoint<B, F> persistance = FixedPoint<B, F>(0.5), 
		FixedPoint<B, F> lacunarity = FixedPoint<B, F>(2)
	) {
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		using vec = V<B, F>;

		vec value = Zero<vec>();
		fp maxValue = 0;
		fp amplitude = fpc::One;
		fp frequency = fpc::One;

		for (unsigned int i = 0; i < octaves; ++i) {
			vec sample = func(
				FPMul<B, F>(x, frequency),
				FPMul<B, F>(y, frequency)
			);

			value = FPAdd<B, F>(
				value, 
				FPMul<B, F>(
					FPSub<B, F>(hn::ShiftLeft<1>(sample), 1), 
					amplitude
				)
			);

			maxValue += amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}


		if (maxValue == 0) return value;
		value = FPDiv<B, F>(value, maxValue << 1);	// Scale from [-mv, mv] to [-0.5, 0.5]
		value = FPAdd<B, F>(value, fp(1) >> 1);	// Shift to [0, 1];
		return value;
	}


	template <size_t B, size_t F, typename Func> requires Noise<B, F, 3, Func>
	inline constexpr V<B, F> Fractal(
		V<B, F> x, V<B, F> y, V<B, F> z, Func& func,
		unsigned int octaves = 4, FixedPoint<B, F> persistance = FixedPoint<B, F>(0.5),
		FixedPoint<B, F> lacunarity = FixedPoint<B, F>(2)
	) {
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		using vec = V<B, F>;

		vec value = Zero<vec>();
		fp maxValue = 0;
		fp amplitude = fpc::One;
		fp frequency = fpc::One;

		for (unsigned int i = 0; i < octaves; ++i) {
			vec vfreq = FPBroadcast<B, F>(frequency);
			vec sample = func(
				FPMul<B, F>(x, vfreq),
				FPMul<B, F>(y, vfreq),
				FPMul<B, F>(z, vfreq)
			);

			value = FPAdd<B, F>(
				value,
				FPMul<B, F>(
					FPSub<B, F>(hn::ShiftLeft<1>(sample), 1),
					amplitude
				)
			);

			maxValue += amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}


		if (maxValue == 0) return value;
		value = FPDiv<B, F>(value, maxValue << 1);	// Scale from [-mv, mv] to [-0.5, 0.5]
		value = FPAdd<B, F>(value, fp(1) >> 1);	// Shift to [0, 1];
		return value;
	}
}
HWY_AFTER_NAMESPACE();


#endif  // include guard