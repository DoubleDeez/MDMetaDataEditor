// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataKey.h"

bool FMDMetaDataKey::operator==(const FMDMetaDataKey& Other) const
{
	return Key == Other.Key && KeyType == Other.KeyType;
}
