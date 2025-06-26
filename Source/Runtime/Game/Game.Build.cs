// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Game : ModuleRules
{
	public Game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject",
			"Engine", 
			"InputCore",
			"EnhancedInput" ,
			"GameCore"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"NoiseGraph",
			"Highway",
			"SIMDCore"
		});
	}
}
