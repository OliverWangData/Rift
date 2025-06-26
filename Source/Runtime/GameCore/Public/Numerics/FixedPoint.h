// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TypeTraits/IntSelector.h"
#include "TypeTraits/FloatSelector.h"

#include "Diagnostics/DebugLog.h"

// *************************************************************************************************
// Forward declarations
template <size_t B, size_t F>
class FixedPoint;

template <typename T>
concept FixedPointType = requires {
	typename T::template is_specialization_of<FixedPoint>;
};

template <size_t B, size_t F>
constexpr FixedPoint<B, F> Multiply(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs);

template <size_t B, size_t F>
constexpr FixedPoint<B, F> Divide(
	FixedPoint<B, F> numerator, FixedPoint<B, F> denominator, FixedPoint<B, F>& remainder
);

// *************************************************************************************************
template <size_t B, size_t F> // Bits Size, Fraction Size
class FixedPoint
{
public:
	static_assert(B <= 64, "Unsupported size.");
	static_assert(B - F > 1, "There must be at least 2 non fraction bits, one for the integers and "
		"one for the sign.");
	// Fixed_pi starts with 011, which represented positive integer of 3. Adjust this as needed for 
	// more constants/
	static_assert(B - F >= 3, "Const implementation requires at least 3 non-fraction bits."); 

	// Alias for convenience
	using base_type = typename IntSelector<B>::signed_type;
	using unsigned_type = typename IntSelector<B>::unsigned_type;

	// *********************************************************************************************
	// [Constants]
	inline static constexpr size_t BitsSize = B;
	inline static constexpr size_t FractionSize = F;
	inline static constexpr size_t IntegerSize = B - F;

	// Bitwise operation constants
	inline static constexpr base_type FractionMask = ~((~base_type(0)) << F); // No sign bit
	inline static constexpr base_type IntegerMask = ~FractionMask; // Includes sign bit

private:
	static constexpr base_type one = base_type(1) << F;

	// *********************************************************************************************
	// [Constructors]
public:
	FixedPoint() = default; // Default constructor
	FixedPoint(const FixedPoint&) = default; // Copy constructor
	FixedPoint& operator=(const FixedPoint&) = default; // Copy assignment constructor

	// Constructor for integer input numbers, with opt-out scaling
	template <typename Number> requires std::is_integral_v<Number>
	constexpr FixedPoint(Number n) : Bits(static_cast<base_type>(n)* one) {}

	// Constructor for floating point input numbers. Ensures wrap-around on overflow
	template <typename Number> requires std::is_floating_point_v<Number>
	constexpr FixedPoint(Number n) {
		constexpr size_t bitSize = sizeof(Number) * 8;
		using fs = FloatSelector<bitSize>;
		using int_type = typename fs::int_type;

		int_type bits = *reinterpret_cast<const int_type*>(&n);
		int_type exponent = (bits & fs::ExponentMask) >> fs::MantissaSize;
		int_type value = (bits & fs::MantissaMask);

		// Sets the leading 1 in the mantissa
		if (exponent != 0) {
			value |= (int_type(1) << fs::MantissaSize);
		}

		// If the number is negative, handle two's complement conversion
		if (bits & (int_type(1) << (bitSize - 1))) {
			value = ~value + 1;
		}

		// Shift the mantissa such that both decimals line up, and then apply the exponent
		// Since both operations are shifts, we can combine them together
		int mantissaShift = static_cast<int>(F) - static_cast<int>(fs::MantissaSize);
		int exponentShift = static_cast<int>(exponent) - static_cast<int>(fs::ExponentBias);
		int totalShift = mantissaShift + exponentShift;

		value = (totalShift >= 0) ? value << totalShift : value >> -totalShift;
		Bits = static_cast<base_type>(value);
	}

	// Tag dispatched no-scaling / direct bit setting
	// This is done since there's no way to do if constexpr in constructors
	struct NoScale {};
	constexpr FixedPoint(base_type n, const NoScale&) : Bits(n) {}

	constexpr static FixedPoint FromBase(base_type n) {
		return FixedPoint(n, NoScale());
	}

	// *********************************************************************************************
	// [Operators]

