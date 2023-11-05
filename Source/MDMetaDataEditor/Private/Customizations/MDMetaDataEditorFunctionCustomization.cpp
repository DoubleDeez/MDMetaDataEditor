// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorFunctionCustomization.h"

#include "BlueprintEditorModule.h"

TSharedPtr<IDetailCustomization> FMDMetaDataEditorFunctionCustomization::MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor)
{
	const TArray<UObject*>* Objects = (BlueprintEditor.IsValid() ? BlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr);
	if (Objects != nullptr && Objects->Num() == 1)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>((*Objects)[0]))
		{
			return MakeShared<FMDMetaDataEditorFunctionCustomization>(BlueprintEditor, MakeWeakObjectPtr(Blueprint));
		}
	}

	return nullptr;
}
