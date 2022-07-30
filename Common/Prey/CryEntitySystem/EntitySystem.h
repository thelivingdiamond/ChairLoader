// Header file automatically created from a PDB.

#pragma once
#include <Prey/CryEntitySystem/EntitySystem.h>
#include <Prey/CryEntitySystem/EntityTimeoutList.h>
#include <Prey/CryEntitySystem/IEntity.h>
#include <Prey/CryEntitySystem/IEntitySystem.h>
#include <Prey/CryEntitySystem/SaltBufferArray.h>
#include <Prey/CryMath/Cry_Geo.h>
#include <Prey/CryNetwork/ISerialize.h>
#include <Prey/CrySystem/TimeValue.h>
#include <Prey/CrySystem/XML/IXml.h>
#include <Prey/CryEntitySystem/SEntityLayerGarbage.h>

class CAreaManager;
class CCharacterBoneAttachmentManager;
class CComponentEventDistributer;
class CEntity;
class CEntityArchetypeManager;
class CEntityClassRegistry;
class CEntityLayer;
class CEntityLoadManager;
class CEntityPoolManager;
class CGeomCacheAttachmentManager;
class CPartitionGrid;
class CPhysicsEventListener;
class CProximityTriggerSystem;
class CScriptBind_Entity;
struct IAreaManager;
struct IBSPTree3D;
struct IBreakableManager;
class ICrySizer;
struct IEntity;
struct IEntityArchetype;
class IEntityArchetypeListener;
struct IEntityClass;
struct IEntityClassRegistry;
struct IEntityEventListener;
struct IEntityIt;
struct IEntityPoolManager;
struct IEntitySystemSink;
struct IPhysicalEntity;
struct ISystem;
struct SEntityEvent;
struct SEntityProximityQuery;
struct SEntityTimerEvent;
struct SEntityUpdateContext;

// Header: Exact
// CryEngine/cryentitysystem/entitysystem.h
class CEntitySystem : public IEntitySystem // Id=800C6B4 Size=263808
{
public:
	std::unordered_map<unsigned int,float> m_queuedEntityVisibilities;
	
	struct OnEventSink // Id=800C9A8 Size=16
	{
		uint64_t subscriptions;
		IEntitySystemSink *pSink;
	};

	using EntitySystemOnEventSinks = std::vector<CEntitySystem::OnEventSink,stl::STLGlobalAllocator<CEntitySystem::OnEventSink> >;
	using EntitySystemSinks = std::vector<IEntitySystemSink *,stl::STLGlobalAllocator<IEntitySystemSink *> >;
	using DeletedEntities = std::vector<CEntity *>;
	using EntityTimersMap = std::multimap<CTimeValue,SEntityTimerEvent,std::less<CTimeValue>>;
	using EntityNamesMap = std::multimap<char const *,unsigned int,std::less<char const *>>;
	using EntitiesMap = std::map<unsigned int,CEntity *,std::less<unsigned int>>;
	using EntitiesSet = std::set<unsigned int,std::less<unsigned int>>;
	using EntityTimersVector = std::vector<SEntityTimerEvent>;
	
	std::vector<IEntitySystemSink *,stl::STLGlobalAllocator<IEntitySystemSink *> > m_sinks[5];
	std::vector<CEntitySystem::OnEventSink,stl::STLGlobalAllocator<CEntitySystem::OnEventSink> > m_onEventSinks;
	ISystem *m_pISystem;
	std::vector<CEntity *> m_EntityArray;
	std::vector<CEntity *> m_deletedEntities;
	std::vector<CEntity *> m_deferredUsedEntities;
	std::map<unsigned int,CEntity *,std::less<unsigned int>> m_mapActiveEntities;
	bool m_tempActiveEntitiesValid;
	std::set<unsigned int,std::less<unsigned int>> m_mapPrePhysicsEntities;
	EntityNamesMap m_mapEntityNames;
	CSaltBufferArray<unsigned short,unsigned short,65533> m_EntitySaltBuffer;
	std::vector<unsigned int> m_tempActiveEntities;
	CComponentEventDistributer *m_pEventDistributer;
	EntityTimersMap m_timersMap;
	std::vector<SEntityTimerEvent> m_currentTimers;
	bool m_bTimersPause;
	CTimeValue m_nStartPause;
	CScriptBind_Entity *m_pEntityScriptBinding;
	CEntityClassRegistry *m_pClassRegistry;
	CPhysicsEventListener *m_pPhysicsEventListener;
	CAreaManager *m_pAreaManager;
	CEntityLoadManager *m_pEntityLoadManager;
	CEntityPoolManager *m_pEntityPoolManager;
	
	using EventListenersMap = std::multimap<unsigned int,IEntityEventListener *,std::less<unsigned int>,std::allocator<std::pair<unsigned int const ,IEntityEventListener *> > >;
	
	std::multimap<unsigned int,IEntityEventListener *,std::less<unsigned int>,std::allocator<std::pair<unsigned int const ,IEntityEventListener *> > > m_eventListeners[60];
	int m_eventListenerBeingProcessedRefCount;
	
	using EntityGuidMap = std::map<unsigned __int64,unsigned int,std::less<unsigned __int64>>;
	
