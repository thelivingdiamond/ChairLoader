#pragma once
#include "IArkSpeaker.h"

class IArkConversation
{
public:
  virtual void AddSpeaker(IArkSpeaker *) = 0;
  virtual void AddListener(IArkConversationListener *const) = 0;
  virtual void RemoveListener(IArkConversationListener *const) = 0;
  virtual unsigned __int64 GetConversationId() = 0;


  };
