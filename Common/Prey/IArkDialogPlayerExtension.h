#pragma once
class IArkDialogPlayerExtension
{
public:
  virtual void OnDialogStart(unsigned __int64) = 0;
  virtual void OnDialogEnd(unsigned __int64) = 0;


  };