	std::map<unsigned __int64,unsigned int,std::less<unsigned __int64>> m_guidMap;
	std::map<unsigned __int64,unsigned int,std::less<unsigned __int64>> m_genIdMap;
	IBreakableManager *m_pBreakableManager;
	CEntityArchetypeManager *m_pEntityArchetypeManager;
	CGeomCacheAttachmentManager *m_pGeomCacheAttachmentManager;
	CCharacterBoneAttachmentManager *m_pCharacterBoneAttachmentManager;
	CPartitionGrid *m_pPartitionGrid;
	CProximityTriggerSystem *m_pProximityTriggerSystem;
	unsigned m_idForced;
	bool m_bLocked;
	CEntityTimeoutList m_entityTimeoutList;
	
	class CCompareEntityIdsByClass // Id=800CB9E Size=8
	{
	public:
		CEntity **m_ppEntities;
		
#if 0
		bool operator()(unsigned arg0, unsigned arg1) const;
#endif
	};

	using TLayers = std::map<string,CEntityLayer *,std::less<string>,std::allocator<std::pair<CryStringT<char> const,CEntityLayer *>>>;
	using THeaps = std::vector<SEntityLayerGarbage>;
	
	std::map<string,CEntityLayer *,std::less<string>,std::allocator<std::pair<CryStringT<char> const,CEntityLayer *>>> m_layers;
	std::vector<SEntityLayerGarbage> m_garbageLayerHeaps;
	std::set<IEntity const *,std::less<IEntity const *>> m_EntitiesUsingPlayerTime;
	std::vector<IEntityClass const *> m_areaTriggeringClasses;
	AABB m_safeBounds;
	bool m_bReseting;
	
