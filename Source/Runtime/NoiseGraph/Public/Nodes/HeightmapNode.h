// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NODEGRAPH_NODES_HEIGHTMAP_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NODEGRAPH_NODES_HEIGHTMAP_SIMD_H_
#undef NODEGRAPH_NODES_HEIGHTMAP_SIMD_H_
#else
#define NODEGRAPH_NODES_HEIGHTMAP_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class HeightmapNode : public NodeBaseSIMD<B, F>
	{
		using fp = FixedPoint<B, F>;
		using fpc = FixedPointConstant<B, F>;
		using vec = V<B, F>;
	public:
		HeightmapNode(
			std::shared_ptr<NodeBaseSIMD<B, F>> base,
			fp upperBound = fp(100),
			fp lowerBound = fp(0)
		) :Base(base), UpperBound(upperBound), LowerBound(lowerBound) {}

		virtual ~HeightmapNode() = default;

		virtual void PreProcess(const std::vector<NoiseSamplingBound<B, F>>& bounds) override {
			Base->PreProcess(bounds);
		}

		vec operator()(vec x, vec y) override {
			return (*Base)(x, y);
		}

		vec operator()(vec x, vec y, vec z) override {
			// We want the sampler bias to be 0 at upper bound, and 1 at lower bound. 
			vec zBias = FPSub<B, F>(z, LowerBound); // Shift z so that lower bound is 0.
			vec bias = FPDiv<B, F>(zBias, UpperBound - LowerBound); // bias of z in bounds
			bias = FPClamp<B, F>(bias, 0, fpc::One);	// In case z is outside of bounds
			return sn::Max(sn::Sub((*Base)(x, y, z), bias), Zero<vec>());
		}

		std::shared_ptr<NodeBaseSIMD<B, F>> Base;
		FixedPoint<B, F> UpperBound;
		FixedPoint<B, F> LowerBound;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
