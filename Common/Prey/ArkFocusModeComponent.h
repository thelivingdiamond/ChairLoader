#pragma once
#include "ArkAudioTrigger.h"
#include "ArkFocusModeUIComponent.h"
#include "ArkFocusModeEffectsManager.h"
#include "CArkFlowNodeFocusModeEnabled.h"
#include "ArkTimeScaler.h"


class ArkFocusModeComponent
{
  ArkFocusModeUIComponent m_uiComponent;
  ArkFocusModeEffectsManager m_effectsManager;
  ArkTimeScaler m_gameTimeScaler;
  ArkTimeScaler m_playerTimeScaler;
  ArkAudioTrigger m_pauseGameTimerTrigger;
  ArkAudioTrigger m_resumeGameTimerTrigger;
  const CArkFlowNodeFocusModeEnabled *m_pFlowNodeEnabled;
  bool m_bEnabled;
  bool m_bJustSerialized;
  int m_noPowersStringIndex;
  int m_inputHandle;
};
