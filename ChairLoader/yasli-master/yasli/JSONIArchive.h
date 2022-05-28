/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "Config.h"
#include "Pointers.h"
#include "yasli/Archive.h"
#include "Token.h"
#include <memory>

namespace yasli{

class MemoryReader;

class JSONIArchive : public Archive{
public:
	JSONIArchive();
	~JSONIArchive();

	bool load(const char* filename);
	bool open(const char* buffer, size_t length, bool free = false);

	bool operator()(bool& value, const char* name = "", const char* label = 0) override;
	bool operator()(char& value, const char* name = "", const char* label = 0) override;
	bool operator()(float& value, const char* name = "", const char* label = 0) override;
	bool operator()(double& value, const char* name = "", const char* label = 0) override;
	bool operator()(i8& value, const char* name = "", const char* label = 0) override;
	bool operator()(i16& value, const char* name = "", const char* label = 0) override;
	bool operator()(i32& value, const char* name = "", const char* label = 0) override;
	bool operator()(i64& value, const char* name = "", const char* label = 0) override;
	bool operator()(u8& value, const char* name = "", const char* label = 0) override;
	bool operator()(u16& value, const char* name = "", const char* label = 0) override;
	bool operator()(u32& value, const char* name = "", const char* label = 0) override;
	bool operator()(u64& value, const char* name = "", const char* label = 0) override;

	bool operator()(StringInterface& value, const char* name = "", const char* label = 0) override;
	bool operator()(WStringInterface& value, const char* name = "", const char* label = 0) override;
	bool operator()(const Serializer& ser, const char* name = "", const char* label = 0) override;
	bool operator()(ContainerInterface& ser, const char* name = "", const char* label = 0) override;
	bool operator()(KeyValueInterface& ser, const char* name = "", const char* label = 0) override;
	bool operator()(PointerInterface& ser, const char* name = "", const char* label = 0) override;

	using Archive::operator();
private:
	bool findName(const char* name, Token* outName = 0);
	bool openBracket();
	bool closeBracket();

	bool openContainerBracket();
	bool closeContainerBracket();

	void checkValueToken();
	bool checkStringValueToken();
	void readToken();
	void putToken();
	int line(const char* position) const; 
	bool isName(Token token) const;

	bool expect(char token);
	void skipBlock();

	struct Level{
		const char* start;
		const char* firstToken;
		bool isContainer;
		bool isKeyValue;
		Level() : isContainer(false), isKeyValue(false) {}
	};
	typedef std::vector<Level> Stack;
	Stack stack_;

	std::auto_ptr<MemoryReader> reader_;
	Token token_;
	std::vector<char> unescapeBuffer_;
	std::string filename_;
};

double parseFloat(const char* s);

}
