/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <map>
#include <vector>

#include "yasli/Config.h"
#include "yasli/Assert.h"
#include "yasli/ClassFactoryBase.h"
#include "yasli/TypeID.h"

// #ifdef _MSC_VER
// # pragma warning(disable : 4251)
// #endif

namespace yasli{

class Archive;

class TypeDescription{
public:
	TypeDescription(TypeID typeID, const char* name, const char *label, std::size_t size)
	: name_(name)
	, label_(label)
	, size_(size)
	, typeID_(typeID)
	{
#if YASLI_NO_RTTI
		const size_t bufLen = sizeof(typeID.typeInfo_->name);
		strncpy(typeID.typeInfo_->name, name, bufLen - 1);
		typeID.typeInfo_->name[bufLen - 1] = '\0';
#endif
	}
	const char* name() const{ return name_; }
	const char* label() const{ return label_; }
	std::size_t size() const{ return size_; }
	TypeID typeID() const{ return typeID_; }

protected:
	const char* name_;
	const char* label_;
	std::size_t size_;
	TypeID typeID_;
};

class ClassFactoryManager{
public:
	static ClassFactoryManager& the(){
		static ClassFactoryManager factoryManager;
		return factoryManager;
	}

	const ClassFactoryBase* find(TypeID baseType) const{
		Factories::const_iterator it = factories_.find(baseType);
		if(it == factories_.end())
			return 0;
		else
			return it->second;
	}

	void registerFactory(TypeID type, const ClassFactoryBase* factory){
		factories_[type] = factory;
	}
protected:
	typedef std::map<TypeID, const ClassFactoryBase*> Factories;
	Factories factories_;
};

template<class BaseType>
class ClassFactory : public ClassFactoryBase{
public:
	static ClassFactory& the(){
		static ClassFactory factory;
		return factory;
	}

	class CreatorBase{
	public:
		CreatorBase() {
			if (head() == this) {
				YASLI_ASSERT(0);
			}
			if (head()) {
				next = head();
				head()->previous = this;
			}
			else {
				next = 0;
				previous = 0;
			}
			head() = this;
		}

		virtual ~CreatorBase() {
			if (previous) {
				previous->next = next;
			}
			if (this == head()) {
				head() = next;
				if (next)
					next->previous = 0;
			}
		}
		virtual BaseType* create() const = 0;
		const TypeDescription& description() const{ return *description_; }
#if YASLI_NO_RTTI
		void* vptr() const{ return vptr_; }
#endif
		static CreatorBase*& head() { static CreatorBase* head; return head; }
		CreatorBase* next;
		CreatorBase* previous;
	protected:
		const TypeDescription* description_;
#if YASLI_NO_RTTI
		void* vptr_;
#endif
	};

#if YASLI_NO_RTTI
	static void* extractVPtr(BaseType* ptr)
	{
		return *((void**)ptr);
	}

#endif

	template<class Derived>
	class Creator : public CreatorBase{
	public:
		Creator(const TypeDescription* description){
			this->description_ = description;
#if YASLI_NO_RTTI
			// TODO: remove unnecessary static initialisation
			Derived vptrProbe;
            CreatorBase::vptr_ = extractVPtr(&vptrProbe);
#endif
			ClassFactory::the().registerCreator(this);
		}
		BaseType* create() const{
			return new Derived();
		}
	};

	ClassFactory()
	: ClassFactoryBase(TypeID::get<BaseType>())
	{
		ClassFactoryManager::the().registerFactory(baseType_, this);
	}

	typedef std::map<TypeID, CreatorBase*> TypeToCreatorMap;

	BaseType* create(TypeID derivedType) const
	{
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(derivedType);
		if(it != typeToCreatorMap_.end())
			return it->second->create();
		else{
			YASLI_ASSERT(!strlen(derivedType.name()), "ClassFactory::create: undefined type %s", derivedType.name());
			return 0;
		}
	}

	TypeID getTypeID(BaseType* ptr) const
	{
#if YASLI_NO_RTTI
		if (ptr == 0)
			return TypeID();
		void* vptr = extractVPtr(ptr);
        typename VPtrToCreatorMap::const_iterator it = vptrToCreatorMap_.find(vptr);
		if (it == vptrToCreatorMap_.end())
			return TypeID();
		return it->second->description().typeID();
#else
		return TypeID(typeid(*ptr));
#endif
	}

	size_t sizeOf(TypeID derivedType) const
	{
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(derivedType);
		if(it != typeToCreatorMap_.end())
			return it->second->description().size();
		else
			return 0;
	}

