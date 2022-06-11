#pragma once
class ArkProperty
{
public:
  void SetValue(ArkReflectedObject *const, const IArkValueBase *) = 0;
  virtual bool IsArray() = 0;
  virtual ArkReflectedObject *GetMemPtr(ArkReflectedObject *const) = 0;
  virtual void Reset(ArkReflectedObject *) = 0;


    CryStringT<char> m_name;
  const char *m_typeStr;
  ArkProperty::EArkType m_arkType;
};
