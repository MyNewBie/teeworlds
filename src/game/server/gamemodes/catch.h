#ifndef GAME_SERVER_GAMEMODES_CATCH_H
#define GAME_SERVER_GAMEMODES_CATCH_H
#include <game/server/gamecontroller.h>

class CGameControllerCatch : public IGameController
{
private:
	enum COLOR
	{
		BLACK			 = 0,
		WHITE			 = 255,
		ORANGE			 = 1507072,
		ORANGE_LIGHT	 = 1507201,
		BEIGE			 = 1834896,
		YELLOW			 = 2621184,
		YELLOW_LIGHT	 = 2686852,
		GREEN			 = 5018112,
		GREEN_LIGHT		 = 5018235,
		AQUA			 = 8453888,
		BLUE			 = 9830144,
		BLUE_LIGHT		 = 9830257,
		PURPLE			 = 12896518,
		PURPLE_LIGHT	 = 12896649,
		PINK			 = 15138560,
		RED_LIGHT		 = 16760679,
		RED				 = 16776960,
	};
public:
	CGameControllerCatch(class CGameContext *pGameServer);
	virtual void Tick();
	virtual void DoWincheck();
};
#endif
