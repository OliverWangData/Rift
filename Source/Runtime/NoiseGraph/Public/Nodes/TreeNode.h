// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NODEGRAPH_NODES_TREE_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NODEGRAPH_NODES_TREE_SIMD_H_
#undef NODEGRAPH_NODES_TREE_SIMD_H_
#else
#define NODEGRAPH_NODES_TREE_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Nodes/NodeBaseSIMD.h"
#include "Numerics/FixedPointSIMD.h"
#include "Functions/Tree.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class TreeNode : public NodeBaseSIMD<B, F>
	{
		using fp = FixedPoint<B, F>;
		using vec = V<B, F>;
	public:
		TreeNode(
			std::shared_ptr<NodeBaseSIMD<B, F>> base,
			fp seed = fp(0),
			unsigned int depth = 0,
			fp regularity = fp(0)
		) :Base(base), Seed(seed), Depth(depth), Regularity(regularity) {}

		virtual ~TreeNode() = default;

		virtual void PreProcess(const std::vector<NoiseSamplingBound<B, F>>& bounds) override {
			GetTreeCache<B, F, 2, NodeBaseSIMD<B, F>>(
				MathVector<fp, 2>(bounds[0].Start, bounds[1].Start), 
				MathVector<fp, 2>(bounds[0].End, bounds[1].End),
				*Base, Seed, Depth, Regularity, Pool
			);
		}

		vec operator()(vec x, vec y) override {
			return Tree<B, F, NodeBaseSIMD<B, F>>(x, y, Depth, Pool);
		}

		vec operator()(vec x, vec y, vec z) override {
			return Zero<vec>();
		}

		std::shared_ptr<NodeBaseSIMD<B, F>> Base;
		FixedPoint<B, F> Seed;
		unsigned int Depth;
		FixedPoint<B, F> Regularity;

	protected:
		TreeCacheAllocPool<B, F, 2> Pool;
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
