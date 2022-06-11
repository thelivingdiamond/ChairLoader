#pragma once
#include "ArkAimAssistShape.h"
#include "ArkBasicTypes.h"
#include "ArkReflectedObject.h"

class ArkAimAssistEntity : ArkReflectedObject
{
  CryStringT<char> m_Name;
  ArkAimAssistShape m_Shape;
};
