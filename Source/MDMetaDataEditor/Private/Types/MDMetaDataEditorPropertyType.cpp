// Copyright Dylan Dumesnil. All Rights Reserved.


#include "MDMetaDataEditorPropertyType.h"

#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"

FMDMetaDataEditorPropertyType::FMDMetaDataEditorPropertyType(
	FName PropertyType,
	FName PropertySubType,
	TSoftObjectPtr<UObject> PropertySubTypeObject,
	FSimpleMemberReference PropertySubTypeMemberReference,
	FInstancedStruct ValueType,
	EMDMetaDataPropertyContainerType ContainerType
)
	: PropertyType(PropertyType)
	, PropertySubType(PropertySubType)
	, PropertySubTypeObject(PropertySubTypeObject)
	, PropertySubTypeMemberReference(PropertySubTypeMemberReference)
	, ValueType(ValueType)
	, ContainerType(ContainerType)

{
	// Fix any newly constructed property types.
	FixUp();
}

void FMDMetaDataEditorPropertyType::FixUp()
{
	// Float/Double types must be set to Real. The sub type is Float/Double.
	// If Float/Double is the primary type, then move it to subtype, and set the primary type to Real.
	if ((PropertyType == UEdGraphSchema_K2::PC_Float || PropertyType == UEdGraphSchema_K2::PC_Double) && PropertySubType == NAME_None)
	{
		PropertySubType = PropertyType;
		PropertyType = UEdGraphSchema_K2::PC_Real;
	}

	// Real type needs a subtype, default to Double.
	if (PropertyType == UEdGraphSchema_K2::PC_Real && PropertySubType == NAME_None)
	{
		PropertySubType = UEdGraphSchema_K2::PC_Double;
	}
}

FEdGraphPinType FMDMetaDataEditorPropertyType::ToGraphPinType() const
{
	FEdGraphPinType PinType;
	PinType.PinCategory = PropertyType;
	PinType.PinSubCategory = PropertySubType;
	PinType.PinSubCategoryObject = PropertySubTypeObject.LoadSynchronous();
	PinType.PinSubCategoryMemberReference = PropertySubTypeMemberReference;

	if (ValueType.IsValid())
	{
		PinType.PinValueType = ValueType.Get<FMDMetaDataEditorPropertyType>().ToGraphTerminalType();
	}

	switch (ContainerType)
	{
	case EMDMetaDataPropertyContainerType::None:
		PinType.ContainerType = EPinContainerType::None;
		break;
	case EMDMetaDataPropertyContainerType::Array:
		PinType.ContainerType = EPinContainerType::Array;
		break;
	case EMDMetaDataPropertyContainerType::Set:
		PinType.ContainerType = EPinContainerType::Set;
		break;
	case EMDMetaDataPropertyContainerType::Map:
		PinType.ContainerType = EPinContainerType::Map;
		break;
	}

	return PinType;
}

FEdGraphTerminalType FMDMetaDataEditorPropertyType::ToGraphTerminalType() const
{
	FEdGraphTerminalType TerminalType;
	TerminalType.TerminalCategory = PropertyType;
	TerminalType.TerminalSubCategory = PropertySubType;
	TerminalType.TerminalSubCategoryObject = PropertySubTypeObject.LoadSynchronous();

	return TerminalType;
}

void FMDMetaDataEditorPropertyType::SetFromGraphPinType(const FEdGraphPinType& GraphPinType)
{
	PropertyType = GraphPinType.PinCategory;
	PropertySubType = GraphPinType.PinSubCategory;
	PropertySubTypeObject = GraphPinType.PinSubCategoryObject.Get();
	PropertySubTypeMemberReference = GraphPinType.PinSubCategoryMemberReference;

	if (!GraphPinType.PinValueType.TerminalCategory.IsNone())
	{
		ValueType = ValueType.Make<FMDMetaDataEditorPropertyType>();
		ValueType.GetMutable<FMDMetaDataEditorPropertyType>().SetFromGraphTerminalType(GraphPinType.PinValueType);
	}
	else if (GraphPinType.ContainerType == EPinContainerType::Map)
	{
		ValueType = ValueType.Make<FMDMetaDataEditorPropertyType>();
	}
	else
	{
		ValueType.Reset();
	}

	switch (GraphPinType.ContainerType)
	{
	case EPinContainerType::None:
		ContainerType = EMDMetaDataPropertyContainerType::None;
		break;
	case EPinContainerType::Array:
		ContainerType = EMDMetaDataPropertyContainerType::Array;
		break;
	case EPinContainerType::Set:
		ContainerType = EMDMetaDataPropertyContainerType::Set;
		break;
	case EPinContainerType::Map:
		ContainerType = EMDMetaDataPropertyContainerType::Map;
		break;
	}
}

void FMDMetaDataEditorPropertyType::SetFromGraphTerminalType(const FEdGraphTerminalType& GraphTerminalType)
{
	PropertyType = GraphTerminalType.TerminalCategory;
	PropertySubType = GraphTerminalType.TerminalSubCategory;
	PropertySubTypeObject = GraphTerminalType.TerminalSubCategoryObject.Get();
}

