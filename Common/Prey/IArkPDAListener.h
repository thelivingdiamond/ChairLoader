#pragma once
class IArkPDAListener
{
public:
  void OnPDAOpen() = 0;
  virtual void OnPDAOpenComplete() = 0;
  virtual void OnPDAClose() = 0;


  };
