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
	virtual TeamStatistics TeamStatistic(int Team = -1, int BaseColor = -1);
		virtual int GetTeamNum();
		virtual int GetPlayersNum(int Team);
		virtual int GetJoinedPlayers();
		virtual bool IsColorUsed(int Color);
		virtual int GetColorOwner(int BaseColor);
	void DoPlayerNumWincheck();
};
#endif
