/* Pathfind class by Sushi and Patafix*/
#ifndef GAME_SERVER_PATH_H
#define GAME_SERVER_PATH_H

#include <base/vmath.h>

#include <base/tl/array.h>
#include <base/tl/sorted_array.h>
#include <base/tl/binaryheap.h>

class CPathfind
{
public:
	CPathfind(class CGameContext *pGameServer);

	struct CNode
	{
		vec2 m_Pos;
		int m_Parent;
		int m_ID;
		int m_G;
		int m_H;
		int m_F;
		int m_Flag;
		bool operator<(const CNode& Other) const { return (this->m_F < Other.m_F); }
		bool operator==(const CNode& Other) const { return (this->m_ID < Other.m_ID); }

	};

	void Init();
	void SetStart(vec2 Pos);
	void SetEnd(vec2 Pos);
	int GetIndex(int XPos, int YPos);
//	int GetDir(int Index, int PrevIndex);

	void FindPath();

	bool WasCancled() { return m_Cancled; }

	array<CNode> m_lFinalPath;
	array<vec2> m_lReachableTiles;

	int m_ReachableTilesNum;

	int m_FinalSize;

private:
	enum
	{
		// Flags
		COL=1,
		CLOSED=2,
		OPEN=4,
		NOTREACHABLE=8,
		UNHOOKABLE=16,
		AVOIDABLE=32, //Bad name

		// special Parents
		START=-3,
		END=-4,

		// add flags
		DOWN = 1,
		UP = 2,
		RIGHT= 4,
		LEFT = 8,
		UPRIGHT = 16,
		DOWNRIGHT = 32,
		DOWNLEFT = 64,
		UPLEFT = 128,
	};

	CGameContext *m_pGameServer;

	array<CNode> m_lMap;
	array<CNode> m_lNodes;

	// binary heap for open nodes
	CBinaryHeap<CNode> m_Open;

	int m_StartIndex;
	int m_EndIndex;

	int m_LayerWidth;
	int m_LayerHeight;

	// fake array sizes
	int m_ClosedNodes;

	bool m_Cancled;
};

#endif
