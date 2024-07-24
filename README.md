# MDMetaDataEditor

MDMetaDataEditor enables editing the metadata of Blueprint-created variables, function parameters, functions, events, macros, and collapsed graphs.

Supports Unreal Engine 5.1 and later.

![](./Resources/readme_GameplayTagFilter.gif)

## Editing Metadata

The plugin adds a new `Metadata` section to the details panel of Blueprint Variables, Functions, Events, and Function Parameters. Here, any exposed metadata values can be set.

### Advanced

An advanced raw metadata editor can be enabled via Editor Preferences -> General -> Metadata Editor (Local Only). This will enabling the raw strings of key-value metadata pairs.

![](./Resources/readme_ForceInlineRow.gif)

![](./Resources/readme_EditCondition.gif)

## Configuring Metadata Keys
By default, the plugin comes with some common metadata keys pre-configured.
The list of available Metadata Keys that are exposed to Blueprint can be configured in **Project Settings -> Editor -> Metadata Editor**.

Here, Metadata Keys can be added, removed, and modified. By default, new keys are set up to work for any variable type in any blueprint.

The **Key Type** should match the expected type of the metadata key, that will also determine which widget is displayed for setting the value of the metadata.

For Metadata Keys meant only for functions, the **Supported Property Types** list should be empty and have **Can be Used by Functions** checked.

See these pages for documentation on various metadata options:
- [benui's all UPROPERTY specifiers](https://benui.ca/unreal/uproperty/)
- [benui's all UFUNCTION specifiers](https://benui.ca/unreal/ufunction/)
- [benui's all UPARAM specifiers](https://benui.ca/unreal/uparam/)
- [Unreal Engine 5.2 UProperty specifiers](https://docs.unrealengine.com/5.2/en-US/unreal-engine-uproperty-specifiers/#metadataspecifiers)
- [Unreal Engine 5.3 Function specifiers](https://docs.unrealengine.com/5.3/en-US/ufunctions-in-unreal-engine/#metadataspecifiers)

![](./Resources/readme_MetaDataKeys.png)

![](./Resources/readme_CustomMetaData.png)

![](./Resources/readme_CustomMetaData.gif)

## FYI

* If a metadata key is removed from the Project Settings, or the specific metadata type is disabled, that metadata key will continue to exist on variables, functions, events, and parameters.

* I couldn't find any useful Metadata Keys for collapsed graphs/macros so I disabled it by default. It can be re-enabled in the Project Settings.
