// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorFieldView.h"

#include "BlueprintActionDatabase.h"
#include "Config/MDMetaDataEditorConfig.h"
#include "Config/MDMetaDataEditorUserConfig.h"
#include "Customizations/MDMetaDataEditorCustomizationBase.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/UserDefinedStruct.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailGroup.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Tunnel.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "SlateOptMacros.h"
#include "Styling/AppStyle.h"
#include "Types/MDMetaDataKey.h"
#include "UObject/MetaData.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SMDMetaDataGameplayTagPicker.h"
#include "Widgets/SMDMetaDataStringComboBox.h"
#include "Widgets/Text/STextBlock.h"

#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5) // On or after UE 5.5
#include "StructUtils/UserDefinedStruct.h"
#endif

namespace MDMDEFV_Private
{
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

const FString FMDMetaDataEditorFieldView::MultipleValues = TEXT("Multiple Values");

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(FProperty* InProperty, UBlueprint* InBlueprint)
	: MetadataProperty(InProperty)
	, BlueprintPtr(InBlueprint)
{
	if (InProperty != nullptr)
	{
		const bool bIsFunctionProperty = IsValid(InProperty->GetOwnerUObject()) && InProperty->GetOwnerUObject()->IsA<UFunction>();
		if (InProperty->IsNative())
		{
			// Can't modify the metadata of native properties
			bIsReadOnly = true;
		}
		else if (bIsFunctionProperty)
		{
			// Non-native function param metadata can be edited
			bIsReadOnly = false;
		}
		else
		{
			// Only metadata created in the current BP can be edited
			bIsReadOnly = !IsValid(InBlueprint) || !FBlueprintEditorUtils::IsVariableCreatedByBlueprint(InBlueprint, InProperty);
		}

		if (bIsFunctionProperty)
		{
			FieldType = EMDMetaDataEditorFieldType::LocalVariable;
		}
		else
		{
			FieldType = EMDMetaDataEditorFieldType::Variable;
		}
	}
}

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(FProperty* InProperty, UUserDefinedStruct* InUserDefinedStruct)
	: MetadataProperty(InProperty)
	, bIsReadOnly(false) // Should this ever be true?
	, FieldType(EMDMetaDataEditorFieldType::StructProperty)
{
}

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(FProperty* InProperty, UK2Node_EditablePinBase* InNode)
	: MetadataProperty(InProperty)
	, BlueprintPtr(IsValid(InNode) ? InNode->GetBlueprint() : nullptr)
{
	if (InProperty != nullptr)
	{
		if (InProperty->IsNative())
		{
			// Can't modify the metadata of native properties
			bIsReadOnly = true;
		}
		else
		{
			// Non-native function param metadata can be edited
			bIsReadOnly = false;
		}

		if (IsValid(InNode))
		{
			for (const TSharedPtr<FUserPinInfo>& PinInfo : InNode->UserDefinedPins)
			{
				if (PinInfo.IsValid() && PinInfo->PinName == InProperty->GetFName())
				{
					// This seems backwards, but inputs show up as outputs within the function graph
					FieldType = (PinInfo->DesiredPinDirection == EGPD_Output)
						? EMDMetaDataEditorFieldType::FunctionParamInput
						: EMDMetaDataEditorFieldType::FunctionParamOutput;
					break;
				}
			}
		}
	}
}

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(UUserDefinedStruct* InUserDefinedStruct)
	: MetadataStruct(InUserDefinedStruct)
	, bIsReadOnly(false) // Should this ever be true?
	, FieldType(EMDMetaDataEditorFieldType::Struct)
{
}

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(UK2Node_FunctionEntry* InFunctionEntry, UBlueprint* InBlueprint)
	: MetadataFunctionEntry(InFunctionEntry)
	, BlueprintPtr(InBlueprint)
	, bIsReadOnly(!IsValid(InFunctionEntry) || !InFunctionEntry->bIsEditable)
	, FieldType(EMDMetaDataEditorFieldType::Function)
{
}

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(UK2Node_Tunnel* InTunnel, UBlueprint* InBlueprint)
	: MetadataTunnel(InTunnel)
	, BlueprintPtr(InBlueprint)
	, bIsReadOnly(!IsValid(InTunnel) || !InTunnel->bIsEditable)
	, FieldType(EMDMetaDataEditorFieldType::Tunnel)
{
}

FMDMetaDataEditorFieldView::FMDMetaDataEditorFieldView(UK2Node_CustomEvent* InCustomEvent, UBlueprint* InBlueprint)
	: MetadataCustomEvent(InCustomEvent)
	, BlueprintPtr(InBlueprint)
	, bIsReadOnly(!IsValid(InCustomEvent) || !InCustomEvent->bIsEditable)
	, FieldType(EMDMetaDataEditorFieldType::CustomEvent)
{
}

const TMap<FName, FString>* FMDMetaDataEditorFieldView::GetMetadataMap() const
{
	if (const FProperty* Property = MetadataProperty.Get())
	{
		return Property->GetMetaDataMap();
	}
	if (const UK2Node_FunctionEntry* FuncNode = MetadataFunctionEntry.Get())
	{
		return &(FuncNode->MetaData.GetMetaDataMap());
	}
	else if (const UK2Node_Tunnel* TunnelNode = MetadataTunnel.Get())
	{
		return &(TunnelNode->MetaData.GetMetaDataMap());
	}
	else if (UK2Node_CustomEvent* EventNode = MetadataCustomEvent.Get())
	{
		return &(EventNode->GetUserDefinedMetaData().GetMetaDataMap());
	}
	else if (UUserDefinedStruct* Struct = MetadataStruct.Get())
	{
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6) // On or after UE 5.6
		return FMetaData::GetMapForObject(Struct);
#else // Pre UE 5.6
		return UMetaData::GetMapForObject(Struct);
#endif
	}

