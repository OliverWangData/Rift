// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

template <size_t T>
struct FloatSelector {
	static_assert(T != T, "FloatSelector has no specialization for the given size.");
};

template<>
struct FloatSelector<32> {
	using base_type = float;
	using int_type = int32_t;
	static constexpr size_t ExponentSize = 8;
	static constexpr size_t MantissaSize = 23;
	static constexpr size_t ExponentBias = 127;
	static constexpr int_type ExponentMask = 0x7F800000;
	static constexpr int_type MantissaMask = 0x007FFFFF;
};

template<>
struct FloatSelector<64> {
	using base_type = double;
	using int_type = int64_t;
	static constexpr size_t ExponentSize = 11;
	static constexpr size_t MantissaSize = 52;
	static constexpr size_t ExponentBias = 1023;
	static constexpr int_type ExponentMask = 0x7FF0000000000000;
	static constexpr int_type MantissaMask = 0x000FFFFFFFFFFFFF;
};
