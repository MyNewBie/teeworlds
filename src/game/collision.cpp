// copyright (c) 2007 magnus auvinen, see licence.txt for more info
#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <math.h>
#include <engine/map.h>
#include <engine/kernel.h>

#include <game/mapitems.h>
#include <game/layers.h>
#include <game/collision.h>

CCollision::CCollision()
{
	m_pTiles = 0;
	m_Width = 0;
	m_Height = 0;
	m_pLayers = 0;
	m_pTele = 0;
	m_pSpeedup = 0;
}

void CCollision::Init(class CLayers *pLayers)
{
	m_pLayers = pLayers;
	m_Width = m_pLayers->GameLayer()->m_Width;
	m_Height = m_pLayers->GameLayer()->m_Height;
	m_pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(m_pLayers->GameLayer()->m_Data));

	// TELEPORT-SPEEDUP
	if(m_pLayers->TeleLayer())
		m_pTele = static_cast<CTeleTile *>(m_pLayers->Map()->GetData(m_pLayers->TeleLayer()->m_Tele));
	if(m_pLayers->SpeedupLayer())
		m_pSpeedup = static_cast<CSpeedupTile *>(m_pLayers->Map()->GetData(m_pLayers->SpeedupLayer()->m_Speedup));
	
	for(int i = 0; i < m_Width*m_Height; i++)
	{
		int Index = m_pTiles[i].m_Index;
		
		if(Index > 128)
			continue;
		
		switch(Index)
		{
		case TILE_DEATH:
			m_pTiles[i].m_Index = COLFLAG_DEATH;
			break;
		case TILE_SOLID:
			m_pTiles[i].m_Index = COLFLAG_SOLID;
			break;
		case TILE_NOHOOK:
			m_pTiles[i].m_Index = COLFLAG_SOLID|COLFLAG_NOHOOK;
			break;
		case TILE_HIDE:
			m_pTiles[i].m_Index = Index;
			break;
		default:
			m_pTiles[i].m_Index = 0;
		}
	}
}

int CCollision::GetTile(int x, int y)
{
	int nx = clamp(x/32, 0, m_Width-1);
	int ny = clamp(y/32, 0, m_Height-1);
	
	// TELEPORT-SPEEDUP
	if(m_pTiles[ny*m_Width+nx].m_Index == COLFLAG_SOLID || m_pTiles[ny*m_Width+nx].m_Index == (COLFLAG_SOLID|COLFLAG_NOHOOK) || m_pTiles[ny*m_Width+nx].m_Index == COLFLAG_DEATH)
		return m_pTiles[ny*m_Width+nx].m_Index;
	else
		return 0;
}

// Check if this Tile are a HideTile
bool CCollision::IsHideTile(vec2 Pos)
{
	int nx = clamp((int)Pos.x/32, 0, m_Width-1);
	int ny = clamp((int)Pos.y/32, 0, m_Height-1);

	return m_pTiles[ny*m_Width+nx].m_Index == TILE_HIDE;
}

bool CCollision::IsTileSolid(int x, int y)
{
	return GetTile(x,y)&COLFLAG_SOLID;
}

// TELEPORT-SPEEDUP
int CCollision::GetIndex(vec2 PrevPos, vec2 Pos)
{
	int Index = 0;
	float d = distance(PrevPos, Pos);
	
	if(!d)
	{
		int nx = clamp((int)Pos.x/32, 0, m_Width-1);
		int ny = clamp((int)Pos.y/32, 0, m_Height-1);
		
		if((m_pTiles[ny*m_Width+nx].m_Index >= TILE_STOPL && m_pTiles[ny*m_Width+nx].m_Index <= 59) ||
			(m_pTele && (m_pTele[ny*m_Width+nx].m_Type == TILE_TELEIN || m_pTele[ny*m_Width+nx].m_Type == TILE_TELEOUT)) ||
			(m_pSpeedup && m_pSpeedup[ny*m_Width+nx].m_Force > 0))
		{
			return ny*m_Width+nx;
		}
	}
	
	float a = 0.0f;
	vec2 Tmp = vec2(0, 0);
	int nx = 0;
	int ny = 0;
	
	for(float f = 0; f < d; f++)
	{
		a = f/d;
		Tmp = mix(PrevPos, Pos, a);
		nx = clamp((int)Tmp.x/32, 0, m_Width-1);
		ny = clamp((int)Tmp.y/32, 0, m_Height-1);
		if((m_pTiles[ny*m_Width+nx].m_Index >= TILE_STOPL && m_pTiles[ny*m_Width+nx].m_Index <= 59) ||
			(m_pTele && (m_pTele[ny*m_Width+nx].m_Type == TILE_TELEIN || m_pTele[ny*m_Width+nx].m_Type == TILE_TELEOUT)) ||
			(m_pSpeedup && m_pSpeedup[ny*m_Width+nx].m_Force > 0))
		{
			return ny*m_Width+nx;
		}
	}
	
	return -1;
}

vec2 CCollision::GetPos(int Index)
{
	int x = Index%m_Width;
	int y = Index/m_Width;
	
	return vec2(x, y);
}

