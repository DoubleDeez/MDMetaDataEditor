// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorStructCustomization.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFieldView.h"
#include "DetailLayoutBuilder.h"
#include "Engine/UserDefinedStruct.h"
#include "Kismet2/StructureEditorUtils.h"

FMDMetaDataEditorStructCustomization::FMDMetaDataEditorStructCustomization(TWeakPtr<FMDUserStructMetaDataEditorView> InStructMetaDataView)
	: FMDMetaDataEditorCustomizationBase(nullptr, nullptr)
	, StructMetaDataViewPtr(InStructMetaDataView)
{

}

void FMDMetaDataEditorStructCustomization::CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj)
{
	UUserDefinedStruct* UserDefinedStruct = Cast<UUserDefinedStruct>(Obj);
	if (IsValid(UserDefinedStruct))
	{
		UserDefinedStructPtr = UserDefinedStruct;

		TMap<FName, IDetailGroup*> GroupMap;
		StructFieldView = MakeShared<FMDMetaDataEditorFieldView>(UserDefinedStruct);
		StructFieldView->RequestRefresh.BindSP(this, &FMDMetaDataEditorStructCustomization::RefreshDetails);
		StructFieldView->GenerateMetadataEditor(DetailLayout, GroupMap);

		for (TFieldIterator<FProperty> PropertyIter(UserDefinedStruct); PropertyIter; ++PropertyIter)
		{
			TSharedPtr<FMDMetaDataEditorFieldView> PropertyFieldView = MakeShared<FMDMetaDataEditorFieldView>(*PropertyIter, UserDefinedStruct);
			PropertyFieldView->RequestRefresh.BindSP(this, &FMDMetaDataEditorStructCustomization::RefreshDetails);
			PropertyFieldView->GenerateMetadataEditor(DetailLayout, GroupMap);
			PropertyFieldViews.Emplace(MoveTemp(PropertyFieldView));
		}
	}
}

void FMDMetaDataEditorStructCustomization::RefreshDetails()
{
	FMDMetaDataEditorCustomizationBase::RefreshDetails();

	FStructureEditorUtils::OnStructureChanged(UserDefinedStructPtr.Get());
}
