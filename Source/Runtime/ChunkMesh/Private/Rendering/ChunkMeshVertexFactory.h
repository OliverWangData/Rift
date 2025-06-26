// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/RenderCore/Public/VertexFactory.h"
#include <Runtime/RHI/Public/RHIFwd.h>

// *************************************************************************************************
// Contains the data needed by the Vertex Factory to initialize its RHI resources. 
// In particular, StreamComponents are used to create the vertex declaration and vertex stream
// SRVs provides a way to access the stream component's buffers inside the shader
struct CHUNKMESH_API FChunkMeshVertexFactoryDataType
{
	// Current layout

	//  x  y  z  px py pz  mat   np    ny    pnp   pny
	// |--|--|--|--|--|--|-----|-----|-----|-----|-----|
	// |-----------|-----------|-----------|-----------|
	//	   32bit       32bit       32bit       32bit
	// 
	// Where:
	//	- x, y, z are 8 bit axis position values
	//	- px, py, pz are the parent axis position values
	//	- mat is the materialID of the vertex/voxel
	//	- np, ny is the 16 bit pitch and yaw of the normal
	//	- pnp, pny is the parent pitch and yaw

	// Note - 2x int2 instead of 4x int because some functions, like GetPositionsSRV, expect a
	// single SRV for all position data (spanning 2x 32 bit ints). There also isn't a situation
	// where we want only the vertex position and not the parent anyways. 
	FVertexStreamComponent PackedComponent0;
	FVertexStreamComponent PackedComponent1;
	FRHIShaderResourceView* PackedComponent0SRV = nullptr;
	FRHIShaderResourceView* PackedComponent1SRV = nullptr;
};

// *************************************************************************************************
/// <summary>
/// The FChunkMeshVertexFactory is responsible for the layout of the vertex attributes, and for
/// defining how vertices are passed from CPU to the GPU, where they'll be used in the vertex 
/// shader. It takes mesh data and creates the required FRHIResources, and on render time, the 
/// renderer uses the vertex factory to access the FRHIResources. 
/// 
/// Primary responsibilities are:
///		- Create vertex buffers and bind them. 
///		- Create input layout and bind it. 
///		- Create vertex shader and bind it. 
/// </summary>
class CHUNKMESH_API FChunkMeshVertexFactory : public FVertexFactory
{
	// UE boilerplate for vertex factory
	DECLARE_VERTEX_FACTORY_TYPE(FChunkMeshVertexFactory);

protected:
	// protected as this isn't a uclass and doesn't benefit from UE garbage collection. 
	// Use CreateVertexFactory() to get an instance. 
	FChunkMeshVertexFactory(float spacing, ERHIFeatureLevel::Type inFeatureLevel)
		: FVertexFactory(inFeatureLevel), Spacing(spacing)
	{ }

public:
	// Creates an instance, with a deleter that guarantees this vertex factory is only deleted
	// on the rendering thread. 
	static TUniquePtr<FChunkMeshVertexFactory> CreateVertexFactory(
		float spacing, ERHIFeatureLevel::Type inFeatureLevel
	);

	// Specify which permutations we want the vertex factory to be compiled in.
	// E.g. for a vertex factory that only uses unlit materials, we can limit compilation to 
	// only materials with an unlit shading model and the material domain is surface. 
	// Required by IMPLEMENT_VERTEX_FACTORY_TYPE
	static bool ShouldCompilePermutation(
		const FVertexFactoryShaderPermutationParameters& params
	);

	static void GetPSOPrecacheVertexFetchElements(
		EVertexInputStreamType vertexInputStreamType, FVertexDeclarationElementList& elements
	);

	// Creates a list of FVertexElements. This list is used to create the vertex declaration, which
	// is the input layout (That defines what vertex attributes there are). 
	void GetVertexElements(
		EVertexInputStreamType inputStreamType, FChunkMeshVertexFactoryDataType& data,
		FVertexDeclarationElementList& elements, FVertexStreamList& inOutStreams
	);

	// Implementation of interface used by TSynchronizedResource to update the resource with new
	// data from the game thread
	void SetData(FRHICommandListBase& rHICmdList, const FChunkMeshVertexFactoryDataType& inData);

	// Copy data from another vertex factory
	void Copy(const FChunkMeshVertexFactory& other);

	//////////// [FRenderResource] Interface Begin
	// Init the RHI resources used by the vertex factory (mainly vertex streams and declarations)
	virtual void InitRHI(FRHICommandListBase& rHICmdList) override;
	// SafeRelease uniform buffers
	virtual void ReleaseRHI() override;
	//////////// [FRenderResource] Interface End

	// Shader Parameters
	float Spacing;

protected:
	FChunkMeshVertexFactoryDataType Data;
};

// *************************************************************************************************
/// <summary>
/// The ChunkMeshVertexFactory's shader parameters. 
/// Shader parameters are bound here. 
/// </summary>
class CHUNKMESH_API FChunkMeshVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FChunkMeshVertexFactoryShaderParameters, NonVirtual);

public:
	// Binds the fields declared in LAYOUT_FIELD with actual shader parameters
	void Bind(const class FShaderParameterMap& parameterMap);
	
	// Pass values to the shader parameters
	void GetElementShaderBindings(
		const FSceneInterface* scene, const FSceneView* view, const FMeshMaterialShader* shader,
		const EVertexInputStreamType inputStreamType, ERHIFeatureLevel::Type featureLevel,
		const FVertexFactory* vertexFactory, const FMeshBatchElement& batchElement,
		FMeshDrawSingleShaderBindings& shaderBindings, FVertexInputStreamArray& vertexStreams
	) const;

	// Declaring shader parameter fields
	LAYOUT_FIELD(FShaderParameter, Spacing);

	// LAYOUT_FIELD(FShaderResourceParameter, MyBufferSRV);
};