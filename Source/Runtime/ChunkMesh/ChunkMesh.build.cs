// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChunkMesh : ModuleRules
{
	public ChunkMesh(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "GameCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Engine",
            "RenderCore",
			"RHI"
        });

    }
}
