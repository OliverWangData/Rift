// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_TYPETRAITS_NOISETYPETRAITS_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_TYPETRAITS_NOISETYPETRAITS_SIMD_H_
#undef NOISEGRAPH_TYPETRAITS_NOISETYPETRAITS_SIMD_H_
#else
#define NOISEGRAPH_TYPETRAITS_NOISETYPETRAITS_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Numerics/FixedPointSIMD.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F, size_t D, typename Func, typename = void>
	struct is_noise_v : std::false_type {};

	template <size_t B, size_t F, typename Func>
	struct is_noise_v <B, F, 2, Func> : std::bool_constant<
		requires(Func f, V<B, F> x, V<B, F> y) {
			{ f(x, y) } -> std::convertible_to<V<B, F>>;
	}> {};

	template <size_t B, size_t F, typename Func>
	struct is_noise_v <B, F, 3, Func> : std::bool_constant <
		requires(Func f, V<B, F> x, V<B, F> y, V<B, F> z) {
			{ f(x, y, z) } -> std::convertible_to<V<B, F>>;
	}> {};

	template <size_t B, size_t F, size_t D, typename Func>
	concept Noise = is_noise_v<B, F, D, Func>::value;





}
HWY_AFTER_NAMESPACE();


#endif  // include guard