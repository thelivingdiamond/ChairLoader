class ArkInputLegendItem : ArkReflectedObject
{
  CCryName m_Action;
  CryStringT<char> m_ActionMap;
  CryStringT<char> m_Label;
  bool m_Hold;
  CryStringT<char> m_ComboString;
  std::vector<ArkInputAction> m_ActionList;
  ArkInputLegendItem::EArkLegendControlScheme m_ControlScheme;
  bool m_enabled;
  CryStringT<wchar_t> m_localizedLabel;
};
