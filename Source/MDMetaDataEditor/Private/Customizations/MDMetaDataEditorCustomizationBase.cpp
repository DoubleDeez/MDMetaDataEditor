// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorCustomizationBase.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailGroup.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Tunnel.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SMDMetaDataGameplayTagPicker.h"
#include "Widgets/SMDMetaDataStringComboBox.h"
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

	UK2Node_FunctionEntry* FindFunctionNode(const UBlueprint* Blueprint, const UFunction* Function)
	{
		if (!IsValid(Blueprint) || !IsValid(Function))
		{
			return nullptr;
		}

		for (UEdGraph* Graph : Blueprint->FunctionGraphs)
		{
			UK2Node_FunctionEntry* FunctionEntry = FindNode<UK2Node_FunctionEntry, false>(Graph);
			if (IsValid(FunctionEntry) && FFunctionFromNodeHelper::FunctionFromNode(FunctionEntry) == Function)
			{
				return FunctionEntry;
			}
		}

		return nullptr;
	}
}

FMDMetaDataEditorCustomizationBase::FMDMetaDataEditorCustomizationBase(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
	: BlueprintEditor(BlueprintEditor)
	, BlueprintPtr(MoveTemp(BlueprintPtr))
{}

void FMDMetaDataEditorCustomizationBase::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	PropertyBeingCustomized.Reset();
	FunctionBeingCustomized.Reset();
	TunnelBeingCustomized.Reset();
	EventBeingCustomized.Reset();

	UBlueprint* Blueprint = BlueprintPtr.Get();
	if (!IsValid(Blueprint))
	{
		return;
	}

	bool bIsReadOnly = false;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	bool bIsFunction = false;
	bool bIsProperty = false;

	UObject* Obj = ObjectsBeingCustomized[0].Get();
	UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(Obj);
	if (FProperty* Prop = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr)
	{
		if (Prop->IsNative() || Prop->GetOwnerUObject() == nullptr)
		{
			return;
		}

		bIsReadOnly |= !(Prop->GetOwnerUObject()->IsA<UFunction>()) && !FBlueprintEditorUtils::IsVariableCreatedByBlueprint(Blueprint, Prop);
		PropertyBeingCustomized = Prop;
		bIsProperty = true;
	}
	else if (UK2Node_FunctionEntry* Function = MDMDECB_Private::FindNode<UK2Node_FunctionEntry, false>(Obj))
	{
		FunctionBeingCustomized = Function;
		bIsFunction = true;
	}
	else if (UK2Node_Tunnel* Tunnel = MDMDECB_Private::FindNode<UK2Node_Tunnel, true>(Obj))
	{
		TunnelBeingCustomized = Tunnel;
		bIsFunction = true;
	}
	else if (UK2Node_CustomEvent* Event = MDMDECB_Private::FindNode<UK2Node_CustomEvent, false>(Obj))
	{
		EventBeingCustomized = Event;
		bIsFunction = true;
	}

	if (bIsProperty)
	{
		// Put Meta Data above Default Value for Variables
		const int32 MetaDataSortOrder = DetailLayout.EditCategory("Variable").GetSortOrder() + 1;
		DetailLayout.EditCategory("MetaData").SetSortOrder(MetaDataSortOrder);
		DetailLayout.EditCategory("DefaultValue").SetSortOrder(MetaDataSortOrder + 1);
	}
	else if (bIsFunction)
	{
		// Put Meta Data above Inputs for Functions
		const int32 MetaDataSortOrder = DetailLayout.EditCategory("Graph").GetSortOrder() + 1;
		DetailLayout.EditCategory("MetaData").SetSortOrder(MetaDataSortOrder);
		DetailLayout.EditCategory("Inputs").SetSortOrder(MetaDataSortOrder + 1);
	}

	TMap<FName, IDetailGroup*> GroupMap;

	auto AddMetaDataKey = [this, &DetailLayout, &GroupMap, bIsReadOnly](const FMDMetaDataKey& Key)
	{
		if (Key.RequiredMetaData != Key.Key && !Key.RequiredMetaData.IsNone() && !HasMetaDataValue(Key.RequiredMetaData))
		{
			return;
		}

		for (const FName& IncompatibleKey : Key.IncompatibleMetaData)
		{
			if (IncompatibleKey != Key.Key && !IncompatibleKey.IsNone() && HasMetaDataValue(IncompatibleKey))
			{
				return;
			}
		}

		IDetailCategoryBuilder& Category = DetailLayout.EditCategory("MetaData");
		IDetailGroup* Group = nullptr;

		FName GroupName = NAME_None;
		TArray<FString> Subgroups;
		Key.Category.ParseIntoArray(Subgroups, TEXT("|"));
		for (const FString& Subgroup : Subgroups)
		{
			const FText DisplayName = FText::FromString(Subgroup);
			if (DisplayName.IsEmptyOrWhitespace())
			{
				continue;
			}

			GroupName = (GroupName.IsNone()) ? *Subgroup : *FString::Printf(TEXT("%s|%s"), *GroupName.ToString(), *Subgroup);

			if (GroupMap.Contains(GroupName))
			{
				Group = GroupMap.FindRef(GroupName);
			}
			else if (Group != nullptr)
			{
				Group = &Group->AddGroup(GroupName, DisplayName, true);
				GroupMap.Add(GroupName, Group);
			}
			else
			{
				Group = &Category.AddGroup(GroupName, DisplayName, false, true);
				GroupMap.Add(GroupName, Group);
			}
		}

		FDetailWidgetRow& MetaDataRow = (Group != nullptr)
			? Group->AddWidgetRow().FilterString(Key.GetFilterText())
			: Category.AddCustomRow(Key.GetFilterText());

		const FUIAction CopyAction = {
			FExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CopyMetaData, Key.Key),
			FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CanCopyMetaData, Key.Key)
		};

		const FUIAction PasteAction = {
			FExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::PasteMetaData, Key.Key),
			FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CanPasteMetaData, Key.Key)
		};

		MetaDataRow
			.CopyAction(CopyAction)
			.PasteAction(PasteAction)
			.IsValueEnabled(!bIsReadOnly)
			.NameContent()
			[
				SNew(STextBlock)
				.Font(DetailLayout.GetDetailFont())
				.Text(Key.GetKeyDisplayText())
				.ToolTipText(Key.GetToolTipText())
			]
			.ValueContent()
			[
				CreateMetaDataValueWidget(Key)
			]
			.ExtensionContent()
			[
				SNew(SButton)
				.IsFocusable(false)
				.ToolTipText(INVTEXT("Remove this meta data from this property"))
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(0)
				.Visibility(this, &FMDMetaDataEditorCustomizationBase::GetRemoveMetaDataButtonVisibility, Key.Key)
				.OnClicked(this, &FMDMetaDataEditorCustomizationBase::OnRemoveMetaData, Key.Key)
				.Content()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.X"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];
	};

	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();
	if (bIsProperty)
	{
		Config->ForEachVariableMetaDataKey(Blueprint, PropertyBeingCustomized.Get(), AddMetaDataKey);
	}
	else if (bIsFunction)
	{
		Config->ForEachFunctionMetaDataKey(Blueprint, AddMetaDataKey);
	}
}

