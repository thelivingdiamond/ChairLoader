#pragma once
#include "ArkAiTreeNode.h"
#include "ArkAiTreeNodeParent.h"

class ArkAiTree : ArkAiTreeNodeParent
{
public:
  void DoNotifyChildNodeActivated(ArkAiTreeNodeParent *this, const ArkAiTreeNotifyActivatedContext<ArkAiTreeNode> *) = 0;


  ArkContiguousMemory m_nodes;
};
