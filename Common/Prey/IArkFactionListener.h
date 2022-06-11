#pragma once
class IArkFactionListener
{
public:
  void OnAssignedFactionToFactionDispositionChange(unsigned int, unsigned int, EArkDisposition, EArkDisposition) = 0;
  virtual void OnAssignedFactionToEntityDispositionChange(unsigned int, unsigned int, EArkDisposition, EArkDisposition) = 0;
  virtual void OnAssignedEntityToFactionDispositionChange(unsigned int, unsigned int, EArkDisposition, EArkDisposition) = 0;
  virtual void OnAssignedEntityToEntityDispositionChange(unsigned int, unsigned int, EArkDisposition, EArkDisposition) = 0;
  virtual void OnEffectiveEntityToEntityDispositionChange(unsigned int, unsigned int, EArkDisposition, EArkDisposition) = 0;


  };
