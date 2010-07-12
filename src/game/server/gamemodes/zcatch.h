#ifndef GAME_SERVER_GAMEMODES_ZCatch_H
#define GAME_SERVER_GAMEMODES_ZCatch_H
#include <game/server/gamecontroller.h>

class CGameControllerZCatch : public IGameController
{
private:
	int m_IsInstagib;
	int m_IsHammerParty;
	int m_GiveWeapons;
	int m_Pickups;
	bool m_RoundRestart;

public:
	CGameControllerZCatch(class CGameContext *pGameServer);
	virtual void Tick();
	virtual bool IsZCatch() const;
	//virtual void StartRound();
	//virtual void EndRound();
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void PostReset();
	void DoPlayerNumWincheck();
	int GetLeaderID();
};
#endif
