// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorCustomizationBase.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "Config/MDMetaDataEditorUserConfig.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailGroup.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
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

	FProperty* FindNodeProperty(const UK2Node_EditablePinBase* Node, const TSharedPtr<FUserPinInfo>& PinInfo)
	{
		// Specifically grab the generated class, not the skeleton class so that UMDMetaDataEditorBlueprintCompilerExtension can grab the meta data after the BP is compiled

		const UBlueprint* Blueprint = IsValid(Node) ? Node->GetBlueprint() : nullptr;
		const UClass* Class = IsValid(Blueprint) ? Blueprint->GeneratedClass : nullptr;
		const UFunction* Function = nullptr;

		if (const UK2Node_FunctionResult* ResultNode = Cast<UK2Node_FunctionResult>(Node))
		{
			// Function result nodes cannot resolve the UFunction, so find the entry node and use that for finding the UFunction
			TArray<UK2Node_FunctionEntry*> EntryNodes;
			ResultNode->GetGraph()->GetNodesOfClass(EntryNodes);
			Node = EntryNodes[0];
		}

		if (const UK2Node_FunctionEntry* FunctionNode = Cast<UK2Node_FunctionEntry>(Node))
		{
			const FName FunctionName = (FunctionNode->CustomGeneratedFunctionName != NAME_None) ? FunctionNode->CustomGeneratedFunctionName : FunctionNode->GetGraph()->GetFName();
			Function = Class->FindFunctionByName(FunctionName);
		}
		else if (const UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
		{
			FName SearchName = EventNode->EventReference.GetMemberName();
			if (SearchName.IsNone())
			{
				SearchName = EventNode->CustomFunctionName;
			}
			Function = Class->FindFunctionByName(SearchName);
		}

		if (!IsValid(Function) || !PinInfo.IsValid())
		{
			return nullptr;
		}

		return Function->FindPropertyByName(PinInfo->PinName);
	}
}

