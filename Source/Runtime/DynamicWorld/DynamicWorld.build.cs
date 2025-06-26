// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DynamicWorld : ModuleRules
{
	public DynamicWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "GameCore",
            "Highway",
            "SIMDCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "GeometryCore"
        });

    }
}
