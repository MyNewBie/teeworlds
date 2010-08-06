#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "projectile.h"
#include <game/server/entities/pickup.h>

CProjectile::CProjectile(CGameWorld *pGameWorld, int Type, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, bool Explosive, float Force, int SoundImpact, int Weapon)
: CEntity(pGameWorld, NETOBJTYPE_PROJECTILE)
{
	m_Type = Type;
	m_Pos = Pos;
	m_Direction = Dir;
	m_LifeSpan = Span;
	m_Owner = Owner;
	m_Force = Force;
	m_Damage = Damage;
	m_SoundImpact = SoundImpact;
	m_Weapon = Weapon;
	m_StartTick = Server()->Tick();
	m_Explosive = Explosive;

	GameWorld()->InsertEntity(this);
}

void CProjectile::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CProjectile::GetPos(float Time)
{
	float Curvature = 0;
	float Speed = 0;
	
	switch(m_Type)
	{
		case WEAPON_GRENADE:
			Curvature = GameServer()->Tuning()->m_GrenadeCurvature;
			Speed = GameServer()->Tuning()->m_GrenadeSpeed;
			break;
			
		case WEAPON_SHOTGUN:
			Curvature = GameServer()->Tuning()->m_ShotgunCurvature;
			Speed = GameServer()->Tuning()->m_ShotgunSpeed;
			break;
			
		case WEAPON_GUN:
			Curvature = GameServer()->Tuning()->m_GunCurvature;
			Speed = GameServer()->Tuning()->m_GunSpeed;
			break;
	}
	
	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}


void CProjectile::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, &CurPos, 0);
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, CurPos, OwnerChar);

	m_LifeSpan--;
	
	if(TargetChr || Collide || m_LifeSpan < 0)
	{
		
		if(m_LifeSpan >= 0 || m_Weapon == WEAPON_GRENADE)
		{
			if((GameServer()->m_pController->JoiningSystem() && TargetChr && GameServer()->m_pController->CheckJoined(TargetChr, OwnerChar)) ||
				!GameServer()->m_pController->JoiningSystem() || (GameServer()->m_pController->JoiningSystem() && !TargetChr))
				GameServer()->CreateSound(CurPos, m_SoundImpact, CmaskCatch(GameServer(), m_Owner));
		}

		if(m_Explosive)
		{
			if((GameServer()->m_pController->JoiningSystem() && TargetChr && GameServer()->m_pController->CheckJoined(TargetChr, OwnerChar)) ||
				!GameServer()->m_pController->JoiningSystem() || (GameServer()->m_pController->JoiningSystem() && !TargetChr))
				GameServer()->CreateExplosion(CurPos, m_Owner, m_Weapon, false);
		}
			
		else if(TargetChr)
		{
			if((GameServer()->m_pController->JoiningSystem() && GameServer()->m_pController->CheckJoined(TargetChr, OwnerChar)) ||
				!GameServer()->m_pController->JoiningSystem())
				TargetChr->TakeDamage(m_Direction * max(0.001f, m_Force), m_Damage, m_Owner, m_Weapon);
			else
				return;
		}

		//CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_HEALTH, 0);
		//pPickup->m_Pos = PrevPos;
		if((GameServer()->m_pController->JoiningSystem() && TargetChr && GameServer()->m_pController->CheckJoined(TargetChr, OwnerChar)) ||
			!GameServer()->m_pController->JoiningSystem() || (GameServer()->m_pController->JoiningSystem() && !TargetChr))
			GameServer()->m_World.DestroyEntity(this);
	}

	int z = GameServer()->Collision()->IsTeleport(GameServer()->Collision()->GetIndex(PrevPos, CurPos));
  	if(g_Config.m_SvTeleport && z)
  	{
		if(g_Config.m_SvTeleportGrenade && m_Weapon == WEAPON_GRENADE)
		{
 			m_Pos = (GameServer()->m_pController)->m_pTeleporter[z-1];
  			m_StartTick = Server()->Tick();
		}
	}
}

void CProjectile::FillInfo(CNetObj_Projectile *pProj)
{
	pProj->m_X = (int)m_Pos.x;
	pProj->m_Y = (int)m_Pos.y;
	pProj->m_VelX = (int)(m_Direction.x*100.0f);
	pProj->m_VelY = (int)(m_Direction.y*100.0f);
	pProj->m_StartTick = m_StartTick;
	pProj->m_Type = m_Type;
}

void CProjectile::Snap(int SnappingClient)
{
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	
	if(NetworkClipped(SnappingClient, GetPos(Ct)))
		return;
	if(GameServer()->m_pController->JoiningSystem() && !GameServer()->m_apPlayers[m_Owner]->m_IsJoined && GameServer()->m_apPlayers[SnappingClient]->m_IsJoined)
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_Id, sizeof(CNetObj_Projectile)));
	FillInfo(pProj);
}
