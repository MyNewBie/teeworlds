#include "catch.h"

CGameControllerCatch::CGameControllerCatch(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "CATCH";
}

void CGameControllerCatch::DoWincheck()
{
	//
}

void CGameControllerCatch::Tick()
{
	IGameController::Tick();
}
