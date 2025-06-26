// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RiftEditorTarget : TargetRules
{
	public RiftEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("Game");

        /*
        OptimizationLevel = OptimizationMode.Speed;
		bUseInlining = true;
		EnableOptimizeCodeForModules = new string[]
		{
			"SIMDCore",
			"Highway",
			"NoiseGraph",
			"NoiseGraphEditor"
		};*/

    }
}
