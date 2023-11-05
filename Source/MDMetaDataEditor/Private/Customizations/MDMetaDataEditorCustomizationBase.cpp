// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorCustomizationBase.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagContainer.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Tunnel.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 3
#include "SGameplayTagCombo.h"
#include "SGameplayTagPicker.h"
#else
#include "SGameplayTagWidget.h"
#endif
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Text/STextBlock.h"

namespace MDMDECB_Private
{
	const FString MultipleValues = TEXT("Multiple Values");

	template<typename T, bool bExact>
	T* FindNode(UObject* Object)
	{
		if (T* Node = Cast<T>(Object))
		{
			return Node;
		}
		if (UEdGraph* Graph = Cast<UEdGraph>(Object))
		{
			for (UEdGraphNode* GraphNode : Graph->Nodes)
			{
				if constexpr (bExact)
				{
					if (T* Node = ExactCast<T>(GraphNode))
					{
						return Node;
					}
				}
				else
				{
					if (T* Node = Cast<T>(GraphNode))
					{
						return Node;
					}
				}
			}
		}
		return nullptr;
	}
}

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

	void Construct(const FArguments& InArgs)
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

private:
	FText GetValue() const
	{
		const TOptional<FString> ValueStringOptional = MetaDataValue.Get(TOptional<FString>());
		if (!ValueStringOptional.IsSet() || ValueStringOptional.GetValue().IsEmpty())
		{
			return INVTEXT("Empty");
		}

		return FText::FromString(ValueStringOptional.GetValue().Replace(TEXT(","), TEXT(", ")));
	}

	FText GetValueToolTip() const
	{
		const TOptional<FString> ValueStringOptional = MetaDataValue.Get(TOptional<FString>());
		if (!ValueStringOptional.IsSet() || ValueStringOptional.GetValue().IsEmpty())
		{
			return FText::GetEmpty();
		}

		return FText::FromString(ValueStringOptional.GetValue().Replace(TEXT(","), TEXT("\r\n")));
	}

	void UpdateMetaDataContainer(const TArray<FGameplayTagContainer>& Containers)
	{
		GameplayTagContainer = !Containers.IsEmpty() ? Containers[0] : FGameplayTagContainer{};
	}

	void UpdateMetaDataTag(const FGameplayTag InTag)
	{
		GameplayTagContainer.Reset(1);
		if (InTag.IsValid())
		{
			GameplayTagContainer.AddTag(InTag);
		}

		UpdateMetaData(false);
	}

	void UpdateMetaData(bool bDontUpdate)
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

	FName Key = NAME_None;
	bool bIsMulti = false;
	FGameplayTagContainer GameplayTagContainer;
	TAttribute<TOptional<FString>> MetaDataValue;
	FOnRemoveMetaData OnRemoveMetaData;
	FOnSetMetaData OnSetMetaData;
};