	BaseType* createByIndex(int index) const
	{
		YASLI_ASSERT(size_t(index) < creators_.size());
		return creators_[index]->create();
	}

	void serializeNewByIndex(Archive& ar, int index, const char* name, const char* label)
	{
		YASLI_ESCAPE(size_t(index) < creators_.size(), return);
		BaseType* ptr = creators_[index]->create();
		ar(*ptr, name, label);
		delete ptr;
	}
	// from ClassFactoryInterface:
	size_t size() const{ return creators_.size(); }
	const TypeDescription* descriptionByIndex(int index) const{
		if(size_t(index) >= int(creators_.size()))
			return 0;
		return &creators_[index]->description();
	}

	const TypeDescription* descriptionByType(TypeID type) const{
		const size_t numCreators = creators_.size();
		for (size_t i = 0; i < numCreators; ++i) {
			if (type == creators_[i]->description().typeID())
				return &creators_[i]->description();
		}
		return 0;
	}

	TypeID findTypeByName(const char* name) const {
		const size_t numCreators = creators_.size();
		for (size_t i = 0; i < numCreators; ++i) {
			const TypeDescription& description = creators_[i]->description();
			if (strcmp(name, description.name()) == 0)
				return description.typeID();
		}
		YASLI_ASSERT(!strlen(name), "ClassFactory::findTypeByName: undefined type %s", name);
		return TypeID();
	}
	// ^^^
	
	CreatorBase* creatorChain() { return CreatorBase::head(); }
	
	void registerChain(CreatorBase* head)
	{
		CreatorBase* current = head;
		while (current) {
			registerCreator(current);
			current = current->next;
		}
	}

	void unregisterChain(CreatorBase* head)
	{
		CreatorBase* current = head;
		while (current) {
			typeToCreatorMap_.erase(current->description().typeID());
#if YASLI_NO_RTTI
			vptrToCreatorMap_.erase(current->vptr());
#endif
			for (size_t i = 0; i < creators_.size(); ++i) {
				if (creators_[i] == current) {
					creators_.erase(creators_.begin() + i);
					--i;
				}
			}
			current = current->next;
		}
	}
protected:
	void registerCreator(CreatorBase* creator){
		typeToCreatorMap_[creator->description().typeID()] = creator;
		creators_.push_back(creator);
#if YASLI_NO_RTTI
		vptrToCreatorMap_[creator->vptr()] =  creator;
#endif
	}

	TypeToCreatorMap typeToCreatorMap_;
	std::vector<CreatorBase*> creators_;

#if YASLI_NO_RTTI
	typedef std::map<void*, CreatorBase*> VPtrToCreatorMap;
	VPtrToCreatorMap vptrToCreatorMap_;
#endif
};

}

#define YASLI_JOIN_HELPER(x,y,z,w) x ## y ## z ## w
#define YASLI_JOIN(x,y,z,w) YASLI_JOIN_HELPER(x,y,z,w)

#define YASLI_CLASS_NULL_HELPER(Counter, BaseType, name) \
	static bool YASLI_JOIN(baseType, _NullRegistered_, _,Counter) = yasli::ClassFactory<BaseType>::the().setNullLabel(name); \


#define YASLI_CLASS_NULL(BaseType, name) \
	YASLI_CLASS_NULL_HELPER(__COUNTER__, BaseType, name)

#define YASLI_CLASS(BaseType, Type, label) \
	static const yasli::TypeDescription Type##BaseType##_DerivedDescription(yasli::TypeID::get<Type>(), #Type, label, sizeof(Type)); \
	static yasli::ClassFactory<BaseType>::Creator<Type> Type##BaseType##_Creator(&Type##BaseType##_DerivedDescription); 

#define YASLI_CLASS_NAME_HELPER(Counter, BaseType, Type, name, label) \
	static yasli::TypeDescription YASLI_JOIN(globalDerivedDescription,__LINE__,_,Counter)(yasli::TypeID::get<Type>(), name, label, sizeof(Type)); \
	static const yasli::ClassFactory<BaseType>::Creator<Type> YASLI_JOIN(globalCreator,__LINE__,_,Counter)(&YASLI_JOIN(globalDerivedDescription,__LINE__,_,Counter)); \

#define YASLI_CLASS_NAME(BaseType, Type, name, label) \
	YASLI_CLASS_NAME_HELPER(__COUNTER__, BaseType, Type, name, label)

#define YASLI_FORCE_CLASS(BaseType, Type) \
	extern int dummyForType_##Type##BaseType; \
	int* dummyForTypePtr_##Type##BaseType = &dummyForType_##Type##BaseType + 1;


