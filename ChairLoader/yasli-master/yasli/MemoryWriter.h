/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <cstddef>
#include "Pointers.h"

#ifdef realloc
#undef realloc
#endif

namespace yasli{

	class MemoryWriter : public RefCounter {
public:
	MemoryWriter(std::size_t size = 128, bool reallocate = true);
	~MemoryWriter();

	const char* c_str() { return memory_; };
	const wchar_t* w_str() { return (wchar_t*)memory_; };
	char* buffer() { return memory_; }
	const char* buffer() const { return memory_; }
	std::size_t size() const{ return size_; }
	void clear() { position_ = memory_; }

	// String interface (after this calls '\0' is always written)
	MemoryWriter& operator<<(i8 value);
	MemoryWriter& operator<<(u8 value);
	MemoryWriter& operator<<(i32 value);
	MemoryWriter& operator<<(u32 value);
	MemoryWriter& operator<<(i64 value);
	MemoryWriter& operator<<(u64 value);
	MemoryWriter& operator<<(float value) { return (*this) << double(value); }
	MemoryWriter& operator<<(double value);
	MemoryWriter& operator<<(char value);
	MemoryWriter& operator<<(const char* value);
	MemoryWriter& operator<<(const wchar_t* value);
	void appendAsString(double);

	// Binary interface (does not writes trailing '\0')
	template<class T>
	void write(const T& value){
		write(&value, sizeof(value));
	}
	void write(char c);
	void write(const char* str);
	bool write(const void* data, std::size_t size);

	std::size_t position() const{ return position_ - memory_; }
	void setPosition(std::size_t pos);

	MemoryWriter& setDigits(int digits) { digits_ = (unsigned char)digits; return *this; }

private:
	void alloc(std::size_t initialSize);
	void realloc(std::size_t newSize);

	std::size_t size_;
	char* position_;
	char* memory_;
	bool reallocate_;
	unsigned char digits_;
};

}
