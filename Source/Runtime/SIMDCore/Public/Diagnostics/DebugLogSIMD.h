// Fill out your copyright notice in the Description page of Project Settings.

// Google Highway requirement
#if defined(SIMD_DEBUGLOG_SIMD_H_) == defined(HWY_TARGET_TOGGLE)
#ifdef SIMD_DEBUGLOG_SIMD_H_
#undef SIMD_DEBUGLOG_SIMD_H_
#else
#define SIMD_DEBUGLOG_SIMD_H_
#endif

#include "hwy/highway.h"
#include "Diagnostics/DebugLog.h"
#include "OperationsSIMD.h"
#include "CoreMinimal.h"

HWY_BEFORE_NAMESPACE();
namespace SIMD::HWY_NAMESPACE
{
#define VLOG(vec, lane) VLog(vec, lane, TEXT(#vec))

	template <class V>
	constexpr void VLog(V v, size_t lane, const TCHAR* msg = TEXT("")) {
		Log(hn::ExtractLane(v, lane), msg);
	}

#define VLOG_ALL(vec) VLogAll(vec, TEXT(#vec))

	template <class V>
	constexpr void VLogAll(V v, const TCHAR* msg = TEXT("")) {
		FString text;
		int laneCount = hn::Lanes(hn::DFromV<V>());

		if (!FString(msg).IsEmpty()) {
			Log(msg);
		}

		for (int i = 0; i < laneCount; ++i) {
			Log(hn::ExtractLane(v, i), *FString::FromInt(i));
		}
	}

#define VLOG_BINARY(vec, lane) VLogBinary(vec, lane, TEXT(#vec))

	template <class V>
	constexpr void VLogBinary(V v, size_t lane, const TCHAR* msg = TEXT("")) {
		LogBinary(hn::ExtractLane(v, lane), msg);
	}

#define VLOG_BINARY_ALL(vec) VLogBinaryAll(vec, TEXT(#vec))

	template <class V>
	constexpr void VLogBinaryAll(V v, const TCHAR* msg = TEXT("")) {
		FString text;
		int laneCount = hn::Lanes(hn::DFromV<V>());

		if (!FString(msg).IsEmpty()) {
			Log(msg);
		}

		for (int i = 0; i < laneCount; ++i) {
			LogBinary(hn::ExtractLane(v, i), *FString::FromInt(i));
		}
	}

}
HWY_AFTER_NAMESPACE();
#endif  // include guard

