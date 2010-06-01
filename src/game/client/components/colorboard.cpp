#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/client/components/motd.h>
#include "skins.h"
#include "colorboard.h"
#include "camera.h"
#include "controls.h"


CColorboard::CColorboard()
{
	OnReset();
}

void CColorboard::ConKeyColorboard(IConsole::IResult *pResult, void *pUserData)
{
	((CColorboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CColorboard::OnReset()
{
	m_Active = false;
	m_StatJustActivated = false;
	m_LastRequest = -1;
	m_Marked = 0;
	m_CurrentLine = 0;
	m_CurrentRow = 0;
	m_StartX = 0;
	m_StartY = 0;
}

void CColorboard::OnConsoleInit()
{
	Console()->Register("+Colorboard", "", CFGFLAG_CLIENT, ConKeyColorboard, this, "Show Colorboard");
}

void CColorboard::RenderColorboard()
{
	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;
	
	float w = 320.0f;
	float h = 400.0f;
	float Info = 0;

	//float FontHeight = 50.0f;
	//float TeeSizeMod = 1.0f;
	float x = (Width/2-w)+25;
	
	float Offset = 0;
	
	CUIRect MainView;
	MainView.x = (Width/2-w)+15;
	MainView.y = 220;
	MainView.w = w*2-32.5;
	MainView.h = 20+h;
	
	CUIRect LeftView, RightView, Label;
	
	Graphics()->MapScreen(0, 0, Width, Height);
	
	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->DrawRoundRect(Width/2-w, 200.0f, w*2, 30+h+Info, 17.0f);
	Graphics()->SetColor(0.7, 0.7, 0.7, 0.25);
	RenderTools()->DrawRoundRect((Width/2-w)+35, 220.0f, w*2-52.5, 50, 17.0f);
	Graphics()->SetColor(0.7, 0.7, 0.7, 0.15);
	RenderTools()->DrawRoundRect((Width/2-w)+15, 290.0f, w*2-32.5, 295, 17.0f);
	Graphics()->QuadsEnd();

	MainView.VSplitLeft(MainView.w/2, &LeftView, &RightView);
	
	// render tee
	CTeeRenderInfo SkinInfo = m_pClient->m_aClients[m_pClient->m_Snap.m_LocalCid].m_RenderInfo;
	if(g_Config.m_PlayerUseCustomColor)
	{
		SkinInfo.m_Texture = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin))->m_ColorTexture;
		SkinInfo.m_ColorBody = m_pClient->m_pSkins->GetColor(g_Config.m_PlayerColorBody);
		SkinInfo.m_ColorFeet = m_pClient->m_pSkins->GetColor(g_Config.m_PlayerColorFeet);
	}
	SkinInfo.m_Size = UI()->Scale()*100.0f;
	
	RenderTools()->RenderTee(CAnimState::GetIdle(), &SkinInfo, 0, vec2(1, 0), vec2(LeftView.x+35, LeftView.y+30));
		
	// render headline
	MainView.VSplitLeft(MainView.w/2-TextRender()->TextWidth(0, 40.0f, "Colorselection", -1)/2, 0, &Label);
	UI()->DoLabel(&Label, "Colorselection", 40.0f, -1);
	
	// Colorrender
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
	int TeamColors[16] = {white, orange, aqua, rpink, yellow, green, red, blue, purple, black, pink, lpurple, lblue, lyellow, lorange, lgreen};
	int Line = 0;
	CTeeRenderInfo SkinInfo1 = m_pClient->m_aClients[m_pClient->m_Snap.m_LocalCid].m_RenderInfo;
	SkinInfo1.m_Texture = m_pClient->m_pSkins->Get(m_pClient->m_pSkins->Find(g_Config.m_PlayerSkin))->m_ColorTexture;
	SkinInfo1.m_Size = UI()->Scale()*100.0f;
	//vec2 SkinPos = vec2(MainView.x+62.5, (MainView.y-60));
	for(int i = 0; i < 16; i++)
	{
		if(i % 6 == 0 && i != 0)
			Line++;
		//Graphics()->QuadsBegin();

		SkinInfo1.m_ColorBody = m_pClient->m_pSkins->GetColor(TeamColors[i]);
		SkinInfo1.m_ColorFeet = m_pClient->m_pSkins->GetColor(TeamColors[i]);
		RenderTools()->RenderTee(CAnimState::GetIdle(), &SkinInfo1, 0, vec2(1, 0), vec2(LeftView.x+((i-Line*6)*100)+50, LeftView.y+(Line*100)+125));
		//RenderTools()->RenderTee(CAnimState::GetIdle(), &SkinInfo1, 0, vec2(1, 0), SkinPos);

		//Graphics()->QuadsEnd();

		//SkinPos.x += MainView.w/6;
	}

	RightView.HSplitTop(60+Offset, 0, &RightView);
	RightView.HSplitTop(200.0f, &Label, &RightView);
	RightView.VSplitLeft(RightView.w/2-TextRender()->TextWidth(0, 40.0f, "Any", -1)/2, 0, &Label);
	UI()->DoLabel(&Label, "Any", 40.0f, -1);

	w = MainView.w/5-20;
	
	MainView.HSplitTop(410, 0, &MainView);
	
	// render key info
	TextRender()->Text(0, MainView.x+MainView.w-TextRender()->TextWidth(0, 20.0f, "Blabla.", -1), 190+h, 20.0f, "Blabla.", -1);
}

void CColorboard::SendStatMsg(int Num)
{
	if(Input()->KeyPressed(KEY_MOUSE_1) && Input()->KeyPressed(KEY_MOUSE_2))
	{
		if(Client()->GameTick() > m_LastRequest+Client()->GameTickSpeed()/12)
		{
			// ugly but we need to change those values
			if(Num == 6)
				Num = 7;
			else if(Num == 7)
				Num = 6;
		}
		m_LastRequest = Client()->GameTick();
	}
}
void CColorboard::OnRender()
{
	// if we activly wanna look on the scoreboard	
	if(m_Active && m_pClient->m_Snap.m_pGameobj && !m_pClient->m_Snap.m_pGameobj->m_GameOver)
	{		
		m_StatJustActivated = true;
		
		// clear motd
		m_pClient->m_pMotd->Clear();
		
		RenderColorboard();
	}
	else if(m_StatJustActivated)
		m_StatJustActivated = false;
}
