
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
	DoPlayerNumWincheck();
	DoPlayerScoreWincheck();
	IGameController::Tick();
}

bool CGameControllerZCatch::IsZCatch() const
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
	pChr->IncreaseHealth(1);
	pChr->GetPlayer()->m_IsJoined = true;
	// give default weapons
	pChr->GiveWeapon(WEAPON_RIFLE, -1);
}

int CGameControllerZCatch::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	int ClientID = pVictim->GetPlayer()->GetCID();
	if(pKiller == pVictim->GetPlayer()) // suicide
	{
		int LeaderID;
		LeaderID = GetLeaderID();
		if(LeaderID > -1)
		{
			GameServer()->m_apPlayers[LeaderID]->m_Caught |= 1 << ClientID;
		}
	}
	else
	{
		char aBuf[512];
		GameServer()->m_apPlayers[pKiller->GetCID()]->m_Caught |= 1 << ClientID;
		str_format(aBuf, sizeof(aBuf), "Caught by \"%s\"", Server()->ClientName(ClientID));
		GameServer()->SendChatTarget(ClientID, aBuf);
	}
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			if(GameServer()->m_apPlayers[ClientID]->m_Caught&(1<<i))
			{
				GameServer()->m_apPlayers[i]->m_IsJoined = true;
			}
		}
	}
	GameServer()->m_apPlayers[ClientID]->m_Caught = 0;
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
	int Num, LeaderID, NumPrev = -1;
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
