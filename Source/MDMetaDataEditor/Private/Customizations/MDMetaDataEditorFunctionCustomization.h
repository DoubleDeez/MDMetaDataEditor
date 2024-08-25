// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDMetaDataEditorCustomizationBase.h"

class UK2Node_EditablePinBase;
class FMDMetaDataEditorFieldView;

class FMDMetaDataEditorFunctionCustomization : public FMDMetaDataEditorCustomizationBase
{
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor);

	FMDMetaDataEditorFunctionCustomization(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
		: FMDMetaDataEditorCustomizationBase(BlueprintEditor, MoveTemp(BlueprintPtr))
	{
	}

	virtual void CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj) override;

private:
	void InitFieldViews(UObject* Obj);

	TSharedPtr<FMDMetaDataEditorFieldView> FunctionFieldView;
	TArray<TSharedPtr<FMDMetaDataEditorFieldView>> ParamFieldViews;
};
