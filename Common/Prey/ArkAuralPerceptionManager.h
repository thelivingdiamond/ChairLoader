#pragma once
#include <map>
#include <unordered_map>
#include "ArkAuralPerceiver.h"
#include "ArkAuralPerceivable.h"
#include "ArkAutoResetTimer.h"
#include "ArkSimpleAuralPerceptionInfo.h"
#include "ArkComplexAuralPerceptionInfo.h"
#include "ArkGameNoiseInfo.h"


class ArkAuralPerceptionManager
{
public:
class PerceivedNoise
{
  unsigned __int64 m_perceiverIndex;
  unsigned __int64 m_noiseIndex;
  float m_occlusion;
};


class PerceivableInfo
{
  bool m_bInVacuum;
  bool m_bIsComplex;
};


class PerceiverInfo
{
  unsigned int m_entityId;
  Vec3_tpl<float> m_hearingPosition;
};


class Noise
{
  unsigned int m_noiseMakerEntityId;
  ArkGameNoiseInfo m_noiseInfo;
  float m_magnitude;
  Vec3_tpl<float> m_position;
  float m_radius;
};


class RepeatingNoise
{
  int m_id;
  ArkAutoResetTimer m_repeatTimer;
  ArkAuralPerceptionManager::Noise m_noise;
};


  float m_noiseRadiusScalarInVacuum;
  std::unordered_map<unsigned int,unsigned __int64> m_perceiverIndexMap;
  std::unordered_map<unsigned int,unsigned __int64> m_perceivableIndexMap;
  std::vector<unsigned __int64> m_freePerceiverIndexes;
  std::vector<ArkAuralPerceiver *> m_perceivers;
  std::vector<unsigned __int64> m_freePerceivableIndexes;
  std::vector<ArkAuralPerceivable const *> m_perceivables;
  std::vector<ArkAuralPerceptionManager::RepeatingNoise> m_repeatingNoises;
  std::vector<ArkAuralPerceptionManager::Noise> m_noises;
  std::vector<ArkAuralPerceptionManager::PerceiverInfo> m_perceiverInfo;
  std::map<unsigned int,ArkAuralPerceptionManager::PerceivableInfo> m_perceivableInfo;
  std::vector<ArkAuralPerceptionManager::PerceivedNoise> m_perceivedNoises;
  std::vector<ArkSimpleAuralPerceptionInfo> m_simpleNotificationBuffer;
  std::vector<ArkComplexAuralPerceptionInfo> m_complexNotificationBuffer;
};
