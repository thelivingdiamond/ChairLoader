#pragma once
#include <unordered_map>

#include "ArkBasicTypes.h"
#include "ArkAiTreeStimulus.h"
#include "ArkAiTree.h"
#include "ArkHashCString.h"
#include "ArkUtils.h"

class ArkAiManager
{
public:
class CachedAiTree
{
  CryStringT<char> m_filePath;
  std::shared_ptr<ArkAiTree> m_pTree;
};

  std::unordered_map<CryStringT<char>,ArkAiManager::CachedAiTree,std::hash<CryStringT<char> >,std::equal_to<CryStringT<char> >,std::allocator<std::pair<CryStringT<char> const ,ArkAiManager::CachedAiTree> > > m_aiTreeCache;
  std::unordered_map<char const *,ArkDelegate<std::unique_ptr<ArkAiTreeStimulus> __cdecl(void)>,ArkHashCString,ArkUtils::ArkCStringEqual,std::allocator<std::pair<char const * const,ArkDelegate<std::unique_ptr<ArkAiTreeStimulus> __cdecl(void)> > > > m_makeStimulusDelegates;
};
