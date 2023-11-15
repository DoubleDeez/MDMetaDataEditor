// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

class FMDMetaDataEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FDelegateHandle VariableCustomizationHandle;
	FDelegateHandle LocalVariableCustomizationHandle;
	FDelegateHandle FunctionCustomizationHandle;
	FDelegateHandle TunnelCustomizationHandle;
	FDelegateHandle EventCustomizationHandle;
};
