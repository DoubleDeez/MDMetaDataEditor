// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "EdGraphSchema_K2.h"
#include "Engine/DeveloperSettings.h"
#include "Types/MDMetaDataEditorPropertyType.h"
#include "Types/MDMetaDataKey.h"

#include "MDMetaDataEditorConfig.generated.h"


USTRUCT()
struct FMDMetaDataEditorBlueprintConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TSoftClassPtr<UBlueprint> BlueprintType;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TMap<FMDMetaDataEditorPropertyType, FMDMetaDataKeyList> VariableMetaDataKeys;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FMDMetaDataKeyList FunctionMetaDataKeys;
};

/**
 * Configure which meta data keys will display on blueprint Properties and Functions.
 * Can be setup per Blueprint type and Property type.
 */
UCLASS(DefaultConfig, Config = Editor, MinimalAPI)
class UMDMetaDataEditorConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMDMetaDataEditorConfig();

	virtual FText GetSectionText() const override;

	void ForEachVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const;
	void ForEachFunctionMetaDataKey(const UBlueprint* Blueprint, const TFunction<void(const FMDMetaDataKey&)>& Func) const;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Editor", meta = (TitleProperty = "ConfigRestartRequired"))
	bool bEnableMetaDataEditorForVariables = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Editor", meta = (TitleProperty = "ConfigRestartRequired"))
	bool bEnableMetaDataEditorForLocalVariables = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Editor", meta = (TitleProperty = "ConfigRestartRequired"))
	bool bEnableMetaDataEditorForFunctions = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Editor", meta = (TitleProperty = "ConfigRestartRequired"))
	bool bEnableMetaDataEditorForCustomEvents = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Editor", DisplayName = "Enable Meta Data Editor For Collapsed Graphs", meta = (TitleProperty = "ConfigRestartRequired"))
	bool bEnableMetaDataEditorForTunnels = true;

private:
	UPROPERTY(EditDefaultsOnly, Config, Category = "Meta Data Keys", meta = (TitleProperty = "BlueprintType"))
	TArray<FMDMetaDataEditorBlueprintConfig> BlueprintMetaDataConfigs;
};
