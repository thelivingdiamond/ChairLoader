#pragma once
#include <vector>

#include "IArkActiveUserManagerListener.h"

class ArkActiveUserManagerBase
{
public:
  void RegisterActiveUser(unsigned int) = 0;
  virtual void ClearActiveUser() = 0;
  virtual bool IsActiveUserLoggedIn() = 0;
  virtual void EnsureActiveUserValid(const unsigned __int64) = 0;
  virtual void OnUserSignOutStarted(const unsigned int) = 0;
  virtual void OnUserSignOutCompleted(const unsigned int) = 0;
  virtual void OnActiveUserControllerRemoved(const unsigned int) = 0;
  virtual void OnActiveUserControllerInvalid(const unsigned int) = 0;


    std::vector<IArkActiveUserManagerListener *> m_listeners;
  bool m_bListening;
  bool m_bReengagementPhase;
  bool m_bDisconnectQueued;
};
