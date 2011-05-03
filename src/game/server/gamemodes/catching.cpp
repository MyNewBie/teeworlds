#include "catching.h"
#include <engine/shared/config.h>


CGameControllerCatching::CGameControllerCatching(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
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

