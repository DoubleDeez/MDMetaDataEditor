// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDMetaDataEditorCustomizationBase.h"

class FMDMetaDataEditorFieldView;
class UUserDefinedStruct;
class FMDUserStructMetaDataEditorView;
class FStructOnScope;

class FMDMetaDataEditorStructCustomization : public FMDMetaDataEditorCustomizationBase
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TWeakPtr<FMDUserStructMetaDataEditorView> InStructMetaDataView)
	{
		return MakeShared<FMDMetaDataEditorStructCustomization>(InStructMetaDataView);
	}

	FMDMetaDataEditorStructCustomization(TWeakPtr<FMDUserStructMetaDataEditorView> InStructMetaDataView);

	virtual void CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj) override;

	virtual void RefreshDetails() override;

private:
	TSharedPtr<FMDMetaDataEditorFieldView> StructFieldView;
	TArray<TSharedPtr<FMDMetaDataEditorFieldView>> PropertyFieldViews;

	TWeakPtr<FMDUserStructMetaDataEditorView> StructMetaDataViewPtr;
	TWeakObjectPtr<UUserDefinedStruct> UserDefinedStructPtr;
};