FMDMetaDataEditorCustomizationBase::FMDMetaDataEditorCustomizationBase(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
	: BlueprintEditor(BlueprintEditor)
	, BlueprintPtr(MoveTemp(BlueprintPtr))
{}

void FMDMetaDataEditorCustomizationBase::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	DetailBuilderPtr = &DetailLayout;

	PropertyBeingCustomized.Reset();
	FunctionBeingCustomized.Reset();
	TunnelBeingCustomized.Reset();
	EventBeingCustomized.Reset();

	UBlueprint* Blueprint = BlueprintPtr.Get();
	if (!IsValid(Blueprint))
	{
		return;
	}

	bool bIsReadOnly = true;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	bool bIsFunction = false;
	bool bIsProperty = false;

	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();
	const UMDMetaDataEditorUserConfig* UserConfig = GetDefault<UMDMetaDataEditorUserConfig>();

	UObject* Obj = ObjectsBeingCustomized[0].Get();
	UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(Obj);
	UK2Node_EditablePinBase* Node = nullptr;
	if (FProperty* Prop = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr)
	{
		bIsReadOnly = Prop->IsNative() || (!(IsValid(Prop->GetOwnerUObject()) && Prop->GetOwnerUObject()->IsA<UFunction>()) && !FBlueprintEditorUtils::IsVariableCreatedByBlueprint(Blueprint, Prop));
		PropertyBeingCustomized = Prop;
		bIsProperty = true;
	}
	else if (UK2Node_FunctionEntry* Function = MDMDECB_Private::FindNode<UK2Node_FunctionEntry, false>(Obj))
	{
		if (Config->bEnableMetaDataEditorForFunctions)
		{
			FunctionBeingCustomized = Function;
		}

		bIsFunction = true;
		Node = Function;
		bIsReadOnly = !Function->bIsEditable;
	}
	else if (UK2Node_Tunnel* Tunnel = MDMDECB_Private::FindNode<UK2Node_Tunnel, true>(Obj))
	{
		if (Config->bEnableMetaDataEditorForTunnels)
		{
			TunnelBeingCustomized = Tunnel;
		}

		bIsFunction = true;
		Node = Tunnel;
		bIsReadOnly = !Tunnel->bIsEditable;
	}
	else if (UK2Node_CustomEvent* Event = MDMDECB_Private::FindNode<UK2Node_CustomEvent, false>(Obj))
	{
		if (Config->bEnableMetaDataEditorForFunctions)
		{
			EventBeingCustomized = Event;
		}

		bIsFunction = true;
		Node = Event;
		bIsReadOnly = !Event->bIsEditable;
	}

	if (bIsProperty)
	{
		// Put Meta Data above Default Value for Variables
		int32 MetaDataSortOrder = DetailLayout.EditCategory("Variable").GetSortOrder();
		DetailLayout.EditCategory("MetaData").SetSortOrder(++MetaDataSortOrder);
		DetailLayout.EditCategory("DefaultValue").SetSortOrder(++MetaDataSortOrder);
	}
	else if (bIsFunction)
	{
		// Put Meta Data above Inputs for Functions
		int32 MetaDataSortOrder = DetailLayout.EditCategory("Graph").GetSortOrder();
		DetailLayout.EditCategory("MetaData").SetSortOrder(++MetaDataSortOrder);
		DetailLayout.EditCategory("Inputs").SetSortOrder(++MetaDataSortOrder);
		DetailLayout.EditCategory("Outputs").SetSortOrder(++MetaDataSortOrder);
	}

	TMap<FName, IDetailGroup*> GroupMap;

	auto AddMetaDataKey = [this, &DetailLayout, &GroupMap, bIsReadOnly](const FMDMetaDataKey& Key)
	{
		if (Key.RequiredMetaData != Key.Key && !Key.RequiredMetaData.IsNone() && !HasMetaDataValue(Key.RequiredMetaData, nullptr))
		{
			return;
		}

		for (const FName& IncompatibleKey : Key.IncompatibleMetaData)
		{
			if (IncompatibleKey != Key.Key && !IncompatibleKey.IsNone() && HasMetaDataValue(IncompatibleKey, nullptr))
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
			FExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CopyMetaData, Key.Key, TWeakFieldPtr<FProperty>()),
			FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CanCopyMetaData, Key.Key, TWeakFieldPtr<FProperty>())
		};

		const FUIAction PasteAction = {
			FExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::PasteMetaData, Key.Key, TWeakFieldPtr<FProperty>()),
			FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CanPasteMetaData, Key.Key, TWeakFieldPtr<FProperty>())
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
				CreateMetaDataValueWidget(Key, nullptr)
			]
			.ExtensionContent()
			[
				SNew(SButton)
				.IsFocusable(false)
				.ToolTipText(INVTEXT("Remove this meta data from this property"))
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(0)
				.Visibility(this, &FMDMetaDataEditorCustomizationBase::GetRemoveMetaDataButtonVisibility, Key.Key, TWeakFieldPtr<FProperty>())
				.OnClicked(this, &FMDMetaDataEditorCustomizationBase::OnRemoveMetaData, Key.Key, TWeakFieldPtr<FProperty>())
				.Content()
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.X"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];
	};

	const TMap<FName, FString>* MetaDataMap = nullptr;

	if (const FProperty* Property = PropertyBeingCustomized.Get())
	{
		MetaDataMap = Property->GetMetaDataMap();

		if (Cast<UFunction>(Property->GetOwnerUObject()))
		{
			Config->ForEachLocalVariableMetaDataKey(Blueprint, Property, AddMetaDataKey);
		}
		else
		{
			Config->ForEachVariableMetaDataKey(Blueprint, Property, AddMetaDataKey);
		}
	}
	else if (IsValid(Node))
	{
		if (const UK2Node_FunctionEntry* FuncNode = Cast<UK2Node_FunctionEntry>(FunctionBeingCustomized.Get()))
		{
			MetaDataMap = &(FuncNode->MetaData.GetMetaDataMap());
		}
		else if (const UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(TunnelBeingCustomized.Get()))
		{
			MetaDataMap = &(TunnelNode->MetaData.GetMetaDataMap());
		}
		else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(EventBeingCustomized.Get()))
		{
			MetaDataMap = &(EventNode->GetUserDefinedMetaData().GetMetaDataMap());
		}

		if (FunctionBeingCustomized.IsValid() || TunnelBeingCustomized.IsValid() || EventBeingCustomized.IsValid())
		{
			Config->ForEachFunctionMetaDataKey(Blueprint, AddMetaDataKey);
		}

		// TODO - Refactor - Adding the param meta data logic made this file/class a mess. Consider pulling that logic out.
		if (Config->bEnableMetaDataEditorForFunctionParameters)
		{
			TMap<FName, IDetailGroup*> ParamGroupMap;

			// Lazy init the groups so that they're not added unless we actually have meta keys to show
			IDetailGroup* InputsGroupPtr = nullptr;
			auto GetInputsGroup = [&InputsGroupPtr, &DetailLayout]() -> IDetailGroup&
			{
				if (InputsGroupPtr == nullptr)
				{
					InputsGroupPtr = &DetailLayout.EditCategory("Inputs").AddGroup(TEXT("ParamMetaData"), INVTEXT("Param Meta Data"));
				}

				return *InputsGroupPtr;
			};

			IDetailGroup* OutputsGroupPtr = nullptr;
			auto GetOutputsGroup = [&OutputsGroupPtr, &DetailLayout]() -> IDetailGroup&
			{
				if (OutputsGroupPtr == nullptr)
				{
					OutputsGroupPtr = &DetailLayout.EditCategory("Outputs").AddGroup(TEXT("ParamMetaData"), INVTEXT("Param Meta Data"));
				}

				return *OutputsGroupPtr;
			};

			for (const TSharedPtr<FUserPinInfo>& PinInfo : Node->UserDefinedPins)
			{
				FProperty* ParamProperty = MDMDECB_Private::FindNodeProperty(Node, PinInfo);
				if (ParamProperty == nullptr)
				{
					continue;
				}

				auto AddParamMetaDataKey =
					[this, &DetailLayout, &ParamGroupMap, bIsReadOnly, ParamProperty, &GetInputsGroup, &GetOutputsGroup, Direction = PinInfo->DesiredPinDirection](const FMDMetaDataKey& Key)
				{
					if (Key.RequiredMetaData != Key.Key && !Key.RequiredMetaData.IsNone() && !HasMetaDataValue(Key.RequiredMetaData, MakeWeakFieldPtr(ParamProperty)))
					{
						return;
					}

					for (const FName& IncompatibleKey : Key.IncompatibleMetaData)
					{
						if (IncompatibleKey != Key.Key && !IncompatibleKey.IsNone() && HasMetaDataValue(IncompatibleKey, MakeWeakFieldPtr(ParamProperty)))
						{
							return;
						}
					}

					IDetailGroup& RootGroup = (Direction == EGPD_Output) ? GetInputsGroup() : GetOutputsGroup();

					FName GroupName = ParamProperty->GetFName();
					IDetailGroup* Group = (ParamGroupMap.Contains(GroupName))
						? ParamGroupMap.FindRef(GroupName)
						: ParamGroupMap.Add(GroupName, &RootGroup.AddGroup(GroupName, ParamProperty->GetDisplayNameText()));
					TArray<FString> Subgroups;
					Key.Category.ParseIntoArray(Subgroups, TEXT("|"));
					for (const FString& Subgroup : Subgroups)
					{
						const FText DisplayName = FText::FromString(Subgroup);
						if (DisplayName.IsEmptyOrWhitespace())
						{
							continue;
						}

						GroupName = *FString::Printf(TEXT("%s|%s"), *GroupName.ToString(), *Subgroup);

						if (ParamGroupMap.Contains(GroupName))
						{
							Group = ParamGroupMap.FindRef(GroupName);
						}
						else if (Group != nullptr)
						{
							Group = &Group->AddGroup(GroupName, DisplayName);
							ParamGroupMap.Add(GroupName, Group);
						}
						else
						{
							Group = &RootGroup.AddGroup(GroupName, DisplayName);
							ParamGroupMap.Add(GroupName, Group);
						}
					}

					const FUIAction CopyAction = {
						FExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CopyMetaData, Key.Key, MakeWeakFieldPtr(ParamProperty)),
						FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CanCopyMetaData, Key.Key, MakeWeakFieldPtr(ParamProperty))
					};

					const FUIAction PasteAction = {
						FExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::PasteMetaData, Key.Key, MakeWeakFieldPtr(ParamProperty)),
						FCanExecuteAction::CreateSP(this, &FMDMetaDataEditorCustomizationBase::CanPasteMetaData, Key.Key, MakeWeakFieldPtr(ParamProperty))
					};

					Group->AddWidgetRow().FilterString(Key.GetFilterText())
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
							CreateMetaDataValueWidget(Key, ParamProperty)
						]
						.ExtensionContent()
						[
							SNew(SButton)
							.IsFocusable(false)
							.ToolTipText(INVTEXT("Remove this meta data from this parameter"))
							.ButtonStyle(FAppStyle::Get(), "SimpleButton")
							.ContentPadding(0)
							.Visibility(this, &FMDMetaDataEditorCustomizationBase::GetRemoveMetaDataButtonVisibility, Key.Key, MakeWeakFieldPtr(ParamProperty))
							.OnClicked(this, &FMDMetaDataEditorCustomizationBase::OnRemoveMetaData, Key.Key, MakeWeakFieldPtr(ParamProperty))
							.Content()
							[
								SNew(SImage)
								.Image(FAppStyle::GetBrush("Icons.X"))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						];
				};

				Config->ForEachParameterMetaDataKey(Blueprint, ParamProperty, AddParamMetaDataKey);

				const TMap<FName, FString>* ParamMetaDataMap = ParamProperty->GetMetaDataMap();
				if (UserConfig->bEnableRawMetaDataEditor && (!bIsReadOnly || (ParamMetaDataMap != nullptr && !ParamMetaDataMap->IsEmpty())))
				{
					IDetailGroup& RootGroup = (PinInfo->DesiredPinDirection == EGPD_Output) ? GetInputsGroup() : GetOutputsGroup();
					FName GroupName = ParamProperty->GetFName();
					IDetailGroup* Group = (ParamGroupMap.Contains(GroupName))
						? ParamGroupMap.FindRef(GroupName)
						: ParamGroupMap.Add(GroupName, &RootGroup.AddGroup(GroupName, ParamProperty->GetDisplayNameText()));

					IDetailGroup& RawMetaDataGroup = Group->AddGroup(TEXT("RawMetaData"), INVTEXT("Raw Meta Data"), true);
					AddRawMetaDataEditor(ParamMetaDataMap, RawMetaDataGroup, bIsReadOnly, ParamProperty);
				}
			}
		}
	}

	if (UserConfig->bEnableRawMetaDataEditor && (!bIsReadOnly || (MetaDataMap != nullptr && !MetaDataMap->IsEmpty())))
	{
		IDetailGroup& RawMetaDataGroup = DetailLayout
			.EditCategory("MetaData")
			.AddGroup(TEXT("RawMetaData"), INVTEXT("Raw Meta Data"), true);
		AddRawMetaDataEditor(MetaDataMap, RawMetaDataGroup, bIsReadOnly, nullptr);
	}
}

