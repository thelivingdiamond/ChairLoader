class SActionMapEvent
{
	enum class EActionMapManagerEvent : __int32
	{
		eActionMapManagerEvent_ActionMapsInitialized = 0x0,
		eActionMapManagerEvent_DefaultActionEntityChanged = 0x1,
		eActionMapManagerEvent_FilterStatusChanged = 0x2,
		eActionMapManagerEvent_ActionMapStatusChanged = 0x3,
	};
  SActionMapEvent::EActionMapManagerEvent event;
  unsigned __int64 wparam;
  unsigned __int64 lparam;
};
