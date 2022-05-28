/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

// Tags are 16-bit xor-hashes, checked for uniqueness in debug.
// Block is automatic: 8, 16 or 32-bits

#include "yasli/Archive.h"
#include "yasli/MemoryWriter.h" 
#include <vector>

namespace yasli{

#ifdef EMSCRIPTEN

template<class T>
class Unaligned
{
	char t_[sizeof(T)];
public:
	Unaligned(const T& t) { *this = t; }
	void operator=(const T& t) { memcpy(t_, &t, sizeof(T)); }
	operator T() const { T t; memcpy(&t, t_, sizeof(T)); return t; }
	void operator+=(const T& t) { *this = operator T() + t; }
};

#else

template<class T>
class Unaligned
{
	T t_;
public:
	Unaligned(const T& t) { *this = t; }
	void operator=(const T& t) { t_ = t; }
	operator T() const { return t_; }
	void operator+=(const T& t) { t_ += t; }
};

#endif

#if YASLI_BIN_ARCHIVE_LEGACY_HASH
inline unsigned short calcHash(const char* str)
{
	unsigned short hash = 0;
	const unsigned short* p = (const unsigned short*)(str);
	for(;;){
		unsigned short w = *p++;
		if(!(w & 0xff))
			break;
		hash ^= w;
		if(!(w & 0xff00))
			break;
	}
	return hash;
}
#else
inline unsigned short calcHash(const char* str)
{
	unsigned int hash = 1315423911;

	while(*str)
		hash ^= (hash << 5) + *str++ + (hash >> 2);

	return (unsigned short)hash;
}
#endif

class BinOArchive : public Archive{
public:

	BinOArchive();
	~BinOArchive() {}

	void clear();
	size_t length() const;
	const char* buffer() const { return stream_.buffer(); }
	bool save(const char* fileName);

	bool operator()(bool& value, const char* name, const char* label) override;
	bool operator()(StringInterface& value, const char* name, const char* label) override;
	bool operator()(WStringInterface& value, const char* name, const char* label) override;
	bool operator()(float& value, const char* name, const char* label) override;
	bool operator()(double& value, const char* name, const char* label) override;
	bool operator()(i8& value, const char* name, const char* label) override;
	bool operator()(i16& value, const char* name, const char* label) override;
	bool operator()(i32& value, const char* name, const char* label) override;
	bool operator()(i64& value, const char* name, const char* label) override;
	bool operator()(u8& value, const char* name, const char* label) override;
	bool operator()(u16& value, const char* name, const char* label) override;
	bool operator()(u32& value, const char* name, const char* label) override;
	bool operator()(u64& value, const char* name, const char* label) override;

	bool operator()(char& value, const char* name, const char* label) override;

	bool operator()(const Serializer &ser, const char* name, const char* label) override;
	bool operator()(ContainerInterface &ser, const char* name, const char* label) override;
	bool operator()(PointerInterface &ptr, const char* name, const char* label) override;

	using Archive::operator();

private:
	void openContainer(const char* name, int size, const char* typeName);
	void openNode(const char* name, bool size8 = true);
	void closeNode(const char* name, bool size8 = true);

	std::vector<unsigned int> blockSizeOffsets_;
	MemoryWriter stream_;

#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	enum BlockType { UNDEFINED, POD, NON_POD };
	std::vector<BlockType> blockTypes_;
#endif
};

//////////////////////////////////////////////////////////////////////////

class BinIArchive : public Archive{
public:

	BinIArchive();
	~BinIArchive();

	bool load(const char* fileName);
	bool open(const char* buffer, size_t length); // не копирует буффер!!!
	bool open(const BinOArchive& ar) { return open(ar.buffer(), ar.length()); }
	void close();

	bool operator()(bool& value, const char* name, const char* label) override;
	bool operator()(char& value, const char* name, const char* label) override;
	bool operator()(float& value, const char* name, const char* label) override;
	bool operator()(double& value, const char* name, const char* label) override;
	bool operator()(i8& value, const char* name, const char* label) override;
	bool operator()(u8& value, const char* name, const char* label) override;
	bool operator()(i16& value, const char* name, const char* label) override;
	bool operator()(u16& value, const char* name, const char* label) override;
	bool operator()(i32& value, const char* name, const char* label) override;
	bool operator()(u32& value, const char* name, const char* label) override;
	bool operator()(i64& value, const char* name, const char* label) override;
	bool operator()(u64& value, const char* name, const char* label) override;


	bool operator()(StringInterface& value, const char* name, const char* label) override;
	bool operator()(WStringInterface& value, const char* name, const char* label) override;
	bool operator()(const Serializer& ser, const char* name, const char* label) override;
	bool operator()(ContainerInterface& ser, const char* name, const char* label) override;
	bool operator()(PointerInterface& ptr, const char* name, const char* label) override;

	using Archive::operator();

private:
	class Block
	{
	public:
		Block(const char* data, int size) : 
		  begin_(data), end_(data + size), curr_(data), complex_(false), disableCheck_(false), isPointer_(false) {}

		  bool get(const char* name, Block& block);

		  void read(void *data, int size)
		  {
			  if(curr_ + size <= end_){
				memcpy(data, curr_, size);
				curr_ += size;	
			  }
			  else
				  YASLI_ASSERT(0);
		  }

		  template<class T>
		  void read(T& x){ read(&x, sizeof(x)); }

		  void read(std::string& s)
		  {
			  if(curr_ + strlen(curr_) < end_){
				s = curr_;
				curr_ += strlen(curr_) + 1;
			  }
			  else{
				  YASLI_ASSERT(0);
				  curr_ = end_;
			  }
		  }
		  void read(std::wstring& s)
		  {
			  // make sure that accessed wchar_t is always aligned
			  const char* strEnd = curr_;
			  const wchar_t nullChar = L'\0';
			  while (strEnd < end_) {
				  if (memcmp(strEnd, &nullChar, sizeof(nullChar)) == 0)
					  break;
				  strEnd += sizeof(wchar_t);
			  }
			  int len = int((strEnd - curr_) / sizeof(wchar_t));

			  s.resize(len);
			  if (len)
				  memcpy(&s[0], curr_, len * sizeof(wchar_t));

			  curr_ = curr_ + (len + 1) * sizeof(wchar_t);
			  if (curr_ > end_){
				  YASLI_ASSERT(0);
				  curr_ = end_;
			  }
		  }

		  unsigned int readPackedSize();

		  bool validToClose() const { return complex_ || curr_ == end_; } // Простые блоки должны быть вычитаны точно
		  void setDisableCheck() { disableCheck_ = true; }
		  void setIsPointer() { isPointer_ = true; }
	
	private:
		const char* begin_;
		const char* end_;
		const char* curr_;
		bool complex_;
		bool disableCheck_;
		bool isPointer_;

		void rewind();

#ifdef YASLI_BIN_ARCHIVE_CHECK_HASH_COLLISION
		typedef std::map<unsigned short, string> HashMap;
		HashMap hashMap_;
#endif
	};

	typedef std::vector<Block> Blocks;
	Blocks blocks_;
	const char* loadedData_;
	string stringBuffer_;
	wstring wstringBuffer_;

	bool openNode(const char* name);
	void closeNode(const char* name, bool check = true);
	Block& currentBlock() { return blocks_.back(); }
	template<class T>
	void read(T& t) { currentBlock().read(t); }
};

}
