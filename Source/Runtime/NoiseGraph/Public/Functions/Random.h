// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_FUNCTIONS_RANDOM_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_FUNCTIONS_RANDOM_SIMD_H_
#undef NOISEGRAPH_FUNCTIONS_RANDOM_SIMD_H_
#else
#define NOISEGRAPH_FUNCTIONS_RANDOM_SIMD_H_
#endif

#include "hwy/highway.h"
#include "TypeTraits/TemplateBundles.h"
#include "Cryptography/HashSIMD.h"
#include "OperationsSIMD.h"
#include "Numerics/FixedPointSIMD.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	inline constexpr V<B, F> Random(V<B, F> x, V<B, F> y, FixedPoint<B, F> seed) {
		using vec = V<B, F>;
		using uvec = UV<B, F>;

		uvec hash = Hash(
			Reinterpret<uvec>(x), Reinterpret<uvec>(y), Broadcast<uvec>(seed.ToRaw())
		);

		return And(Reinterpret<vec>(hash), FixedPoint<B, F>::FractionMask);
	}

	template <size_t B, size_t F>
	inline constexpr V<B, F> Random(
		V<B, F> x, V<B, F> y, V<B, F> z, FixedPoint<B, F> seed
	) {
		using vec = V<B, F>;
		using uvec = UV<B, F>;

		uvec hash = Hash(
			Reinterpret<uvec>(x), Reinterpret<uvec>(y), Reinterpret<uvec>(z), 
			Broadcast<uvec>(seed.ToRaw())
		);
		return And(Reinterpret<vec>(hash), FixedPoint<B, F>::FractionMask);
	}

	template <size_t B, size_t F>
	inline constexpr V<B, F> Random(
		V<B, F> x, V<B, F> y, V<B, F> z, V<B, F> w, FixedPoint<B, F> seed
	) {
		using vec = V<B, F>;
		using uvec = UV<B, F>;

		uvec hash = Hash(
			Reinterpret<uvec>(x), Reinterpret<uvec>(y), Reinterpret<uvec>(z), 
			Reinterpret<uvec>(w), Broadcast<uvec>(seed.ToRaw())
		);
		return And(Reinterpret<vec>(hash), FixedPoint<B, F>::FractionMask);
	}
}
HWY_AFTER_NAMESPACE();

#endif  // include guard