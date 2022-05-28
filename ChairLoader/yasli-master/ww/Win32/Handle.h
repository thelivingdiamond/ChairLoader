/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Assert.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Win32{

template<class HANDLE_TYPE = HANDLE>
class Handle{
public:
	Handle(HANDLE_TYPE handle = 0)
	: handle_(handle)
	{
	}

	operator HANDLE_TYPE() const{ return handle_; }

	HANDLE_TYPE detach(){
		HANDLE_TYPE result = handle_;
		handle_ = 0;
		return result;
	}

	void set(HANDLE_TYPE handle){
		if(handle_)
			::DeleteObject(handle_);
        handle_ = handle;
	}

	Handle& operator=(HANDLE_TYPE rhs){
		set(rhs);
		return *this;
	}

	~Handle(){
		set(0);
	}
protected:
	HANDLE_TYPE handle_;
private:
	void operator=(const Handle& rhs){
	}
};

class AutoSelector{
public:
	AutoSelector(HDC dc, HGDIOBJ object)
	: object_(object)
	, dc_(dc)
	{
		oldObject_ = ::SelectObject(dc_, object_);
        //YASLI_ASSERT(oldObject_ != 0);
	}

	~AutoSelector(){
        if(oldObject_){
		    HGDIOBJ object = ::SelectObject(dc_, oldObject_);
		    YASLI_ASSERT(object == object_);
        }
	}
	operator HGDIOBJ() const{ return oldObject_; }
protected:
	bool deleteObject_;
	HDC dc_;
	HGDIOBJ object_;
	HGDIOBJ oldObject_;
};

class DeletingSelector{
public:
	DeletingSelector(HDC dc, HGDIOBJ object)
	: object_(object)
	, dc_(dc)
	{
		oldObject_ = ::SelectObject(dc_, object_);
	}

	~DeletingSelector(){
        if(oldObject_){
		    HGDIOBJ object = ::SelectObject(dc_, oldObject_);
		    YASLI_ASSERT(object == object_);
        }
		::DeleteObject(object_);
	}
	operator HGDIOBJ() const{ return oldObject_; }
protected:
	bool deleteObject_;
	HDC dc_;
	HGDIOBJ object_;
	HGDIOBJ oldObject_;
};

class StockSelector{
public:
	StockSelector(HDC dc, HGDIOBJ object)
	: object_(object)
	, dc_(dc)
	{
		oldObject_ = ::SelectObject(dc_, object_);
	}

	~StockSelector(){
		HGDIOBJ object = ::SelectObject(dc_, oldObject_);
		YASLI_ASSERT(object == object_);
	}
	operator HGDIOBJ() const{ return oldObject_; }
protected:
	HDC dc_;
	HGDIOBJ object_;
	HGDIOBJ oldObject_;
};

}