	CEntitySystem(ISystem *pSystem);
	virtual ~CEntitySystem();
	bool Init(ISystem *pSystem) { return FInit(this,pSystem); }
	virtual void RegisterCharactersForRendering();
	virtual void Release();
	virtual void Update();
	virtual void DeletePendingEntities();
	virtual void PrePhysicsUpdate();
	virtual void Reset();
	virtual void Unload();
	virtual void PurgeHeaps();
	virtual void EditorReset(bool _bEnteringGameMode);
	virtual IEntityClassRegistry *GetClassRegistry();
	IEntity *SpawnEntityInternal(SEntitySpawnParams &params, bool bAutoInit) { return FSpawnEntityInternal(this,params,bAutoInit); }
	virtual IEntity *SpawnEntityFromArchetype(IEntityArchetype *_pArchetype, SEntitySpawnParams &_params, bool _bAutoInit);
	virtual IEntity *SpawnEntity(SEntitySpawnParams &_params, bool _bAutoInit);
	virtual bool InitEntity(IEntity *pEntity, SEntitySpawnParams &params);
	virtual IEntity *GetEntity(unsigned id) const;
	virtual IEntity *ArkSafeGetEntity(unsigned _id) const;
	virtual unsigned GetClonedEntityId(unsigned origId, unsigned refId) const;
	virtual IEntity *FindEntityByName(const char *sEntityName) const;
	virtual void ReserveEntityId(const unsigned id);
	virtual unsigned ReserveUnknownEntityId();
	virtual void RemoveEntity(unsigned entity, bool bForceRemoveNow);
	virtual unsigned GetNumEntities() const;
	virtual IEntityIt *GetEntityIterator();
	virtual void SendEventViaEntityEvent(IEntity *piEntity, SEntityEvent &event);
	virtual void SendEventToAll(SEntityEvent &event);
	virtual int QueryProximity(SEntityProximityQuery &query);
	virtual void ResizeProximityGrid(int nWidth, int nHeight);
	virtual int GetPhysicalEntitiesInBox(Vec3 const &origin, float radius, IPhysicalEntity **&pList, int physFlags) const;
	virtual IEntity *GetEntityFromPhysics(IPhysicalEntity *pPhysEntity) const;
	virtual void AddSink(IEntitySystemSink *pSink, unsigned subscriptions, uint64_t onEventSubscriptions);
	virtual void RemoveSink(IEntitySystemSink *pSink);
	virtual void PauseTimers(bool bPause, bool bResume);
	virtual bool IsIDUsed(unsigned nID) const;
	virtual void GetMemoryStatistics(ICrySizer *pSizer) const;
	virtual ISystem *GetSystem() const;
	virtual void SetNextSpawnId(unsigned id);
	virtual void ResetAreas();
	virtual void UnloadAreas();
	virtual void AddEntityEventListener(unsigned nEntity, EEntityEvent event, IEntityEventListener *pListener);
	virtual void RemoveEntityEventListener(unsigned nEntity, EEntityEvent event, IEntityEventListener *pListener);
	virtual bool HasEntityEventListener(IEntityEventListener const &_listener) const;
	virtual unsigned FindEntityByGuid(const uint64_t &guid) const;
	virtual unsigned FindEntityByEditorGuid(const char *pGuid) const;
	virtual IEntityArchetype *LoadEntityArchetype(XmlNodeRef oArchetype);
	virtual IEntityArchetype *LoadEntityArchetype(const char *sArchetype);
	virtual IEntityArchetype *FindEntityArchetype(const char *_archetypeName) const;
	virtual IEntityArchetype *CreateEntityArchetype(IEntityClass *pClass, const char *sArchetype, uint64_t _id);
	virtual IEntityArchetype *GetEntityArchetype(uint64_t _id);
	virtual IEntityArchetype const *GetEntityArchetype(uint64_t _id) const;
	virtual uint64_t GetArchetypeId(const char *_archetypeName) const;
	virtual void DeleteEntityArchetype(const char *_archetypeName);
	virtual void RenameArchetype(const char *_archetypeName, const char *_newArchetypeName) const;
	virtual std::vector<string> GetArchetypeNames(string const &_libraryName) const;
	virtual std::vector<IEntityArchetype *> GetArchetypes(string const &_libraryName) const;
	virtual bool LoadArchetypeLibrary(string const &_libraryName);
	virtual void RegisterEntityArchetypeListener(IEntityArchetypeListener *_pListener);
	virtual void UnregisterEntityArchetypeListener(IEntityArchetypeListener *_pListener);
	virtual void Serialize(TSerialize ser);
	virtual void DumpEntities();
	virtual int GetLayerId(const char *szLayerName) const;
	virtual const char *GetLayerName(int layerId) const;
	virtual int GetLayerChildCount(const char *szLayerName) const;
	virtual const char *GetLayerChild(const char *szLayerName, int childIdx) const;
	virtual int GetVisibleLayerIDs(uint8_t *pLayerMask, const unsigned maxCount) const;
	virtual void LockSpawning(bool lock);
	virtual bool IsSpawningLocked() const;
	virtual bool OnLoadLevel(const char *szLevelPath);
	virtual void OnLevelEnd();
	void OnLevelLoadStart() { FOnLevelLoadStart(this); }
	virtual CEntityLayer *AddLayer(const char *name, const char *parent, uint16_t id, bool bHasPhysics, int specs, bool bDefaultLoaded, int _loadTokenId, int _loadTokenValue);
	virtual std::vector<bool> GetLayersState() const;
	virtual void LoadLayers(const char *dataFile);
	virtual void LinkLayerChildren();
	virtual void AddEntityToLayer(const char *layer, unsigned id);
	virtual void RemoveEntityFromLayers(unsigned id);
	virtual const char *GetLayerNameForEntity(unsigned id);
	virtual void ClearLayers();
	virtual void EnableDefaultLayers(bool isSerialized);
	virtual void EnableLayer(const char *layer, bool _bIsVisible, bool isEnable, bool isSerialized);
	virtual bool IsLayerEnabled(const char *layer, bool bMustBeLoaded) const;
	virtual bool ShouldSerializedEntity(IEntity *pEntity);
	virtual void RegisterPhysicCallbacks();
	virtual void UnregisterPhysicCallbacks();
	CEntityLayer *FindLayer(const char *layer) { return FFindLayer(this,layer); }
	virtual void MarkEntityAsUsingPlayerTime(IEntity const &rEntity);
	virtual void UnmarkEntityAsUsingPlayerTime(IEntity const &rEntity);
	virtual bool ShouldEntityUsePlayerTime(IEntity const &rEntity) const;
	CEntityLayer *GetLayerForEntity(unsigned id) { return FGetLayerForEntity(this,id); }
	bool OnBeforeSpawn(SEntitySpawnParams &params) { return FOnBeforeSpawn(this,params); }
	void OnEntityReused(IEntity *pEntity, SEntitySpawnParams &params) { FOnEntityReused(this,pEntity,params); }
	void AddTimerEvent(SEntityTimerEvent &event, CTimeValue startTime) { FAddTimerEvent(this,event,startTime); }
	virtual void LoadEntities(XmlNodeRef &objectsNode, bool bIsLoadingLevelFile);
	virtual void LoadEntities(XmlNodeRef &objectsNode, bool bIsLoadingLevelFile, Vec3 const &segmentOffset, std::vector<IEntity *> *outGlobalEntityIds, std::vector<IEntity *> *outLocalEntityIds);
	virtual void HoldLayerEntities(const char *pLayerName);
	virtual void CloneHeldLayerEntities(const char *pLayerName, Vec3 const &localOffset, Matrix34 const &l2w, const char **pExcludeLayers, int numExcludeLayers);
	virtual void ReleaseHeldEntities();
	virtual bool ExtractEntityLoadParams(XmlNodeRef &entityNode, SEntitySpawnParams &spawnParams) const;
	virtual void BeginCreateEntities(int nAmtToCreate);
	virtual bool CreateEntity(XmlNodeRef &entityNode, SEntitySpawnParams &pParams, unsigned &outUsingId);
	virtual void EndCreateEntities();
	void RemoveTimerEvent(unsigned id, int nTimerId) { FRemoveTimerEvent(this,id,nTimerId); }
	void ActivateEntity(CEntity *pEntity, bool bActivate) { FActivateEntity(this,pEntity,bActivate); }
	void ActivatePrePhysicsUpdateForEntity(CEntity *pEntity, bool bActivate) { FActivatePrePhysicsUpdateForEntity(this,pEntity,bActivate); }
	bool IsPrePhysicsActive(CEntity *pEntity) { return FIsPrePhysicsActive(this,pEntity); }
	void OnEntityEvent(CEntity *pEntity, SEntityEvent &event) { FOnEntityEvent(this,pEntity,event); }
	virtual IAreaManager *GetAreaManager() const;
	virtual IBreakableManager *GetBreakableManager() const;
	virtual IEntityPoolManager *GetIEntityPoolManager() const;
	virtual void ReloadAllEntityArchetypes();
	virtual void ReloadEntityArchetype(const char *_sArchetype);
	unsigned GenerateEntityId(bool bStaticId) { return FGenerateEntityId(this,bStaticId); }
	bool ResetEntityId(CEntity *pEntity, unsigned newEntityId) { return FResetEntityId(this,pEntity,newEntityId); }
	void RegisterEntityGuid(const uint64_t &guid, unsigned id) { FRegisterEntityGuid(this,guid,id); }
	void UnregisterEntityGuid(const uint64_t &guid) { FUnregisterEntityGuid(this,guid); }
	void ChangeEntityName(CEntity *pEntity, const char *sNewName) { FChangeEntityName(this,pEntity,sNewName); }
	void RemoveEntityEventListeners(CEntity *pEntity) { FRemoveEntityEventListeners(this,pEntity); }
	CEntity *GetEntityFromID(unsigned id) const { return FGetEntityFromID(this,id); }
	virtual void PurgeDeferredCollisionEvents(bool bForce);
	void ComponentRegister(unsigned id, std::shared_ptr<IComponent> pComponent, const int flags) { FComponentRegister(this,id,pComponent,flags); }
	virtual void ComponentEnableEvent(const unsigned id, const int eventID, const bool enable);
	virtual void DebugDraw();
	void DebugDraw(CEntity *ce, float timeMs) { FDebugDrawOv0(this,ce,timeMs); }
	virtual IBSPTree3D *CreateBSPTree3D(DynArray<DynArray<Vec3,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc>>>,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc>>> const &faceList);
	virtual void ReleaseBSPTree3D(IBSPTree3D *&pTree);
	virtual void SetSafeBounds(AABB const &_safeBounds);
	virtual bool IsInSafeBounds(Vec3 const &_pos) const;
	void QueueEntityVisibilityNotify(unsigned _entityId, float _distance) { FQueueEntityVisibilityNotify(this,_entityId,_distance); }
	void DoUpdateLoop(float fFrameTime) { FDoUpdateLoop(this,fFrameTime); }
	void DeleteEntity(CEntity *pEntity) { FDeleteEntity(this,pEntity); }
	void RemoveEntityFromActiveList(CEntity *pEntity) { FRemoveEntityFromActiveList(this,pEntity); }
	void UpdateEngineCVars() { FUpdateEngineCVars(this); }
	void UpdateTimers() { FUpdateTimers(this); }
	void DebugDrawEntityUsage() { FDebugDrawEntityUsage(this); }
	void DebugDrawProximityTriggers() { FDebugDrawProximityTriggers(this); }
	void ClearEntityArray() { FClearEntityArray(this); }
	void DumpEntity(IEntity *pEntity) { FDumpEntity(this,pEntity); }
	void UpdateTempActiveEntities() { FUpdateTempActiveEntities(this); }
	
#if 0
	void RemoveEntityEventListener(std::multimap<unsigned int,IEntityEventListener *,std::less<unsigned int>,std::allocator<std::pair<unsigned int const ,IEntityEventListener *> > > &arg0, std::_Tree_iterator<std::_Tree_val<std::_Tree_simple_types<std::pair<unsigned int const ,IEntityEventListener *> > > > arg1);
	CComponentEventDistributer *GetEventDistributer();
	void ChangeEntityNameRemoveTimerEvent(unsigned arg0);
	CScriptBind_Entity *GetScriptBindEntity();
	CEntityPoolManager *GetEntityPoolManager() const;
	CEntityLoadManager *GetEntityLoadManager() const;
	CGeomCacheAttachmentManager *GetGeomCacheAttachmentManager() const;
	CCharacterBoneAttachmentManager *GetCharacterBoneAttachmentManager() const;
	static uint16_t IdToIndex(const unsigned arg0);
	static CSaltHandle<unsigned short,unsigned short> IdToHandle(const unsigned arg0);
	static unsigned HandleToId(CSaltHandle<unsigned short,unsigned short> arg0);
	CPartitionGrid *GetPartitionGrid();
	CProximityTriggerSystem *GetProximityTriggerSystem();
	bool HasEntity(unsigned arg0) const;
	void ProcessQueuedEntityVisbilityNotifications();
	void DoPrePhysicsUpdate();
	void DoPrePhysicsUpdateFast();
	void UpdateEntity(CEntity *arg0, SEntityUpdateContext &arg1);
	void UpdateDeletedEntities();
	void UpdateNotSeenTimeouts();
	void OnBind(unsigned arg0, unsigned arg1);
	void OnUnbind(unsigned arg0, unsigned arg1);
	void DebugDrawLayerInfo();
	void CheckInternalConsistency() const;
	static IEntityClass *GetClassForEntity(CEntity *arg0);
#endif
	
