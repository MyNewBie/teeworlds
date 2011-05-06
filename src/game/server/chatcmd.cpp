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
	else if(!str_comp(Message, "/admincmd") && Server()->IsAuthed(ClientID))
	{
		SendChatTarget(ClientID, "-- Admin Commands --");
		SendChatTarget(ClientID, "'/tphere <ID>' - Teleport the Player with this ID near you.");
		SendChatTarget(ClientID, "'/tp <ID>' - Teleport you near the Player with this ID.");
	}
	else if(!str_comp_num(Message, "/tphere", 7) && Server()->IsAuthed(ClientID))
	{
		int TargetID;
		sscanf(Message, "/tphere %d", &TargetID);

		if(TargetID >= MAX_CLIENTS || TargetID < 0)
			SendChatTarget(ClientID, "Please set a valid ID \"/tphere <ID>\".");
		else
		{
			TargetID = clamp(TargetID, 0, MAX_CLIENTS-1);
			CCharacter* pChar = GetPlayerChar(TargetID);

			if(m_apPlayers[TargetID] && pChar && TargetID != ClientID)
			{
				pChar->m_Core.m_Pos = m_apPlayers[ClientID]->m_ViewPos;
				SendChatTarget(TargetID, "Wooosh...");
			}
			else if(TargetID == ClientID)
				SendChatTarget(ClientID, "Yea... Funny...");
			else
				SendChatTarget(ClientID, "Invalid ID");
			CreateDeath(m_apPlayers[TargetID]->m_ViewPos, TargetID);
		}
	}
	else if(!str_comp_num(Message, "/tp", 3) && Server()->IsAuthed(ClientID))
	{
		int TargetID;
		sscanf(Message, "/tp %d", &TargetID);
		if(TargetID >= MAX_CLIENTS || TargetID < 0)
			SendChatTarget(ClientID, "Please set a valid ID \"/tp <ID>\".");
		else
		{
			TargetID = clamp(TargetID, 0, MAX_CLIENTS-1);
			CCharacter* pChar = GetPlayerChar(ClientID);

			if(m_apPlayers[TargetID] && pChar && TargetID != ClientID)
			{
				pChar->m_Core.m_Pos = m_apPlayers[TargetID]->m_ViewPos;
				SendChatTarget(ClientID, "Wooosh...");
			}
			else if(TargetID == ClientID)
				SendChatTarget(ClientID, "Yea... Funny...");
			else
				SendChatTarget(ClientID, "Invalid ID");
			CreateDeath(m_apPlayers[ClientID]->m_ViewPos, ClientID);
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