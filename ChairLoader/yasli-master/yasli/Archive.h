/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#ifdef _MSC_VER
#pragma warning (disable : 4100) 
#endif

#include "yasli/Config.h"
#include "yasli/Helpers.h"
#include "yasli/Serializer.h"
#include "yasli/KeyValue.h"
#include "yasli/TypeID.h"

namespace yasli{

class Archive;

template<class T>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, T& object, const char* name, const char* label);

class Object;
class KeyValueInterface;
class EnumDescription;
template <class Enum>
EnumDescription& getEnumDescription();
template <class Enum>
bool serializeEnum(const EnumDescription& desc, Archive& ar, Enum& value, const char* name, const char* label);

// Context is used to pass arguments to nested serialization functions.
// Example of usage:
//
// void EntityLibrary::serialize(yasli::Archive& ar)
// {
//   yasli::Context libraryContext(ar, this);
//   ar(entities, "entities");
// }
//
// void Entity::serialize(yasli::Archive& ar)
// {
//   if (EntityLibrary* library = ar.context<EntityLibrary>()) {
//     ...
//   }
// }
//
// You may have multiple contexts of different types, but note that the context
// lookup complexity is O(n) in respect to number of contexts.
struct Context {
	void* object;
	TypeID type;
	Context* previousContext;
	Archive* archive;

	Context() : object(0), previousContext(0) {}
	template<class T>
	void set(T* object);
	template<class T>
	Context(Archive& ar, T* context);
	template<class T>
	Context(T* context);
	~Context();
};

class Archive{
public:
	enum ArchiveCaps{
		INPUT = 1 << 0,
		OUTPUT = 1 << 1,
		TEXT = 1 << 2,
		BINARY = 1 << 3,
		EDIT = 1 << 4,
		CUSTOM1 = 1 << 5,
		CUSTOM2 = 1 << 6,
		CUSTOM3 = 1 << 7
	};

	Archive(int caps)
	: lastContext_(0)
	, caps_(caps)
	, filter_(YASLI_DEFAULT_FILTER)
	{
	}
	virtual ~Archive() {}

	bool isInput() const{ return caps_ & INPUT ? true : false; }
	bool isOutput() const{ return caps_ & OUTPUT ? true : false; }
	bool isEdit() const{ return caps_ & EDIT ? true : false; }
	bool caps(int caps) const { return (caps_ & caps) == caps; }

	void setFilter(int filter){
		filter_ = filter;
	}
	int getFilter() const{ return filter_; }
	bool filter(int flags) const{
		YASLI_ASSERT(flags != 0 && "flags is supposed to be a bit mask");
		YASLI_ASSERT(filter_ && "Filter is not set!");
		return (filter_ & flags) != 0;
	}