	static inline auto FBitNotCEntitySystem = PreyFunction<void(CEntitySystem *const _this)>(0x91E880);
	static inline auto FInit = PreyFunction<bool(CEntitySystem *const _this, ISystem *pSystem)>(0x923A90);
	static inline auto FRegisterCharactersForRendering = PreyFunction<void(CEntitySystem *const _this)>(0x9254A0);
	static inline auto FRelease = PreyFunction<void(CEntitySystem *const _this)>(0xA97D40);
	static inline auto FUpdate = PreyFunction<void(CEntitySystem *const _this)>(0x927920);
	static inline auto FDeletePendingEntities = PreyFunction<void(CEntitySystem *const _this)>(0x921770);
	static inline auto FPrePhysicsUpdate = PreyFunction<void(CEntitySystem *const _this)>(0x924FD0);
	static inline auto FReset = PreyFunction<void(CEntitySystem *const _this)>(0x925E70);
	static inline auto FUnload = PreyFunction<void(CEntitySystem *const _this)>(0x9276C0);
	static inline auto FPurgeHeaps = PreyFunction<void(CEntitySystem *const _this)>(0x925250);
	static inline auto FEditorReset = PreyFunction<void(CEntitySystem *const _this, bool _bEnteringGameMode)>(0x922720);
	static inline auto FGetClassRegistry = PreyFunction<IEntityClassRegistry *(CEntitySystem *const _this)>(0x922D60);
	static inline auto FSpawnEntityInternal = PreyFunction<IEntity *(CEntitySystem *const _this, SEntitySpawnParams &params, bool bAutoInit)>(0x927340);
	static inline auto FSpawnEntityFromArchetype = PreyFunction<IEntity *(CEntitySystem *const _this, IEntityArchetype *_pArchetype, SEntitySpawnParams &_params, bool _bAutoInit)>(0x926FB0);
	static inline auto FSpawnEntity = PreyFunction<IEntity *(CEntitySystem *const _this, SEntitySpawnParams &_params, bool _bAutoInit)>(0x926F90);
	static inline auto FInitEntity = PreyFunction<bool(CEntitySystem *const _this, IEntity *pEntity, SEntitySpawnParams &params)>(0x923BC0);
	static inline auto FGetEntity = PreyFunction<IEntity *(CEntitySystem const *const _this, unsigned id)>(0x922DF0);
	static inline auto FArkSafeGetEntity = PreyFunction<IEntity *(CEntitySystem const *const _this, unsigned _id)>(0x91FA50);
	static inline auto FGetClonedEntityId = PreyFunction<unsigned(CEntitySystem const *const _this, unsigned origId, unsigned refId)>(0x922D70);
	static inline auto FFindEntityByName = PreyFunction<IEntity *(CEntitySystem const *const _this, const char *sEntityName)>(0x9229C0);
	static inline auto FReserveEntityId = PreyFunction<void(CEntitySystem *const _this, const unsigned id)>(0x925DA0);
	static inline auto FReserveUnknownEntityId = PreyFunction<unsigned(CEntitySystem *const _this)>(0x925DE0);
	static inline auto FRemoveEntity = PreyFunction<void(CEntitySystem *const _this, unsigned entity, bool bForceRemoveNow)>(0x9255D0);
	static inline auto FGetNumEntities = PreyFunction<unsigned(CEntitySystem const *const _this)>(0x9237C0);
	static inline auto FGetEntityIterator = PreyFunction<IEntityIt *(CEntitySystem *const _this)>(0x922E50);
	static inline auto FSendEventViaEntityEvent = PreyFunction<void(CEntitySystem *const _this, IEntity *piEntity, SEntityEvent &event)>(0x9266B0);
	static inline auto FSendEventToAll = PreyFunction<void(CEntitySystem *const _this, SEntityEvent &event)>(0x926570);
	static inline auto FQueryProximity = PreyFunction<int(CEntitySystem *const _this, SEntityProximityQuery &query)>(0x9252C0);
	static inline auto FResizeProximityGrid = PreyFunction<void(CEntitySystem *const _this, int nWidth, int nHeight)>(0x926540);
	static inline auto FGetPhysicalEntitiesInBox = PreyFunction<int(CEntitySystem const *const _this, Vec3 const &origin, float radius, IPhysicalEntity **&pList, int physFlags)>(0x923800);
	static inline auto FGetEntityFromPhysics = PreyFunction<IEntity *(CEntitySystem const *const _this, IPhysicalEntity *pPhysEntity)>(0x922E30);
	static inline auto FAddSink = PreyFunction<void(CEntitySystem *const _this, IEntitySystemSink *pSink, unsigned subscriptions, uint64_t onEventSubscriptions)>(0x91F8E0);
	static inline auto FRemoveSink = PreyFunction<void(CEntitySystem *const _this, IEntitySystemSink *pSink)>(0x925BA0);
	static inline auto FPauseTimers = PreyFunction<void(CEntitySystem *const _this, bool bPause, bool bResume)>(0x924CE0);
	static inline auto FIsIDUsed = PreyFunction<bool(CEntitySystem const *const _this, unsigned nID)>(0x923DD0);
	static inline auto FGetMemoryStatistics = PreyFunction<void(CEntitySystem const *const _this, ICrySizer *pSizer)>(0x9234A0);
	static inline auto FGetSystem = PreyFunction<ISystem *(CEntitySystem const *const _this)>(0x155ACD0);
	static inline auto FSetNextSpawnId = PreyFunction<void(CEntitySystem *const _this, unsigned id)>(0x926D30);
	static inline auto FResetAreas = PreyFunction<void(CEntitySystem *const _this)>(0x926420);
	static inline auto FUnloadAreas = PreyFunction<void(CEntitySystem *const _this)>(0x9277C0);
	static inline auto FAddEntityEventListener = PreyFunction<void(CEntitySystem *const _this, unsigned nEntity, EEntityEvent event, IEntityEventListener *pListener)>(0x91F5F0);
	static inline auto FRemoveEntityEventListenerOv1 = PreyFunction<void(CEntitySystem *const _this, unsigned nEntity, EEntityEvent event, IEntityEventListener *pListener)>(0x925860);
	static inline auto FHasEntityEventListener = PreyFunction<bool(CEntitySystem const *const _this, IEntityEventListener const &_listener)>(0x9239F0);
	static inline auto FFindEntityByGuid = PreyFunction<unsigned(CEntitySystem const *const _this, const uint64_t &guid)>(0x922930);
	static inline auto FFindEntityByEditorGuid = PreyFunction<unsigned(CEntitySystem const *const _this, const char *pGuid)>(0x922920);
	static inline auto FLoadEntityArchetypeOv1 = PreyFunction<IEntityArchetype *(CEntitySystem *const _this, XmlNodeRef oArchetype)>(0x9241A0);
	static inline auto FLoadEntityArchetypeOv0 = PreyFunction<IEntityArchetype *(CEntitySystem *const _this, const char *sArchetype)>(0x924190);
	static inline auto FFindEntityArchetype = PreyFunction<IEntityArchetype *(CEntitySystem const *const _this, const char *_archetypeName)>(0x922910);
	static inline auto FCreateEntityArchetype = PreyFunction<IEntityArchetype *(CEntitySystem *const _this, IEntityClass *pClass, const char *sArchetype, uint64_t _id)>(0x920080);
	static inline auto FGetEntityArchetypeOv1 = PreyFunction<IEntityArchetype *(CEntitySystem *const _this, uint64_t _id)>(0x922DE0);
	static inline auto FGetEntityArchetypeOv0 = PreyFunction<IEntityArchetype const *(CEntitySystem const *const _this, uint64_t _id)>(0x922DE0);
	static inline auto FGetArchetypeId = PreyFunction<uint64_t(CEntitySystem const *const _this, const char *_archetypeName)>(0x922CC0);
	static inline auto FDeleteEntityArchetype = PreyFunction<void(CEntitySystem *const _this, const char *_archetypeName)>(0x921760);
	static inline auto FRenameArchetype = PreyFunction<void(CEntitySystem const *const _this, const char *_archetypeName, const char *_newArchetypeName)>(0x925D90);
	static inline auto FGetArchetypeNames = PreyFunction<std::vector<string>(CEntitySystem const *const _this, string const &_libraryName)>(0x922D00);
	static inline auto FGetArchetypes = PreyFunction<std::vector<IEntityArchetype *>(CEntitySystem const *const _this, string const &_libraryName)>(0x922D20);
	static inline auto FLoadArchetypeLibrary = PreyFunction<bool(CEntitySystem *const _this, string const &_libraryName)>(0x924070);
	static inline auto FRegisterEntityArchetypeListener = PreyFunction<void(CEntitySystem *const _this, IEntityArchetypeListener *_pListener)>(0x9254B0);
	static inline auto FUnregisterEntityArchetypeListener = PreyFunction<void(CEntitySystem *const _this, IEntityArchetypeListener *_pListener)>(0x9278E0);
	static inline auto FSerialize = PreyFunction<void(CEntitySystem *const _this, TSerialize ser)>(0x9266C0);
	static inline auto FDumpEntities = PreyFunction<void(CEntitySystem *const _this)>(0x9220D0);
	static inline auto FGetLayerId = PreyFunction<int(CEntitySystem const *const _this, const char *szLayerName)>(0x923100);
	static inline auto FGetLayerName = PreyFunction<const char *(CEntitySystem const *const _this, int layerId)>(0x923170);
	static inline auto FGetLayerChildCount = PreyFunction<int(CEntitySystem const *const _this, const char *szLayerName)>(0x922F30);
	static inline auto FGetLayerChild = PreyFunction<const char *(CEntitySystem const *const _this, const char *szLayerName, int childIdx)>(0x922EA0);
	static inline auto FGetVisibleLayerIDs = PreyFunction<int(CEntitySystem const *const _this, uint8_t *pLayerMask, const unsigned maxCount)>(0x923900);
	static inline auto FLockSpawning = PreyFunction<void(CEntitySystem *const _this, bool lock)>(0x924760);
	static inline auto FIsSpawningLocked = PreyFunction<bool(CEntitySystem const *const _this)>(0x923F70);
	static inline auto FOnLoadLevel = PreyFunction<bool(CEntitySystem *const _this, const char *szLevelPath)>(0x924C10);
	static inline auto FOnLevelEnd = PreyFunction<void(CEntitySystem *const _this)>(0x924BB0);
	static inline auto FOnLevelLoadStart = PreyFunction<void(CEntitySystem *const _this)>(0x924C00);
	static inline auto FAddLayer = PreyFunction<CEntityLayer *(CEntitySystem *const _this, const char *name, const char *parent, uint16_t id, bool bHasPhysics, int specs, bool bDefaultLoaded, int _loadTokenId, int _loadTokenValue)>(0x91F6A0);
	static inline auto FGetLayersState = PreyFunction<std::vector<bool>(CEntitySystem const *const _this)>(0x923210);
	static inline auto FLoadLayers = PreyFunction<void(CEntitySystem *const _this, const char *dataFile)>(0x9243B0);
	static inline auto FLinkLayerChildren = PreyFunction<void(CEntitySystem *const _this)>(0x923FA0);
	static inline auto FAddEntityToLayer = PreyFunction<void(CEntitySystem *const _this, const char *layer, unsigned id)>(0x91F670);
	static inline auto FRemoveEntityFromLayers = PreyFunction<void(CEntitySystem *const _this, unsigned id)>(0x925B00);
	static inline auto FGetLayerNameForEntity = PreyFunction<const char *(CEntitySystem *const _this, unsigned id)>(0x9231F0);
	static inline auto FClearLayers = PreyFunction<void(CEntitySystem *const _this)>(0x91FCF0);
	static inline auto FEnableDefaultLayers = PreyFunction<void(CEntitySystem *const _this, bool isSerialized)>(0x922730);
	static inline auto FEnableLayer = PreyFunction<void(CEntitySystem *const _this, const char *layer, bool _bIsVisible, bool isEnable, bool isSerialized)>(0x9227F0);
	static inline auto FIsLayerEnabled = PreyFunction<bool(CEntitySystem const *const _this, const char *layer, bool bMustBeLoaded)>(0x923ED0);
	static inline auto FShouldSerializedEntity = PreyFunction<bool(CEntitySystem *const _this, IEntity *pEntity)>(0x926DF0);
	static inline auto FRegisterPhysicCallbacks = PreyFunction<void(CEntitySystem *const _this)>(0x925530);
	static inline auto FUnregisterPhysicCallbacks = PreyFunction<void(CEntitySystem *const _this)>(0x927900);
	static inline auto FFindLayer = PreyFunction<CEntityLayer *(CEntitySystem *const _this, const char *layer)>(0x922AC0);
	static inline auto FMarkEntityAsUsingPlayerTime = PreyFunction<void(CEntitySystem *const _this, IEntity const &rEntity)>(0x924770);
	static inline auto FUnmarkEntityAsUsingPlayerTime = PreyFunction<void(CEntitySystem *const _this, IEntity const &rEntity)>(0x9277F0);
	static inline auto FShouldEntityUsePlayerTime = PreyFunction<bool(CEntitySystem const *const _this, IEntity const &rEntity)>(0x926DA0);
	static inline auto FGetLayerForEntity = PreyFunction<CEntityLayer *(CEntitySystem *const _this, unsigned id)>(0x922FB0);
	static inline auto FOnBeforeSpawn = PreyFunction<bool(CEntitySystem *const _this, SEntitySpawnParams &params)>(0x924850);
	static inline auto FOnEntityReused = PreyFunction<void(CEntitySystem *const _this, IEntity *pEntity, SEntitySpawnParams &params)>(0x924B00);
	static inline auto FAddTimerEvent = PreyFunction<void(CEntitySystem *const _this, SEntityTimerEvent &event, CTimeValue startTime)>(0x91F990);
	static inline auto FLoadEntitiesOv1 = PreyFunction<void(CEntitySystem *const _this, XmlNodeRef &objectsNode, bool bIsLoadingLevelFile)>(0x924080);
	static inline auto FLoadEntitiesOv0 = PreyFunction<void(CEntitySystem *const _this, XmlNodeRef &objectsNode, bool bIsLoadingLevelFile, Vec3 const &segmentOffset, std::vector<IEntity *> *outGlobalEntityIds, std::vector<IEntity *> *outLocalEntityIds)>(0x924150);
	static inline auto FHoldLayerEntities = PreyFunction<void(CEntitySystem *const _this, const char *pLayerName)>(0x923A80);
	static inline auto FCloneHeldLayerEntities = PreyFunction<void(CEntitySystem *const _this, const char *pLayerName, Vec3 const &localOffset, Matrix34 const &l2w, const char **pExcludeLayers, int numExcludeLayers)>(0x91FDB0);
	static inline auto FReleaseHeldEntities = PreyFunction<void(CEntitySystem *const _this)>(0x9255A0);
	static inline auto FExtractEntityLoadParams = PreyFunction<bool(CEntitySystem const *const _this, XmlNodeRef &entityNode, SEntitySpawnParams &spawnParams)>(0x922900);
	static inline auto FBeginCreateEntities = PreyFunction<void(CEntitySystem *const _this, int nAmtToCreate)>(0x91FAB0);
	static inline auto FCreateEntity = PreyFunction<bool(CEntitySystem *const _this, XmlNodeRef &entityNode, SEntitySpawnParams &pParams, unsigned &outUsingId)>(0x920070);
	static inline auto FEndCreateEntities = PreyFunction<void(CEntitySystem *const _this)>(0x9228F0);
	static inline auto FRemoveTimerEvent = PreyFunction<void(CEntitySystem *const _this, unsigned id, int nTimerId)>(0x925C50);
	static inline auto FActivateEntity = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity, bool bActivate)>(0x91F280);
	static inline auto FActivatePrePhysicsUpdateForEntity = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity, bool bActivate)>(0x91F3E0);
	static inline auto FIsPrePhysicsActive = PreyFunction<bool(CEntitySystem *const _this, CEntity *pEntity)>(0x923F10);
	static inline auto FOnEntityEvent = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity, SEntityEvent &event)>(0x924900);
	static inline auto FGetAreaManager = PreyFunction<IAreaManager *(CEntitySystem const *const _this)>(0x922D40);
	static inline auto FGetBreakableManager = PreyFunction<IBreakableManager *(CEntitySystem const *const _this)>(0x922D50);
	static inline auto FGetIEntityPoolManager = PreyFunction<IEntityPoolManager *(CEntitySystem const *const _this)>(0x922E90);
	static inline auto FReloadAllEntityArchetypes = PreyFunction<void(CEntitySystem *const _this)>(0x9255B0);
	static inline auto FReloadEntityArchetype = PreyFunction<void(CEntitySystem *const _this, const char *_sArchetype)>(0x9255C0);
	static inline auto FGenerateEntityId = PreyFunction<unsigned(CEntitySystem *const _this, bool bStaticId)>(0x922BB0);
	static inline auto FResetEntityId = PreyFunction<bool(CEntitySystem *const _this, CEntity *pEntity, unsigned newEntityId)>(0x926440);
	static inline auto FRegisterEntityGuid = PreyFunction<void(CEntitySystem *const _this, const uint64_t &guid, unsigned id)>(0x9254C0);
	static inline auto FUnregisterEntityGuid = PreyFunction<void(CEntitySystem *const _this, const uint64_t &guid)>(0x9278F0);
	static inline auto FChangeEntityName = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity, const char *sNewName)>(0x91FAC0);
	static inline auto FRemoveEntityEventListeners = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity)>(0x925940);
	static inline auto FGetEntityFromID = PreyFunction<CEntity *(CEntitySystem const *const _this, unsigned id)>(0x922DF0);
	static inline auto FPurgeDeferredCollisionEvents = PreyFunction<void(CEntitySystem *const _this, bool bForce)>(0x925190);
	static inline auto FComponentRegister = PreyFunction<void(CEntitySystem *const _this, unsigned id, std::shared_ptr<IComponent> pComponent, const int flags)>(0x91FF90);
	static inline auto FComponentEnableEvent = PreyFunction<void(CEntitySystem *const _this, const unsigned id, const int eventID, const bool enable)>(0x91FF80);
	static inline auto FDebugDrawOv1 = PreyFunction<void(CEntitySystem *const _this)>(0x9206F0);
	static inline auto FDebugDrawOv0 = PreyFunction<void(CEntitySystem *const _this, CEntity *ce, float timeMs)>(0x920090);
	static inline auto FCreateBSPTree3D = PreyFunction<IBSPTree3D *(CEntitySystem *const _this, DynArray<DynArray<Vec3,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc>>>,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc>>> const &faceList)>(0x920040);
	static inline auto FReleaseBSPTree3D = PreyFunction<void(CEntitySystem *const _this, IBSPTree3D *&pTree)>(0x925570);
	static inline auto FSetSafeBounds = PreyFunction<void(CEntitySystem *const _this, AABB const &_safeBounds)>(0x926D80);
	static inline auto FIsInSafeBounds = PreyFunction<bool(CEntitySystem const *const _this, Vec3 const &_pos)>(0x923DF0);
	static inline auto FQueueEntityVisibilityNotify = PreyFunction<void(CEntitySystem *const _this, unsigned _entityId, float _distance)>(0x925340);
	static inline auto FDoUpdateLoop = PreyFunction<void(CEntitySystem *const _this, float fFrameTime)>(0x9217D0);
	static inline auto FDeleteEntity = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity)>(0x9215C0);
	static inline auto FRemoveEntityFromActiveList = PreyFunction<void(CEntitySystem *const _this, CEntity *pEntity)>(0x925A60);
	static inline auto FUpdateEngineCVars = PreyFunction<void(CEntitySystem *const _this)>(0x927E00);
	static inline auto FUpdateTimers = PreyFunction<void(CEntitySystem *const _this)>(0x9281C0);
	static inline auto FDebugDrawEntityUsage = PreyFunction<void(CEntitySystem *const _this)>(0x920700);
	static inline auto FDebugDrawProximityTriggers = PreyFunction<void(CEntitySystem *const _this)>(0x9212F0);
	static inline auto FClearEntityArray = PreyFunction<void(CEntitySystem *const _this)>(0x91FC90);
	static inline auto FDumpEntity = PreyFunction<void(CEntitySystem *const _this, IEntity *pEntity)>(0x9221B0);
	static inline auto FUpdateTempActiveEntities = PreyFunction<void(CEntitySystem *const _this)>(0x9280C0);
};

