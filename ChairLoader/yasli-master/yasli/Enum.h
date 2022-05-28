/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include <map>

#include "yasli/Archive.h"
#include "yasli/StringList.h"
#include "yasli/TypeID.h"

namespace yasli{

class Archive;

struct LessStrCmp : std::binary_function<const char*, const char*, bool>
{
	bool operator()(const char* l, const char* r) const{
		return strcmp(l, r) < 0;
	}
};

class EnumDescription{
public:
	int value(const char* name) const;
	int valueByIndex(int index) const;
	int valueByLabel(const char* label) const;
	const char* name(int value) const;
	const char* nameByIndex(int index) const;
	const char* label(int value) const;
	const char* labelByIndex(int index) const;
	const char* indexByName(const char* name) const;
	int indexByValue(int value) const;

	template<class Enum>
	bool serialize(Archive& ar, Enum& value, const char* name, const char* label) const
	{
		int index = 0;
		if(ar.isOutput())
			index = indexByValue(value);

		StringListStaticValue stringListValue(ar.isEdit() ? labels() : names(), index);
		if(ar(stringListValue, name, label)){
			if(ar.isInput())
				value = Enum(ar.isEdit() ? valueByLabel(stringListValue.c_str()) : this->value(stringListValue.c_str()));
			return true;
		}
		return false;
	}

	bool serializeBitVector(Archive& ar, int& value, const char* name, const char* label) const;

	void add(int value, const char* name, const char* label = ""); // TODO
	int count() const{ return (int)values_.size(); }
	const StringListStatic& names() const{ return names_; }
	const StringListStatic& labels() const{ return labels_; }
	StringListStatic nameCombination(int bitVector) const;
	StringListStatic labelCombination(int bitVector) const;
	bool registered() const { return !names_.empty(); }
	TypeID type() const{ return type_; }
private:
	StringListStatic names_;
	StringListStatic labels_;

	typedef std::map<const char*, int, LessStrCmp> NameToValue;
	NameToValue nameToValue_;
	typedef std::map<const char*, int, LessStrCmp> LabelToValue;
	LabelToValue labelToValue_;
	typedef std::map<int, int> ValueToIndex;
	ValueToIndex valueToIndex_;
	typedef std::map<int, const char*> ValueToName;
	ValueToName valueToName_;
	typedef std::map<int, const char*> ValueToLabel;
	ValueToName valueToLabel_;
	std::vector<int> values_;
	TypeID type_;
};

template<class Enum>
class EnumDescriptionImpl : public EnumDescription{
public: static EnumDescription& the(){
		static EnumDescriptionImpl description;
		return description;
	}
};

template<class Enum>
EnumDescription& getEnumDescription(){
	return EnumDescriptionImpl<Enum>::the();
}

template<class Enum>
bool serializeEnum(const EnumDescription& desc, Archive& ar, Enum& value, const char* name, const char* label){
	return desc.serialize(ar, value, name, label);
}

}

#define YASLI_ENUM_BEGIN(Type, label)                                                \
    namespace {                                                                     \
        bool registerEnum_##Type();                                                 \
        bool Type##_enum_registered = registerEnum_##Type();                        \
        bool registerEnum_##Type(){                                                 \
		yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Type>::the();

#define YASLI_ENUM_BEGIN_NESTED(Class, Enum, label)                                  \
    namespace {                                                                     \
	bool registerEnum_##Class##_##Enum();                                           \
		bool Class##_##Enum##_enum_registered = registerEnum_##Class##_##Enum();    \
		bool registerEnum_##Class##_##Enum(){                                       \
			yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Class::Enum>::the();

#define YASLI_ENUM_BEGIN_NESTED2(Class, Class1, Enum, label)                                  \
	namespace {                                                                     \
	bool registerEnum_##Class##Class1##_##Enum();                                           \
	bool Class##Class1##_##Enum##_enum_registered = registerEnum_##Class##Class1##_##Enum();    \
	bool registerEnum_##Class##Class1##_##Enum(){                                       \
	yasli::EnumDescription& description = yasli::EnumDescriptionImpl<Class::Class1::Enum>::the();

                                                                                    
#define YASLI_ENUM_VALUE(value, label)                                              \
		description.add(int(value), #value, label);                                      

#define YASLI_ENUM(value, name, label)                                              \
		description.add(int(value), name, label);                                      

#define YASLI_ENUM_VALUE_NESTED(Class, value, label)                                       \
	description.add(int(Class::value), #value, label);                                      

#define YASLI_ENUM_VALUE_NESTED2(Class, Class1, value, label)                                       \
	description.add(int(Class::Class1::value), #value, label);                                      


#define YASLI_ENUM_END()													        \
            return true;                                                            \
        };                                                                          \
    };
// vim:ts=4 sw=4:
