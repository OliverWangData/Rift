// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShaderCore : ModuleRules
{
	public ShaderCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
