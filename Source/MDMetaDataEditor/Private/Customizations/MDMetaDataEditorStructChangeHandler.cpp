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
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6) // On or after UE 5.6
			if (TMap<FName, FString>* MetaDataMap = FMetaData::GetMapForObject(Struct))
#else // Pre UE 5.6
			if (TMap<FName, FString>* MetaDataMap = UMetaData::GetMapForObject(Struct))
#endif
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
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6) // On or after UE 5.6
				TMap<FName, FString>& MetaDataMap = Struct->GetOutermost()->GetMetaData().ObjectMetaDataMap.FindOrAdd(Struct);
#else // Pre UE 5.6
				TMap<FName, FString>& MetaDataMap = Struct->GetOutermost()->GetMetaData()->ObjectMetaDataMap.FindOrAdd(Struct);
#endif
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