EVisibility FMDMetaDataEditorCustomizationBase::GetRemoveMetaDataButtonVisibility(FName Key) const
{
	return HasMetaDataValue(Key) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply FMDMetaDataEditorCustomizationBase::OnRemoveMetaData(FName Key)
{
	RemoveMetaDataKey(Key);

	return FReply::Handled();
}

TSharedRef<SWidget> FMDMetaDataEditorCustomizationBase::CreateMetaDataValueWidget(const FMDMetaDataKey& Key)
{
	if (Key.KeyType == EMDMetaDataEditorKeyType::Flag)
	{
		return SNew(SCheckBox)
			.ToolTipText(this, &FMDMetaDataEditorCustomizationBase::GetCheckBoxToolTip<false>, Key.Key)
			.IsChecked(this, &FMDMetaDataEditorCustomizationBase::IsChecked<false>, Key.Key)
			.OnCheckStateChanged(this, &FMDMetaDataEditorCustomizationBase::HandleChecked<false>, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Boolean)
	{
		return SNew(SCheckBox)
			.ToolTipText(this, &FMDMetaDataEditorCustomizationBase::GetCheckBoxToolTip<true>, Key.Key)
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
		// SNumericEntryBox doesn't show a background unless it has a value, so display a non-interactive one
		return SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SEditableTextBox)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Visibility(EVisibility::HitTestInvisible)
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SNumericEntryBox<int32>)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.AllowSpin(Key.bAllowSlider)
				.MinValue(Key.MinInt)
				.MaxValue(Key.MaxInt)
				.MinSliderValue(Key.MinSliderInt)
				.MaxSliderValue(Key.MaxSliderInt)
				.UndeterminedString(INVTEXT("-"))
				.Value(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueInt, Key.Key)
				.OnValueCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueIntCommitted, Key.Key)
			];
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Float)
	{
		// SNumericEntryBox doesn't show a background unless it has a value, so display a non-interactive one
		return SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SEditableTextBox)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Visibility(EVisibility::HitTestInvisible)
			]
			+SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SNumericEntryBox<float>)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.AllowSpin(Key.bAllowSlider)
				.MinValue(Key.MinFloat)
				.MaxValue(Key.MaxFloat)
				.MinSliderValue(Key.MinSliderFloat)
				.MaxSliderValue(Key.MaxSliderFloat)
				.UndeterminedString(INVTEXT("-"))
				.Value(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueFloat, Key.Key)
				.OnValueCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueFloatCommitted, Key.Key)
			];
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
	else if (Key.KeyType == EMDMetaDataEditorKeyType::ValueList)
	{
		return SNew(SMDMetaDataStringComboBox)
			.Key(Key.Key)
			.ValueList(Key.ValueList)
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
	TOptional<FString> CurrentValue = GetMetaDataValue(Key);
	if (CurrentValue.IsSet() && CurrentValue->Equals(Value))
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Set Meta Data [{0}={1}]"), FText::FromName(Key), FText::FromString(Value)));

	if (FProperty* Property = PropertyBeingCustomized.Get())
	{
		if (UBlueprint* Blueprint = BlueprintPtr.Get())
		{
			bool bDidFindMetaData = false;

			for (FBPVariableDescription& VariableDescription : Blueprint->NewVariables)
			{
				if (VariableDescription.VarName == Property->GetFName())
				{
					Blueprint->Modify();
					Property->SetMetaData(Key, FString(Value));
					VariableDescription.SetMetaData(Key, Value);
					bDidFindMetaData = true;
				}
			}

			if (!bDidFindMetaData)
			{
				// Is it a local variable?
				if (UK2Node_FunctionEntry* FuncNode = MDMDECB_Private::FindFunctionNode(Blueprint, Cast<UFunction>(Property->GetOwnerUObject())))
				{
					for (FBPVariableDescription& VariableDescription : FuncNode->LocalVariables)
					{
						if (VariableDescription.VarName == Property->GetFName())
						{
							FuncNode->Modify();
							Property->SetMetaData(Key, FString(Value));
							VariableDescription.SetMetaData(Key, Value);
						}
					}
				}
			}
		}
	}

	FKismetUserDeclaredFunctionMetadata* MetaData = nullptr;

	if (UK2Node_FunctionEntry* FuncNode = FunctionBeingCustomized.Get())
	{
		FuncNode->Modify();
		MetaData = &(FuncNode->MetaData);
	}
	else if (UK2Node_Tunnel* TunnelNode = TunnelBeingCustomized.Get())
	{
		TunnelNode->Modify();
		MetaData = &(TunnelNode->MetaData);
	}
	else if (UK2Node_CustomEvent* EventNode = EventBeingCustomized.Get())
	{
		EventNode->Modify();
		MetaData = &(EventNode->GetUserDefinedMetaData());
	}

	if (MetaData != nullptr)
	{
		MetaData->SetMetaData(Key, FString(Value));
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
}

bool FMDMetaDataEditorCustomizationBase::HasMetaDataValue(const FName& Key) const
{
	return GetMetaDataValue(Key).IsSet();
}

TOptional<FString> FMDMetaDataEditorCustomizationBase::GetMetaDataValue(FName Key) const
{
	TOptional<FString> Value;

	if (const FProperty* Prop = PropertyBeingCustomized.Get())
	{
		if (Prop->HasMetaData(Key))
		{
			if (!Value.IsSet())
			{
				Value = Prop->GetMetaData(Key);
			}
			else if (Value.GetValue() != Prop->GetMetaData(Key))
			{
				return MDMDECB_Private::MultipleValues;
			}
		}
	}

	const FKismetUserDeclaredFunctionMetadata* MetaData = nullptr;

	if (const UK2Node_FunctionEntry* FuncNode = Cast<UK2Node_FunctionEntry>(FunctionBeingCustomized.Get()))
	{
		MetaData = &(FuncNode->MetaData);
	}
	else if (const UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(TunnelBeingCustomized.Get()))
	{
		MetaData = &(TunnelNode->MetaData);
	}
	else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(EventBeingCustomized.Get()))
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
			return MDMDECB_Private::MultipleValues;
		}
	}

	return Value;
}

void FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey(const FName& Key)
{
	if (!HasMetaDataValue(Key))
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Removed Meta Data [{0}]"), FText::FromName(Key)));
	if (FProperty* Property = PropertyBeingCustomized.Get())
	{
		if (UBlueprint* Blueprint = BlueprintPtr.Get())
		{
			bool bDidFindMetaData = false;

			for (FBPVariableDescription& VariableDescription : Blueprint->NewVariables)
			{
				if (VariableDescription.VarName == Property->GetFName())
				{
					Blueprint->Modify();
					Property->RemoveMetaData(Key);
					VariableDescription.RemoveMetaData(Key);
					bDidFindMetaData = false;
				}
			}

			if (!bDidFindMetaData)
			{
				// Is it a local variable?
				if (UK2Node_FunctionEntry* FuncNode = MDMDECB_Private::FindFunctionNode(Blueprint, Cast<UFunction>(Property->GetOwnerUObject())))
				{
					for (FBPVariableDescription& VariableDescription : FuncNode->LocalVariables)
					{
						if (VariableDescription.VarName == Property->GetFName())
						{
							FuncNode->Modify();
							Property->RemoveMetaData(Key);
							VariableDescription.RemoveMetaData(Key);
						}
					}
				}
			}
		}
	}

	FKismetUserDeclaredFunctionMetadata* MetaData = nullptr;

	if (UK2Node_FunctionEntry* FuncNode = Cast<UK2Node_FunctionEntry>(FunctionBeingCustomized.Get()))
	{
		MetaData = &(FuncNode->MetaData);
	}
	else if (UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(TunnelBeingCustomized.Get()))
	{
		MetaData = &(TunnelNode->MetaData);
	}
	else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(EventBeingCustomized.Get()))
	{
		MetaData = &(EventNode->GetUserDefinedMetaData());
	}

	if (MetaData != nullptr)
	{
		MetaData->RemoveMetaData(Key);
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
}

