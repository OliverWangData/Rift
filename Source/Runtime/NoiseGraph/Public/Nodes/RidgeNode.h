// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NODEGRAPH_NODES_RIDGE_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NODEGRAPH_NODES_RIDGE_SIMD_H_
#undef NODEGRAPH_NODES_RIDGE_SIMD_H_
#else
#define NODEGRAPH_NODES_RIDGE_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class RidgeNode : public NodeBaseSIMD<B, F>
	{
		using fpc = FixedPointConstant<B, F>;
		using vec = V<B, F>;
	public:
		RidgeNode(std::shared_ptr<NodeBaseSIMD<B, F>> base) :Base(base) {}
		virtual ~RidgeNode() = default;

		virtual void PreProcess(const std::vector<NoiseSamplingBound<B, F>>& bounds) override {
			Base->PreProcess(bounds);
		}

		vec operator()(vec x, vec y) override {
			// From [0, 1] to [-1, 1]
			vec rescale = hn::ShiftRight<1>(FPAdd<B, F>(fpc::One, (*Base)(x, y)));
			vec abs = hn::Abs(rescale);
			return hn::Abs(rescale);
		}

		vec operator()(vec x, vec y, vec z) override {
			// From [0, 1] to [-1, 1]
			vec rescale = hn::ShiftRight<1>(FPAdd<B, F>(fpc::One, (*Base)(x, y, z)));
			vec abs = hn::Abs(rescale);
			return hn::Abs(rescale);
		}

		std::shared_ptr<NodeBaseSIMD<B, F>> Base;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
