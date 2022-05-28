/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/decorators/Button.h"
#include "yasli/decorators/HorizontalLine.h"

namespace yasli{
class Archive;
}

namespace ww{ 

// OBSOLETE, should not be used in the new code
typedef yasli::Button ButtonDecorator;
typedef yasli::HorizontalLine HLineDecorator;
// ^^^

struct RadioDecorator{
	RadioDecorator()
	: value_(false)
	, valuePtr_(0)
	{
	}
	RadioDecorator(bool& value)
	: value_(value)
	, valuePtr_(&value)
	{}
	~RadioDecorator() { 
		if(valuePtr_)
			*valuePtr_ = value_;
	}

	RadioDecorator& operator=(const RadioDecorator& rhs){
		value_ = rhs.value_;
		return *this;
	}
	RadioDecorator& operator=(bool value){
		value_ = value;
		return *this;
	}
    void serialize(yasli::Archive& ar)
    {
        ar(value_, "value", "Value");
    }
	operator bool() const{ return value_; }
	bool value_;
	bool* valuePtr_;
};


struct NotDecorator{
	NotDecorator()
	: value_(false)
	, valuePtr_(0)
	{
	}
	NotDecorator(const NotDecorator& original)
	: value_(original.value_)
	, valuePtr_(0)
	{
	}
	NotDecorator(bool& value)
	: value_(value)
	, valuePtr_(&value)
	{}
	~NotDecorator() { 
		if(valuePtr_)
			*valuePtr_ = value_;
	}

	NotDecorator& operator=(const NotDecorator& rhs){
		value_ = rhs.value_;
		return *this;
	}
	NotDecorator& operator=(bool value){
		value_ = value;
		return *this;
	}
    void serialize(yasli::Archive& ar)
    {
        ar(value_, "value", "Value");
    }
	operator bool() const{ return value_; }
	bool value_;
	bool* valuePtr_;
};


// template<class T, class PointerType = SharedPtr<T> >
// struct OptionalPtr : PointerType{
// 	void serialize(yasli::Archive& ar){
// 		YASLI_ASSERT(ar.isEdit());
// 		bool optionallyEnabled = false;
// 		if(ar.isInput()){
// 			ar(optionallyEnabled, "optionallyEnabled", "^");
// 			if(optionallyEnabled){
// 				if(!get())
// 					set(new T());
// 				get()->serialize(ar);
// 			}
// 			else
// 				set(0);
// 		}
// 		else{
// 			optionallyEnabled = get() != 0;
// 			ar(optionallyEnabled, "optionallyEnabled", "^");
// 			if(optionallyEnabled)
// 				get()->serialize(ar);
// 		}
// 	}
// };

inline bool serialize(yasli::Archive& ar, ww::RadioDecorator& value, const char* name, const char* label){		
    if(ar.isEdit())
        return ar(yasli::Serializer(value), name, label);
    else
        return ar(value.value_, name, label);
}

inline bool serialize(yasli::Archive& ar, ww::NotDecorator& value, const char* name, const char* label){		
    if(ar.isEdit())
        return ar(yasli::Serializer(value), name, label);
    else
        return ar(value.value_, name, label);
}

}
