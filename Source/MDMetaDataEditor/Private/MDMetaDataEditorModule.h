// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Delegates/IDelegateInstance.h"
#include "Modules/ModuleInterface.h"
#include "Templates/SharedPointer.h"

class FMDMetaDataEditorStructChangeHandler;

class FMDMetaDataEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RestartModule();

private:
	void OnAssetEditorOpened(UObject* Asset);

	TSharedPtr<FMDMetaDataEditorStructChangeHandler> StructChangeHandler;

	FDelegateHandle VariableCustomizationHandle;
	FDelegateHandle LocalVariableCustomizationHandle;
	FDelegateHandle FunctionCustomizationHandle;
	FDelegateHandle TunnelCustomizationHandle;
	FDelegateHandle EventCustomizationHandle;
};
