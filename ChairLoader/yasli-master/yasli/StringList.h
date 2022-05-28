/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <string.h>
#include "yasli/Assert.h"
#include "yasli/Config.h"

namespace yasli{

class Archive;
class StringListStatic : public StringListStaticBase{
public:
    int find(const char* value) const{
		int numItems = int(size());
		for(int i = 0; i < numItems; ++i){
            if(strcmp((*this)[i], value) == 0)
                return i;
        }
		YASLI_ASSERT(0, "Value not found: %s", value);
        return 0;
    }
    static StringListStatic EMPTY;
};

class StringListStaticValue{
public:
	StringListStaticValue(const StringListStaticValue& original)
	: stringList_(original.stringList_)
	, index_(original.index_)
	{
	}
    explicit StringListStaticValue(const StringListStatic& stringList = StringListStatic::EMPTY, int value = 0)
    : stringList_(&stringList)
    , index_(value)
    {
    }
    StringListStaticValue(const StringListStatic& stringList, const char* value)
    : stringList_(&stringList)
    , index_(stringList.find(value))
    {
    }
    StringListStaticValue& operator=(const char* value){
        index_ = stringList_->find(value);
		return *this;
    }
    StringListStaticValue& operator=(int value){
		YASLI_ASSERT(value >= 0 && std::size_t(value) < stringList_->size());
        index_ = value;
		return *this;
    }
    StringListStaticValue& operator=(const StringListStaticValue& rhs){
        stringList_ = rhs.stringList_;
		index_ = rhs.index_;
        return *this;
    }
    const char* c_str() const{
        if(index_ >= 0 && std::size_t(index_) < stringList_->size())
			return (*stringList_)[index_];
		else
			return "";
    }
    int index() const{ return index_; }
    const StringListStatic& stringList() const{ return *stringList_; }
    template<class Archive>
    void YASLI_SERIALIZE_METHOD(Archive& ar) {
        ar(index_, "index");
    }
protected:
    const StringListStatic* stringList_;
    int index_;
};

class StringList: public StringListBase{
public:
    StringList() {}
    StringList(const StringList& rhs) : StringListBase(rhs){
    }
    StringList(const StringListStatic& rhs){
        const int size = int(rhs.size());
        resize(size);
        for(int i = 0; i < int(size); ++i)
            (*this)[i] = rhs[i];
    }
    enum { npos = -1 };
    int find(const char* value) const{
		const int numItems = int(size());
		for(int i = 0; i < numItems; ++i){
            if((*this)[i] == value)
                return i;
        }
        return npos;
    }
    static StringList EMPTY;
};

class StringListValue{
public:
    explicit StringListValue(const StringListStaticValue &value)
	{
        stringList_.insert( stringList_.end(), value.stringList().begin(), value.stringList().end() );
        index_ = value.index();
	}
    StringListValue(const StringListValue &value)
	{
		stringList_ = value.stringList_;
        index_ = value.index_;
	}
    explicit StringListValue(const StringList& stringList = StringList::EMPTY, int value = StringList::npos)
    : stringList_(stringList)
    , index_(value)
    {
    }
    StringListValue(const StringList& stringList, const char* value)
    : stringList_(stringList)
    , index_(stringList.find(value))
    {
        YASLI_ASSERT(index_ != StringList::npos);
    }
    StringListValue(const StringListStatic& stringList, const char* value)
    : stringList_(stringList)
    , index_(stringList.find(value))
    {
        YASLI_ASSERT(index_ != StringList::npos);
    }
    StringListValue& operator=(const char* value){
        index_ = stringList_.find(value);
		return *this;
    }
    StringListValue& operator=(int value){
		YASLI_ASSERT(value >= 0 && std::size_t(value) < stringList_.size());
        index_ = value;
		return *this;
    }
    const char* c_str() const{
        if(index_ >= 0 && std::size_t(index_) < stringList_.size())
			return stringList_[index_].c_str();
		else
			return "";
    }
    int index() const{ return index_; }
    const StringList& stringList() const{ return stringList_; }
    template<class Archive>
    void YASLI_SERIALIZE_METHOD(Archive& ar) {
        ar(index_, "index");
		ar(stringList_, "stringList");
    }
protected:
    StringList stringList_;
    int index_;
};

class Archive;

void splitStringList(StringList* result, const char *str, char sep);
void joinStringList(std::string* result, const StringList& stringList, char sep);
void joinStringList(std::string* result, const StringListStatic& stringList, char sep);

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringList& value, const char* name, const char* label);
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringListValue& value, const char* name, const char* label);
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringListStaticValue& value, const char* name, const char* label);

}
