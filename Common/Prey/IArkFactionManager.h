#pragma once
class IArkFactionManager
{
public:
  unsigned int GetFactionIndex(const unsigned __int64 *) = 0;
  virtual unsigned int GetFactionIndex(const char *const) = 0;
  virtual const CryStringT<char> *GetFactionName(unsigned __int64) = 0;
  virtual const CryStringT<char> *GetFactionName(unsigned int) = 0;
  virtual unsigned __int64 GetFactionId(unsigned int) = 0;
  virtual unsigned __int64 GetFactionId(const char *const) = 0;
  virtual unsigned int GetEntityFaction(unsigned int) = 0;
  virtual void SetEntityDispositionToEntity(unsigned int, unsigned int, EArkDisposition, bool) = 0;
  virtual void SetFactionDispositionToEntity(unsigned int, unsigned int, EArkDisposition, bool) = 0;
  virtual void SetFactionDispositionToFaction(unsigned int, unsigned int, EArkDisposition, bool) = 0;
  virtual EArkDisposition GetEffectiveFactionDispositionToEntity(unsigned int, unsigned int) = 0;
  virtual EArkDisposition GetEffectiveEntityDispositionToEntity(unsigned int, unsigned int) = 0;
  virtual void ReloadFactions() = 0;
  virtual const std::unordered_map<unsigned __int64,unsigned int> *GetGuidMap() = 0;


  };
