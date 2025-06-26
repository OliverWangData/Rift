// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/Texture2DDynamic.h"
#include "NoiseGraph.h"

#include "CoreMinimal.h"
#include "EditorUtilityLibrary.h"
#include "AsyncActionNoiseToTex2DDynamic.generated.h"

UCLASS()
class NOISEGRAPHEDITOR_API UAsyncActionNoiseToTex2DDynamic : 
	public UEditorUtilityBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Make sure these two are compatible. 
	using TSamples = uint8_t;
	static const EPixelFormat PixelFormat = PF_B8G8R8A8;

	// ****************************************************************************************************
	// UE API
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncActionNoiseToTex2DDynamic* NoiseToTex2DDynamic(
		UNoiseGraph* noiseGraph, UTexture2DDynamic* texture,
		float offsetXOverride, float offsetYOverride, float offsetZOverride,
		float scaleOverride, bool use3D, bool rebuildNoiseGraph
	);

	virtual void Activate() override;

	// ****************************************************************************************************
	// Functions
private:
	void QueueGPUWriteSamplesToTexture();

	void CompletedTask();
	void FailedTask();

	// ****************************************************************************************************
	// Events
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCompleted);
	UPROPERTY(BlueprintAssignable)
	FOnCompleted OnCompleted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFailed);
	UPROPERTY(BlueprintAssignable)
	FOnFailed OnFailed;

	// ****************************************************************************************************
	// Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UNoiseGraph* NoiseGraph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UTexture2DDynamic* Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float OffsetXOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float OffsetYOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float OffsetZOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float ScaleOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool Use3D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool RebuildNoiseGraph;

	AlignedArray<TSamples> Samples;
	FGPUFenceRHIRef Fence;
};