void FMDMetaDataEditorCustomizationBase::CopyMetaData(FName Key) const
{
	const TOptional<FString> Value = GetMetaDataValue(Key);

	// Copy in Key=Value format
	FPlatformApplicationMisc::ClipboardCopy(*FString::Printf(TEXT("%s=\"%s\""), *Key.ToString(), *Value.Get(TEXT("")).ReplaceCharWithEscapedChar()));
}

bool FMDMetaDataEditorCustomizationBase::CanCopyMetaData(FName Key) const
{
	return HasMetaDataValue(Key);
}

void FMDMetaDataEditorCustomizationBase::PasteMetaData(FName Key)
{
	FString Clipboard;
	FPlatformApplicationMisc::ClipboardPaste(Clipboard);
	Clipboard.TrimStartAndEndInline();

	// Handle pasting the same metadata format used in C++
	int32 EqualIndex = INDEX_NONE;
	if (Clipboard.FindChar(TEXT('='), EqualIndex) && EqualIndex > 0)
	{
		// Skip if the clipboard has != or == without starting with "Key=" (like a raw edit condition) but ending with = means a blank value
		if (Clipboard[EqualIndex - 1] != TEXT('!') && (EqualIndex == (Clipboard.Len() - 1) || Clipboard[EqualIndex + 1] != TEXT('=')))
		{
			// Allow paste if the key matches
			const FString ClipKey = Clipboard.Left(EqualIndex).TrimStartAndEnd();
			if (*ClipKey != Key)
			{
				return;
			}

			const FString ClipValue = Clipboard.Mid(EqualIndex + 1).TrimStartAndEnd().TrimQuotes();
			SetMetaDataValue(Key, Clipboard.ReplaceEscapedCharWithChar());
		}
	}

	// Clipboard is just the value
	if (!Clipboard.IsEmpty())
	{
		Clipboard.TrimQuotesInline();
		SetMetaDataValue(Key, Clipboard.ReplaceEscapedCharWithChar());
	}
}

bool FMDMetaDataEditorCustomizationBase::CanPasteMetaData(FName Key) const
{
	FString Clipboard;
	FPlatformApplicationMisc::ClipboardPaste(Clipboard);
	Clipboard.TrimStartAndEndInline();

	// Handle pasting the same metadata format used in C++
	int32 EqualIndex = INDEX_NONE;
	if (Clipboard.FindChar(TEXT('='), EqualIndex) && EqualIndex > 0)
	{
		// Skip if the clipboard has != or == without starting with "Key=" (like a raw edit condition) but ending with = means a blank value
		if (Clipboard[EqualIndex - 1] != TEXT('!') && (EqualIndex == (Clipboard.Len() - 1) || Clipboard[EqualIndex + 1] != TEXT('=')))
		{
			// Allow paste if the key matches
			Clipboard.LeftInline(EqualIndex);
			Clipboard.TrimStartAndEndInline();
			return *Clipboard == Key;
		}
	}

	// Clipboard is just the value
	return !Clipboard.IsEmpty();
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
		// Don't assume unset == false, a meta data key could have different behaviour between the 2.
		if (!Value.IsSet())
		{
			return ECheckBoxState::Undetermined;
		}

		return Value.GetValue().ToBool() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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

template <bool bIsBoolean>
FText FMDMetaDataEditorCustomizationBase::GetCheckBoxToolTip(FName Key) const
{
	switch (IsChecked<bIsBoolean>(Key))
	{
	case ECheckBoxState::Unchecked:
		return bIsBoolean ? INVTEXT("False") : INVTEXT("Unset");
	case ECheckBoxState::Checked:
		return bIsBoolean ? INVTEXT("True") : INVTEXT("Set");
	case ECheckBoxState::Undetermined:
		return INVTEXT("Unset");
	default:
		return INVTEXT("Unhandled State");
	}
}
