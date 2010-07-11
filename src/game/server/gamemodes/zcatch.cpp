
#include "zcatch.h"
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>

CGameControllerZcatch::CGameControllerZcatch(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "zCatch";
}

void CGameControllerZcatch::Tick()
{
	DoPlayerScoreWincheck();
	IGameController::Tick();
}

bool CGameControllerZcatch::IsZcatch() const
{
	return true;
}

void CGameControllerZcatch::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(1);
	pChr->GetPlayer()->m_IsJoined = true;
	// give default weapons
	pChr->GiveWeapon(WEAPON_RIFLE, -1);
}

int CGameControllerZcatch::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	int ClientId = pVictim->GetPlayer()->GetCID();
	if(pKiller == pVictim->GetPlayer()) // suicide
	{
		int Num, NumPrev, LeaderID;
		NumPrev = 0;
		LeaderID = -1;
		for(int i=0; i<MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				Num = 0;
				for(int j=0; j<MAX_CLIENTS; j++)
				{
					if((GameServer()->m_apPlayers[j] && GameServer()->m_apPlayers[j]->m_CaughtBy == i))
						Num++;
				}
				if(Num > NumPrev)
				{
					LeaderID = i;
					NumPrev = Num;
				}
			}
		}
		if(LeaderID > -1)
		{
			pKiller->m_CaughtBy = LeaderID;
			pKiller->m_IsJoined = 0;
		}
	}
	else
	{
		char Buf[256];
		pKiller->m_Score++;
		GameServer()->m_apPlayers[ClientId]->m_CaughtBy = pKiller->GetCID();
		GameServer()->m_apPlayers[ClientId]->m_IsJoined = false;
		str_format(Buf, sizeof(Buf), "Caught by \"%s\"", Server()->ClientName(ClientId));
		GameServer()->SendChatTarget(ClientId, Buf);
	}
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			if(GameServer()->m_apPlayers[i]->m_CaughtBy == ClientId)
			{
				GameServer()->m_apPlayers[i]->m_CaughtBy = -1;
				GameServer()->m_apPlayers[i]->m_IsJoined = true;
				if(pKiller != pVictim->GetPlayer())
					pKiller->m_Score++;
			}
		}
	}
	return 0;
}
