// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Widgets/SCompoundWidget.h"

/**
 * A custom gameplay tag picker that supports Unreal 5.1+ and works around engine bugs
 */
class SMDMetaDataGameplayTagPicker : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam(FOnRemoveMetaData, const FName&);
	DECLARE_DELEGATE_TwoParams(FOnSetMetaData, const FName&, const FString& Value);

	SLATE_BEGIN_ARGS(SMDMetaDataGameplayTagPicker)
	{}
		SLATE_ARGUMENT_DEFAULT(bool, bMultiSelect) = false;
		SLATE_ARGUMENT_DEFAULT(FName, Key) = NAME_None;

		SLATE_ATTRIBUTE(TOptional<FString>, MetaDataValue);
		SLATE_EVENT(FOnRemoveMetaData, OnRemoveMetaData);
		SLATE_EVENT(FOnSetMetaData, OnSetMetaData);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FText GetValue() const;
	FText GetValueToolTip() const;

	void UpdateMetaDataContainer(const TArray<FGameplayTagContainer>& Containers);

	void UpdateMetaDataTag(const FGameplayTag InTag);

	void UpdateMetaData(bool bDontUpdate);

	FName Key = NAME_None;
	bool bIsMulti = false;
	FGameplayTagContainer GameplayTagContainer;
	TAttribute<TOptional<FString>> MetaDataValue;
	FOnRemoveMetaData OnRemoveMetaData;
	FOnSetMetaData OnSetMetaData;
};
