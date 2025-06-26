// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NODEGRAPH_NODES_WARP_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NODEGRAPH_NODES_WARP_SIMD_H_
#undef NODEGRAPH_NODES_WARP_SIMD_H_
#else
#define NODEGRAPH_NODES_WARP_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"
#include "Functions/Warp.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class WarpNode : public NodeBaseSIMD<B, F>
	{
		using vec = V<B, F>;
	public:
		WarpNode(
			std::shared_ptr<NodeBaseSIMD<B, F>> base,
			std::shared_ptr<NodeBaseSIMD<B, F>> shift,
			unsigned int layers = 1,
			FixedPoint<B, F> strength = FixedPoint<B, F>(1)
		) :Base(base), Shift(shift), Layers(layers), Strength(strength) {}

		virtual ~WarpNode() = default;

		virtual void PreProcess(const std::vector<NoiseSamplingBound<B, F>>& bounds) override {
			std::vector<NoiseSamplingBound<B, F>> newBounds = bounds;

			// Each layer increases the range of the axis by strength. 
			FixedPoint<B, F> strengthOffset = 1;

			for (size_t i = 0; i < Layers; ++i) {
				strengthOffset *= Strength;
			}

			for (size_t i = 0; i < newBounds.size(); ++i) {
				newBounds[i].Start -= strengthOffset;
				newBounds[i].End += strengthOffset;
			}

			Base->PreProcess(newBounds);

			// Each axis also needs to have + 0.5, as warp implementation uses an offset. 
			for (size_t i = 0; i < newBounds.size(); ++i) {
				newBounds[i].End += 0.5;
			}

			Shift->PreProcess(newBounds);
		}

		vec operator()(vec x, vec y) override {
			return Warp<B, F, NodeBaseSIMD<B, F>>(
				x, y, *Base, *Shift, Layers, Strength
			);
		}

		vec operator()(vec x, vec y, vec z) override {
			return Warp<B, F, NodeBaseSIMD<B, F>>(
				x, y, z, *Base, *Shift, Layers, Strength
			);
		}

		std::shared_ptr<NodeBaseSIMD<B, F>> Base;
		std::shared_ptr<NodeBaseSIMD<B, F>> Shift;
		unsigned int Layers;
		FixedPoint<B, F> Strength;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
