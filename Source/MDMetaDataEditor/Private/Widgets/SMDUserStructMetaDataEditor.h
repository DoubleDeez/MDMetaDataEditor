// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Engine/UserDefinedStruct.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Misc/NotifyHook.h"
#include "Widgets/SCompoundWidget.h"

class FSpawnTabArgs;
class IDetailsView;
class SDockTab;

class FMDUserStructMetaDataEditorView : public FStructureEditorUtils::INotifyOnStructChanged, public TSharedFromThis<FMDUserStructMetaDataEditorView>
{
public:
	explicit FMDUserStructMetaDataEditorView(UUserDefinedStruct* EditedStruct)
		: UserDefinedStruct(EditedStruct)
	{
	}

	virtual ~FMDUserStructMetaDataEditorView() override
	{
	}

	void Initialize();

	UUserDefinedStruct* GetUserDefinedStruct() const { return UserDefinedStruct.Get(); }
	TSharedPtr<SWidget> GetWidget() const;

	virtual void PreChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;
	virtual void PostChange(const UUserDefinedStruct* Struct, FStructureEditorUtils::EStructureEditorChangeInfo Info) override;

private:
	/** Struct on scope data that is being viewed in the details panel */
	TSharedPtr<FStructOnScope> StructData;

	/** Details view being used for viewing the struct */
	TSharedPtr<IDetailsView> DetailsView;

	/** User defined struct that is being represented */
	const TWeakObjectPtr<UUserDefinedStruct> UserDefinedStruct;
};

class SMDUserStructMetaDataEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMDUserStructMetaDataEditor)
		{}

		SLATE_ARGUMENT(TWeakObjectPtr<UUserDefinedStruct>, UserDefinedStruct);

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void UpdateStruct(UUserDefinedStruct* UserDefinedStruct);

	static const FName TabId;

	static TSharedRef<SDockTab> CreateStructMetaDataEditorTab(const FSpawnTabArgs& TabArgs, TWeakObjectPtr<UUserDefinedStruct> UserDefinedStruct);

private:
	TWeakObjectPtr<UUserDefinedStruct> UserDefinedStructPtr;
	TSharedPtr<FMDUserStructMetaDataEditorView> MetaDataEditorView;
};
