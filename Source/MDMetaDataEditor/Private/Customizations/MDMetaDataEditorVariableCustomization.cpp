// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorVariableCustomization.h"

#include "BlueprintEditorModule.h"
#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFieldView.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Blueprint.h"
#include "IDetailGroup.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

namespace MDMDEVC_Private
{
	static EMDPropertyVisibility GetVisibilityFromFlags(uint64 PropertyFlags)
	{
		const bool bEditConst         = (PropertyFlags & CPF_EditConst) != 0;
		const bool bDisableOnInstance = (PropertyFlags & CPF_DisableEditOnInstance) != 0;
		const bool bDisableOnTemplate = (PropertyFlags & CPF_DisableEditOnTemplate) != 0;

		if (bDisableOnInstance && bDisableOnTemplate)
		{
			return EMDPropertyVisibility::HiddenEverywhere;
		}
		if (bEditConst)
		{
			if (bDisableOnInstance)   return EMDPropertyVisibility::VisibleDefaultsOnly;
			if (bDisableOnTemplate)   return EMDPropertyVisibility::VisibleInstanceOnly;
			return EMDPropertyVisibility::VisibleAnywhere;
		}
		if (bDisableOnInstance)       return EMDPropertyVisibility::EditDefaultsOnly;
		if (bDisableOnTemplate)       return EMDPropertyVisibility::EditInstanceOnly;
		return EMDPropertyVisibility::EditAnywhere;
	}

	static uint64 GetFlagsFromVisibility(EMDPropertyVisibility Visibility)
	{
		switch (Visibility)
		{
		case EMDPropertyVisibility::HiddenEverywhere:    return CPF_EditConst | CPF_DisableEditOnInstance | CPF_DisableEditOnTemplate;
		case EMDPropertyVisibility::VisibleDefaultsOnly: return CPF_EditConst | CPF_DisableEditOnInstance;
		case EMDPropertyVisibility::VisibleInstanceOnly: return CPF_EditConst | CPF_DisableEditOnTemplate;
		case EMDPropertyVisibility::VisibleAnywhere:     return CPF_EditConst;
		case EMDPropertyVisibility::EditDefaultsOnly:    return CPF_DisableEditOnInstance;
		case EMDPropertyVisibility::EditInstanceOnly:    return CPF_DisableEditOnTemplate;
		case EMDPropertyVisibility::EditAnywhere:        return 0;
		default:                                         return 0;
		}
	}

	static FText GetVisibilityDisplayText(EMDPropertyVisibility Visibility)
	{
		switch (Visibility)
		{
		case EMDPropertyVisibility::HiddenEverywhere:    return INVTEXT("Hidden Everywhere");
		case EMDPropertyVisibility::VisibleDefaultsOnly: return INVTEXT("Visible Defaults Only");
		case EMDPropertyVisibility::VisibleInstanceOnly: return INVTEXT("Visible Instance Only");
		case EMDPropertyVisibility::VisibleAnywhere:     return INVTEXT("Visible Anywhere");
		case EMDPropertyVisibility::EditDefaultsOnly:    return INVTEXT("Edit Defaults Only");
		case EMDPropertyVisibility::EditInstanceOnly:    return INVTEXT("Edit Instance Only");
		case EMDPropertyVisibility::EditAnywhere:        return INVTEXT("Edit Anywhere");
		default:                                         return INVTEXT("Custom");
		}
	}

	static FText GetVisibilityTooltip(EMDPropertyVisibility Visibility)
	{
		switch (Visibility)
		{
		case EMDPropertyVisibility::HiddenEverywhere:    return INVTEXT("Variable is hidden in the class defaults and level instances.");
		case EMDPropertyVisibility::VisibleDefaultsOnly: return INVTEXT("Variable is read-only in the class defaults, and hidden on level instances.");
		case EMDPropertyVisibility::VisibleInstanceOnly: return INVTEXT("Variable is read-only on level instances, and hidden in the class defaults.");
		case EMDPropertyVisibility::VisibleAnywhere:     return INVTEXT("Variable is read-only in all contexts: on level instances and in class defaults.");
		case EMDPropertyVisibility::EditDefaultsOnly:    return INVTEXT("Variable is editable in the class defaults, but hidden on placed level instances.");
		case EMDPropertyVisibility::EditInstanceOnly:    return INVTEXT("Variable is editable on level instances, but hidden in the class defaults.");
		case EMDPropertyVisibility::EditAnywhere:        return INVTEXT("Variable is editable in all contexts: on level instances and in class defaults.");
		default:                                         return INVTEXT("");
		}
	}

