// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

template <size_t T>
struct IntSelector {
	static_assert(T != T, "IntSelector has no specialization for the given size.");
	using signed_type = void;
	using unsigned_type = void;
};

template<>
struct IntSelector<8> {
	using signed_type = int8_t;
	using unsigned_type = uint8_t;
};

template <>
struct IntSelector<16> {
	using signed_type = int16_t;
	using unsigned_type = uint16_t;
};

template<>
struct IntSelector<32> {
	using signed_type = int32_t;
	using unsigned_type = uint32_t;
};

template<>
struct IntSelector<64> {
	using signed_type = int64_t;
	using unsigned_type = uint64_t;
};
