// Copyright Epic Games, Inc. All Rights Reserved.

using System.Security.Permissions;
using UnrealBuildTool;

public class NoiseGraphEditor : ModuleRules
{
	public NoiseGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "NoiseGraph"
            });

            PrivateDependencyModuleNames.AddRange(new string[] {
                "Highway",

                // Render to dynamic texture action
                "RHI",
                "RenderCore",
                "Blutility"
            });
        }
    }
}
