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

bool FMDMetaDataKey::operator==(const FMDMetaDataKey& Other) const
{
	return Key == Other.Key && KeyType == Other.KeyType;
}