bool FMDMetaDataEditorPropertyType::DoesMatchProperty(const FProperty* Property) const
{
	if (Property == nullptr)
	{
		return false;
	}

	const FProperty* EffectiveProp = Property;

	if (ContainerType == EMDMetaDataPropertyContainerType::Array || Property->IsA<FArrayProperty>())
	{
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
		if (ArrayProperty == nullptr)
		{
			return false;
		}

		EffectiveProp = ArrayProperty->Inner;
	}
	else if (ContainerType == EMDMetaDataPropertyContainerType::Set || Property->IsA<FSetProperty>())
	{
		const FSetProperty* SetProperty = CastField<FSetProperty>(Property);
		if (SetProperty == nullptr)
		{
			return false;
		}

		EffectiveProp = SetProperty->ElementProp;
	}
	else if (ContainerType == EMDMetaDataPropertyContainerType::Map || Property->IsA<FMapProperty>())
	{
		const FMapProperty* MapProperty = CastField<FMapProperty>(Property);
		if (MapProperty == nullptr)
		{
			return false;
		}

		// Only validate ValueType if we're explicitly checking for a map
		if (ContainerType == EMDMetaDataPropertyContainerType::Map)
		{
			const FMDMetaDataEditorPropertyType* ValueTypePtr = ValueType.GetPtr<FMDMetaDataEditorPropertyType>();
			if (ValueTypePtr == nullptr || !ValueTypePtr->DoesMatchProperty(MapProperty->ValueProp))
			{
				return false;
			}
		}

		EffectiveProp = MapProperty->KeyProp;
	}

	if (PropertyType == UEdGraphSchema_K2::PC_Wildcard)
	{
		return true;
	}

	if (PropertyType == UEdGraphSchema_K2::PC_Struct)
	{
		const FStructProperty* StructProperty = CastField<FStructProperty>(EffectiveProp);
		if (StructProperty == nullptr || StructProperty->Struct == nullptr)
		{
			return false;
		}

		if (PropertySubTypeObject.IsNull() || StructProperty->Struct->IsChildOf(Cast<UStruct>(PropertySubTypeObject.Get())))
		{
			return true;
		}
	}

	if (PropertyType == UEdGraphSchema_K2::PC_Object || PropertyType == UEdGraphSchema_K2::PC_SoftObject)
	{
		const FObjectPropertyBase* ObjectProperty = (PropertyType == UEdGraphSchema_K2::PC_Object)
			? CastField<FObjectPropertyBase>(EffectiveProp)
			: CastField<FSoftObjectProperty>(EffectiveProp);

		if (ObjectProperty == nullptr || ObjectProperty->PropertyClass == nullptr)
		{
			return false;
		}

		if (PropertySubTypeObject.IsNull() || ObjectProperty->PropertyClass->IsChildOf(Cast<UClass>(PropertySubTypeObject.Get())))
		{
			return true;
		}
	}

	if (PropertyType == UEdGraphSchema_K2::PC_Class || PropertyType == UEdGraphSchema_K2::PC_SoftClass)
	{
		if (PropertyType == UEdGraphSchema_K2::PC_Class && !EffectiveProp->IsA<FClassProperty>())
		{
			return false;
		}

		if (PropertyType == UEdGraphSchema_K2::PC_SoftClass && !EffectiveProp->IsA<FSoftClassProperty>())
		{
			return false;
		}

		const UClass* MetaClass = (PropertyType == UEdGraphSchema_K2::PC_Class)
			? CastField<FClassProperty>(EffectiveProp)->MetaClass
			: CastField<FSoftClassProperty>(EffectiveProp)->MetaClass;
		if (MetaClass == nullptr)
		{
			return false;
		}

		if (PropertySubTypeObject.IsNull() || MetaClass->IsChildOf(Cast<UClass>(PropertySubTypeObject.Get())))
		{
			return true;
		}
	}

	if (PropertyType == UEdGraphSchema_K2::PC_Enum || (PropertyType == UEdGraphSchema_K2::PC_Byte && PropertySubTypeObject.IsValid()))
	{
		const UEnum* PropertyEnum = [Property]() -> const UEnum*
		{
			if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
			{
				return EnumProperty->GetEnum();
			}
			else if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
			{
				return ByteProperty->Enum;
			}

			return nullptr;
		}();

		if (!IsValid(PropertyEnum))
		{
			return false;
		}

		return PropertySubTypeObject == UEnum::StaticClass() || PropertySubTypeObject == PropertyEnum;
	}

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	FEdGraphPinType PinType;
	if (!K2Schema->ConvertPropertyToPinType(EffectiveProp, PinType))
	{
		return false;
	}

	return PropertyType == PinType.PinCategory
		&& PropertySubType == PinType.PinSubCategory
		&& PropertySubTypeObject.Get() == PinType.PinSubCategoryObject
		&& PropertySubTypeMemberReference == PinType.PinSubCategoryMemberReference;
}

bool FMDMetaDataEditorPropertyType::operator==(const FMDMetaDataEditorPropertyType& Other) const
{
	return PropertyType == Other.PropertyType
		&& PropertySubType == Other.PropertySubType
		&& PropertySubTypeObject == Other.PropertySubTypeObject
		&& PropertySubTypeMemberReference == Other.PropertySubTypeMemberReference
		&& ValueType == Other.ValueType
		&& ContainerType == Other.ContainerType;
}
