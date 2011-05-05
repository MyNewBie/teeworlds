#ifndef GAME_SERVER_GAMEMODES_CATCHING_H
#define GAME_SERVER_GAMEMODES_CATCHING_H
#include <game/server/gamecontroller.h>

class CGameControllerCatching : public IGameController
{
private:
public:

	CGameControllerCatching(class CGameContext *pGameServer);
	virtual void Tick();
	virtual bool IsCatching() const;
	virtual void OnPlayerInfoChange(class CPlayer *pP);
	virtual TeamStatistics TeamStatistic(int Team = -1);
	void DoPlayerNumWincheck();
};
#endif