	// The three flags we manage
	static constexpr uint64 AllVisibilityFlags = CPF_EditConst | CPF_DisableEditOnInstance | CPF_DisableEditOnTemplate;
}

TSharedPtr<IDetailCustomization> FMDMetaDataEditorVariableCustomization::MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor)
{
	const TArray<UObject*>* Objects = (BlueprintEditor.IsValid() ? BlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr);
	if (Objects != nullptr && Objects->Num() == 1)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>((*Objects)[0]))
		{
			return MakeShared<FMDMetaDataEditorVariableCustomization>(BlueprintEditor, MakeWeakObjectPtr(Blueprint));
		}
	}

	return nullptr;
}

void FMDMetaDataEditorVariableCustomization::CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj)
{
	UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(Obj);
	CachedVariableProperty = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr;
	FProperty* VariableProperty = CachedVariableProperty.Get();

	if (!CachedVariableProperty.IsValid())
	{
		return;
	}


	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();
	const bool bIsLocalVariable = IsValid(Cast<UFunction>(VariableProperty->GetOwnerUObject()));
	const bool bIsMetaDataEnabled =
		(bIsLocalVariable && Config->bEnableMetaDataEditorForLocalVariables)
		|| (!bIsLocalVariable && Config->bEnableMetaDataEditorForVariables);

	// Property Visibility
	if (bIsMetaDataEnabled && !bIsLocalVariable && Config->bEnablePropertyVisibilityEditor)
	{
		UBlueprint* Blueprint = GetBlueprint();
		if (IsValid(Blueprint))
		{
			bIsReadOnly = !FBlueprintEditorUtils::IsVariableCreatedByBlueprint(Blueprint, VariableProperty);

			VisibilityOptions.Reserve(7);
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::HiddenEverywhere));
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::VisibleDefaultsOnly));
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::VisibleInstanceOnly));
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::VisibleAnywhere));
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::EditDefaultsOnly));
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::EditInstanceOnly));
			VisibilityOptions.Emplace(MakeShared<EMDPropertyVisibility>(EMDPropertyVisibility::EditAnywhere));

			IDetailCategoryBuilder& MetadataCategory = DetailLayout.EditCategory("Metadata");
			IDetailGroup& PropertyVisibilityGroup = MetadataCategory.AddGroup(
				"MDDetailsPanelVisibility",
				INVTEXT("Property Visibility"),
				/*bForAdvanced=*/false,
				/*bStartExpanded=*/false);

			PropertyVisibilityGroup.HeaderRow()
				.NameContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "NoBorder")
						.ContentPadding(FMargin(0, 2, 0, 2))
						.ForegroundColor(FSlateColor::UseForeground())
						.ClickMethod(EButtonClickMethod::MouseDown)
						.OnClicked_Lambda([&PropertyVisibilityGroup]()
						{
							PropertyVisibilityGroup.ToggleExpansion(!PropertyVisibilityGroup.GetExpansionState());
							return FReply::Handled();
						})
						.Content()
						[
							SNew(STextBlock)
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.Text(INVTEXT("Property Visibility"))
								.ToolTipText(INVTEXT("Controls how visible and editable the property is in the class defaults and object instance details panels."))
						]
				]
				.ValueContent()
				.MinDesiredWidth(165.f)
				[
					SNew(SComboBox<TSharedPtr<EMDPropertyVisibility>>)
						.IsEnabled(!bIsReadOnly)
						.OptionsSource(&VisibilityOptions)
						.OnGenerateWidget(this, &FMDMetaDataEditorVariableCustomization::GenerateVisibilityOptionWidget)
						.OnSelectionChanged_Lambda([this](TSharedPtr<EMDPropertyVisibility> NewOption, ESelectInfo::Type SelectInfo)
							{
								if (NewOption.IsValid() && SelectInfo != ESelectInfo::Direct)
								{
									ApplyVisibility(*NewOption);
								}
							})
						.Content()
						[
							SNew(STextBlock)
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.Text_Lambda([this]()
									{
										return MDMDEVC_Private::GetVisibilityDisplayText(GetCurrentVisibility());
									})
								.ToolTipText_Lambda([this]()
									{
										return MDMDEVC_Private::GetVisibilityTooltip(GetCurrentVisibility());
									})
						]
				];

			// Visible on Class Defaults
			PropertyVisibilityGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(INVTEXT("Visible on Class Defaults"))
						.ToolTipText(INVTEXT("Show this variable in the class defaults Details panel."))
						.IsEnabled(!bIsReadOnly)
				]
				.ValueContent()
				[
					SNew(SCheckBox)
						.IsChecked(this, &FMDMetaDataEditorVariableCustomization::GetVisibleOnClassDefaultsState)
						.OnCheckStateChanged(this, &FMDMetaDataEditorVariableCustomization::OnVisibleOnClassDefaultsChanged)
						.IsEnabled(!bIsReadOnly)
				];

			// Visible on Instances
			PropertyVisibilityGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(INVTEXT("Visible on Instances"))
						.ToolTipText(INVTEXT("Show this variable on placed instances in the level."))
						.IsEnabled(!bIsReadOnly)
				]
				.ValueContent()
				[
					SNew(SCheckBox)
						.IsChecked(this, &FMDMetaDataEditorVariableCustomization::GetVisibleOnInstancesState)
						.OnCheckStateChanged(this, &FMDMetaDataEditorVariableCustomization::OnVisibleOnInstancesChanged)
						.IsEnabled(!bIsReadOnly)
				];

			// Editable
			PropertyVisibilityGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(INVTEXT("Editable"))
						.ToolTipText(INVTEXT("Allow editing this variable where it is visible. When disabled, the variable is shown as read-only."))
						.IsEnabled(this, &FMDMetaDataEditorVariableCustomization::IsEditableCheckboxEnabled)
				]
				.ValueContent()
				[
					SNew(SCheckBox)
						.IsChecked(this, &FMDMetaDataEditorVariableCustomization::GetEditableState)
						.OnCheckStateChanged(this, &FMDMetaDataEditorVariableCustomization::OnEditableChanged)
						.IsEnabled(this, &FMDMetaDataEditorVariableCustomization::IsEditableCheckboxEnabled)
				];
		}
	}

	// Metadata Section
	if (bIsMetaDataEnabled)
	{
		TMap<FName, IDetailGroup*> GroupMap;
		VariableFieldView = MakeShared<FMDMetaDataEditorFieldView>(VariableProperty, GetBlueprint());
		VariableFieldView->RequestRefresh.BindSP(this, &FMDMetaDataEditorVariableCustomization::RefreshDetails);
		VariableFieldView->GenerateMetadataEditor(DetailLayout, GroupMap);
	}
}

