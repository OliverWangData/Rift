// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChunkMeshComponent.generated.h"

/// <summary>
/// The UChunkMeshComponent stores a reference to a UChunkMesh, and is responsible for rendering
/// that mesh, applying materials, and handling transformations of the mesh in the world. 
/// </summary>
UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent))
class CHUNKMESH_API UChunkMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:

	//////////// [USceneComponent] Interface Begin
	//virtual FBoxSphereBounds CalcBounds(const FTransform& localToWorld) const override;
	//////////// [USceneComponent] Interface End
	

	//////////// [UPrimitiveComponent] Interface Begin
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//////////// [UPrimitiveComponent] Interface End


	//////////// [UMeshComponent] Interface Begin
	
	//////////// [UMeshComponent] Interface End
};