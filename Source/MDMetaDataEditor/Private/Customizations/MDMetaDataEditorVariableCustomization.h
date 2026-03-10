// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "MDMetaDataEditorCustomizationBase.h"

class FMDMetaDataEditorFieldView;

enum class EMDPropertyVisibility : uint8
{
	VisibleDefaultsOnly,  // CPF_EditConst | CPF_DisableEditOnInstance
	VisibleInstanceOnly,  // CPF_EditConst | CPF_DisableEditOnTemplate
	VisibleAnywhere,      // CPF_EditConst
	EditDefaultsOnly,     // CPF_DisableEditOnInstance
	EditInstanceOnly,     // CPF_DisableEditOnTemplate
	EditAnywhere,         // None
	HiddenEverywhere,     // CPF_DisableEditOnTemplate | CPF_DisableEditOnInstance
};

class FMDMetaDataEditorVariableCustomization : public FMDMetaDataEditorCustomizationBase
{
public:
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor);

	FMDMetaDataEditorVariableCustomization(const TWeakPtr<IBlueprintEditor>& BlueprintEditor, TWeakObjectPtr<UBlueprint>&& BlueprintPtr)
		: FMDMetaDataEditorCustomizationBase(BlueprintEditor, MoveTemp(BlueprintPtr))
	{
	}

	virtual void CustomizeObject(IDetailLayoutBuilder& DetailLayout, UObject* Obj) override;

private:
	TSharedPtr<FMDMetaDataEditorFieldView> VariableFieldView;

	/** Cached property for the variable we are affecting */
	TWeakFieldPtr<FProperty> CachedVariableProperty;
	
	TArray<TSharedPtr<EMDPropertyVisibility>> VisibilityOptions;

	// When displaying variable on child blueprint and is not editable.
	bool bIsReadOnly = false;

	TSharedRef<SWidget> GenerateVisibilityOptionWidget(TSharedPtr<EMDPropertyVisibility> Option) const;

	uint64 GetCurrentPropertyFlags() const;
	

	EMDPropertyVisibility GetCurrentVisibility() const;
	void ApplyVisibility(EMDPropertyVisibility NewVisibility);

	ECheckBoxState GetVisibleOnClassDefaultsState() const;
	void OnVisibleOnClassDefaultsChanged(ECheckBoxState NewState);

	ECheckBoxState GetVisibleOnInstancesState() const;
	void OnVisibleOnInstancesChanged(ECheckBoxState NewState);

	bool IsEditableCheckboxEnabled() const;
	ECheckBoxState GetEditableState() const;
	void OnEditableChanged(ECheckBoxState NewState);
	
	void ChangeVisibilityFlag(uint64 FlagMask, bool bSet, const FText& TransactionLabel);
};
