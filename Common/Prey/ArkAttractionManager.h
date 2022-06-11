#pragma once
#include <unordered_map>
#include <vector>

class ArkAttractionManager
{
  std::vector<unsigned int> m_attractors;
  std::unordered_map<unsigned int,std::vector<unsigned int>> m_containerToContainedAttractors;
};
