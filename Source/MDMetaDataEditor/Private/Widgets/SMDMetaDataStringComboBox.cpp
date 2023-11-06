// Copyright Dylan Dumesnil. All Rights Reserved.

#include "SMDMetaDataStringComboBox.h"

#include "DetailLayoutBuilder.h"
#include "Widgets/Input/STextComboBox.h"

void SMDMetaDataStringComboBox::Construct(const FArguments& InArgs)
{
	Key = InArgs._Key;
	MetaDataValue = InArgs._MetaDataValue;
	OnSetMetaData = InArgs._OnSetMetaData;

	Algo::Transform(InArgs._ValueList, ValueList, [](const FString& Value)
	{
		return MakeShared<FString>(Value);
	});

	ChildSlot
	[
		SNew(STextComboBox)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OptionsSource(&ValueList)
		.InitiallySelectedItem(GetCurrentValue())
		.OnSelectionChanged(this, &SMDMetaDataStringComboBox::OnSelected)
	];
}

TSharedPtr<FString> SMDMetaDataStringComboBox::GetCurrentValue() const
{
	const TOptional<FString> ValueStringOptional = MetaDataValue.Get(TOptional<FString>());
	if (ValueStringOptional.IsSet())
	{
		const FString& Value = ValueStringOptional.GetValue();
		for (const TSharedPtr<FString>& ValuePtr : ValueList)
		{
			if (ValuePtr.IsValid() && *ValuePtr == Value)
			{
				return ValuePtr;
			}
		}
	}

	return nullptr;
}

void SMDMetaDataStringComboBox::OnSelected(TSharedPtr<FString> ValuePtr, ESelectInfo::Type SelectInfo)
{
	if (ValuePtr.IsValid())
	{
		OnSetMetaData.ExecuteIfBound(Key, *ValuePtr);
	}
}
