// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorModule.h"

#include "BlueprintEditorModule.h"
#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFunctionCustomization.h"
#include "Customizations/MDMetaDataEditorPropertyTypeCustomization.h"
#include "Customizations/MDMetaDataEditorStructChangeHandler.h"
#include "Customizations/MDMetaDataEditorVariableCustomization.h"
#include "Engine/UserDefinedStruct.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Tunnel.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SMDUserStructMetaDataEditor.h"

void FMDMetaDataEditorModule::StartupModule()
{
	const UMDMetaDataEditorConfig* Config = GetDefault<UMDMetaDataEditorConfig>();

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");

	if (Config->bEnableMetaDataEditorForVariables)
	{
		VariableCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&FMDMetaDataEditorVariableCustomization::MakeInstance));
	}

	if (Config->bEnableMetaDataEditorForLocalVariables)
	{
		LocalVariableCustomizationHandle = BlueprintEditorModule.RegisterLocalVariableCustomization(FProperty::StaticClass(), FOnGetVariableCustomizationInstance::CreateStatic(&FMDMetaDataEditorVariableCustomization::MakeInstance));
	}

	if (Config->bEnableMetaDataEditorForFunctions || Config->bEnableMetaDataEditorForFunctionParameters)
	{
		FunctionCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(UK2Node_FunctionEntry::StaticClass(), FOnGetFunctionCustomizationInstance::CreateStatic(&FMDMetaDataEditorFunctionCustomization::MakeInstance));
	}

	if (Config->bEnableMetaDataEditorForTunnels)
	{
		TunnelCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(UK2Node_Tunnel::StaticClass(), FOnGetFunctionCustomizationInstance::CreateStatic(&FMDMetaDataEditorFunctionCustomization::MakeInstance));
	}

	if (Config->bEnableMetaDataEditorForCustomEvents || Config->bEnableMetaDataEditorForFunctionParameters)
	{
		EventCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(UK2Node_CustomEvent::StaticClass(), FOnGetFunctionCustomizationInstance::CreateStatic(&FMDMetaDataEditorFunctionCustomization::MakeInstance));
	}

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FMDMetaDataEditorPropertyType::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMDMetaDataEditorPropertyTypeCustomization::MakeInstance));

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorOpened().AddRaw(this, &FMDMetaDataEditorModule::OnAssetEditorOpened);

	if (Config->bEnableMetaDataEditorForStructs)
	{
		StructChangeHandler = MakeShared<FMDMetaDataEditorStructChangeHandler>();
	}
}

void FMDMetaDataEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SMDUserStructMetaDataEditor::TabId);

	if (FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet"))
	{
		BlueprintEditorModule->UnregisterVariableCustomization(FProperty::StaticClass(), VariableCustomizationHandle);
		BlueprintEditorModule->UnregisterLocalVariableCustomization(FProperty::StaticClass(), LocalVariableCustomizationHandle);
		BlueprintEditorModule->UnregisterFunctionCustomization(UK2Node_FunctionEntry::StaticClass(), FunctionCustomizationHandle);
		BlueprintEditorModule->UnregisterFunctionCustomization(UK2Node_Tunnel::StaticClass(), TunnelCustomizationHandle);
		BlueprintEditorModule->UnregisterFunctionCustomization(UK2Node_CustomEvent::StaticClass(), EventCustomizationHandle);
	}

	if (GEditor)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetEditorSubsystem->OnAssetEditorOpened().RemoveAll(this);
		}
	}

	if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyEditorModule->UnregisterCustomPropertyTypeLayout(FMDMetaDataEditorPropertyType::StaticStruct()->GetFName());
	}

	StructChangeHandler.Reset();
}

void FMDMetaDataEditorModule::RestartModule()
{
	ShutdownModule();
	StartupModule();
}

void FMDMetaDataEditorModule::OnAssetEditorOpened(UObject* Asset)
{
	UUserDefinedStruct* UserDefinedStruct = Cast<UUserDefinedStruct>(Asset);
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!IsValid(AssetEditorSubsystem) || !IsValid(UserDefinedStruct))
	{
		return;
	}

	static const FName StructEditorName = TEXT("UserDefinedStructureEditor");
	constexpr bool bFocus = false;
	IAssetEditorInstance* Editor = AssetEditorSubsystem->FindEditorForAsset(Asset, bFocus);
	if (Editor == nullptr || Editor->GetEditorName() != StructEditorName)
	{
		return;
	}

	FAssetEditorToolkit* Toolkit = static_cast<FAssetEditorToolkit*>(Editor);
	if (const TSharedPtr<FTabManager> TabManager = Toolkit->GetTabManager())
	{
		// Because we register our tab spawn late, we end up with an unrecognized tab getting saved in the layout, clear it out on launch
		const FTabId UnrecognizedId(FName(TEXT("Unrecognized")), ETabIdFlags::None);
		while (TSharedPtr<SDockTab> UnrecognizedTab = TabManager->FindExistingLiveTab(UnrecognizedId))
		{
			UnrecognizedTab->RequestCloseTab();
		}

		if (GetDefault<UMDMetaDataEditorConfig>()->bEnableMetaDataEditorForStructs)
		{
			TabManager->RegisterTabSpawner(SMDUserStructMetaDataEditor::TabId, FOnSpawnTab::CreateStatic(&SMDUserStructMetaDataEditor::CreateStructMetaDataEditorTab, MakeWeakObjectPtr(UserDefinedStruct)))
         		.SetDisplayName(INVTEXT("Metadata"))
         		.SetGroup(Toolkit->GetWorkspaceMenuCategory())
         		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "PropertyEditor.Grid.TabIcon"));

			TabManager->TryInvokeTab(SMDUserStructMetaDataEditor::TabId);
		}
	}
}

IMPLEMENT_MODULE(FMDMetaDataEditorModule, MDMetaDataEditor)
