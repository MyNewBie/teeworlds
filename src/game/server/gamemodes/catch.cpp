#include "catch.h"
#include <engine/shared/config.h>
#include <base/system.h>

CGameControllerCatch::CGameControllerCatch(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	if (!str_comp_nocase(g_Config.m_SvGametype, "icatch"))
		// ol' instagib
		m_pGameType = "iCATCH";
	else if (!str_comp_nocase(g_Config.m_SvGametype, "gcatch"))
		// grenade instagib
		m_pGameType = "gCATCH";
	else
		// vanilla weapons
		m_pGameType = "CATCH";
}


bool CGameControllerCatch::IsCatch() { return true; }

bool CGameControllerCatch::IsInsta() { return m_pGameType == "iCATCH" || m_pGameType == "gCATCH"; }

bool CGameControllerCatch::IsGInsta() { return m_pGameType == "gCATCH"; }

void CGameControllerCatch::DoWincheck()
{
	//
}

void CGameControllerCatch::Tick()
{
	IGameController::Tick();
}
