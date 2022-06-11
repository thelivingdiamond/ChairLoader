#pragma once
class IArkPauseMenuListener
{
public:
  virtual void OnOptionMenuClosed() = 0;
  virtual void OnPauseMenuOpened() = 0;


  };