	// Comparison Operators
	constexpr bool operator==(FixedPoint rhs) const {
		return Bits == rhs.Bits;
	}

	constexpr bool operator!=(FixedPoint rhs) const {
		return Bits != rhs.Bits;
	}

	constexpr bool operator<(FixedPoint rhs) const {
		return Bits < rhs.Bits;
	}

	constexpr bool operator>(FixedPoint rhs) const {
		return Bits > rhs.Bits;
	}

	constexpr bool operator<=(FixedPoint rhs) const {
		return Bits <= rhs.Bits;
	}

	constexpr bool operator>=(FixedPoint rhs) const {
		return Bits >= rhs.Bits;
	}

	// Unary Operators
	constexpr bool operator!() const {
		return !Bits;
	}

	constexpr FixedPoint operator~() const {
		return FromBase(~Bits);
	}

	constexpr FixedPoint operator-() const {
		return FromBase(-Bits);
	}

	constexpr FixedPoint operator+() const {
		return FromBase(+Bits);
	}

	constexpr FixedPoint& operator++() {
		Bits += one;
		return *this;
	}

	constexpr FixedPoint& operator--() {
		Bits -= one;
		return *this;
	}

	constexpr FixedPoint operator++(int) {
		FixedPoint tmp(*this);
		Bits += one;
		return tmp;
	}

	constexpr FixedPoint operator--(int) {
		FixedPoint tmp(*this);
		Bits -= one;
		return tmp;
	}

	// Binary operators
	constexpr FixedPoint& operator&=(FixedPoint n) {
		Bits &= n.Bits;
		return *this;
	}

	constexpr FixedPoint& operator|=(FixedPoint n) {
		Bits |= n.Bits;
		return *this;
	}

	constexpr FixedPoint& operator^=(FixedPoint n) {
		Bits ^= n.Bits;
		return *this;
	}

	template <std::integral Integer>
	constexpr FixedPoint& operator&=(Integer n) {
		Bits &= n;
		return *this;
	}

	template <std::integral Integer>
	constexpr FixedPoint& operator|=(Integer n) {
		Bits |= n;
		return *this;
	}

	template <std::integral Integer>
	constexpr FixedPoint& operator^=(Integer n) {
		Bits ^= n;
		return *this;
	}

	// Shift operators
	template <std::integral Integer>
	constexpr FixedPoint& operator>>=(Integer n) {
		Bits >>= n;
		return *this;
	}

	template <std::integral Integer>
	constexpr FixedPoint& operator<<=(Integer n) {
		Bits <<= n;
		return *this;
	}

	// Assignment math operators
	constexpr FixedPoint& operator+=(FixedPoint n) {
		Bits += n.Bits;
		return *this;
	}

	constexpr FixedPoint& operator-=(FixedPoint n) {
		Bits -= n.Bits;
		return *this;
	}

	constexpr FixedPoint& operator*=(FixedPoint n) {
		Bits = Multiply(*this, n).ToRaw();
		return *this;
	}

	constexpr FixedPoint& operator/=(FixedPoint n) {
		FixedPoint temp;
		Bits = Divide(*this, n, temp).ToRaw();
		return *this;
	}

	constexpr FixedPoint& operator%=(FixedPoint n) {
		FixedPoint remainder;
		Divide(*this, n, remainder);
		Bits = remainder.ToRaw();
		return *this;
	}

	// *********************************************************************************************
	// [Conversions]

	// Equivalent to (Bits + 0.5) >> F which rounds to the nearest int. 
	// But a shift is done before adding to prevent overflow. 
	constexpr base_type ToInt() const {
		return ((Bits >> 1) + (one >> 2)) >> (F - 1);
	}

	constexpr unsigned_type ToUInt() const {
		return static_cast<unsigned_type>(ToInt());
	}

	constexpr float ToFloat() const {
		return static_cast<float>(Bits) / one;
	}

	constexpr double ToDouble() const {
		return static_cast<double>(Bits) / one;
	}

	constexpr FString ToString() const
	{
		return FString::Printf(TEXT("%f"), this->ToDouble());
	}

