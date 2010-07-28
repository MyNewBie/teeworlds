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
	
	m_IsInstagib = g_Config.m_SvInstagib;
	m_IsHammerParty = g_Config.m_SvHammerParty;
	m_GiveWeapons = g_Config.m_SvGiveWeapons;
	m_Pickups = g_Config.m_SvPickups;
	m_RoundRestart = false;
}

void CGameControllerCatching::Tick()
{
	DoPlayerNumWincheck();
	// Reload Map
	if((g_Config.m_SvInstagib != m_IsInstagib ||
		g_Config.m_SvHammerParty != m_IsHammerParty ||
		g_Config.m_SvGiveWeapons != m_GiveWeapons ||
		g_Config.m_SvPickups != m_Pickups))
		GameServer()->Console()->ExecuteLine("reload");

	// do warmup
	if(m_Warmup)
	{
		m_Warmup--;
		if(!m_Warmup)
			StartRound();
	}
	
	if(m_GameOverTick != -1)
	{
		// game over.. wait for restart
		if(Server()->Tick() > m_GameOverTick+Server()->TickSpeed()*g_Config.m_SvGameOverTime)
		{
			CycleMap();
			StartRound();
			m_RoundCount++;
		}
	}
	
	// update browse info
	int Prog = -1;
	if(!m_Warmup && g_Config.m_SvScorelimit)
	{
		if(IsTeamplay())
		{
			Prog = max(Prog, (m_aTeamscore[0]*100)/g_Config.m_SvScorelimit);
			Prog = max(Prog, (m_aTeamscore[1]*100)/g_Config.m_SvScorelimit);
		}
		else
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(GameServer()->m_apPlayers[i])
					Prog = max(Prog, (GameServer()->m_apPlayers[i]->m_Score*100)/g_Config.m_SvScorelimit);
			}
		}
	}
	
	Server()->SetBrowseInfo(m_pGameType, Prog);
}

bool CGameControllerCatching::IsCatching() const
{
	return true;
}

bool CGameControllerCatching::JoiningSystem() const
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

void CGameControllerCatching::PostReset()
{
	//bool IsRoundRestart = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->Respawn();
			if(GameServer()->m_apPlayers[i]->m_BaseCatchingTeam != -1 && !GameServer()->m_apPlayers[i]->m_IsJoined && GameServer()->m_apPlayers[i]->GetTeam() != -1)
				GameServer()->m_apPlayers[i]->m_IsJoined = true;
			GameServer()->m_apPlayers[i]->m_CatchingTeam = GameServer()->m_apPlayers[i]->m_BaseCatchingTeam;
			GameServer()->m_apPlayers[i]->m_PrevCatchingTeam = GameServer()->m_apPlayers[i]->m_BaseCatchingTeam;
			OnPlayerInfoChange(GameServer()->m_apPlayers[i]);
			if(m_RoundRestart)
			{
				GameServer()->m_apPlayers[i]->m_Score = 0;
				//IsRoundRestart = true;
			}
			GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
			GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
		}
	}
	//if(IsRoundRestart)
	m_RoundRestart = false;
}

void CGameControllerCatching::OnPlayerInfoChange(class CPlayer *pPlayer)
{
	// Teamcolorwave \o/
	int black = 0,
		grey = 196,
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
	int TeamColors[MAX_CLIENTS] = {grey, orange, aqua, rpink, yellow, green, red, blue, purple, black, pink, lpurple, lblue, lyellow, lorange, lgreen};
	TeamColors[-1] = white;

	if(pPlayer->m_BaseCatchingTeam != -1 && pPlayer->m_CatchingTeam == -1)
		pPlayer->m_CatchingTeam = pPlayer->m_BaseCatchingTeam;

	if(/*pPlayer->m_BaseCatchingTeam != -1 && */(pPlayer->m_CatchingTeam >= -1 || pPlayer->m_CatchingTeam < MAX_CLIENTS))
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 1;
		pPlayer->m_TeeInfos.m_ColorBody = TeamColors[pPlayer->m_CatchingTeam];
		if(pPlayer->m_PrevCatchingTeam == -1)
			pPlayer->m_TeeInfos.m_ColorFeet = TeamColors[pPlayer->m_CatchingTeam];
		else
			pPlayer->m_TeeInfos.m_ColorFeet = TeamColors[pPlayer->m_PrevCatchingTeam];
	}
}

