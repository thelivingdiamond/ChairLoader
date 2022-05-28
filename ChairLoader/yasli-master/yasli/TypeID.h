/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <typeinfo>
#include "yasli/Config.h"
#include "yasli/Assert.h"
#include <string.h>

#if !YASLI_NO_RTTI
#ifndef _MSC_VER
using std::type_info;
#endif
#endif

namespace yasli{

class Archive;
struct TypeInfo;

class TypeID{
	friend class TypesFactory;
public:
	TypeID()
	: 
#if YASLI_NO_RTTI
	runtimeID_(0),
#endif
    typeInfo_(0)
	{}

	TypeID(const TypeID& original)
	:
	typeInfo_(original.typeInfo_),
#if !YASLI_NO_RTTI
	name_(original.name_)
#else
	runtimeID_(original.runtimeID_)
#endif
	{
	}

#if !YASLI_NO_RTTI
	explicit TypeID(const type_info& typeInfo)
	: typeInfo_(&typeInfo)
	{
	}
#endif

	operator bool() const{
		return *this != TypeID();
	}

	template<class T>
	static TypeID get();
	std::size_t sizeOf() const;
	const char* name() const;
	bool registered() const;

	bool operator==(const TypeID& rhs) const{
		if(typeInfo_ && rhs.typeInfo_)
			return typeInfo_ == rhs.typeInfo_;
		else
		{
			const char* name1 = name();
			const char* name2 = rhs.name();
			return strcmp(name1, name2) == 0;
		}
	}
	bool operator!=(const TypeID& rhs) const{
#if YASLI_NO_RTTI
		return runtimeID_ != rhs.runtimeID_;
#else
		return typeInfo_ != rhs.typeInfo_;
#endif
	}
	bool operator<(const TypeID& rhs) const{
#if YASLI_NO_RTTI
		return runtimeID_ < rhs.runtimeID_;
#else
		if(typeInfo_ && rhs.typeInfo_)
			return typeInfo_->before(*rhs.typeInfo_);
		else if(!typeInfo_)
			return rhs.typeInfo_!= 0;
		else if(!rhs.typeInfo_)
			return false;
		return false;
#endif
	}

#if YASLI_NO_RTTI
	const TypeInfo* typeInfo() const{ return typeInfo_; }
#else
	const type_info* typeInfo() const{ return typeInfo_; }
#endif
private:
#if YASLI_NO_RTTI 
	unsigned int runtimeID_;
	TypeInfo* typeInfo_;
    friend class bTypeInfo;
#else
	const type_info* typeInfo_;
	string name_;
#endif
	friend class TypeDescription;
    friend struct TypeInfo;
};

#if YASLI_NO_RTTI
struct TypeInfo
{
	TypeID id;
	size_t size;
	char name[128];

	// We are trying to minimize type names here. Stripping namespaces,
	// whitespaces and S/C/E/I prefixes. Why namespaces? Type names are usually
	// used in two contexts: for unique name within factory context, where
	// collision is unlikely, or for filtering in PropertyTree where concise
	// name is much more useful.
	static void cleanTypeName(char*& d, const char* dend, const char*& s, const char* send)
	{
		if(strncmp(s, "class ", 6) == 0)
			s += 6;
		else if(strncmp(s, "struct ", 7) == 0)
			s += 7;

		while(*s == ' ' && s != send)
			++s;

		// strip C/S/I/E prefixes
		if ((*s == 'C' || *s == 'S' || *s == 'I' || *s == 'E') && s[1] >= 'A' && s[1] <= 'Z')
			++s;

		if (s >= send)
			return;

		char* startd = d;
		while (d != dend && s != send) {
			while(*s == ' ' && s != send)
				++s;
			if (s == send)
				break;
			if (*s == ':' && s[1] == ':') {
				// strip namespaces
				s += 2;
				d = startd;

				if ((*s == 'C' || *s == 'S' || *s == 'I' || *s == 'E') && s[1] >= 'A' && s[1] <= 'Z')
					++s;
			}
			if (s >= send)
				break;
			if (*s == '<') {
				*d = '<';
				++d;
				++s;
				cleanTypeName(d, dend, s, send);
			}
			else if (*s == '>') {
				*d = '\0';
				return;
			}
			*d = *s;
			++s;
			++d;
		}
	}

	template<size_t nameLen>
	static void extractTypeName(char (&name)[nameLen], const char* funcName)
	{
#ifdef __clang__
        const char* s;
        const char* send;
        const char* lastSquareBracket = strrchr(funcName, ']');
        const char* lastRoundBracket = strrchr(funcName, ')');
        if (lastSquareBracket && lastRoundBracket && lastSquareBracket > lastRoundBracket)
        {
            // static yasli::TypeID yasli::TypeID::get() [T = ActualTypeName]
            s = strstr(funcName, "[T = ");
            if (s)
                s += 5;
            send = strrchr(funcName, ']');
        }
        else
        {
            // old versions of Xcode
            // static yasli::funcNameHelper(ActualTypeName *)
            s = strstr(funcName, "(");
            if (s)
                s += 1;
            send = strstr(funcName, "*)");
            if (send > s && *(send-1) == ' ')
                --send;
        }
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)) // at least 4.4.0
		// static yasli::TypeID yasli::TypeID::get() [with T = ActualTypeName]
		const char* s = strstr(funcName, "[with T = ");
		if (s)
			s += 10;
		const char* send = strrchr(funcName, ']');
#else
		// static yasli::TypeID yasli::TypeID::get<ActualTypeName>()
		const char* s = strchr(funcName, '<');
		const char* send = strrchr(funcName, '>');
		YASLI_ASSERT(s != 0  && send != 0);
		if (s != send)
			++s;
#endif
		YASLI_ASSERT(s != 0  && send != 0);

		char* d = name;
		const char* dend = name + sizeof(name) - 1;
		cleanTypeName(d, dend, s, send);
		*d = '\0';
		
		// This assertion is not critical, but may result in collision as
		// stripped name wil be used, e.g. for lookup in factory.
		YASLI_ASSERT(s == send && "Type name does not fit into the buffer");
	}

	static unsigned int generateTypeID()
	{
		// on gcc 4.6.3: if we start this counter from the zero
		// then the first two calls of this function returns same value
		static unsigned int counter = 1;
		return counter++;
	}

	TypeInfo(size_t size, const char* templatedFunctionName)
	: size(size)
	{
		id.runtimeID_ = generateTypeID();
		extractTypeName(name, templatedFunctionName);
		id.typeInfo_ = this;
	}
};
#endif

#ifdef __APPLE__
template<class T> const char* funcNameHelper(T* ref) { return __PRETTY_FUNCTION__; }
#endif
    
template<class T>
TypeID TypeID::get()
{
#if YASLI_NO_RTTI
# ifdef _MSC_VER
	static TypeInfo typeInfo(sizeof(T), __FUNCSIG__);
# else
#if defined(__APPLE__)
    static TypeInfo typeInfo(sizeof(T), funcNameHelper((T*)0));
#else
	static TypeInfo typeInfo(sizeof(T), __PRETTY_FUNCTION__);
#endif
# endif
	return typeInfo.id;
#else
	static TypeID result(typeid(T));
	return result;
#endif
}

template<class T>
T* createDerivedClass(TypeID typeID);

}

