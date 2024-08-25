// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Math/NumericLimits.h"
#include "MDMetaDataEditorPropertyType.h"

#include "MDMetaDataKey.generated.h"

UENUM()
enum class EMDMetaDataEditorKeyType : uint8
{
	// The meta data key is added or removed with a checkbox but doesn't have a value
	Flag,
	// The meta data value is set to true or false with a checkbox,
	Boolean,
	// The meta data value is a user-specified string
	String,
	// The meta data value is a user-specified integer
	Integer,
	// The meta data value is a user-specified floating point
	Float,
	// The meta data value is a user-specified Gameplay Tag
	GameplayTag,
	// The meta data value is one or more user-specified Gameplay Tags
	GameplayTagContainer,
	// The meta data value is selected from the list of specified values
	ValueList
	// TODO - The meta data value is a UStruct path
	// Struct,
	// TODO - The meta data value is a UClass path
	// Class,
	// TODO - The meta data value is a UInterface path
	// Interface,
	// TODO - The meta data value is a UEnum path
	// Enum,
	// TODO - The meta data value is a value of a specific enum (maybe always of the property? Let user select UEnum if property isn't an enum?)
	// EnumValue,
	// TODO - The meta data value is the name of a param on this function
	// Param,
	// TODO - Provide a custom Struct that creates the editor widget and sets the value of the meta data
	// Custom,
};

