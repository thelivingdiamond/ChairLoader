#pragma once
class ArkDataComponentBase
{
public:
  virtual bool IsValidData(const unsigned __int64) = 0;


    std::vector<unsigned __int64> m_collected;
};
