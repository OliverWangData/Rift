// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SIMDCore : ModuleRules
{
	public SIMDCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"GameCore", 
			"Highway" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
