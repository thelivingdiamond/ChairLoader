#pragma once
class IArkPlayerCarryListener
{
public:
  void OnStartCarry(IEntity *const) = 0;
  virtual void OnStopCarry(IEntity *const, bool) = 0;


  };
