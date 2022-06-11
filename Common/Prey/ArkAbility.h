#pragma once
#include "ArkBasicTypes.h"
#include "ArkEnums.h"
#include "ArkReflectedObject.h"
#include "ArkStatModifier.h"
#include "ArkSignalModifer.h"

class ArkAbility : ArkReflectedObject
{
  unsigned __int64 m_ID;
  CryStringT<char> m_Icon;
  CryStringT<char> m_Name;
  CryStringT<char> m_Label;
  CryStringT<char> m_Description;
  int m_Tier;
  float m_NightmareHeat;
  unsigned __int8 m_Cost;
  bool m_RequireScanner;
  std::vector<unsigned __int64> m_Prereqs;
  EArkPsiPowers m_Power;
  unsigned __int8 m_PowerLevel;
  std::vector<ArkSignalModifer> m_SignalModifiers;
  std::vector<ArkStatModifier> m_StatModifiers;
  CryStringT<char> m_Tutorial;
  CryStringT<char> m_TutorialPC;
  CryStringT<char> m_TutorialImage;
};
