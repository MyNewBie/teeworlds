#ifndef GAME_SERVER_GAMEMODES_CATCHING_H
#define GAME_SERVER_GAMEMODES_CATCHING_H
#include <game/server/gamecontroller.h>

class CGameControllerCatching : public IGameController
{
public:
	vec2 *m_pTeleporter;

	CGameControllerCatching(class CGameContext *pGameServer);
	~CGameControllerCatching();

	void InitTeleporter();

	virtual void Tick();
	virtual void OnPlayerInfoChange(class CPlayer *pP);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);

	virtual TeamStatistics TeamStatistic(int Team = -1, int BaseColor = -1);
	virtual int GetTeamNum();

	virtual int GetPlayersNum(int Team);
	virtual int GetJoinedPlayers();

	virtual bool IsColorUsed(int Color);
	virtual int GetColorOwner(int BaseColor);
	virtual bool IsWishUsed(int Color);

	virtual int GetTeamscore(int Team);

	virtual void PostReset();
	void DoPlayerNumWincheck();

	virtual bool IsCatching() const;
};
#endif
