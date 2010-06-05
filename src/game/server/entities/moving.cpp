#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include "moving.h"
#include "laser.h"

CMoving::CMoving(CGameWorld *pGameWorld, int ClientID, vec2 Pos, int Type, int SubType)
: CEntity(pGameWorld, NETOBJTYPE_PICKUP)
{
	m_Type = Type;
	m_Subtype = SubType;
	m_ProximityRadius = PickupPhysSize;
	m_Pos = Pos;
	m_LastPos = Pos;
	m_PathSize = 0;
	m_lPath.set_size(GameServer()->Layers()->GameLayer()->m_Height*GameServer()->Layers()->GameLayer()->m_Width);
	m_PowerupTime = Server()->TickSpeed() * 60 * g_Config.m_SvPowerupTime;
	m_Owner = ClientID;
	m_Hits = 0;
	m_HitTick = 0;
	
	GameWorld()->InsertEntity(this);
}

void CMoving::Reset()
{
	GameWorld()->DestroyEntity(this);
	if(GameServer()->m_apPlayers[m_Owner])
		GameServer()->m_apPlayers[m_Owner]->m_NoBroadcast = 0;
}

void CMoving::Tick()
{
	m_PowerupTime--;

	CPlayer *pOwner = GameServer()->m_apPlayers[m_Owner];

	if(pOwner)
		pOwner->m_NoBroadcast = m_PowerupTime;
	if(!pOwner || m_PowerupTime <= 0)
	{
		GameServer()->CreateExplosion(m_Pos, -1, WEAPON_GRENADE, true);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		if(pOwner)
			pOwner->m_NoBroadcast = 0;
		GameWorld()->DestroyEntity(this);
		return;
	}
	if(m_PowerupTime)
	{
		int ClientID = m_Owner;
		int left = m_PowerupTime/Server()->TickSpeed();
		char Buf[128];
		if(left < 30 && g_Config.m_SvPowerupTime >= 2)
		{
			if(left == 29 && Server()->Tick()%(Server()->TickSpeed()) == 0)
				GameServer()->SendChatTarget(ClientID, "Get the Ninja to extend your Powerup");
			str_format(Buf, sizeof(Buf),  "Get The Ninja!\n%d Seconds left.", left);
		}
		else
			str_format(Buf, sizeof(Buf),  "\n%d Seconds left.", left);
		GameServer()->SendBroadcast(Buf, ClientID);
	}

	if(m_HitTick)
		m_HitTick--;
	CCharacter *pChar = GameServer()->m_World.ClosestTeamCharacter(m_Pos, 4000, (CEntity*) pOwner->GetCharacter(), pOwner->m_CatchingTeam, pOwner->m_BaseCatchingTeam);
	if(!pChar || distance(pChar->m_Pos, m_Pos) < 32) // If Player Hit
	{
		if(!pChar)
			return;
		if(!m_HitTick)
		{
			m_Hits++;
			GameServer()->CreateExplosion(m_Pos, pOwner->GetCID(), WEAPON_GAME, false, true);
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
			m_HitTick = Server()->TickSpeed()/2*g_Config.m_SvHitDelay;
			if(m_Hits >= g_Config.m_SvMaxHits)
				GameWorld()->DestroyEntity(this);
		}
		return;
	}

	if(!GameServer()->Collision()->IntersectLine2(pChar->m_Pos, m_Pos, 0x0, 0x0))
	{
		// Direction
		vec2 Direction = normalize(pChar->m_Pos-m_Pos);

		// Effects
		if(Server()->Tick()%(Server()->TickSpeed()/10) == 0 && g_Config.m_SvPowerupEffect)
		{
			GameServer()->CreateDeath(m_Pos, -1);
			if(g_Config.m_SvPowerupSound)
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA);
		}
	
		// move the heart
		const int Speed = g_Config.m_SvPowerupSpeed;
		m_LastPos = m_Pos;
		
		vec2 Vel = Direction * Speed;

		GameServer()->Collision()->MoveBox(&m_Pos, &Vel, vec2(PickupPhysSize, PickupPhysSize), 0.05f);

		return;
	}
	else if(Server()->Tick()-m_WayTick >= (Server()->TickSpeed()/2) || m_LastPos == m_Pos)
	{
		m_WayTick = Server()->Tick();

		GameServer()->m_pController->m_Path.Init();
		GameServer()->m_pController->m_Path.SetStart(m_Pos);
		GameServer()->m_pController->m_Path.SetEnd(pChar->m_Pos);
		GameServer()->m_pController->m_Path.FindPath();
		m_PathSize = GameServer()->m_pController->m_Path.m_FinalSize;
		// put found way to waypoints
		for(int i = m_PathSize-1, j = 0; i >= 0; i--, j++)
		{
			m_lPath[j] = vec2(GameServer()->m_pController->m_Path.m_lFinalPath[i].m_Pos.x*32+16, GameServer()->m_pController->m_Path.m_lFinalPath[i].m_Pos.y*32+16);
		}
		if(g_Config.m_SvShowWay)
		{
			for(int i = 0; i < m_PathSize; i++)
				new CLaser(&GameServer()->m_World, m_lPath[i], vec2(0,0), 10, -1);
		}
	}
	if(m_PathSize < 1)
		return;
	// get the direction along the way
	vec2 Direction;
	for(int  i = 0; i < m_PathSize &&  i < 10; i++)
	{
		if(GameServer()->Collision()->IntersectLine2(m_lPath[i], m_Pos, 0x0, 0x0)) // check for wall
		{
			Direction = normalize(m_lPath[i-1]-m_Pos);
			break;
		}
		else
		{
			Direction = normalize(m_lPath[i]-m_Pos);
		}
	}
	// Effects
	if(Server()->Tick()%(Server()->TickSpeed()/10) == 0 && g_Config.m_SvPowerupEffect)
	{
		GameServer()->CreateDeath(m_Pos, -1);
		if(g_Config.m_SvPowerupSound)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA);
	}
	
	// move the heart
	const int Speed = g_Config.m_SvPowerupSpeed;
	m_LastPos = m_Pos;
	
	vec2 Vel = Direction * Speed;

	GameServer()->Collision()->MoveBox(&m_Pos, &Vel, vec2(PickupPhysSize, PickupPhysSize), 0.05f);
}

void CMoving::Snap(int SnappingClient)
{
	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_Id, sizeof(CNetObj_Pickup)));
	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
	pP->m_Subtype = m_Subtype;
}
