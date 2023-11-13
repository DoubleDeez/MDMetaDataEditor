// Copyright Dylan Dumesnil. All Rights Reserved.

#include "BlueprintCompilationManager.h"
#include "Extensions/MDMetaDataEditorBlueprintCompilerExtension.h"
#include "Modules/ModuleManager.h"


class FMDMetaDataEditorGraphModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		CompilerExtensionPtr = NewObject<UMDMetaDataEditorBlueprintCompilerExtension>();
		CompilerExtensionPtr->AddToRoot();

		FBlueprintCompilationManager::RegisterCompilerExtension(UBlueprint::StaticClass(), CompilerExtensionPtr.Get());
	}

	virtual void ShutdownModule() override
	{
		if (UMDMetaDataEditorBlueprintCompilerExtension* CompilerExtension = CompilerExtensionPtr.Get())
		{
			CompilerExtension->RemoveFromRoot();
			CompilerExtension = nullptr;
		}
	}

private:
	// Use a weak ptr even though we add it to root, it can be destroyed before the module shuts down
	TWeakObjectPtr<UMDMetaDataEditorBlueprintCompilerExtension> CompilerExtensionPtr = nullptr;
};

IMPLEMENT_MODULE(FMDMetaDataEditorGraphModule, MDMetaDataEditorGraph)