#ifndef GAME_SERVER_GAMEMODES_ZCATCH_H
#define GAME_SERVER_GAMEMODES_ZCATCH_H
#include <game/server/gamecontroller.h>

class CGameControllerZcatch : public IGameController
{
public:
	CGameControllerZcatch(class CGameContext *pGameServer);
	virtual void Tick();
	virtual bool IsZcatch() const;
	//void StartRound();
	//void EndRound();
	void OnCharacterSpawn(class CCharacter *pChr);
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
};
#endif
