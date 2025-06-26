// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

NOISEGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogNoiseGraph, Log, All);

class FNoiseGraphModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};