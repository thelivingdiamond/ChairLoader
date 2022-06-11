#pragma once
class IEntity
{
public:
	virtual ~IEntity(IEntity* this) = 0;
	virtual unsigned int GetId(IEntity* this) = 0;
	virtual unsigned __int64 GetGuid(IEntity* this) = 0;
	virtual IEntityClass* GetClass(IEntity* this) = 0;
	virtual const IEntityArchetype* GetArchetype(IEntity* this) = 0;
	virtual IEntityArchetype* GetArchetype(IEntity* this) = 0;
	virtual void SetFlags(IEntity* this, unsigned int) = 0;
	virtual unsigned int GetFlags(IEntity* this) = 0;
	virtual void AddFlags(IEntity* this, unsigned int) = 0;
	virtual void ClearFlags(IEntity* this, unsigned int) = 0;
	virtual bool CheckFlags(IEntity* this, unsigned int) = 0;
	virtual void SetFlagsExtended(IEntity* this, unsigned int) = 0;
	virtual unsigned int GetFlagsExtended(IEntity* this) = 0;
	virtual bool IsGarbage(IEntity* this) = 0;
	virtual void SetName(IEntity* this, const char*) = 0;
	virtual const char* GetName(IEntity* this) = 0;
	virtual const char* GetEntityTextDescription(IEntity* this) = 0;
	virtual void SerializeXML(IEntity* this, XmlNodeRef*, bool) = 0;
	virtual bool IsLoadedFromLevelFile(IEntity* this) = 0;
	virtual void AttachChild(IEntity* this, IEntity*, const SChildAttachParams*) = 0;
	virtual void DetachAll(IEntity* this, int) = 0;
	virtual void DetachThis(IEntity* this, int, int) = 0;
	virtual int GetChildCount(IEntity* this) = 0;
	virtual IEntity* GetChild(IEntity* this, int) = 0;
	virtual IEntity* GetParent(IEntity* this) = 0;
	virtual Matrix34_tpl<float>* GetParentAttachPointWorldTM(IEntity* this, Matrix34_tpl<float>* result) = 0;
	virtual bool IsParentAttachmentValid(IEntity* this) = 0;
	virtual void SetWorldTM(IEntity* this, const Matrix34_tpl<float>*, int) = 0;
	virtual void SetLocalTM(IEntity* this, const Matrix34_tpl<float>*, int) = 0;
	virtual const Matrix34_tpl<float>* GetWorldTM(IEntity* this) = 0;
	virtual Matrix34_tpl<float>* GetLocalTM(IEntity* this, Matrix34_tpl<float>* result) = 0;
	virtual void GetWorldBounds(IEntity* this, AABB*) = 0;
	virtual void GetLocalBounds(IEntity* this, AABB*) = 0;
	virtual void SetPos(IEntity* this, const Vec3_tpl<float>*, int, bool, bool) = 0;
	virtual const Vec3_tpl<float>* GetPos(IEntity* this) = 0;
	virtual void SetRotation(IEntity* this, const Quat_tpl<float>*, int) = 0;
	virtual const Quat_tpl<float>* GetRotation(IEntity* this) = 0;
	virtual void SetScale(IEntity* this, const Vec3_tpl<float>*, int) = 0;
	virtual const Vec3_tpl<float>* GetScale(IEntity* this) = 0;
	virtual void SetPosRotScale(IEntity* this, const Vec3_tpl<float>*, const Quat_tpl<float>*, const Vec3_tpl<float>*, int) = 0;
	virtual Vec3_tpl<float>* GetWorldPos(IEntity* this, Vec3_tpl<float>* result) = 0;
	virtual Ang3_tpl<float>* GetWorldAngles(IEntity* this, Ang3_tpl<float>* result) = 0;
	virtual Quat_tpl<float>* GetWorldRotation(IEntity* this, Quat_tpl<float>* result) = 0;
	virtual const Vec3_tpl<float>* GetForwardDir(IEntity* this) = 0;
	virtual void Activate(IEntity* this, bool) = 0;
	virtual bool IsActive(IEntity* this) = 0;
	virtual bool IsFromPool(IEntity* this) = 0;
	virtual void PrePhysicsActivate(IEntity* this, bool) = 0;
	virtual bool IsPrePhysicsActive(IEntity* this) = 0;
	virtual void Serialize(IEntity* this, CSerializeWrapper<ISerialize>, int) = 0;
	virtual bool SendEvent(IEntity* this, SEntityEvent*) = 0;
	virtual void SetTimer(IEntity* this, int, int) = 0;
	virtual void KillTimer(IEntity* this, int) = 0;
	virtual void Hide(IEntity* this, bool) = 0;
	virtual bool IsHidden(IEntity* this) = 0;
	virtual void Invisible(IEntity* this, bool) = 0;
	virtual bool IsInvisible(IEntity* this) = 0;
	virtual IAIObject* GetAI(IEntity* this) = 0;
	virtual bool HasAI(IEntity* this) = 0;
	virtual unsigned int GetAIObjectID(IEntity* this) = 0;
	virtual void SetAIObjectID(IEntity* this, unsigned int) = 0;
	virtual bool RegisterInAISystem(IEntity* this, const AIObjectParams*) = 0;
	virtual void SetUpdatePolicy(IEntity* this, EEntityUpdatePolicy) = 0;
	virtual EEntityUpdatePolicy GetUpdatePolicy(IEntity* this) = 0;
	virtual IEntityProxy* GetProxy(IEntity* this, EEntityProxy) = 0;
	virtual void SetProxy(IEntity* this, EEntityProxy, std::shared_ptr<IEntityProxy>) = 0;
	virtual std::shared_ptr<IEntityProxy>* CreateProxy(IEntity* this, std::shared_ptr<IEntityProxy>* result, EEntityProxy) = 0;
	virtual void RegisterComponent(IEntity* this, std::shared_ptr<IComponent>, const int) = 0;
	virtual void Physicalize(IEntity* this, SEntityPhysicalizeParams*) = 0;
	virtual IPhysicalEntity* GetPhysics(IEntity* this) = 0;
	virtual int PhysicalizeSlot(IEntity* this, int, SEntityPhysicalizeParams*) = 0;
	virtual void UnphysicalizeSlot(IEntity* this, int) = 0;
	virtual void UpdateSlotPhysics(IEntity* this, int) = 0;
	virtual void SetPhysicsState(IEntity* this, XmlNodeRef*) = 0;
	virtual bool HasPhysicsState(IEntity* this) = 0;
	virtual void SetMaterial(IEntity* this, IMaterial*) = 0;
	virtual IMaterial* GetMaterial(IEntity* this) = 0;
	virtual bool IsSlotValid(IEntity* this, int) = 0;
	virtual void FreeSlot(IEntity* this, int) = 0;
	virtual int GetSlotCount(IEntity* this) = 0;
	virtual bool GetSlotInfo(IEntity* this, int, SEntitySlotInfo*) = 0;
	virtual const Matrix34_tpl<float>* GetSlotWorldTM(IEntity* this, int) = 0;
	virtual const Matrix34_tpl<float>* GetSlotLocalTM(IEntity* this, int, bool) = 0;
	virtual void SetSlotLocalTM(IEntity* this, int, const Matrix34_tpl<float>*, int) = 0;
	virtual void SetSlotCameraSpacePos(IEntity* this, int, const Vec3_tpl<float>*) = 0;
	virtual void GetSlotCameraSpacePos(IEntity* this, int, Vec3_tpl<float>*) = 0;
	virtual bool SetParentSlot(IEntity* this, int, int) = 0;
	virtual void SetSlotMaterial(IEntity* this, int, IMaterial*) = 0;
	virtual void SetSlotFlags(IEntity* this, int, unsigned int) = 0;
	virtual unsigned int GetSlotFlags(IEntity* this, int) = 0;
	virtual bool ShouldUpdateCharacter(IEntity* this, int) = 0;
	virtual ICharacterInstance* GetCharacter(IEntity* this, int) = 0;
	virtual int SetCharacter(IEntity* this, ICharacterInstance*, int) = 0;
	virtual IStatObj* GetStatObj(IEntity* this, int) = 0;
	virtual IParticleEmitter* GetParticleEmitter(IEntity* this, int) = 0;
	virtual IGeomCacheRenderNode* GetGeomCacheRenderNode(IEntity* this, int) = 0;
	virtual void MoveSlot(IEntity* this, IEntity*, int) = 0;
	virtual int SetStatObj(IEntity* this, IStatObj*, int, bool, float) = 0;
	virtual int LoadGeometry(IEntity* this, int, const char*, const char*, int) = 0;
	virtual int LoadCharacter(IEntity* this, int, const char*, int) = 0;
	virtual int LoadGeomCache(IEntity* this, int, const char*) = 0;
	virtual int LoadParticleEmitter(IEntity* this, int, IParticleEffect*, const SpawnParams*, bool, bool) = 0;
	virtual int SetParticleEmitter(IEntity* this, int, IParticleEmitter*, bool) = 0;
	virtual int SetBreakableGlass(IEntity* this, int, IArkGlass*) = 0;
	virtual IArkGlass* GetBreakableGlass(IEntity* this, int) = 0;
	virtual int LoadLight(IEntity* this, int, CDLight*) = 0;
	virtual void InvalidateTM(IEntity* this, int, bool) = 0;
	virtual void EnablePhysics(IEntity* this, bool) = 0;
	virtual IEntityLink* GetEntityLinks(IEntity* this) = 0;
	virtual IEntityLink* AddEntityLink(IEntity* this, const char*, unsigned int) = 0;
	virtual void RemoveEntityLink(IEntity* this, IEntityLink*) = 0;
	virtual void RemoveAllEntityLinks(IEntity* this) = 0;
	virtual void SetSlotAlpha(IEntity* this, int, float) = 0;
	virtual IEntity* UnmapAttachedChild(IEntity* this, int*) = 0;
	virtual bool IsInitialized(IEntity* this) = 0;
	virtual void DebugDraw(IEntity* this, const SGeometryDebugDrawInfo*) = 0;
	virtual void GetMemoryUsage(IEntity* this, ICrySizer*) = 0;
	virtual void IncKeepAliveCounter(IEntity* this) = 0;
	virtual void DecKeepAliveCounter(IEntity* this) = 0;
	virtual void ResetKeepAliveCounter(IEntity* this) = 0;
	virtual bool IsKeptAlive(IEntity* this) = 0;
	virtual const char* GetDisplayName(IEntity* this) = 0;
	virtual void SetDisplayName(IEntity* this, const char*) = 0;
};
