/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "BinArchive.h"
#include <map>
#include "yasli/MemoryWriter.h"
#include "yasli/ClassFactory.h"

using namespace std;

namespace yasli{

static const unsigned char SIZE16 = 254;
static const unsigned char SIZE32 = 255;

static const unsigned int BIN_MAGIC = 0xb1a4c17f;

BinOArchive::BinOArchive()
: Archive(OUTPUT | BINARY)
{
    clear();
}

void BinOArchive::clear()
{
    stream_.clear();
    stream_.write((const char*)&BIN_MAGIC, sizeof(BIN_MAGIC));

#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	blockTypes_.push_back(UNDEFINED);
#endif
}

size_t BinOArchive::length() const
{ 
    return stream_.position();
}

bool BinOArchive::save(const char* filename)
{
    FILE* f = fopen(filename, "wb");
    if(!f)
        return false;

    if(fwrite(buffer(), 1, length(), f) != length())
    {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

inline void BinOArchive::openNode(const char* name, bool size8)
{
#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	YASLI_ASSERT(blockTypes_.back() == UNDEFINED || blockTypes_.back() == (!strlen(name) ? POD : NON_POD), "Mixing empty and non-empty names is dangerous for BinArchives");
	blockTypes_.back() = (!strlen(name) ? POD : NON_POD);
	blockTypes_.push_back(UNDEFINED);
#endif

	if(!strlen(name))
		return;

	unsigned short hash = calcHash(name);
	stream_.write(hash);

	blockSizeOffsets_.push_back(int(stream_.position()));
	stream_.write((unsigned char)0); 
	if(!size8)
		stream_.write((unsigned short)0); 
}

inline void BinOArchive::closeNode(const char* name, bool size8)
{
#ifdef YASLI_BIN_ARCHIVE_CHECK_EMPTY_NAME_MIX
	blockTypes_.pop_back();
#endif

	if(!strlen(name))
		return;

	unsigned int offset = blockSizeOffsets_.back();
	unsigned int size = (unsigned int)(stream_.position() - offset - sizeof(unsigned char) - (size8 ? 0 : sizeof(unsigned short)));
	blockSizeOffsets_.pop_back();
	unsigned char* sizePtr = (unsigned char*)(stream_.buffer() + offset);

	if(size < SIZE16){
		*sizePtr = size;
		if(!size8){
			unsigned char* buffer = sizePtr + 3;
			memmove(buffer - 2, buffer, size);
			stream_.setPosition(stream_.position() - 2);
		}
	}
	else{
		YASLI_ASSERT(!size8);
		if(size < 0x10000){
			*sizePtr = SIZE16;
			*((Unaligned<unsigned short>*)(sizePtr + 1)) = size;
		}
		else{
			unsigned char* buffer = sizePtr + 3;
			stream_.write((unsigned short)0);
			*sizePtr = SIZE32;
			memmove(buffer + 2, buffer, size);
			*((Unaligned<unsigned int>*)(sizePtr + 1)) = size;
		}
	}
}

bool BinOArchive::operator()(bool& value, const char* name, const char* label)
{
	openNode(name);
	stream_.write(value);
	closeNode(name);
	return true;
}

bool BinOArchive::operator()(StringInterface& value, const char* name, const char* label)
{
	bool size8 = strlen(value.get()) + 1 < SIZE16;
    openNode(name, size8);
	stream_ << value.get();
	stream_.write(char(0));
    closeNode(name, size8);
    return true;
}

bool BinOArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	bool size8 = (wcslen(value.get()) + 1)*sizeof(wchar_t) < SIZE16;
	openNode(name, size8);
	stream_ << value.get();
	stream_.write(wchar_t(0));
	closeNode(name, size8);
	return true;
}

bool BinOArchive::operator()(float& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(double& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(i16& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(i8& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(u8& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(char& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(u16& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(i32& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(u32& value, const char* name, const char* label)
{
    openNode(name);
	stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(i64& value, const char* name, const char* label)
{
    openNode(name);
    stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(u64& value, const char* name, const char* label)
{
    openNode(name);
    stream_.write(value);
    closeNode(name);
    return true;
}

bool BinOArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
    openNode(name, false);
    ser(*this);
    closeNode(name, false);
    return true;
}

bool BinOArchive::operator()(ContainerInterface& ser, const char* name, const char* label)
{
	openNode(name, false);

	unsigned int size = (unsigned int)ser.size();
	if(size < SIZE16)
		stream_.write((unsigned char)size);
	else if(size < 0x10000){
		stream_.write(SIZE16);
		stream_.write((unsigned short)size);
	}
	else{
		stream_.write(SIZE32);
		stream_.write(size);
	}

	if(strlen(name)){
		if(size > 0){
			int i = 0;
			do {
				char buffer[16];
#ifdef _MSC_VER
				_itoa(i++, buffer, 10);
#else
				sprintf(buffer, "%d", i++);
#endif
				ser(*this, buffer, "");
			} while (ser.next());
		}

		closeNode(name, false);
	}
	else{
		if(size > 0)
			do 
				ser(*this, "", "");
				while (ser.next());
	}

    return true;
}

bool BinOArchive::operator()(PointerInterface& ptr, const char* name, const char* label)
{
	openNode(name, false);


	TypeID derived = ptr.type();
	const TypeDescription* desc = 0;
	const char* typeName = "";
	if (derived) {
		desc = ptr.factory()->descriptionByType(derived);
		typeName = desc->name();
		YASLI_ASSERT(desc != 0 && "Writing unregistered class. Use YASLI_CLASS macro for registration.");
	}

    TypeID baseType = ptr.baseType();

	if(ptr.get()){
		stream_ << typeName;
		stream_.write(char(0));
		ptr.serializer()(*this);
	}
	else
		stream_.write(char(0));

    closeNode(name, false);
    return true;
}


//////////////////////////////////////////////////////////////////////////

BinIArchive::BinIArchive()
: Archive(INPUT | BINARY)
, loadedData_(0)
{
}

BinIArchive::~BinIArchive()
{
	close();
}

bool BinIArchive::load(const char* filename)
{
	close();

	FILE* f = fopen(filename, "rb");
	if(!f)
		return false;
	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	fseek(f, 0, SEEK_SET);
	if(length == 0){
		fclose(f);
		return false;
	}
	loadedData_ = new char[length];
	if(fread((void*)loadedData_, 1, length, f) != length || !open(loadedData_, length)){
		close();
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool BinIArchive::open(const char* buffer, size_t size)
{
    if(!buffer)
        return false;
    if(size < sizeof(unsigned int))
        return false;
	if(memcmp(buffer, &BIN_MAGIC, sizeof(unsigned int)))
		return false;
	buffer += sizeof(unsigned int);
	size -= sizeof(unsigned int);

	blocks_.push_back(Block(buffer, (unsigned int)size));
	return true;
}

void BinIArchive::close()
{
	if(loadedData_)
		delete[] loadedData_;
	loadedData_ = 0;
}

bool BinIArchive::openNode(const char* name)
{
	Block block(0, 0);
	if(currentBlock().get(name, block)){
		blocks_.push_back(block);
		return true;
	}
	return false;
}

void BinIArchive::closeNode(const char* name, bool check) 
{
	YASLI_ASSERT(!check || currentBlock().validToClose());
	blocks_.pop_back();
}

bool BinIArchive::operator()(bool& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(StringInterface& value, const char* name, const char* label)
{
	if(!strlen(name)){
		stringBuffer_.clear();
		read(stringBuffer_);
		value.set(stringBuffer_.c_str());
		return true;
	}

	if(!openNode(name))
		return false;

	stringBuffer_.clear();
	read(stringBuffer_);
	value.set(stringBuffer_.c_str());
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(WStringInterface& value, const char* name, const char* label)
{
	if(!strlen(name)){
		wstringBuffer_.clear();
		read(wstringBuffer_);
		value.set(wstringBuffer_.c_str());
		return true;
	}

	if(!openNode(name))
		return false;

	wstringBuffer_.clear();
	read(wstringBuffer_);
	value.set(wstringBuffer_.c_str());
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(float& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(double& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(i16& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u16& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}


bool BinIArchive::operator()(i32& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u32& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(i64& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u64& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(i8& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(u8& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(char& value, const char* name, const char* label)
{
	if(!strlen(name)){
		read(value);
		return true;
	}

	if(!openNode(name))
		return false;

	read(value);
	closeNode(name);
	return true;
}

bool BinIArchive::operator()(const Serializer& ser, const char* name, const char* label)
{
	if(!strlen(name)){
		ser(*this);
		return true;
	}

	if(!openNode(name))
		return false;

	ser(*this);
	closeNode(name, false);
	return true;
}

bool BinIArchive::operator()(ContainerInterface& ser, const char* name, const char* label)
{
	if(strlen(name)){
		if(!openNode(name))
			return false;

		currentBlock().setDisableCheck();

		size_t size = currentBlock().readPackedSize();
		ser.resize(size);

		if(size > 0){
			int i = 0;
			do{
				char buffer[16];
#ifdef _MSC_VER
				_itoa(i++, buffer, 10);
#else
				sprintf(buffer, "%d", i++);
#endif
				ser(*this, buffer, "");
			}
			while(ser.next());
		}
		closeNode(name);
		return true;
	}
	else{
		size_t size = currentBlock().readPackedSize();
		ser.resize(size);
		if(size > 0){
			do
				ser(*this, "", "");
				while(ser.next());
		}
		return true;
	}
}

bool BinIArchive::operator()(PointerInterface& ptr, const char* name, const char* label)
{
	if(strlen(name) && !openNode(name))
		return false;

	currentBlock().setIsPointer();
	string& typeName = stringBuffer_;
	typeName.clear();
	read(typeName);
	TypeID type;
	if(!typeName.empty())
		type = ptr.factory()->findTypeByName(typeName.c_str());
	if(ptr.type() && (!type || (type != ptr.type())))
		ptr.create(TypeID()); // 0

	if(type && !ptr.get())
		ptr.create(type);

	if(Serializer ser = ptr.serializer())
		ser(*this);

	if(strlen(name))
		closeNode(name);
	return true;
}

unsigned int BinIArchive::Block::readPackedSize()
{
	unsigned char size8;
	read(size8);
	if(size8 < SIZE16)
		return size8;
	if(size8 == SIZE16){
		unsigned short size16;
		read(size16);
		return size16;
	}
	unsigned int size32;
	read(size32);
	return size32;
}

bool BinIArchive::Block::get(const char* name, Block& block) 
{
	if(begin_ == end_)
		return false;
	complex_ = true;
	unsigned short hashName = calcHash(name);

#ifdef YASLI_BIN_ARCHIVE_CHECK_HASH_COLLISION
	if(!disableCheck_){
		HashMap::iterator i = hashMap_.find(hashName);
		if(i != hashMap_.end() && i->second != name)
			YASLI_ASSERT(0, "BinArchive hash colliding: %s - %s", i->second.c_str(), name);
		hashMap_[hashName] = name;
	}
#endif

	const char* currInitial = curr_;
	bool restarted = false;
	if(curr_ >= end_){
		rewind();
		restarted = true;
	}
	for(;;){
		unsigned short hash;
		read(hash);
		unsigned int size = readPackedSize();
		
		const char* currPrev = curr_;
		if((curr_ += size) >= end_){
			if(restarted)
				return false;
			rewind();
			restarted = true;
		}

		if(hash == hashName){
			block = Block(currPrev, size);
			return true;
		}

		if(curr_ == currInitial)
			return false;
	}
}

void BinIArchive::Block::rewind()
{
	curr_ = begin_;
	if(isPointer_){
		string typeName;
		read(typeName);
	}
}



}
