// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SMDMetaDataStringComboBox : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_TwoParams(FOnSetMetaData, const FName&, const FString& Value);

	SLATE_BEGIN_ARGS(SMDMetaDataStringComboBox)
	{}
		SLATE_ARGUMENT_DEFAULT(FName, Key) = NAME_None;
		SLATE_ARGUMENT(TArray<FString>, ValueList);
		SLATE_ATTRIBUTE(TOptional<FString>, MetaDataValue);
		SLATE_EVENT(FOnSetMetaData, OnSetMetaData);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FString> GetCurrentValue() const;
	void OnSelected(TSharedPtr<FString> ValuePtr, ESelectInfo::Type SelectInfo);

	FName Key = NAME_None;
	TAttribute<TOptional<FString>> MetaDataValue;
	FOnSetMetaData OnSetMetaData;

	TArray<TSharedPtr<FString>> ValueList;

};
