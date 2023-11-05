// Copyright Dylan Dumesnil. All Rights Reserved.


#include "MDMetaDataEditorConfig.h"

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "WidgetBlueprint.h"

FEdGraphPinType FMDMetaDataEditorPropertyType::ToGraphPinType() const
{
	FEdGraphPinType PinType;
	PinType.PinCategory = PropertyType;
	PinType.PinSubCategory = PropertySubType;
	PinType.PinSubCategoryObject = PropertySubTypeObject.LoadSynchronous();
	PinType.PinSubCategoryMemberReference = PropertySubTypeMemberReference;

	if (ValueType.IsValid())
	{
		PinType.PinValueType = ValueType.Get<FMDMetaDataEditorPropertyType>().ToGraphTerminalType();
	}

	switch (ContainerType)
	{
	case EMDMetaDataPropertyContainerType::None:
		PinType.ContainerType = EPinContainerType::None;
		break;
	case EMDMetaDataPropertyContainerType::Array:
		PinType.ContainerType = EPinContainerType::Array;
		break;
	case EMDMetaDataPropertyContainerType::Set:
		PinType.ContainerType = EPinContainerType::Set;
		break;
	case EMDMetaDataPropertyContainerType::Map:
		PinType.ContainerType = EPinContainerType::Map;
		break;
	}

	return PinType;
}

FEdGraphTerminalType FMDMetaDataEditorPropertyType::ToGraphTerminalType() const
{
	FEdGraphTerminalType TerminalType;
	TerminalType.TerminalCategory = PropertyType;
	TerminalType.TerminalSubCategory = PropertySubType;
	TerminalType.TerminalSubCategoryObject = PropertySubTypeObject.LoadSynchronous();

	return TerminalType;
}

void FMDMetaDataEditorPropertyType::SetFromGraphPinType(const FEdGraphPinType& GraphPinType)
{
	PropertyType = GraphPinType.PinCategory;
	PropertySubType = GraphPinType.PinSubCategory;
	PropertySubTypeObject = GraphPinType.PinSubCategoryObject.Get();
	PropertySubTypeMemberReference = GraphPinType.PinSubCategoryMemberReference;

	if (!GraphPinType.PinValueType.TerminalCategory.IsNone())
	{
		ValueType = ValueType.Make<FMDMetaDataEditorPropertyType>();
		ValueType.GetMutable<FMDMetaDataEditorPropertyType>().SetFromGraphTerminalType(GraphPinType.PinValueType);
	}
	else if (GraphPinType.ContainerType == EPinContainerType::Map)
	{
		ValueType = ValueType.Make<FMDMetaDataEditorPropertyType>();
	}
	else
	{
		ValueType.Reset();
	}

	switch (GraphPinType.ContainerType)
	{
	case EPinContainerType::None:
		ContainerType = EMDMetaDataPropertyContainerType::None;
		break;
	case EPinContainerType::Array:
		ContainerType = EMDMetaDataPropertyContainerType::Array;
		break;
	case EPinContainerType::Set:
		ContainerType = EMDMetaDataPropertyContainerType::Set;
		break;
	case EPinContainerType::Map:
		ContainerType = EMDMetaDataPropertyContainerType::Map;
		break;
	}
}

void FMDMetaDataEditorPropertyType::SetFromGraphTerminalType(const FEdGraphTerminalType& GraphTerminalType)
{
	PropertyType = GraphTerminalType.TerminalCategory;
	PropertySubType = GraphTerminalType.TerminalSubCategory;
	PropertySubTypeObject = GraphTerminalType.TerminalSubCategoryObject.Get();
}

