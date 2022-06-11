#pragma once
class ArkFocusModeUIComponent : ArkUIMenuBase<ArkFocusModeUIComponent>, IUIControlSchemeListener
{
public:
  void OnUIEvent(IUIElementEventListener *this, IUIElement *, const SUIEventDesc *, const SUIArguments *) = 0;


  std::vector<ArkFocusModeMenuPower> m_menuPowers;
  std::vector<ArkFocusModeMenuWeapon> m_menuWeapons;
  ArkInputLegendHandler m_powerInputLegendHandler;
  ArkInputLegendHandler m_weaponInputLegendHandler;
  unsigned int m_emptyLegendHandle;
  Vec2_tpl<float> m_menuInputVec;
  Vec2_tpl<float> m_prevMenuInputVec;
  float m_currentMenuAngle;
  float m_desiredMenuAngle;
  float m_menuAngularSpeed;
  const ArkFocusModeComponentProperties *m_properties;
  ArkFocusModeUIComponent::ArkActiveControllerStick m_activeControllerStick;
  int m_selectedItemIndex;
  bool m_bVisible;
  std::vector<unsigned __int64> m_foodArchetypes;
};
