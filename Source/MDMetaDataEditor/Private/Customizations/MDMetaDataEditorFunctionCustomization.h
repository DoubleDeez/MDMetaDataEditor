// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDMetaDataEditorCustomizationBase.h"

class FMDMetaDataEditorFunctionCustomization : public FMDMetaDataEditorCustomizationBase
{
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor);

	FMDMetaDataEditorFunctionCustomization(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
		: FMDMetaDataEditorCustomizationBase(BlueprintEditor, MoveTemp(BlueprintPtr))
	{
	}
};
