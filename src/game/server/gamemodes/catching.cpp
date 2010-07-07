// copyright (c) 2007 magnus auvinen, see licence.txt for more info
#include "catching.h"
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>


CGameControllerCatching::CGameControllerCatching(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	if(g_Config.m_SvInstagib)
		m_pGameType = "iCatch";
	else
		m_pGameType = "Catch";
}

void CGameControllerCatching::Tick()
{
	//DoPlayerScoreWincheck();
	DoPlayerNumWincheck();
	IGameController::Tick();
}

bool CGameControllerCatching::IsCatching() const
{
	return true;
}

void CGameControllerCatching::OnPlayerInfoChange(class CPlayer *pPlayer)
{
	// Teamcolorwave \o/
	int black = 0,
		white = 255,
		red = 16776960,
		pink = 16760679,
		orange = 1507072,
		lorange = 1507201,
		lyellow = 2686852,
		yellow = 2621184,
		rpink = 15138560,
		green = 5018112,
		aqua = 8453888,
		blue = 9830144,
		lblue = 9830257,
		lgreen = 5018235,
		purple = 12896518,
		lpurple = 12896649;
	int TeamColors[MAX_CLIENTS] = {white, orange, aqua, rpink, yellow, green, red, blue, purple, black, pink, lpurple, lblue, lyellow, lorange, lgreen};
	if(pPlayer->m_BaseCatchingTeam != -1 && pPlayer->m_CatchingTeam == -1)
		pPlayer->m_CatchingTeam = pPlayer->m_BaseCatchingTeam;
	if(pPlayer->m_BaseCatchingTeam != -1 && (pPlayer->m_CatchingTeam >= 0 || pPlayer->m_CatchingTeam < MAX_CLIENTS))
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 1;
		pPlayer->m_TeeInfos.m_ColorBody = TeamColors[pPlayer->m_CatchingTeam];
		if(pPlayer->m_PrevCatchingTeam == -1)
			pPlayer->m_TeeInfos.m_ColorFeet = TeamColors[pPlayer->m_CatchingTeam];
		else
			pPlayer->m_TeeInfos.m_ColorFeet = TeamColors[pPlayer->m_PrevCatchingTeam];
	}
}
