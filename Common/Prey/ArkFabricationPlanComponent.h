#pragma once
#include <vector>

#include "ArkBasicTypes.h"

class ArkFabricationPlanComponent
{
public:
class Data
{
  unsigned __int64 m_id;
  bool m_bNew;
  int m_count;
  bool m_bErrorSet;
  CryStringT<char> m_errorHeading;
  CryStringT<char> m_errorBody;
};


  std::vector<ArkFabricationPlanComponent::Data> m_plans;
};
