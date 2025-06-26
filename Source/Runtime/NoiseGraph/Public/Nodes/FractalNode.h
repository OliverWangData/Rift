// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NODEGRAPH_NODES_FRACTAL_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NODEGRAPH_NODES_FRACTAL_SIMD_H_
#undef NODEGRAPH_NODES_FRACTAL_SIMD_H_
#else
#define NODEGRAPH_NODES_FRACTAL_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"
#include "Functions/Fractal.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class FractalNode : public NodeBaseSIMD<B, F>
	{
		using vec = V<B, F>;
	public:
		FractalNode(
			std::shared_ptr<NodeBaseSIMD<B, F>> base,
			unsigned int octaves = 4,
			FixedPoint<B, F> persistance = FixedPoint<B, F>(0.5),
			FixedPoint<B, F> lacunarity = FixedPoint<B, F>(2)
		) :Base(base), Octaves(octaves), Persistance(persistance), Lacunarity(lacunarity) {}

		virtual ~FractalNode() = default;

		virtual void PreProcess(const std::vector<NoiseSamplingBound<B, F>>& bounds) override {
			std::vector<NoiseSamplingBound<B, F>> newBounds = bounds;

			// The higher the frequency, the smaller and closer the highest octave samples go to 0
			FixedPoint<B, F> frequencyScale = 1;

			for (size_t i = 1; i < Octaves; ++i) {
				frequencyScale *= Lacunarity;
			}

			for (size_t i = 0; i < newBounds.size(); ++i) {
				newBounds[i].Start = 
					(newBounds[i].Start < 0) ? 
					newBounds[i].Start :
					(newBounds[i].Start * frequencyScale);

				newBounds[i].End =
					(newBounds[i].End < 0) ?
					(newBounds[i].End * frequencyScale) :
					newBounds[i].End;
			}

			Base->PreProcess(newBounds);
		}

		vec operator()(vec x, vec y) override {
			return Fractal<B, F, NodeBaseSIMD<B, F>>(
				x, y, *Base, Octaves, Persistance, Lacunarity
			);
		}

		vec operator()(vec x, vec y, vec z) override {
			return Fractal<B, F, NodeBaseSIMD<B, F>>(
				x, y, z, *Base, Octaves, Persistance, Lacunarity
			);
		}

		std::shared_ptr<NodeBaseSIMD<B, F>> Base;
		unsigned int Octaves;
		FixedPoint<B, F> Persistance;
		FixedPoint<B, F> Lacunarity;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
