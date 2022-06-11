#pragma once
#include "SActionMapEvent.h"

class IActionMapEventListener
{
public:
  virtual ~IActionMapEventListener() = 0;
  virtual void OnActionMapEvent(const SActionMapEvent *) = 0;


  };
