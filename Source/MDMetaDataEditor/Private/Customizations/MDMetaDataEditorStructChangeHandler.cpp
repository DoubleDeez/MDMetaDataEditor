// Copyright Dylan Dumesnil. All Rights Reserved.


#include "MDMetaDataEditorStructChangeHandler.h"

#include "UObject/MetaData.h"
#include "UObject/Package.h"

void FMDMetaDataEditorStructChangeHandler::PreChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
	if (IsValid(Struct))
	{
		FMDMetaDataEditorCachedStructMetadata& Cache = CachedStructMetadata.FindOrAdd(Struct);
		if (Cache.Count++ == 0)
		{
			if (TMap<FName, FString>* MetaDataMap = UMetaData::GetMapForObject(Struct))
			{
				Cache.StructMetadata = *MetaDataMap;
			}

			for (TFieldIterator<FProperty> PropertyIter(Struct); PropertyIter; ++PropertyIter)
			{
				if (const TMap<FName, FString>* PropertyMetaDataMap = PropertyIter->GetMetaDataMap())
				{
					Cache.PropertyMetadata.FindOrAdd(PropertyIter->GetFName()) = *PropertyMetaDataMap;
				}
			}
		}
	}
}

void FMDMetaDataEditorStructChangeHandler::PostChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info)
{
	if (IsValid(Struct))
	{
		if (FMDMetaDataEditorCachedStructMetadata* Cache = CachedStructMetadata.Find(Struct))
		{
			if (--Cache->Count == 0)
			{
				TMap<FName, FString>& MetaDataMap = Struct->GetOutermost()->GetMetaData()->ObjectMetaDataMap.FindOrAdd(Struct);
				MetaDataMap.Append(MoveTemp(Cache->StructMetadata));

				for (TFieldIterator<FProperty> PropertyIter(Struct); PropertyIter; ++PropertyIter)
				{
					if (TMap<FName, FString>* PropertyMetaDataMap = Cache->PropertyMetadata.Find(PropertyIter->GetFName()))
					{
						PropertyIter->AppendMetaData(*PropertyMetaDataMap);
					}
				}

				CachedStructMetadata.Remove(Struct);
			}
		}
	}
}
