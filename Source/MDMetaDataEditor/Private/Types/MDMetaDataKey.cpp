// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataKey.h"

bool FMDMetaDataKey::DoesSupportBlueprint(const UBlueprint* Blueprint) const
{
	if (!IsValid(Blueprint))
	{
		return false;
	}

	for (const TSoftClassPtr<UBlueprint>& BPClass : SupportedBlueprints)
	{
		if (Blueprint->IsA(BPClass.Get()))
		{
			return true;
		}
	}

	return false;
}

bool FMDMetaDataKey::DoesSupportProperty(const FProperty* Property) const
{
	if (Property == nullptr)
	{
		return false;
	}

	for (const FMDMetaDataEditorPropertyType& PropertyType : SupportedPropertyTypes)
	{
		if (PropertyType.DoesMatchProperty(Property))
		{
			return true;
		}
	}

	return false;
}

FText FMDMetaDataKey::GetKeyDisplayText() const
{
	if (bUseDisplayNameOverride)
	{
		return DisplayNameOverride;
	}
	else
	{
		return FText::FromString(FName::NameToDisplayString(Key.ToString(), false));
	}
}

FText FMDMetaDataKey::GetToolTipText() const
{
	FString ToolTip = Description;
	if (bUseDisplayNameOverride)
	{
		ToolTip += "\r\n\r\nMeta Data Key: \"" + Key.ToString() + "\"";
	}
	return FText::FromString(ToolTip);
}

FText FMDMetaDataKey::GetFilterText() const
{
	FString Filter = Key.ToString();
	if (bUseDisplayNameOverride)
	{
		Filter += TEXT(" ") + DisplayNameOverride.ToString();
	}
	return FText::FromString(Filter);
}

bool FMDMetaDataKey::operator==(const FMDMetaDataKey& Other) const
{
	return Key == Other.Key && KeyType == Other.KeyType;
}
