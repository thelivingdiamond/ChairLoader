/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#ifndef TRANSLATE
# define TRANSLATE(x) x
#endif

#include "ww/SafeCast.h"
#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "yasli/STLImpl.h"

namespace ww{
	class Widget;
	class Container;

	void _ensureChildren(ww::Container* container, ww::Widget* widget);

	template<class T, class Object>
	class _Property{
	public:
		typedef void(Object::*Setter)(T);
		_Property(T& value, Object* object, Setter setter)
		: object_(object)
		, setter_(setter)
		, value_(value)
		{
		}

		T& value_;
		Object* object_;
		Setter setter_;
	};


	template<class T, class Object>
	class _PropertyConstRef{
	public:
		typedef void(Object::*Setter)(const T&);
		_PropertyConstRef(T& value, Object* object, Setter setter)
		: object_(object)
		, setter_(setter)
		, value_(value)
		{
		}

		T& value_;
		Object* object_;
		Setter setter_;
	};

	template<class T, class Object>
	_Property<T, Object> _property(T& value, Object* object, void(Object::*setter)(T)){
		return _Property<T, Object>(value, object, setter);
	}
	template<class T, class Object>
	_PropertyConstRef<T, Object> _property(T& value, Object* object, void(Object::*setter)(const T&)){
		return _Property<T, Object>(value, object, setter);
	}

	template<class T, class Object>
	inline bool serialize(yasli::Archive& ar, _Property<T, Object>& prop, const char* name, const char* label){
	  T value(prop.value_);
	  bool result = ar(value, name, label);
	  if(ar.isInput() && value != prop.value_)
		(prop.object_->*prop.setter_)(value);
	  return result;
	}


	template<class T, class Object>
	bool serialize(yasli::Archive& ar, _PropertyConstRef<T, Object>& prop, const char* name, const char* label){
	  T value(prop.value_);
	  bool result = ar(value, name, label);
	  if(ar.isInput() && value != prop.value_)
		(prop.object_->*prop.setter_)(value);
	  return result;
	}

}
