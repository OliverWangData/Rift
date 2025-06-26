// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChunkMeshComponent.h"
#include "Rendering/ChunkMeshVertexFactory.h"

/// <summary>
/// The SceneProxy is responsible for mirroring the game thread's mesh component (In particular, 
/// its UPrimitiveComponent) into the render thread. 
/// </summary>
class CHUNKMESH_API FChunkMeshComponentSceneProxy : public FPrimitiveSceneProxy
{
public:
	FChunkMeshComponentSceneProxy(UChunkMeshComponent* component);

	// --- FPrimitiveSceneProxy Interface ---
	virtual size_t GetTypeHash() const override;
	virtual uint32 GetMemoryFootprint(void) const override;
	// --- End FPrimitiveSceneProxy Interface ---

protected:
	TUniquePtr<FChunkMeshVertexFactory> VertexFactory;
	FMaterialRelevance MaterialRelevance;

};