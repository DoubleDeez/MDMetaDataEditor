// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Kismet2/StructureEditorUtils.h"
#include "UObject/ObjectKey.h"

struct FMDMetaDataEditorCachedStructMetadata
{
	int32 Count = 0;
	TMap<FName, FString> StructMetadata;
	TMap<FName, TMap<FName, FString>> PropertyMetadata;
};

// Structs recreate their properties when compiled (just like classes) so this object will cache the metadata before compiling and
// then restore the metadata afterward (similarly to UMDMetaDataEditorBlueprintCompilerExtension for function metadata)
class FMDMetaDataEditorStructChangeHandler : public FStructureEditorUtils::INotifyOnStructChanged, public TSharedFromThis<FMDMetaDataEditorStructChangeHandler>
{
public:
	virtual void PreChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;
	virtual void PostChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;

	TMap<TObjectKey<UUserDefinedStruct>, FMDMetaDataEditorCachedStructMetadata> CachedStructMetadata;
};
