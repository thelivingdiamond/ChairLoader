#pragma once
class CBaseInput : IInput, ISystemEventListener
{
public:


  std::vector<SInputSymbol *> m_holdSymbols;
  std::list<IInputEventListener *> m_listeners;
  std::list<IInputEventListener *> m_consoleListeners;
  IInputEventListener *m_pExclusiveListener;
  CListenerSet<ITouchEventListener *> m_touchListeners;
  bool m_enableEventPosting;
  bool m_retriggering;
  CryLockT<2> m_postInputEventMutex;
  bool m_hasFocus;
  std::vector<IInputDevice *> m_inputDevices;
  int m_forceFeedbackDeviceIndex;
  std::list<SInputBlockData> m_inputBlockData;
  int m_modifiers;
  CInputCVars *m_pCVars;
  CKinectInputNULL *m_pKinectInput;
  CNaturalPointInputNull *m_pNaturalPointInput;
  unsigned int m_platformFlags;
};
