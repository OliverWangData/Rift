// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DynamicMesh/DynamicMesh3.h"
#include "Mathematics/Indexing.h"

template <typename TVertexIndex>
struct TerrainData
{
	TArray<FVector3f> VertexPositions;
	TArray<FVector3f> VertexNormals;
	TArray<TVertexIndex> Triangles;
	TArray<TVertexIndex> LatticeToBufferIndices;
	TArray<TVertexIndex> BufferToLatticeIndices;
};

template <typename TVertexIndex, int VoxelsX, int VoxelsY, int VoxelsZ>
UE::Geometry::FDynamicMesh3 CreateDynamicMesh3(
	TerrainData<TVertexIndex> terrain,
	TerrainData<TVertexIndex> parentTerrain,
	int octantIndex
) {

	UE::Geometry::FDynamicMesh3 dynamicMesh = UE::Geometry::FDynamicMesh3(true, true, true, false);

	for (int v = 0; v < terrain.VertexPositions.num(); ++v) {

		// Convert terrain voxel's index into parent voxel's index
		TVertexIndex i = terrain.BufferToLatticeIndices[v];
		TVertexIndex x, y, z;
		Unravel(i, VoxelsX, VoxelsY, x, y, z);
		TVertexIndex pi = Flatten(
			x / 2 + (octantIndex % 2 != 0 ? VoxelsX / 2 : 0),
			y / 2 + (octantIndex % 4 >= 2 ? VoxelsY / 2 : 0),
			z / 2 + (octantIndex >= 4 ? VoxelsZ / 2 : 0),
			VoxelsX, VoxelsY);
		TVertexIndex bpi = parentTerrain.LatticeToBufferIndices[pi];



	}
}