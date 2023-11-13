// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDMetaDataEditorGraph : ModuleRules
{
    public MDMetaDataEditorGraph(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
	            "BlueprintGraph",
	            "Core",
                "CoreUObject",
                "Engine",
                "Kismet",
                "Slate",
                "SlateCore",
                "UnrealEd"
            }
        );
    }
}