	virtual void warning(const char* message) {}
	virtual bool operator()(bool& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(char& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(u8& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(i8& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(i16& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(u16& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(i32& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(u32& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(i64& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(u64& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(float& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(double& value, const char* name = "", const char* label = 0) { notImplemented(); return false; }

	virtual bool operator()(StringInterface& value, const char* name = "", const char* label = 0)    { notImplemented(); return false; }
	virtual bool operator()(WStringInterface& value, const char* name = "", const char* label = 0)    { notImplemented(); return false; }
	virtual bool operator()(const Serializer& ser, const char* name = "", const char* label = 0) { notImplemented(); return false; }
	virtual bool operator()(ContainerInterface& ser, const char* name = "", const char* label = 0) { return false; }
	virtual bool operator()(PointerInterface& ptr, const char* name = "", const char* label = 0);
	virtual bool operator()(Object& obj, const char* name = "", const char* label = 0) { return false; }
	virtual bool operator()(KeyValueInterface& keyValue, const char* name = "", const char* label = 0) { return operator()(Serializer(keyValue), name, label); }

	// No point in supporting long double since it is represented as double on MSVC
	bool operator()(long double& value, const char* name = "", const char* label = 0)         { notImplemented(); return false; }

	// block call are osbolete, please do not use
	virtual bool openBlock(const char* name, const char* label) { return true; }
	virtual void closeBlock() {}

	// templated switch
	template<class T>
	bool operator()(const T& value, const char* name = "", const char* label = 0);

	template<class T>
	T* context() const {
		TypeID type = TypeID::get<T>();
		for (Context* current = lastContext_; current != 0; current = current->previousContext)
			if (current->type == type)
				return (T*)current->object;
		return 0;
	}
	Context* setLastContext(Context* context) {
		Context* previousContext = lastContext_;
		lastContext_ = context;
		return previousContext;
	}
	Context* lastContext() const{ return lastContext_; }
protected:
	Context* lastContext_;
	int caps_;

private:
	void notImplemented() { YASLI_ASSERT(0 && "Not implemented!"); }

	int filter_;
};

namespace Helpers{

template<class T>
struct SerializeStruct{
	static bool invoke(Archive& ar, T& value, const char* name, const char* label){
		Serializer serializer(value);
		return ar(serializer, name, label);
	};
};

template<class Enum>
struct SerializeEnum{
	static bool invoke(Archive& ar, Enum& value, const char* name, const char* label){
		const EnumDescription& enumDescription = getEnumDescription<Enum>();
		return serializeEnum(enumDescription, ar, value, name, label);
	};
};

template<class T>
struct SerializeArray{};

template<class T, int Size>
struct SerializeArray<T[Size]>{
	static bool invoke(Archive& ar, T value[Size], const char* name, const char* label){
		ContainerArray<T, Size> ser(value);
		return ar(static_cast<ContainerInterface&>(ser), name, label);
	}
};

}

template<class T>
Context::Context(Archive& ar, T* context) {
	archive = &ar;
	object = (void*)context;
	type = TypeID::get<T>();
	if (archive)
		previousContext = ar.setLastContext(this);
	else
		previousContext = 0;
}

template<class T>
Context::Context(T* context) {
    archive = 0;
    previousContext = 0;
    set<T>(context);
}

template<class T>
void Context::set(T* object) {
	this->object = (void*)object;
	type = TypeID::get<T>();
}

inline Context::~Context() {
	if (archive)
		archive->setLastContext(previousContext);
}

template<class T>
bool Archive::operator()(const T& value, const char* name, const char* label){
    return YASLI_SERIALIZE_OVERRIDE(*this, const_cast<T&>(value), name, label);
}

inline bool Archive::operator()(PointerInterface& ptr, const char* name, const char* label)
{
	Serializer ser(ptr);
	return operator()(ser, name, label);
}

template<class T, int Size>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, T object[Size], const char* name, const char* label)
{
	YASLI_ASSERT(0);
	return false;
}

template<class T>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, const T& object, const char* name, const char* label)
{
	T::unable_to_serialize_CONST_object();
	YASLI_ASSERT(0);
	return false;
}

// Archives are using yasli integer typedefs (taken from stdint.h). These
// types will be selected directly by Archive::operator() overloads. Those,
// however, do not cover all C++ integer types, as compilers usually have
// multiple integer types of the same type (like int and long in MSVC/x86). We
// would like to handle handle all C++ standard types, but not add duplicated
// and unportable methods to each archive implementations. The remaining types
// are cast to types of the same in functions below. E.g. long will be cast to
// int on MSVC/x86 and long long will be cast to long on GCC/x64.
template<class T, class CastTo>
struct CastInteger
{
	static bool invoke(Archive& ar, T& value, const char* name, const char* label)
	{
		return ar(reinterpret_cast<CastTo&>(value), name, label);
	}
};

template<class T>
bool castInteger(Archive& ar, T& v, const char* name, const char* label)
{
	using namespace Helpers;
	return 
		Select< IsSigned<T>,
			SelectIntSize< sizeof(T),
				CastInteger<T, i8>,
				CastInteger<T, i16>,
				CastInteger<T, i32>,
				CastInteger<T, i64>
			>,
			SelectIntSize< sizeof(T),
				CastInteger<T, u8>,
				CastInteger<T, u16>,
				CastInteger<T, u32>,
				CastInteger<T, u64>
			>
		>::type::invoke(ar, v, name, label);
}
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, unsigned char& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, unsigned short& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, unsigned int& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, unsigned long& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, unsigned long long& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed char& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed short& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed int& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed long& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, signed long long& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
#ifdef _NATIVE_WCHAR_T_DEFINED // MSVC
inline bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, wchar_t& v, const char* name, const char* label) { return castInteger(ar, v, name, label); }
#endif


template<class T>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, T& object, const char* name, const char* label)
{
	using namespace Helpers;

	return
		Select< IsClass<T>,
				Identity< SerializeStruct<T> >,
				Select< IsArray<T>,
					Identity< SerializeArray<T> >,
					Identity< SerializeEnum<T> >
				>
		>::type::invoke(ar, object, name, label);
}

}

#include "yasli/SerializerImpl.h"

// vim: ts=4 sw=4:
