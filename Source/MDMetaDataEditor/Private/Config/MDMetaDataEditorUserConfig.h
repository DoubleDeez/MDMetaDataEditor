// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "MDMetaDataEditorUserConfig.generated.h"

/**
 * Configure meta data editor behaviours.
 * Settings are specific to the local user.
 */
UCLASS(Config = EditorPerProjectUserSettings, MinimalAPI)
class UMDMetaDataEditorUserConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FText GetSectionText() const override;

	// If true, a raw key-value editor will be displayed on variables/functions that can have meta data
	// This viewing/editing of all meta data, even if not exposed by the Meta Data Editor config.
	UPROPERTY(EditDefaultsOnly, Config, Category = "Editor Config")
	bool bEnableRawMetaDataEditor = false;
};
