#pragma once
class IEntityEventListener
{
public:
  virtual ~IEntityEventListener() = 0;
  virtual void OnEntityEvent(IEntity *, SEntityEvent *) = 0;


  };