	constexpr FString ToBinaryString() const
	{
		const int32 numBits = sizeof(base_type) * 8;
		FString outputString;

		for (int32 i = numBits - 1; i >= 0; --i) {
			bool isBitSet = (Bits & (base_type(1) << i)) != 0;
			outputString += isBitSet ? TEXT("1") : TEXT("0");
		}

		return outputString;
	}

	// Allows us to have constexpr getter for Bits, without limiting Bits to be a constexpr
	constexpr base_type ToRaw() const {
		return Bits;
	}

	constexpr void swap(FixedPoint& rhs) {
		using std::swap;
		swap(Bits, rhs.Bits);
	}

	// TODO: Conversions between different sized FixedPoint points
	// See: https://github.com/eteran/cpp-utilities/blob/master/fixed/include/cpp-utilities/fixed.h
	// See also: http://stackoverflow.com/questions/79677/whats-the-best-way-to-do-fixed-point-math

	// *********************************************************************************************
	// [Data Members]
	base_type Bits = 0;
};


// *************************************************************************************************
// Non-Member Math Operators
template <size_t B, size_t F>
constexpr FixedPoint<B, F> Multiply(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	using base_type = typename IntSelector<B>::signed_type;


	const base_type a_upper = lhs.ToRaw() >> F;
	const base_type b_upper = rhs.ToRaw() >> F;
	const base_type a_lower = lhs.ToRaw() & FixedPoint<B, F>::FractionMask;
	const base_type b_lower = rhs.ToRaw() & FixedPoint<B, F>::FractionMask;

	const base_type x1 = a_upper * b_upper;
	const base_type x2 = a_upper * b_lower;
	const base_type x3 = a_lower * b_upper;
	const base_type x4 = a_lower * b_lower;

	return FixedPoint<B, F>::FromBase(
		(x1 << F) + (x2 + x3) + ((x4 >> F) & FixedPoint<B, F>::FractionMask)
	);
}

// Pulled from: https://gist.github.com/Eisenwave/11372ace8dcc8334210fd6b45d0b7f91
template <size_t B, size_t F>
constexpr FixedPoint<B, F> Divide(
	FixedPoint<B, F> numerator, FixedPoint<B, F> denominator, FixedPoint<B, F>& remainder) {

	using unsigned_type = IntSelector<B>::unsigned_type;
	constexpr int bits = B;

	if (denominator == 0) {
		throw std::exception("division by zero");
	}

	int sign = 0;
	FixedPoint<B, F> quotient;

	if (numerator < 0) {
		sign ^= 1;
		numerator = -numerator;
	}

	if (denominator < 0) {
		sign ^= 1;
		denominator = -denominator;
	}

	unsigned_type n = numerator.ToRaw();
	unsigned_type d = denominator.ToRaw();
	unsigned_type x = 1;
	unsigned_type answer = 0;

	// Egyptian division algorithm
	while ((n >= d) && (((d >> (bits - 1)) & 1) == 0)) {
		x <<= 1;
		d <<= 1;
	}

	while (x != 0) {
		if (n >= d) {
			n -= d;
			answer += x;
		}

		x >>= 1;
		d >>= 1;
	}

	unsigned_type l1 = n;
	unsigned_type l2 = denominator.ToRaw();

	// calculate the lower bits (needs to be unsigned)
	while (l1 >> (bits - F) > 0) {
		l1 >>= 1;
		l2 >>= 1;
	}

	const unsigned_type lo = (l1 << F) / l2;
	quotient = FixedPoint<B, F>::FromBase((answer << F) | lo);
	remainder = n;

	if (sign) {
		quotient = -quotient;
	}

	return quotient;
}


// *************************************************************************************************
// Non-Member Output Operators
// 
// OStream works with the std string, but Unreal uses FString natively so it should be avoided.
template <size_t B, size_t F>
std::ostream& operator<<(std::ostream& os, FixedPoint<B, F> f) {
	os << f.to_double();
	return os;
}

// *************************************************************************************************
// [Non-Member Operator Overloads]

