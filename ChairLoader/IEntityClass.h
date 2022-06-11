#pragma once
#include <memory>

#include "IEntity.h"

class IEntityClass
{
public:
	virtual ~IEntityClass(IEntityClass* this) = 0;
	virtual void Release(IEntityClass* this) = 0;
	virtual const char* GetName(IEntityClass* this) = 0;
	virtual unsigned int GetFlags(IEntityClass* this) = 0;
	virtual void SetFlags(IEntityClass* this, unsigned int) = 0;
	virtual const char* GetScriptFile(IEntityClass* this) = 0;
	virtual IEntityScript* GetIEntityScript(IEntityClass* this) = 0;
	virtual IScriptTable* GetScriptTable(IEntityClass* this) = 0;
	virtual IEntityPropertyHandler* GetPropertyHandler(IEntityClass* this) = 0;
	virtual IEntityEventHandler* GetEventHandler(IEntityClass* this) = 0;
	virtual IEntityScriptFileHandler* GetScriptFileHandler(IEntityClass* this) = 0;
	virtual const SEditorClassInfo* GetEditorClassInfo(IEntityClass* this) = 0;
	virtual void SetEditorClassInfo(IEntityClass* this, const SEditorClassInfo*) = 0;
	virtual bool LoadScript(IEntityClass* this, bool) = 0;
	virtual std::shared_ptr<IEntityProxy>* GetUserProxyCreateFunc(IEntityClass* this)(std::shared_ptr<IEntityProxy>* result, IEntity*, SEntitySpawnParams*, void*) = 0;
	virtual void* GetUserProxyData(IEntityClass* this) = 0;
	virtual int GetEventCount(IEntityClass* this) = 0;
	virtual IEntityClass::SEventInfo* GetEventInfo(IEntityClass* this, IEntityClass::SEventInfo* result, int) = 0;
	virtual bool FindEventInfo(IEntityClass* this, const char*, IEntityClass::SEventInfo*) = 0;
	virtual const DynArray<std::shared_ptr<IEntityAttribute>, int, NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > >* GetClassAttributes(IEntityClass* this) = 0;
	virtual DynArray<std::shared_ptr<IEntityAttribute>, int, NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > >* GetClassAttributes(IEntityClass* this) = 0;
	virtual const DynArray<std::shared_ptr<IEntityAttribute>, int, NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > >* GetEntityAttributes(IEntityClass* this) = 0;
	virtual DynArray<std::shared_ptr<IEntityAttribute>, int, NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > >* GetEntityAttributes(IEntityClass* this) = 0;
	virtual void GetMemoryUsage(IEntityClass* this, ICrySizer*) = 0;
};
