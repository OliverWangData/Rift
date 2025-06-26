// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DYNAMICWORLD_API DECLARE_LOG_CATEGORY_EXTERN(LogDynamicWorld, Log, All);

class FDynamicWorldModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};