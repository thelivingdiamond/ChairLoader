#pragma once
#include <unordered_map>

#include "IEntityClass.h"
#include "IEntitySystemSink.h"

class ArkDistractionEntitySink : IEntitySystemSink
{
public:
class DistractionInfo
{
  unsigned __int64 m_distractionId;
  ArkDistractionEntitySink::EDistractionLifetime m_lifetime;
};


class EntityClassDistractionInfo
{
  unsigned __int64 m_distractionId;
  ArkDistractionEntitySink::EDistractionLifetime m_lifetime;
  float m_minimumMass;
  float m_maximumMass;
};




  std::unordered_map<IEntityClass const *,std::vector<ArkDistractionEntitySink::EntityClassDistractionInfo>> m_entityClassToDistractionsMap;
  std::unordered_map<unsigned int,std::vector<ArkDistractionEntitySink::DistractionInfo>> m_archetypeCrcToDistractionsMap;
  std::unordered_map<unsigned __int64,std::vector<ArkDistractionEntitySink::DistractionInfo>> m_metaTagCrcToDistractionsMap;
  unsigned __int64 m_mimicryDistractionId;
};