FMDMetaDataEditorCustomizationBase::FMDMetaDataEditorCustomizationBase(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
	: BlueprintEditor(BlueprintEditor)
	, BlueprintPtr(MoveTemp(BlueprintPtr))
{}

void FMDMetaDataEditorCustomizationBase::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	PropertiesBeingCustomized.Reset();
	FunctionsBeingCustomized.Reset();

	UBlueprint* Blueprint = BlueprintPtr.Get();
	if (!IsValid(Blueprint))
	{
		return;
	}

	bool bIsReadOnly = false;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() > 0)
	{
		for (TWeakObjectPtr<UObject>& Obj : ObjectsBeingCustomized)
		{
			UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(Obj.Get());
			if (FProperty* PropertyBeingCustomized = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr)
			{
				if (PropertyBeingCustomized->IsNative())
				{
					continue;
				}

				bIsReadOnly |= !FBlueprintEditorUtils::IsVariableCreatedByBlueprint(Blueprint, PropertyBeingCustomized);
				PropertiesBeingCustomized.Emplace(PropertyBeingCustomized);
			}
			else if (UK2Node_FunctionEntry* Function = MDMDECB_Private::FindNode<UK2Node_FunctionEntry, false>(Obj.Get()))
			{
				FunctionsBeingCustomized.Emplace(Function);
			}
			else if (UK2Node_Tunnel* Tunnel = MDMDECB_Private::FindNode<UK2Node_Tunnel, true>(Obj.Get()))
			{
				TunnelsBeingCustomized.Emplace(Tunnel);
			}
			else if (UK2Node_CustomEvent* Event = MDMDECB_Private::FindNode<UK2Node_CustomEvent, false>(Obj.Get()))
			{
				EventsBeingCustomized.Emplace(Event);
			}
		}
	}

	// Populate the set of meta data keys to display, only display the keys in-common with all the selected fields
	TSet<FMDMetaDataKey> MetaDataKeys;
	{
		const UMDMetaDataEditorConfig* MetaDataConfig = GetDefault<UMDMetaDataEditorConfig>();

		for (const TWeakFieldPtr<FProperty>& PropertyPtr : PropertiesBeingCustomized)
		{
			TSet<FMDMetaDataKey> PropMetaDataKeys;

			MetaDataConfig->ForEachVariableMetaDataKey(Blueprint, PropertyPtr.Get(), [&PropMetaDataKeys](const FMDMetaDataKey& Key)
			{
				PropMetaDataKeys.Add(Key);
			});

			if (MetaDataKeys.IsEmpty())
			{
				MetaDataKeys = PropMetaDataKeys;
			}
			else
			{
				MetaDataKeys = MetaDataKeys.Intersect(PropMetaDataKeys);
			}
		}

		auto GatherFunctionKeys = [&MetaDataKeys, Blueprint, MetaDataConfig](const auto& NodeArray)
		{
			for (const auto& NodePtr : NodeArray)
			{
				TSet<FMDMetaDataKey> FuncMetaDataKeys;

				MetaDataConfig->ForEachFunctionMetaDataKey(Blueprint, [&FuncMetaDataKeys](const FMDMetaDataKey& Key)
				{
					FuncMetaDataKeys.Add(Key);
				});

				if (MetaDataKeys.IsEmpty())
				{
					MetaDataKeys = FuncMetaDataKeys;
				}
				else
				{
					MetaDataKeys = MetaDataKeys.Intersect(FuncMetaDataKeys);
				}
			}
		};

		GatherFunctionKeys(FunctionsBeingCustomized);
		GatherFunctionKeys(TunnelsBeingCustomized);
		GatherFunctionKeys(EventsBeingCustomized);
	}

	const int32 MetaDataSortOrder = DetailLayout.EditCategory("Variable").GetSortOrder() + 1;
	DetailLayout.EditCategory("MetaData").SetSortOrder(MetaDataSortOrder);
	DetailLayout.EditCategory("DefaultValue").SetSortOrder(MetaDataSortOrder + 1);

	TArray<FMDMetaDataKey> MetaDataKeysSorted = MetaDataKeys.Array();
	MetaDataKeysSorted.Sort([](const FMDMetaDataKey& A, const FMDMetaDataKey& B)
	{
		return A.Key.Compare(B.Key) < 0;
	});

	for (const FMDMetaDataKey& Key : MetaDataKeysSorted)
	{
		DetailLayout.EditCategory("MetaData")
			.AddCustomRow(FText::FromName(Key.Key))
			.IsValueEnabled(!bIsReadOnly)
			.NameContent()
			[
				SNew(STextBlock)
				.Font(DetailLayout.GetDetailFont())
				.Text(FText::FromName(Key.Key))
				.ToolTipText(FText::FromString(Key.Description))
			]
			.ValueContent()
			[
				CreateMetaDataValueWidget(Key)
			];
	}
}

