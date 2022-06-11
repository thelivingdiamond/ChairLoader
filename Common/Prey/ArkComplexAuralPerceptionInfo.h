#pragma once
#include "ArkGameNoiseInfo.h"
#include "CryMath/Cry_Vector3.h"

class ArkComplexAuralPerceptionInfo
{
  unsigned int m_noiseMakerEntityId;
  ArkGameNoiseInfo m_noiseInfo;
  float m_magnitude;
  Vec3_tpl<float> m_noisePosition;
};
