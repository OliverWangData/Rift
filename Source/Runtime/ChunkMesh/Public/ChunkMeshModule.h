// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

CHUNKMESH_API DECLARE_LOG_CATEGORY_EXTERN(LogChunkMesh, Log, All);

class FChunkMeshModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};