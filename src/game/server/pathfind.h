#ifndef GAME_SERVER_PATH_H
#define GAME_SERVER_PATH_H
#include <base/tl/array.h>
#include "binaryheap.h"
#include "gamecontext.h"

// player object
class CPathfind
{
public:
	CPathfind(CGameContext *pGameServer);
	
	struct CNode
	{
		vec2 m_Pos;
		int m_Parent;
		int m_ID;
		int m_G;
		int m_H;
		int m_F;
		bool m_IsCol;
		bool m_IsClosed;
		bool m_IsOpen;
		
		bool operator<(const CNode& Other) const { return (this->m_F < Other.m_F); }
		bool operator==(const CNode& Other) const { return (this->m_ID < Other.m_ID); }
	};
	
	void Init();
	void SetStart(vec2 Pos);
	void SetEnd(vec2 Pos);
	int GetIndex(int XPos, int YPos);
	
	void FindPath();
	
	array<CNode> m_lMap;
	array<CNode> m_lFinalPath;
	
	int m_FinalSize;
private:
	enum
	{
		START=-3,
		END=-4,
	};
	
	CGameContext *m_pGameServer;
	
	array<CNode> m_lNodes;
	
	// binary heap for open nodes
	CBinaryHeap<CNode> m_Open;
	
	int m_StartIndex;
	int m_EndIndex;
	
	int m_LayerWidth;
	int m_LayerHeight;
	
	// fake array sizes
	int m_ClosedNodes;
};

#endif