TSharedRef<SWidget> FMDMetaDataEditorVariableCustomization::GenerateVisibilityOptionWidget(TSharedPtr<EMDPropertyVisibility> Option) const
{
	const EMDPropertyVisibility Visibility = Option.IsValid() ? *Option : EMDPropertyVisibility::HiddenEverywhere;
	return SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(MDMDEVC_Private::GetVisibilityDisplayText(Visibility))
		.ToolTipText(MDMDEVC_Private::GetVisibilityTooltip(Visibility));
}

uint64 FMDMetaDataEditorVariableCustomization::GetCurrentPropertyFlags() const
{
	FProperty* Property = CachedVariableProperty.Get();
	UBlueprint* Blueprint = GetBlueprint();

	if (Property == nullptr)
	{
		return 0;
	}

	return Property->GetPropertyFlags();
}

EMDPropertyVisibility FMDMetaDataEditorVariableCustomization::GetCurrentVisibility() const
{
	return MDMDEVC_Private::GetVisibilityFromFlags(GetCurrentPropertyFlags());
}

void FMDMetaDataEditorVariableCustomization::ApplyVisibility(EMDPropertyVisibility NewVisibility)
{
	FProperty* Property = CachedVariableProperty.Get();
	UBlueprint* Blueprint = GetBlueprint();
	if (!IsValid(Blueprint) || Property == nullptr)
	{
		return;
	}

	for (FBPVariableDescription& VariableDescription : Blueprint->NewVariables)
	{
		if (VariableDescription.VarName == Property->GetFName())
		{
			const uint64 NewFlags = MDMDEVC_Private::GetFlagsFromVisibility(NewVisibility);

			FScopedTransaction Transaction(INVTEXT("Set Property Visibility"));
			Blueprint->Modify();

			// Only clear/set the flags we manage (EditConst, DisableOnInstance, DisableOnTemplate).
			VariableDescription.PropertyFlags &= ~MDMDEVC_Private::AllVisibilityFlags;
			VariableDescription.PropertyFlags |= NewFlags;

			Property->ClearPropertyFlags(static_cast<EPropertyFlags>(MDMDEVC_Private::AllVisibilityFlags));
			Property->SetPropertyFlags(static_cast<EPropertyFlags>(NewFlags));

			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
			break;
		}
	}
}