	return nullptr;
}

FKismetUserDeclaredFunctionMetadata* FMDMetaDataEditorFieldView::GetFunctionMetadataWithModify() const
{
	if (UK2Node_FunctionEntry* FuncNode = MetadataFunctionEntry.Get())
	{
		FuncNode->Modify();
		return &(FuncNode->MetaData);
	}
	else if (UK2Node_Tunnel* TunnelNode = MetadataTunnel.Get())
	{
		TunnelNode->Modify();
		return &(TunnelNode->MetaData);
	}
	else if (UK2Node_CustomEvent* EventNode = MetadataCustomEvent.Get())
	{
		EventNode->Modify();
		return &(EventNode->GetUserDefinedMetaData());
	}

	return nullptr;
}

void FMDMetaDataEditorFieldView::GenerateMetadataEditor(IDetailLayoutBuilder& DetailLayout, TMap<FName, IDetailGroup*>& GroupMap)
{
	if (!IsConfigEnabled())
	{
		return;
	}

	FMDMetadataBuilderRow BuilderRow = InitCategories(DetailLayout, GroupMap);

	auto AddMetaDataKey = [this, &GroupMap, &BuilderRow](const FMDMetaDataKey& Key)
	{
		AddMetadataValueEditor(Key, BuilderRow, GroupMap);
	};

	AddMetadataValueEditor(AddMetaDataKey);

	const UMDMetaDataEditorUserConfig* UserConfig = GetDefault<UMDMetaDataEditorUserConfig>();
	const TMap<FName, FString>* MetadataMap = GetMetadataMap();
	if (UserConfig->bEnableRawMetaDataEditor && (!bIsReadOnly || (MetadataMap != nullptr && !MetadataMap->IsEmpty())))
	{
		AddRawMetadataEditor(BuilderRow);
	}
}

