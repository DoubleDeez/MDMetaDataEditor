// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "Runtime/Launch/Resources/Version.h"
#include "UObject/WeakFieldPtr.h"

class IDetailGroup;

namespace ETextCommit
{
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 2
	enum Type : int;
#else
	enum Type;
#endif
}

class IBlueprintEditor;
class SWidget;
class UBlueprint;
class UK2Node_CustomEvent;
class UK2Node_FunctionEntry;
class UK2Node_Tunnel;
enum class ECheckBoxState : uint8;
struct FMDMetaDataKey;

class FMDMetaDataEditorCustomizationBase : public IDetailCustomization
{
public:
	FMDMetaDataEditorCustomizationBase(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr);

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// This version doesn't actually get called (see FBlueprintGraphActionDetails::CustomizeDetails and https://github.com/EpicGames/UnrealEngine/pull/11137)
	// virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;

	UBlueprint* GetBlueprint() const { return BlueprintPtr.Get(); }

	virtual void RefreshDetails();

private:
	virtual void CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj) = 0;

	TWeakPtr<IBlueprintEditor> BlueprintEditor;
	TWeakObjectPtr<UBlueprint> BlueprintPtr;

	IDetailLayoutBuilder* DetailBuilderPtr = nullptr;
};
