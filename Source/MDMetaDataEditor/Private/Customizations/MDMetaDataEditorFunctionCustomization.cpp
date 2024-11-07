// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorFunctionCustomization.h"

#include "BlueprintEditorModule.h"
#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFieldView.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_Event.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Tunnel.h"
#include "Kismet2/BlueprintEditorUtils.h"

namespace MDMDEFC_Private
{
	template<typename T, bool bExact>
	T* FindNode(UObject* Object)
	{
		if (T* Node = Cast<T>(Object))
		{
			return Node;
		}
		if (UEdGraph* Graph = Cast<UEdGraph>(Object))
		{
			for (UEdGraphNode* GraphNode : Graph->Nodes)
			{
				if constexpr (bExact)
				{
					if (T* Node = ExactCast<T>(GraphNode))
					{
						return Node;
					}
				}
				else
				{
					if (T* Node = Cast<T>(GraphNode))
					{
						return Node;
					}
				}
			}
		}
		return nullptr;
	}

	UK2Node_FunctionEntry* FindFunctionNode(const UBlueprint* Blueprint, const UFunction* Function)
	{
		if (!IsValid(Blueprint) || !IsValid(Function))
		{
			return nullptr;
		}

		for (UEdGraph* Graph : Blueprint->FunctionGraphs)
		{
			UK2Node_FunctionEntry* FunctionEntry = FindNode<UK2Node_FunctionEntry, false>(Graph);
			if (IsValid(FunctionEntry) && FFunctionFromNodeHelper::FunctionFromNode(FunctionEntry) == Function)
			{
				return FunctionEntry;
			}
		}

		return nullptr;
	}

	FProperty* FindNodeProperty(const UK2Node_EditablePinBase* Node, const TSharedPtr<FUserPinInfo>& PinInfo, bool bUseSkelClass)
	{
		// Specifically grab the generated class, not the skeleton class so that UMDMetaDataEditorBlueprintCompilerExtension can grab the meta data after the BP is compiled

		const UBlueprint* Blueprint = IsValid(Node) ? Node->GetBlueprint() : nullptr;
		const UClass* Class = IsValid(Blueprint) ? (bUseSkelClass ? Blueprint->SkeletonGeneratedClass : Blueprint->GeneratedClass) : nullptr;
		const UFunction* Function = nullptr;

		if (const UK2Node_FunctionResult* ResultNode = Cast<UK2Node_FunctionResult>(Node))
		{
			// Function result nodes cannot resolve the UFunction, so find the entry node and use that for finding the UFunction
			TArray<UK2Node_FunctionEntry*> EntryNodes;
			ResultNode->GetGraph()->GetNodesOfClass(EntryNodes);
			Node = EntryNodes[0];
		}

		if (const UK2Node_FunctionEntry* FunctionNode = Cast<UK2Node_FunctionEntry>(Node))
		{
			const FName FunctionName = (FunctionNode->CustomGeneratedFunctionName != NAME_None) ? FunctionNode->CustomGeneratedFunctionName : FunctionNode->GetGraph()->GetFName();
			Function = Class->FindFunctionByName(FunctionName);
		}
		else if (const UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
		{
			FName SearchName = EventNode->EventReference.GetMemberName();
			if (SearchName.IsNone())
			{
				SearchName = EventNode->CustomFunctionName;
			}
			Function = Class->FindFunctionByName(SearchName);
		}

		if (!IsValid(Function) || !PinInfo.IsValid())
		{
			return nullptr;
		}

		return Function->FindPropertyByName(PinInfo->PinName);
	}
}

TSharedPtr<IDetailCustomization> FMDMetaDataEditorFunctionCustomization::MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor)
{
	const TArray<UObject*>* Objects = (BlueprintEditor.IsValid() ? BlueprintEditor->GetObjectsCurrentlyBeingEdited() : nullptr);
	if (Objects != nullptr && Objects->Num() == 1)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>((*Objects)[0]))
		{
			return MakeShared<FMDMetaDataEditorFunctionCustomization>(BlueprintEditor, MakeWeakObjectPtr(Blueprint));
		}
	}

	return nullptr;
}

void FMDMetaDataEditorFunctionCustomization::CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj)
{
	// Put Metadata above Inputs for Functions
	InitFieldViews(Obj);

	TMap<FName, IDetailGroup*> GroupMap;
	if (FunctionFieldView.IsValid())
	{
		if (FunctionFieldView->IsConfigEnabled())
		{
			FunctionFieldView->GenerateMetadataEditor(DetailLayout, GroupMap);
		}
		else
		{
			FunctionFieldView->InitCategories(DetailLayout, GroupMap);
		}
	}

	for (const TSharedPtr<FMDMetaDataEditorFieldView>& ParamFieldView : ParamFieldViews)
	{
		if (ParamFieldView.IsValid())
		{
			ParamFieldView->GenerateMetadataEditor(DetailLayout, GroupMap);
		}
	}
}

void FMDMetaDataEditorFunctionCustomization::InitFieldViews(UObject* Obj)
{
	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();

	UK2Node_EditablePinBase* Node = nullptr;
	if (UK2Node_FunctionEntry* Function = MDMDEFC_Private::FindNode<UK2Node_FunctionEntry, false>(Obj))
	{
		Node = Function;
		FunctionFieldView = MakeShared<FMDMetaDataEditorFieldView>(Function, GetBlueprint());
	}
	else if (UK2Node_Tunnel* Tunnel = MDMDEFC_Private::FindNode<UK2Node_Tunnel, true>(Obj))
	{
		Node = Tunnel;
		FunctionFieldView = MakeShared<FMDMetaDataEditorFieldView>(Tunnel, GetBlueprint());
	}
	else if (UK2Node_CustomEvent* Event = MDMDEFC_Private::FindNode<UK2Node_CustomEvent, false>(Obj))
	{
		Node = Event;
		FunctionFieldView = MakeShared<FMDMetaDataEditorFieldView>(Event, GetBlueprint());
	}

	if (FunctionFieldView.IsValid())
	{
		FunctionFieldView->RequestRefresh.BindSP(this, &FMDMetaDataEditorFunctionCustomization::RefreshDetails);
	}

	ParamFieldViews.Reset();

	if (IsValid(Node) && Config->bEnableMetaDataEditorForFunctionParameters)
	{
		for (const TSharedPtr<FUserPinInfo>& PinInfo : Node->UserDefinedPins)
		{
			FProperty* ParamProperty = MDMDEFC_Private::FindNodeProperty(Node, PinInfo, false);
			if (ParamProperty == nullptr)
			{
				continue;
			}
			FProperty* SkeletonProperty = MDMDEFC_Private::FindNodeProperty(Node, PinInfo, true);

			TSharedPtr<FMDMetaDataEditorFieldView> ParamFieldView = MakeShared<FMDMetaDataEditorFieldView>(ParamProperty, SkeletonProperty, Node);
			ParamFieldView->RequestRefresh.BindSP(this, &FMDMetaDataEditorFunctionCustomization::RefreshDetails);
			ParamFieldViews.Emplace(MoveTemp(ParamFieldView));
		}
	}
}
