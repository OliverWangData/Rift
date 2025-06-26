// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkMeshComponent.h"
#include "Rendering/ChunkMeshComponentSceneProxy.h"

FPrimitiveSceneProxy* UChunkMeshComponent::CreateSceneProxy()
{
	if (!GetSceneData().SceneProxy)
	{
		return new FChunkMeshComponentSceneProxy(this);
	}
	else
	{
		return GetSceneData().SceneProxy;
	}
}