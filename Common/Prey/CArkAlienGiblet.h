#pragma once
class CArkAlienGiblet : CGameObjectExtensionHelper<CArkAlienGiblet,IGameObjectExtension,64>, IArkPlayerInteractionListener
{
public:


  unsigned int m_ownerId;
  unsigned int m_masterGibId;
  ArkSimpleTimer m_interactTimer;
  ArkSimpleTimer m_gibFlingTimer;
  bool m_bHasFlingQueued;
  Vec3_tpl<float> m_fling;
  bool m_bIsArticulated;
  unsigned __int64 m_characterId;
  EArkGibletType m_type;
};