TUnion<IDetailCategoryBuilder*, IDetailGroup*> FMDMetaDataEditorFieldView::InitCategories(IDetailLayoutBuilder& DetailLayout, TMap<FName, IDetailGroup*>& GroupMap)
{
	FMDMetadataBuilderRow Result;

	switch (FieldType) {
	case EMDMetaDataEditorFieldType::Variable:
	case EMDMetaDataEditorFieldType::LocalVariable:
	{
		// Put Metadata above Default Value for Variables
		int32 MetadataSortOrder = DetailLayout.EditCategory("Variable").GetSortOrder();
		IDetailCategoryBuilder& MetadataCategory = DetailLayout.EditCategory("Metadata");
		MetadataCategory.SetSortOrder(++MetadataSortOrder);
		DetailLayout.EditCategory("DefaultValue").SetSortOrder(++MetadataSortOrder);
		Result.SetSubtype<IDetailCategoryBuilder*>(&MetadataCategory);
		break;
	}
	case EMDMetaDataEditorFieldType::FunctionParamInput:
	case EMDMetaDataEditorFieldType::FunctionParamOutput:
	{
		if (MetadataProperty.IsValid())
		{
			const FName CategoryName =  (FieldType == EMDMetaDataEditorFieldType::FunctionParamInput)
				? TEXT("Inputs")
				: TEXT("Outputs");
			IDetailCategoryBuilder& Category = DetailLayout.EditCategory(CategoryName);

			const FName GroupName =  (FieldType == EMDMetaDataEditorFieldType::FunctionParamInput)
				? TEXT("Input Param Metadata")
				: TEXT("Output Param Metadata");
			IDetailGroup*& ParamGroup = GroupMap.FindOrAdd(GroupName);
			if (ParamGroup == nullptr)
			{
				ParamGroup = &Category.AddGroup(TEXT("Param Metadata"), INVTEXT("Param Metadata"));
			}
			IDetailGroup& Group = ParamGroup->AddGroup(MetadataProperty->GetFName(), MetadataProperty->GetDisplayNameText());
			Result.SetSubtype<IDetailGroup*>(&Group);
		}
		break;
	}
	case EMDMetaDataEditorFieldType::StructProperty:
	{
		if (MetadataProperty.IsValid())
		{
			IDetailGroup*& Group = GroupMap.FindOrAdd(MetadataProperty->GetFName());
			if (Group == nullptr)
			{
				Group = &DetailLayout.EditCategory("Property Metadata").AddGroup(MetadataProperty->GetFName(), MetadataProperty->GetDisplayNameText());
			}

			Result.SetSubtype<IDetailGroup*>(Group);
		}
		break;
	}
	case EMDMetaDataEditorFieldType::Function:
	case EMDMetaDataEditorFieldType::Tunnel:
	case EMDMetaDataEditorFieldType::CustomEvent:
	{
		int32 MetadataSortOrder = DetailLayout.EditCategory("Graph").GetSortOrder();
		IDetailCategoryBuilder& MetadataCategory = DetailLayout.EditCategory("Metadata");
		MetadataCategory.SetSortOrder(++MetadataSortOrder);
		DetailLayout.EditCategory("Inputs").SetSortOrder(++MetadataSortOrder);
		DetailLayout.EditCategory("Outputs").SetSortOrder(++MetadataSortOrder);
		Result.SetSubtype<IDetailCategoryBuilder*>(&MetadataCategory);
		break;
	}
	case EMDMetaDataEditorFieldType::Struct:
	{
		IDetailCategoryBuilder& MetadataCategory = DetailLayout.EditCategory("Struct Metadata");
		int32 MetadataSortOrder = MetadataCategory.GetSortOrder();
		DetailLayout.EditCategory("Property Metadata").SetSortOrder(++MetadataSortOrder);
		Result.SetSubtype<IDetailCategoryBuilder*>(&MetadataCategory);
		break;
	}
	}

	return Result;
}

