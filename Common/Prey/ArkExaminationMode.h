#pragma once

#include "IArkPlayerCarryListener.h"
#include "IEntityEventListener.h"
#include "IUIControlSchemeListener.h"
#include "CryMath/Cry_Vector2.h"
#include "CryMath/Cry_Vector3.h"
#include "ArkInputLegend.h"
#include "ArkInputLegendHandler.h"

class ArkExaminationMode : IEntityEventListener, IUIControlSchemeListener, IArkPlayerCarryListener
{
public:


  Quat_tpl<float> m_initialRotation;
  Vec3_tpl<float> m_playerCameraPosition;
  Quat_tpl<float> m_playerCameraRotation;
  Ang3_tpl<float> m_localRotation;
  Quat_tpl<float> m_rotation;
  Vec2_tpl<float> m_reticlePos;
  Vec3_tpl<float> m_optimalCameraPos;
  float m_cameraLerpStartTime;
  float m_activeLerpDuration;
  float m_viewingBuffer;
  float m_maxCameraRotation;
  float m_cameraSpeedMultiplier;
  float m_cameraLerpDuration;
  float m_telescopeLerpDuration;
  float m_distFromNonWorldUI;
  ArkExaminationMode::EArkExaminationType m_examinationType;
  ArkExaminationMode::EArkExaminationState m_examinationState;
  unsigned int m_targetEntity;
  int m_inputHandle;
  int m_zoomHandle;
  ArkExaminationMode::ExamineMode m_mode;
  ArkInputLegend m_inputLegendWorldUI;
  ArkInputLegend m_inputLegendDefault;
  ArkInputLegend m_inputLegendTelescope;
  ArkInputLegendHandler m_examinationLegendHandler;
};
