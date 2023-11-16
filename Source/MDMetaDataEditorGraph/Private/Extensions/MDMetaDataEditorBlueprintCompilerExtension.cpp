// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorBlueprintCompilerExtension.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "K2Node_FunctionEntry.h"

namespace MDMDEBCE_Private
{
	FProperty* FindNodePropertyInClass(const UK2Node_FunctionEntry& FunctionNode, const FName& PropertyName, const UClass* Class)
	{
		const FName FunctionName = (FunctionNode.CustomGeneratedFunctionName != NAME_None)
			? FunctionNode.CustomGeneratedFunctionName : FunctionNode.GetGraph()->GetFName();
		const UFunction*Function = Class->FindFunctionByName(FunctionName);

		if (!IsValid(Function))
		{
			return nullptr;
		}

		// Can't use Function->FindPropertyByName here since Function->PropertyLink is null
		for (FField* Field = Function->ChildProperties; Field != nullptr; Field = Field->Next)
		{
			if (Field->GetFName() == PropertyName)
			{
				return CastField<FProperty>(Field);
			}
		}

		return nullptr;
	}

	// Inits meta data on function-related properties (params & variables) that the engine doesn't already support
	void InitFunctionNodeMetaData(const UK2Node_FunctionEntry& FunctionNode, const UClass* OldClass, const UBlueprint* Blueprint)
	{
		auto InitFunctionMetaData = [&FunctionNode, &OldClass](const UFunction* Function)
		{
			if (!IsValid(Function))
			{
				return;
			}

			// Only functions created in the blueprint can have meta data set
			const UFunction* SuperFunc = Function->GetSuperFunction();
			while (IsValid(SuperFunc))
			{
				if (!SuperFunc->GetOuter()->IsA<UBlueprintGeneratedClass>())
				{
					return;
				}

				SuperFunc = SuperFunc->GetSuperFunction();
			}

			for (TFieldIterator<FProperty> It(Function); It; ++It)
			{
				FProperty* Prop = *It;
				if (Prop == nullptr)
				{
					continue;
				}

				if (Prop->HasAnyPropertyFlags(CPF_Parm))
				{
					if (IsValid(OldClass))
					{
						if (const FProperty* OldProperty = FindNodePropertyInClass(FunctionNode, Prop->GetFName(), OldClass))
						{
							FField::CopyMetaData(OldProperty, Prop);
						}
					}
				}
				else
				{
					for (const FBPVariableDescription& Variable : FunctionNode.LocalVariables)
					{
						if (Variable.MetaDataArray.IsEmpty())
						{
							continue;
						}

						if (Prop->GetFName() == Variable.VarName)
						{
							for (const FBPVariableMetaDataEntry& Entry : Variable.MetaDataArray)
							{
								Prop->SetMetaData(Entry.DataKey, *Entry.DataValue);
							}
						}
					}
				}
			}
		};

		const FName FunctionName = (FunctionNode.CustomGeneratedFunctionName != NAME_None)
			? FunctionNode.CustomGeneratedFunctionName : FunctionNode.GetGraph()->GetFName();
		const UFunction* Function = IsValid(Blueprint->GeneratedClass) ? Blueprint->GeneratedClass->FindFunctionByName(FunctionName) : nullptr;
		const UFunction* SkeletonFunction = IsValid(Blueprint->SkeletonGeneratedClass) ? Blueprint->SkeletonGeneratedClass->FindFunctionByName(FunctionName) : nullptr;
		InitFunctionMetaData(Function);
		InitFunctionMetaData(SkeletonFunction);
	}
}

void UMDMetaDataEditorBlueprintCompilerExtension::ProcessBlueprintCompiled(const FKismetCompilerContext& CompilationContext, const FBlueprintCompiledData& Data)
{
	Super::ProcessBlueprintCompiled(CompilationContext, Data);

	if (!IsValid(CompilationContext.Blueprint))
	{
		return;
	}

	TArray<TObjectPtr<UEdGraph>> Graphs = CompilationContext.Blueprint->FunctionGraphs;
	Graphs.Append(CompilationContext.Blueprint->EventGraphs);

	for (UEdGraph* Graph : Graphs)
	{
		for (UEdGraphNode* GraphNode : Graph->Nodes)
		{
			const UK2Node_FunctionEntry* FunctionNode = Cast<UK2Node_FunctionEntry>(GraphNode);
			if (!IsValid(FunctionNode))
			{
				continue;
			}

			MDMDEBCE_Private::InitFunctionNodeMetaData(*FunctionNode, CompilationContext.OldClass, CompilationContext.Blueprint);
		}
	}
}
