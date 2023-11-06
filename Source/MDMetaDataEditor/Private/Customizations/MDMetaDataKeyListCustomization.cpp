// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataKeyListCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Types/MDMetaDataKey.h"

void FMDMetaDataKeyListCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const TSharedPtr<IPropertyHandle> KeyListHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMDMetaDataKeyList, MetaDataKeys));

	FUIAction CopyAction;
	FUIAction PasteAction;
	KeyListHandle->CreateDefaultPropertyCopyPasteActions(CopyAction, PasteAction);

	HeaderRow
	.CopyAction(CopyAction)
	.PasteAction(PasteAction)
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		KeyListHandle->CreatePropertyValueWidget()
	];
}

void FMDMetaDataKeyListCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const TSharedPtr<IPropertyHandleArray> ArrayHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FMDMetaDataKeyList, MetaDataKeys))->AsArray();
	uint32 NumKeys = 0;
	ArrayHandle->GetNumElements(NumKeys);

	for (uint32 i = 0; i < NumKeys; ++i)
	{
		ChildBuilder.AddProperty(ArrayHandle->GetElement(i));
	}
}
