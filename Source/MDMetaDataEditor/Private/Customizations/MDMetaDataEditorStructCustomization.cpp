// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorStructCustomization.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFieldView.h"
#include "DetailLayoutBuilder.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_NEWER_THAN_OR_EQUAL(5,5,0)
	#include "StructUtils/UserDefinedStruct.h"
#else
	#include "Engine/UserDefinedStruct.h"
#endif

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
