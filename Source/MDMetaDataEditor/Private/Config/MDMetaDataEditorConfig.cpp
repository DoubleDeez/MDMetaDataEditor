// Copyright Dylan Dumesnil. All Rights Reserved.


#include "MDMetaDataEditorConfig.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Widget.h"
#include "Engine/DataTable.h"
#include "Engine/UserDefinedStruct.h"
#include "GameplayTagContainer.h"
#include "MDMetaDataEditorModule.h"
#include "Modules/ModuleManager.h"
#include "WidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "MDMetaDataEditor"

UMDMetaDataEditorConfig::UMDMetaDataEditorConfig()
{
	// Setup some useful defaults, with some ugly code

	// Any property
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("EditCondition"), EMDMetaDataEditorKeyType::String, LOCTEXT("EditCondition_Description", "Enter a condition to determine whether or not this property can be edited. Supports Bools and Enums.") }.CanBeUsedByFunctions(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("EditConditionHides"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("EditConditionHides_Description", "If this property's EditCondition is false, it will be hidden.") }.CanBeUsedByFunctions(false).SetRequiredMetaData(TEXT("EditCondition")).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("DisplayAfter"), EMDMetaDataEditorKeyType::String, LOCTEXT("DisplayAfter_Description", "In the details panel, this property will be displayed after the property specified here.") }.CanBeUsedByFunctions(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("DisplayPriority"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("DisplayPriority_Description", "The priority to display this property in the deatils panel, lower values are first.") }.CanBeUsedByFunctions(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("NoResetToDefault"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("NoResetToDefault_Description", "If set, this property will never show the 'Reset to Default' arrow button.") }.CanBeUsedByFunctions(false).CanBeUsedOnFunctionParameters(false),
	});

	// Bool properties
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("InlineEditConditionToggle"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("InlineEditConditionToggle_Description", "If this bool is an EditCondition for another property, it will be displayed inline.") }.SetSupportedProperty({ UEdGraphSchema_K2::PC_Boolean}).CanBeUsedOnFunctionParameters(false)
	});

	// Any numeric property
	const TSet<FMDMetaDataEditorPropertyType> NumericTypes = {
		{ UEdGraphSchema_K2::PC_Int },
		{ UEdGraphSchema_K2::PC_Int64 },
		{ UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float },
		{ UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double },
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, LOCTEXT("NoSpinbox_Description", "Disables the click and drag functionality for setting the value of this property.") }.SetSupportedProperties(NumericTypes)
	});

	// Integers
	const TSet<FMDMetaDataEditorPropertyType> IntegerTypes = {
		{ UEdGraphSchema_K2::PC_Int },
		{ UEdGraphSchema_K2::PC_Int64 },
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("SliderExponent_Description", "How fast the value should change while dragging to set the value.") }.SetSupportedProperties(IntegerTypes).SetMinInt(1),
		FMDMetaDataKey{ TEXT("Delta"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("Delta_Description", "How much to change the value by when dragging.") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("Multiple_Description", "Forces the property value to be a multiple of this value.") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("ArrayClamp"), EMDMetaDataEditorKeyType::String, LOCTEXT("ArrayClamp_Description", "Clamps the valid values that can be entered in the UI to be between 0 and the length of the array specified."), TEXT("Value Range") }.SetSupportedProperties(IntegerTypes).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("ClampMin"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("ClampMin_Description", "Specifies the minimum value that may be entered for the property."), TEXT("Value Range") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("ClampMax"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("ClampMax_Description", "Specifies the maximum value that may be entered for the property."), TEXT("Value Range") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("UIMin"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("UIMin_Description", "Specifies the lowest that the value slider should represent."), TEXT("Value Range") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("UIMax"), EMDMetaDataEditorKeyType::Integer, LOCTEXT("UIMax_Description", "Specifies the highest that the value slider should represent."), TEXT("Value Range") }.SetSupportedProperties(IntegerTypes)
	});

	// Float types
	const TSet<FMDMetaDataEditorPropertyType> FloatTypes = {
		{ UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float },
		{ UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double },
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Float, LOCTEXT("SliderExponent_Description", "How fast the value should change while dragging to set the value.") }.SetSupportedProperties(FloatTypes).SetMinFloat(1.f),
		FMDMetaDataKey{ TEXT("Delta"), EMDMetaDataEditorKeyType::Float, LOCTEXT("Delta_Description", "How much to change the value by when dragging.") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Float, LOCTEXT("Multiple_Description", "Forces the property value to be a multiple of this value.") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("ClampMin"), EMDMetaDataEditorKeyType::Float, LOCTEXT("ClampMin_Description", "Specifies the minimum value that may be entered for the property."), TEXT("Value Range") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("ClampMax"), EMDMetaDataEditorKeyType::Float, LOCTEXT("ClampMax_Description", "Specifies the maximum value that may be entered for the property."), TEXT("Value Range") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("UIMin"), EMDMetaDataEditorKeyType::Float, LOCTEXT("UIMin_Description", "Specifies the lowest that the value slider should represent."), TEXT("Value Range") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("UIMax"), EMDMetaDataEditorKeyType::Float, LOCTEXT("UIMax_Description", "Specifies the highest that the value slider should represent."), TEXT("Value Range") }.SetSupportedProperties(FloatTypes)
	});

	// Non-localized strings
	const TSet<FMDMetaDataEditorPropertyType> NonLocStringTypes = {
		{ UEdGraphSchema_K2::PC_String },
		{ UEdGraphSchema_K2::PC_Name }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("GetOptions"), EMDMetaDataEditorKeyType::String, LOCTEXT("GetOptions_Description", "Specify a function that returns a list of Strings or Names that are valid values for this property. Seems to only support C++ functions since BP functions don't return anything for UFunction::GetReturnProperty().") }.SetSupportedProperties(NonLocStringTypes)
	});

	// Any Array
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("NoElementDuplicate"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("NoElementDuplicate_Description", "Indicates that the duplicate icon should not be shown for entries of this array in the property panel.") }.SetSupportedProperties({ FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Wildcard }.SetContainerType(EMDMetaDataPropertyContainerType::Array) })
	});

	// Any Map
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("ForceInlineRow"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("ForceInlineRow_Description", "Force the Key and Value of a TMap to display in the same row.") }.SetSupportedProperties({ FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Wildcard }.SetValueType({UEdGraphSchema_K2::PC_Wildcard}).SetContainerType(EMDMetaDataPropertyContainerType::Map) })
	});

	// Array of Any Struct
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("TitleProperty"), EMDMetaDataEditorKeyType::String, LOCTEXT("TitleProperty_Description", "Specify a child property or FText style format of child properties to use as the summary.") }.SetSupportedProperties({ FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Struct }.SetContainerType(EMDMetaDataPropertyContainerType::Array) })
	});

	// Any Struct
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("ShowOnlyInnerProperties"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("ShowOnlyInnerProperties_Description", "Removes the struct layer in the details panel, directly displaying the child properties of the struct.") }.SetSupportedProperty({ UEdGraphSchema_K2::PC_Struct }).CanBeUsedOnFunctionParameters(false)
	});

	// Any Enum
	const TSet<FMDMetaDataEditorPropertyType> EnumTypes = {
		{ UEdGraphSchema_K2::PC_Enum, NAME_None, UEnum::StaticClass() },
		{ UEdGraphSchema_K2::PC_Byte, NAME_None, UEnum::StaticClass() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("ValidEnumValues"), EMDMetaDataEditorKeyType::String, LOCTEXT("ValidEnumValues_Description", "Restricts selection to a subset of the enum's values.") }.SetSupportedProperties(EnumTypes),
		FMDMetaDataKey{ TEXT("InvalidEnumValues"), EMDMetaDataEditorKeyType::String, LOCTEXT("InvalidEnumValues_Description", "Prevents selecting a subset of the enum's values.") }.SetSupportedProperties(EnumTypes)
	});

	// Gameplay Tags and Containers
	const TSet<FMDMetaDataEditorPropertyType> GameplayTagTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, FGameplayTag::StaticStruct() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, FGameplayTagContainer::StaticStruct() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("Categories"), EMDMetaDataEditorKeyType::GameplayTagContainer, LOCTEXT("Categories_Description", "Limit which gameplay tags may be selected to one or more specific root tags.") }.SetSupportedProperties(GameplayTagTypes).SetDisplayNameOverride(LOCTEXT("Categories_DisplayName","Tag Filter"))
	});

	// Primary Asset IDs
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("AllowedTypes"), EMDMetaDataEditorKeyType::String, LOCTEXT("AllowedTypes_Description", "Limit which Primary Data Assets may be selected to one or more specific Primary Asset Types.") }.SetSupportedProperty({ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FPrimaryAssetId>::Get() })
	});

	// Data Tables
	const TSet<FMDMetaDataEditorPropertyType> DataTableTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, FDataTableRowHandle::StaticStruct() },
		{ UEdGraphSchema_K2::PC_Object, NAME_None, UDataTable::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UDataTable::StaticClass() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("RowType"), EMDMetaDataEditorKeyType::String, LOCTEXT("RowType_Description", "Limit the selection to a specific data table row struct type.") }.SetSupportedProperties(DataTableTypes),
		FMDMetaDataKey{ TEXT("RequiredAssetDataTags"), EMDMetaDataEditorKeyType::String, LOCTEXT("RequiredAssetDataTags_Description", "Limit the selection to data tables with matching asset data tags.") }.SetSupportedProperties(DataTableTypes)
	});

	// Colors
	const TSet<FMDMetaDataEditorPropertyType> ColorTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FLinearColor>::Get() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FColor>::Get() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("HideAlphaChannel"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("HideAlphaChannel_Description", "Hide the alpha channel from the color picker.") }.SetSupportedProperties(ColorTypes)
	});

	// Any UObject type
	const TSet<FMDMetaDataEditorPropertyType> ObjectTypes = {
		{ UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FSoftObjectPath>::Get() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FSoftClassPath>::Get() }
	};
	MetaDataKeys.Append({
		// None yet
	});

	// Any soft reference to a UObject type
	const TSet<FMDMetaDataEditorPropertyType> SoftObjectTypes = {
		{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FSoftObjectPath>::Get() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FSoftClassPath>::Get() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("AssetBundles"), EMDMetaDataEditorKeyType::String, LOCTEXT("AssetBundles_Description", "The name of the bundle to store the secondary asset in.") }.SetSupportedProperties(SoftObjectTypes),
		FMDMetaDataKey{ TEXT("Untracked"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("Untracked_Description", "Specify that the soft reference should not be tracked and therefore not automatically cooked or checked during delete or redirector fixup.") }.SetSupportedProperties(SoftObjectTypes)
	});

	// Any UClass type
	const TSet<FMDMetaDataEditorPropertyType> ClassTypes = {
		{ UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FSoftClassPath>::Get() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("AllowAbstract"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("AllowAbstract_Description", "Include abstract classes in the class picker for this property.") }.SetSupportedProperties(ClassTypes),
		FMDMetaDataKey{ TEXT("ShowTreeView"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("ShowTreeView_Description", "Show a tree of class inheritence instead of a list view for the class picker.") }.SetSupportedProperties(ClassTypes),
		FMDMetaDataKey{ TEXT("BlueprintBaseOnly"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("BlueprintBaseOnly_Description", "Only allow selecting blueprint classes.") }.SetSupportedProperties(ClassTypes),
		FMDMetaDataKey{ TEXT("ExactClass"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("ExactClass_Description", "Only allow selecting specifically from the list of allowed classes, no subclasses.") }.SetSupportedProperties(ClassTypes).SetRequiredMetaData(TEXT("AllowedClasses")),
		FMDMetaDataKey{ TEXT("MustImplement"), EMDMetaDataEditorKeyType::String, LOCTEXT("MustImplement_Description", "Only allow classes that inherit the specified interface.") }.SetSupportedProperties(ClassTypes)
	});

	// UMG Only
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("DesignerRebuild"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("DesignerRebuild_Description", "When this property changes, the widget preview will be rebuilt."), TEXT("UMG") }.SetSupportedBlueprint(UWidgetBlueprint::StaticClass()).CanBeUsedOnLocalVariables(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("BindWidget"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("BindWidget_Description", "This property requires a widget be bound to it in any child Widget Blueprints."), TEXT("UMG|Bind Widget") }.SetSupportedBlueprint(UWidgetBlueprint::StaticClass()).SetSupportedObjectProperty(UWidget::StaticClass()).SetIncompatibleMetaData(TEXT("BindWidgetOptional")).CanBeUsedOnLocalVariables(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("BindWidgetOptional"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("BindWidgetOptional_Description", "This property allows a widget be bound to it in any child Widget Blueprints."), TEXT("UMG|Bind Widget") }.SetSupportedBlueprint(UWidgetBlueprint::StaticClass()).SetSupportedObjectProperty(UWidget::StaticClass()).SetIncompatibleMetaData(TEXT("BindWidget")).CanBeUsedOnLocalVariables(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("BindWidgetAnim"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("BindWidgetAnim_Description", "This property requires a widget animation be bound to it in any child Widget Blueprints."), TEXT("UMG|Bind Widget Anim") }.SetSupportedBlueprint(UWidgetBlueprint::StaticClass()).SetSupportedObjectProperty(UWidgetAnimation::StaticClass()).SetIncompatibleMetaData(TEXT("BindWidgetAnimOptional")).CanBeUsedOnLocalVariables(false).CanBeUsedOnFunctionParameters(false),
		FMDMetaDataKey{ TEXT("BindWidgetAnimOptional"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("BindWidgetAnimOptional_Description", "This property allows a widget animation be bound to it in any child Widget Blueprints."), TEXT("UMG|Bind Widget Anim") }.SetSupportedBlueprint(UWidgetBlueprint::StaticClass()).SetSupportedObjectProperty(UWidgetAnimation::StaticClass()).SetIncompatibleMetaData(TEXT("BindWidgetAnim")).CanBeUsedOnLocalVariables(false).CanBeUsedOnFunctionParameters(false)
	});

	// Functions Only
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("DefaultToSelf"), EMDMetaDataEditorKeyType::String, LOCTEXT("DefaultToSelf_Description", "Specify which function parameter should default to \"self\".") }.SetFunctionsOnly()
	});

	// Objects and PrimaryAssetID
	TSet<FMDMetaDataEditorPropertyType> AssetTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FPrimaryAssetId>::Get() }
	};
	AssetTypes.Append(ObjectTypes);
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("DisplayThumbnail"), EMDMetaDataEditorKeyType::Boolean, LOCTEXT("DisplayThumbnail_Description", "Whether or not to display the asset thumbnail.") }.SetSupportedProperties(AssetTypes),
		FMDMetaDataKey{ TEXT("AllowedClasses"), EMDMetaDataEditorKeyType::String, LOCTEXT("AllowedClasses_Description", "Filter the selection to classes that inherit from specific classes or implement specific interfaces.") }.SetSupportedProperties(AssetTypes),
		FMDMetaDataKey{ TEXT("DisallowedClasses"), EMDMetaDataEditorKeyType::String, LOCTEXT("DisallowedClasses_Description", "Filter out classes that inherit from specific classes or implement specific interfaces from the selection.") }.SetSupportedProperties(AssetTypes)
	});

	// USTRUCT
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("HiddenByDefault"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("HiddenByDefault_Description", "Pins in Make and Break nodes are hidden by default.") }.SetStructsOnly(),
		FMDMetaDataKey{ TEXT("DisableSplitPin"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("DisableSplitPin_Description", "Indicates that node pins of this struct type cannot be split.") }.SetStructsOnly()
	});

	// Instanced Structs
	TSet<FMDMetaDataEditorPropertyType> InstancedStructs = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FInstancedStruct>::Get() }
	};
	AssetTypes.Append(ObjectTypes);
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("BaseStruct"), EMDMetaDataEditorKeyType::String, LOCTEXT("InstancedStruct_BaseStruct_Description", "The minimum allowable type holdable by this struct.") }.SetSupportedProperties(InstancedStructs),
		FMDMetaDataKey{ TEXT("ExcludeBaseStruct"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("InstancedStruct_ExcludeBaseStruct_Description", "Only allow subclasses of the BaseStruct type.") }.SetSupportedProperties(InstancedStructs).SetRequiredMetaData(TEXT("BaseStruct")),
		FMDMetaDataKey{ TEXT("AllowedClasses"), EMDMetaDataEditorKeyType::String, LOCTEXT("InstancedStruct_AllowedClasses_Description", "Inclusive list of allowed struct classes.") }.SetSupportedProperties(InstancedStructs).SetDisplayNameOverride(LOCTEXT("InstancedStruct_AllowedClasses_DisplayName","Allowed Struct Classes")),
		FMDMetaDataKey{ TEXT("DisallowedClasses"), EMDMetaDataEditorKeyType::String, LOCTEXT("InstancedStruct_DisallowedClasses_Description", "List of struct classes to hide from picker.") }.SetSupportedProperties(InstancedStructs).SetDisplayNameOverride(LOCTEXT("InstancedStruct_DisallowedClasses_DisplayName","Disallowed Struct Classes")),
		FMDMetaDataKey{ TEXT("ShowTreeView"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("InstancedStruct_ShowTreeView_Description", "Dispay the Struct Class picker as a tree view.") }.SetSupportedProperties(InstancedStructs),
		FMDMetaDataKey{ TEXT("StructTypeConst"), EMDMetaDataEditorKeyType::Flag, LOCTEXT("InstancedStruct_StructTypeConst_Description", "Struct class cannot be changed.") }.SetSupportedProperties(InstancedStructs).SetDisplayNameOverride(LOCTEXT("InstancedStruct_StructTypeConst_DisplayName","Struct Type is Constant")),
	});

	// Sort pre-defined keys
	MetaDataKeys.Sort([](const FMDMetaDataKey& A, const FMDMetaDataKey& B)
	{
		return A.Key.Compare(B.Key) < 0;
	});
}

