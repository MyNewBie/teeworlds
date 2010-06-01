#ifndef GAME_CLIENT_COMPONENTS_COLORBOARD_H
#define GAME_CLIENT_COMPONENTS_COLORBOARD_H
#include <game/client/component.h>

class CColorboard : public CComponent
{
	void RenderColorboard();

	static void ConKeyColorboard(IConsole::IResult *pResult, void *pUserData);
	
	bool m_StatJustActivated;
	
	int m_LastRequest;
	
	int m_CurrentLine;
	int m_CurrentRow;
	int m_StartX;
	int m_StartY;
	
	void SendStatMsg(int Num);
	
public:
	CColorboard();
	
	bool m_Active;
	int m_Marked;
	
	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnRender();
};

#endif