void FMDMetaDataEditorFieldView::AddMetadataValueEditor(const TFunctionRef<void(const FMDMetaDataKey&)>& Func)
{
	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();
	switch (FieldType) {
	case EMDMetaDataEditorFieldType::Variable:
		Config->ForEachVariableMetaDataKey(BlueprintPtr.Get(), MetadataProperty.Get(), Func);
		break;
	case EMDMetaDataEditorFieldType::LocalVariable:
		Config->ForEachLocalVariableMetaDataKey(BlueprintPtr.Get(), MetadataProperty.Get(), Func);
		break;
	case EMDMetaDataEditorFieldType::FunctionParamInput:
	case EMDMetaDataEditorFieldType::FunctionParamOutput:
		Config->ForEachParameterMetaDataKey(BlueprintPtr.Get(), MetadataProperty.Get(), Func);
		break;
	case EMDMetaDataEditorFieldType::StructProperty:
		Config->ForEachStructPropertyMetaDataKey(MetadataProperty.Get(), Func);
		break;
	case EMDMetaDataEditorFieldType::Function:
	case EMDMetaDataEditorFieldType::Tunnel:
	case EMDMetaDataEditorFieldType::CustomEvent:
		Config->ForEachFunctionMetaDataKey(BlueprintPtr.Get(), Func);
		break;
	case EMDMetaDataEditorFieldType::Struct:
		Config->ForEachStructMetaDataKey(Func);
		break;
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FMDMetaDataEditorFieldView::AddMetadataValueEditor(const FMDMetaDataKey& Key, FMDMetadataBuilderRow BuilderRow, TMap<FName, IDetailGroup*>& GroupMap)
{
	if (Key.RequiredMetaData != Key.Key && !Key.RequiredMetaData.IsNone() && !HasMetadataValue(Key.RequiredMetaData))
	{
		return;
	}

	for (const FName& IncompatibleKey : Key.IncompatibleMetaData)
	{
		if (IncompatibleKey != Key.Key && !IncompatibleKey.IsNone() && HasMetadataValue(IncompatibleKey))
		{
			return;
		}
	}

	IDetailCategoryBuilder* Category = BuilderRow.HasSubtype<IDetailCategoryBuilder*>() ? BuilderRow.GetSubtype<IDetailCategoryBuilder*>() : nullptr;
	IDetailGroup* Group = BuilderRow.HasSubtype<IDetailGroup*>() ? BuilderRow.GetSubtype<IDetailGroup*>() : nullptr;

	FName GroupName = MetadataProperty.IsValid() ? MetadataProperty->GetFName() : NAME_None;
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
		else if (Category != nullptr)
		{
			Group = &Category->AddGroup(GroupName, DisplayName, false, true);
			GroupMap.Add(GroupName, Group);
		}
	}

	if (Group == nullptr && Category == nullptr)
	{
		return;
	}

	FDetailWidgetRow& MetaDataRow = (Group != nullptr)
		? Group->AddWidgetRow().FilterString(Key.GetFilterText())
		: Category->AddCustomRow(Key.GetFilterText());

	const FUIAction CopyAction = {
		FExecuteAction::CreateSP(this, &FMDMetaDataEditorFieldView::CopyMetadata, Key.Key),
		FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorFieldView::CanCopyMetadata, Key.Key)
	};

	const FUIAction PasteAction = {
		FExecuteAction::CreateSP(this, &FMDMetaDataEditorFieldView::PasteMetadata, Key.Key),
		FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorFieldView::CanPasteMetadata, Key.Key)
	};

	MetaDataRow
		.CopyAction(CopyAction)
		.PasteAction(PasteAction)
		.IsValueEnabled(!bIsReadOnly)
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
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
			.ToolTipText(INVTEXT("Remove this metadata entry"))
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ContentPadding(0)
			.Visibility(this, &FMDMetaDataEditorFieldView::GetRemoveMetadataButtonVisibility, Key.Key)
			.OnClicked(this, &FMDMetaDataEditorFieldView::OnRemoveMetadata, Key.Key)
			.Content()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.X"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FMDMetaDataEditorFieldView::AddRawMetadataEditor(FMDMetadataBuilderRow BuilderRow)
{
	IDetailCategoryBuilder* Category = BuilderRow.HasSubtype<IDetailCategoryBuilder*>() ? BuilderRow.GetSubtype<IDetailCategoryBuilder*>() : nullptr;
	IDetailGroup* Group = BuilderRow.HasSubtype<IDetailGroup*>() ? BuilderRow.GetSubtype<IDetailGroup*>() : nullptr;

	if (Category == nullptr && Group == nullptr)
	{
		return;
	}

	IDetailGroup& DetailGroup = (Group != nullptr)
		? Group->AddGroup(TEXT("RawMetadata"), INVTEXT("Raw Metadata"))
		: Category->AddGroup(TEXT("RawMetadata"), INVTEXT("Raw Metadata"));

	if (const TMap<FName, FString>* MetadataMap = GetMetadataMap())
	{
		for (const TPair<FName, FString>& MetaDataPair : *MetadataMap)
		{
			DetailGroup.AddWidgetRow()
				.FilterString(FText::Format(INVTEXT("{0}={1}"), FText::FromName(MetaDataPair.Key), FText::FromString(MetaDataPair.Value)))
				.IsEnabled(!bIsReadOnly)
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromName(MetaDataPair.Key))
					.OnTextCommitted(this, &FMDMetaDataEditorFieldView::OnMetadataKeyTextCommitted, MetaDataPair.Key)
					.RevertTextOnEscape(true)
				]
				.ValueContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
					.Text(this, &FMDMetaDataEditorFieldView::GetMetadataValueText, MetaDataPair.Key)
					.ToolTipText(this, &FMDMetaDataEditorFieldView::GetMetadataValueText, MetaDataPair.Key)
					.OnTextCommitted(this, &FMDMetaDataEditorFieldView::OnMetadataValueTextCommittedAllowingEmpty, MetaDataPair.Key)
					.RevertTextOnEscape(true)
				]
				.ExtensionContent()
				[
					SNew(SButton)
					.IsFocusable(false)
					.ToolTipText(INVTEXT("Remove this meta data"))
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ContentPadding(0)
					.Visibility(this, &FMDMetaDataEditorFieldView::GetRemoveMetadataButtonVisibility, MetaDataPair.Key)
					.OnClicked(this, &FMDMetaDataEditorFieldView::OnRemoveMetadata, MetaDataPair.Key)
					.Content()
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.X"))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				];
		}
	}

	// Add meta data row
	if (!bIsReadOnly)
	{
		DetailGroup.AddWidgetRow()
			.FilterString(INVTEXT("Add Meta Data"))
			.IsEnabled(!bIsReadOnly)
			.NameContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SEditableTextBox)
				.Text(FText::GetEmpty())
				.HintText(INVTEXT("New Meta Data entry..."))
				.OnTextCommitted(this, &FMDMetaDataEditorFieldView::OnMetadataKeyTextCommitted, FName(NAME_None))
				.RevertTextOnEscape(true)
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SEditableTextBox)
				.IsEnabled(false) // Always disabled, require that the key be entered first
				.Text(FText::GetEmpty())
			];
	}
}