USTRUCT()
struct FMDMetaDataKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FName Key = NAME_None;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	EMDMetaDataEditorKeyType KeyType = EMDMetaDataEditorKeyType::Flag;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (MultiLine))
	FString Description;

	// Group meta data in sub-categories by setting this value, nested categories delineated with pipe characters `|`
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FString Category;

	// This metadata value will be hidden unless the property/function also has the specified `RequiredMetaData`
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (GetOptions = "GetMetaDataKeyNames"))
	FName RequiredMetaData = NAME_None;
	FMDMetaDataKey& SetRequiredMetaData(const FName& InRequiredMetaData) { RequiredMetaData = InRequiredMetaData; return *this; }

	// This metadata value will be hidden if the property/function also has any of the specified `IncompatibleMetaData`
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (GetOptions = "GetMetaDataKeyNames"))
	TSet<FName> IncompatibleMetaData;
	FMDMetaDataKey& AddIncompatibleMetaData(const FName& InIncompatibleMetaData) { IncompatibleMetaData.Add(InIncompatibleMetaData); return *this; }
	FMDMetaDataKey& SetIncompatibleMetaData(const FName& InIncompatibleMetaData) { IncompatibleMetaData = { InIncompatibleMetaData}; return *this; }
	FMDMetaDataKey& SetIncompatibleMetaData(TSet<FName>&& InIncompatibleMetaData) { IncompatibleMetaData = MoveTemp(InIncompatibleMetaData); return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TSet<TSoftClassPtr<UBlueprint>> SupportedBlueprints = { UBlueprint::StaticClass() };
	FMDMetaDataKey& ClearSupportedBlueprints() { SupportedBlueprints.Reset(); return *this; }
	FMDMetaDataKey& AddSupportedBlueprint(TSoftClassPtr<UBlueprint>&& InSupportedBlueprint) { SupportedBlueprints.Emplace(MoveTemp(InSupportedBlueprint)); return *this; }
	FMDMetaDataKey& SetSupportedBlueprint(TSoftClassPtr<UBlueprint>&& InSupportedBlueprint) { ClearSupportedBlueprints().AddSupportedBlueprint(MoveTemp(InSupportedBlueprint)); return *this; }
	FMDMetaDataKey& SetSupportedBlueprints(const TSet<TSoftClassPtr<UBlueprint>>& InSupportedBlueprints) { SupportedBlueprints = InSupportedBlueprints; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TSet<FMDMetaDataEditorPropertyType> SupportedPropertyTypes = { { UEdGraphSchema_K2::PC_Wildcard } };
	FMDMetaDataKey& ClearSupportedProperties() { SupportedPropertyTypes.Reset(); return *this; }
	FMDMetaDataKey& AddSupportedProperty(FMDMetaDataEditorPropertyType&& InSupportedProperty) { SupportedPropertyTypes.Emplace(MoveTemp(InSupportedProperty)); return *this; }
	FMDMetaDataKey& SetSupportedProperty(FMDMetaDataEditorPropertyType&& InSupportedProperty) { ClearSupportedProperties().AddSupportedProperty(MoveTemp(InSupportedProperty)); return *this; }
	FMDMetaDataKey& SetSupportedProperties(const TSet<FMDMetaDataEditorPropertyType>& InSupportedProperties) { SupportedPropertyTypes = InSupportedProperties; return *this; }
	FMDMetaDataKey& AddSupportedObjectProperty(UClass* ObjectClass, bool bIncludeSoft = false)
	{
		check(ObjectClass);
		AddSupportedProperty({ UEdGraphSchema_K2::PC_Object, NAME_None, ObjectClass });
		if (bIncludeSoft)
		{
			AddSupportedProperty({ UEdGraphSchema_K2::PC_SoftObject, NAME_None, ObjectClass });
		}
		return *this;
	}
	FMDMetaDataKey& SetSupportedObjectProperty(UClass* ObjectClass, bool bIncludeSoft = false) { return ClearSupportedProperties().AddSupportedObjectProperty(ObjectClass, bIncludeSoft); }
	FMDMetaDataKey& AddSupportedClassProperty(UClass* ObjectClass, bool bIncludeSoft = false)
	{
		check(ObjectClass);
		AddSupportedProperty({ UEdGraphSchema_K2::PC_Class, NAME_None, ObjectClass });
		if (bIncludeSoft)
		{
			AddSupportedProperty({ UEdGraphSchema_K2::PC_Class, NAME_None, ObjectClass });
		}
		return *this;
	}
	FMDMetaDataKey& SetSupportedClassProperty(UClass* ObjectClass, bool bIncludeSoft = false) { return ClearSupportedProperties().AddSupportedClassProperty(ObjectClass, bIncludeSoft); }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	bool bCanBeUsedByFunctions = false;
	FMDMetaDataKey& CanBeUsedByFunctions(bool InCanBeUsedByFunctions) { bCanBeUsedByFunctions = InCanBeUsedByFunctions; return *this; }
	FMDMetaDataKey& SetFunctionsOnly() { bCanBeUsedByFunctions = true; bCanBeUsedByStructs = false; bCanBeUsedOnFunctionParameters = false; bCanBeUsedOnVariables = false; bCanBeUsedOnLocalVariables = false; ClearSupportedProperties(); return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	bool bCanBeUsedByStructs = false;
	FMDMetaDataKey& CanBeUsedByStructs(bool InCanBeUsedByStructs) { bCanBeUsedByStructs = InCanBeUsedByStructs; return *this; }
	FMDMetaDataKey& SetStructsOnly() { bCanBeUsedByFunctions = false; bCanBeUsedByStructs = true; bCanBeUsedOnFunctionParameters = false; bCanBeUsedOnVariables = false; bCanBeUsedOnLocalVariables = false; ClearSupportedProperties(); return *this; }

	// Whether or not this meta data can be used on Blueprint Variables
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	bool bCanBeUsedOnVariables = true;
	FMDMetaDataKey& CanBeUsedOnVariables(bool InCanBeUsedOnVariables) { bCanBeUsedOnVariables = InCanBeUsedOnVariables; return *this; }

	// Whether or not this meta data can be used on Blueprint Function's Local Variables
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	bool bCanBeUsedOnLocalVariables = true;
	FMDMetaDataKey& CanBeUsedOnLocalVariables(bool InCanBeUsedOnLocalVariables) { bCanBeUsedOnLocalVariables = InCanBeUsedOnLocalVariables; return *this; }

	// Whether or not this meta data can be used on Blueprint Function Parameters
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	bool bCanBeUsedOnFunctionParameters = true;
	FMDMetaDataKey& CanBeUsedOnFunctionParameters(bool InCanBeUsedOnFunctionParameters) { bCanBeUsedOnFunctionParameters = InCanBeUsedOnFunctionParameters; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Integer || KeyType == EMDMetaDataEditorKeyType::Float"))
	bool bAllowSlider = true;
	FMDMetaDataKey& AllowSlider(bool InAllowSlider) { bAllowSlider = InAllowSlider; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MinInt = TNumericLimits<int32>::Lowest();
	FMDMetaDataKey& SetMinInt(int32 InMinInt) { MinInt = InMinInt; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MaxInt = TNumericLimits<int32>::Max();
	FMDMetaDataKey& SetMaxInt(int32 InMaxInt) { MaxInt = InMaxInt; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MinSliderInt = 0;
	FMDMetaDataKey& SetMinSliderInt(int32 InMinSliderInt) { MinSliderInt = InMinSliderInt; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MaxSliderInt = 100;
	FMDMetaDataKey& SetMaxSliderInt(int32 InMaxSliderInt) { MaxSliderInt = InMaxSliderInt; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Float"))
	float MinFloat = TNumericLimits<float>::Lowest();
	FMDMetaDataKey& SetMinFloat(float InMinFloat) { MinFloat = InMinFloat; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Float"))
	float MaxFloat = TNumericLimits<float>::Max();
	FMDMetaDataKey& SetMaxFloat(float InMaxFloat) { MaxFloat = InMaxFloat; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Float"))
	float MinSliderFloat = 0.f;
	FMDMetaDataKey& SetMinSliderFloat(float InMinSliderFloat) { MinSliderFloat = InMinSliderFloat; return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Float"))
	float MaxSliderFloat = 100.f;
	FMDMetaDataKey& SetMaxSliderFloat(float InMaxSliderFloat) { MaxSliderFloat = InMaxSliderFloat; return *this; }

	// The list of values the user can select from when setting the value of this meta data key
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::ValueList"))
	TArray<FString> ValueList;

	bool DoesSupportBlueprint(const UBlueprint* Blueprint) const;
	bool DoesSupportProperty(const FProperty* Property) const;

	// Overrides the User friendly name to show for this key if not empty.
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (InlineEditConditionToggle))
	bool bUseDisplayNameOverride = false;

	// Overrides the User friendly name to show for this key if not empty.
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (DisplayAfter = "KeyType", EditCondition = "bUseDisplayNameOverride"))
	FText DisplayNameOverride;
	FMDMetaDataKey& SetDisplayNameOverride(FText&& InDisplayNameOverride) { bUseDisplayNameOverride = true; DisplayNameOverride = MoveTemp(InDisplayNameOverride); return *this; }

	FText GetKeyDisplayText() const;
	FText GetToolTipText() const;
	FText GetFilterText() const;

	bool operator==(const FMDMetaDataKey& Other) const;
	bool operator!=(const FMDMetaDataKey& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FMDMetaDataKey& MetaDataKey)
	{
		return HashCombine(GetTypeHash(MetaDataKey.Key), GetTypeHash(MetaDataKey.KeyType));
	}
};