// Header: Exact
// CryEngine/cryentitysystem/entitysystem.h
struct SEntityLoadParams // Id=800C790 Size=160
{
	SEntitySpawnParams spawnParams;
	CEntity *pReuseEntity;
	bool bCallInit;
	int clonedLayerId;
	
	SEntityLoadParams();
	SEntityLoadParams(SEntityLoadParams const &other);
	~SEntityLoadParams() { FBitNotSEntityLoadParams(this); }
	SEntityLoadParams &operator=(SEntityLoadParams const &other) { return FoperatorEq(this,other); }
	void UseClonedEntityNode(XmlNodeRef sourceEntityNode, XmlNodeRef parentNode) { FUseClonedEntityNode(this,sourceEntityNode,parentNode); }
	static bool CloneXmlNode(XmlNodeRef sourceNode, XmlNodeRef destNode) { return FCloneXmlNode(sourceNode,destNode); }
	
#if 0
	void AddRef();
	void RemoveRef();
#endif
	
	static inline auto FBitNotSEntityLoadParams = PreyFunction<void(SEntityLoadParams *const _this)>(0x91EFF0);
	static inline auto FoperatorEq = PreyFunction<SEntityLoadParams &(SEntityLoadParams *const _this, SEntityLoadParams const &other)>(0x91F050);
	static inline auto FUseClonedEntityNode = PreyFunction<void(SEntityLoadParams *const _this, XmlNodeRef sourceEntityNode, XmlNodeRef parentNode)>(0x928410);
	static inline auto FCloneXmlNode = PreyFunction<bool(XmlNodeRef sourceNode, XmlNodeRef destNode)>(0x91FDC0);
};