bool FMDMetaDataEditorPropertyType::DoesMatchProperty(const FProperty* Property) const
{
	if (Property == nullptr)
	{
		return false;
	}

	const FProperty* EffectiveProp = Property;

	if (ContainerType == EMDMetaDataPropertyContainerType::Array && !Property->IsA<FArrayProperty>())
	{
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
		if (ArrayProperty == nullptr)
		{
			return false;
		}

		EffectiveProp = ArrayProperty->Inner;
	}
	else if (ContainerType == EMDMetaDataPropertyContainerType::Set && !Property->IsA<FSetProperty>())
	{
		const FSetProperty* SetProperty = CastField<FSetProperty>(Property);
		if (SetProperty == nullptr)
		{
			return false;
		}

		EffectiveProp = SetProperty->ElementProp;
	}
	else if (ContainerType == EMDMetaDataPropertyContainerType::Map)
	{
		const FMapProperty* MapProperty = CastField<FMapProperty>(Property);
		if (MapProperty == nullptr)
		{
			return false;
		}

		const FMDMetaDataEditorPropertyType* ValueTypePtr = ValueType.GetPtr<FMDMetaDataEditorPropertyType>();
		if (ValueTypePtr == nullptr || !ValueTypePtr->DoesMatchProperty(MapProperty->ValueProp))
		{
			return false;
		}

		EffectiveProp = MapProperty->KeyProp;
	}

	if (PropertyType == UEdGraphSchema_K2::PC_Wildcard)
	{
		return true;
	}

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	FEdGraphPinType PinType;
	if (!K2Schema->ConvertPropertyToPinType(EffectiveProp, PinType))
	{
		return false;
	}

	return PropertyType == PinType.PinCategory
		&& PropertySubType == PinType.PinSubCategory
		&& PropertySubTypeObject.Get() == PinType.PinSubCategoryObject
		&& PropertySubTypeMemberReference == PinType.PinSubCategoryMemberReference;
}

bool FMDMetaDataEditorPropertyType::operator==(const FMDMetaDataEditorPropertyType& Other) const
{
	return PropertyType == Other.PropertyType
		&& PropertySubType == Other.PropertySubType
		&& PropertySubTypeObject == Other.PropertySubTypeObject
		&& PropertySubTypeMemberReference == Other.PropertySubTypeMemberReference
		&& ValueType == Other.ValueType
		&& ContainerType == Other.ContainerType;
}

bool FMDMetaDataKey::operator==(const FMDMetaDataKey& Other) const
{
	return Key == Other.Key && KeyType == Other.KeyType;
}

UMDMetaDataEditorConfig::UMDMetaDataEditorConfig()
{
	// Setup some useful defaults

	BlueprintMetaDataConfigs.Add({
		UBlueprint::StaticClass(),
		{
			{
				{ },
				{
					{
						{ TEXT("EditCondition"), EMDMetaDataEditorKeyType::String, TEXT("Enter a condition to determine whether or not this property can be edited. Supports Bools and Enums.") },
						{ TEXT("EditConditionHides"), EMDMetaDataEditorKeyType::Flag, TEXT("If this property's EditCondition is false, it will be hidden.") }
					}
				}
			},
			{
				{ UEdGraphSchema_K2::PC_Boolean },
				{ { { TEXT("InlineEditConditionToggle"), EMDMetaDataEditorKeyType::Flag, TEXT("If this bool is an EditCondition for another property, it will be displayed inline.") } } }
			},
			{
				{ UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr, {}, FInstancedStruct::Make<FMDMetaDataEditorPropertyType>(), EMDMetaDataPropertyContainerType::Map },
				{ { { TEXT("ForceInlineRow"), EMDMetaDataEditorKeyType::Flag, TEXT("Force the Key and Value of a TMap to display in the same row.") } } }
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
				{ { { TEXT("RowType"), EMDMetaDataEditorKeyType::String, TEXT("Limit the selection to a specific data table row struct type.") } } }
			}
		},
		{
			{
				{ TEXT("DefaultToSelf"), EMDMetaDataEditorKeyType::String, TEXT("Specify which function parameter should default to \"self\".") }
			}
		}
	});

	BlueprintMetaDataConfigs.Add({
		UWidgetBlueprint::StaticClass(),
		{
			{
				{ },
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
