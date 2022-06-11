#pragma once
#include "ArkDialogAudioTrigger.h"
class IArkDialogAudioTriggerOwner
{
public:
  virtual void OnTriggerLoaded(const ArkDialogAudioTrigger *, bool) = 0;
  virtual void OnTriggerCutoff(const ArkDialogAudioTrigger *) = 0;
  virtual void OnTriggerFinished(const ArkDialogAudioTrigger *) = 0;


  };
