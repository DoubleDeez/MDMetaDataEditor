// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "EdGraphSchema_K2.h"
#include "Engine/DeveloperSettings.h"
#include "Types/MDMetaDataKey.h"

#include "MDMetaDataEditorConfig.generated.h"

/**
 * Configure which metadata keys will display on blueprint Properties and Functions.
 * Can be setup per Blueprint type and Property type.
 */
UCLASS(DefaultConfig, Config = Editor, MinimalAPI)
class UMDMetaDataEditorConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMDMetaDataEditorConfig();

	virtual void PostInitProperties() override;

	virtual FText GetSectionText() const override;

	void ForEachVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const;
	void ForEachLocalVariableMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const;
	void ForEachParameterMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const;
	void ForEachPropertyMetaDataKey(const UBlueprint* Blueprint, const FProperty* Property, const TFunction<void(const FMDMetaDataKey&)>& Func) const;
	void ForEachFunctionMetaDataKey(const UBlueprint* Blueprint, const TFunction<void(const FMDMetaDataKey&)>& Func) const;

	// If true, the metadata keys will automatically be sorted alphabetically
	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor")
	bool bSortMetaDataAlphabetically = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor")
	bool bEnableMetaDataEditorForVariables = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor")
	bool bEnableMetaDataEditorForLocalVariables = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor")
	bool bEnableMetaDataEditorForFunctions = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor")
	bool bEnableMetaDataEditorForCustomEvents = true;

	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor", DisplayName = "Enable Metadata Editor for Collapsed Graphs and Macros")
	bool bEnableMetaDataEditorForTunnels = false;

	// Param metadata only works for Function/Event parameters
	UPROPERTY(EditDefaultsOnly, Config, Category = "Metadata Editor")
	bool bEnableMetaDataEditorForFunctionParameters = true;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

private:
	UFUNCTION()
	TArray<FName> GetMetaDataKeyNames() const;

	UPROPERTY(EditAnywhere, Config, Category = "Metadata Keys", meta = (TitleProperty = "{Key} ({KeyType})"))
	TArray<FMDMetaDataKey> MetaDataKeys;
};
