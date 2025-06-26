// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkMeshComponentSceneProxy.h"
#include "ChunkMeshCore.h"

FChunkMeshComponentSceneProxy::FChunkMeshComponentSceneProxy(UChunkMeshComponent* component)
	: FPrimitiveSceneProxy(component), 
	MaterialRelevance(component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	VertexFactory = FChunkMeshVertexFactory::CreateVertexFactory(0, GetScene().GetFeatureLevel());


}

size_t FChunkMeshComponentSceneProxy::GetTypeHash() const
{
	// Implementation from Realtime Mesh Component
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

uint32 FChunkMeshComponentSceneProxy::GetMemoryFootprint(void) const
{
	// Implementation from Realtime Mesh Component
	return (sizeof(*this) + GetAllocatedSize());
}
