// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TypeTraits/OperatorTypeTraits.h"
#include <array>

template <typename Func, typename T, typename TOut>
concept HasApplyOperator = requires(Func f, T t) {
	{ f(t) } -> std::convertible_to<TOut>;
};

template<typename T, size_t Size>
class MathVector
{
public:
	// *********************************************************************************************
	// [Constructors]
	MathVector() = default; // Default constructor
	MathVector(const MathVector&) = default; // Copy constructor
	MathVector& operator=(const MathVector&) = default; // Copy assignment constructor

	// Constructor that has Size number of arguments of T.
	template<typename... Args, typename = std::enable_if<
		sizeof...(Args) == Size && (std::is_convertible<Args, T>::value && ...)
	>::type>
	constexpr MathVector(Args... args) : elements{ static_cast<T>(args)... } { }

	// *********************************************************************************************
	// [Math Functions]
	inline constexpr T Sum() const requires HasAdditionAssignmentOperator<T, T> {
		T t = 0;

		for (size_t i = 0; i < Size; ++i) {
			t += elements[i];
		}

		return t;
	}

	inline constexpr T Product() const requires HasMultiplicationAssignmentOperator<T, T> {
		if constexpr (Size == 0) return 0;

		T t = 1;

		for (size_t i = 0; i < Size; ++i) {
			t *= elements[i];
		}

		return t;
	}

	inline constexpr T Dot(MathVector other) const requires HasMultiplicationOperator<T, T, T> {
		T t = 0;

		for (size_t i = 0; i < Size; ++i) {
			t += elements[i] * other[i];
		}

		return t;
	}

	// *********************************************************************************************
	// [Operators]

	// Apply function

	template <typename TOut, typename Func> requires HasApplyOperator<Func, T, TOut>
	inline constexpr MathVector<TOut, Size> Apply(Func func) const {
		MathVector<TOut, Size> temp;
		for (size_t i = 0; i < Size; ++i) {
			temp[i] = func((*this).Get(i));
		}
		return temp;
	}

	// Access operator
	inline T& operator[](size_t index) {
		return elements[index];
	}

	template <size_t Index>
	inline constexpr const T& Get() const {
		static_assert(Index < Size, "Index out of bounds.");
		return elements[Index];
	}

	inline constexpr const T& Get(size_t index) const {
		return elements[index];
	}

	// Relational operators
	inline constexpr bool operator==(MathVector rhs) const requires HasNotEqualToOperator<T, T> {
		for (size_t i = 0; i < Size; ++i) {
			if ((*this)[i] != rhs[i]) {
				return false;
			}
		}

		return true;
	}

	inline constexpr bool operator!=(MathVector rhs) const requires HasEqualToOperator<T, T> {
		for (size_t i = 0; i < Size; ++i) {
			if ((*this)[i] == rhs[i]) {
				return false;
			}
		}

		return true;
	}

	// Unary operators
	inline constexpr MathVector operator~() const requires HasBitwiseNotOperator<T, T> {
		MathVector temp(*this);
		for (size_t i = 0; i < Size; ++i) {
			temp[i] = ~(*this)[i];
		}
		return temp;
	}

	inline constexpr MathVector operator-() const requires HasUnaryMinusOperator<T, T> {
		MathVector temp(*this);
		for (size_t i = 0; i < Size; ++i) {
			temp[i] = -(*this)[i];
		}
		return temp;
	}

	inline constexpr MathVector operator+() const requires HasUnaryPlusOperator<T, T> {
		MathVector temp(*this);
		for (size_t i = 0; i < Size; ++i) {
			temp[i] = +(*this)[i];
		}
		return temp;
	}

	inline constexpr MathVector operator++() const requires HasIncrementOperator<T> {
		MathVector temp(*this);
		for (size_t i = 0; i < Size; ++i) {
			temp[i]++;
		}
		return temp;
	}

	inline constexpr MathVector operator--() const requires HasIncrementOperator<T> {
		MathVector temp(*this);
		for (size_t i = 0; i < Size; ++i) {
			temp[i]--;
		}
		return temp;
	}

	// Bitwise operator

	
	// *********************************************************************************************
	// [Data Members]
private:
	std::array<T, Size> elements;
};


// *************************************************************************************************
// [Non-Member Operators]
// 
// Basic math operators

template<typename T, size_t Size>
inline constexpr MathVector<T, Size> operator+(MathVector<T, Size> lhs, MathVector<T, Size> rhs) 
	requires HasAdditionAssignmentOperator<T, T> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] += rhs[i];
	}
	return lhs;
}

template<typename T, size_t Size>
inline constexpr MathVector<T, Size> operator-(MathVector<T, Size> lhs, MathVector<T, Size> rhs) 
	requires HasSubtractionAssignmentOperator<T, T> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] -= rhs[i];
	}
	return lhs;
}

template<typename T, size_t Size>
inline constexpr MathVector<T, Size> operator*(MathVector<T, Size> lhs, MathVector<T, Size> rhs) 
	requires HasMultiplicationAssignmentOperator<T, T> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] *= rhs[i];
	}
	return lhs;
}

template<typename T, size_t Size>
inline constexpr MathVector<T, Size> operator/(MathVector<T, Size> lhs, MathVector<T, Size> rhs) 
	requires HasDivisionAssignmentOperator<T, T> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] /= rhs[i];
	}
	return lhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator+(MathVector<T, Size> lhs, Number rhs) 
	requires HasAdditionAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] += rhs;
	}
	return lhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator-(MathVector<T, Size> lhs, Number rhs) 
	requires HasSubtractionAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] -= rhs;
	}
	return lhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator*(MathVector<T, Size> lhs, Number rhs) 
	requires HasMultiplicationAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] *= rhs;
	}
	return lhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator/(MathVector<T, Size> lhs, Number rhs) 
	requires HasDivisionAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		lhs[i] /= rhs;
	}
	return lhs;
}


template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator+(Number lhs, MathVector<T, Size> rhs) 
	requires HasAdditionAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		rhs[i] += lhs;
	}
	return rhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator-(Number lhs, MathVector<T, Size> rhs) 
	requires HasSubtractionAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		rhs[i] -= lhs;
	}
	return rhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator*(Number lhs, MathVector<T, Size> rhs) 
	requires HasMultiplicationAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		rhs[i] *= lhs;
	}
	return rhs;
}

template<typename T, size_t Size, typename Number>
inline constexpr MathVector<T, Size> operator/(Number lhs, MathVector<T, Size> rhs) 
	requires HasDivisionAssignmentOperator<T, Number> {
	for (size_t i = 0; i < Size; ++i) {
		rhs[i] /= lhs;
	}
	return rhs;
}


// *************************************************************************************************
// Math functions
template<typename T, size_t Size>
inline constexpr T Dot(MathVector<T, Size> a, MathVector<T, Size> b) {
	return (a * b).Sum();
}