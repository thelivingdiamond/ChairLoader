// Header file automatically created from a PDB.

#pragma once
#include <Prey/CryEntitySystem/Entity.h>
#include <Prey/CryEntitySystem/IEntity.h>
#include <Prey/CryNetwork/ISerialize.h>

struct AABB;
struct AIObjectParams;
struct AnimEventInstance;
class CDLight;
class CEntityObject;
class CEntitySystem;
class CPhysicalProxy;
class CRenderProxy;
class CScriptProxy;
class CTimeValue;
struct IAIObject;
struct IArkGlass;
struct ICharacterInstance;
class ICrySizer;
struct IEntityArchetype;
struct IEntityClass;
struct IEntityLink;
struct IEntityProxy;
struct IEntitySystem;
struct IGeomCacheRenderNode;
struct IMaterial;
struct IParticleEffect;
struct IParticleEmitter;
struct IPhysicalEntity;
struct IStatObj;
struct SChildAttachParams;
struct SEntityEvent;
struct SEntityLoadParams;
struct SEntityPhysicalizeParams;
struct SEntitySlotInfo;
struct SEntitySpawnParams;
struct SEntityUpdateContext;
struct SFogVolumeProperties;
struct SGeometryDebugDrawInfo;
struct SGridLocation;
struct SProximityElement;
struct SVolumeObjectMovementProperties;
struct SpawnParams;
class XmlNodeRef;

// Header: Exact
// CryEngine/cryentitysystem/entity.h
class CEntity : public IEntity // Id=800C730 Size=272
{
public:
	using TProxyPair = std::pair<int,std::shared_ptr<IEntityProxy> >;
	
	struct SBindings // Id=800C79F Size=40
	{
		enum EBindingType
		{
			eBT_Pivot = 0,
			eBT_GeomCacheNode = 1,
			eBT_CharacterBone = 2,
		};

		std::vector<CEntity *> childs;
		CEntity *pParent;
		CEntity::SBindings::EBindingType parentBindingType;
	};

	unsigned m_bActive : 1;
	unsigned m_bInActiveList : 1;
	unsigned m_bBoundsValid : 1;
	unsigned m_bInitialized : 1;
	unsigned m_bHidden : 1;
	unsigned m_bGarbage : 1;
	unsigned m_bHaveEventListeners : 1;
	unsigned m_bTrigger : 1;
	unsigned m_bWasRellocated : 1;
	unsigned m_nUpdateCounter : 4;
	unsigned m_eUpdatePolicy : 4;
	unsigned m_bInvisible : 1;
	unsigned m_bNotInheritXform : 1;
	unsigned m_bInShutDown : 1;
	unsigned m_bIsFromPool : 1;
	bool m_bDirtyForwardDir : 1;
	unsigned m_bLoadedFromLevelFile : 1;
	unsigned m_bHasPhysicsState : 1;
	unsigned m_nID;
	uint64_t m_guid;
	unsigned m_flags;
	unsigned m_flagsExtended;
	static string m_szDescription;
	IEntityClass *m_pClass;
	IEntityArchetype *m_pArchetype;
	Vec3 m_vPos;
	Quat m_qRotation;
	Vec3 m_vScale;
	Matrix34 m_worldTM;
	Vec3 m_vForwardDir;
	CEntity::SBindings *m_pBinds;
	unsigned m_aiObjectID;
	_smart_ptr<IMaterial> m_pMaterial;
	
	using TProxyContainer = std::map<int,std::shared_ptr<IEntityProxy>,std::less<unsigned int>>;
	
	std::map<int,std::shared_ptr<IEntityProxy>,std::less<unsigned int>> m_proxy;
	
	using TComponents = std::set<std::shared_ptr<IComponent>,std::less<std::shared_ptr<IComponent>>>;
	
	std::set<std::shared_ptr<IComponent>,std::less<std::shared_ptr<IComponent>>> m_components;
	IEntityLink *m_pEntityLinks;
	SGridLocation *m_pGridLocation;
	SProximityElement *m_pProximityEntity;
	unsigned m_nKeepAliveCounter;
	string m_szName;
	int m_cloneLayerId;
	uint8_t m_initialSceneMask;
	string m_displayName;
	
