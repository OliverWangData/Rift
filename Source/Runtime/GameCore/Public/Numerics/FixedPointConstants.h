// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FixedPoint.h"

// *************************************************************************************************
// [Value Constants]
// Constants that are represented in binary literals are to avoid non-deterministic float 
// arithmetics for conversions. Left-most digit is signed bit. 
// 
// Careful when adding negative number constant - the right shift will usually duplicate the sign 
// instead of add 0s, so you'll want to take that into account

// Remark
// This is the best way to do compile-time constants. The alternative is functions.
// See: https://stackoverflow.com/questions/28755416/best-way-of-defining-a-compile-time-constant
template <size_t B, size_t F>
class FixedPointConstant
{
private:
	using fixed_type = FixedPoint<B, F>;
	using base_type = fixed_type::base_type;

	// *********************************************************************************************
	// Static class so no constructors
	FixedPointConstant() = delete; // Default constructor
	FixedPointConstant(const FixedPointConstant&) = delete; // Copy
	FixedPointConstant& operator=(const FixedPointConstant&) = delete; // Copy assign
	FixedPointConstant(FixedPointConstant&&) = delete; // Move
	FixedPointConstant& operator=(FixedPointConstant&&) = delete; // Move assign
	~FixedPointConstant() = delete; // Destructor
	// *********************************************************************************************

public:
#define FPC_ATTR inline static constexpr

	FPC_ATTR fixed_type Zero = fixed_type(0);

	FPC_ATTR fixed_type One = fixed_type::FromBase(base_type(1) << F);

	FPC_ATTR fixed_type Max = fixed_type::FromBase(std::numeric_limits<base_type>::max());

	FPC_ATTR fixed_type Min = fixed_type::FromBase(std::numeric_limits<base_type>::min());

	FPC_ATTR fixed_type Pi = fixed_type::FromBase(static_cast<base_type>(
		0b011'0010010000111111011010101000100010000101101000110000100011010 >> (64 - F - 3)));

	FPC_ATTR fixed_type Sqrt2 = fixed_type::FromBase(static_cast<base_type>(
		0b01'01101010000010011110011001100111111100111011110011001001000010 >> (64 - F - 2)));

	FPC_ATTR fixed_type Sqrt3 = fixed_type::FromBase(static_cast<base_type>(
		0b01'10111011011001111010111010000101100001001100101010100111001110 >> (64 - F - 2)));

	FPC_ATTR fixed_type InvSqrt2 = fixed_type::FromBase(static_cast<base_type>(
		0b0'101101010000010011110011001100111111100111011110011010000000000 >> (64 - F - 1)));

	FPC_ATTR fixed_type InvSqrt3 = fixed_type::FromBase(static_cast<base_type>(
		0b0'100100111011010001111111010111101101010100010111100101000111101 >> (64 - F - 1)));

	FPC_ATTR fixed_type Sqrt_2AddSqrt2 = fixed_type::FromBase(static_cast<base_type>(
		0b01'11011011110001001010101110101001001101010111010111111011101010 >> (64 - F - 2)));

	FPC_ATTR fixed_type Sqrt_2SubSqrt2 = fixed_type::FromBase(static_cast<base_type>(
		0b0'110001001110010100111100011010001101111011000010101101001110011 >> (64 - F - 1)));

#undef FPC_ATTR
};
