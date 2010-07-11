#ifndef GAME_SERVER_GAMEMODES_CATCHING_H
#define GAME_SERVER_GAMEMODES_CATCHING_H
#include <game/server/gamecontroller.h>

class CGameControllerCatching : public IGameController
{
public:
	CGameControllerCatching(class CGameContext *pGameServer);
	virtual void Tick();
	virtual bool IsCatching() const;
	bool OnEntity(int Index, vec2 Pos);
	void OnPlayerInfoChange(class CPlayer *pPlayer);
	void OnCharacterSpawn(class CCharacter *pChr);
};
#endif
