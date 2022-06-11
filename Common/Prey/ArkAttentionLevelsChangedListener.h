#pragma once
class ArkAttentionLevelsChangedListener
{
public:
  virtual void DoOnAttentionLevelsChanged(const ArkAttentionLevelChange *, const ArkAttentionLevelChange *) = 0;


  };
