// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Delegates/IDelegateInstance.h"
#include "Modules/ModuleInterface.h"

class FMDMetaDataEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RestartModule();

private:
	FDelegateHandle VariableCustomizationHandle;
	FDelegateHandle LocalVariableCustomizationHandle;
	FDelegateHandle FunctionCustomizationHandle;
	FDelegateHandle TunnelCustomizationHandle;
	FDelegateHandle EventCustomizationHandle;
};
