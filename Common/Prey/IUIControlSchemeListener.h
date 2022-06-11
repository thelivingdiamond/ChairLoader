#pragma once
#include "EControlScheme.h"

class IUIControlSchemeListener
{
public:
  virtual ~IUIControlSchemeListener() = 0;
  virtual bool OnControlSchemeChanged(const EControlScheme) = 0;


  };