EVisibility FMDMetaDataEditorCustomizationBase::GetRemoveMetaDataButtonVisibility(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	return HasMetaDataValue(Key, PropertyPtr) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply FMDMetaDataEditorCustomizationBase::OnRemoveMetaData(FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	RemoveMetaDataKey(Key, PropertyPtr);

	return FReply::Handled();
}

FReply FMDMetaDataEditorCustomizationBase::OnRemoveMetaDataAndRefresh(FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	RemoveMetaDataKey(Key, PropertyPtr);
	RefreshDetails();

	return FReply::Handled();
}

TSharedRef<SWidget> FMDMetaDataEditorCustomizationBase::CreateMetaDataValueWidget(const FMDMetaDataKey& Key, FProperty* Property)
{
	if (Key.KeyType == EMDMetaDataEditorKeyType::Flag)
	{
		return SNew(SCheckBox)
			.ToolTipText(this, &FMDMetaDataEditorCustomizationBase::GetCheckBoxToolTip<false>, Key.Key, MakeWeakFieldPtr(Property))
			.IsChecked(this, &FMDMetaDataEditorCustomizationBase::IsChecked<false>, Key.Key, MakeWeakFieldPtr(Property))
			.OnCheckStateChanged(this, &FMDMetaDataEditorCustomizationBase::HandleChecked<false>, Key.Key, MakeWeakFieldPtr(Property));
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::Boolean)
	{
		return SNew(SCheckBox)
			.ToolTipText(this, &FMDMetaDataEditorCustomizationBase::GetCheckBoxToolTip<true>, Key.Key, MakeWeakFieldPtr(Property))
			.IsChecked(this, &FMDMetaDataEditorCustomizationBase::IsChecked<true>, Key.Key, MakeWeakFieldPtr(Property))
			.OnCheckStateChanged(this, &FMDMetaDataEditorCustomizationBase::HandleChecked<true>, Key.Key, MakeWeakFieldPtr(Property));
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::String)
	{
		return SNew(SEditableTextBox)
			.Text(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueText, Key.Key, MakeWeakFieldPtr(Property))
			.OnTextCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueTextCommitted, Key.Key, MakeWeakFieldPtr(Property))
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
				.Value(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueInt, Key.Key, MakeWeakFieldPtr(Property))
				.OnValueCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueIntCommitted, Key.Key, MakeWeakFieldPtr(Property))
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
				.Value(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueFloat, Key.Key, MakeWeakFieldPtr(Property))
				.OnValueCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueFloatCommitted, Key.Key, MakeWeakFieldPtr(Property))
			];
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::GameplayTag)
	{
		return SNew(SMDMetaDataGameplayTagPicker)
			.Key(Key.Key)
			.bMultiSelect(false)
			.OnRemoveMetaData(this, &FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey, MakeWeakFieldPtr(Property))
			.OnSetMetaData(this, &FMDMetaDataEditorCustomizationBase::SetMetaDataValue, MakeWeakFieldPtr(Property))
			.MetaDataValue(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValue, Key.Key, MakeWeakFieldPtr(Property));
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::GameplayTagContainer)
	{
		return SNew(SMDMetaDataGameplayTagPicker)
			.Key(Key.Key)
			.bMultiSelect(true)
			.OnRemoveMetaData(this, &FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey, MakeWeakFieldPtr(Property))
			.OnSetMetaData(this, &FMDMetaDataEditorCustomizationBase::SetMetaDataValue, MakeWeakFieldPtr(Property))
			.MetaDataValue(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValue, Key.Key, MakeWeakFieldPtr(Property));
	}
	else if (Key.KeyType == EMDMetaDataEditorKeyType::ValueList)
	{
		return SNew(SMDMetaDataStringComboBox)
			.Key(Key.Key)
			.ValueList(Key.ValueList)
			.OnSetMetaData(this, &FMDMetaDataEditorCustomizationBase::SetMetaDataValue, MakeWeakFieldPtr(Property))
			.MetaDataValue(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValue, Key.Key, MakeWeakFieldPtr(Property));
	}

	return SNullWidget::NullWidget;
}

