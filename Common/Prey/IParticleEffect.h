#pragma once
class IParticleEffect : _i_reference_target<int>
{
public:
  virtual int GetVersion() = 0;
  virtual void GetMemoryUsage(ICrySizer *) = 0;
  virtual IParticleEmitter *Spawn(const ParticleLoc *, unsigned int, const SpawnParams *) = 0;
  virtual void SetName(const char *) = 0;
  virtual const char *GetName() = 0;
  virtual CryStackStringT<char,512> *GetFullName(CryStackStringT<char,512> *result) = 0;
  virtual void SetEnabled(bool) = 0;
  virtual bool IsEnabled() = 0;
  virtual bool IsTemporary() = 0;
  virtual void SetParticleParams(const ParticleParams *) = 0;
  virtual const ParticleParams *GetParticleParams() = 0;
  virtual const ParticleParams *GetDefaultParams() = 0;
  virtual int GetChildCount() = 0;
  virtual IParticleEffect *GetChild(int) = 0;
  virtual void ClearChilds() = 0;
  virtual void InsertChild(int, IParticleEffect *) = 0;
  virtual int FindChild(IParticleEffect *) = 0;
  virtual void SetParent(IParticleEffect *) = 0;
  virtual IParticleEffect *GetParent() = 0;
  virtual bool LoadResources() = 0;
  virtual void UnloadResources() = 0;
  virtual void Serialize(XmlNodeRef, bool, bool) = 0;
  virtual void Reload(bool) = 0;
  virtual const IParticleAttributes *GetAttributes() = 0;


};
