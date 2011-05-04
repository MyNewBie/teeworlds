#include "catching.h"
#include <engine/shared/config.h>


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
	DoPlayerScoreWincheck();
	IGameController::Tick();
}

bool CGameControllerCatching::IsCatching() const
{
	return true;
}

