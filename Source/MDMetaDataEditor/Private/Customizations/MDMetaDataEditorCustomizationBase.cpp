// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorCustomizationBase.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "Config/MDMetaDataEditorUserConfig.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/UserDefinedStruct.h"
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

FMDMetaDataEditorCustomizationBase::FMDMetaDataEditorCustomizationBase(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
	: BlueprintEditor(BlueprintEditor)
	, BlueprintPtr(MoveTemp(BlueprintPtr))
{}

void FMDMetaDataEditorCustomizationBase::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	DetailBuilderPtr = &DetailLayout;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.Num() != 1)
	{
		return;
	}

	CustomizeObject(DetailLayout, ObjectsBeingCustomized[0].Get());
}

void FMDMetaDataEditorCustomizationBase::RefreshDetails()
{
	if (DetailBuilderPtr != nullptr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
		DetailBuilderPtr = nullptr;
	}
}
