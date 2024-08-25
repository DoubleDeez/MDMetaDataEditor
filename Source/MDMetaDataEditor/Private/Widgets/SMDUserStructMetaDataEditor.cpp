// Copyright Dylan Dumesnil. All Rights Reserved.


#include "SMDUserStructMetaDataEditor.h"

#include "Customizations/MDMetaDataEditorStructCustomization.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SlateOptMacros.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"

const FName SMDUserStructMetaDataEditor::TabId = TEXT("UserDefinedStruct_MDMetaDataEditor");

void FMDUserStructMetaDataEditorView::Initialize()
{
	StructData = MakeShared<FStructOnScope>(UserDefinedStruct.Get());
	UserDefinedStruct.Get()->InitializeDefaultValue(StructData->GetStructMemory());
	StructData->SetPackage(UserDefinedStruct->GetOutermost());

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs ViewArgs;
	ViewArgs.bAllowSearch = false;
	ViewArgs.bHideSelectionTip = false;
	ViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	DetailsView = PropertyModule.CreateDetailView(ViewArgs);
	FOnGetDetailCustomizationInstance LayoutStructDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FMDMetaDataEditorStructCustomization::MakeInstance, AsWeak());
	DetailsView->RegisterInstancedCustomPropertyLayout(UUserDefinedStruct::StaticClass(), MoveTemp(LayoutStructDetails));
	DetailsView->SetObject(UserDefinedStruct.Get());
}

void FMDUserStructMetaDataEditorView::PreChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
	// No need to destroy the struct data if only the default values are changing
	if (Info != FStructureEditorUtils::DefaultValueChanged)
	{
		StructData->Destroy();
		DetailsView->SetObject(nullptr);
		DetailsView->OnFinishedChangingProperties().Clear();
	}
}

void FMDUserStructMetaDataEditorView::PostChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
	// If change is due to default value, then struct data was not destroyed (see PreChange) and therefore does not need to be re-initialized
	if (Info != FStructureEditorUtils::DefaultValueChanged)
	{
		StructData->Initialize(UserDefinedStruct.Get());

		// Force the set object call because we may be called multiple times in a row if more than one struct was changed at the same time
		DetailsView->SetObject(UserDefinedStruct.Get(), true);
	}

	UserDefinedStruct.Get()->InitializeDefaultValue(StructData->GetStructMemory());
}

TSharedPtr<class SWidget> FMDUserStructMetaDataEditorView::GetWidget() const
{
	return DetailsView;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMDUserStructMetaDataEditor::Construct(const FArguments& InArgs)
{
	UpdateStruct(InArgs._UserDefinedStruct.Get());
}

void SMDUserStructMetaDataEditor::UpdateStruct(UUserDefinedStruct* UserDefinedStruct)
{
	UserDefinedStructPtr = UserDefinedStruct;
	if (IsValid(UserDefinedStruct))
	{
		MetaDataEditorView = MakeShared<FMDUserStructMetaDataEditorView>(UserDefinedStruct);
		MetaDataEditorView->Initialize();

		ChildSlot
		[
			MetaDataEditorView->GetWidget().ToSharedRef()
		];
	}
}

TSharedRef<SDockTab> SMDUserStructMetaDataEditor::CreateStructMetaDataEditorTab(const FSpawnTabArgs& TabArgs, TWeakObjectPtr<UUserDefinedStruct> UserDefinedStruct)
{
	return SNew(SDockTab)
		[
			SNew(SMDUserStructMetaDataEditor)
			.UserDefinedStruct(UserDefinedStruct)
		];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
