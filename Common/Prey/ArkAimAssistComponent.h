#pragma once
#include <IEntity.h>
#include <unordered_map>
#include <vector>

#include "IEntityClass.h"
#include "IEntitySystemSink.h"
#include "ArkAimAssistConfig.h"
#include "ArkAimAssistEntity.h"
#include "CryMath/Cry_Vector3.h"

class ArkAimAssistComponent : IEntitySystemSink
{
public:
class ArkAimAssistTarget
{
  std::vector<Vec3_tpl<float>> attachPoints;
  Vec3_tpl<float> aimPos;
  IEntity *pEntity;
  float score;
  float distanceSq;
  bool bHasLOS;
};



  ArkAimAssistConfig m_config;
  ArkAimAssistComponent::ArkAimAssistTarget m_target;
  std::vector<IEntity *> m_potentialTargets;
  std::unordered_map<IEntityClass const *,ArkAimAssistEntity const *> m_entityConfigs;
  unsigned __int64 m_CQCPostProcessId;
  float m_fCQCMinDistanceSq;
  float m_deadZoneSq;
  unsigned int m_nAnalyzed;
  bool m_bShowingCQCPostProcess;
};
