class ArkCameraShake : ArkReflectedObject
{
public:
  EArkCameraShakeType GetCameraShakeType() = 0;


    unsigned __int64 m_ID;
  CryStringT<char> m_Name;
  unsigned __int64 m_Channel;
  float m_InnerRadius;
  float m_OuterRadius;
  float m_UIHorizontalAmplitude;
  float m_UIVerticalAmplitude;
  float m_RampInDuration;
  float m_HoldDuration;
  float m_RampOutDuration;
  float m_VerticalFrequency;
  float m_HorizontalFrequency;
  CryStringT<char> m_ForceFeedback;
};
