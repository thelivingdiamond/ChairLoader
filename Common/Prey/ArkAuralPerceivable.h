#pragma once
class ArkAuralPerceivable
{
public:
  virtual unsigned int DoGetEntityId() = 0;
  virtual bool DoGetAuralPerceivableIsInVacuum() = 0;
  virtual bool DoGetAuralPerceivableIsComplexAttentionObject() = 0;


  };
