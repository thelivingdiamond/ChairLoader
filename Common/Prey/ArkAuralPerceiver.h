#pragma once
#include "ArkComplexAuralPerceptionInfo.h"
#include "ArkSimpleAuralPerceptionInfo.h"
#include "CryMath/Cry_Vector3.h"

class ArkAuralPerceiver
{
public:
  virtual unsigned int DoGetEntityId() = 0;
  virtual Vec3_tpl<float> *DoGetHearingPosition(Vec3_tpl<float> *result) = 0;
  virtual void DoUpdateAuralPerception(const ArkSimpleAuralPerceptionInfo *, const ArkSimpleAuralPerceptionInfo *, const ArkComplexAuralPerceptionInfo *, const ArkComplexAuralPerceptionInfo *) = 0;


  };