TSharedRef<SWidget> FMDMetaDataEditorCustomizationBase::CreateMetaDataValueWidget(const FMDMetaDataKey& Key)
{
	if (Key.KeyType == EMDMetaDataEditorKeyType::Flag)
	{
		return SNew(SCheckBox)
			.IsChecked(this, &FMDMetaDataEditorCustomizationBase::IsChecked<false>, Key.Key)
			.OnCheckStateChanged(this, &FMDMetaDataEditorCustomizationBase::HandleChecked<false>, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Boolean)
	{
		return SNew(SCheckBox)
			.IsChecked(this, &FMDMetaDataEditorCustomizationBase::IsChecked<true>, Key.Key)
			.OnCheckStateChanged(this, &FMDMetaDataEditorCustomizationBase::HandleChecked<true>, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::String)
	{
		return SNew(SEditableTextBox)
			.Text(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueText, Key.Key)
			.OnTextCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueTextCommitted, Key.Key)
			.RevertTextOnEscape(true);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Integer)
	{
		return SNew(SNumericEntryBox<int32>)
			.AllowSpin(Key.bAllowSlider)
			.MinValue(Key.MinInt)
			.MaxValue(Key.MaxInt)
			.MinSliderValue(Key.MinSliderInt)
			.MaxSliderValue(Key.MaxSliderInt)
			.Value(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueInt, Key.Key)
			.OnValueCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueIntCommitted, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Float)
	{
		return SNew(SNumericEntryBox<float>)
			.AllowSpin(Key.bAllowSlider)
			.MinValue(Key.MinFloat)
			.MaxValue(Key.MaxFloat)
			.MinSliderValue(Key.MinSliderFloat)
			.MaxSliderValue(Key.MaxSliderFloat)
			.Value(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueFloat, Key.Key)
			.OnValueCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueFloatCommitted, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::GameplayTag)
	{
		return SNew(SMDMetaDataGameplayTagPicker)
			.Key(Key.Key)
			.bMultiSelect(false)
			.OnRemoveMetaData(this, &FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey)
			.OnSetMetaData(this, &FMDMetaDataEditorCustomizationBase::SetMetaDataValue)
			.MetaDataValue(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValue, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::GameplayTagContainer)
	{
		return SNew(SMDMetaDataGameplayTagPicker)
			.Key(Key.Key)
			.bMultiSelect(true)
			.OnRemoveMetaData(this, &FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey)
			.OnSetMetaData(this, &FMDMetaDataEditorCustomizationBase::SetMetaDataValue)
			.MetaDataValue(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValue, Key.Key);
	}

	return SNullWidget::NullWidget;
}

void FMDMetaDataEditorCustomizationBase::AddMetaDataKey(const FName& Key)
{
	SetMetaDataValue(Key, TEXT(""));
}

void FMDMetaDataEditorCustomizationBase::SetMetaDataValue(const FName& Key, const FString& Value)
{
	if (UBlueprint* Blueprint = BlueprintPtr.Get())
	{
		for (TWeakFieldPtr<FProperty>& WeakProperty : PropertiesBeingCustomized)
		{
			if (FProperty* Property = WeakProperty.Get())
			{
				for (FBPVariableDescription& VariableDescription : Blueprint->NewVariables)
				{
					if (VariableDescription.VarName == Property->GetFName())
					{
						Property->SetMetaData(Key, FString(Value));
						VariableDescription.SetMetaData(Key, Value);
					}
				}
			}
		}
	}

	auto UpdateMetaData = [&Value, Key](const auto& NodeArray)
	{
		for (const auto& NodePtr : NodeArray)
		{
			FKismetUserDeclaredFunctionMetadata* MetaData = nullptr;

			UK2Node_EditablePinBase* FunctionEntryNode = NodePtr.Get();
			if (UK2Node_FunctionEntry* TypedEntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNode))
			{
				MetaData = &(TypedEntryNode->MetaData);
			}
			else if (UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(FunctionEntryNode))
			{
				MetaData = &(TunnelNode->MetaData);
			}
			else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(FunctionEntryNode))
			{
				MetaData = &(EventNode->GetUserDefinedMetaData());
			}

			if (MetaData != nullptr)
			{
				MetaData->SetMetaData(Key, FString(Value));
			}
		}
	};

	UpdateMetaData(FunctionsBeingCustomized);
	UpdateMetaData(TunnelsBeingCustomized);
	UpdateMetaData(EventsBeingCustomized);

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
}

bool FMDMetaDataEditorCustomizationBase::HasMetaDataValue(const FName& Key) const
{
	return GetMetaDataValue(Key).IsSet();
}

TOptional<FString> FMDMetaDataEditorCustomizationBase::GetMetaDataValue(FName Key) const
{
	TOptional<FString> Value;

	for (const TWeakFieldPtr<FProperty>& PropertyPtr : PropertiesBeingCustomized)
	{
		if (const FProperty* Prop = PropertyPtr.Get())
		{
			if (Prop->HasMetaData(Key))
			{
				if (!Value.IsSet())
				{
					Value = Prop->GetMetaData(Key);
				}
				else if (Value.GetValue() != Prop->GetMetaData(Key))
				{
					Value = MDMDECB_Private::MultipleValues;
					return Value;
				}
			}
		}
	}

	auto GetMetaDataValue = [&Value, Key](const auto& NodeArray)
	{
		for (const auto& NodePtr : NodeArray)
		{
			const FKismetUserDeclaredFunctionMetadata* MetaData = nullptr;

			UK2Node_EditablePinBase* FunctionEntryNode = NodePtr.Get();
			if (const UK2Node_FunctionEntry* TypedEntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNode))
			{
				MetaData = &(TypedEntryNode->MetaData);
			}
			else if (const UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(FunctionEntryNode))
			{
				MetaData = &(TunnelNode->MetaData);
			}
			else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(FunctionEntryNode))
			{
				MetaData = &(EventNode->GetUserDefinedMetaData());
			}

			if (MetaData != nullptr && MetaData->HasMetaData(Key))
			{
				if (!Value.IsSet())
				{
					Value = MetaData->GetMetaData(Key);
				}
				else if (Value.GetValue() != MetaData->GetMetaData(Key))
				{
					Value = MDMDECB_Private::MultipleValues;
					return;
				}
			}
		}
	};

	GetMetaDataValue(FunctionsBeingCustomized);
	GetMetaDataValue(TunnelsBeingCustomized);
	GetMetaDataValue(EventsBeingCustomized);

	return Value;
}

void FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey(const FName& Key)
{
	if (UBlueprint* Blueprint = BlueprintPtr.Get())
	{
		for (TWeakFieldPtr<FProperty>& WeakProperty : PropertiesBeingCustomized)
		{
			if (FProperty* Property = WeakProperty.Get())
			{
				for (FBPVariableDescription& VariableDescription : Blueprint->NewVariables)
				{
					if (VariableDescription.VarName == Property->GetFName())
					{
						Property->RemoveMetaData(Key);
						VariableDescription.RemoveMetaData(Key);
					}
				}
			}
		}
	}

	auto UpdateMetaData = [Key](const auto& NodeArray)
	{
		for (const auto& NodePtr : NodeArray)
		{
			FKismetUserDeclaredFunctionMetadata* MetaData = nullptr;

			UK2Node_EditablePinBase* FunctionEntryNode = NodePtr.Get();
			if (UK2Node_FunctionEntry* TypedEntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNode))
			{
				MetaData = &(TypedEntryNode->MetaData);
			}
			else if (UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(FunctionEntryNode))
			{
				MetaData = &(TunnelNode->MetaData);
			}
			else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(FunctionEntryNode))
			{
				MetaData = &(EventNode->GetUserDefinedMetaData());
			}

			if (MetaData != nullptr)
			{
				MetaData->RemoveMetaData(Key);
			}
		}
	};

	UpdateMetaData(FunctionsBeingCustomized);
	UpdateMetaData(TunnelsBeingCustomized);
	UpdateMetaData(EventsBeingCustomized);

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
}

FText FMDMetaDataEditorCustomizationBase::GetMetaDataValueText(FName Key) const
{
	return FText::FromString(GetMetaDataValue(Key).Get(TEXT("")));
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName Key)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		if (!NewText.IsEmptyOrWhitespace())
		{
			SetMetaDataValue(Key, NewText.ToString());
		}
		else
		{
			RemoveMetaDataKey(Key);
		}
	}
}

TOptional<int32> FMDMetaDataEditorCustomizationBase::GetMetaDataValueInt(FName Key) const
{
	TOptional<FString> Value = GetMetaDataValue(Key);
	return Value.IsSet() ? TOptional<int32>(FCString::Atoi(*Value.GetValue())) : TOptional<int32>{};
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueIntCommitted(int32 Value, ETextCommit::Type InTextCommit, FName Key)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		SetMetaDataValue(Key, FString::FromInt(Value));
	}
}

TOptional<float> FMDMetaDataEditorCustomizationBase::GetMetaDataValueFloat(FName Key) const
{
	TOptional<FString> Value = GetMetaDataValue(Key);
	return Value.IsSet() ? TOptional<float>(FCString::Atof(*Value.GetValue())) : TOptional<float>{};
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueFloatCommitted(float Value, ETextCommit::Type InTextCommit, FName Key)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		SetMetaDataValue(Key, FString::SanitizeFloat(Value));
	}
}

template <bool bIsBoolean>
ECheckBoxState FMDMetaDataEditorCustomizationBase::IsChecked(FName Key) const
{
	TOptional<FString> Value = GetMetaDataValue(Key);

	if (Value.IsSet() && Value.GetValue() == MDMDECB_Private::MultipleValues)
	{
		return ECheckBoxState::Undetermined;
	}

	if constexpr (bIsBoolean)
	{
		return (Value.IsSet() && Value.GetValue().ToBool()) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	else
	{
		return Value.IsSet() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
}

template <bool bIsBoolean>
void FMDMetaDataEditorCustomizationBase::HandleChecked(ECheckBoxState State, FName Key)
{
	if constexpr (bIsBoolean)
	{
		SetMetaDataValue(Key, (State == ECheckBoxState::Checked) ? TEXT("true") : TEXT("false"));
	}
	else
	{
		if (State == ECheckBoxState::Checked)
		{
			AddMetaDataKey(Key);
		}
		else
		{
			RemoveMetaDataKey(Key);
		}
	}
}
