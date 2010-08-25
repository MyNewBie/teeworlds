#include "gamecontext.h"
#include <engine/shared/config.h>
#include <game/version.h>
#include <stdio.h>

bool CGameContext::ChatCommands(int ClientId, CPlayer *p, CNetMsg_Cl_Say *pMsg)
{
	bool Passed = true;
	char Buf[512];

	if(!str_comp(pMsg->m_pMessage, "/info"))
	{
		str_format(Buf, sizeof(Buf), "Catching %s for Teeworlds %s", CATCHING_VERSION, GAME_VERSION);
		SendChatTarget(ClientId, Buf);
		SendChatTarget(ClientId, "By Nox Nebula and Erd B\xC3\xA4r"); // "\xC3\xA4" = UTF-8 "ä"
		SendChatTarget(ClientId, " ");
		SendChatTarget(ClientId, "Say \"/thanks\" for Special Thanks.");
		SendChatTarget(ClientId, "Say \"/cmdlist\" for list of command available.");
	}
	else if(!str_comp(pMsg->m_pMessage, "/thanks"))
	{
		SendChatTarget(ClientId, "Special Thanks to:");
		SendChatTarget(ClientId, "  SushiTee:");
		SendChatTarget(ClientId, "    Wincondition fix (Catching)");
		SendChatTarget(ClientId, "    Pathfinding");
	}
	else
		Passed = false;

	return Passed;
}

bool CGameContext::ChatCommandsCatching(int ClientId, CPlayer *p, CNetMsg_Cl_Say *pMsg)
{
	// Check for global commands
	bool Passed = ChatCommands(ClientId, p, pMsg);
	if(Passed)
		return false;

	bool DisplayChat = false;
	char Buf[512];

	if(!str_comp(pMsg->m_pMessage, "/cmdlist"))
	{
		SendChatTarget(ClientId, "Say \"/info\" for modinfo.");
		if(p->m_BaseCatchingTeam == -1)
		{
			SendChatTarget(ClientId, "Say \"/color\" to select your catching-color.");
			SendChatTarget(ClientId, "Say \"/colorlist\" for list of colors available.");
		}
	}
	else if(!str_comp(pMsg->m_pMessage, "/about:color"))
	{
		str_format(Buf, sizeof(Buf),  "Your Color: %d", p->m_BaseCatchingTeam);
		SendChatTarget(ClientId, Buf);
	}
	else if(!str_comp(pMsg->m_pMessage, "/colorlist"))
	{
		SendChatTarget(ClientId, "ID - Color  |  ID - Color");
		SendChatTarget(ClientId, "0 - default  |  8 - purple");
		SendChatTarget(ClientId, "1 - orange  |  9 - black/grey");
		SendChatTarget(ClientId, "2 - aqua  |  10 - rose");
		SendChatTarget(ClientId, "3 - pink  |  11 - light purple");
		SendChatTarget(ClientId, "4 - yellow  |  12 - light blue");
		SendChatTarget(ClientId, "5 - green  |  13 - light yellow");
		SendChatTarget(ClientId, "6 - red  |  14 - light orange");
		SendChatTarget(ClientId, "7 - blue  |  15 - light green");
		SendChatTarget(ClientId, "0 - any color");
	}
	else if(!str_comp_num(pMsg->m_pMessage, "/color", 6))
	{
		int Color;
		sscanf(pMsg->m_pMessage, "/color %d", &Color);

		if(m_apPlayers[ClientId]->m_BaseCatchingTeam != -1)
		{
			if(Color == -1)
			{
				p->m_IsJoined = false;
				p->m_BaseCatchingTeam = -1;
				p->m_CatchingTeam = -1;
				m_pController->OnPlayerInfoChange(p);
				p->GetCharacter()->CreateDieExplosion(false);
			}
			else
				SendChatTarget(ClientId, "Use /color -1 to delete your team");
		}
		else
		{
			// Check for used Colors and how many players playing
			int UsedColor[MAX_CLIENTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
			int NumPlayers = 0;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != -1 && m_apPlayers[i]->m_BaseCatchingTeam != -1)
					UsedColor[m_apPlayers[i]->m_BaseCatchingTeam] = 1;

				if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != -1 && m_apPlayers[i]->m_IsJoined)
					NumPlayers++;
			}

			if(Color >= 0 && Color < MAX_CLIENTS)
			{
				if(!UsedColor[Color]) // Check if Color is already used
				{
					p->m_BaseCatchingTeam = Color;

					if(NumPlayers < g_Config.m_SvCheatProtection)
					{
						p->m_IsJoined = true;
						p->GetCharacter()->CreateDieExplosion(false);
					}
					else
						SendChatTarget(ClientId, "Please wait until this round ends");

					dbg_msg("Colorselect", "Player: %d -> Color: %d", ClientId, Color);
					m_pController->OnPlayerInfoChange(p);
				}
				else if(Color == 0) // Give a random Color
				{
					for(int i = 0; i < MAX_CLIENTS; i++)
					{
						if(!UsedColor[i])
						{
							Color = i;
							break;
						}
					}
					p->m_BaseCatchingTeam = Color;

					if(NumPlayers < g_Config.m_SvCheatProtection)
					{
						p->m_IsJoined = true;
						p->GetCharacter()->CreateDieExplosion(false);
					}
					else
						SendChatTarget(ClientId, "Please wait until this round ends");

					dbg_msg("Colorselect", "Player: %d -> Random-Color: %d", ClientId, Color);
					m_pController->OnPlayerInfoChange(p);
					str_format(Buf, sizeof(Buf),  "Random Color: %d", Color);
					SendChatTarget(ClientId, Buf);
				}
				else
					SendChatTarget(ClientId, "This color is already in use");
			}
			else
			{
				SendChatTarget(ClientId, "Use /color <color ID>");
				SendChatTarget(ClientId, "Use /color 0 to get any color");
				SendChatTarget(ClientId, "Use /colorlist to displays the Colorlist");
			}
		}
	}
	else if(!str_comp_num(pMsg->m_pMessage, "/", 1))
	{
		SendChatTarget(ClientId, "Wrong command.");
		SendChatTarget(ClientId, "Say \"/cmdlist\" for list of command available.");
	}
	else
		DisplayChat = true;

	return DisplayChat;
}

void CGameContext::ChatCommandsZCatch(int ClientId, CPlayer *p, CNetMsg_Cl_Say *pMsg)
{
	// Check for global commands
	bool Passed = ChatCommands(ClientId, p, pMsg);
	if(Passed)
		return;

	char aBuf[512];
	if(!str_comp_num(pMsg->m_pMessage, "/switch", 7)) // only for testing purposes
	{
		p->m_IsJoined ^= 1;
		str_format(aBuf, sizeof(aBuf), "You are now %d", p->m_IsJoined);
		SendChatTarget(ClientId, aBuf);
	}
	else
	{
		SendChatTarget(ClientId, "Wrong command.");
		SendChatTarget(ClientId, "Say \"/cmdlist\" for list of command available.");
	}
}