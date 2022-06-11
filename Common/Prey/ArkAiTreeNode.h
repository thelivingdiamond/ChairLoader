#pragma once
class ArkAiTreeNode : ArkAiTreeNodeParent
{
public:
  void DoNotifyChildNodeActivated(ArkAiTreeNodeParent *this, const ArkAiTreeNotifyActivatedContext<ArkAiTreeNode> *) = 0;
  virtual virtual ~ArkAiTreeNode() = 0;
  virtual const char *DoGetClassName() = 0;
  virtual unsigned __int64 DoGetAiTreeInstanceNodeAlignment() = 0;
  virtual unsigned __int64 DoGetAiTreeInstanceNodeSize() = 0;
  virtual ArkAiTreeInstanceNode *DoEmplaceAiTreeInstanceNode(void *, ArkAiTreeInstanceNodeParent *, ArkAiTreeInstanceNode **, ArkAiTreeInstanceNode **) = 0;
  virtual void DoInitialize(const ArkAiTreeInitializeContext<ArkAiTreeNode> *) = 0;
  virtual void DoFinalize(const ArkAiTreeFinalizeContext<ArkAiTreeNode> *) = 0;
  virtual void DoEnable(const ArkAiTreeEnableResumedContext<ArkAiTreeNode> *) = 0;
  virtual void DoEnable(const ArkAiTreeEnableSuspendedContext<ArkAiTreeNode> *) = 0;
  virtual void DoDisable(const ArkAiTreeDisableResumedContext<ArkAiTreeNode> *) = 0;
  virtual void DoDisable(const ArkAiTreeDisableSuspendedContext<ArkAiTreeNode> *) = 0;
  virtual void DoActivate(const ArkAiTreeActivateResumedContext<ArkAiTreeNode> *) = 0;
  virtual void DoActivate(const ArkAiTreeActivateSuspendedContext<ArkAiTreeNode> *) = 0;
  virtual void DoDeactivate(const ArkAiTreeDeactivateResumedContext<ArkAiTreeNode> *) = 0;
  virtual void DoDeactivate(const ArkAiTreeDeactivateSuspendedContext<ArkAiTreeNode> *) = 0;
  virtual void DoSuspend(const ArkAiTreeSuspendActivatedContext<ArkAiTreeNode> *) = 0;
  virtual void DoSuspend(const ArkAiTreeSuspendEnabledContext<ArkAiTreeNode> *) = 0;
  virtual void DoSuspend(const ArkAiTreeSuspendInitializedContext<ArkAiTreeNode> *) = 0;
  virtual void DoResume(const ArkAiTreeResumeActivatedContext<ArkAiTreeNode> *) = 0;
  virtual void DoResume(const ArkAiTreeResumeEnabledContext<ArkAiTreeNode> *) = 0;
  virtual void DoResume(const ArkAiTreeResumeInitializedContext<ArkAiTreeNode> *) = 0;
  virtual void DoRequestActivation(const ArkAiTreeRequestActivationContext<ArkAiTreeNode> *) = 0;
  virtual void DoUpdate(const ArkAiTreeUpdateResumedContext<ArkAiTreeNode> *, float) = 0;
  virtual void DoUpdate(const ArkAiTreeUpdateSuspendedContext<ArkAiTreeNode> *, float) = 0;
  virtual void DoOnChildNodeActivated(const ArkAiTreeResumedOnChildNodeActivatedContext<ArkAiTreeNode> *, const ArkAiTreeNotifyActivatedContext<ArkAiTreeNode> *) = 0;
  virtual void DoOnChildNodeActivated(const ArkAiTreeSuspendedOnChildNodeActivatedContext<ArkAiTreeNode> *, const ArkAiTreeNotifyActivatedContext<ArkAiTreeNode> *) = 0;
  virtual void DoOnChildNodeCompleted(const ArkAiTreeResumedOnChildNodeCompletedContext<ArkAiTreeNode> *, const ArkAiTreeNotifyCompletedContext<ArkAiTreeNode> *, EArkAiTreeCompletionResult) = 0;
  virtual void DoOnChildNodeCompleted(const ArkAiTreeSuspendedOnChildNodeCompletedContext<ArkAiTreeNode> *, const ArkAiTreeNotifyCompletedContext<ArkAiTreeNode> *, EArkAiTreeCompletionResult) = 0;
  virtual void DoSerialize(const ArkAiTreeSerializeContext<ArkAiTreeNode> *, CSerializeWrapper<ISerialize>) = 0;
  virtual void DoPostSerialize(const ArkAiTreePostSerializeContext<ArkAiTreeNode> *) = 0;


  const unsigned __int64 m_traversalIndex;
  ArkAiTreeNodeParent *m_pParent;
  const ArkAiTreeNode **const m_ppChildrenBegin;
  const ArkAiTreeNode **const m_ppChildrenEnd;
};
