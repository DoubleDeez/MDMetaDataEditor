// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Math/NumericLimits.h"
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
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor", meta = (TitleProperty = "{Key} ({KeyType})"))
	TArray<FMDMetaDataKey> MetaDataKeys;
};
