// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphPin.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
#include "StructUtils/InstancedStruct.h"
#else
#include "InstancedStruct.h"
#endif

#include "MDMetaDataEditorPropertyType.generated.h"

struct FEdGraphPinType;
struct FEdGraphTerminalType;

UENUM()
enum class EMDMetaDataPropertyContainerType : uint8
{
	None,
	Array,
	Set,
	Map
};

USTRUCT()
struct FMDMetaDataEditorPropertyType
{
	GENERATED_BODY()

public:
	FMDMetaDataEditorPropertyType(
		FName PropertyType = NAME_None,
		FName PropertySubType = NAME_None,
		TSoftObjectPtr<UObject> PropertySubTypeObject = nullptr,
		FSimpleMemberReference PropertySubTypeMemberReference = FSimpleMemberReference(),
		FInstancedStruct ValueType = FInstancedStruct(),
		EMDMetaDataPropertyContainerType ContainerType = EMDMetaDataPropertyContainerType::None
	);

	void FixUp();

	FEdGraphPinType ToGraphPinType() const;
	FEdGraphTerminalType ToGraphTerminalType() const;

	void SetFromGraphPinType(const FEdGraphPinType& GraphPinType);
	void SetFromGraphTerminalType(const FEdGraphTerminalType& GraphTerminalType);

	bool DoesMatchProperty(const FProperty* Property) const;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FName PropertyType = NAME_None;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FName PropertySubType = NAME_None;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	TSoftObjectPtr<UObject> PropertySubTypeObject;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FSimpleMemberReference PropertySubTypeMemberReference;

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	FInstancedStruct ValueType;
	FMDMetaDataEditorPropertyType& SetValueType(FMDMetaDataEditorPropertyType&& InValueType) { ValueType.InitializeAs<FMDMetaDataEditorPropertyType>(InValueType); return *this; }

	UPROPERTY(EditAnywhere, Config, Category = "Meta Data Editor")
	EMDMetaDataPropertyContainerType ContainerType = EMDMetaDataPropertyContainerType::None;
	FMDMetaDataEditorPropertyType& SetContainerType(EMDMetaDataPropertyContainerType InContainerType) { ContainerType = InContainerType; return *this; }

	bool operator==(const FMDMetaDataEditorPropertyType& Other) const;
	bool operator!=(const FMDMetaDataEditorPropertyType& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FMDMetaDataEditorPropertyType& Key)
	{
		uint32 Hash = GetTypeHash(Key.PropertyType);
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubType));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeObject));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeMemberReference.MemberParent));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeMemberReference.MemberName));
		Hash = HashCombine(Hash, GetTypeHash(Key.PropertySubTypeMemberReference.MemberGuid));
		Hash = HashCombine(Hash, GetTypeHash(Key.ValueType.GetScriptStruct()));
		Hash = HashCombine(Hash, GetTypeHash(Key.ValueType.GetMemory()));
		Hash = HashCombine(Hash, GetTypeHash(Key.ContainerType));
		return Hash;
	}
};
