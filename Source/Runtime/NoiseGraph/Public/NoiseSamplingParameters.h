// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"

template <size_t B, size_t F>
struct NoiseSamplingBound
{
	FixedPoint<B, F> Start;
	FixedPoint<B, F> End;
};

template <size_t B, size_t F>
struct NoiseSamplingParameters
{
	FixedPoint<B, F> Spacing = FixedPointConstant<B, F>::One;
protected:
	std::vector<int> Sizes;
	std::vector<NoiseSamplingBound<B, F>> Bounds;

public:

	NoiseSamplingParameters() = default;
	NoiseSamplingParameters(FixedPoint<B, F> spacing) : Spacing(spacing) {}

	void Add(FixedPoint<B, F> start, int size) {
		Sizes.push_back(size);

		NoiseSamplingBound<B, F> bound;
		bound.Start = start;
		bound.End = start + (Spacing * size);
		Bounds.push_back(bound);
	}

	// Read-only vector accessors
	inline const std::vector<NoiseSamplingBound<B, F>>& GetBounds() const {
		return Bounds;
	}

	inline const std::vector<int>& GetSizes() const {
		return Sizes;
	}

	inline const int Size(size_t index) const {
		return Sizes[index];
	}

	inline const FixedPoint<B, F> Start(size_t index) const {
		return Bounds[index].Start;
	}

	inline const FixedPoint<B, F> End(size_t index) const {
		return Bounds[index].End;
	}



	const int TotalSize() const {
		size_t dimensions = Sizes.size();

		if (dimensions == 0) {
			return 0;
		}

		size_t total = 1;

		for (size_t i = 0; i < dimensions; ++i) {
			total *= Sizes[i];
		}

		return total;
	}

	const int GetDimensions() const {
		return Sizes.size();
	}

};