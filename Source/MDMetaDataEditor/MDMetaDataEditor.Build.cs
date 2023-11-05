// Copyright Dylan Dumesnil. All Rights Reserved.

using UnrealBuildTool;

public class MDMetaDataEditor : ModuleRules
{
	public MDMetaDataEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
		PCHUsage = PCHUsageMode.NoPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
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
				"StructUtils",
				"UMGEditor",
				"UnrealEd"
			}
		);
	}
}
