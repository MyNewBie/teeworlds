// copyright (c) 2007 magnus auvinen, see licence.txt for more info
#include "catching.h"
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>


CGameControllerCatching::CGameControllerCatching(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	if(g_Config.m_SvInstagib)
		m_pGameType = "iCatch";
	else
		m_pGameType = "Catch";
}

void CGameControllerCatching::Tick()
{
	//DoPlayerScoreWincheck();
	DoPlayerNumWincheck();
	IGameController::Tick();
}

bool CGameControllerCatching::IsCatching() const
{
	return true;
}

bool CGameControllerCatching::OnEntity(int Index, vec2 Pos)
{
	if((Index == ENTITY_ARMOR_1 || Index == ENTITY_HEALTH_1) && (
		g_Config.m_SvHammerParty ||
		g_Config.m_SvInstagib ||
		!g_Config.m_SvPickups))
		return false;
	else if(
		(g_Config.m_SvHammerParty ||
		g_Config.m_SvInstagib ||
		g_Config.m_SvGiveWeapons) && (
		Index == ENTITY_WEAPON_SHOTGUN ||
		Index == ENTITY_WEAPON_GRENADE ||
		Index == ENTITY_WEAPON_RIFLE ||
		(Index == ENTITY_POWERUP_NINJA && !g_Config.m_SvInstagib)))
		return false;

	return IGameController::OnEntity(Index, Pos);
}
void CGameControllerCatching::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(10);
	
	// give default weapons
	if(g_Config.m_SvHammerParty)
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
	else if(g_Config.m_SvGiveWeapons)
	{
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		pChr->GiveWeapon(WEAPON_RIFLE, -1);
		pChr->GiveWeapon(WEAPON_GRENADE, -1);
		pChr->GiveWeapon(WEAPON_SHOTGUN, -1);
		pChr->GiveWeapon(WEAPON_GUN, 10);
	}
	else if(g_Config.m_SvInstagib)
		pChr->GiveWeapon(WEAPON_RIFLE, -1);
}

void CGameControllerCatching::OnPlayerInfoChange(class CPlayer *pPlayer)
{
	// Teamcolorwave \o/
	int black = 0,
		white = 255,
		red = 16776960,
		pink = 16760679,
		orange = 1507072,
		lorange = 1507201,
		lyellow = 2686852,
		yellow = 2621184,
		rpink = 15138560,
		green = 5018112,
		aqua = 8453888,
		blue = 9830144,
		lblue = 9830257,
		lgreen = 5018235,
		purple = 12896518,
		lpurple = 12896649;
	int TeamColors[MAX_CLIENTS] = {white, orange, aqua, rpink, yellow, green, red, blue, purple, black, pink, lpurple, lblue, lyellow, lorange, lgreen};
	if(pPlayer->m_BaseCatchingTeam != -1 && pPlayer->m_CatchingTeam == -1)
		pPlayer->m_CatchingTeam = pPlayer->m_BaseCatchingTeam;
	if(pPlayer->m_BaseCatchingTeam != -1 && (pPlayer->m_CatchingTeam >= 0 || pPlayer->m_CatchingTeam < MAX_CLIENTS))
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 1;
		pPlayer->m_TeeInfos.m_ColorBody = TeamColors[pPlayer->m_CatchingTeam];
		if(pPlayer->m_PrevCatchingTeam == -1)
			pPlayer->m_TeeInfos.m_ColorFeet = TeamColors[pPlayer->m_CatchingTeam];
		else
			pPlayer->m_TeeInfos.m_ColorFeet = TeamColors[pPlayer->m_PrevCatchingTeam];
	}
}
