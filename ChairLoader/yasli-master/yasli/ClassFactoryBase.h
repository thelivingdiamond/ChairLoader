/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <map>

#include "yasli/Assert.h"
#include "yasli/TypeID.h"
#include "yasli/Config.h"

namespace yasli{

class Archive;
class TypeDescription;

class ClassFactoryBase{
public: 
	ClassFactoryBase(TypeID baseType)
	: baseType_(baseType)
	, nullLabel_(0)
	{
	}
	virtual ~ClassFactoryBase(){}
	virtual size_t size() const = 0;
	virtual const TypeDescription* descriptionByIndex(int index) const = 0;	
	virtual const TypeDescription* descriptionByType(TypeID type) const = 0;
	virtual TypeID findTypeByName(const char* name) const = 0;	
	virtual size_t sizeOf(TypeID typeID) const = 0;
	virtual void serializeNewByIndex(Archive& ar, int index, const char* name, const char* label) = 0;

	bool setNullLabel(const char* label){ nullLabel_ = label ? label : ""; return true; }
	const char* nullLabel() const{ return nullLabel_; }
protected:
	TypeID baseType_;
	const char* nullLabel_;
};


struct TypeIDWithFactory
{
	TypeID type;
	ClassFactoryBase* factory;

	TypeIDWithFactory(TypeID type = TypeID(), ClassFactoryBase* factory = 0)
	: type(type)
	, factory(factory)
	{
	}
};

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, TypeIDWithFactory& value, const char* name, const char* label);

}

// vim:ts=4 sw=4:
