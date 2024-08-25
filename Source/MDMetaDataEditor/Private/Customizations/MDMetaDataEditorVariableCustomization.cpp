// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorVariableCustomization.h"

#include "BlueprintEditorModule.h"
#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFieldView.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

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
	FProperty* Property = PropertyWrapper ? PropertyWrapper->GetProperty() : nullptr;
	if (Property != nullptr)
	{
		const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();
		const bool bIsLocalVariable = IsValid(Cast<UFunction>(Property->GetOwnerUObject()));
		const bool bIsEnabled = (bIsLocalVariable && Config->bEnableMetaDataEditorForLocalVariables)
			|| (!bIsLocalVariable && Config->bEnableMetaDataEditorForVariables);
		if (bIsEnabled)
		{
			TMap<FName, IDetailGroup*> GroupMap;
			VariableFieldView = MakeShared<FMDMetaDataEditorFieldView>(Property, GetBlueprint());
			VariableFieldView->RequestRefresh.BindSP(this, &FMDMetaDataEditorVariableCustomization::RefreshDetails);
			VariableFieldView->GenerateMetadataEditor(DetailLayout, GroupMap);
		}
	}
}
