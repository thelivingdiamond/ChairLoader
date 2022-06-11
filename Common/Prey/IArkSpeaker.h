#pragma once
#include "IArkConversation.h"

class IArkSpeaker
{
public:
  bool PlayDialog(SDialogParams *) = 0;
  virtual const char *GetName() = 0;
  virtual const IArkConversation *GetConversation() = 0;
  virtual void Stop(bool, bool) = 0;


  };
