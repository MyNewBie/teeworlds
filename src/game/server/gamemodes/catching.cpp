#include "catching.h"
#include <engine/shared/config.h>
#include "../gamecontext.h"

CGameControllerCatching::CGameControllerCatching(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	if(!str_comp(g_Config.m_SvGametype, "icatch"))
		m_pGameType = "iCatch";
	else
		m_pGameType = "Catch";
}

void CGameControllerCatching::Tick()
{
	DoPlayerNumWincheck();
	IGameController::Tick();
}

bool CGameControllerCatching::IsCatching() const
{
	return true;
}

void CGameControllerCatching::OnPlayerInfoChange(class CPlayer *pP)
{
}

TeamStatistics CGameControllerCatching::TeamStatistic(int Team)
{
	/*
		How many teams?
		How many players per team (int teamid)
		How many players are joined?
		Is (Base)Color used?
	*/

	int TeamNum = 0;
	int PlayerNum = 0;
	int PlayerJoined = 0;
	bool IsUsed = false;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			// How many teams
			if(GameServer()->m_apPlayers[i]->GetBaseTeam() > -1)
				TeamNum++;

			// How many players per team
			if(GameServer()->m_apPlayers[i]->GetCurrentTeam() == Team && Team > -1)
				PlayerNum++;
			
			// How many players are joined
			if(GameServer()->m_apPlayers[i]->IsJoined())
				PlayerJoined++;

			// Is (Base)Color used
			if(GameServer()->m_apPlayers[i]->GetBaseTeam() == Team)
				IsUsed = true;
		}
	}
	
	TeamStatistics Return = {
		TeamNum,
		PlayerNum,
		PlayerJoined,
		IsUsed
	};
	return Return;
}

void CGameControllerCatching::DoPlayerNumWincheck()
{
}
