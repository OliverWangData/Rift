// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AsyncActionNoiseToTex2DDynamic.h"
#include "NoiseGraphEditorModule.h"
#include "Mathematics/Indexing.h"

#include "hwy/highway.h"

#include <atomic>
#include <functional>

#include "Diagnostics/DebugLog.h"

UAsyncActionNoiseToTex2DDynamic* UAsyncActionNoiseToTex2DDynamic::NoiseToTex2DDynamic(
	UNoiseGraph* noiseGraph, UTexture2DDynamic* texture,
	float offsetXOverride, float offsetYOverride, float offsetZOverride,
	float scaleOverride, bool use3D, bool rebuildNoiseGraph
) {
	UAsyncActionNoiseToTex2DDynamic* node = NewObject<UAsyncActionNoiseToTex2DDynamic>();
	node->NoiseGraph = noiseGraph;
	node->Texture = texture;
	node->OffsetXOverride = offsetXOverride;
	node->OffsetYOverride = offsetYOverride;
	node->OffsetZOverride = offsetZOverride;
	node->ScaleOverride = scaleOverride;
	node->Use3D = use3D;
	node->RebuildNoiseGraph = rebuildNoiseGraph;
	return node;
}

void UAsyncActionNoiseToTex2DDynamic::Activate() {
	// Validations
	if (!NoiseGraph) {
		UE_LOG(LogNoiseGraphEditor, Warning, 
			TEXT("Cannot activate UAsyncActionNoiseToTex2DDynamic without a valid NoiseGraph.")
		);
		FailedTask();
		return;
	}

	if (!Texture) {
		UE_LOG(LogNoiseGraphEditor, Warning,
			TEXT("Cannot activate UAsyncActionNoiseToTex2DDynamic without a texture asset.")
		);
		FailedTask();
		return;
	}

	if (UNoiseGraph::Fp(ScaleOverride) == 0) {
		FailedTask();
		return;
	}

	UNoiseGraph::SamplingParameters params;
	params.Spacing = (1 / UNoiseGraph::Fp(ScaleOverride));
	params.Add(UNoiseGraph::Fp(OffsetXOverride), Texture->SizeX);
	params.Add(UNoiseGraph::Fp(OffsetYOverride), Texture->SizeY);

	if (Use3D) {
		params.Add(UNoiseGraph::Fp(OffsetZOverride), 1);
	}

	Samples = UNoiseGraph::Allocate<TSamples>(params);

	// Optimization - Avoids rebuilding the noise graph unless required due to changes. 
	// this is to prevent re-allocating many times, allows nodes to "reuse" their allocations. 
	if (RebuildNoiseGraph) {
		NoiseGraph->Build();
	}

	NoiseGraph->Sample<TSamples>(params, Samples);

	/*
	TSamples min = std::numeric_limits<TSamples>::max();
	TSamples max = std::numeric_limits<TSamples>::min();

	for (int i = 0; i < params.GetSize(); i++) {
		if (min > Samples[i]) {
			min = Samples[i];
		}

		if (max < Samples[i]) {
			max = Samples[i];
		}
	}
	UE_LOG(LogNoiseGraphEditor, Warning, TEXT("Max: %d   Min: %d"), max, min);
	*/

	QueueGPUWriteSamplesToTexture();
}

void UAsyncActionNoiseToTex2DDynamic::QueueGPUWriteSamplesToTexture() {
	// Used to ensure the gpu finishes the render command before completing the node
	Fence = RHICreateGPUFence(TEXT("GPUFence"));

	// Use ENQUEUE_RENDER_COMMAND to ensure RHI calls are made in the correct thread context
	ENQUEUE_RENDER_COMMAND(WriteToTextureCmd)(
		[this](
			FRHICommandListImmediate& RHICmdList
			) {

			// Lock the texture to access pixel data
			FTexture2DDynamicResource* textureResource = 
				static_cast<FTexture2DDynamicResource*>(Texture->GetResource());
			FTextureRHIRef textureRHI = textureResource->GetTextureRHI();

			// Ensures RHI is valid. Texture may not always be available. 
			// If its not valid and lock is attempted, could cause fatal errors. 
			if (!textureRHI.IsValid()) {
				UE_LOG(LogNoiseGraphEditor, Error, TEXT("Texture RHI is invalid."));
				FailedTask();
				return;
			}

			// Ensure runtime pixel format is correct
			// TODO: Replace this with a dynamic way to change the pixel format. 
			// Right now, this is hard coded at 4 channels. 
			if (textureRHI->GetFormat() != PixelFormat) {
				UE_LOG(LogNoiseGraphEditor, Warning, 
					TEXT("Pixel format is incorrect. Expecting: %s. Actual: %s"),
					GetPixelFormatString(PixelFormat),
					GetPixelFormatString(textureRHI->GetFormat())
				);

				FailedTask();
				return;
			}
			uint32 destStride;
			void* textureData = RHILockTexture2D(textureRHI, 0, RLM_WriteOnly, destStride, false);
			destStride = destStride / sizeof(TSamples);

			int sizeX = Texture->SizeX;
			int sizeY = Texture->SizeY;
			TSamples* dest = static_cast<TSamples*>(textureData);
			TSamples* source = Samples.GetPtr();

			for (int i = 0, y = 0; y < sizeY; y++) {
				for (int x = 0; x < sizeX; x++) {
					TSamples* finalDest = dest + (y * destStride) + (x * 4);
					finalDest[0] = source[i];
					finalDest[1] = source[i];
					finalDest[2] = source[i];
					finalDest[3] = std::numeric_limits<TSamples>::max();
					++i;
				}
			}

			// Unlock the texture
			RHIUnlockTexture2D(textureRHI, 0, false);

			// Tells the GPU to signal the fence that task is complete
			RHICmdList.WriteGPUFence(Fence);
		}
		);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[this]() {
			// Wait for fence to complete
			while (!Fence->Poll()) {}

			CompletedTask();
		}
	);
}

void UAsyncActionNoiseToTex2DDynamic::CompletedTask() {
	AsyncTask(ENamedThreads::GameThread, [this]() {

		// Note - This is not needed! For some reason adding this will jumble the texture.
		//Texture->UpdateResource();

		OnCompleted.Broadcast();
		SetReadyToDestroy();
		}
	);

}

void UAsyncActionNoiseToTex2DDynamic::FailedTask() {
	AsyncTask(ENamedThreads::GameThread, [this]() {
		OnFailed.Broadcast();
		SetReadyToDestroy();
		}
	);

}