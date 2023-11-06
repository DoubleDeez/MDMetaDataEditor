// Copyright Dylan Dumesnil. All Rights Reserved.


#include "SMDMetaDataGameplayTagPicker.h"

#include "DetailLayoutBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 3
#include "SGameplayTagCombo.h"
#include "SGameplayTagPicker.h"
#else
#include "SGameplayTagWidget.h"
#endif
#include "Widgets/Input/SComboButton.h"

void SMDMetaDataGameplayTagPicker::Construct(const FArguments& InArgs)
{
	Key = InArgs._Key;
	bIsMulti = InArgs._bMultiSelect;
	MetaDataValue = InArgs._MetaDataValue;
	OnRemoveMetaData = InArgs._OnRemoveMetaData;
	OnSetMetaData = InArgs._OnSetMetaData;

	const TOptional<FString> ValueStringOptional = MetaDataValue.Get(TOptional<FString>());
	if (ValueStringOptional.IsSet() && !ValueStringOptional.GetValue().IsEmpty())
	{
		TArray<FString> TagStrings;
		ValueStringOptional.GetValue().ParseIntoArray(TagStrings, TEXT(","));

		for (const FString& TagString : TagStrings)
		{
			const FGameplayTag FoundTag = FGameplayTag::RequestGameplayTag(*TagString);
			if (FoundTag.IsValid())
			{
				GameplayTagContainer.AddTag(FoundTag);
			}
		}
	}

	ChildSlot
	[
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 3
		!bIsMulti
			? TSharedRef<SWidget>(SNew(SGameplayTagCombo)
				.Tag(GameplayTagContainer.First())
				.OnTagChanged(this, &SMDMetaDataGameplayTagPicker::UpdateMetaDataTag))
			:
#endif
		SNew(SComboButton)
		.ToolTipText(this, &SMDMetaDataGameplayTagPicker::GetValueToolTip)
		.OnMenuOpenChanged(this, &SMDMetaDataGameplayTagPicker::UpdateMetaData)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &SMDMetaDataGameplayTagPicker::GetValue)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.MenuContent()
		[
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 3
			SNew(SGameplayTagPicker)
				.ShowMenuItems(true)
				.TagContainers({ GameplayTagContainer })
				.OnTagChanged(this, &SMDMetaDataGameplayTagPicker::UpdateMetaDataContainer)
#else
			SNew(SGameplayTagWidget, TArray<SGameplayTagWidget::FEditableGameplayTagContainerDatum>{ { nullptr, &GameplayTagContainer } })
			.MultiSelect(bIsMulti)
			.OnTagChanged(this, &SMDMetaDataGameplayTagPicker::UpdateMetaData, bIsMulti)
#endif
		]
	];
}

FText SMDMetaDataGameplayTagPicker::GetValue() const
{
	const TOptional<FString> ValueStringOptional = MetaDataValue.Get(TOptional<FString>());
	if (!ValueStringOptional.IsSet() || ValueStringOptional.GetValue().IsEmpty())
	{
		return INVTEXT("Empty");
	}

	return FText::FromString(ValueStringOptional.GetValue().Replace(TEXT(","), TEXT(", ")));
}

FText SMDMetaDataGameplayTagPicker::GetValueToolTip() const
{
	const TOptional<FString> ValueStringOptional = MetaDataValue.Get(TOptional<FString>());
	if (!ValueStringOptional.IsSet() || ValueStringOptional.GetValue().IsEmpty())
	{
		return FText::GetEmpty();
	}

	return FText::FromString(ValueStringOptional.GetValue().Replace(TEXT(","), TEXT("\r\n")));
}

void SMDMetaDataGameplayTagPicker::UpdateMetaDataContainer(const TArray<FGameplayTagContainer>& Containers)
{
	GameplayTagContainer = !Containers.IsEmpty() ? Containers[0] : FGameplayTagContainer{};
}

void SMDMetaDataGameplayTagPicker::UpdateMetaDataTag(const FGameplayTag InTag)
{
	GameplayTagContainer.Reset(1);
	if (InTag.IsValid())
	{
		GameplayTagContainer.AddTag(InTag);
	}

	UpdateMetaData(false);
}

void SMDMetaDataGameplayTagPicker::UpdateMetaData(bool bDontUpdate)
{
	if (bDontUpdate)
	{
		return;
	}

	if (GameplayTagContainer.IsEmpty())
	{
		OnRemoveMetaData.ExecuteIfBound(Key);
	}
	else
	{
		TArray<FGameplayTag> GameplayTagArray;
		GameplayTagContainer.GetGameplayTagArray(GameplayTagArray);

		FString Value;
		for (const FGameplayTag& TagIt : GameplayTagArray)
		{
			if (!Value.IsEmpty())
			{
				Value += TEXT(",");
			}

			Value += TagIt.ToString();
		}

		OnSetMetaData.ExecuteIfBound(Key, Value);
	}
}