bool FMDMetaDataEditorFieldView::IsConfigEnabled() const
{
	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();

	switch (FieldType) {
	case EMDMetaDataEditorFieldType::Unknown:
		return false;
	case EMDMetaDataEditorFieldType::Variable:
		return Config->bEnableMetaDataEditorForVariables;
	case EMDMetaDataEditorFieldType::LocalVariable:
		return Config->bEnableMetaDataEditorForLocalVariables;
	case EMDMetaDataEditorFieldType::FunctionParamInput:
	case EMDMetaDataEditorFieldType::FunctionParamOutput:
		return Config->bEnableMetaDataEditorForFunctionParameters;
	case EMDMetaDataEditorFieldType::StructProperty:
		return Config->bEnableMetaDataEditorForStructs;
	case EMDMetaDataEditorFieldType::Function:
		return Config->bEnableMetaDataEditorForFunctions;
	case EMDMetaDataEditorFieldType::Tunnel:
		return Config->bEnableMetaDataEditorForTunnels;
	case EMDMetaDataEditorFieldType::CustomEvent:
		return Config->bEnableMetaDataEditorForCustomEvents;
	case EMDMetaDataEditorFieldType::Struct:
		return Config->bEnableMetaDataEditorForStructs;
	}

	return false;
}

TSharedRef<SWidget> FMDMetaDataEditorFieldView::CreateMetaDataValueWidget(const FMDMetaDataKey& Key)
{
	if (Key.KeyType == EMDMetaDataEditorKeyType::Flag)
	{
		return SNew(SCheckBox)
			.ToolTipText(this, &FMDMetaDataEditorFieldView::GetCheckBoxToolTip<false>, Key.Key)
			.IsChecked(this, &FMDMetaDataEditorFieldView::IsChecked<false>, Key.Key)
			.OnCheckStateChanged(this, &FMDMetaDataEditorFieldView::HandleChecked<false>, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Boolean)
	{
		return SNew(SCheckBox)
			.ToolTipText(this, &FMDMetaDataEditorFieldView::GetCheckBoxToolTip<true>, Key.Key)
			.IsChecked(this, &FMDMetaDataEditorFieldView::IsChecked<true>, Key.Key)
			.OnCheckStateChanged(this, &FMDMetaDataEditorFieldView::HandleChecked<true>, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::String)
	{
		return SNew(SEditableTextBox)
			.Text(this, &FMDMetaDataEditorFieldView::GetMetadataValueText, Key.Key)
			.OnTextCommitted(this, &FMDMetaDataEditorFieldView::OnMetadataValueTextCommitted, Key.Key)
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
				.Value(this, &FMDMetaDataEditorFieldView::GetMetadataValueInt, Key.Key)
				.OnValueCommitted(this, &FMDMetaDataEditorFieldView::OnMetadataValueIntCommitted, Key.Key)
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
				.Value(this, &FMDMetaDataEditorFieldView::GetMetadataValueFloat, Key.Key)
				.OnValueCommitted(this, &FMDMetaDataEditorFieldView::OnMetadataValueFloatCommitted, Key.Key)
			];
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::GameplayTag)
	{
		return SNew(SMDMetaDataGameplayTagPicker)
			.Key(Key.Key)
			.bMultiSelect(false)
			.OnRemoveMetaData(this, &FMDMetaDataEditorFieldView::RemoveMetadataKey)
			.OnSetMetaData(this, &FMDMetaDataEditorFieldView::SetMetadataValue)
			.MetaDataValue(this, &FMDMetaDataEditorFieldView::GetMetadataValue, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::GameplayTagContainer)
	{
		return SNew(SMDMetaDataGameplayTagPicker)
			.Key(Key.Key)
			.bMultiSelect(true)
			.OnRemoveMetaData(this, &FMDMetaDataEditorFieldView::RemoveMetadataKey)
			.OnSetMetaData(this, &FMDMetaDataEditorFieldView::SetMetadataValue)
			.MetaDataValue(this, &FMDMetaDataEditorFieldView::GetMetadataValue, Key.Key);
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::ValueList)
	{
		return SNew(SMDMetaDataStringComboBox)
			.Key(Key.Key)
			.ValueList(Key.ValueList)
			.OnSetMetaData(this, &FMDMetaDataEditorFieldView::SetMetadataValue)
			.MetaDataValue(this, &FMDMetaDataEditorFieldView::GetMetadataValue, Key.Key);
	}

	return SNullWidget::NullWidget;
}

EVisibility FMDMetaDataEditorFieldView::GetRemoveMetadataButtonVisibility(FName Key) const
{
	return HasMetadataValue(Key) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply FMDMetaDataEditorFieldView::OnRemoveMetadata(FName Key)
{
	RemoveMetadataKey(Key);

	return FReply::Handled();
}

void FMDMetaDataEditorFieldView::OnMetadataKeyTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName OldKey)
{
	if (!NewText.IsEmptyOrWhitespace() && (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus))
	{
		if (OldKey.IsNone())
		{
			AddMetadataKey(*NewText.ToString());
			RequestRefresh.ExecuteIfBound();
		}
		else
		{
			SetMetadataKey(OldKey, *NewText.ToString());
		}
	}
}

FText FMDMetaDataEditorFieldView::GetMetadataValueText(FName Key) const
{
	return FText::FromString(GetMetadataValue(Key).Get(TEXT("")));
}

void FMDMetaDataEditorFieldView::OnMetadataValueTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName Key)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		if (!NewText.IsEmptyOrWhitespace())
		{
			SetMetadataValue(Key, NewText.ToString());
		}
		else
		{
			RemoveMetadataKey(Key);
		}
	}
}

