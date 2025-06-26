// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(SIMD_INDEXING_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef SIMD_INDEXING_SIMD_H_
#undef SIMD_INDEXING_SIMD_H_
#else
#define SIMD_INDEXING_SIMD_H_
#endif

#include "hwy/highway.h"
#include "OperationsSIMD.h"
#include "Mathematics/Indexing.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
	// Uses vector math to create index vectors for the consecutive indices
	// Ordering follows the order from GameCore Indexing.h
	//
	// How this works is the following steps:
	// xc, yc, zc	are the counting vectors that count up from the current x, y, z
	// yd, zd		are the delta vectors that determine how much to add to the count vector
	// 
	// Example with x = 7, y = 2, z = 6, sizeX = 8, sizeY = 3, sizeZ = 7
	// 1. Create xc.				E.g. 6, 7, 8, 9,10,11,12,13
	// 2. Use xc to create x.		E.g. 6, 7, 0, 1, 2, 3, 4, 5
	// 3. Use xc to create yd.		E.g. 0, 0, 1, 1, 1, 1, 1, 1
	// 4. Use yd to create yc.		E.g. 2, 2, 3, 3, 3, 3, 3, 3
	// 5. Use yc to create y.		E.g. 2, 2, 0, 0, 0, 0, 0, 0
	// 6. Use yc to create zd.		E.g. 0, 0, 1, 1, 1, 1, 1, 1
	// 7. Use zd to create zc.		E.g. 6, 6, 7, 7, 7, 7, 7, 7
	// 8. Use zc to create z.		E.g. 6, 6, 0, 0, 0, 0, 0, 0
	//
	// So in the end, we have the following vectors:
	// x = [6, 7, 0, 1, 2, 3, 4, 5]
	// y = [2, 2, 0, 0, 0, 0, 0, 0]
	// z = [3, 3, 4, 4, 4, 4, 4, 4]
	// 
	// Which corresponds to consecutive 3d array indices in Indexing.h
	//
	// Note:
	// Because lane size is fixed, the Z index may end up wrapping around to 0. This is to prevent
	// retrieving data from unallocated memory. Ensure some technique like array padding or some
	// other check is implemented before using these indices for writing to memory. 
	//
	template <class V>
	HWY_INLINE void VUnravel(
		int startX, int startY, int startZ, int sizeX, int sizeY, int sizeZ,
		V& x, V& y, V& z
	) {
		const hn::DFromV<V> d;

		V xc = hn::Iota(d, startX);
		x = sn::Mod(xc, sizeX);
		V yd = sn::Div(xc, sizeX);
		V yc = sn::Add(yd, startY);
		y = sn::Mod(yc, sizeY);
		V zd = sn::Div(yc, sizeY);
		V zc = sn::Add(zd, startZ);
		z = sn::Mod(zc, sizeZ);
	}

	template <class V>
	HWY_INLINE void VUnravel(
		int i, int sizeX, int sizeY, int sizeZ,
		V& x, V& y, V& z
	) {
		int startX, startY, startZ;
		Unravel(i, sizeX, sizeY, startX, startY, startZ);
		VUnravel(startX, startY, startZ, sizeX, sizeY, sizeZ, x, y, z);
	}

	// See 3D version for implementation detail.s
	template <class V>
	HWY_INLINE void VUnravel(
		int startX, int startY, int sizeX, int sizeY, V& x, V& y
	) {
		const hn::DFromV<V> d;

		V xc = hn::Iota(d, startX);
		x = sn::Mod(xc, sizeX);
		V yd = sn::Div(xc, sizeX);
		V yc = sn::Add(yd, startY);
		y = sn::Mod(yc, sizeY);
	}

	template <class V>
	HWY_INLINE void VUnravel(int i, int sizeX, int sizeY, V& x, V& y) {
		int startX, startY;
		Unravel(i, sizeX, startX, startY);
		VUnravel(startX, startY, sizeX, sizeY, x, y);
	}

	template <class V>
	HWY_INLINE V VFlatten(V x, V y, int sizeX) {
		return sn::Add(sn::Mul(y, sizeX), x);
	}

	template <class V>
	HWY_INLINE V VFlatten(V x, V y, V z, int sizeX, int sizeY) {
		return sn::Add(
			sn::Mul(z, sizeX * sizeY),
			sn::Add(sn::Mul(y, sizeX), x)
		);
	}
}
HWY_AFTER_NAMESPACE();
#endif  // include guard