void UMDMetaDataEditorConfig::PostInitProperties()
{
	Super::PostInitProperties();

	if (bSortMetaDataAlphabetically)
	{
		// TODO - This also needs to happen when saving, but we don't want to rearrange the list while it's being edited so we'd need to know when the user leaves the Project settings screen or something
		MetaDataKeys.Sort([](const FMDMetaDataKey& A, const FMDMetaDataKey& B)
		{
			return A.Key.Compare(B.Key) < 0;
		});
	}

	// Fix Supported Property Types which may be coming from old configs.
	for (FMDMetaDataKey& Key : MetaDataKeys)
	{
		for (FMDMetaDataEditorPropertyType& SupportedPropertyType : Key.SupportedPropertyTypes)
		{
			SupportedPropertyType.FixUp();
		}
	}
}

FText UMDMetaDataEditorConfig::GetSectionText() const
{
	return INVTEXT("Meta Data Editor");
}

void UMDMetaDataEditorConfig::ForEachVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	ForEachPropertyMetaDataKey(Blueprint, Property, [&Func](const FMDMetaDataKey& Key)
	{
		if (Key.bCanBeUsedOnVariables)
		{
			Func(Key);
		}
	});
}

void UMDMetaDataEditorConfig::ForEachLocalVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	ForEachPropertyMetaDataKey(Blueprint, Property, [&Func](const FMDMetaDataKey& Key)
	{
		if (Key.bCanBeUsedOnLocalVariables)
		{
			Func(Key);
		}
	});
}

