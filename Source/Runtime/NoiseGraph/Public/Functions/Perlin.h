// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(NOISEGRAPH_FUNCTIONS_PERLIN_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef NOISEGRAPH_FUNCTIONS_PERLIN_SIMD_H_
#undef NOISEGRAPH_FUNCTIONS_PERLIN_SIMD_H_
#else
#define NOISEGRAPH_FUNCTIONS_PERLIN_SIMD_H_
#endif

#include "hwy/highway.h"

#include <array>
#include "OperationsSIMD.h"
#include "Numerics/FixedPoint.h"
#include "Numerics/FixedPointConstants.h"
#include "Numerics/FixedPointSIMD.h"
#include "Random.h"

#include "Diagnostics/DebugLog.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	template <size_t B, size_t F>
	inline constexpr V<B, F> PerlinDotGradient(
		V<B, F> x, V<B, F> y, 
		V<B, F> ix, V<B, F> iy, FixedPoint<B, F> seed
	) {
		using vec = V<B, F>;
		using fpc = FixedPointConstant<B, F>;

		HWY_ALIGN static std::array<T<B, F>, 32> unitTable = {
			// Cardinals
			fpc::One.ToRaw(),						0,										// 0
			0,										fpc::One.ToRaw(),						// 90
			-fpc::One.ToRaw(),						0,										// 180
			0,										-fpc::One.ToRaw(),						// 270

			// Diagonals
			(fpc::Sqrt2 >> 1).ToRaw(),				(fpc::Sqrt2 >> 1).ToRaw(),				// 45
			-(fpc::Sqrt2 >> 1).ToRaw(),				(fpc::Sqrt2 >> 1).ToRaw(),				// 135
			-(fpc::Sqrt2 >> 1).ToRaw(),				-(fpc::Sqrt2 >> 1).ToRaw(),				// 225
			(fpc::Sqrt2 >> 1).ToRaw(),				-(fpc::Sqrt2 >> 1).ToRaw(),				// 315

			// 22.5 Degree Increments
			(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),		(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),		// 22.5
			(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),		(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),		// 67.5
			-(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),	(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),		// 112.5
			-(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),	(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),		// 157.5
			-(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),	-(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),	// 202.5
			-(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),	-(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),	// 247.5
			(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw(),		-(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),	// 292.5
			(fpc::Sqrt_2AddSqrt2 >> 1).ToRaw(),		-(fpc::Sqrt_2SubSqrt2 >> 1).ToRaw()		// 337.5
		};

		const D<B, F> d;

		vec deltaX = FPSub<B, F>(x, ix);
		vec deltaY = FPSub<B, F>(y, iy);
		// Non-FP operations for index. We want to keep the rightmost 4 bits (For up to 16 digits)
		vec idx = hn::ShiftLeft<1>(And(Random<B, F>(ix, iy, seed.ToRaw()), 15));
		vec gradX = hn::GatherIndex(d, unitTable.data(), idx);
		vec gradY = hn::GatherIndex(d, unitTable.data(), Add(idx, 1));

		return FPAdd<B, F>(
			FPMul<B, F>(deltaX, gradX),
			FPMul<B, F>(deltaY, gradY)
		);
	}

	template <size_t B, size_t F>
	inline constexpr V<B, F> PerlinDotGradient(
		V<B, F> x, V<B, F> y, V<B, F> z,
		V<B, F> ix, V<B, F> iy, V<B, F> iz, FixedPoint<B, F> seed) {
		using vec = V<B, F>;
		using fpc = FixedPointConstant<B, F>;

		HWY_ALIGN static std::array<T<B, F>, 48> unitTable = {
			fpc::InvSqrt2.ToRaw(),	fpc::InvSqrt2.ToRaw(),	0,
			fpc::InvSqrt2.ToRaw(),	-fpc::InvSqrt2.ToRaw(),	0,
			-fpc::InvSqrt2.ToRaw(),	fpc::InvSqrt2.ToRaw(),	0,
			-fpc::InvSqrt2.ToRaw(),	-fpc::InvSqrt2.ToRaw(),	0,

			fpc::InvSqrt2.ToRaw(),	0,						fpc::InvSqrt2.ToRaw(),
			fpc::InvSqrt2.ToRaw(),	0,						-fpc::InvSqrt2.ToRaw(),
			-fpc::InvSqrt2.ToRaw(),	0,						fpc::InvSqrt2.ToRaw(),
			-fpc::InvSqrt2.ToRaw(),	0,						-fpc::InvSqrt2.ToRaw(),

			0,						fpc::InvSqrt2.ToRaw(),	fpc::InvSqrt2.ToRaw(),
			0,						fpc::InvSqrt2.ToRaw(),	-fpc::InvSqrt2.ToRaw(),
			0,						-fpc::InvSqrt2.ToRaw(),	fpc::InvSqrt2.ToRaw(),
			0,						-fpc::InvSqrt2.ToRaw(),	-fpc::InvSqrt2.ToRaw(),

			fpc::InvSqrt3.ToRaw(),	fpc::InvSqrt3.ToRaw(),	fpc::InvSqrt3.ToRaw(),
			fpc::InvSqrt3.ToRaw(),	fpc::InvSqrt3.ToRaw(),	-fpc::InvSqrt3.ToRaw(),
			fpc::InvSqrt3.ToRaw(),	-fpc::InvSqrt3.ToRaw(),	fpc::InvSqrt3.ToRaw(),
			-fpc::InvSqrt3.ToRaw(),	fpc::InvSqrt3.ToRaw(),	fpc::InvSqrt3.ToRaw()
		};

		const D<B, F> d;

		vec deltaX = FPSub<B, F>(x, ix);
		vec deltaY = FPSub<B, F>(y, iy);
		vec deltaZ = FPSub<B, F>(z, iz);

		// Non-FP operations for index. We want to keep the rightmost 4 bits (For up to 16 digits)
		vec idx = Mul(And(Random<B, F>(ix, iy, iz, seed.ToRaw()), 15), 3);
		vec gradX = hn::GatherIndex(d, unitTable.data(), idx);
		vec gradY = hn::GatherIndex(d, unitTable.data(), Add(idx, 1));
		vec gradZ = hn::GatherIndex(d, unitTable.data(), Add(idx, 2));

		return FPAdd<B, F>(
			FPAdd<B, F>(
				FPMul<B, F>(deltaX, gradX),
				FPMul<B, F>(deltaY, gradY)
			), 
			FPMul<B, F>(deltaZ, gradZ)
		);
	}

	// Equivalent to:
	// t3 * ((6 * t2) - (15 * t) + 10)
	template <size_t B, size_t F>
	HWY_INLINE constexpr V<B, F> PerlinFade(V<B, F> t) {

		V<B, F> t2 = FPMul<B, F>(t, t);
		V<B, F> t3 = FPMul<B, F>(t2, t);

		return FPMul<B, F>(
			t3, 
			FPAdd<B, F>(
				FPSub<B, F>(
					FPMul<B, F>(6, t2),
					FPMul<B, F>(15, t)), 
				10
			)
		);
	}

	template <size_t B, size_t F>
	inline constexpr V<B, F> Perlin(V<B, F> x, V<B, F> y, FixedPoint<B, F> seed) {
		using vec = V<B, F>;
		// Grid coordinates
		vec x0 = FPFloor<B, F>(x);
		vec y0 = FPFloor<B, F>(y);
		vec x1 = FPAdd<B, F>(x0, 1);
		vec y1 = FPAdd<B, F>(y0, 1);

		// Dot products
		vec d00 = PerlinDotGradient<B, F>(x, y, x0, y0, seed);
		vec d01 = PerlinDotGradient<B, F>(x, y, x0, y1, seed);
		vec d10 = PerlinDotGradient<B, F>(x, y, x1, y0, seed);
		vec d11 = PerlinDotGradient<B, F>(x, y, x1, y1, seed);

		// Fade lerps
		vec xf = PerlinFade<B, F>(x - x0);
		vec yf = PerlinFade<B, F>(y - y0);
		vec d0010 = FPLerp<B, F>(d00, d10, xf);
		vec d0111 = FPLerp<B, F>(d01, d11, xf);

		// Result falls in range [-sqrt(N/4), sqrt(N/4)] where N is the dimension.
		// See: https://digitalfreepen.com/2017/06/20/range-perlin-noise.html
		vec result = FPLerp<B, F>(d0010, d0111, yf);
		result = FPMul<B, F>(result, FixedPointConstant<B, F>::Sqrt2); // Scaling to [-1, 1]
		return hn::ShiftRight<1>(FPAdd<B, F>(result, 1)); // Scaling to [0, 1]
	}

	template <size_t B, size_t F>
	inline constexpr V<B, F> Perlin(
		V<B, F> x, V<B, F> y, V<B, F> z, FixedPoint<B, F> seed
	) {
		using vec = V<B, F>;

		// Grid coordinates
		vec x0 = FPFloor<B, F>(x);
		vec y0 = FPFloor<B, F>(y);
		vec z0 = FPFloor<B, F>(z);
		vec x1 = FPAdd<B, F>(x0, 1);
		vec y1 = FPAdd<B, F>(y0, 1);
		vec z1 = FPAdd<B, F>(z0, 1);

		// Dot products
		vec d000 = PerlinDotGradient<B, F>(x, y, z, x0, y0, z0, seed);
		vec d001 = PerlinDotGradient<B, F>(x, y, z, x0, y0, z1, seed);
		vec d010 = PerlinDotGradient<B, F>(x, y, z, x0, y1, z0, seed);
		vec d011 = PerlinDotGradient<B, F>(x, y, z, x0, y1, z1, seed);
		vec d100 = PerlinDotGradient<B, F>(x, y, z, x1, y0, z0, seed);
		vec d101 = PerlinDotGradient<B, F>(x, y, z, x1, y0, z1, seed);
		vec d110 = PerlinDotGradient<B, F>(x, y, z, x1, y1, z0, seed);
		vec d111 = PerlinDotGradient<B, F>(x, y, z, x1, y1, z1, seed);

		// Fade lerps
		vec xf = PerlinFade<B, F>(x - x0);
		vec yf = PerlinFade<B, F>(y - y0);
		vec zf = PerlinFade<B, F>(z - z0);

		// Interpolate in x-direction
		vec d000100 = FPLerp<B, F>(d000, d100, xf);
		vec d001101 = FPLerp<B, F>(d001, d101, xf);
		vec d010110 = FPLerp<B, F>(d010, d110, xf);
		vec d011111 = FPLerp<B, F>(d011, d111, xf);

		// Interpolate in y-direction
		vec d00_10 = FPLerp<B, F>(d000100, d010110, yf);
		vec d01_11 = FPLerp<B, F>(d001101, d011111, yf);

		// Result falls in range [-sqrt(N/4), sqrt(N/4)] where N is the dimension.
		// See: https://digitalfreepen.com/2017/06/20/range-perlin-noise.html
		vec result = FPLerp<B, F>(d00_10, d01_11, zf);
		result = FPMul<B, F>(result, FixedPointConstant<B, F>::Sqrt3); // Scaling to [-1, 1]
		return hn::ShiftRight<1>(FPAdd<B, F>(result, 1)); // to [0, 1]
	}
}
HWY_AFTER_NAMESPACE();


#endif  // include guard