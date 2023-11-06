// Copyright Dylan Dumesnil. All Rights Reserved.


#include "MDMetaDataEditorConfig.h"

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "WidgetBlueprint.h"

UMDMetaDataEditorConfig::UMDMetaDataEditorConfig()
{
	// Setup some useful defaults, with some ugly code

	// Any property
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("EditCondition"), EMDMetaDataEditorKeyType::String, TEXT("Enter a condition to determine whether or not this property can be edited. Supports Bools and Enums.") }.CanBeUsedByFunctions(false),
		FMDMetaDataKey{ TEXT("EditConditionHides"), EMDMetaDataEditorKeyType::Flag, TEXT("If this property's EditCondition is false, it will be hidden.") }.CanBeUsedByFunctions(false),
		FMDMetaDataKey{ TEXT("DisplayAfter"), EMDMetaDataEditorKeyType::String, TEXT("In the details panel, this property will be displayed after the property specified here.") }.CanBeUsedByFunctions(false),
		FMDMetaDataKey{ TEXT("DisplayPriority"), EMDMetaDataEditorKeyType::Integer, TEXT("The priority to display this property in the deatils panel, lower values are first.") }.CanBeUsedByFunctions(false),
		FMDMetaDataKey{ TEXT("NoResetToDefault"), EMDMetaDataEditorKeyType::Flag, TEXT("If set, this property will never show the 'Reset to Default' arrow button.") }.CanBeUsedByFunctions(false),
	});

	// Bool properties
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("InlineEditConditionToggle"), EMDMetaDataEditorKeyType::Flag, TEXT("If this bool is an EditCondition for another property, it will be displayed inline.") }.SetSupportedProperty({ UEdGraphSchema_K2::PC_Boolean})
	});

	// Any numeric property
	const TSet<FMDMetaDataEditorPropertyType> NumericTypes = {
		{ UEdGraphSchema_K2::PC_Int },
		{ UEdGraphSchema_K2::PC_Int64 },
		{ UEdGraphSchema_K2::PC_Float },
		{ UEdGraphSchema_K2::PC_Double },
		{ UEdGraphSchema_K2::PC_Real },
	};
	MetaDataKeys.Append({
			FMDMetaDataKey{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") }.SetSupportedProperties(NumericTypes)
	});

	// Integers
	const TSet<FMDMetaDataEditorPropertyType> IntegerTypes = {
		{ UEdGraphSchema_K2::PC_Int },
		{ UEdGraphSchema_K2::PC_Int64 },
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Integer, TEXT("How fast the value should change while dragging to set the value.") }.SetSupportedProperties(IntegerTypes).SetMinInt(1),
		FMDMetaDataKey{ TEXT("Delta"), EMDMetaDataEditorKeyType::Integer, TEXT("How much to change the value by when dragging.") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Integer, TEXT("Forces the property value to be a multiple of this value.") }.SetSupportedProperties(IntegerTypes),
		FMDMetaDataKey{ TEXT("ArrayClamp"), EMDMetaDataEditorKeyType::String, TEXT("Clamps the valid values that can be entered in the UI to be between 0 and the length of the array specified.") }.SetSupportedProperties(IntegerTypes)
	});

	// Float types
	const TSet<FMDMetaDataEditorPropertyType> FloatTypes = {
		{ UEdGraphSchema_K2::PC_Float },
		{ UEdGraphSchema_K2::PC_Double },
		{ UEdGraphSchema_K2::PC_Real },
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Float, TEXT("How fast the value should change while dragging to set the value.") }.SetSupportedProperties(FloatTypes).SetMinFloat(1.f),
		FMDMetaDataKey{ TEXT("Delta"), EMDMetaDataEditorKeyType::Float, TEXT("How much to change the value by when dragging.") }.SetSupportedProperties(FloatTypes),
		FMDMetaDataKey{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Float, TEXT("Forces the property value to be a multiple of this value.") }.SetSupportedProperties(FloatTypes)
	});

	// Non-localized strings
	const TSet<FMDMetaDataEditorPropertyType> NonLocStringTypes = {
		{ UEdGraphSchema_K2::PC_String },
		{ UEdGraphSchema_K2::PC_Name }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("GetOptions"), EMDMetaDataEditorKeyType::String, TEXT("Specify a function that returns a list of Strings or Names that are valid values for this property.") }.SetSupportedProperties(NonLocStringTypes)
	});

	// Any Array
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("NoElementDuplicate"), EMDMetaDataEditorKeyType::Flag, TEXT("Indicates that the duplicate icon should not be shown for entries of this array in the property panel.") }.SetSupportedProperties({ FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Wildcard }.SetContainerType(EMDMetaDataPropertyContainerType::Array) })
	});

	// Any Map
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("ForceInlineRow"), EMDMetaDataEditorKeyType::Flag, TEXT("Force the Key and Value of a TMap to display in the same row.") }.SetSupportedProperties({ FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Wildcard }.SetValueType({UEdGraphSchema_K2::PC_Wildcard}).SetContainerType(EMDMetaDataPropertyContainerType::Map) })
	});

	// Array of Any Struct
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("TitleProperty"), EMDMetaDataEditorKeyType::String, TEXT("Specify a child property or FText style format of child properties to use as the summary.") }.SetSupportedProperties({ FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Struct }.SetContainerType(EMDMetaDataPropertyContainerType::Array) })
	});

	// Any Struct
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("ShowOnlyInnerProperties"), EMDMetaDataEditorKeyType::Flag, TEXT("Removes the struct layer in the details panel, directly displaying the child properties of the struct.") }.SetSupportedProperty({ UEdGraphSchema_K2::PC_Struct })
	});

	// Gameplay Tags and Containers
	const TSet<FMDMetaDataEditorPropertyType> GameplayTagTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, FGameplayTag::StaticStruct() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, FGameplayTagContainer::StaticStruct() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("Categories"), EMDMetaDataEditorKeyType::GameplayTagContainer, TEXT("Limit which gameplay tags may be selected to one or more specific root tags.") }.SetSupportedProperties(GameplayTagTypes)
	});

	// Data Tables
	const TSet<FMDMetaDataEditorPropertyType> DataTableTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, FDataTableRowHandle::StaticStruct() },
		{ UEdGraphSchema_K2::PC_Object, NAME_None, UDataTable::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UDataTable::StaticClass() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("RowType"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to a specific data table row struct type.") }.SetSupportedProperties(DataTableTypes),
		FMDMetaDataKey{ TEXT("RequiredAssetDataTags"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to data tables with matching asset data tags.") }.SetSupportedProperties(DataTableTypes)
	});

	// Colors
	const TSet<FMDMetaDataEditorPropertyType> ColorTypes = {
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FLinearColor>::Get() },
		{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FColor>::Get() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("HideAlphaChannel"), EMDMetaDataEditorKeyType::Flag, TEXT("Hide the alpha channel from the color picker.") }.SetSupportedProperties(ColorTypes)
	});

	// Any UObject type
	const TSet<FMDMetaDataEditorPropertyType> ObjectTypes = {
		{ UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass() },
		{ UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass() }
	};
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("DisplayThumbnail"), EMDMetaDataEditorKeyType::Boolean, TEXT("Whether or not to display the asset thumbnail.") }.SetSupportedProperties(ObjectTypes),
		FMDMetaDataKey{ TEXT("NoClear"), EMDMetaDataEditorKeyType::Flag, TEXT("Prevent this propert from being clear/set to none.") }.SetSupportedProperties(ObjectTypes)
	});

	// UMG Only
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("DesignerRebuild"), EMDMetaDataEditorKeyType::Flag, TEXT("When this property changes, the widget preview will be rebuilt.") }.SetSupportedBlueprint(UWidgetBlueprint::StaticClass())
	});

	// Functions Only
	MetaDataKeys.Append({
		FMDMetaDataKey{ TEXT("DefaultToSelf"), EMDMetaDataEditorKeyType::String, TEXT("Specify which function parameter should default to \"self\".") }.SetFunctionsOnly(),
		FMDMetaDataKey{ TEXT("DevelopmentOnly"), EMDMetaDataEditorKeyType::Flag, TEXT("Only allow this function to run in Development Mode.") }.SetFunctionsOnly(),
		FMDMetaDataKey{ TEXT("BlueprintAutocast"), EMDMetaDataEditorKeyType::Flag, TEXT("For Pure Blueprint Function Library functions, indicate that this function can be used to automatically cast between the first and return type.") }.SetFunctionsOnly()
	});
}

void UMDMetaDataEditorConfig::PostInitProperties()
{
	Super::PostInitProperties();

	// TODO - This also needs to happen when saving, but we don't want to rearrange the list while it's being edited
	MetaDataKeys.Sort([](const FMDMetaDataKey& A, const FMDMetaDataKey& B)
	{
		return A.Key.Compare(B.Key) < 0;
	});
}

FText UMDMetaDataEditorConfig::GetSectionText() const
{
	return INVTEXT("Meta Data Editor");
}

void UMDMetaDataEditorConfig::ForEachVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const
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

void UMDMetaDataEditorConfig::ForEachFunctionMetaDataKey(const UBlueprint* Blueprint, const TFunction<void(const FMDMetaDataKey&)>& Func) const
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
