#pragma once
class IEntityArchetype
{
public:
	virtual ~IEntityArchetype(IEntityArchetype* this) = 0;
	virtual IEntityClass* GetClass(IEntityArchetype* this) = 0;
	virtual const char* GetName(IEntityArchetype* this) = 0;
	virtual unsigned __int64 GetId(IEntityArchetype* this) = 0;
	virtual const IScriptTable* GetProperties(IEntityArchetype* this) = 0;
	virtual IScriptTable* GetProperties(IEntityArchetype* this) = 0;
	virtual XmlNodeRef* GetObjectVars(IEntityArchetype* this, XmlNodeRef* result) = 0;
	virtual void LoadFromXML(IEntityArchetype* this, XmlNodeRef*, XmlNodeRef*) = 0;
};