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
	m_Vel = vec2(0, 0);
	
	GameWorld()->InsertEntity(this);
}

void CMoving::Reset()
{
	GameWorld()->DestroyEntity(this);
	if(GameServer()->m_apPlayers[m_Owner])
		GameServer()->m_apPlayers[m_Owner]->m_NoBroadcast = 0;
}

float CMoving::SaturatedAdd(float Min, float Max, float Current, float Modifier)
{
	if(Modifier < 0)
	{
		if(Current < Min)
			return Current;
		Current += Modifier;
		if(Current < Min)
			Current = Min;
		return Current;
	}
	else
	{
		if(Current > Max)
			return Current;
		Current += Modifier;
		if(Current > Max)
			Current = Max;
		return Current;
	}
}

void CMoving::Tick()
{
	m_PowerupTime--;

	// effekte und wann es zerstört wird, zuordnung und so
	CPlayer *pOwner = GameServer()->m_apPlayers[m_Owner];
	if(pOwner)
		pOwner->m_NoBroadcast = m_PowerupTime;
	if(!pOwner || m_PowerupTime <= 0)
	{
		GameServer()->CreateExplosion(m_Pos, -1, WEAPON_POWERUP, true);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, CmaskCatch(GameServer(), m_Owner));
		if(pOwner)
			pOwner->m_NoBroadcast = 0;
		GameWorld()->DestroyEntity(this);
		return;
	}

	// Effects
	if(Server()->Tick()%(Server()->TickSpeed()/10) == 0 && g_Config.m_SvPowerupEffect)
	{
		GameServer()->CreateDeath(m_Pos, -1);
		if(g_Config.m_SvPowerupSound)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA, CmaskCatch(GameServer(), m_Owner));
	}

	if(m_PowerupTime) // Broadcast overlap with catching messages // Need fixes
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

	// wegfinden bzw setzen des ziels
	CCharacter *pChar = GameServer()->m_World.ClosestTeamCharacter(m_Pos, 4000, (CEntity*) pOwner->GetCharacter(), pOwner->m_CatchingTeam, pOwner->m_BaseCatchingTeam);

	vec2 Direction;
	const int Speed = g_Config.m_SvPowerupSpeed;
	if(pChar && !GameServer()->Collision()->IntersectLine2(pChar->m_Pos, m_Pos, 0x0, 0x0))
	{
		// Direction
		Direction = normalize(pChar->m_Pos-m_Pos);
	
		// move the heart
		const int Speed = g_Config.m_SvPowerupSpeed;
		m_LastPos = m_Pos;
		
		vec2 m_Vel = Direction * Speed;

		GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(PickupPhysSize, PickupPhysSize), 0.05f);

		return;
	}
	else if(pChar && (Server()->Tick()-m_WayTick >= (Server()->TickSpeed()/2) || m_LastPos == m_Pos))
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

	// get the direction along the way
	for(int i = 0; i < m_PathSize &&  i < 10; i++)
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


	// äußere einflüsse
	m_Vel = Direction * Speed;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetCharacter())
		{
			if(distance(GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_HookPos, m_Pos) < 48 &&
				distance(GameServer()->m_apPlayers[i]->GetCharacter()->m_Pos, m_Pos) > 52) 
			{
				// NEED FIXES (!)
				GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_HookPos = m_Pos;
				GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_HookState = HOOK_GRABBED;
				GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_HookedPowerup = true;

				float d = distance(GameServer()->m_apPlayers[i]->GetCharacter()->m_Pos, m_Pos);
					
				if(d > CCharacter::ms_PhysSize*1.50f)
				{
					vec2 Dir = normalize(GameServer()->m_apPlayers[i]->GetCharacter()->m_Pos - m_Pos);
					float Accel = CWorldCore().m_Tuning.m_HookDragAccel * (d/CWorldCore().m_Tuning.m_HookLength);
					float DragSpeed = CWorldCore().m_Tuning.m_HookDragSpeed;
				
					// add force to the heart
					m_Vel.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, Accel*Dir.x*1.5f);
					m_Vel.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, Accel*Dir.y*1.5f);
	
					// add a little bit force to the guy who has the grip
					GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_Vel.x -= SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel*Dir.x*1.25f);
					GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_Vel.y -= SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel*Dir.y*1.25f);
				}
			}
			else
				GameServer()->m_apPlayers[i]->GetCharacter()->m_Core.m_HookedPowerup = false;
		}
	}

	/*if(!pChar || distance(pChar->m_Pos, m_Pos) < 32) // If Player Hit
	{
		if(!pChar)
		{
			//if(pOwner->GetCharacter()->m_Core.m_HookState == HOOK_GRABBED)
				GameServer()->Collision()->MoveBox(&m_Pos, &AddVel, vec2(PickupPhysSize, PickupPhysSize), 0.05f);
			return;
		}
		if(!m_HitTick)
		{
			m_Hits++;
			GameServer()->CreateExplosion(m_Pos, pOwner->GetCID(), WEAPON_POWERUP, false);
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
			m_HitTick = Server()->TickSpeed()/2*g_Config.m_SvHitDelay;
			if(m_Hits >= g_Config.m_SvMaxHits)
				GameWorld()->DestroyEntity(this);
		}
		return;
	}*/

	//if(m_PathSize < 1)
		//return;
	
	// move the heart
	m_LastPos = m_Pos;

	GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(PickupPhysSize, PickupPhysSize), 0.05f);
}

void CMoving::Snap(int SnappingClient)
{
	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_Id, sizeof(CNetObj_Pickup)));
	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
	pP->m_Subtype = m_Subtype;
}
