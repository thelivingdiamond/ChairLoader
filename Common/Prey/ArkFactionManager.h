#pragma once
#include <set>

#include "ArkBasicTypes.h"
#include "IArkFactionManager.h"
#include "ArkEnums.h"
#include "IArkFactionListener.h"

class ArkFactionManager : IArkFactionManager
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


  unsigned int m_numberOfFactions;
  std::unordered_map<unsigned int,unsigned int> m_entityFactionMap;
  std::vector<std::vector<unsigned int>> m_factionFollowers;
  std::vector<enum EArkDisposition> m_factionDispositions;
  std::unordered_map<std::pair<unsigned int,unsigned int>,enum EArkDisposition,ArkPairHash<std::hash>,std::equal_to<std::pair<unsigned int,unsigned int> >,std::allocator<std::pair<std::pair<unsigned int,unsigned int> const ,enum EArkDisposition> > > m_factionToEntityDisposition;
  std::unordered_map<std::pair<unsigned int,unsigned int>,enum EArkDisposition,ArkPairHash<std::hash>,std::equal_to<std::pair<unsigned int,unsigned int> >,std::allocator<std::pair<std::pair<unsigned int,unsigned int> const ,enum EArkDisposition> > > m_entityToFactionDisposition;
  std::unordered_map<std::pair<unsigned int,unsigned int>,enum EArkDisposition,ArkPairHash<std::hash>,std::equal_to<std::pair<unsigned int,unsigned int> >,std::allocator<std::pair<std::pair<unsigned int,unsigned int> const ,enum EArkDisposition> > > m_entityToEntityDisposition;
  std::set<IArkFactionListener *> m_factionListeners;
  std::unordered_map<CryStringT<char>,unsigned int,std::hash<CryStringT<char> >,std::equal_to<CryStringT<char> >,std::allocator<std::pair<CryStringT<char> const ,unsigned int> > > m_nameToIndexMap;
  std::unordered_map<unsigned __int64,unsigned int> m_idToIndexMap;
  bool m_bDisableListeners;
};
