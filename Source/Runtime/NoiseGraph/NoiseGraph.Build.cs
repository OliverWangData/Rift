// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NoiseGraph : ModuleRules
{
	public NoiseGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "GameCore",
            "Highway",
            "SIMDCore",
            "BlueprintCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
        });

    }
}
