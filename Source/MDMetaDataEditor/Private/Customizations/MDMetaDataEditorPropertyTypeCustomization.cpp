// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorPropertyTypeCustomization.h"

#include "Config/MDMetaDataEditorConfig.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EdGraphSchema_K2.h"
#include "SPinTypeSelector.h"
#include "Widgets/Layout/SBox.h"

namespace MDMDEPTC_Private
{
	FEdGraphPinType GetPinType(TWeakPtr<IPropertyHandle> PropertyHandlePtr)
	{
		if (!PropertyHandlePtr.IsValid())
		{
			return FEdGraphPinType();
		}

		const IPropertyHandle* PropertyHandle = PropertyHandlePtr.Pin().Get();

		void* PropertyValue = nullptr;
		if (PropertyHandle->GetValueData(PropertyValue) != FPropertyAccess::Success || PropertyValue == nullptr)
		{
			return FEdGraphPinType();
		}

		const FMDMetaDataEditorPropertyType* PropertyType = static_cast<FMDMetaDataEditorPropertyType*>(PropertyValue);
		return PropertyType->ToGraphPinType();
	}

	void OnPinTypeChanged(const FEdGraphPinType& PinType, TWeakPtr<IPropertyHandle> PropertyHandlePtr)
	{
		if (!PropertyHandlePtr.IsValid())
		{
			return;
		}

		IPropertyHandle* PropertyHandle = PropertyHandlePtr.Pin().Get();

		void* PropertyValue = nullptr;
		if (PropertyHandle->GetValueData(PropertyValue) != FPropertyAccess::Success || PropertyValue == nullptr)
		{
			return;
		}

		PropertyHandle->NotifyPreChange();

		FMDMetaDataEditorPropertyType* PropertyType = static_cast<FMDMetaDataEditorPropertyType*>(PropertyValue);
		PropertyType->SetFromGraphPinType(PinType);

		PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		PropertyHandle->NotifyFinishedChangingProperties();
	}
}

TSharedRef<IPropertyTypeCustomization> FMDMetaDataEditorPropertyTypeCustomization::MakeInstance()
{
	return MakeShared<FMDMetaDataEditorPropertyTypeCustomization>();
}

void FMDMetaDataEditorPropertyTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
                                                                 IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		[
			SNew(SPinTypeSelector, FGetPinTypeTree::CreateUObject(GetDefault<UEdGraphSchema_K2>(), &UEdGraphSchema_K2::GetVariableTypeTree))
			.TargetPinType_Static(&MDMDEPTC_Private::GetPinType, TWeakPtr<IPropertyHandle>(PropertyHandle))
			.OnPinTypeChanged_Static(&MDMDEPTC_Private::OnPinTypeChanged, TWeakPtr<IPropertyHandle>(PropertyHandle))
			.TypeTreeFilter(ETypeTreeFilter::AllowWildcard)
			.Schema(GetDefault<UEdGraphSchema_K2>())
			.bAllowArrays(true)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
}
