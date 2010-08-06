#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include <base/vmath.h>

class CCollision
{
	class CTile *m_pTiles;
	int m_Width;
	int m_Height;
	class CLayers *m_pLayers;

	bool IsTileSolid(int x, int y);
	int GetTile(int x, int y);

public:
	enum
	{
		COLFLAG_SOLID=1,
		COLFLAG_DEATH=2,
		COLFLAG_NOHOOK=4,
	};

	CCollision();
	void Init(class CLayers *pLayers);
	bool CheckPoint(float x, float y) { return IsTileSolid(round(x), round(y)); }
	bool CheckPoint(vec2 p) { return CheckPoint(p.x, p.y); }
	int GetCollisionAt(float x, float y) { return GetTile(round(x), round(y)); }
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }
	int IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision);
	int IntersectLine2(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision);
	void MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *Bpounces);
	void MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity);
	bool TestBox(vec2 Pos, vec2 Size);
	bool IsHideTile(vec2 Pos);
	// TELEPORT-SPEEDUP
	int GetIndex(vec2 PrevPos, vec2 Pos);
	vec2 GetPos(int Index);
	int GetCollisionIndex(int Index);
	int IsTeleport(int Index);
	int IsSpeedup(int Index);
	void GetSpeedup(int Index, vec2 *Dir, int *Force);
	
	class CTeleTile *m_pTele;
	class CSpeedupTile *m_pSpeedup;
	
	class CLayers *Layers() { return m_pLayers; }
};

#endif
