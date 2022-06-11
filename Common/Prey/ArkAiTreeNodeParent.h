#pragma once
class ArkAiTreeNodeParent
{
public:
  void DoNotifyChildNodeActivated(const ArkAiTreeNotifyActivatedContext<ArkAiTreeNode> *) = 0;
  virtual void DoNotifyChildNodeCompleted(const ArkAiTreeNotifyCompletedContext<ArkAiTreeNode> *, EArkAiTreeCompletionResult) = 0;


  };
