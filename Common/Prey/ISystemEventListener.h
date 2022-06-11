#pragma once
class ISystemEventListener
{
public:
  virtual ~ISystemEventListener() = 0;
  virtual void OnSystemEventAnyThread(ESystemEvent, unsigned __int64, unsigned __int64) = 0;
  virtual void OnSystemEvent(ESystemEvent, unsigned __int64, unsigned __int64) = 0;


  };
