#ifndef GAME_SERVER_ENTITIES_MOVING_H
#define GAME_SERVER_ENTITIES_MOVING_H

#include <game/server/entity.h>

const int PickupPhysSize = 14;

class CMoving : public CEntity
{
public:
	CMoving(CGameWorld *pGameWorld, int ClientID, vec2 Pos, int Type, int SubType = 0);
	
	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	
private:
	int m_Type;
	int m_Subtype;

	int m_Owner;
	int m_PathSize;
	int m_WayTick;
	int m_PowerupTime;
	int m_Hits;
	int m_HitTick;
	float SaturatedAdd(float Min, float Max, float Current, float Modifier);
	vec2 m_LastPos;
	array<vec2> m_lPath;
};
#endif