int CCollision::GetCollisionIndex(int Index)
{
	if(Index < 0)
		return 0;
		
	return m_pTiles[Index].m_Index;
}

int CCollision::IsTeleport(int Index)
{
	if(!m_pTele || Index < 0)
		return 0;
	
	int Tele = 0;
	if(m_pTele[Index].m_Type == TILE_TELEIN)
		Tele = m_pTele[Index].m_Number;
		
	return Tele;
}

int CCollision::IsSpeedup(int Index)
{
	if(!m_pSpeedup || Index < 0)
		return -1;
	
	if(m_pSpeedup[Index].m_Force > 0)
		return Index;
		
	return -1;
}

void CCollision::GetSpeedup(int Index, vec2 *Dir, int *Force)
{
	vec2 Direction = vec2(1, 0);
	float Angle = m_pSpeedup[Index].m_Angle * (3.14159265f/180.0f);
	*Force = m_pSpeedup[Index].m_Force;
	
	vec2 TmpDir;
	TmpDir.x = (Direction.x*cos(Angle)) - (Direction.y*sin(Angle));
	TmpDir.y = (Direction.x*sin(Angle)) + (Direction.y*cos(Angle));
	*Dir = TmpDir;
}

// TODO: rewrite this smarter!
int CCollision::IntersectLine(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision)
{
	float d = distance(Pos0, Pos1);
	vec2 Last = Pos0;
	
	for(float f = 0; f < d; f++)
	{
		float a = f/d;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(Pos.x, Pos.y);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

int CCollision::IntersectLine2(vec2 Pos0, vec2 Pos1, vec2 *pOutCollision, vec2 *pOutBeforeCollision)
{
	float d = distance(Pos0, Pos1);
	vec2 Last = Pos0;
	
	for(float f = 0; f < d; f+=32)
	{
		float a = f/d;
		vec2 Pos = mix(Pos0, Pos1, a);
		if(CheckPoint(Pos.x, Pos.y))
		{
			if(pOutCollision)
				*pOutCollision = Pos;
			if(pOutBeforeCollision)
				*pOutBeforeCollision = Last;
			return GetCollisionAt(Pos.x, Pos.y);
		}
		Last = Pos;
	}
	if(pOutCollision)
		*pOutCollision = Pos1;
	if(pOutBeforeCollision)
		*pOutBeforeCollision = Pos1;
	return 0;
}

// TODO: OPT: rewrite this smarter!
void CCollision::MovePoint(vec2 *pInoutPos, vec2 *pInoutVel, float Elasticity, int *pBounces)
{
	if(pBounces)
		*pBounces = 0;
	
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	if(CheckPoint(Pos + Vel))
	{
		int Affected = 0;
		if(CheckPoint(Pos.x + Vel.x, Pos.y))
		{
			pInoutVel->x *= -Elasticity;
			if(pBounces)
				(*pBounces)++;			
			Affected++;
		}

		if(CheckPoint(Pos.x, Pos.y + Vel.y))
		{
			pInoutVel->y *= -Elasticity;
			if(pBounces)
				(*pBounces)++;			
			Affected++;
		}
		
		if(Affected == 0)
		{
			pInoutVel->x *= -Elasticity;
			pInoutVel->y *= -Elasticity;
		}
	}
	else
	{
		*pInoutPos = Pos + Vel;
	}
}

bool CCollision::TestBox(vec2 Pos, vec2 Size)
{
	Size *= 0.5f;
	if(CheckPoint(Pos.x-Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y-Size.y))
		return true;
	if(CheckPoint(Pos.x-Size.x, Pos.y+Size.y))
		return true;
	if(CheckPoint(Pos.x+Size.x, Pos.y+Size.y))
		return true;
	return false;
}

void CCollision::MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity)
{
	// do the move
	vec2 Pos = *pInoutPos;
	vec2 Vel = *pInoutVel;
	
	float Distance = length(Vel);
	int Max = (int)Distance;
	
	if(Distance > 0.00001f)
	{
		//vec2 old_pos = pos;
		float Fraction = 1.0f/(float)(Max+1);
		for(int i = 0; i <= Max; i++)
		{
			//float amount = i/(float)max;
			//if(max == 0)
				//amount = 0;
			
			vec2 NewPos = Pos + Vel*Fraction; // TODO: this row is not nice
			
			if(TestBox(vec2(NewPos.x, NewPos.y), Size))
			{
				int Hits = 0;
				
				if(TestBox(vec2(Pos.x, NewPos.y), Size))
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					Hits++;
				}
				
				if(TestBox(vec2(NewPos.x, Pos.y), Size))
				{
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
					Hits++;
				}
				
				// neither of the tests got a collision.
				// this is a real _corner case_!
				if(Hits == 0)
				{
					NewPos.y = Pos.y;
					Vel.y *= -Elasticity;
					NewPos.x = Pos.x;
					Vel.x *= -Elasticity;
				}
			}
			
			Pos = NewPos;
		}
	}
	
	*pInoutPos = Pos;
	*pInoutVel = Vel;
}
