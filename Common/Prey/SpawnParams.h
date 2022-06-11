#pragma once
class SpawnParams
{
  EGeomType eAttachType;
  EGeomForm eAttachForm;
  bool bCountPerUnit;
  bool bPrime;
  bool bRegisterByBBox;
  bool bNowhere;
  bool bUseUITimer;
  bool bForceVAClip;
  float fCountScale;
  float fSizeScale;
  float fSpeedScale;
  float fTimeScale;
  float fPulsePeriod;
  float fStrength;
  bool bEnableAudio;
  CryStringT<char> audioRtpc;
};
