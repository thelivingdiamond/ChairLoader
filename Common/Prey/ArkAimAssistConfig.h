#pragma once
#include "ArkAimAssistEntity.h"
#include "ArkReflectedObject.h"

class ArkAimAssistConfig : ArkReflectedObject
{
  float m_Magnetism;
  float m_SprintMagnetism;
  float m_Centering;
  float m_SprintCentering;
  float m_AimAssistDeadZone;
  std::vector<ArkAimAssistEntity> m_Entities;
};
