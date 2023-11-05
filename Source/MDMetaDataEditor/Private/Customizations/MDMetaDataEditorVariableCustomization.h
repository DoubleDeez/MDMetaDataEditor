// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDMetaDataEditorCustomizationBase.h"

class FMDMetaDataEditorVariableCustomization : public FMDMetaDataEditorCustomizationBase
{
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor);

	FMDMetaDataEditorVariableCustomization(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
		: FMDMetaDataEditorCustomizationBase(BlueprintEditor, MoveTemp(BlueprintPtr))
	{
	}
};
