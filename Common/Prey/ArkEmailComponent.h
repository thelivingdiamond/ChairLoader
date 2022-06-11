#pragma once
#include <unordered_map>
#include <vector>

#include "ArkDataComponentBase.h"

class ArkEmailComponent : ArkDataComponentBase
{
public:

  std::vector<unsigned __int64> m_downloads;
  std::unordered_map<unsigned __int64,bool> m_downloadHidden;
  std::unordered_map<unsigned __int64,bool> m_sentEmails;
};
