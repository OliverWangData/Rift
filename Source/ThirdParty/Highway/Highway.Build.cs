// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.IO;

public class Highway : ModuleRules
{
	public Highway(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		string libraryName = "Highway";
        string projectDirectory = Path.GetDirectoryName(Target.ProjectFile.FullName);
        string libraryDirectory = Path.Combine(
            projectDirectory, "Libraries", "ThirdParty", libraryName
            );

        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Include"));
		PublicAdditionalLibraries.Add(Path.Combine(libraryDirectory, "hwy.lib"));
		//PublicAdditionalLibraries.Add(Path.Combine(libraryDirectory, "hwy_contrib.lib"));
		//PublicAdditionalLibraries.Add(Path.Combine(libraryDirectory, "hwy_test.lib"));

    }
}
