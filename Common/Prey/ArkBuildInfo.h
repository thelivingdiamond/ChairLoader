#pragma once
#include <map>

#include "ArkBasicTypes.h"

class ArkBuildInfo
{
  bool m_isLocalBuild;
  std::map<CryStringT<char>,CryStringT<char>,std::less<CryStringT<char> >,std::allocator<std::pair<CryStringT<char> const ,CryStringT<char> > > > m_buildInfo;
  const CryStringT<char> k_buildIDKey;
  const CryStringT<char> k_branchKey;
  const CryStringT<char> k_changeListKey;
  CryStringT<char> m_Description;
  CryStringT<char> m_FullInfo;
  std::mutex m_buildInfoMutex;
};
