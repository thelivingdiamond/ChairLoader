#pragma once
#include "CryMath/Cry_Vector2.h"

class ArkNpc
{
public:
  virtual void InitializeTrackviewTempNpc() = 0;
  virtual void OnAnimatedMovement_Start() = 0;
  virtual void OnAnimatedMovement_Stop() = 0;
  virtual bool IsMovementAnimated() = 0;
  virtual void OnJumpAnimActionSequence_Start() = 0;
  virtual void OnJumpAnimActionSequence_Stop() = 0;
  virtual void OnJumpAnimAction_Fall() = 0;
  virtual void OnMantleAnimAction_Start() = 0;
  virtual void OnMantleAnimAction_Stop() = 0;
  virtual bool PerformMimicReorientation(const Vec2_tpl<float> *) = 0;
  virtual void PerformMimicGlitch() = 0;
  virtual void StopMimicGlitch() = 0;
  virtual unsigned int GetTopAttentionTargetEntityId() = 0;
  virtual void OnTargetDead(unsigned int) = 0;
  virtual void ShiftTelegraph(MovementStyle::ShiftType) = 0;
  virtual void ShiftBegin(MovementStyle::ShiftType) = 0;
  virtual void ShiftEnd(MovementStyle::ShiftType) = 0;
  virtual bool SupportsLookAt() = 0;
  virtual bool IsGlooSlow() = 0;
  virtual bool IsFalling() = 0;
  virtual void SetBreakGlooEntity(unsigned int) = 0;
  virtual void BreakGlooEntity() = 0;
  virtual void ForceMovementReplan() = 0;
  virtual void ForceReevaluateMovementAnimAction() = 0;
  virtual void CancelMovement() = 0;
  virtual void PushDisableHitReactions() = 0;
  virtual void PopDisableHitReactions() = 0;
  virtual void PushAIAlwaysUpdate() = 0;
  virtual void PopAIAlwaysUpdate() = 0;
  virtual void ResetAIAlwaysUpdate() = 0;
  virtual void OnNewAttentionTarget(unsigned int, bool) = 0;


  };
