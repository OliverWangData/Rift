// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

NOISEGRAPHEDITOR_API DECLARE_LOG_CATEGORY_EXTERN(LogNoiseGraphEditor, Log, All);

class FNoiseGraphEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};