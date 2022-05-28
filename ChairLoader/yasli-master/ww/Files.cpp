/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "yasli/Assert.h"
#include "Files.h"

#ifdef WIN32
# include <fcntl.h>
# include <io.h>
# include <sys/stat.h>
# include <windows.h>
#else
# include <sys/stat.h>
# include <dirent.h>
# include <unistd.h>
#endif


#include <malloc.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include "Unicode.h"

namespace Files{

using ww::string;
using ww::wstring;

time_t getModifyTime(const char* path)
{
#ifdef WIN32
	int desc = _wopen(ww::toWideChar(path).c_str(), _O_RDONLY, 0);
	if(desc <= 0)
		return 0;
	struct _stat64 fileStat;
	_fstat64(desc, &fileStat);
	_close(desc);
	return fileStat.st_mtime;
#else
	// TODO: implement
	return 0;
#endif
}

bool exists(const char* fileName)
{
    if(std::FILE* file = ::ww::fopen(fileName, "rb")){
        fclose(file);
        return true;
    }
    else
        return false;
}

bool isDirectory(const char* fileName)
{
#ifdef WIN32
    DWORD attributes = GetFileAttributesW( ww::toWideChar(fileName).c_str() );
	if(attributes == INVALID_FILE_ATTRIBUTES)
		return false;
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	if(DIR* dir = opendir(fileName)){
		closedir(dir);
		return true;
	}
	return false;
#endif
}

string currentDirectory() // TODO make unicode!
{
#ifdef WIN32
    wchar_t buf[MAX_PATH];
    GetCurrentDirectoryW(ARRAY_LEN(buf), buf);
    return ww::fromWideChar(buf);
#else
    YASLI_ASSERT(0 && "Not implemented");
    return string();
#endif
}

bool remove(const char* fileName)
{
#ifdef WIN32
    return DeleteFileW( ww::toWideChar(fileName).c_str() ) != FALSE;
#else
	return unlink(fileName) == 0;
#endif
}

bool createDirectory(const char* path)
{
#ifndef WIN32
    mode_t mask = umask(0);
    umask(mask);
#endif

    int len = (int)strlen(path);
    char* pathCopy = (char*)alloca(len + 1);
    memcpy(pathCopy, path, len + 1);

    char* p = pathCopy;
    while(*p != '\0'){
        if(*p == PATH_SEPARATOR[0] || *p == PATH_SEPARATOR2[0]){
            *p = '\0';
#ifdef WIN32
			if(!CreateDirectoryW(ww::toWideChar(path).c_str(), 0))
				return false;
#else
            if(mkdir(path, mask) != 0)
                return false;
#endif
            *p = PATH_SEPARATOR[0];
        }
		++p;
    }

#ifdef WIN32
    return CreateDirectoryW(ww::toWideChar(pathCopy).c_str(), 0) != FALSE;
#else
    return mkdir(pathCopy, mask) == 0;
#endif
}

string extractFilePath(const char* path)
{
	size_t len = strlen(path);
	if(len == 0)
		return string();

	const char* p = path + len - 1;
	while(p != path){
		if(p[0] == PATH_SEPARATOR[0] || p[0] == PATH_SEPARATOR2[0])
			return string(path, p);
		--p;
	}
	return string();
}

string relativePath(const char* path, const char* toDirectory)
{
    YASLI_ESCAPE(path != 0 && toDirectory != 0, return string());
#ifdef WIN32
	// TODO: здесь нужно использовать Unicode. Возможность использовать "..".
    wchar_t fullPath[MAX_PATH + 1];
	wchar_t fullPathLower[MAX_PATH + 1];
    GetFullPathNameW( ww::toWideChar(path).c_str(), ARRAYSIZE(fullPath), fullPath, 0 );
	wcscpy(fullPathLower, fullPath);
	CharLowerBuffW( fullPathLower, (int)wcslen(fullPathLower) );
    wchar_t fullPathDir[MAX_PATH + 1];
    GetFullPathNameW( ww::toWideChar(toDirectory).c_str(), ARRAYSIZE(fullPathDir), fullPathDir, 0 );
	CharLowerBuffW( fullPathDir, (int)wcslen(fullPathDir) );
	if ( wcsstr( fullPathLower, fullPathDir ) == fullPathLower )
    {
        wchar_t* start = fullPath + wcslen(fullPathDir);
        if(*start == L'\\')
            ++start;
        return ww::fromWideChar(wstring(start, fullPath + wcslen(fullPath)).c_str());
    }
	else
		return string();
#else
    YASLI_ASSERT(0 && "Not implemented");
    return string();
#endif
}

string extractFileBase(const char* path)
{
    YASLI_ESCAPE(path != 0, return string());
    const char* fileNameStart = path;
    const char* end = path + strlen(path);
    const char* p = std::find(path, end, pathSeparator()[0]);
    if(p != end){
        fileNameStart = p + 1;
    }
    p = end;
    while(--p != fileNameStart){
        if(*p =='.') 
            break;
    }
    if(p == fileNameStart)
        p = end;
	return string(fileNameStart, p);
}


bool createDirectoryForFile(const char* path)
{
    YASLI_ESCAPE(path != 0, return false);
    size_t len = strlen(path);
    const char* p = path + len;
    while(--p != path){
        if(*p == pathSeparator()[0]){
            string directoryPath(path, p);
            return createDirectory(directoryPath.c_str());
        }
    }
    return true;
}

bool copy(const char* sourceFile, const char* destinationFile)
{
#ifdef WIN32
	return CopyFileA(sourceFile, destinationFile, FALSE) != FALSE;
#else
    return system((string("cp \"") + sourceFile + "\" \"" + destinationFile + "\"").c_str()) == 0;
#endif
}

string setExtention(const char* file_name, const char* extention)
{
	string str = file_name;
	size_t pos = str.rfind(".");
	if(pos != string::npos)
		str.erase(pos, str.size());
	if(!*extention)
		return str;
	str += ".";
	str += extention;
	return str;
}

string extractExtension(const char* fileName)
{
	string str = fileName;
	size_t pos = str.rfind(".");
	if(pos != string::npos)
        return string(str.begin() + pos + 1, str.end());
    return string();
}

string fixSlashes(const char* _path)
{
	string path(_path);
	for(unsigned int i = 0; i < path.size(); i++)
		if(path[i] == '/')
			path[i] = '\\';
	return path;
}

// ---------------------------------------------------------------------------

#ifdef WIN32
class iteratorImpl : public yasli::RefCounter{
public:
	iteratorImpl()
	{
		handle_ = INVALID_HANDLE_VALUE;
	}
	iteratorImpl(const char* path)
	{
		handle_ = FindFirstFileA(path, &findData_);
		if(handle_ == INVALID_HANDLE_VALUE)
			return;
		if(!strcmp(findData_.cFileName, ".") && !FindNextFileA(handle_, &findData_)){
			handle_ = INVALID_HANDLE_VALUE;
			return;
		}
		if(!strcmp(findData_.cFileName, "..") && !FindNextFileA(handle_, &findData_)){
			handle_ = INVALID_HANDLE_VALUE;
			return;
		}
		entry_.name_ = findData_.cFileName;
		entry_.isDirectory_ = (findData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	~iteratorImpl(){
		if(handle_ != INVALID_HANDLE_VALUE)
			FindClose(handle_);
	}

	bool compare(const iteratorImpl& rhs){
		return handle_ == rhs.handle_;
	}

	bool next()
	{
		YASLI_ASSERT(handle_ != INVALID_HANDLE_VALUE && "Incrementing bad Files::iterator");
		if(!FindNextFileA(handle_, &findData_)){
			FindClose(handle_);
			handle_ = INVALID_HANDLE_VALUE;
			return false;
		}
		else{
			entry_.name_ = findData_.cFileName;
			entry_.isDirectory_ = (findData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			return true;
		}
	}
	bool isValid(){
		return handle_ != INVALID_HANDLE_VALUE;
	}

	WIN32_FIND_DATAA findData_;
	HANDLE handle_;
	DirectoryEntry entry_;

	friend class iterator;
};
#else

class iteratorImpl : public RefCounter{
public:
	iteratorImpl()
	{
		dir_ = 0;
	}

	iteratorImpl(const char* path)
	{
		string p = extractFilePath(path);
		path = p.c_str();
		dir_ = opendir(path);
		if(!dir_)
			return;
		dirent* entry = readdir(dir_);
		YASLI_ASSERT(entry);
		YASLI_ASSERT(strcmp(entry->d_name, ".") == 0);
		entry = readdir(dir_);
		YASLI_ASSERT(entry);
		YASLI_ASSERT(strcmp(entry->d_name, "..") == 0);
		entry = readdir(dir_);
		if(entry){
			entry_.name_ = entry->d_name;
			entry_.isDirectory_ = isDirectory(entry_.name());
		}
		else{
			closedir(dir_);
			dir_ = 0;
		}
	}
	~iteratorImpl(){
		if(dir_)
			closedir(dir_);
	}

	bool compare(const iteratorImpl& rhs){
		if(dir_ == 0 && rhs.dir_ == 0)
			return true;
		if(!dir_ || !rhs.dir_)
			return false;
		return telldir(dir_) == telldir(rhs.dir_);
	}

	bool next()
	{
		YASLI_ASSERT(dir_ && "Incrementing bad Files::iterator");
		dirent* entry = readdir(dir_);
		if(!entry){
			closedir(dir_);
			dir_ = 0;
			return false;
		}
		else{
			entry_.name_ = entry->d_name;
			entry_.isDirectory_ = isDirectory(entry_.name());
			return true;
		}
	}
	bool isValid(){
		return dir_ != 0;
	}

	DIR* dir_;
	DirectoryEntry entry_;

	friend class iterator;
};

#endif

// ---------------------------------------------------------------------------

iterator::iterator(const char* path)
: impl_(0)
{
	if(path){
		set(new iteratorImpl(path));
		if(!impl_->isValid())
			set(0);
	}
}

iterator::~iterator()
{
	set(0);
}

void iterator::set(iteratorImpl* impl)
{
	if(impl_ != impl){
		if(impl_ && !impl_->release())
			delete impl_;

		impl_ = impl;
		if(impl_)
			impl_->acquire();
	}
}

DirectoryEntry& iterator::operator*()
{
	if(impl_)
		return impl_->entry_;
	else{
		YASLI_ASSERT(0 && "Accessing invalid Files::iterator");
		return *(DirectoryEntry*)(0);
	}
}

DirectoryEntry* iterator::operator->()
{
	return &operator*();
}

bool iterator::operator!=(const iterator& rhs)
{
	return !operator==(rhs);
}

bool iterator::operator==(const iterator& rhs)
{
	if(!impl_ && !rhs.impl_)
		return true;
	if(impl_ && !rhs.impl_ || !impl_ && rhs.impl_)
		return false;
	return impl_->compare(*rhs.impl_);
}

iterator& iterator::operator++()
{
	if(impl_){
		YASLI_ASSERT(impl_->refCount() == 1);

		if(!impl_->next())
			set(0);
	}
	else{
		YASLI_ASSERT(0 && "Incrementing invalid Files::iterator");
	}
	return *this;
}

iterator::operator bool() const
{
	return impl_ != 0;
}

}