void FMDMetaDataEditorCustomizationBase::AddRawMetaDataEditor(const TMap<FName, FString>* MetaDataMap, IDetailGroup& RawMetaDataGroup, bool bIsReadOnly, FProperty* Property)
{
	if (MetaDataMap != nullptr)
	{
		for (const TPair<FName, FString>& MetaDataPair : *MetaDataMap)
		{
			RawMetaDataGroup.AddWidgetRow()
				.FilterString(FText::Format(INVTEXT("{0}={1}"), FText::FromName(MetaDataPair.Key), FText::FromString(MetaDataPair.Value)))
				.IsEnabled(!bIsReadOnly)
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromName(MetaDataPair.Key))
					.OnTextCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataKeyTextCommitted, MetaDataPair.Key, MakeWeakFieldPtr(Property))
					.RevertTextOnEscape(true)
				]
				.ValueContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
					.Text(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueText, MetaDataPair.Key, MakeWeakFieldPtr(Property))
					.ToolTipText(this, &FMDMetaDataEditorCustomizationBase::GetMetaDataValueText, MetaDataPair.Key, MakeWeakFieldPtr(Property))
					.OnTextCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataValueTextCommittedAllowingEmpty, MetaDataPair.Key, MakeWeakFieldPtr(Property))
					.RevertTextOnEscape(true)
				]
				.ExtensionContent()
				[
					SNew(SButton)
					.IsFocusable(false)
					.ToolTipText(INVTEXT("Remove this meta data"))
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ContentPadding(0)
					.Visibility(this, &FMDMetaDataEditorCustomizationBase::GetRemoveMetaDataButtonVisibility, MetaDataPair.Key, MakeWeakFieldPtr(Property))
					.OnClicked(this, &FMDMetaDataEditorCustomizationBase::OnRemoveMetaDataAndRefresh, MetaDataPair.Key, MakeWeakFieldPtr(Property))
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
		RawMetaDataGroup.AddWidgetRow()
			.FilterString(INVTEXT("Add Meta Data"))
			.IsEnabled(!bIsReadOnly)
			.NameContent()
			.HAlign(HAlign_Fill)
			[
				SNew(SEditableTextBox)
				.Text(FText::GetEmpty())
				.HintText(INVTEXT("New Meta Data entry..."))
				.OnTextCommitted(this, &FMDMetaDataEditorCustomizationBase::OnMetaDataKeyTextCommitted, FName(NAME_None), MakeWeakFieldPtr(Property))
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

void FMDMetaDataEditorCustomizationBase::AddMetaDataKey(const FName& Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	SetMetaDataValue(Key, TEXT(""), PropertyPtr);
}

void FMDMetaDataEditorCustomizationBase::SetMetaDataValue(const FName& Key, const FString& Value, TWeakFieldPtr<FProperty> PropertyPtr)
{
	TOptional<FString> CurrentValue = GetMetaDataValue(Key, PropertyPtr);
	if (CurrentValue.IsSet() && CurrentValue->Equals(Value))
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Set Meta Data [{0}={1}]"), FText::FromName(Key), FText::FromString(Value)));

	if (FProperty* ParamProperty = PropertyPtr.Get())
	{
		if (UObject* ParamOwner = ParamProperty->GetOwnerUObject())
		{
			ParamOwner->Modify();
		}

		ParamProperty->SetMetaData(Key, FString(Value));
	}
	else
	{
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
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
}

bool FMDMetaDataEditorCustomizationBase::HasMetaDataValue(const FName& Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	return GetMetaDataValue(Key, PropertyPtr).IsSet();
}

TOptional<FString> FMDMetaDataEditorCustomizationBase::GetMetaDataValue(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	TOptional<FString> Value;

	if (const FProperty* ParamProperty = PropertyPtr.Get())
	{
		if (ParamProperty->HasMetaData(Key))
		{
			Value = ParamProperty->GetMetaData(Key);
		}

		return Value;
	}

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

void FMDMetaDataEditorCustomizationBase::SetMetaDataKey(const FName& OldKey, const FName& NewKey, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (HasMetaDataValue(NewKey, PropertyPtr))
	{
		return;
	}

	TOptional<FString> Value = GetMetaDataValue(OldKey, PropertyPtr);
	if (!Value.IsSet())
	{
		// Not set means we don't have meta data with OldKey
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Changed Meta Data Key [{0} -> {1}]"), FText::FromName(OldKey), FText::FromName(NewKey)));
	RemoveMetaDataKey(OldKey, PropertyPtr);
	SetMetaDataValue(NewKey, Value.GetValue(), PropertyPtr);
}

void FMDMetaDataEditorCustomizationBase::RemoveMetaDataKey(const FName& Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (!HasMetaDataValue(Key, PropertyPtr))
	{
		return;
	}

	FScopedTransaction Transaction(FText::Format(INVTEXT("Removed Meta Data [{0}]"), FText::FromName(Key)));

	if (FProperty* ParamProperty = PropertyPtr.Get())
	{
		if (UObject* ParamOwner = ParamProperty->GetOwnerUObject())
		{
			ParamOwner->Modify();
		}

		ParamProperty->RemoveMetaData(Key);
	}
	else
	{
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
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr.Get());
}

void FMDMetaDataEditorCustomizationBase::CopyMetaData(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	const TOptional<FString> Value = GetMetaDataValue(Key, PropertyPtr);

	// Copy in Key=Value format
	FPlatformApplicationMisc::ClipboardCopy(*FString::Printf(TEXT("%s=\"%s\""), *Key.ToString(), *Value.Get(TEXT("")).ReplaceCharWithEscapedChar()));
}

bool FMDMetaDataEditorCustomizationBase::CanCopyMetaData(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	return HasMetaDataValue(Key, PropertyPtr);
}

void FMDMetaDataEditorCustomizationBase::PasteMetaData(FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
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
			SetMetaDataValue(Key, Clipboard.ReplaceEscapedCharWithChar(), PropertyPtr);
		}
	}

	// Clipboard is just the value
	if (!Clipboard.IsEmpty())
	{
		Clipboard.TrimQuotesInline();
		SetMetaDataValue(Key, Clipboard.ReplaceEscapedCharWithChar(), PropertyPtr);
	}
}

bool FMDMetaDataEditorCustomizationBase::CanPasteMetaData(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
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

void FMDMetaDataEditorCustomizationBase::OnMetaDataKeyTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName OldKey, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (!NewText.IsEmptyOrWhitespace() && (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus))
	{
		if (OldKey.IsNone())
		{
			AddMetaDataKey(*NewText.ToString(), PropertyPtr);
			RefreshDetails();
		}
		else
		{
			SetMetaDataKey(OldKey, *NewText.ToString(), PropertyPtr);
		}
	}
}

FText FMDMetaDataEditorCustomizationBase::GetMetaDataValueText(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	return FText::FromString(GetMetaDataValue(Key, PropertyPtr).Get(TEXT("")));
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		if (!NewText.IsEmptyOrWhitespace())
		{
			SetMetaDataValue(Key, NewText.ToString(), PropertyPtr);
		}
		else
		{
			RemoveMetaDataKey(Key, PropertyPtr);
		}
	}
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueTextCommittedAllowingEmpty(const FText& NewText, ETextCommit::Type InTextCommit, FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (!Key.IsNone() && (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus))
	{
		SetMetaDataValue(Key, NewText.ToString(), PropertyPtr);
	}
}

TOptional<int32> FMDMetaDataEditorCustomizationBase::GetMetaDataValueInt(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	TOptional<FString> Value = GetMetaDataValue(Key, PropertyPtr);
	return Value.IsSet() ? TOptional<int32>(FCString::Atoi(*Value.GetValue())) : TOptional<int32>{};
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueIntCommitted(int32 Value, ETextCommit::Type InTextCommit, FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		SetMetaDataValue(Key, FString::FromInt(Value), PropertyPtr);
	}
}

TOptional<float> FMDMetaDataEditorCustomizationBase::GetMetaDataValueFloat(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	TOptional<FString> Value = GetMetaDataValue(Key, PropertyPtr);
	return Value.IsSet() ? TOptional<float>(FCString::Atof(*Value.GetValue())) : TOptional<float>{};
}

void FMDMetaDataEditorCustomizationBase::OnMetaDataValueFloatCommitted(float Value, ETextCommit::Type InTextCommit, FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
	{
		SetMetaDataValue(Key, FString::SanitizeFloat(Value), PropertyPtr);
	}
}

void FMDMetaDataEditorCustomizationBase::RefreshDetails()
{
	if (DetailBuilderPtr != nullptr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
		DetailBuilderPtr = nullptr;
	}
}

template <bool bIsBoolean>
ECheckBoxState FMDMetaDataEditorCustomizationBase::IsChecked(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	TOptional<FString> Value = GetMetaDataValue(Key, PropertyPtr);

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
void FMDMetaDataEditorCustomizationBase::HandleChecked(ECheckBoxState State, FName Key, TWeakFieldPtr<FProperty> PropertyPtr)
{
	if constexpr (bIsBoolean)
	{
		SetMetaDataValue(Key, (State == ECheckBoxState::Checked) ? TEXT("true") : TEXT("false"), PropertyPtr);
	}
	else
	{
		if (State == ECheckBoxState::Checked)
		{
			AddMetaDataKey(Key, PropertyPtr);
		}
		else
		{
			RemoveMetaDataKey(Key, PropertyPtr);
		}
	}
}

template <bool bIsBoolean>
FText FMDMetaDataEditorCustomizationBase::GetCheckBoxToolTip(FName Key, TWeakFieldPtr<FProperty> PropertyPtr) const
{
	switch (IsChecked<bIsBoolean>(Key, PropertyPtr))
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
