class IGameObjectExtension : IComponent
{
public:
  virtual void GetMemoryUsage(ICrySizer *) = 0;
  virtual bool Init(IGameObject *) = 0;
  virtual void PostInit(IGameObject *) = 0;
  virtual void InitClient(int) = 0;
  virtual void PostInitClient(int) = 0;
  virtual bool ReloadExtension(IGameObject *, const SEntitySpawnParams *) = 0;
  virtual void PostReloadExtension(IGameObject *, const SEntitySpawnParams *) = 0;
  virtual bool GetEntityPoolSignature(CSerializeWrapper<ISerialize>) = 0;
  virtual void Release() = 0;
  virtual void FullSerialize(CSerializeWrapper<ISerialize>) = 0;
  virtual bool NetSerialize(CSerializeWrapper<ISerialize>, EEntityAspects, unsigned __int8, int) = 0;
  virtual unsigned int GetNetSerializeAspects() = 0;
  virtual void PostSerialize() = 0;
  virtual void SerializeSpawnInfo(CSerializeWrapper<ISerialize>) = 0;
  virtual _smart_ptr<ISerializableInfo> *GetSpawnInfo(_smart_ptr<ISerializableInfo> *result) = 0;
  virtual void Update(SEntityUpdateContext *, int) = 0;
  virtual void HandleEvent(const SGameObjectEvent *) = 0;
  virtual void SetChannelId(unsigned __int16) = 0;
  virtual void SetAuthority(bool) = 0;
  virtual const void *GetRMIBase() = 0;
  virtual void PostUpdate(float) = 0;
  virtual void PostRemoteSpawn() = 0;


  IGameObject *m_pGameObject;
  unsigned int m_entityId;
  IEntity *m_pEntity;
};