void CGameControllerCatching::DoPlayerNumWincheck()
{
	if(m_GameOverTick != -1 || m_Warmup)
		return;
	// get winning team
	int FirstFoundTeam = -1;
	int NumPlayers = 0;
	bool MoreThanOneTeam = false;
	// gather some stats
	int Topscore = 0;
	int TopscoreCount = 0;
	int TopID = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() > -1 && GameServer()->m_apPlayers[i]->m_CatchingTeam > -1)
		{
			NumPlayers++;
			if(GameServer()->m_apPlayers[i]->m_Score > Topscore)
			{
				Topscore = GameServer()->m_apPlayers[i]->m_Score;
				TopscoreCount = 1;
				TopID = i;
			}
			else if(GameServer()->m_apPlayers[i]->m_Score == Topscore)
				TopscoreCount++;
		}
		else
			continue;
			
		if(FirstFoundTeam < 0)
		{
			FirstFoundTeam = GameServer()->m_apPlayers[i]->m_CatchingTeam;
			continue;
		}

		// if more than 1 team ingame dont end round
		if(GameServer()->m_apPlayers[i]->m_CatchingTeam != FirstFoundTeam)
		{
			MoreThanOneTeam = true;
		}
	}
	
	// check score win condition
	if((g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit) ||
		(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
	{
		if(TopscoreCount == 1)
		{
			char buf[1024];
			str_format(buf, sizeof(buf), "%s won the game. A new round start", Server()->ClientName(TopID));
			GameServer()->SendBroadcast(buf, -1);
			GameServer()->SendChatTarget(-1, buf);
			m_RoundRestart = true;
			EndRound();
		}
		else
			m_SuddenDeath = 1;
	}

	// get the winner
	if(NumPlayers > 1 && !MoreThanOneTeam)
	{
		int Winner = -1;
		for(int i = 0; i < MAX_CLIENTS; i++)
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->m_BaseCatchingTeam == FirstFoundTeam)
				Winner = i;
		
		if(Winner > -1)
		{
			char buf[1024];
			str_format(buf, sizeof(buf), "%s's Team wins", Server()->ClientName(Winner));
			GameServer()->SendBroadcast(buf, -1);
			GameServer()->SendChatTarget(-1, buf);
			GameServer()->m_apPlayers[Winner]->m_Score += g_Config.m_SvScoreIncrease;
			EndRound();
        }
		else
		{
			GameServer()->SendBroadcast("No Team wins", -1);
			GameServer()->SendChatTarget(-1, "No Team wins");
			EndRound();
		}
	}
}

int CGameControllerCatching::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	// do scoreing
	if(!pKiller || Weapon == WEAPON_GAME)
		return 0;
	if(pKiller == pVictim->GetPlayer())
		pVictim->GetPlayer()->m_Score--; // suicide
	else
	{
		if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
			pKiller->m_Score--; // teamkill
		else
			pKiller->m_Score++; // normal kill
	}
	
	// set new team, send messages (hinweis an nox: war vorher in character.
	//								stört da aber da es auch andere gametypen beeinflusst!
	//								(und ständige "if isCatching()" unschön, unleserlich
	//								und schwer zu warten sind.))
	int TeamOwner = -1,
		KillerID = pKiller->GetCID(),
		VictimID = pVictim->GetPlayer()->GetCID();
	char KillerMsg[128], VictimMsg[128], OwnerMsg[128];
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != -1 &&
			GameServer()->m_apPlayers[i]->m_BaseCatchingTeam == pKiller->m_CatchingTeam &&
			pKiller->m_BaseCatchingTeam != pKiller->m_CatchingTeam)
			TeamOwner = i;
	}
	if(pKiller->m_CatchingTeam == pKiller->m_BaseCatchingTeam)
	{
		str_format(KillerMsg, sizeof(KillerMsg),  "You caught %s in your team", Server()->ClientName(VictimID));
		str_format(VictimMsg, sizeof(VictimMsg),  "You are now in %s's team", Server()->ClientName(KillerID));
	}
	else if(TeamOwner != -1)
	{
		str_format(KillerMsg, sizeof(KillerMsg),  "You caught %s in %s's team", Server()->ClientName(VictimID), Server()->ClientName(TeamOwner));
		str_format(VictimMsg, sizeof(VictimMsg),  "You are now in %s's team", Server()->ClientName(TeamOwner));
		str_format(OwnerMsg, sizeof(OwnerMsg),  "%s is now in your team", Server()->ClientName(VictimID));
	}
	else
	{
		str_format(KillerMsg, sizeof(KillerMsg),  "You caught %s in the same team as you", Server()->ClientName(VictimID));
		str_format(VictimMsg, sizeof(VictimMsg),  "You are now in the same team as %s", Server()->ClientName(KillerID));
	}
	pKiller->m_NoBroadcast = Server()->TickSpeed() * 5;
	pKiller->m_TickBroadcast = true;
	GameServer()->SendBroadcast(KillerMsg, KillerID);

	pVictim->GetPlayer()->m_NoBroadcast = Server()->TickSpeed() * 5;
	pVictim->GetPlayer()->m_TickBroadcast = true;
	GameServer()->SendBroadcast(VictimMsg, VictimID);

	if(TeamOwner != -1)
	{
		GameServer()->m_apPlayers[TeamOwner]->m_NoBroadcast = Server()->TickSpeed() * 3;
		GameServer()->m_apPlayers[TeamOwner]->m_TickBroadcast = true;
		GameServer()->SendBroadcast(OwnerMsg, TeamOwner);
	}
	
	pVictim->ChangeTeam(VictimID, KillerID, pVictim->GetPlayer()->m_CatchingTeam, pKiller->m_CatchingTeam);
	return 0;
}
