// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "MDMetaDataEditorUserConfig.generated.h"

/**
 * Configure metadata editor behaviours.
 * Settings are specific to the local user.
 */
UCLASS(Config = EditorPerProjectUserSettings, MinimalAPI)
class UMDMetaDataEditorUserConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FText GetSectionText() const override;

	// If true, a raw key-value editor will be displayed on variables/functions that can have metadata
	// This viewing/editing of all metadata, even if not exposed by the Metadata Editor config.
	UPROPERTY(EditDefaultsOnly, Config, Category = "Editor Config")
	bool bEnableRawMetaDataEditor = false;
};
