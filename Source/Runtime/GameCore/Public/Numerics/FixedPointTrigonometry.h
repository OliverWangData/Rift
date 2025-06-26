// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FixedPoint.h"
#include "FixedPointConstants.h"
#include "MathVector.h"

// Finds the minimum distance point on the line between line segment AB and the point P.
template <size_t B, size_t F>
MathVector<FixedPoint<B, F>, 2> MinimumDistancePoint(
	MathVector<FixedPoint<B, F>, 2> a, MathVector<FixedPoint<B, F>, 2> b, 
	MathVector<FixedPoint<B, F>, 2> p) {
	const FixedPoint<B, F> lengthSquared = (b - a).Dot(b - a);

	// Case where A == B
	if (lengthSquared == 0) return a;

	// Consider the line extending the segment, parameterized as A + t (B - A).
	// We find projection of point P onto the line. 
	// It falls where t = [(P - A) dot (B-A)] / |B-A|^2
	// We clamp t from [0,1] to handle points outside the segment vw.
	const FixedPoint<B, F> t = Clamp<B, F>((p - a).Dot(b - a) / lengthSquared, 0, 1);
	return a + t * (b - a);
}