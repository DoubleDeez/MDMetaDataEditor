// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorBlueprintCompilerExtension.h"

#include "K2Node_FunctionEntry.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UMDMetaDataEditorBlueprintCompilerExtension::ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data)
{
	Super::ProcessBlueprintCompiled(CompilationContext, Data);

	if (!IsValid(CompilationContext.Blueprint))
	{
		return;
	}

	for (UEdGraph* Graph : CompilationContext.Blueprint->FunctionGraphs)
	{
		for (UEdGraphNode* GraphNode : Graph->Nodes)
		{
			const UK2Node_FunctionEntry* FunctionNode = Cast<UK2Node_FunctionEntry>(GraphNode);
			const UFunction* Function = FFunctionFromNodeHelper::FunctionFromNode(FunctionNode);
			if (!IsValid(Function))
			{
				continue;
			}

			for (const FBPVariableDescription& Variable : FunctionNode->LocalVariables)
			{
				if (Variable.MetaDataArray.IsEmpty())
				{
					continue;
				}

				for (TFieldIterator<FProperty> It(Function); It; ++It)
				{
					FProperty* Prop = *It;
					if (Prop != nullptr && Prop->GetFName() == Variable.VarName)
					{
						for (const FBPVariableMetaDataEntry& Entry : Variable.MetaDataArray)
						{
							Prop->SetMetaData(Entry.DataKey, *Entry.DataValue);
						}
					}
				}
			}
		}
	}
}
