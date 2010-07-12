#ifndef GAME_SERVER_GAMEMODES_CATCHING_H
#define GAME_SERVER_GAMEMODES_CATCHING_H
#include <game/server/gamecontroller.h>

class CGameControllerCatching : public IGameController
{
private:
	int m_IsInstagib;
	int m_IsHammerParty;
	int m_GiveWeapons;
	int m_Pickups;
	bool m_RoundRestart;

public:
	CGameControllerCatching(class CGameContext *pGameServer);
	virtual void Tick();
	virtual bool IsCatching() const;
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual void OnPlayerInfoChange(class CPlayer *pPlayer);
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual void PostReset();
	void DoPlayerNumWincheck();
};
#endif