ECheckBoxState FMDMetaDataEditorVariableCustomization::GetVisibleOnClassDefaultsState() const
{
	return (GetCurrentPropertyFlags() & CPF_DisableEditOnTemplate) ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

void FMDMetaDataEditorVariableCustomization::OnVisibleOnClassDefaultsChanged(ECheckBoxState NewState)
{
	ChangeVisibilityFlag(CPF_DisableEditOnTemplate, NewState == ECheckBoxState::Unchecked, INVTEXT("Set Visible on Class Defaults"));
}

ECheckBoxState FMDMetaDataEditorVariableCustomization::GetVisibleOnInstancesState() const
{
	return (GetCurrentPropertyFlags() & CPF_DisableEditOnInstance) ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

void FMDMetaDataEditorVariableCustomization::OnVisibleOnInstancesChanged(ECheckBoxState NewState)
{
	ChangeVisibilityFlag(CPF_DisableEditOnInstance, NewState == ECheckBoxState::Unchecked, INVTEXT("Set Visible on Instances"));
}

bool FMDMetaDataEditorVariableCustomization::IsEditableCheckboxEnabled() const
{
	if (bIsReadOnly)
	{
		return false;
	}

	const uint64 Flags = GetCurrentPropertyFlags();
	const bool bHiddenOnInstance = (Flags & CPF_DisableEditOnInstance) != 0;
	const bool bHiddenOnTemplate = (Flags & CPF_DisableEditOnTemplate) != 0;
	return !(bHiddenOnInstance && bHiddenOnTemplate);
}

ECheckBoxState FMDMetaDataEditorVariableCustomization::GetEditableState() const
{
	return (GetCurrentPropertyFlags() & CPF_EditConst) ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

void FMDMetaDataEditorVariableCustomization::OnEditableChanged(ECheckBoxState NewState)
{
	ChangeVisibilityFlag(CPF_EditConst, NewState == ECheckBoxState::Unchecked, INVTEXT("Set Editable"));
}

void FMDMetaDataEditorVariableCustomization::ChangeVisibilityFlag(uint64 FlagMask, bool bSet, const FText& TransactionLabel)
{
	FProperty* Property = CachedVariableProperty.Get();
	UBlueprint* Blueprint = GetBlueprint();
	if (!IsValid(Blueprint) || Property == nullptr)
	{
		return;
	}

	for (FBPVariableDescription& VariableDescription : Blueprint->NewVariables)
	{
		if (VariableDescription.VarName == Property->GetFName())
		{
			FScopedTransaction Transaction(TransactionLabel);
			Blueprint->Modify();

			if (bSet)
			{
				VariableDescription.PropertyFlags |= FlagMask;
				Property->SetPropertyFlags(static_cast<EPropertyFlags>(FlagMask));
			}
			else
			{
				VariableDescription.PropertyFlags &= ~FlagMask;
				Property->ClearPropertyFlags(static_cast<EPropertyFlags>(FlagMask));
			}

			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
			break;
		}
	}
}
