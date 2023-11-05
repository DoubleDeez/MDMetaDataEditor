// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "EdGraphSchema_K2.h"
#include "Engine/DeveloperSettings.h"
#include "InstancedStruct.h"
#include "Math/NumericLimits.h"

#include "MDMetaDataEditorConfig.generated.h"

UENUM()
enum class EMDMetaDataPropertyContainerType : uint8
{
	None,
	Array,
	Set,
	Map
};

USTRUCT()
struct FMDMetaDataEditorPropertyType
{
	GENERATED_BODY()

public:

	FEdGraphPinType ToGraphPinType() const;
	FEdGraphTerminalType ToGraphTerminalType() const;

	void SetFromGraphPinType(const FEdGraphPinType& GraphPinType);
	void SetFromGraphTerminalType(const FEdGraphTerminalType& GraphTerminalType);

	bool DoesMatchProperty(const FProperty* Property) const;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FName PropertyType = UEdGraphSchema_K2::PC_Wildcard;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FName PropertySubType = NAME_None;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TSoftObjectPtr<UObject> PropertySubTypeObject;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FSimpleMemberReference PropertySubTypeMemberReference;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FInstancedStruct ValueType;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	EMDMetaDataPropertyContainerType ContainerType = EMDMetaDataPropertyContainerType::None;

	bool operator==(const FMDMetaDataEditorPropertyType& Other) const;
	bool operator!=(const FMDMetaDataEditorPropertyType& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FMDMetaDataEditorPropertyType& Key)
	{
		uint32 Hash = GetTypeHash(Key.PropertyType);
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubType));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeObject));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeMemberReference.MemberParent));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeMemberReference.MemberName));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeMemberReference.MemberGuid));
		Hash = HashCombine(Hash, GetTypeHash(Key.ValueType.GetScriptStruct()));
		Hash = HashCombine(Hash, GetTypeHash(Key.ValueType.GetMemory()));
		Hash = HashCombine(Hash, GetTypeHash(Key.ContainerType));
		return Hash;
	}
};

UENUM()
enum class EMDMetaDataEditorKeyType : uint8
{
	// The meta data key is added or removed with a checkbox
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
	// TODO - The meta data value is a UStruct path
	// Struct,
	// TODO - The meta data value is a UClass path
	// Class
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

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FString Description;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Integer || KeyType == EMDMetaDataEditorKeyType::Float"))
	bool bAllowSlider = true;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MinInt = TNumericLimits<int32>::Lowest();

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MaxInt = TNumericLimits<int32>::Max();

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MinSliderInt = 0;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Integer"))
	int32 MaxSliderInt = 100;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Float"))
	float MinFloat = TNumericLimits<float>::Lowest();

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "KeyType == EMDMetaDataEditorKeyType::Float"))
	float MaxFloat = TNumericLimits<float>::Max();

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Float"))
	float MinSliderFloat = 0.f;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (EditConditionHides, EditCondition = "bAllowSlider && KeyType == EMDMetaDataEditorKeyType::Float"))
	float MaxSliderFloat = 100.f;

	bool operator==(const FMDMetaDataKey& Other) const;
	bool operator!=(const FMDMetaDataKey& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FMDMetaDataKey& Key)
	{
		return HashCombine(GetTypeHash(Key.Key), GetTypeHash(Key.KeyType));
	}
};

USTRUCT()
struct FMDMetaDataKeyList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (TitleProperty = "Key"))
	TArray<FMDMetaDataKey> MetaDataKeys;
};

USTRUCT()
struct FMDMetaDataEditorBlueprintConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TSoftClassPtr<UBlueprint> BlueprintType;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TMap<FMDMetaDataEditorPropertyType, FMDMetaDataKeyList> VariableMetaDataKeys;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FMDMetaDataKeyList FunctionMetaDataKeys;
};

/**
 * Configure which meta data keys will display on blueprint Properties and Functions.
 * Can be setup per Blueprint type and Property type.
 */
UCLASS(DefaultConfig, Config = Editor, MinimalAPI)
class UMDMetaDataEditorConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMDMetaDataEditorConfig();

	virtual FText GetSectionText() const override;

	void ForEachVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const;
	void ForEachFunctionMetaDataKey(const UBlueprint* Blueprint, const TFunction<void(const FMDMetaDataKey&)>& Func) const;

private:
	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Editor", meta = (TitleProperty = "BlueprintType"))
	TArray<FMDMetaDataEditorBlueprintConfig> BlueprintMetaDataConfigs;
};
