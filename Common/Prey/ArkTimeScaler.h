#pragma once
class ArkTimeScaler
{
  float m_initialTimeScale;
  float m_targetTimeScale;
  float m_currentTimeScale;
  float m_elapsedSec;
  float m_durationSec;
  int m_timeScaleHandle;
  ArkTimeScaleManager::EArkTimerFlag m_timer;
};
