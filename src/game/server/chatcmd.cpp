#include "gamecontext.h"
#include <engine/shared/config.h>
#include <game/version.h>

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
	else if(!str_comp_num(Message, "/", 1))
	{
		SendChatTarget(ClientID, "Wrong command.");
		SendChatTarget(ClientID, "Say \"/cmdlist\" for list of command available.");
	}
	else
		SendToChat = true;

	return SendToChat;
}