// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_NODES_RANDOM_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_NODES_RANDOM_SIMD_H_
#undef NOISEGRAPH_NODES_RANDOM_SIMD_H_
#else
#define NOISEGRAPH_NODES_RANDOM_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"
#include "Functions/Random.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class RandomNode : public NodeBaseSIMD<B, F>
	{
		using vec = V<B, F>;

	public:
		RandomNode(FixedPoint<B, F> seed = FixedPoint<B, F>(0)) : Seed(seed) {}

		vec operator()(vec x, vec y) override {
			return Random<B, F>(x, y, Seed);
		}

		vec operator()(vec x, vec y, vec z) override {
			return Random<B, F>(x, y, z, Seed);
		}

		FixedPoint<B, F> Seed;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
