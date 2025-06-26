// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkMeshVertexFactory.h"
#include "ChunkMeshCore.h"
#include "MeshDrawShaderBindings.h"
#include <array>

// *************************************************************************************************
// UE vertex factory boilerplate macros

// Implements the type layout declared in FChunkMeshVertexFactoryShaderParameters
IMPLEMENT_TYPE_LAYOUT(FChunkMeshVertexFactoryShaderParameters);

// Binds the ShaderParameters class to the VertexFactory class. The Shader parameters specify
// how the vertex factory's data should be laid out for a specific shader stage. 
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(
	FChunkMeshVertexFactory, SF_Vertex, FChunkMeshVertexFactoryShaderParameters
);

// This macro links the Vertex Factory to its header shader, and handles a lot of boilerplate to
// integrate the vertex factory with UE's rendering pipeline. 
IMPLEMENT_VERTEX_FACTORY_TYPE(
	FChunkMeshVertexFactory,
	"Shaders/ChunkMeshVertexFactory.ush",
	EVertexFactoryFlags::UsedWithMaterials
	//| EVertexFactoryFlags::SupportsStaticLighting
	//| EVertexFactoryFlags::SupportsDynamicLighting
	//| EVertexFactoryFlags::SupportsPrecisePrevWorldPos
	| EVertexFactoryFlags::SupportsPositionOnly
	//| EVertexFactoryFlags::SupportsCachingMeshDrawCommands
	//| EVertexFactoryFlags::SupportsPrimitiveIdStream
	//| EVertexFactoryFlags::SupportsNaniteRendering
	//| EVertexFactoryFlags::SupportsRayTracing
	//| EVertexFactoryFlags::SupportsRayTracingDynamicGeometry
	//| EVertexFactoryFlags::SupportsRayTracingProceduralPrimitive
	//| EVertexFactoryFlags::SupportsLightmapBaking
	//| EVertexFactoryFlags::SupportsPSOPrecaching
	//| EVertexFactoryFlags::SupportsManualVertexFetch
	//| EVertexFactoryFlags::DoesNotSupportNullPixelShader
	//| EVertexFactoryFlags::SupportsGPUSkinPassThrough
	//| EVertexFactoryFlags::SupportsComputeShading
	//| EVertexFactoryFlags::SupportsLumenMeshCards
	//| EVertexFactoryFlags::SupportsLandscape
);

// *************************************************************************************************

TUniquePtr<FChunkMeshVertexFactory> FChunkMeshVertexFactory::CreateVertexFactory(float spacing, ERHIFeatureLevel::Type inFeatureLevel)
{
	return MakeUnique<FChunkMeshVertexFactory>(
		spacing, inFeatureLevel, FRenderThreadDeleter<FChunkMeshVertexFactory>()
	);
}

bool FChunkMeshVertexFactory::ShouldCompilePermutation(
	const FVertexFactoryShaderPermutationParameters& parameters
) 
{
	return true;
}

// Remark - The implementation in FLocalVertexFactory is static. But AccessStreamComponent with
// EVertexInputStreamType is non-static. FLocalVertexFactory implementation handles it by forcing
// the inputstreamtype to be default for this function, and uses the non-static method in InitRHI. 
// Instead, I just changed it to non-static to allow both static and nonstatic AccessStreamComponent
// calls. 
void FChunkMeshVertexFactory::GetVertexElements(
	EVertexInputStreamType inputStreamType, FChunkMeshVertexFactoryDataType& data,
	FVertexDeclarationElementList& elements, FVertexStreamList& inOutStreams
)
{
	auto addElementLambda = [&](
		EVertexInputStreamType streamType, FVertexStreamComponent& streamComponent, int index
		) {
			if (streamComponent.VertexBuffer != nullptr)
			{
				if (streamType == EVertexInputStreamType::Default)
				{
					elements.Add(AccessStreamComponent(streamComponent, index, inOutStreams));
				}
				else
				{
					elements.Add(AccessStreamComponent(streamComponent, index, streamType));
				}
			}
	};

	addElementLambda(inputStreamType, data.PackedComponent0, 0);

	if (inputStreamType != EVertexInputStreamType::PositionOnly)
	{
		addElementLambda(inputStreamType, data.PackedComponent1, 1);
	}

	// Normally we have another if (inputStreamType == Default) here and add vertex attributes that
	// aren't position or normal, but because of the current layout, the material attribute is 
	// attached to the position component1, so it will always be passed. 
}

void FChunkMeshVertexFactory::SetData(
	FRHICommandListBase& rHICmdList, const FChunkMeshVertexFactoryDataType& inData
)
{
	Data = inData;
	UpdateRHI(rHICmdList);
}

void FChunkMeshVertexFactory::Copy(const FChunkMeshVertexFactory& other)
{
	FChunkMeshVertexFactory* vertexFactory = this;
	const FChunkMeshVertexFactoryDataType* dataCopy = &other.Data;

	ENQUEUE_RENDER_COMMAND(FChunkMeshVertexFactoryCopyData)(
		[vertexFactory, dataCopy](FRHICommandListImmediate& RHICmdList)
		{
			vertexFactory->Data = *dataCopy;
		});
	BeginUpdateResourceRHI(this);
}

void FChunkMeshVertexFactory::InitRHI(FRHICommandListBase& rHICmdList)
{
	// Ensures the current hardware matches the requirements of the vertex factory. 
	check(HasValidFeatureLevel());

	// For default, positionOnly, and positionAndNormalOnly, create the vertex streams and 
	// generate the FVertexElements list. Use that list to create the vertex declarations.
	constexpr std::array<EVertexInputStreamType, 3> streamTypes = {
		EVertexInputStreamType::Default,
		EVertexInputStreamType::PositionOnly,
		EVertexInputStreamType::PositionAndNormalOnly
	};

	for (auto streamType : streamTypes)
	{
		FVertexDeclarationElementList elements;
		GetVertexElements(streamType, Data, elements, Streams);
		InitDeclaration(elements, streamType);
	}

	// Ensure the created declaration is valid
	check(IsValidRef(GetDeclaration()));
}

void FChunkMeshVertexFactory::ReleaseRHI()
{
	FVertexFactory::ReleaseRHI();
}

// *************************************************************************************************
// FChunkMeshVertexFactoryShaderParameters functions

void FChunkMeshVertexFactoryShaderParameters::Bind(const FShaderParameterMap& parameterMap)
{
	Spacing.Bind(parameterMap, TEXT("CMSpacing"), SPF_Mandatory);
}

void FChunkMeshVertexFactoryShaderParameters::GetElementShaderBindings(
	const FSceneInterface* scene, const FSceneView* view, const FMeshMaterialShader* shader,
	const EVertexInputStreamType inputStreamType, ERHIFeatureLevel::Type featureLevel, 
	const FVertexFactory* vertexFactory, const FMeshBatchElement& batchElement, 
	FMeshDrawSingleShaderBindings& shaderBindings, FVertexInputStreamArray& vertexStreams
) const
{
	const FChunkMeshVertexFactory* chunkMeshVertexFactory = 
		((FChunkMeshVertexFactory*)vertexFactory);

	shaderBindings.Add(Spacing, chunkMeshVertexFactory->Spacing);
}
