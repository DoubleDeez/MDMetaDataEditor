// Copyright Dylan Dumesnil. All Rights Reserved.


#include "MDMetaDataEditorConfig.h"

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "WidgetBlueprint.h"

UMDMetaDataEditorConfig::UMDMetaDataEditorConfig()
{
	// Setup some useful defaults, with some ugly code

	BlueprintMetaDataConfigs.Add({
		UBlueprint::StaticClass(),
		{
			{
				{ UEdGraphSchema_K2::PC_Wildcard },
				{
					{
						{ TEXT("EditCondition"), EMDMetaDataEditorKeyType::String, TEXT("Enter a condition to determine whether or not this property can be edited. Supports Bools and Enums.") },
						{ TEXT("EditConditionHides"), EMDMetaDataEditorKeyType::Flag, TEXT("If this property's EditCondition is false, it will be hidden.") },
						{ TEXT("DisplayAfter"), EMDMetaDataEditorKeyType::String, TEXT("In the details panel, this property will be displayed after the property specified here.") },
						{ TEXT("DisplayPriority"), EMDMetaDataEditorKeyType::Integer, TEXT("The priority to display this property in the deatils panel, lower values are first.") },
						{ TEXT("NoResetToDefault"), EMDMetaDataEditorKeyType::Flag, TEXT("If set, this property will never show the 'Reset to Default' arrow button.") },
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_Boolean },
				{ { { TEXT("InlineEditConditionToggle"), EMDMetaDataEditorKeyType::Flag, TEXT("If this bool is an EditCondition for another property, it will be displayed inline.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Int },
				{
					{
						{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") },
						FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Integer, TEXT("How fast the value should change while dragging to set the value.") }.SetMinInt(1),
						{ TEXT("Delta"), EMDMetaDataEditorKeyType::Integer, TEXT("How much to change the value by when dragging.") },
						{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Integer, TEXT("Forces the property value to be a multiple of this value.") },
						{ TEXT("ArrayClamp"), EMDMetaDataEditorKeyType::String, TEXT("Clamps the valid values that can be entered in the UI to be between 0 and the length of the array specified.") }
					}
				},
			},
			{
				{ UEdGraphSchema_K2::PC_Int64 },
				{
					{
						{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") },
						FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Integer, TEXT("How fast the value should change while dragging to set the value.") }.SetMinInt(1),
						{ TEXT("Delta"), EMDMetaDataEditorKeyType::Integer, TEXT("How much to change the value by when dragging.") },
						{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Integer, TEXT("Forces the property value to be a multiple of this value.") },
						{ TEXT("ArrayClamp"), EMDMetaDataEditorKeyType::String, TEXT("Clamps the valid values that can be entered in the UI to be between 0 and the length of the array specified.") }
					}
				},
			},
			{
				{ UEdGraphSchema_K2::PC_Float },
				{
					{
						{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") },
						FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Float, TEXT("How fast the value should change while dragging to set the value.") }.SetMinFloat(1.f),
						{ TEXT("Delta"), EMDMetaDataEditorKeyType::Float, TEXT("How much to change the value by when dragging.") },
						{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Float, TEXT("Forces the property value to be a multiple of this value.") }
					}
				},
			},
			{
				{ UEdGraphSchema_K2::PC_Double },
				{
					{
						{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") },
						FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Float, TEXT("How fast the value should change while dragging to set the value.") }.SetMinFloat(1.f),
						{ TEXT("Delta"), EMDMetaDataEditorKeyType::Float, TEXT("How much to change the value by when dragging.") },
						{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Float, TEXT("Forces the property value to be a multiple of this value.") }
					}
				},
			},
			{
				{ UEdGraphSchema_K2::PC_Real },
				{
					{
						{ TEXT("NoSpinbox"), EMDMetaDataEditorKeyType::Boolean, TEXT("Disables the click and drag functionality for setting the value of this property.") },
						FMDMetaDataKey{ TEXT("SliderExponent"), EMDMetaDataEditorKeyType::Float, TEXT("How fast the value should change while dragging to set the value.") }.SetMinFloat(1.f),
						{ TEXT("Delta"), EMDMetaDataEditorKeyType::Float, TEXT("How much to change the value by when dragging.") },
						{ TEXT("Multiple"), EMDMetaDataEditorKeyType::Float, TEXT("Forces the property value to be a multiple of this value.") }
					}
				},
			},
			{
				{ UEdGraphSchema_K2::PC_String },
				{ { { TEXT("GetOptions"), EMDMetaDataEditorKeyType::String, TEXT("Specify a function that returns a list of Strings that are valid values for this property.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Name },
				{ { { TEXT("GetOptions"), EMDMetaDataEditorKeyType::String, TEXT("Specify a function that returns a list of Names that are valid values for this property.") } } }
			},
			{
				FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Wildcard }.SetContainerType(EMDMetaDataPropertyContainerType::Array),
				{ { { TEXT("NoElementDuplicate"), EMDMetaDataEditorKeyType::Flag, TEXT("Indicates that the duplicate icon should not be shown for entries of this array in the property panel.") } } }
			},
			{
				FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Wildcard }.SetValueType({UEdGraphSchema_K2::PC_Wildcard}).SetContainerType(EMDMetaDataPropertyContainerType::Map),
				{ { { TEXT("ForceInlineRow"), EMDMetaDataEditorKeyType::Flag, TEXT("Force the Key and Value of a TMap to display in the same row.") } } }
			},
			{
				FMDMetaDataEditorPropertyType{ UEdGraphSchema_K2::PC_Struct }.SetContainerType(EMDMetaDataPropertyContainerType::Array),
				{ { { TEXT("TitleProperty"), EMDMetaDataEditorKeyType::String, TEXT("Specify a child property or FText style format of child properties to use as the summary.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Struct },
				{ { { TEXT("ShowOnlyInnerProperties"), EMDMetaDataEditorKeyType::Flag, TEXT("Removes the struct layer in the details panel, directly displaying the child properties of the struct.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Struct, NAME_None, FGameplayTag::StaticStruct() },
				{ { { TEXT("Categories"), EMDMetaDataEditorKeyType::GameplayTagContainer, TEXT("Limit which gameplay tags may be selected to one or more specific root tags.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Struct, NAME_None, FGameplayTagContainer::StaticStruct() },
				{ { { TEXT("Categories"), EMDMetaDataEditorKeyType::GameplayTagContainer, TEXT("Limit which gameplay tags may be selected to one or more specific root tags.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Struct, NAME_None, FDataTableRowHandle::StaticStruct() },
				{
					{
						{ TEXT("RowType"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to a specific data table row struct type.") },
						{ TEXT("RequiredAssetDataTags"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to data tables with matching asset data tags.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FLinearColor>::Get() },
				{ { { TEXT("HideAlphaChannel"), EMDMetaDataEditorKeyType::Flag, TEXT("Hide the alpha channel from the color picker.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Struct, NAME_None, TBaseStructure<FColor>::Get() },
				{ { { TEXT("HideAlphaChannel"), EMDMetaDataEditorKeyType::Flag, TEXT("Hide the alpha channel from the color picker.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Object, NAME_None, UObject::StaticClass() },
				{
					{
						{ TEXT("DisplayThumbnail"), EMDMetaDataEditorKeyType::Boolean, TEXT("Whether or not to display the asset thumbnail.") },
						{ TEXT("NoClear"), EMDMetaDataEditorKeyType::Flag, TEXT("Prevent this propert from being clear/set to none.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UObject::StaticClass() },
				{
					{
						{ TEXT("DisplayThumbnail"), EMDMetaDataEditorKeyType::Boolean, TEXT("Whether or not to display the asset thumbnail.") },
						{ TEXT("NoClear"), EMDMetaDataEditorKeyType::Flag, TEXT("Prevent this propert from being clear/set to none.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_Class, NAME_None, UObject::StaticClass() },
				{
					{
						{ TEXT("DisplayThumbnail"), EMDMetaDataEditorKeyType::Boolean, TEXT("Whether or not to display the asset thumbnail.") },
						{ TEXT("NoClear"), EMDMetaDataEditorKeyType::Flag, TEXT("Prevent this propert from being clear/set to none.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_SoftClass, NAME_None, UObject::StaticClass() },
				{
					{
						{ TEXT("DisplayThumbnail"), EMDMetaDataEditorKeyType::Boolean, TEXT("Whether or not to display the asset thumbnail.") },
						{ TEXT("NoClear"), EMDMetaDataEditorKeyType::Flag, TEXT("Prevent this propert from being clear/set to none.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_Object, NAME_None, UDataTable::StaticClass() },
				{
					{
						{ TEXT("RowType"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to a specific data table row struct type.") },
						{ TEXT("RequiredAssetDataTags"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to data tables with matching asset data tags.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_SoftObject, NAME_None, UDataTable::StaticClass() },
				{
					{
						{ TEXT("RowType"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to a specific data table row struct type.") },
						{ TEXT("RequiredAssetDataTags"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to data tables with matching asset data tags.") }
					}
				}
			},
		},
		{
			{
				{ TEXT("DefaultToSelf"), EMDMetaDataEditorKeyType::String, TEXT("Specify which function parameter should default to \"self\".") },
				{ TEXT("DevelopmentOnly"), EMDMetaDataEditorKeyType::Flag, TEXT("Only allow this function to run in Development Mode.") },
				{ TEXT("BlueprintAutocast"), EMDMetaDataEditorKeyType::Flag, TEXT("For Pure Blueprint Function Library functions, indicate that this function can be used to automatically cast between the first and return type.") }
			}
		}
	});

	BlueprintMetaDataConfigs.Add({
		UWidgetBlueprint::StaticClass(),
		{
			{
				{ UEdGraphSchema_K2::PC_Wildcard },
				{ { { TEXT("DesignerRebuild"), EMDMetaDataEditorKeyType::Flag, TEXT("When this property changes, the widget preview will be rebuilt.") } } }
			}
		}
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

	for (const FMDMetaDataEditorBlueprintConfig& Config : BlueprintMetaDataConfigs)
	{
		if (!Blueprint->IsA(Config.BlueprintType.Get()))
		{
			continue;
		}

		for (const auto& Pair : Config.VariableMetaDataKeys)
		{
			if (Pair.Key.DoesMatchProperty(Property))
			{
				for (const FMDMetaDataKey& Key : Pair.Value.MetaDataKeys)
				{
					Func(Key);
				}
			}
		}
	}
}

void UMDMetaDataEditorConfig::ForEachFunctionMetaDataKey(const UBlueprint* Blueprint, const TFunction<void(const FMDMetaDataKey&)>& Func) const
{
	if (!IsValid(Blueprint))
	{
		return;
	}

	for (const FMDMetaDataEditorBlueprintConfig& Config : BlueprintMetaDataConfigs)
	{
		if (!Blueprint->IsA(Config.BlueprintType.Get()))
		{
			continue;
		}

		for (const FMDMetaDataKey& Key : Config.FunctionMetaDataKeys.MetaDataKeys)
		{
			Func(Key);
		}
	}
}
