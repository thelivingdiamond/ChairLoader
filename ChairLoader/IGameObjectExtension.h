#pragma once
class IGameObjectExtension : IComponent
{
public:
	virtual void GetMemoryUsage(IGameObjectExtension* this, ICrySizer*) = 0;
	virtual bool Init(IGameObjectExtension* this, IGameObject*) = 0;
	virtual void PostInit(IGameObjectExtension* this, IGameObject*) = 0;
	virtual void InitClient(IGameObjectExtension* this, int) = 0;
	virtual void PostInitClient(IGameObjectExtension* this, int) = 0;
	virtual bool ReloadExtension(IGameObjectExtension* this, IGameObject*, const SEntitySpawnParams*) = 0;
	virtual void PostReloadExtension(IGameObjectExtension* this, IGameObject*, const SEntitySpawnParams*) = 0;
	virtual bool GetEntityPoolSignature(IGameObjectExtension* this, CSerializeWrapper<ISerialize>) = 0;
	virtual void Release(IGameObjectExtension* this) = 0;
	virtual void FullSerialize(IGameObjectExtension* this, CSerializeWrapper<ISerialize>) = 0;
	virtual bool NetSerialize(IGameObjectExtension* this, CSerializeWrapper<ISerialize>, EEntityAspects, unsigned __int8, int) = 0;
	virtual unsigned int GetNetSerializeAspects(IGameObjectExtension* this) = 0;
	virtual void PostSerialize(IGameObjectExtension* this) = 0;
	virtual void SerializeSpawnInfo(IGameObjectExtension* this, CSerializeWrapper<ISerialize>) = 0;
	virtual _smart_ptr<ISerializableInfo>* GetSpawnInfo(IGameObjectExtension* this, _smart_ptr<ISerializableInfo>* result) = 0;
	virtual void Update(IGameObjectExtension* this, SEntityUpdateContext*, int) = 0;
	virtual void HandleEvent(IGameObjectExtension* this, const SGameObjectEvent*) = 0;
	virtual void SetChannelId(IGameObjectExtension* this, unsigned __int16) = 0;
	virtual void SetAuthority(IGameObjectExtension* this, bool) = 0;
	virtual const void* GetRMIBase(IGameObjectExtension* this) = 0;
	virtual void PostUpdate(IGameObjectExtension* this, float) = 0;
	virtual void PostRemoteSpawn(IGameObjectExtension* this) = 0;
	IGameObject* m_pGameObject;
	virtual unsigned int m_entityId;
	virtual IEntity* m_pEntity;
};