// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_NODES_BASESIMD_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_NODES_BASESIMD_SIMD_H_
#undef NOISEGRAPH_NODES_BASESIMD_SIMD_H_
#else
#define NOISEGRAPH_NODES_BASESIMD_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Mathematics/IndexingSIMD.h"
#include "Numerics/FixedPointSIMD.h"
#include "NoiseSamplingParameters.h"
#include "NodeBase.h"
#include <variant>

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	class NodeBaseSIMD : public NodeBase<B, F>
	{
		using vec = V<B, F>;
		using typename NodeBase<B, F>::VarPtr;

	public:
		NodeBaseSIMD() = default;
		virtual ~NodeBaseSIMD() = default;

		virtual vec operator()(vec x, vec y) {
			return Zero<vec>();
		}

		virtual vec operator()(vec x, vec y, vec z) {
			return Zero<vec>();
		}

	protected:
		// Virtual variant to concrete T version of the Process function
		virtual void ProcessSIMD(NoiseSamplingParameters<B, F> params, VarPtr outptr) final {
			std::visit([&](auto&& ptr) {
				ProcessSIMDImpl(params, ptr);
				}, outptr);
		}

	private:

		template <typename TOut>
		void ProcessSIMDImpl(NoiseSamplingParameters<B, F> params, TOut* HWY_RESTRICT outArray) {
			// Number of output samples
			size_t count = params.TotalSize();

			// laneCount might not evenly fit into X*Y(*Z) = count. We also don't want to write past
			// the size of the input array. So, we have a normal loop and a "remainder" write. The
			// normal loop writes normally, but the remainder write is to only write enough to fill
			// the array and not more. 
			const size_t laneCount = hn::Lanes(D<B, F>());
			size_t remainderStoreSize = count % laneCount;

			// We're always going to do the "remainder" store operation. If the lastWriteSize is 0,
			// just take all valid entries from the vector, and turn those into "remainder" stores. 
			if (remainderStoreSize == 0) remainderStoreSize = laneCount;

			// Shifts the sample to fit fully into TOut.
			auto shiftLambda = [](vec sample) {
				// Shifts the fractional part of the sample to the left, then additional shift to
				// account for the difference in size between input and output.
				constexpr int shift = (int(B) - int(F)) - 
					(int(sizeof(UT<B, F>)) - int(sizeof(TOut))) * 8;

				if constexpr (shift < 0) {
					return hn::ShiftRight<-shift>(sample);
				}
				else if constexpr (shift > 0) {
					return hn::ShiftLeft<shift>(sample);
				}
				else {
					return sample;
				}
			};
			
			// Write every chunk but the last one to the array
			int i = 0;
			for (; i < int(count) - int(laneCount); i += laneCount) {
				vec result = Sample(i, params);
				result = shiftLambda(result);
				Store<T<B, F>, TOut>(result, outArray + i);
			}

			// Write last chunk to the array
			vec result = Sample(i, params);
			result = shiftLambda(result);
			Store<T<B, F>, TOut>(result, outArray + i);
		}

		// TODO: Make this function more generic
		vec Sample(size_t i, NoiseSamplingParameters<B, F> params) {

			auto GetOffsetLambda = [&](vec indices) {
				return FPMul<B, F>(hn::ShiftLeft<F>(indices), params.Spacing);
			};

			size_t dimensions = params.GetDimensions();

			if (dimensions == 2) {
				vec idX, idY;
				VUnravel(i, params.Size(0), params.Size(1), idX, idY);
				vec sampleX = FPAdd<B, F>(params.Start(0), GetOffsetLambda(idX));
				vec sampleY = FPAdd<B, F>(params.Start(1), GetOffsetLambda(idY));
				return this->operator()(sampleX, sampleY);
			}

			else if (dimensions == 3) {
				vec idX, idY, idZ;
				VUnravel(i, params.Size(0), params.Size(1), params.Size(2), idX, idY, idZ);
				vec sampleX = FPAdd<B, F>(params.Start(0), GetOffsetLambda(idX));
				vec sampleY = FPAdd<B, F>(params.Start(1), GetOffsetLambda(idY));
				vec sampleZ = FPAdd<B, F>(params.Start(2), GetOffsetLambda(idZ));
				return this->operator()(sampleX, sampleY, sampleZ);
			}

			else {
				return Zero<vec>();
			}
		}
	};
}
HWY_AFTER_NAMESPACE();

#endif  // include guard
