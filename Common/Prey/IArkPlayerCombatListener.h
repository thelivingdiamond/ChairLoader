class IArkPlayerCombatListener
{
public:
  void OnCombatChange(bool) = 0;
  virtual void OnGrenadeThrown(IEntity *const) = 0;


  };