	CEntity(SEntitySpawnParams &params);
	virtual ~CEntity();
	bool Init(SEntitySpawnParams &params) { return FInit(this,params); }
	void PrePhysicsUpdate(float fFrameTime) { FPrePhysicsUpdate(this,fFrameTime); }
	void Update(SEntityUpdateContext &ctx) { FUpdate(this,ctx); }
	void ShutDown(bool bRemoveAI, bool bRemoveProxies) { FShutDown(this,bRemoveAI,bRemoveProxies); }
	virtual unsigned GetId() const;
	virtual uint64_t GetGuid() const;
	virtual IEntityClass *GetClass() const;
	virtual IEntityArchetype *GetArchetype();
	virtual IEntityArchetype const *GetArchetype() const;
	virtual void SetFlags(unsigned flags);
	virtual unsigned GetFlags() const;
	virtual void AddFlags(unsigned flagsToAdd);
	virtual void ClearFlags(unsigned flagsToClear);
	virtual bool CheckFlags(unsigned flagsToCheck) const;
	virtual void SetFlagsExtended(unsigned _flags);
	virtual unsigned GetFlagsExtended() const;
	virtual bool IsGarbage() const;
	virtual bool IsLoadedFromLevelFile() const;
	virtual void SetName(const char *sName);
	virtual const char *GetName() const;
	virtual string GetEntityTextDescription() const;
	virtual void AttachChild(IEntity *pChildEntity, SChildAttachParams const &attachParams);
	virtual void DetachAll(int nDetachFlags);
	virtual void DetachThis(int nDetachFlags, int nWhyFlags);
	virtual int GetChildCount() const;
	virtual IEntity *GetChild(int nIndex) const;
	virtual void EnableInheritXForm(bool bEnable);
	virtual IEntity *GetParent() const;
	virtual Matrix34 GetParentAttachPointWorldTM() const;
	virtual bool IsParentAttachmentValid() const;
	virtual IEntity *GetAdam();
	virtual void SetWorldTM(Matrix34 const &tm, int nWhyFlags);
	virtual void SetLocalTM(Matrix34 const &tm, int nWhyFlags);
	virtual void SetPosRotScale(Vec3 const &vPos, Quat const &qRotation, Vec3 const &vScale, int nWhyFlags);
	virtual Matrix34 GetLocalTM() const;
	virtual Matrix34 const &GetWorldTM() const;
	virtual void GetWorldBounds(AABB &bbox) const;
	virtual void GetLocalBounds(AABB &bbox) const;
	virtual void SetPos(Vec3 const &vPos, int nWhyFlags, bool bRecalcPhyBounds, bool bForce);
	virtual Vec3 const &GetPos() const;
	virtual void SetRotation(Quat const &qRotation, int nWhyFlags);
	virtual Quat const &GetRotation() const;
	virtual void SetScale(Vec3 const &vScale, int nWhyFlags);
	virtual Vec3 const &GetScale() const;
	virtual Vec3 GetWorldPos() const;
	virtual Ang3 GetWorldAngles() const;
	virtual Quat GetWorldRotation() const;
	virtual Vec3 const &GetForwardDir() const;
	virtual void Activate(bool bActive);
	virtual bool IsActive() const;
	virtual bool IsFromPool() const;
	virtual void PrePhysicsActivate(bool bActive);
	virtual bool IsPrePhysicsActive();
	virtual void Serialize(TSerialize ser, int nFlags);
	virtual bool SendEvent(SEntityEvent &event);
	virtual void SetTimer(int nTimerId, int nMilliSeconds);
	virtual void KillTimer(int nTimerId);
	virtual void Hide(bool bHide);
	virtual bool IsHidden() const;
	virtual void Invisible(bool bInvisible);
	virtual bool IsInvisible() const;
	virtual IAIObject *GetAI();
	virtual bool HasAI() const;
	virtual unsigned GetAIObjectID() const;
	virtual void SetAIObjectID(unsigned id);
	virtual bool RegisterInAISystem(AIObjectParams const &params);
	void UpdateAIObject() { FUpdateAIObject(this); }
	virtual void SetUpdatePolicy(EEntityUpdatePolicy eUpdatePolicy);
	virtual EEntityUpdatePolicy GetUpdatePolicy() const;
	virtual IEntityProxy *GetProxy(EEntityProxy proxy) const;
	virtual void SetProxy(EEntityProxy proxy, std::shared_ptr<IEntityProxy> pProxy);
	virtual std::shared_ptr<IEntityProxy> CreateProxy(EEntityProxy proxy);
	virtual void RegisterComponent(std::shared_ptr<IComponent> pComponentPtr, const int flagsOriginal);
	virtual void Physicalize(SEntityPhysicalizeParams &params);
	virtual void EnablePhysics(bool enable);
	virtual IPhysicalEntity *GetPhysics() const;
	virtual int PhysicalizeSlot(int slot, SEntityPhysicalizeParams &params);
	virtual void UnphysicalizeSlot(int slot);
	virtual void UpdateSlotPhysics(int slot);
	virtual void SetPhysicsState(XmlNodeRef &physicsState);
	virtual bool HasPhysicsState() const;
	virtual void SetMaterial(IMaterial *pMaterial);
	virtual IMaterial *GetMaterial();
	virtual bool IsSlotValid(int nSlot) const;
	virtual void FreeSlot(int nSlot);
	virtual int GetSlotCount() const;
	virtual bool GetSlotInfo(int nSlot, SEntitySlotInfo &slotInfo) const;
	virtual Matrix34 const &GetSlotWorldTM(int nSlot) const;
	virtual Matrix34 const &GetSlotLocalTM(int nSlot, bool bRelativeToParent) const;
	virtual void SetSlotLocalTM(int nSlot, Matrix34 const &localTM, int nWhyFlags);
	virtual void SetSlotCameraSpacePos(int nSlot, Vec3 const &cameraSpacePos);
	virtual void GetSlotCameraSpacePos(int nSlot, Vec3 &cameraSpacePos) const;
	virtual bool SetParentSlot(int nParentSlot, int nChildSlot);
	virtual void SetSlotMaterial(int nSlot, IMaterial *pMaterial);
	virtual void SetSlotFlags(int nSlot, unsigned nFlags);
	virtual unsigned GetSlotFlags(int nSlot) const;
	virtual bool ShouldUpdateCharacter(int nSlot) const;
	virtual ICharacterInstance *GetCharacter(int nSlot);
	virtual int SetCharacter(ICharacterInstance *pCharacter, int nSlot);
	virtual IStatObj *GetStatObj(int nSlot);
	virtual int SetStatObj(IStatObj *pStatObj, int nSlot, bool bUpdatePhysics, float mass);
	virtual IParticleEmitter *GetParticleEmitter(int nSlot);
	virtual IGeomCacheRenderNode *GetGeomCacheRenderNode(int nSlot);
	virtual void MoveSlot(IEntity *targetIEnt, int nSlot);
	virtual int LoadGeometry(int nSlot, const char *sFilename, const char *sGeomName, int nLoadFlags);
	virtual int LoadCharacter(int nSlot, const char *sFilename, int nLoadFlags);
	virtual int LoadGeomCache(int nSlot, const char *sFilename);
	virtual int SetParticleEmitter(int nSlot, IParticleEmitter *pEmitter, bool bSerialize);
	virtual int LoadParticleEmitter(int nSlot, IParticleEffect *pEffect, SpawnParams const *params, bool bPrime, bool bSerialize);
	virtual int SetBreakableGlass(int _nSlot, IArkGlass *_pGlass);
	virtual IArkGlass *GetBreakableGlass(int _nSlot);
	virtual int LoadLight(int nSlot, CDLight *pLight);
	int LoadLightImpl(int nSlot, CDLight *pLight) { return FLoadLightImpl(this,nSlot,pLight); }
	virtual bool UpdateLightClipBounds(CDLight &light);
	int LoadFogVolume(int nSlot, SFogVolumeProperties const &properties) { return FLoadFogVolume(this,nSlot,properties); }
	int FadeGlobalDensity(int nSlot, float fadeTime, float newGlobalDensity) { return FFadeGlobalDensity(this,nSlot,fadeTime,newGlobalDensity); }
	int LoadVolumeObject(int nSlot, const char *sFilename) { return FLoadVolumeObject(this,nSlot,sFilename); }
	int SetVolumeObjectMovementProperties(int nSlot, SVolumeObjectMovementProperties const &properties) { return FSetVolumeObjectMovementProperties(this,nSlot,properties); }
	virtual void InvalidateTM(int nWhyFlags, bool bRecalcPhyBounds);
	virtual void SerializeXML(XmlNodeRef &node, bool bLoading);
	virtual IEntityLink *GetEntityLinks();
	virtual IEntityLink *AddEntityLink(const char *sLinkName, unsigned entityId);
	virtual void RemoveEntityLink(IEntityLink *pLink);
	virtual void RemoveAllEntityLinks();
	virtual void SetSlotAlpha(int _nSlot, float _alpha);
	virtual IEntity *UnmapAttachedChild(int &partId);
	virtual void GetMemoryUsage(ICrySizer *pSizer) const;
	virtual bool IsInitialized() const;
	virtual void DebugDraw(SGeometryDebugDrawInfo const &info);
	CEntityObject *GetSlot(int nSlot) const { return FGetSlot(this,nSlot); }
	bool GetSignature(TSerialize &signature) { return FGetSignature(this,signature); }
	void SerializeXML_ExceptScriptProxy(XmlNodeRef &node, bool bLoading) { FSerializeXML_ExceptScriptProxy(this,node,bLoading); }
	bool ReloadEntity(SEntityLoadParams &loadParams) { return FReloadEntity(this,loadParams); }
	void SetPoolControl(bool bSet) { FSetPoolControl(this,bSet); }
	void ActivateForNumUpdates(int numUpdates) { FActivateForNumUpdates(this,numUpdates); }
	void SetUpdateStatus() { FSetUpdateStatus(this); }
	void CalcLocalTM(Matrix34 &tm) const { FCalcLocalTM(this,tm); }
	virtual void IncKeepAliveCounter();
	virtual void DecKeepAliveCounter();
	virtual void ResetKeepAliveCounter();
	virtual bool IsKeptAlive() const;
	void CheckMaterialFlags() { FCheckMaterialFlags(this); }
	virtual const char *GetDisplayName() const;
	virtual void SetDisplayName(const char *_pName);
	uint8_t GetSceneMask() const { return FGetSceneMask(this); }
	void AllocBindings() { FAllocBindings(this); }
	void DeallocBindings() { FDeallocBindings(this); }
	void OnRellocate(int nWhyFlags) { FOnRellocate(this,nWhyFlags); }
	void AnimationEvent(ICharacterInstance *pCharacter, AnimEventInstance const &event) { FAnimationEvent(this,pCharacter,event); }
	
#if 0
	IEntitySystem *GetEntitySystem() const;
	void SetLoadedFromLevelFile(const bool arg0);
	void Hide(bool arg0, EEntityEvent arg1, EEntityEvent arg2);
	CRenderProxy *GetRenderProxy() const;
	CPhysicalProxy *GetPhysicalProxy() const;
	CScriptProxy *GetScriptProxy() const;
	CEntitySystem *GetCEntitySystem() const;
	bool IsPoolControlled() const;
	bool GetUpdateStatus() const;
	Matrix34 const &GetWorldTM_Fast() const;
	SProximityElement *GetProximityElement();
	static void ClearStaticData();
	void SetCloneLayerId(int arg0);
	int GetCloneLayerId() const;
	bool IsOnlyInOtherScene() const;
	void LogEvent(SEntityEvent &arg0, CTimeValue arg1);
	void ComputeForwardDir() const;
	bool IsScaled(float arg0) const;
	IAIObject *GetAIObject();
#endif
	
