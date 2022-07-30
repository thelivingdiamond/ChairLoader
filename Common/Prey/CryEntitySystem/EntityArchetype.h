// Header file automatically created from a PDB.

#pragma once
#include <Prey/CryEntitySystem/EntityArchetype.h>
#include <Prey/CryEntitySystem/IEntitySystem.h>
#include <Prey/CrySystem/XML/IXml.h>
#include <Prey/GameDll/ark/arkscripttable.h>
#include <Prey/CryEntitySystem/IEntityClass.h>
#include <ChairLoader/PreyFunction.h>

class CEntityArchetype;
class IEntityArchetypeListener;
struct IEntityClass;
struct IScriptTable;

// Header: FromCpp
// CryEngine/cryentitysystem/entityarchetype.h
class CEntityArchetypeManager // Id=800CB3E Size=112
{
public:
	using ArchetypesNameMap = std::map<char const *,_smart_ptr<CEntityArchetype>,std::less<char const *>>;
	using ArchetypesIdMap = std::unordered_map<unsigned __int64,_smart_ptr<CEntityArchetype>>;
	
	ArchetypesNameMap m_nameToArchetypeMap;
	ArchetypesIdMap m_idToArchetypeMap;
	DynArray<string,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc>>> m_loadedLibs;
	std::vector<IEntityArchetypeListener *> m_listeners;
	
	IEntityArchetype *CreateArchetype(IEntityClass *pClass, const char *sArchetype, uint64_t _id) { return FCreateArchetype(this,pClass,sArchetype,_id); }
	IEntityArchetype *FindArchetype(const char *sArchetype) { return FFindArchetypeOv1(this,sArchetype); }
	IEntityArchetype *LoadArchetype(const char *sArchetype) { return FLoadArchetype(this,sArchetype); }
	IEntityArchetype *GetArchetype(uint64_t _id) { return FGetArchetypeOv1(this,_id); }
	void DeleteArchetype(const char *sArchetype) { FDeleteArchetype(this,sArchetype); }
	void ReloadAllArchetypes() { FReloadAllArchetypes(this); }
	void ReloadArchetype(const char *_sArchetypeName) { FReloadArchetype(this,_sArchetypeName); }
	void RenameArchetype(const char *_archetypeName, const char *_newArchetypeName) { FRenameArchetype(this,_archetypeName,_newArchetypeName); }
	void Reset() { FReset(this); }
	std::vector<string> GetArchetypeNames(string const &_libraryName) const { return FGetArchetypeNames(this,_libraryName); }
	std::vector<IEntityArchetype *> GetArchetypes(string const &_libraryName) const { return FGetArchetypes(this,_libraryName); }
	bool LoadLibrary(string const &library) { return FLoadLibrary(this,library); }
	void RegisterListener(IEntityArchetypeListener *_pListener) { FRegisterListener(this,_pListener); }
	void UnregisterListener(IEntityArchetypeListener *_pListener) { FUnregisterListener(this,_pListener); }
	std::vector<std::pair<char const * const,_smart_ptr<CEntityArchetype> >,std::allocator<std::pair<char const * const,_smart_ptr<CEntityArchetype> > > > GetArchetypeInfoFromMap(string const &_libraryName) const { return FGetArchetypeInfoFromMap(this,_libraryName); }
	
#if 0
	IEntityArchetype const *FindArchetype(const char *arg0) const;
	IEntityArchetype const *GetArchetype(uint64_t arg0) const;
	static string GetLibraryFromName(string const &arg0);
#endif
	
