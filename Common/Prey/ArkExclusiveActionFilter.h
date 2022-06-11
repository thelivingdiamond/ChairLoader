#pragma once
#include "IUIControlSchemeListener.h"
#include "IActionMapEventListener.h"

class ArkExclusiveActionFilter : IUIControlSchemeListener, IActionMapEventListener
{
public:
  bool m_bIgnoreActionMapState;
  CryStringT<char> m_filterName;
  CryStringT<char> m_actionMap;
  std::vector<CCryName> m_filteredActionIds;
  std::vector<CCryName> m_allowedActionIds;
};
