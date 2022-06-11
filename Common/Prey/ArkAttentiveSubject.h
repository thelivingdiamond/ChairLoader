#pragma once
#include "CryMath/Cry_Vector3.h"
#include "ArkAttentionLevelChange.h"

class ArkAttentiveSubject
{
public:
  virtual unsigned int DoGetEntityId() = 0;
  virtual unsigned __int64 DoGetAttentionModelId() = 0;
  virtual void DoGetAttentiveSubjectInfo(bool *, Vec3_tpl<float> *) = 0;
  virtual void DoOnAttentionLevelsChanged(const ArkAttentionLevelChange *, const ArkAttentionLevelChange *) = 0;
  virtual void DoOnAttentionProxiesUpdated(const unsigned int *, const unsigned int *) = 0;


  };
