#pragma once
class IEntitySystemSink
{
public:
  virtual ~IEntitySystemSink() = 0;
  virtual bool OnBeforeSpawn(SEntitySpawnParams *) = 0;
  virtual void OnSpawn(IEntity *, SEntitySpawnParams *) = 0;
  virtual bool OnRemove(IEntity *) = 0;
  virtual void OnReused(IEntity *, SEntitySpawnParams *) = 0;
  virtual void OnEvent(IEntity *, SEntityEvent *) = 0;
  virtual void GetMemoryUsage(ICrySizer *) = 0;


  };
