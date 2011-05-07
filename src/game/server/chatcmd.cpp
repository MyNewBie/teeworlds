#include "gamecontext.h"
#include <engine/shared/config.h>
#include <game/version.h>
#include <stdio.h>

bool CGameContext::ChatCommands(int ClientID, CPlayer *pPlayer, const char * Message)
{
	bool SendToChat = false;

	char Buf[512];
	if(!str_comp(Message, "/info"))
	{
		str_format(Buf, sizeof(Buf), "Catching %s for Teeworlds %s", CATCHING_VERSION, GAME_VERSION);
		SendChatTarget(ClientID, Buf);
		SendChatTarget(ClientID, "By Nox Nebula (Noxi Foxi) and Erd B\xC3\xA4r"); // "\xC3\xA4" = UTF-8 "ä"
		SendChatTarget(ClientID, " ");
		SendChatTarget(ClientID, "Say \"/cmdlist\" for list of command available.");
	}
	else if(!str_comp(Message, "/cmdlist"))
	{
		SendChatTarget(ClientID, "-- Commands --");
		SendChatTarget(ClientID, "'/info' - Display infos about this Mod.");
		if(Server()->IsAuthed(ClientID))
			SendChatTarget(ClientID, "'/admincmd' - Display admin commands.");
	}
	else if(!str_comp_num(Message, "/color", 6) && m_apPlayers[ClientID]->GetTeam() != TEAM_SPECTATORS)
	{
		char aColor[64];
		sscanf(Message, "/color %s", aColor);

		char TeamColors[MAX_CLIENTS][64] = {"Default", "Orange", "Aqua", "RedPink", "Yellow", "Green", "Red", "Blue", "Purple",
											"Black", "Pink", "LightPurple", "LightBlue", "LightYellow", "LightOrange", "LightGreen"};
		int ColorID = -1;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!str_comp_nocase(aColor, TeamColors[i]))
				ColorID = i;
		}

		if(ColorID == -1)
			SendChatTarget(ClientID, "The specified color could not be found.");
		else
		{
			m_apPlayers[ClientID]->SetCatchingTeam(ColorID, true);
			str_format(Buf, sizeof(Buf), "You are now %s", TeamColors[ColorID]);
			SendChatTarget(ClientID, Buf);
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
			SendChatTarget(ClientID, "The specified name could not be found.");
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
			else
				SendChatTarget(ClientID, "Invalid ID");
		}
	}
	else if(!str_comp_num(Message, "/tp", 3) && Server()->IsAuthed(ClientID))
	{
		
		char aTarget[MAX_NAME_LENGTH];
		sscanf(Message, "/tp %s", aTarget);

		int TargetID = SearchName(aTarget);
		if(TargetID == -1)
			SendChatTarget(ClientID, "The specified name could not be found.");
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
			else
				SendChatTarget(ClientID, "Invalid ID");
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