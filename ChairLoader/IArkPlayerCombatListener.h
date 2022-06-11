#pragma once
class IArkPlayerCombatListener
{
public:
	virtual void OnCombatChange(IArkPlayerCombatListener* this, bool) = 0;
	virtual void OnGrenadeThrown(IArkPlayerCombatListener* this, IEntity* const) = 0;
};
