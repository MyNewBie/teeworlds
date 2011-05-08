#include "catching.h"
#include <engine/shared/config.h>
#include "../gamecontext.h"

CGameControllerCatching::CGameControllerCatching(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	if(!str_comp(g_Config.m_SvGametype, "icatch"))
		m_pGameType = "iCatch";
	else
		m_pGameType = "Catch";
}

void CGameControllerCatching::Tick()
{
	DoPlayerNumWincheck();
	IGameController::Tick();
}

bool CGameControllerCatching::IsCatching() const
{
	return true;
}

void CGameControllerCatching::OnPlayerInfoChange(class CPlayer *pP)
{
	// Good old "Teamcolorwave" \o/
	int Black = 0,
		White = 255,
		Def = 1834896,
		Red = 16776960,
		Pink = 15138560,
		Orange = 1507072,
		LOrange = 1507201,
		LYellow = 2686852,
		Yellow = 2621184,
		LRed = 16760679,
		Green = 5018112,
		Aqua = 8453888,
		Blue = 9830144,
		LBlue = 9830257,
		LGreen = 5018235,
		Purple = 12896518,
		LPurple = 12896649;
	int TeamColors[MAX_CLIENTS + 1] = {White, Def, Orange, Aqua, Pink, Yellow, Green, Red, Blue, Purple, Black, LRed, LPurple, LBlue, LYellow, LOrange, LGreen};

	// If something was wrong, update Team
	if(pP->GetBaseTeam() != -1 && pP->GetCurrentTeam() == -1)
		pP->SetCatchingTeam(pP->GetBaseTeam());

	if(pP->GetCurrentTeam() >= -1 && pP->GetCurrentTeam() < MAX_CLIENTS)
	{
		// Set the teecolor
		pP->m_TeeInfos.m_UseCustomColor = 1;
		pP->m_TeeInfos.m_ColorBody = TeamColors[pP->GetCurrentTeam() + 1];
		pP->m_TeeInfos.m_ColorFeet = TeamColors[pP->GetPrevTeam() + 1];
	}
}

TeamStatistics CGameControllerCatching::TeamStatistic(int Team, int BaseColor)
{
	/*
		How many teams?
		How many players per team (int teamid)
		How many players are joined?
		Is (Base)Color used?
		Wich Player(ID) has the Base Color?
		Is this Color-Wish used?
	*/

	if(BaseColor == -1)
		BaseColor = Team;

	int TeamNum = 0;
	int PlayerNum = 0;
	int PlayerJoined = 0;
	bool IsUsed = false;
	int PlayerID = -1;
	bool WishIsUsed = false;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			// How many teams
			if(GameServer()->m_apPlayers[i]->IsJoined() && GameServer()->m_apPlayers[i]->GetBaseTeam() > -1)
				TeamNum++;

			// How many players per team
			if(GameServer()->m_apPlayers[i]->IsJoined() && GameServer()->m_apPlayers[i]->GetCurrentTeam() == Team && Team > -1)
				PlayerNum++;
			
			// How many players are joined
			if(GameServer()->m_apPlayers[i]->IsJoined() && GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
				PlayerJoined++;

			// Is (Base)Color used
			// Wich Player(ID) has the Base Color
			if(GameServer()->m_apPlayers[i]->GetBaseTeam() == BaseColor) {
				IsUsed = true;
				PlayerID = i;
			}

			// Is this Color-Wish used
			if(GameServer()->m_apPlayers[i]->GetColorWish() == BaseColor)
				WishIsUsed = true;
		}
	}
	
	TeamStatistics Return = {
		TeamNum,
		PlayerNum,
		PlayerJoined,
		IsUsed,
		PlayerID,
		WishIsUsed
	};
	return Return;
}

// Single Functions
	int CGameControllerCatching::GetTeamNum()
	{
		TeamStatistics Return = TeamStatistic();
		return Return.TeamNum;
	}

	int CGameControllerCatching::GetPlayersNum(int Team)
	{
		TeamStatistics Return = TeamStatistic(Team);
		return Return.PlayerNum;
	}

	int CGameControllerCatching::GetJoinedPlayers()
	{
		TeamStatistics Return = TeamStatistic();
		return Return.PlayerJoined;
	}

	bool CGameControllerCatching::IsColorUsed(int Color)
	{
		TeamStatistics Return = TeamStatistic(Color);
		return Return.IsUsed;
	}

	int CGameControllerCatching::GetColorOwner(int BaseColor)
	{
		TeamStatistics Return = TeamStatistic(BaseColor);
		return Return.PlayerID;
	}

	bool CGameControllerCatching::IsWishUsed(int Color)
	{
		TeamStatistics Return = TeamStatistic(Color);
		return Return.WishIsUsed;
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
		// Successful Catch
		pVictim->CaughtAnimation(pKiller->GetCID(), true);
		pVictim->GetPlayer()->SetCatchingTeam(pKiller->GetCurrentTeam());
		pKiller->m_Score++;
	}
	return 0;
}

void CGameControllerCatching::PostReset()
{
	bool WasFinalRound = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->SetCatchingTeam(GameServer()->m_apPlayers[i]->GetBaseTeam(), true, true);

			GameServer()->m_apPlayers[i]->Respawn();
			GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
			GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
			
			if(m_FinalRound)
			{
				WasFinalRound = true;
				GameServer()->m_apPlayers[i]->m_Score = 0;
			}
		}
	}
	if(WasFinalRound)
		m_FinalRound = false;
}

void CGameControllerCatching::DoPlayerNumWincheck()
{
	if(m_GameOverTick == -1 && !m_Warmup)
	{
		// Team Win
		int PlayerNum = 0;
		int ColorID = -1;
		// Score Win
		int Topscore = 0;
		int TopscoreCount = 0;
		int PlayerID = -1;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(IsColorUsed(i)) {
				if(GetPlayersNum(i) > PlayerNum)
				{
					PlayerNum = GetPlayersNum(i);
					ColorID = i;
				}
			}

			if(GameServer()->m_apPlayers[i])
			{
				if(GameServer()->m_apPlayers[i]->m_Score > Topscore)
				{
					Topscore = GameServer()->m_apPlayers[i]->m_Score;
					PlayerID = i;
					TopscoreCount = 1;
				}
				else if(GameServer()->m_apPlayers[i]->m_Score > Topscore)
					TopscoreCount++;

			}
		}

		// Score Win
		if((g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit) ||
			(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
		{
			if(TopscoreCount == 1)
			{
				m_FinalRound = true;
				EndRound();
			}
			else
				m_SuddenDeath = 1;
		}

		// Team Win
		if(PlayerNum >= GetJoinedPlayers() && GetJoinedPlayers() > 1)
		{
			int Owner = GetColorOwner(ColorID);
			if(GameServer()->m_apPlayers[Owner])
				GameServer()->m_apPlayers[Owner]->m_Score += g_Config.m_SvExtraPoints;
			EndRound();
		}
	}
}
