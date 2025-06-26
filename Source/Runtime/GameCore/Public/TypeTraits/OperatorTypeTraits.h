// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


// *************************************************************************************************
// [Arithmetic]
template<typename Out, typename T1, typename T2>
concept HasAdditionOperator = requires(T1 a, T2 b) { { a + b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasSubtractionOperator = requires(T1 a, T2 b) { { a - b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasMultiplicationOperator = requires(T1 a, T2 b) { { a * b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasDivisionOperator = requires(T1 a, T2 b) { { a / b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasModuloOperator = requires(T1 a, T2 b) { { a % b } -> std::convertible_to<Out>; };

template<typename Out, typename T1>
concept HasUnaryPlusOperator = requires(T1 a) { { +a } -> std::convertible_to<Out>; };

template<typename Out, typename T1>
concept HasUnaryMinusOperator = requires(T1 a) { { -a } -> std::convertible_to<Out>; };

// *************************************************************************************************
// [Relational]
template<typename T1, typename T2>
concept HasEqualToOperator = requires(T1 a, T2 b) { { a == b }; };

template<typename T1, typename T2>
concept HasNotEqualToOperator = requires(T1 a, T2 b) { { a != b }; };

template<typename T1, typename T2>
concept HasLessThanOperator = requires(T1 a, T2 b) { { a < b }; };

template<typename T1, typename T2>
concept HasLessThanOrEqualToOperator = requires(T1 a, T2 b) { { a <= b }; };

template<typename T1, typename T2>
concept HasGreaterThanOperator = requires(T1 a, T2 b) { { a > b }; };

template<typename T1, typename T2>
concept HasGreaterThanOrEqualToOperator = requires(T1 a, T2 b) { { a >= b }; };

// *************************************************************************************************
// [Logical]

// &&
// ||
// !

// *************************************************************************************************
// [Bitwise]

template<typename Out, typename T1, typename T2>
concept HasBitwiseAndOperator = requires(T1 a, T2 b) { { a & b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasBitwiseOrOperator = requires(T1 a, T2 b) { { a | b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasBitwiseXorOperator = requires(T1 a, T2 b) { { a ^ b } -> std::convertible_to<Out>; };

template<typename Out, typename T1>
concept HasBitwiseNotOperator = requires(T1 a) { { ~a } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasLeftShiftOperator = requires(T1 a, T2 b) { { a << b } -> std::convertible_to<Out>; };

template<typename Out, typename T1, typename T2>
concept HasRightShiftOperator = requires(T1 a, T2 b) { { a >> b } -> std::convertible_to<Out>; };

// *************************************************************************************************
// [Assignment]

template<typename T1, typename T2>
concept HasAdditionAssignmentOperator = requires(T1 a, T2 b) { { a += b }; };

template<typename T1, typename T2>
concept HasSubtractionAssignmentOperator = requires(T1 a, T2 b) { { a -= b }; };

template<typename T1, typename T2>
concept HasMultiplicationAssignmentOperator = requires(T1 a, T2 b) { { a *= b }; };

template<typename T1, typename T2>
concept HasDivisionAssignmentOperator = requires(T1 a, T2 b) { { a /= b }; };

template<typename T1, typename T2>
concept HasModuloAssignmentOperator = requires(T1 a, T2 b) { { a %= b }; };

template<typename T1, typename T2>
concept HasAndAssignmentOperator = requires(T1 a, T2 b) { { a &= b }; };

template<typename T1, typename T2>
concept HasOrAssignmentOperator = requires(T1 a, T2 b) { { a |= b }; };

template<typename T1, typename T2>
concept HasXorAssignmentOperator = requires(T1 a, T2 b) { { a ^= b }; };

template<typename T1, typename T2>
concept HasLeftShiftAssignmentOperator = requires(T1 a, T2 b) { { a <<= b }; };

template<typename T1, typename T2>
concept HasRightShiftAssignmentOperator = requires(T1 a, T2 b) { { a >>= b }; };

// *************************************************************************************************
// [Others]

// Increment / Decrement
template<typename T1>
concept HasIncrementOperator = requires(T1 a) { { a++ }; };

template<typename T1>
concept HasDecrementOperator = requires(T1 a) { { a-- }; };