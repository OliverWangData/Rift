// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NoiseSamplingParameters.h"
#include "AlignedArray.h"
#include <variant>
#include <TypeTraits/VariantTypeTraits.h>


template <size_t B, size_t F>
class NodeBase
{
public:
	using VarPtr = std::variant<uint8_t*, uint16_t*, uint32_t*, uint64_t*>;

	NodeBase() = default;
	virtual ~NodeBase() = default;

	// To be implemented by nodes.
	// Note - Bounds is used instead of the sampling parameters because the spacing / count of 
	// samples cannot be assumed. Some noise like warp will mess with spacing. So caching should
	// take that into account. 
	virtual void PreProcess(const std::vector<NoiseSamplingBound<B, F>>& bounds) {

	}

	// to be implemented by nodes. Cleanup for anything created in PreProcess()
	virtual void PostProcess() {

	}

	// Non-simd accessible version of Process. 
	template <typename T> requires exists_in_variant_v<T, VarPtr, true>
	void Process(
		NoiseSamplingParameters<B, F> params, 
		AlignedArray<T>& array) {

		// Ensure the output aligned array is large enough to 
		assert(params.TotalSize() <= array.GetSize());
		ProcessSIMD(params, array.GetPtr());
	}

protected:
	// Implemented in NodeBaseSIMD.
	// This function does NOT check for bounds, nor control lifetime of the output array!!
	virtual void ProcessSIMD(NoiseSamplingParameters<B, F> params, VarPtr outArray) { }

};


