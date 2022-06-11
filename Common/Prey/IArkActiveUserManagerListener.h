#pragma once
class IArkActiveUserManagerListener
{
public:
  virtual void OnActiveUserIdChanged(unsigned int) = 0;
  virtual void OnActiveUserDisconnected(bool) = 0;
  virtual bool OnActiveUserReengaged() = 0;
  virtual void OnActiveUserPostReengaged() = 0;
  virtual void OnActiveUserSignedOut() = 0;


  };