void UMDMetaDataEditorConfig::ForEachParameterMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	ForEachPropertyMetaDataKey(Blueprint, Property, [&Func](const FMDMetaDataKey& Key)
	{
		if (Key.bCanBeUsedOnFunctionParameters)
		{
			Func(Key);
		}
	});
}

void UMDMetaDataEditorConfig::ForEachPropertyMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	if (!IsValid(Blueprint) || Property == nullptr)
	{
		return;
	}

	for (const FMDMetaDataKey& Key : MetaDataKeys)
	{
		if (!Key.DoesSupportBlueprint(Blueprint))
		{
			continue;
		}

		if (Key.DoesSupportProperty(Property))
		{
			Func(Key);
		}
	}
}

void UMDMetaDataEditorConfig::ForEachStructPropertyMetaDataKey(const FProperty* Property, const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	// Test as if the struct were a basic blueprint
	ForEachPropertyMetaDataKey(UBlueprint::StaticClass()->GetDefaultObject<UBlueprint>(), Property, Func);
}

void UMDMetaDataEditorConfig::ForEachFunctionMetaDataKey(const UBlueprint* Blueprint, const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	if (!IsValid(Blueprint))
	{
		return;
	}

	for (const FMDMetaDataKey& Key : MetaDataKeys)
	{
		if (!Key.bCanBeUsedByFunctions || !Key.DoesSupportBlueprint(Blueprint))
		{
			continue;
		}

		Func(Key);
	}
}

void UMDMetaDataEditorConfig::ForEachStructMetaDataKey(const TFunctionRef<void(const FMDMetaDataKey&)>& Func) const
{
	for (const FMDMetaDataKey& Key : MetaDataKeys)
	{
		if (!Key.bCanBeUsedByStructs)
		{
			continue;
		}

		Func(Key);
	}
}

#if WITH_EDITOR
void UMDMetaDataEditorConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (FMDMetaDataEditorModule* Module = FModuleManager::GetModulePtr<FMDMetaDataEditorModule>(TEXT("MDMetaDataEditor")))
	{
		Module->RestartModule();
	}
}
#endif //WITH_EDITOR

TArray<FName> UMDMetaDataEditorConfig::GetMetaDataKeyNames() const
{
	TArray<FName> Result;
	Result.Add(NAME_None);

	Algo::Transform(MetaDataKeys, Result, [](const FMDMetaDataKey& Key)
	{
		return Key.Key;
	});

	return Result;
}

#undef LOCTEXT_NAMESPACE
