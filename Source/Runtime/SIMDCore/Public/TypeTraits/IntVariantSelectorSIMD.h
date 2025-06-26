// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "hwy/base.h"
#include <variant>

template <size_t T>
struct IntVariantSelectorSIMD {
	static_assert(T != T, "IntSelector has no specialization for the given size.");
	using signed_ptr_restrict = void;
	using unsigned_ptr_restrict = void;
};

template<>
struct IntVariantSelectorSIMD<8> {
	using signed_ptr_restrict = std::variant<int8_t* HWY_RESTRICT>;
	using unsigned_ptr_restrict = std::variant<uint8_t* HWY_RESTRICT>;

};

template <>
struct IntVariantSelectorSIMD<16> {
	using signed_ptr_restrict = std::variant<int8_t* HWY_RESTRICT, int16_t* HWY_RESTRICT>;
	using unsigned_ptr_restrict = std::variant<uint8_t* HWY_RESTRICT, uint16_t* HWY_RESTRICT>;
};

template<>
struct IntVariantSelectorSIMD<32> {
	using signed_ptr_restrict = std::variant<
		int8_t* HWY_RESTRICT, int16_t* HWY_RESTRICT, 
		int32_t* HWY_RESTRICT
	>;
	using unsigned_ptr_restrict = std::variant<
		uint8_t* HWY_RESTRICT, uint16_t* HWY_RESTRICT, 
		uint32_t* HWY_RESTRICT
	>;
};

template<>
struct IntVariantSelectorSIMD<64> {
	using signed_ptr_restrict = std::variant<
		int8_t* HWY_RESTRICT, int16_t* HWY_RESTRICT, 
		int32_t* HWY_RESTRICT, int64_t* HWY_RESTRICT
	>;
	using unsigned_ptr_restrict = std::variant<
		uint8_t* HWY_RESTRICT, uint16_t* HWY_RESTRICT, 
		uint32_t* HWY_RESTRICT, uint64_t* HWY_RESTRICT
	>;
};
