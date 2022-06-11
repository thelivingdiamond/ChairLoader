#pragma once
class ArkReflectedLibrary : ArkReflectedObject
{
public:
  virtual ~ArkReflectedLibrary() = 0;
  virtual bool Init() = 0;
  virtual bool Shutdown() = 0;
  virtual bool IsAlwaysLoaded() = 0;


  };