void FMDMetaDataEditorFieldView::OnMetadataValueTextCommittedAllowingEmpty(const FText& NewText, ETextCommit::Type InTextCommit, FName Key)
{
	if (!Key.IsNone() && (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus))
	{
		SetMetadataValue(Key, NewText.ToString());
	}
}

TOptional<int32> FMDMetaDataEditorFieldView::GetMetadataValueInt(FName Key) const
{
	TOptional<FString> Value = GetMetadataValue(Key);
	return Value.IsSet() ? TOptional<int32>(FCString::Atoi(*Value.GetValue())) : TOptional<int32>{};
}

void FMDMetaDataEditorFieldView::OnMetadataValueIntCommitted(int32 Value, ETextCommit::Type InTextCommit, FName Key)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		SetMetadataValue(Key, FString::FromInt(Value));
	}
}

TOptional<float> FMDMetaDataEditorFieldView::GetMetadataValueFloat(FName Key) const
{
	TOptional<FString> Value = GetMetadataValue(Key);
	return Value.IsSet() ? TOptional<float>(FCString::Atof(*Value.GetValue())) : TOptional<float>{};
}

void FMDMetaDataEditorFieldView::OnMetadataValueFloatCommitted(float Value, ETextCommit::Type InTextCommit, FName Key)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		SetMetadataValue(Key, FString::SanitizeFloat(Value));
	}
}

