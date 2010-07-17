
#include "zcatch.h"
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

CGameControllerZCatch::CGameControllerZCatch(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	if(g_Config.m_SvInstagib)
		m_pGameType = "izCatch";
	else
		m_pGameType = "zCatch";
	
	m_IsInstagib = g_Config.m_SvInstagib;
	m_IsHammerParty = g_Config.m_SvHammerParty;
	m_GiveWeapons = g_Config.m_SvGiveWeapons;
	m_Pickups = g_Config.m_SvPickups;
	m_RoundRestart = false;
}

void CGameControllerZCatch::Tick()
{
	// Reload Map
	if((g_Config.m_SvInstagib != m_IsInstagib ||
		g_Config.m_SvHammerParty != m_IsHammerParty ||
		g_Config.m_SvGiveWeapons != m_GiveWeapons ||
		g_Config.m_SvPickups != m_Pickups))
		GameServer()->Console()->ExecuteLine("reload");
	
	DoPlayerNumWincheck();
	DoPlayerScoreWincheck();
	IGameController::Tick();
}

bool CGameControllerZCatch::IsZCatch() const
{
	return true;
}

bool CGameControllerZCatch::JoiningSystem() const
{
	return true;
}

void CGameControllerZCatch::DoPlayerNumWincheck()
{
	if(m_GameOverTick == -1  && !m_Warmup)
	{
		// DRUEBER NACHDENKEN - STICHWORT SUSHI MASKE
		/*int Num, NumPrev = 0, LeaderID = -1;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				Num = 0;
				for(int j = 0; j < MAX_CLIENTS; j++)
					if(GameServer()->m_apPlayers[j])
						if(GameServer()->m_apPlayers[j]->CaughtBy == i)
							Num++;
				if(Num > NumPrev
			}
		}*/
	}
}
bool CGameControllerZCatch::OnEntity(int Index, vec2 Pos)
{
	if((Index == ENTITY_ARMOR_1 || Index == ENTITY_HEALTH_1) && (
		g_Config.m_SvHammerParty ||
		g_Config.m_SvInstagib ||
		!g_Config.m_SvPickups))
		return false;
	else if(
		(g_Config.m_SvHammerParty ||
		g_Config.m_SvInstagib ||
		g_Config.m_SvGiveWeapons) && (
		Index == ENTITY_WEAPON_SHOTGUN ||
		Index == ENTITY_WEAPON_GRENADE ||
		Index == ENTITY_WEAPON_RIFLE ||
		(Index == ENTITY_POWERUP_NINJA && !g_Config.m_SvInstagib)))
		return false;

	return IGameController::OnEntity(Index, Pos);
}

void CGameControllerZCatch::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(10);
	
	// give default weapons
	if(g_Config.m_SvInstagib)
	{
		pChr->GiveWeapon(WEAPON_RIFLE, -1);
		pChr->SetWeapon(WEAPON_RIFLE);
	}
	else
	{
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		pChr->GiveWeapon(WEAPON_GUN, 10);
	}
}

int CGameControllerZCatch::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	int VictimID = pVictim->GetPlayer()->GetCID(),
		KillerID = pKiller->GetCID();
	if(pKiller == pVictim->GetPlayer()) // suicide
	{
		int LeaderID;
		LeaderID = GetLeaderID();
		if(LeaderID > -1)
		{
			GameServer()->m_apPlayers[LeaderID]->m_Caught |= 1 << VictimID;
		}
	}
	else
	{
		char aBuf[512];
		GameServer()->m_apPlayers[KillerID]->m_Caught |= 1 << VictimID;
		str_format(aBuf, sizeof(aBuf), "Caught by \"%s\" (%d)",
			Server()->ClientName(KillerID), pKiller->m_IsJoined);
		GameServer()->SendChatTarget(VictimID, aBuf);
		str_format(aBuf, sizeof(aBuf), "You caught \"%s\" (%d)",
			Server()->ClientName(VictimID), pVictim->GetPlayer()->m_IsJoined);
		GameServer()->SendChatTarget(KillerID, aBuf);
	}
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			if(GameServer()->m_apPlayers[VictimID]->m_Caught&(1<<i))
			{
				GameServer()->m_apPlayers[i]->m_IsJoined = true;
			}
		}
	}
	pVictim->GetPlayer()->m_Caught = 0;
	pVictim->GetPlayer()->m_IsJoined = false;
	return 0;
}

void CGameControllerZCatch::PostReset()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->Respawn();
			GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
			GameServer()->m_apPlayers[i]->m_Caught = 0;
		}
	}
}

int CGameControllerZCatch::GetLeaderID()
{
	int Num, LeaderID = -1, NumPrev = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			Num = 0;
			for(int j = 0; j < MAX_CLIENTS; j++)
			{
				if(GameServer()->m_apPlayers[j])
					if(GameServer()->m_apPlayers[i]->m_Caught&(1<<j))
						Num++;
			}
			if(Num > NumPrev)
			{
				LeaderID = i;
				NumPrev = Num;
			}
			else if(Num == NumPrev)
				LeaderID = -1;
		}
	}
	return LeaderID;
}
