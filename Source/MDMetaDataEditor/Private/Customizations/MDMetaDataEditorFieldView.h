// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Containers/Union.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "SCheckBoxList.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakFieldPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UK2Node_EditablePinBase;
class IDetailCategoryBuilder;
class IDetailLayoutBuilder;
struct FMDMetaDataKey;
struct FKismetUserDeclaredFunctionMetadata;

namespace ETextCommit
{
	enum Type : int;
}

class IDetailGroup;
class UUserDefinedStruct;
class UBlueprint;
class UK2Node_FunctionEntry;
class UK2Node_Tunnel;
class UK2Node_CustomEvent;

enum class EMDMetaDataEditorFieldType : uint8
{
	Unknown,
	Variable,
	LocalVariable,
	FunctionParamInput,
	FunctionParamOutput,
	StructProperty,
	Function,
	Tunnel,
	CustomEvent,
	Struct
};

class MDMETADATAEDITOR_API FMDMetaDataEditorFieldView : public TSharedFromThis<FMDMetaDataEditorFieldView>
{
public:
	FMDMetaDataEditorFieldView(FProperty* InProperty, FProperty* InSkelProperty, UBlueprint* InBlueprint);
	FMDMetaDataEditorFieldView(FProperty* InProperty, UUserDefinedStruct* InUserDefinedStruct);
	FMDMetaDataEditorFieldView(FProperty* InProperty, FProperty* InSkelProperty, UK2Node_EditablePinBase* InNode);
	explicit FMDMetaDataEditorFieldView(UUserDefinedStruct* InUserDefinedStruct);
	FMDMetaDataEditorFieldView(UK2Node_FunctionEntry* InFunctionEntry, UBlueprint* InBlueprint);
	FMDMetaDataEditorFieldView(UK2Node_Tunnel* InTunnel, UBlueprint* InBlueprint);
	FMDMetaDataEditorFieldView(UK2Node_CustomEvent* InCustomEvent, UBlueprint* InBlueprint);

	static const FString MultipleValues;

	const TMap<FName, FString>* GetMetadataMap() const;
	FKismetUserDeclaredFunctionMetadata* GetFunctionMetadataWithModify() const;

	void GenerateMetadataEditor(IDetailLayoutBuilder& DetailLayout, TMap<FName, IDetailGroup*>& GroupMap);

	typedef TUnion<IDetailCategoryBuilder*, IDetailGroup*> FMDMetadataBuilderRow;
	FMDMetadataBuilderRow InitCategories(IDetailLayoutBuilder& DetailLayout, TMap<FName, IDetailGroup*>& GroupMap);

	void AddMetadataValueEditor(const TFunctionRef<void(const FMDMetaDataKey&)>& Func);
	void AddMetadataValueEditor(const FMDMetaDataKey& Key, FMDMetadataBuilderRow BuilderRow, TMap<FName, IDetailGroup*>& GroupMap);
	void AddRawMetadataEditor(FMDMetadataBuilderRow BuilderRow);

	bool IsConfigEnabled() const;

	FSimpleDelegate RequestRefresh;

private:
	TSharedRef<SWidget> CreateMetaDataValueWidget(const FMDMetaDataKey& Key);

	EVisibility GetRemoveMetadataButtonVisibility(FName Key) const;
	FReply OnRemoveMetadata(FName Key);

	void OnMetadataKeyTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName OldKey);

	FText GetMetadataValueText(FName Key) const;
	void OnMetadataValueTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit, FName Key);
	void OnMetadataValueTextCommittedAllowingEmpty(const FText& NewText, ETextCommit::Type InTextCommit, FName Key);

	TOptional<int32> GetMetadataValueInt(FName Key) const;
	void OnMetadataValueIntCommitted(int32 Value, ETextCommit::Type InTextCommit, FName Key);

	TOptional<float> GetMetadataValueFloat(FName Key) const;
	void OnMetadataValueFloatCommitted(float Value, ETextCommit::Type InTextCommit, FName Key);

	template<bool bIsBoolean>
	ECheckBoxState IsChecked(FName Key) const;
	template<bool bIsBoolean>
	void HandleChecked(ECheckBoxState State, FName Key);
	template<bool bIsBoolean>
	FText GetCheckBoxToolTip(FName Key) const;

	void AddMetadataKey(const FName& Key);
	void SetMetadataValue(const FName& Key, const FString& Value);
	bool HasMetadataValue(const FName& Key) const;
	TOptional<FString> GetMetadataValue(FName Key) const;
	void SetMetadataKey(const FName& OldKey, const FName& NewKey);
	void RemoveMetadataKey(const FName& Key);

	void CopyMetadata(FName Key) const;
	bool CanCopyMetadata(FName Key) const;
	void PasteMetadata(FName Key);
	bool CanPasteMetadata(FName Key) const;

	TWeakFieldPtr<FProperty> MetadataProperty;
	TWeakFieldPtr<FProperty> MetadataSkeletonProperty;
	TWeakObjectPtr<UUserDefinedStruct> MetadataStruct;
	TWeakObjectPtr<UK2Node_FunctionEntry> MetadataFunctionEntry;
	TWeakObjectPtr<UK2Node_Tunnel> MetadataTunnel;
	TWeakObjectPtr<UK2Node_CustomEvent> MetadataCustomEvent;
	TWeakObjectPtr<UBlueprint> BlueprintPtr;

	bool bIsReadOnly = false;
	EMDMetaDataEditorFieldType FieldType = EMDMetaDataEditorFieldType::Unknown;
};
