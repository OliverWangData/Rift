// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_NODES_CELLULAR_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_NODES_CELLULAR_SIMD_H_
#undef NOISEGRAPH_NODES_CELLULAR_SIMD_H_
#else
#define NOISEGRAPH_NODES_CELLULAR_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"
#include "Functions/Cellular.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F, size_t Feature>
	class CellularNode : public NodeBaseSIMD<B, F>
	{
		using vec = V<B, F>;

	public:
		CellularNode(
			FixedPoint<B, F> seed = FixedPoint<B, F>(0), 
			unsigned int maxPointsPerGrid = 1
		) : Seed(seed), MaxPointsPerGrid(maxPointsPerGrid) {}

		vec operator()(vec x, vec y) override {
			return Cellular<B, F, Feature>(x, y, Seed, MaxPointsPerGrid);
		}

		vec operator()(vec x, vec y, vec z) override {
			return Cellular<B, F, Feature>(x, y, z, Seed, MaxPointsPerGrid);
		}

		FixedPoint<B, F> Seed;
		unsigned int MaxPointsPerGrid;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
