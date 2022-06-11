#pragma once
#include <vector>

#include "ArkAutoResetTimer.h"
#include "ArkCameraShake.h"
#include "ArkSimpleTimer.h"
#include "SAnimationContext.h"
#include "CryMath/Cry_Vector3.h"

class ArkCameraShakeManager
{
public:
class ChannelEntry
{
  unsigned __int64 m_channelId;
  unsigned int m_disabledCount;
};


class CameraShakeEntry
{
  const ArkCameraShake *m_pCameraShake;
  ArkSimpleTimer m_phaseTimeRemaining;
  ArkAutoResetTimer m_horizontalTimer;
  ArkAutoResetTimer m_verticalTimer;
  unsigned char m_currentPhase[4];
  Vec3_tpl<float> m_sourcePosition;
  float m_intensity;
  unsigned __int16 m_forceFeedbackId;
  _smart_ptr<TAction<SAnimationContext> > m_pAnimAction;
};


  std::vector<ArkCameraShakeManager::CameraShakeEntry> m_cameraShakes;
  std::vector<ArkCameraShakeManager::ChannelEntry> m_channels;
  float m_horizontalUIOffset;
  float m_verticalUIOffset;
};