	static inline auto FInit = PreyFunction<bool(CEntity *const _this, SEntitySpawnParams &params)>(0x905CB0);
	static inline auto FPrePhysicsUpdate = PreyFunction<void(CEntity *const _this, float fFrameTime)>(0x907FB0);
	static inline auto FUpdate = PreyFunction<void(CEntity *const _this, SEntityUpdateContext &ctx)>(0x90CE70);
	static inline auto FShutDown = PreyFunction<void(CEntity *const _this, bool bRemoveAI, bool bRemoveProxies)>(0x90CA70);
	static inline auto FGetId = PreyFunction<unsigned(CEntity const *const _this)>(0xD9C610);
	static inline auto FGetGuid = PreyFunction<uint64_t(CEntity const *const _this)>(0xD87500);
	static inline auto FGetClass = PreyFunction<IEntityClass *(CEntity const *const _this)>(0xD87510);
	static inline auto FGetArchetypeOv1 = PreyFunction<IEntityArchetype *(CEntity *const _this)>(0x862100);
	static inline auto FGetArchetypeOv0 = PreyFunction<IEntityArchetype const *(CEntity const *const _this)>(0x862100);
	static inline auto FSetFlags = PreyFunction<void(CEntity *const _this, unsigned flags)>(0x90AF50);
	static inline auto FGetFlags = PreyFunction<unsigned(CEntity const *const _this)>(0x12380B0);
	static inline auto FAddFlags = PreyFunction<void(CEntity *const _this, unsigned flagsToAdd)>(0x902BF0);
	static inline auto FClearFlags = PreyFunction<void(CEntity *const _this, unsigned flagsToClear)>(0x903530);
	static inline auto FCheckFlags = PreyFunction<bool(CEntity const *const _this, unsigned flagsToCheck)>(0x903270);
	static inline auto FSetFlagsExtended = PreyFunction<void(CEntity *const _this, unsigned _flags)>(0x90AFE0);
	static inline auto FGetFlagsExtended = PreyFunction<unsigned(CEntity const *const _this)>(0xEB9170);
	static inline auto FIsGarbage = PreyFunction<bool(CEntity const *const _this)>(0x906500);
	static inline auto FIsLoadedFromLevelFile = PreyFunction<bool(CEntity const *const _this)>(0x906550);
	static inline auto FSetName = PreyFunction<void(CEntity *const _this, const char *sName)>(0x90B410);
	static inline auto FGetName = PreyFunction<const char *(CEntity const *const _this)>(0x36CFD0);
	static inline auto FGetEntityTextDescription = PreyFunction<const char *(CEntity const *const _this)>(0x36CFD0);
	static inline auto FAttachChild = PreyFunction<void(CEntity *const _this, IEntity *pChildEntity, SChildAttachParams const &attachParams)>(0x902E20);
	static inline auto FDetachAll = PreyFunction<void(CEntity *const _this, int nDetachFlags)>(0x903FF0);
	static inline auto FDetachThis = PreyFunction<void(CEntity *const _this, int nDetachFlags, int nWhyFlags)>(0x904050);
	static inline auto FGetChildCount = PreyFunction<int(CEntity const *const _this)>(0x904680);
	static inline auto FGetChild = PreyFunction<IEntity *(CEntity const *const _this, int nIndex)>(0x904650);
	static inline auto FEnableInheritXForm = PreyFunction<void(CEntity *const _this, bool bEnable)>(0x904290);
	static inline auto FGetParent = PreyFunction<IEntity *(CEntity const *const _this)>(0x904B30);
	static inline auto FGetParentAttachPointWorldTM = PreyFunction<Matrix34(CEntity const *const _this)>(0x904B50);
	static inline auto FIsParentAttachmentValid = PreyFunction<bool(CEntity const *const _this)>(0x906720);
	static inline auto FGetAdam = PreyFunction<IEntity *(CEntity *const _this)>(0x904550);
	static inline auto FSetWorldTM = PreyFunction<void(CEntity *const _this, Matrix34 const &tm, int nWhyFlags)>(0x90C400);
	static inline auto FSetLocalTM = PreyFunction<void(CEntity *const _this, Matrix34 const &tm, int nWhyFlags)>(0x90B060);
	static inline auto FSetPosRotScale = PreyFunction<void(CEntity *const _this, Vec3 const &vPos, Quat const &qRotation, Vec3 const &vScale, int nWhyFlags)>(0x90B980);
	static inline auto FGetLocalTM = PreyFunction<Matrix34(CEntity const *const _this)>(0x9049F0);
	static inline auto FGetWorldTM = PreyFunction<Matrix34 const &(CEntity const *const _this)>(0x10BD9E0);
	static inline auto FGetWorldBounds = PreyFunction<void(CEntity const *const _this, AABB &bbox)>(0x9053C0);
	static inline auto FGetLocalBounds = PreyFunction<void(CEntity const *const _this, AABB &bbox)>(0x904840);
	static inline auto FSetPos = PreyFunction<void(CEntity *const _this, Vec3 const &vPos, int nWhyFlags, bool bRecalcPhyBounds, bool bForce)>(0x90B930);
	static inline auto FGetPos = PreyFunction<Vec3 const &(CEntity const *const _this)>(0x10BDA20);
	static inline auto FSetRotation = PreyFunction<void(CEntity *const _this, Quat const &qRotation, int nWhyFlags)>(0x90BC10);
	static inline auto FGetRotation = PreyFunction<Quat const &(CEntity const *const _this)>(0x10BD7E0);
	static inline auto FSetScale = PreyFunction<void(CEntity *const _this, Vec3 const &vScale, int nWhyFlags)>(0x90BC70);
	static inline auto FGetScale = PreyFunction<Vec3 const &(CEntity const *const _this)>(0x10BDA40);
	static inline auto FGetWorldPos = PreyFunction<Vec3(CEntity const *const _this)>(0x905700);
	static inline auto FGetWorldAngles = PreyFunction<Ang3(CEntity const *const _this)>(0x905250);
	static inline auto FGetWorldRotation = PreyFunction<Quat(CEntity const *const _this)>(0x905720);
	static inline auto FGetForwardDir = PreyFunction<Vec3 const &(CEntity const *const _this)>(0x9046B0);
	static inline auto FActivate = PreyFunction<void(CEntity *const _this, bool bActive)>(0x902A40);
	static inline auto FIsActive = PreyFunction<bool(CEntity const *const _this)>(0x9064E0);
	static inline auto FIsFromPool = PreyFunction<bool(CEntity const *const _this)>(0x9064F0);
	static inline auto FPrePhysicsActivate = PreyFunction<void(CEntity *const _this, bool bActive)>(0x907F90);
	static inline auto FIsPrePhysicsActive = PreyFunction<bool(CEntity *const _this)>(0x906780);
	static inline auto FSerialize = PreyFunction<void(CEntity *const _this, TSerialize ser, int nFlags)>(0x909440);
	static inline auto FSendEvent = PreyFunction<bool(CEntity *const _this, SEntityEvent &event)>(0x908DF0);
	static inline auto FSetTimer = PreyFunction<void(CEntity *const _this, int nTimerId, int nMilliSeconds)>(0x90C190);
	static inline auto FKillTimer = PreyFunction<void(CEntity *const _this, int nTimerId)>(0x906820);
	static inline auto FHideOv1 = PreyFunction<void(CEntity *const _this, bool bHide)>(0x905B90);
	static inline auto FIsHidden = PreyFunction<bool(CEntity const *const _this)>(0x906510);
	static inline auto FInvisible = PreyFunction<void(CEntity *const _this, bool bInvisible)>(0x9063E0);
	static inline auto FIsInvisible = PreyFunction<bool(CEntity const *const _this)>(0x906530);
	static inline auto FGetAI = PreyFunction<IAIObject *(CEntity *const _this)>(0x904510);
	static inline auto FHasAI = PreyFunction<bool(CEntity const *const _this)>(0x905B70);
	static inline auto FGetAIObjectID = PreyFunction<unsigned(CEntity const *const _this)>(0x1873970);
	static inline auto FSetAIObjectID = PreyFunction<void(CEntity *const _this, unsigned id)>(0x90AC10);
	static inline auto FRegisterInAISystem = PreyFunction<bool(CEntity *const _this, AIObjectParams const &params)>(0x9082F0);
	static inline auto FUpdateAIObject = PreyFunction<void(CEntity *const _this)>(0x90D030);
	static inline auto FSetUpdatePolicy = PreyFunction<void(CEntity *const _this, EEntityUpdatePolicy eUpdatePolicy)>(0x90C200);
	static inline auto FGetUpdatePolicy = PreyFunction<EEntityUpdatePolicy(CEntity const *const _this)>(0x905240);
	static inline auto FGetProxy = PreyFunction<IEntityProxy *(CEntity const *const _this, EEntityProxy proxy)>(0x904D50);
	static inline auto FSetProxy = PreyFunction<void(CEntity *const _this, EEntityProxy proxy, std::shared_ptr<IEntityProxy> pProxy)>(0x90BAD0);
	static inline auto FCreateProxy = PreyFunction<std::shared_ptr<IEntityProxy>(CEntity *const _this, EEntityProxy proxy)>(0x903540);
	static inline auto FRegisterComponent = PreyFunction<void(CEntity *const _this, std::shared_ptr<IComponent> pComponentPtr, const int flagsOriginal)>(0x908190);
	static inline auto FPhysicalize = PreyFunction<void(CEntity *const _this, SEntityPhysicalizeParams &params)>(0x907E00);
	static inline auto FEnablePhysics = PreyFunction<void(CEntity *const _this, bool enable)>(0x9042B0);
	static inline auto FGetPhysics = PreyFunction<IPhysicalEntity *(CEntity const *const _this)>(0x904CB0);
	static inline auto FPhysicalizeSlot = PreyFunction<int(CEntity *const _this, int slot, SEntityPhysicalizeParams &params)>(0x907F30);
	static inline auto FUnphysicalizeSlot = PreyFunction<void(CEntity *const _this, int slot)>(0x90CE20);
	static inline auto FUpdateSlotPhysics = PreyFunction<void(CEntity *const _this, int slot)>(0x90D1D0);
	static inline auto FSetPhysicsState = PreyFunction<void(CEntity *const _this, XmlNodeRef &physicsState)>(0x90B650);
	static inline auto FHasPhysicsState = PreyFunction<bool(CEntity const *const _this)>(0x905B80);
	static inline auto FSetMaterial = PreyFunction<void(CEntity *const _this, IMaterial *pMaterial)>(0x90B380);
	static inline auto FGetMaterial = PreyFunction<IMaterial *(CEntity *const _this)>(0x11A67A0);
	static inline auto FIsSlotValid = PreyFunction<bool(CEntity const *const _this, int nSlot)>(0x906790);
	static inline auto FFreeSlot = PreyFunction<void(CEntity *const _this, int nSlot)>(0x9044B0);
	static inline auto FGetSlotCount = PreyFunction<int(CEntity const *const _this)>(0x905000);
	static inline auto FGetSlotInfo = PreyFunction<bool(CEntity const *const _this, int nSlot, SEntitySlotInfo &slotInfo)>(0x9050C0);
	static inline auto FGetSlotWorldTM = PreyFunction<Matrix34 const &(CEntity const *const _this, int nSlot)>(0x905180);
	static inline auto FGetSlotLocalTM = PreyFunction<Matrix34 const &(CEntity const *const _this, int nSlot, bool bRelativeToParent)>(0x905120);
	static inline auto FSetSlotLocalTM = PreyFunction<void(CEntity *const _this, int nSlot, Matrix34 const &localTM, int nWhyFlags)>(0x90BED0);
	static inline auto FSetSlotCameraSpacePos = PreyFunction<void(CEntity *const _this, int nSlot, Vec3 const &cameraSpacePos)>(0x90BE10);
	static inline auto FGetSlotCameraSpacePos = PreyFunction<void(CEntity const *const _this, int nSlot, Vec3 &cameraSpacePos)>(0x904F70);
	static inline auto FSetParentSlot = PreyFunction<bool(CEntity *const _this, int nParentSlot, int nChildSlot)>(0x90B4A0);
	static inline auto FSetSlotMaterial = PreyFunction<void(CEntity *const _this, int nSlot, IMaterial *pMaterial)>(0x90BF30);
	static inline auto FSetSlotFlags = PreyFunction<void(CEntity *const _this, int nSlot, unsigned nFlags)>(0x90BE70);
	static inline auto FGetSlotFlags = PreyFunction<unsigned(CEntity const *const _this, int nSlot)>(0x905060);
	static inline auto FShouldUpdateCharacter = PreyFunction<bool(CEntity const *const _this, int nSlot)>(0xDD23F0);
	static inline auto FGetCharacter = PreyFunction<ICharacterInstance *(CEntity *const _this, int nSlot)>(0x9045F0);
	static inline auto FSetCharacter = PreyFunction<int(CEntity *const _this, ICharacterInstance *pCharacter, int nSlot)>(0x90AD50);
	static inline auto FGetStatObj = PreyFunction<IStatObj *(CEntity *const _this, int nSlot)>(0x9051E0);
	static inline auto FSetStatObj = PreyFunction<int(CEntity *const _this, IStatObj *pStatObj, int nSlot, bool bUpdatePhysics, float mass)>(0x90BFA0);
	static inline auto FGetParticleEmitter = PreyFunction<IParticleEmitter *(CEntity *const _this, int nSlot)>(0x904C50);
	static inline auto FGetGeomCacheRenderNode = PreyFunction<IGeomCacheRenderNode *(CEntity *const _this, int nSlot)>(0x9047E0);
	static inline auto FMoveSlot = PreyFunction<void(CEntity *const _this, IEntity *targetIEnt, int nSlot)>(0x9075E0);
	static inline auto FLoadGeometry = PreyFunction<int(CEntity *const _this, int nSlot, const char *sFilename, const char *sGeomName, int nLoadFlags)>(0x906DE0);
	static inline auto FLoadCharacter = PreyFunction<int(CEntity *const _this, int nSlot, const char *sFilename, int nLoadFlags)>(0x906840);
	static inline auto FLoadGeomCache = PreyFunction<int(CEntity *const _this, int nSlot, const char *sFilename)>(0x906CA0);
	static inline auto FSetParticleEmitter = PreyFunction<int(CEntity *const _this, int nSlot, IParticleEmitter *pEmitter, bool bSerialize)>(0x90B500);
	static inline auto FLoadParticleEmitter = PreyFunction<int(CEntity *const _this, int nSlot, IParticleEffect *pEffect, SpawnParams const *params, bool bPrime, bool bSerialize)>(0x907350);
	static inline auto FSetBreakableGlass = PreyFunction<int(CEntity *const _this, int _nSlot, IArkGlass *_pGlass)>(0x90AC20);
	static inline auto FGetBreakableGlass = PreyFunction<IArkGlass *(CEntity *const _this, int _nSlot)>(0x904590);
	static inline auto FLoadLight = PreyFunction<int(CEntity *const _this, int nSlot, CDLight *pLight)>(0x907090);
	static inline auto FLoadLightImpl = PreyFunction<int(CEntity *const _this, int nSlot, CDLight *pLight)>(0x9071F0);
	static inline auto FUpdateLightClipBounds = PreyFunction<bool(CEntity *const _this, CDLight &light)>(0x90D0D0);
	static inline auto FLoadFogVolume = PreyFunction<int(CEntity *const _this, int nSlot, SFogVolumeProperties const &properties)>(0x906B70);
	static inline auto FFadeGlobalDensity = PreyFunction<int(CEntity *const _this, int nSlot, float fadeTime, float newGlobalDensity)>(0x904360);
	static inline auto FLoadVolumeObject = PreyFunction<int(CEntity *const _this, int nSlot, const char *sFilename)>(0x9074B0);
	static inline auto FSetVolumeObjectMovementProperties = PreyFunction<int(CEntity *const _this, int nSlot, SVolumeObjectMovementProperties const &properties)>(0x90C2D0);
	static inline auto FInvalidateTM = PreyFunction<void(CEntity *const _this, int nWhyFlags, bool bRecalcPhyBounds)>(0x905E70);
	static inline auto FSerializeXML = PreyFunction<void(CEntity *const _this, XmlNodeRef &node, bool bLoading)>(0x90A9C0);
	static inline auto FGetEntityLinks = PreyFunction<IEntityLink *(CEntity *const _this)>(0x155ACD0);
	static inline auto FAddEntityLink = PreyFunction<IEntityLink *(CEntity *const _this, const char *sLinkName, unsigned entityId)>(0x902AF0);
	static inline auto FRemoveEntityLink = PreyFunction<void(CEntity *const _this, IEntityLink *pLink)>(0x908D20);
	static inline auto FRemoveAllEntityLinks = PreyFunction<void(CEntity *const _this)>(0x908C70);
	static inline auto FSetSlotAlpha = PreyFunction<void(CEntity *const _this, int _nSlot, float _alpha)>(0x90BCC0);
	static inline auto FUnmapAttachedChild = PreyFunction<IEntity *(CEntity *const _this, int &partId)>(0x9696C0);
	static inline auto FGetMemoryUsage = PreyFunction<void(CEntity const *const _this, ICrySizer *pSizer)>(0x904A10);
	static inline auto FIsInitialized = PreyFunction<bool(CEntity const *const _this)>(0x906520);
	static inline auto FDebugDraw = PreyFunction<void(CEntity *const _this, SGeometryDebugDrawInfo const &info)>(0x903F80);
	static inline auto FGetSlot = PreyFunction<CEntityObject *(CEntity const *const _this, int nSlot)>(0x904EE0);
	static inline auto FGetSignature = PreyFunction<bool(CEntity *const _this, TSerialize &signature)>(0x904DB0);
	static inline auto FSerializeXML_ExceptScriptProxy = PreyFunction<void(CEntity *const _this, XmlNodeRef &node, bool bLoading)>(0x90AAE0);
	static inline auto FReloadEntity = PreyFunction<bool(CEntity *const _this, SEntityLoadParams &loadParams)>(0x908650);
	static inline auto FSetPoolControl = PreyFunction<void(CEntity *const _this, bool bSet)>(0x90B8E0);
	static inline auto FActivateForNumUpdates = PreyFunction<void(CEntity *const _this, int numUpdates)>(0x902A60);
	static inline auto FSetUpdateStatus = PreyFunction<void(CEntity *const _this)>(0x90C220);
	static inline auto FCalcLocalTM = PreyFunction<void(CEntity const *const _this, Matrix34 &tm)>(0x9030D0);
	static inline auto FIncKeepAliveCounter = PreyFunction<void(CEntity *const _this)>(0x905CA0);
	static inline auto FDecKeepAliveCounter = PreyFunction<void(CEntity *const _this)>(0x903FE0);
	static inline auto FResetKeepAliveCounter = PreyFunction<void(CEntity *const _this)>(0x908DE0);
	static inline auto FIsKeptAlive = PreyFunction<bool(CEntity const *const _this)>(0x906540);
	static inline auto FCheckMaterialFlags = PreyFunction<void(CEntity *const _this)>(0x903280);
	static inline auto FGetDisplayName = PreyFunction<const char *(CEntity const *const _this)>(0x9046A0);
	static inline auto FSetDisplayName = PreyFunction<void(CEntity *const _this, const char *_pName)>(0x90AF20);
	static inline auto FGetSceneMask = PreyFunction<uint8_t(CEntity const *const _this)>(0x904DA0);
	static inline auto FAllocBindings = PreyFunction<void(CEntity *const _this)>(0x902C00);
	static inline auto FDeallocBindings = PreyFunction<void(CEntity *const _this)>(0x903EF0);
	static inline auto FOnRellocate = PreyFunction<void(CEntity *const _this, int nWhyFlags)>(0x907B10);
	static inline auto FAnimationEvent = PreyFunction<void(CEntity *const _this, ICharacterInstance *pCharacter, AnimEventInstance const &event)>(0x902C60);
};