	static inline auto FCreateArchetype = PreyFunction<IEntityArchetype *(CEntityArchetypeManager *const _this, IEntityClass * pClass, const char *sArchetype, uint64_t _id)>(0x90F5E0);
	static inline auto FFindArchetypeOv1 = PreyFunction<IEntityArchetype *(CEntityArchetypeManager *const _this, const char *sArchetype)>(0x90FB70);
	static inline auto FLoadArchetype = PreyFunction<IEntityArchetype *(CEntityArchetypeManager *const _this, const char *sArchetype)>(0x9100E0);
	static inline auto FGetArchetypeOv1 = PreyFunction<IEntityArchetype *(CEntityArchetypeManager *const _this, uint64_t _id)>(0x90FC30);
	static inline auto FDeleteArchetype = PreyFunction<void(CEntityArchetypeManager *const _this, const char *sArchetype)>(0x90FAD0);
	static inline auto FReloadAllArchetypes = PreyFunction<void(CEntityArchetypeManager *const _this)>(0x910C90);
	static inline auto FReloadArchetype = PreyFunction<void(CEntityArchetypeManager *const _this, const char *_sArchetypeName)>(0x910DE0);
	static inline auto FRenameArchetype = PreyFunction<void(CEntityArchetypeManager *const _this, const char *_archetypeName, const char *_newArchetypeName)>(0x910EA0);
	static inline auto FReset = PreyFunction<void(CEntityArchetypeManager *const _this)>(0x911060);
	static inline auto FGetArchetypeNames = PreyFunction<std::vector<string>(CEntityArchetypeManager const *const _this, string const &_libraryName)>(0x90FEC0);
	static inline auto FGetArchetypes = PreyFunction<std::vector<IEntityArchetype *>(CEntityArchetypeManager const *const _this, string const &_libraryName)>(0x910000);
	static inline auto FLoadLibrary = PreyFunction<bool(CEntityArchetypeManager *const _this, string const &library)>(0x910430);
	static inline auto FRegisterListener = PreyFunction<void(CEntityArchetypeManager *const _this, IEntityArchetypeListener *_pListener)>(0x910BD0);
	static inline auto FUnregisterListener = PreyFunction<void(CEntityArchetypeManager *const _this, IEntityArchetypeListener *_pListener)>(0x911290);
	static inline auto FGetArchetypeInfoFromMap = PreyFunction<std::vector<std::pair<char const * const,_smart_ptr<CEntityArchetype> >,std::allocator<std::pair<char const * const,_smart_ptr<CEntityArchetype> > > >(CEntityArchetypeManager const *const _this, string const &_libraryName)>(0x90FC80);
};

// Header: Exact
// CryEngine/cryentitysystem/entityarchetype.h
class CEntityArchetype : public IEntityArchetype, public _i_reference_target<int> // Id=800CB45 Size=64
{
public:
	string m_name;
	uint64_t m_id;
	SmartScriptTable m_pProperties;
	XmlNodeRef m_ObjectVars;
	IEntityClass *m_pClass;
	
	virtual IEntityClass *GetClass() const;
	virtual const char *GetName() const;
	virtual uint64_t GetId() const;
	virtual IScriptTable *GetProperties();
	virtual IScriptTable const *GetProperties() const;
	virtual XmlNodeRef GetObjectVars();
	virtual void LoadFromXML(XmlNodeRef &propertiesNode, XmlNodeRef &objectVarsNode);
	void Reload() { FReload(this); }
	virtual ~CEntityArchetype();
	
#if 0
	void SetName(string const &arg0);
	void SetId(const uint64_t arg0);
#endif
	
	static inline auto FGetClass = PreyFunction<IEntityClass *(CEntityArchetype * _this)>(0x864AF0);
	static inline auto FGetName = PreyFunction<const char *(CEntityArchetype const *const _this)>(0xD87500);
	static inline auto FGetId = PreyFunction<uint64_t(CEntityArchetype const *const _this)>(0x1140FE0);
	static inline auto FGetPropertiesOv1 = PreyFunction<IScriptTable *(CEntityArchetype *const _this)>(0xD87510);
	static inline auto FGetPropertiesOv0 = PreyFunction<IScriptTable const *(CEntityArchetype const *const _this)>(0xD87510);
	static inline auto FGetObjectVars = PreyFunction<XmlNodeRef(CEntityArchetype *const _this)>(0x9100B0);
	static inline auto FLoadFromXML = PreyFunction<void(CEntityArchetype *const _this, XmlNodeRef &propertiesNode, XmlNodeRef &objectVarsNode)>(0x910260);
	static inline auto FReload = PreyFunction<void(CEntityArchetype *const _this)>(0x910C20);
};

