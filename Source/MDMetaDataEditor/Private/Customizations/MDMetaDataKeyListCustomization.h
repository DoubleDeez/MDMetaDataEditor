// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

// Customization to flatten the FMDMetaDataKeyList with the its internal array
class FMDMetaDataKeyListCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FMDMetaDataKeyListCustomization>(); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
};
