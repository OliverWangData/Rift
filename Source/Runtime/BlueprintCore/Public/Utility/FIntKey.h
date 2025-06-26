// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FIntKey.generated.h"

USTRUCT(BlueprintType)
struct BLUEPRINTCORE_API FIntKey
{
	GENERATED_BODY()

private:
	int32 Value;
public:
	FIntKey() : Value(0) {}

	explicit FIntKey(int32 value) : Value(value) {}

	int32 GetValue() const { return Value; }

	// Comparison and hash operators for use in TMap
	bool operator==(const FIntKey& Other) const { return Value == Other.Value; }
	bool operator!=(const FIntKey& Other) const { return Value != Other.Value; }
	size_t operator()(const FIntKey& Key) const { return std::hash<int32>()(Key.GetValue()); }
};
