// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDMetaDataEditorModule.h"

#include "BlueprintEditorModule.h"
#include "Config/MDMetaDataEditorConfig.h"
#include "Customizations/MDMetaDataEditorFunctionCustomization.h"
#include "Customizations/MDMetaDataEditorPropertyTypeCustomization.h"
#include "Customizations/MDMetaDataEditorVariableCustomization.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Tunnel.h"
#include "PropertyEditorModule.h"

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

	if (Config->bEnableMetaDataEditorForFunctions)
	{
		FunctionCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(UK2Node_FunctionEntry::StaticClass(), FOnGetFunctionCustomizationInstance::CreateStatic(&FMDMetaDataEditorFunctionCustomization::MakeInstance));
	}

	if (Config->bEnableMetaDataEditorForTunnels)
	{
		TunnelCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(UK2Node_Tunnel::StaticClass(), FOnGetFunctionCustomizationInstance::CreateStatic(&FMDMetaDataEditorFunctionCustomization::MakeInstance));
	}

	if (Config->bEnableMetaDataEditorForCustomEvents)
	{
		EventCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(UK2Node_CustomEvent::StaticClass(), FOnGetFunctionCustomizationInstance::CreateStatic(&FMDMetaDataEditorFunctionCustomization::MakeInstance));
	}

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FMDMetaDataEditorPropertyType::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FMDMetaDataEditorPropertyTypeCustomization::MakeInstance));
}

void FMDMetaDataEditorModule::ShutdownModule()
{
	if (FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet"))
	{
		BlueprintEditorModule->UnregisterVariableCustomization(FProperty::StaticClass(), VariableCustomizationHandle);
		BlueprintEditorModule->UnregisterLocalVariableCustomization(FProperty::StaticClass(), LocalVariableCustomizationHandle);
		BlueprintEditorModule->UnregisterFunctionCustomization(UK2Node_FunctionEntry::StaticClass(), FunctionCustomizationHandle);
		BlueprintEditorModule->UnregisterFunctionCustomization(UK2Node_Tunnel::StaticClass(), TunnelCustomizationHandle);
		BlueprintEditorModule->UnregisterFunctionCustomization(UK2Node_CustomEvent::StaticClass(), EventCustomizationHandle);
	}

	if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyEditorModule->UnregisterCustomPropertyTypeLayout(FMDMetaDataEditorPropertyType::StaticStruct()->GetFName());
	}
}

IMPLEMENT_MODULE(FMDMetaDataEditorModule, MDMetaDataEditor)
