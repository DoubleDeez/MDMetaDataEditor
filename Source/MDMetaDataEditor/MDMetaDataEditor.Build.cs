// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDMetaDataEditor : ModuleRules
{
	public MDMetaDataEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"EditorFramework",
				"Engine",
				"GameplayTags",
				"GameplayTagsEditor",
				"InputCore",
				"KismetWidgets",
				"Slate",
				"SlateCore",
				"UMG",
				"UMGEditor",
				"UnrealEd"
			}
		);

        // Use StructUtils 5.0 - 5.4
        if (Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion <= 4)
        {
            PrivateDependencyModuleNames.Add("StructUtils");
        }
    }
}
