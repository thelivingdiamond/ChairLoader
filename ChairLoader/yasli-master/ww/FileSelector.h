/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "ww/Strings.h"

namespace yasli{
	class Archive;
}

namespace ww{

class FileSelector;
bool serialize(yasli::Archive& ar, ww::FileSelector& selector, const char* name, const char* label);

// File selector is a serialization decorator.
// It allows you to select filename with file open/save dialog.
// Editor part is implement in class PropertyRowFileSelector.
class FileSelector{
public:
	struct Options{
		Options(const char* _filter, bool _save = false, const char* _rootDirectory = "")
		: filter(_filter)
		, save(_save)
        , rootDirectory(_rootDirectory)
		{
		}
		string filter;
		string rootDirectory;
		bool save;
	};

	FileSelector()
	: fileNamePtr_(0)
	, options_(0)
	{
	}

	FileSelector& operator=(const FileSelector& _original)
	{
		if (_original.fileNamePtr_)
			options_ = _original.options_;
		fileName_ = _original.fileName_;
		return *this;
	}

	FileSelector(string& fileName, const Options& options)
	: fileNamePtr_(&fileName)
	, fileName_(fileName)
	, options_(&options)
	{
	}

	~FileSelector(){
		if(fileNamePtr_)
			*fileNamePtr_ = fileName_;
	}

	FileSelector& operator=(const char* rhs){
		fileName_ = rhs;
		return *this;
	}
	const char* c_str() const{ return fileName_.c_str(); }
	const Options* options(){ return options_; }

	void serialize(yasli::Archive& ar);
protected:
	string* fileNamePtr_;
	string fileName_;
	const Options* options_;
	friend bool serialize(yasli::Archive& ar, ww::FileSelector& selector, const char* name, const char* label);
};

}