// Comparison operators with arithmetic types
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator==(FixedPoint<B, F> lhs, Number rhs) {
	return lhs == FixedPoint<B, F>(rhs);
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator!=(FixedPoint<B, F> lhs, Number rhs) {
	return lhs != FixedPoint<B, F>(rhs);
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator<(FixedPoint<B, F> lhs, Number rhs) {
	return lhs < FixedPoint<B, F>(rhs);
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator>(FixedPoint<B, F> lhs, Number rhs) {
	return lhs > FixedPoint<B, F>(rhs);
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator<=(FixedPoint<B, F> lhs, Number rhs) {
	return lhs <= FixedPoint<B, F>(rhs);
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator>=(FixedPoint<B, F> lhs, Number rhs) {
	return lhs >= FixedPoint<B, F>(rhs);
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator==(Number lhs, FixedPoint<B, F> rhs) {
	return FixedPoint<B, F>(lhs) == rhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator!=(Number lhs, FixedPoint<B, F> rhs) {
	return FixedPoint<B, F>(lhs) != rhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator<(Number lhs, FixedPoint<B, F> rhs) {
	return FixedPoint<B, F>(lhs) < rhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator>(Number lhs, FixedPoint<B, F> rhs) {
	return FixedPoint<B, F>(lhs) > rhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator<=(Number lhs, FixedPoint<B, F> rhs) {
	return FixedPoint<B, F>(lhs) <= rhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr bool operator>=(Number lhs, FixedPoint<B, F> rhs) {
	return FixedPoint<B, F>(lhs) >= rhs;
}

// Binary operators
template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator&(FixedPoint<B, F> lhs, Integer rhs) {
	lhs &= rhs;
	return lhs;
}

template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator|(FixedPoint<B, F> lhs, Integer rhs) {
	lhs |= rhs;
	return lhs;
}

template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator^(FixedPoint<B, F> lhs, Integer rhs) {
	lhs ^= rhs;
	return lhs;
}

template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator&(Integer lhs, FixedPoint<B, F> rhs) {
	rhs &= lhs;
	return rhs;
}

template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator|(Integer lhs, FixedPoint<B, F> rhs) {
	rhs |= lhs;
	return rhs;
}

template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator^(Integer lhs, FixedPoint<B, F> rhs) {
	rhs ^= lhs;
	return rhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator&(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs &= rhs;
	return lhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator|(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs |= rhs;
	return lhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator^(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs ^= rhs;
	return lhs;
}

// Shift operators
template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator<<(FixedPoint<B, F> lhs, Integer rhs) {
	lhs <<= rhs;
	return lhs;
}

template <size_t B, size_t F, std::integral Integer>
inline constexpr FixedPoint<B, F> operator>>(FixedPoint<B, F> lhs, Integer rhs) {
	lhs >>= rhs;
	return lhs;
}

// Basic math operators
template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator+(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs += rhs;
	return lhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator-(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs -= rhs;
	return lhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator*(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs *= rhs;
	return lhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator/(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs /= rhs;
	return lhs;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> operator%(FixedPoint<B, F> lhs, FixedPoint<B, F> rhs) {
	lhs %= rhs;
	return lhs;
}

// Basic math operators with arithmetic types
template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator+(FixedPoint<B, F> lhs, Number rhs) {
	lhs += FixedPoint<B, F>(rhs);
	return lhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator-(FixedPoint<B, F> lhs, Number rhs) {
	lhs -= FixedPoint<B, F>(rhs);
	return lhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator*(FixedPoint<B, F> lhs, Number rhs) {
	lhs *= FixedPoint<B, F>(rhs);
	return lhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator/(FixedPoint<B, F> lhs, Number rhs) {
	lhs /= FixedPoint<B, F>(rhs);
	return lhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator%(FixedPoint<B, F> lhs, Number rhs) {
	lhs %= FixedPoint<B, F>(rhs);
	return lhs;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator+(Number lhs, FixedPoint<B, F> rhs) {
	FixedPoint<B, F> temp(lhs);
	temp += rhs;
	return temp;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator-(Number lhs, FixedPoint<B, F> rhs) {
	FixedPoint<B, F> temp(lhs);
	temp -= rhs;
	return temp;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator*(Number lhs, FixedPoint<B, F> rhs) {
	FixedPoint<B, F> temp(lhs);
	temp *= rhs;
	return temp;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator/(Number lhs, FixedPoint<B, F> rhs) {
	FixedPoint<B, F> temp(lhs);
	temp /= rhs;
	return temp;
}

template <size_t B, size_t F, typename Number> requires std::is_arithmetic_v<Number>
inline constexpr FixedPoint<B, F> operator%(Number lhs, FixedPoint<B, F> rhs) {
	FixedPoint<B, F> temp(lhs);
	temp %= rhs;
	return temp;
}

// *************************************************************************************************
// Math functions
template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Max(FixedPoint<B, F> a, FixedPoint<B, F> b) {
	return (a >= b) ? a : b;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Min(FixedPoint<B, F> a, FixedPoint<B, F> b) {
	return (a <= b) ? a : b;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Abs(FixedPoint<B, F> value) {
	return value & ~(FixedPoint<B, F>::base_type(1) << (B - 1));
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Lerp(FixedPoint<B, F> a, FixedPoint<B, F> b, FixedPoint<B, F> t) {
	return a + (b - a) * t;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Clamp(
	FixedPoint<B, F> value,
	FixedPoint<B, F> a, FixedPoint<B, F> b) {
	if (value < a) return a;
	else if (b < value) return b;
	return value;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Round(FixedPoint<B, F> value) {
	return (value + 0.5) & FixedPoint<B, F>::IntegerMask;
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Floor(FixedPoint<B, F> value) {
	return (value & FixedPoint<B, F>::IntegerMask);
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Floor(FixedPoint<B, F> value, int n) {
	return (value & (FixedPoint<B, F>::IntegerMask >> n));
}

template <size_t B, size_t F, bool roundUpIntegers = false>
inline constexpr FixedPoint<B, F> Ceil(FixedPoint<B, F> value) {
	if constexpr (roundUpIntegers) {
		return Floor<B, F>(value) + 1;
	}
	else {
		bool hasFrac = (value & FixedPoint<B, F>::FractionMask) != 0;
		return Floor<B, F>(value) + (hasFrac ? 1 : 0);
	}
}

template <size_t B, size_t F, bool roundUpIntegers = false>
inline constexpr FixedPoint<B, F> Ceil(FixedPoint<B, F> value, int n) {
	if constexpr (roundUpIntegers) {
		return Floor<B, F>(value, n) + FixedPoint<B, F>::FromBase((1 << F) >> n);
	}
	else {
		bool hasFrac = ((value & FixedPoint<B, F>::FractionMask) >> n) != 0;
		return Floor<B, F>(value, n) + (hasFrac ? FixedPoint<B, F>::FromBase((1 << F) >> n) : 0);
	}
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> RoundToInterval(
	FixedPoint<B, F> value, FixedPoint<B, F> interval) {
	return interval * Round(value / interval);
}

template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> FloorToInterval(
	FixedPoint<B, F> value, FixedPoint<B, F> interval) {
	return interval * ((value / interval) & FixedPoint<B, F>::IntegerMask);
}

// Newton-Raphson Square Root
template <size_t B, size_t F>
inline constexpr FixedPoint<B, F> Sqrt(FixedPoint<B, F> value, unsigned int iterations = 8) {
	using fp = FixedPoint<B, F>;

	if (value < 0) {
		throw std::exception("negative root.");
	}

	fp k = (value + 1) >> 1;

	for (unsigned int i = 0; i < iterations; ++i) {
		if (k == 0) return 0;
		k = (k + (value / k)) >> 1;
	}

	return k;
}

template <size_t B, size_t F, size_t N>
inline constexpr FixedPoint<B, F> Sqrt(FixedPoint<B, F> value, unsigned int iterations = 8) {
	using fp = FixedPoint<B, F>;

	if (value < 0) {
		throw std::exception("negative root.");
	}

	fp k = (value >> 1) + (1 << (F - 1));

	for (unsigned int i = 0; i < iterations; ++i) {
		fp kPower = k;

		for (size_t j = 1; j < N - 1; ++j) {
			kPower *= k;
		}

		if (k == 0) return 0;
		fp valueOverKPower = value / kPower;
		k = (k * (fp(N - 1) / fp(N))) + (valueOverKPower * (fp(1) / fp(N)));
	}

	return k;
}