template <bool bIsBoolean>
ECheckBoxState FMDMetaDataEditorFieldView::IsChecked(FName Key) const
{
	TOptional<FString> Value = GetMetadataValue(Key);

	if (Value.IsSet() && Value.GetValue() == MultipleValues)
	{
		return ECheckBoxState::Undetermined;
	}

	if constexpr (bIsBoolean)
	{
		// Don't assume unset == false, a metadata key could have different behaviour between the 2.
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
void FMDMetaDataEditorFieldView::HandleChecked(ECheckBoxState State, FName Key)
{
	if constexpr (bIsBoolean)
	{
		SetMetadataValue(Key, (State == ECheckBoxState::Checked) ? TEXT("true") : TEXT("false"));
	}
	else
	{
		if (State == ECheckBoxState::Checked)
		{
			AddMetadataKey(Key);
		}
		else
		{
			RemoveMetadataKey(Key);
		}
	}
}

template <bool bIsBoolean>
FText FMDMetaDataEditorFieldView::GetCheckBoxToolTip(FName Key) const
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

void FMDMetaDataEditorFieldView::AddMetadataKey(const FName& Key)
{
	SetMetadataValue(Key, TEXT(""));
}

void FMDMetaDataEditorFieldView::SetMetadataValue(const FName& Key, const FString& Value)
{
	TOptional<FString> CurrentValue = GetMetadataValue(Key);
	if (CurrentValue.IsSet() && CurrentValue->Equals(Value))
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Set Meta Data [{0}={1}]"), FText::FromName(Key), FText::FromString(Value)));

	if (FProperty* Property = MetadataProperty.Get())
	{
		bool bDidFindMetaData = false;

		if (UBlueprint* Blueprint = BlueprintPtr.Get())
		{
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
				if (UK2Node_FunctionEntry* FuncNode = MDMDEFV_Private::FindFunctionNode(Blueprint, Cast<UFunction>(Property->GetOwnerUObject())))
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

		if (!bDidFindMetaData)
		{
			if (UObject* ParamOwner = Property->GetOwnerUObject())
			{
				ParamOwner->Modify();
			}

			Property->SetMetaData(Key, FString(Value));
		}
	}

	if (FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetadataWithModify())
	{
		MetaData->SetMetaData(Key, FString(Value));
	}

	if (UUserDefinedStruct* Struct = MetadataStruct.Get())
	{
		Struct->SetMetaData(Key, *Value);
	}

	if (BlueprintPtr.IsValid())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
	}
	else
	{
		RequestRefresh.ExecuteIfBound();
	}
}

bool FMDMetaDataEditorFieldView::HasMetadataValue(const FName& Key) const
{
	return GetMetadataValue(Key).IsSet();
}

TOptional<FString> FMDMetaDataEditorFieldView::GetMetadataValue(FName Key) const
{
	if (const TMap<FName, FString>* MetadataMap = GetMetadataMap())
	{
		if (const FString* MetadataValue = MetadataMap->Find(Key))
		{
			return *MetadataValue;
		}
	}

	return TOptional<FString>{};
}

void FMDMetaDataEditorFieldView::SetMetadataKey(const FName& OldKey, const FName& NewKey)
{
	if (HasMetadataValue(NewKey))
	{
		return;
	}

	TOptional<FString> Value = GetMetadataValue(OldKey);
	if (!Value.IsSet())
	{
		// Not set means we don't have metadata with OldKey
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Changed Meta Data Key [{0} -> {1}]"), FText::FromName(OldKey), FText::FromName(NewKey)));
	RemoveMetadataKey(OldKey);
	SetMetadataValue(NewKey, Value.GetValue());
}

void FMDMetaDataEditorFieldView::RemoveMetadataKey(const FName& Key)
{
	if (!HasMetadataValue(Key))
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Removed Meta Data [{0}]"), FText::FromName(Key)));

	if (FProperty* Property = MetadataProperty.Get())
	{
		bool bDidFindMetaData = false;

		if (UBlueprint* Blueprint = BlueprintPtr.Get())
		{
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
				if (UK2Node_FunctionEntry* FuncNode = MDMDEFV_Private::FindFunctionNode(Blueprint, Cast<UFunction>(Property->GetOwnerUObject())))
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

		// Just a standard property?
		if (!bDidFindMetaData)
		{
			if (UObject* ParamOwner = Property->GetOwnerUObject())
			{
				ParamOwner->Modify();
			}

			Property->RemoveMetaData(Key);
		}
	}

	if (FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetadataWithModify())
	{
		MetaData->RemoveMetaData(Key);
	}

	if (UUserDefinedStruct* Struct = MetadataStruct.Get())
	{
		Struct->RemoveMetaData(Key);
	}

	if (BlueprintPtr.IsValid())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
	}
	else
	{
		RequestRefresh.ExecuteIfBound();
	}
}

void FMDMetaDataEditorFieldView::CopyMetadata(FName Key) const
{
	const TOptional<FString> Value = GetMetadataValue(Key);

	// Copy in Key=Value format
	FPlatformApplicationMisc::ClipboardCopy(*FString::Printf(TEXT("%s=\"%s\""), *Key.ToString(), *Value.Get(TEXT("")).ReplaceCharWithEscapedChar()));
}

bool FMDMetaDataEditorFieldView::CanCopyMetadata(FName Key) const
{
	return HasMetadataValue(Key);
}

void FMDMetaDataEditorFieldView::PasteMetadata(FName Key)
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
			SetMetadataValue(Key, Clipboard.ReplaceEscapedCharWithChar());
		}
	}

	// Clipboard is just the value
	if (!Clipboard.IsEmpty())
	{
		Clipboard.TrimQuotesInline();
		SetMetadataValue(Key, Clipboard.ReplaceEscapedCharWithChar());
	}
}

bool FMDMetaDataEditorFieldView::CanPasteMetadata(FName Key) const
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
