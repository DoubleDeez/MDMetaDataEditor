// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "BlueprintCompilerExtension.h"
#include "MDMetaDataEditorBlueprintCompilerExtension.generated.h"


/**
 * Compiler extension to copy meta data into function local variable properties
 */
UCLASS()
class UMDMetaDataEditorBlueprintCompilerExtension : public UBlueprintCompilerExtension
{
	GENERATED_BODY()

public:
	virtual void ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data) override;
};
