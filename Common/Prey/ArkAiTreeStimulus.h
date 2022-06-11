#pragma once
class ArkAiTreeStimulus
{
public:
  virtual ~ArkAiTreeStimulus() = 0;
  virtual void Serialize(CSerializeWrapper<ISerialize>) = 0;


    const char *m_pClassName;
};
