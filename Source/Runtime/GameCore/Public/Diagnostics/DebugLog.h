// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TypeTraits/FloatSelector.h"

#define MODULE_API GAMECORE_API

template <typename T>
concept HasToStringMemberFunction = requires(T a) { 
	{ a.ToString() } -> std::convertible_to<FString>;
};

template <typename T>
concept HasToBinaryStringMemberFunction = requires(T a) { 
	{ a.ToBinaryString() } -> std::convertible_to<FString>;
};

template<typename T> requires std::is_integral_v<T>
constexpr const TCHAR* GetString(T t) {
	return TEXT("%d");
}

template<typename T> requires std::is_floating_point_v<T>
constexpr const TCHAR* GetString(T t) {
	return TEXT("%f");
}

// *************************************************************************************************
// Log Value

// Logs the variable and the name of the variable
#define LOG(var) Log(var, TEXT(#var))

// Logs a TEXT()
MODULE_API void Log(const TCHAR* msg);

// Various templated Log functions that accept an object and attempt to display its value.
template<typename T> requires std::is_signed<T>::value
constexpr void Log(T t, const TCHAR* msg = TEXT("")) {
	UE_LOG(LogTemp, Warning, TEXT("%d %s"), t, msg);
}

template<typename T> requires std::is_unsigned<T>::value
constexpr void Log(T t, const TCHAR* msg = TEXT("")) {
	UE_LOG(LogTemp, Warning, TEXT("%u %s"), t, msg);
}

template<typename T> requires std::is_floating_point<T>::value
constexpr void Log(T t, const TCHAR* msg = TEXT("")) {
	UE_LOG(LogTemp, Warning, TEXT("%f %s"), t, msg);
}

template<typename T> requires HasToStringMemberFunction<T>
constexpr void Log(T t, const TCHAR* msg = TEXT("")) {
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *t.ToString(), msg);
}

// *************************************************************************************************
// Log Binary

// Logs the binary of the variable and it's name
#define LOG_BINARY(var) LogBinary(var, TEXT(#var))

// Used by ToBinaryString() to add '_' dividers to long binaries to improve visibility.
enum class BinaryStringDivider
{
	None,
	Int8,
	Float,
	Double
};

// Converts an input integer into its binary representation
template <typename T> requires std::is_integral_v<T>
constexpr FString ToBinaryString(T t, BinaryStringDivider divider = BinaryStringDivider::None) {
	FString binaryString;
	const int32 numBits = sizeof(T) * 8;

	for (int32 i = numBits - 1; i >= 0; --i) {
		bool isBitSet = (t & (T(1) << i)) != 0;
		binaryString += isBitSet ? TEXT("1") : TEXT("0");

		if (divider == BinaryStringDivider::Int8) {
			if (i % 8 == 0 && i != 0 && i != numBits - 1) {
				binaryString += TEXT("_");
			}
		}
		else if (divider == BinaryStringDivider::Float) {
			if (i == 31 || i == 23) {
				binaryString += TEXT("_");
			}
		}
		else if (divider == BinaryStringDivider::Double) {
			if (i == 63 || i == 52) {
				binaryString += TEXT("_");
			}
		}
	}

	return binaryString;
}

// Various templated LogBinary functions that accept an object and attempt to display its binary
template<typename T> requires std::is_integral_v<T>
constexpr void LogBinary(T t, const TCHAR* msg = TEXT(""), 
	BinaryStringDivider divider = BinaryStringDivider::Int8) {
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *ToBinaryString(t, divider), msg);
}

MODULE_API constexpr void LogBinary(float t, const TCHAR* msg = TEXT(""),
	BinaryStringDivider divider = BinaryStringDivider::Float) {
	LogBinary(
		*reinterpret_cast<const FloatSelector<32>::int_type*>(&t), 
		msg, divider
	);
}


MODULE_API constexpr void LogBinary(double t, const TCHAR* msg = TEXT(""),
	BinaryStringDivider divider = BinaryStringDivider::Double) {
	LogBinary(
		*reinterpret_cast<const FloatSelector<64>::int_type*>(&t), 
		msg, divider
	);
}

template<typename T> requires HasToBinaryStringMemberFunction<T>
constexpr void LogBinary(T t, const TCHAR* msg = TEXT(""), 
	BinaryStringDivider divider = BinaryStringDivider::Int8) {
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *t.ToBinaryString(), msg);
}

#undef MODULE_API