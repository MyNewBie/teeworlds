
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
	int LeaderID = GetLeaderID(), Num = 0, Total = 0;
	char aBuf[512];
	if(m_GameOverTick == -1  && !m_Warmup)
	{
		if(LeaderID > -1)
		{
			for(int i=0; i<MAX_CLIENTS; i++)
			{
				if(GameServer()->m_apPlayers[i])
				{
					Total++;
					if(GameServer()->m_apPlayers[i]->m_CaughtBy == LeaderID)
					{
						Num++;
					}
				}
			}
			if(Total > 1 && Num == Total - 1)
			{
				GameServer()->m_apPlayers[LeaderID]->m_Score++;
				str_format(aBuf, sizeof(aBuf), "%s remained, congratulations!", Server()->ClientName(LeaderID));
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				EndRound();
			}
		}
	}
}

void CGameControllerZCatch::DoPlayerScoreWincheck()
{
	if(m_GameOverTick == -1  && !m_Warmup)
	{
		// gather some stats
		int Topscore = 0;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				if(GameServer()->m_apPlayers[i]->m_Score > Topscore)
					Topscore = GameServer()->m_apPlayers[i]->m_Score;
			}
		}
		
		// check score win condition
		if((g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit) ||
			(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
		{
			m_RoundRestart = true;
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "Scorelimit reached, starting new Round...");
			EndRound();
		}
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
		(Index == ENTITY_POWERUP_NINJA && g_Config.m_SvInstagib)))
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
			pVictim->GiveWeapon(WEAPON_HAMMER, 0);
			pVictim->SetWeapon(WEAPON_HAMMER);
			pVictim->GetPlayer()->m_CaughtBy = LeaderID;
			pVictim->GetPlayer()->m_IsJoined = false;
		}
	}
	else if(pKiller->m_IsJoined)
	{
		char aBuf[512];
		pVictim->GiveWeapon(WEAPON_HAMMER, 0);
		pVictim->SetWeapon(WEAPON_HAMMER);
		pVictim->GetPlayer()->m_IsJoined = false;
		pVictim->GetPlayer()->m_CaughtBy = KillerID;
		
		str_format(aBuf, sizeof(aBuf), "Caught by \"%s\" (%d)",
			Server()->ClientName(KillerID), pKiller->m_IsJoined);
		GameServer()->SendChatTarget(VictimID, aBuf);
		str_format(aBuf, sizeof(aBuf), "You caught \"%s\" (%d)",
			Server()->ClientName(VictimID), pVictim->GetPlayer()->m_IsJoined);
		GameServer()->SendChatTarget(KillerID, aBuf);
	}
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			if(GameServer()->m_apPlayers[i]->m_CaughtBy == VictimID)
			{
				GameServer()->m_apPlayers[i]->m_IsJoined = true;
				GameServer()->m_apPlayers[i]->m_CaughtBy = -1;
				GameServer()->m_apPlayers[i]->GetCharacter()->GiveWeapon(WEAPON_HAMMER, -1);
			}
		}
	}
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
			GameServer()->m_apPlayers[i]->m_CaughtBy = -1;
			GameServer()->m_apPlayers[i]->m_IsJoined = true;
			if(m_RoundRestart)
				GameServer()->m_apPlayers[i]->m_Score = 0;
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
					if(GameServer()->m_apPlayers[j]->m_CaughtBy == i)
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

void CGameControllerZCatch::SetColor(class CPlayer *pP)
{
	pP->m_TeeInfos.m_UseCustomColor = 1;
	if(!pP->m_IsJoined)
	{
		pP->m_TeeInfos.m_ColorBody = 255;
		pP->m_TeeInfos.m_ColorFeet = 255;
	}
	else
	{
		int Caught = 161;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				if(GameServer()->m_apPlayers[i]->m_CaughtBy == pP->GetCID())
				{
					Caught -= 20;
				}
			}
		}
		pP->m_TeeInfos.m_ColorBody = 0xff00 + Caught * 0x010000;
		pP->m_TeeInfos.m_ColorFeet = 0xff00 + Caught * 0x010000;
	}
}
