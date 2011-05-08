#include "gamecontext.h"
#include <engine/shared/config.h>
#include <game/version.h>
#include <stdio.h>

bool CGameContext::ChatCommands(int ClientID, CPlayer *pPlayer, const char * Message)
{
	bool SendToChat = false;

	char aTeamColors[MAX_CLIENTS][64] =
		{"Default", "Orange", "Aqua", "Pink", "Yellow", "Green", "Red", "Blue", "Purple",
		"Black", "LightRed", "LightPurple", "LightBlue", "LightYellow", "LightOrange", "LightGreen"};

	char Buf[512];
	if(!str_comp(Message, "/info"))
	{
		str_format(Buf, sizeof(Buf), "Catching %s for Teeworlds %s", CATCHING_VERSION, GAME_VERSION);
		SendChatTarget(ClientID, Buf);
		SendChatTarget(ClientID, "By Nox Nebula (Noxi Foxi) and Erd B\xC3\xA4r"); // "\xC3\xA4" = UTF-8 "ä"
		SendChatTarget(ClientID, " ");
		SendChatTarget(ClientID, "Say \"/cmdlist\" for list of command available.");
	}
	else if(!str_comp(Message, "/cmdlist") || !str_comp(Message, "/cmd"))
	{
		SendChatTarget(ClientID, "-- Commands --");
		SendChatTarget(ClientID, "'/info' - Display infos about this Mod.");
		SendChatTarget(ClientID, "'/color <Color>' - Choose your color from the color list.");
		SendChatTarget(ClientID, "'/colorlist' or '/list' - Display all available colors.");
		if(Server()->IsAuthed(ClientID))
			SendChatTarget(ClientID, "'/admincmd' - Display admin commands.");
	}
	else if(!str_comp(Message, "/colorlist") || !str_comp(Message, "/list"))
	{
		SendChatTarget(ClientID, "-- Color List --");
		for(int i = 0; i < MAX_CLIENTS; i += 2)
		{
			str_format(Buf, sizeof(Buf), "%i. %s        %i. %s", i + 1, aTeamColors[i], i + 2, aTeamColors[i + 1]);
			SendChatTarget(ClientID, Buf);
		}
	}
	else if(!str_comp_num(Message, "/color", 6) && m_apPlayers[ClientID]->GetTeam() != TEAM_SPECTATORS)
	{
		char aColor[64];
		sscanf(Message, "/color %s", aColor);

		int ColorID = -1;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!str_comp_nocase(aColor, aTeamColors[i]))
				ColorID = i;
		}

		if(ColorID == -1)
			SendChatTarget(ClientID, "The specified color could not be found.");
		else if(m_pController->IsColorUsed(ColorID))
			SendChatTarget(ClientID, "This color is already taken.");
		else
		{
			if(m_pController->GetJoinedPlayers() < 3 || !m_apPlayers[ClientID]->IsJoined())
			{
				m_apPlayers[ClientID]->SetCatchingTeam(ColorID, true);

				if(m_apPlayers[ClientID]->IsJoined())
				{
					str_format(Buf, sizeof(Buf), "You are now '%s'", aTeamColors[ColorID]);
					m_apPlayers[ClientID]->GetCharacter()->CaughtAnimation(ClientID);
				}
				else
					str_format(Buf, sizeof(Buf), "You join automaticaly in '%s', when a new round starts", aTeamColors[ColorID]);
				SendChatTarget(ClientID, Buf);
			}
			else
			{
				if(m_pController->IsWishUsed(ColorID))
					SendChatTarget(ClientID, "This color is already wished by another player, please try it later again.");
				else
				{
					m_apPlayers[ClientID]->SetColorWish(ColorID);
					SendChatTarget(ClientID, "Your color will change automaticaly after this round.");
				}
			}
		}
	}
	else if(!str_comp(Message, "/admincmd") && Server()->IsAuthed(ClientID))
	{
		SendChatTarget(ClientID, "-- Admin Commands --");
		SendChatTarget(ClientID, "'/tphere <Name>' - Teleport the Player with this Name near you.");
		SendChatTarget(ClientID, "'/tp <Name>' - Teleport you near the Player with this Name.");
	}
	else if(!str_comp_num(Message, "/tphere", 7) && Server()->IsAuthed(ClientID))
	{
		char aTarget[MAX_NAME_LENGTH];
		sscanf(Message, "/tphere %s", aTarget);

		int TargetID = SearchName(aTarget);
		if(TargetID == -1)
		{
			str_format(Buf, sizeof(Buf), "The specified name '%s' could not be found.", aTarget);
			SendChatTarget(ClientID, Buf);
		}
		else
		{
			CCharacter* pChar = GetPlayerChar(TargetID);

			if(m_apPlayers[TargetID] && pChar && TargetID != ClientID)
			{
				pChar->m_Core.m_Pos = m_apPlayers[ClientID]->m_ViewPos;
				SendChatTarget(TargetID, "Wooosh...");
				CreateDeath(m_apPlayers[TargetID]->m_ViewPos, TargetID);
				CreateSound(m_apPlayers[ClientID]->m_ViewPos, SOUND_PLAYER_DIE);
			}
			else if(TargetID == ClientID)
				SendChatTarget(ClientID, "Yeah... Funny...");
			else // Invalid ID
			{
				//str_format(Buf, sizeof(Buf), "Oops, yo can't teleport '%s' here, because the character of ID '%i' wasn't found.", aTarget, TargetID);
				SendChatTarget(ClientID, "No match found.");
			}
		}
	}
	else if(!str_comp_num(Message, "/tp", 3) && Server()->IsAuthed(ClientID))
	{
		
		char aTarget[MAX_NAME_LENGTH];
		sscanf(Message, "/tp %s", aTarget);

		int TargetID = SearchName(aTarget);
		if(TargetID == -1)
		{
			str_format(Buf, sizeof(Buf), "The specified name '%s' could not be found.", aTarget);
			SendChatTarget(ClientID, Buf);
		}
		else
		{
			CCharacter* pChar = GetPlayerChar(ClientID);

			if(m_apPlayers[TargetID] && pChar && TargetID != ClientID)
			{
				pChar->m_Core.m_Pos = m_apPlayers[TargetID]->m_ViewPos;
				SendChatTarget(ClientID, "Wooosh...");
				CreateDeath(m_apPlayers[ClientID]->m_ViewPos, ClientID);
				CreateSound(m_apPlayers[ClientID]->m_ViewPos, SOUND_PLAYER_DIE);
			}
			else if(TargetID == ClientID)
				SendChatTarget(ClientID, "Yeah... Funny...");
			else // Invalid ID
			{
				//str_format(Buf, sizeof(Buf), "Oops, yo can't teleport to '%s', because the character of ID '%i' wasn't found.", aTarget, TargetID);
				SendChatTarget(ClientID, "No match found.");
			}
		}
	}
	else if(!str_comp_num(Message, "/", 1))
	{
		SendChatTarget(ClientID, "Wrong command.");
		SendChatTarget(ClientID, "Say \"/cmdlist\" for list of command available.");
	}
	else
		SendToChat = true;

	return SendToChat;
}

int CGameContext::SearchName(char Name[MAX_NAME_LENGTH])
{
	// Search playername and set his ID
	int TargetID = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
		if(!str_comp_nocase(Name, Server()->ClientName(i)))
			TargetID = i;
	if(TargetID == -1)
		for(int i = 0; i < MAX_CLIENTS; i++)
			if(str_find_nocase(Server()->ClientName(i), Name))
				TargetID = i;

	return TargetID;
}