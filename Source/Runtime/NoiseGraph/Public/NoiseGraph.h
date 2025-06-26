// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NoiseGraphModule.h"
#include "Numerics/FixedPoint.h"
#include "Nodes/NodeBase.h"
#include "AlignedArray.h"
#include "TypeTraits/VariantTypeTraits.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NoiseGraph.generated.h"

#define NOISEGRAPH_FP_PARAMS 32, 16

USTRUCT(BlueprintType)
struct NOISEGRAPH_API FNoiseKey
{
	GENERATED_BODY()
public:
	using Sampler = std::shared_ptr<NodeBase<NOISEGRAPH_FP_PARAMS>>;

	FNoiseKey() : ptr(nullptr) {}
	FNoiseKey(Sampler sampler) : ptr(sampler) {}

	const Sampler Get() const {
		return ptr;
	}

protected:
	Sampler ptr;

};

UCLASS(Blueprintable)
class NOISEGRAPH_API UNoiseGraph : public UObject
{
	GENERATED_BODY()
public:
	using Fp = FixedPoint<NOISEGRAPH_FP_PARAMS>;
	using SamplingParameters = NoiseSamplingParameters< NOISEGRAPH_FP_PARAMS>;
	using Base = NodeBase<NOISEGRAPH_FP_PARAMS>;
	using Sampler = std::shared_ptr<Base>;

	template <typename T>
	using AlignedArray = AlignedArray<T>;

	// *********************************************************************************************
	// Constructor
	UNoiseGraph() = default;

	// *********************************************************************************************
	// Building API
	// 
	// How this works:
	// Build() logic is implemented in blueprints and will set the FNoiseKey Output. 
	// Call it again if the blueprint changes to reflect the change. 
	UFUNCTION(BlueprintImplementableEvent)
	void Build();

	UPROPERTY(BlueprintReadWrite)
	FNoiseKey Output;

	// *********************************************************************************************
	// Allocations and Sampling API
	template <typename T> requires exists_in_variant_v<T, Base::VarPtr, true>
	static AlignedArray<T> Allocate(SamplingParameters params) {
		return AlignedArray<T>(params.TotalSize());
	}

	template <typename T>  requires exists_in_variant_v<T, Base::VarPtr, true>
	void Sample(SamplingParameters params, AlignedArray<T>& array) {

		// If not built yet, builds the sampler. 
		if (!Output.Get()) {
			Build();

			if (!Output.Get()) {
				UE_LOG(LogNoiseGraph, Warning, TEXT("Could not build NoiseGraph."));
				return;
			}
		}

		Sampler sampler = Output.Get();
		sampler->PreProcess(params.GetBounds());
		sampler->Process(params, array);
		sampler->PostProcess();
	}

	// *********************************************************************************************
	// Nodes
	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName="Key") FNoiseKey GetRandom(float seed = 0);

	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetPerlin(float seed = 0);

	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetCellular(
		int feature = 0, float seed = 0, int maxPointsPerGrid = 1
	);
	
	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetFractal(
		FNoiseKey baseKey, int octaves = 4, float persistance = 0.5, float lacunarity = 2
	);

	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetWarp(
		FNoiseKey baseKey, FNoiseKey shiftKey, int layers = 1, float strength = 1
	);

	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetTree(
		FNoiseKey baseKey, float seed = 0, int depth = 0, float regularity = 0
	);

	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetHeightmap(
		FNoiseKey baseKey, float upperBound = 100, float lowerBound = 0
	);

	// Modifiers

	UFUNCTION(BlueprintPure)
	static UPARAM(DisplayName = "Key") FNoiseKey GetInvert(FNoiseKey baseKey);


	// *********************************************************************************************
	// Events
	// Broadcast when anything in the blueprint class changes. 
	// Called in the PostEditChangeProperty function.

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNoiseGraphChanged);
	UPROPERTY(BlueprintAssignable)
	FOnNoiseGraphChanged OnNoiseGraphChanged;
};
