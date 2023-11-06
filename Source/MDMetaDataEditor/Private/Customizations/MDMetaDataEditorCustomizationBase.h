// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "UObject/WeakFieldPtr.h"

namespace ETextCommit
{
	enum Type : int;
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

private:
	EVisibility GetRemoveMetaDataButtonVisibility(FName Key) const;
	FReply OnRemoveMetaData(FName Key);

	TSharedRef<SWidget> CreateMetaDataValueWidget(const FMDMetaDataKey& Key);

	void AddMetaDataKey(const FName& Key);
	void SetMetaDataValue(const FName& Key, const FString& Value);
	bool HasMetaDataValue(const FName& Key) const;
	TOptional<FString> GetMetaDataValue(FName Key) const;
	void RemoveMetaDataKey(const FName& Key);

	template<bool bIsBoolean>
	ECheckBoxState IsChecked(FName Key) const;
	template<bool bIsBoolean>
	void HandleChecked(ECheckBoxState State, FName Key);
	template<bool bIsBoolean>
	FText GetCheckBoxToolTip(FName Key) const;

	FText GetMetaDataValueText(FName Key) const;
	void OnMetaDataValueTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName Key);

	TOptional<int32> GetMetaDataValueInt(FName Key) const;
	void OnMetaDataValueIntCommitted(int32 Value, ETextCommit::Type InTextCommit, FName Key);

	TOptional<float> GetMetaDataValueFloat(FName Key) const;
	void OnMetaDataValueFloatCommitted(float Value, ETextCommit::Type InTextCommit, FName Key);

	TWeakPtr<IBlueprintEditor> BlueprintEditor;
	TWeakObjectPtr<UBlueprint> BlueprintPtr;

	TWeakFieldPtr<FProperty> PropertyBeingCustomized;
	TWeakObjectPtr<UK2Node_FunctionEntry> FunctionBeingCustomized;
	TWeakObjectPtr<UK2Node_Tunnel> TunnelBeingCustomized;
	TWeakObjectPtr<UK2Node_CustomEvent> EventBeingCustomized;
};
