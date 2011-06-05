/* Pathfind class by Sushi and Patafix*/
#include <engine/shared/config.h>

#include "gamecontext.h"
#include "pathfind.h"

CPathfind::CPathfind(CGameContext *pGameServer) : m_pGameServer(pGameServer)
{
	m_StartIndex = -1;
	m_EndIndex = -1;
	
	m_ReachableTilesNum = 0;
	
	m_LayerWidth = m_pGameServer->Layers()->GameLayer()->m_Width;
	m_LayerHeight = m_pGameServer->Layers()->GameLayer()->m_Height;
	
	// set size for array
	m_lMap.set_size(m_LayerWidth*m_LayerHeight);
	m_lNodes.set_size(m_LayerWidth*m_LayerHeight);
	m_lFinalPath.set_size(g_Config.m_SvMaxWayCalc);
	m_lReachableTiles.set_size(m_LayerWidth*m_LayerHeight);
	
	// init size for heap
	m_Open.SetSize(4*g_Config.m_SvMaxWayCalc);
	
	CNode Node;
	vec2 NodePos = vec2(-1, -1);
	int Hit = 0;
	int TmpHit = 0;
	float Angle = 0;
	int ID = 0;
	for(int i = 0; i < m_LayerHeight; i++)
	{
		for(int j = 0; j < m_LayerWidth; j++)
		{
			Node.m_Pos = vec2(j,i);
			Node.m_Parent = -1;
			Node.m_ID = ID;
			Node.m_G = 0;
			Node.m_H = 0;
			Node.m_F = 0;
			Node.m_Flag = 0;
			if(m_pGameServer->Collision()->CheckPoint(j*32+16, i*32+16))
				Node.m_Flag |= COL;
			
			// check collision near node
			NodePos = vec2(Node.m_Pos.x*32+16, Node.m_Pos.y*32+16);
			if(!(Node.m_Flag&COL))
			{
				Hit = 0;
				TmpHit = 0;
				Angle = 0;
				
				// do a circle
				for(int i = 0; i < 16; i++)
				{
					Angle += 3.141593f/8;
					TmpHit = 0;
					TmpHit = m_pGameServer->Collision()->FastIntersectLine(NodePos, NodePos+vec2(cosf(Angle), sinf(Angle))*m_pGameServer->Tuning()->m_HookLength, 0x0, 0x0, 32);
					
					if(TmpHit > 0)
						Hit = TmpHit;
						
					if(Hit&CCollision::COLFLAG_SOLID && !(Hit&CCollision::COLFLAG_NOHOOK))
						break;
						
					if(TmpHit && i == 3)
					{
						Hit = CCollision::COLFLAG_SOLID;
						break;
					}
				}
				
				if(!Hit)
					Node.m_Flag |= NOTREACHABLE;
				else if(Hit&CCollision::COLFLAG_NOHOOK)
					Node.m_Flag |= UNHOOKABLE;

		
			}
			//add the note to the list
			m_lMap[i*m_LayerWidth+j] = Node;
			// store reachable tiles in a array
			if(Node.m_Flag == 0)
			{
				if(m_pGameServer->Collision()->FastIntersectLine(NodePos, NodePos+vec2(0,1)*(m_pGameServer->Tuning()->m_HookLength*1.5f), 0x0, 0x0))
				{
					m_lReachableTiles[m_ReachableTilesNum] = NodePos;
					m_ReachableTilesNum++;
				}
			}
			
			ID++;
		}
	}

	int Index = 0;
	int IndexA = 0;
	int IndexB = 0;
	int IndexC = 0;
	bool IsAvoidable = true;
	bool IsNearSolid = false;
	for(int i = 0; i < m_LayerHeight; i++)
	{
		for(int j = 0; j < m_LayerWidth; j++)
		{
			Index = (i*m_LayerWidth+j);
			IsAvoidable = true;
			IsNearSolid = false;
			if(Index < 0 || Index >= m_lMap.size() || m_pGameServer->Collision()->GetTileIndex(j*32+16, i*32+16) > 0)
					continue;
									
			for(int a = 0; a < 16;a++)
			{
				switch(a)
				{
				case 0:
					IndexA = Index + 1;
					IndexB = Index - 1;
					IndexC = 0;
					break;
				case 1:
					IndexA = Index + m_LayerWidth;
					IndexB = Index - m_LayerWidth;
					IndexC = 0;
					break;
				case 2:
					IndexA = Index + 1 + m_LayerWidth;
					IndexB = Index - 1 - m_LayerWidth;
					IndexC = 0;
					break;
				case 3:
					IndexA = Index + 1 - m_LayerWidth;
					IndexB = Index - 1 + m_LayerWidth;
					IndexC = 0;
					break;
				case 4:
					IndexA = Index - 1 - m_LayerWidth;
					IndexB = Index + 1 - m_LayerWidth;
					IndexC = Index - m_LayerWidth;
					break;
				case 5:
					IndexA = Index + 1 - m_LayerWidth;
					IndexB = Index + 1 + m_LayerWidth;
					IndexC = Index + 1;
					break;
				case 6:
					IndexA = Index + 1 + m_LayerWidth;
					IndexB = Index - 1 + m_LayerWidth;
					IndexC = Index + m_LayerWidth;
					break;
				case 7:
					IndexA = Index - 1 + m_LayerWidth;
					IndexB = Index - 1 - m_LayerWidth;
					IndexC = Index - 1;
					break;
				case 8:
					IndexA = Index - 1 - m_LayerWidth;
					IndexB = Index + 1;
					IndexC = Index - m_LayerWidth;
					break;
				case 9:
					IndexA = Index + 1 - m_LayerWidth;
					IndexB = Index - 1;
					IndexC = Index - m_LayerWidth;
					break;
				case 10:
					IndexA = Index + 1 + m_LayerWidth;
					IndexB = Index - 1;
					IndexC = Index + m_LayerWidth;
					break;
				case 11:
					IndexA = Index - 1 + m_LayerWidth;
					IndexB = Index + 1;
					IndexC = Index + m_LayerWidth;
					break;
				case 12:
					IndexA = Index + 1 - m_LayerWidth;
					IndexB = Index + m_LayerWidth;
					IndexC = Index + 1;
					break;
				case 13:
					IndexA = Index - 1 - m_LayerWidth;
					IndexB = Index  + m_LayerWidth;
					IndexC = Index - 1;
					break;
				case 14:
					IndexA = Index + 1 + m_LayerWidth;
					IndexB = Index - m_LayerWidth;
					IndexC = Index + 1;
					break;
				case 15:
					IndexA = Index - 1 + m_LayerWidth;
					IndexB = Index - m_LayerWidth;
					IndexC = Index - 1;
					break;
				}

				if(IndexA < 0 || IndexB < 0 || IndexA >= m_lMap.size() || IndexB >= m_lMap.size())
					continue;	
				
				if((m_lMap[IndexA].m_Flag&COL) || (m_lMap[IndexB].m_Flag&COL))
					IsNearSolid = true;
				
				if((m_lMap[IndexA].m_Flag&COL) && (m_lMap[IndexB].m_Flag&COL))
				{
					IsAvoidable = false;	
					
				}
				else if((m_lMap[IndexA].m_Flag&COL) && (m_lMap[IndexB].m_Flag&AVOIDABLE) && (a < 4 || !(m_lMap[IndexC].m_Flag&COL)))
				{	
					m_lMap[IndexB].m_Flag ^= AVOIDABLE;		
					IsAvoidable = false;		
					
				}
				else if((m_lMap[IndexB].m_Flag&COL) && (m_lMap[IndexA].m_Flag&AVOIDABLE) && (a < 4 || !(m_lMap[IndexC].m_Flag&COL)))
				{						
					m_lMap[IndexA].m_Flag ^= AVOIDABLE;					
					IsAvoidable = false;				
				}
			}

			if(IsAvoidable && IsNearSolid)
				m_lMap[Index].m_Flag |= AVOIDABLE;
		}		
	}
}
void CPathfind::Init()
{
	m_Cancled = false;
	
	// set fake sizes
	m_ClosedNodes = 0;
	m_FinalSize = 0;
	
	// empty the heap
	m_Open.MakeEmpty();
		
	// copy map list into Nodes list
	m_lNodes = m_lMap;
}

void CPathfind::SetStart(vec2 Pos)
{
	Pos.x = clamp((int)Pos.x, 0, m_LayerWidth*32);
	Pos.y = clamp((int)Pos.y, 0, m_LayerHeight*32);
	int Index = GetIndex((int)(Pos.x/32), (int)(Pos.y/32));
	
	m_lNodes[Index].m_Parent = START;
	m_lNodes[Index].m_Flag |= CLOSED;
	//m_lClosed[m_ClosedSize] = m_lNodes[Index];
	m_ClosedNodes++;
	m_StartIndex = Index;
}

void CPathfind::SetEnd(vec2 Pos)
{	
	int Index = GetIndex((int)(Pos.x/32), (int)(Pos.y/32));
	
	
	if((m_lNodes[Index].m_Flag&AVOIDABLE))
	{
			if(Index - 1 >= 0 && !(m_lNodes[Index -1].m_Flag&(COL|AVOIDABLE)))
				Index -= 1;
			else if(Index + 1 < m_lMap.size() && !(m_lNodes[Index + 1].m_Flag&(COL|AVOIDABLE)))
				Index += 1;
			else if(Index - m_pGameServer->Layers()->GameLayer()->m_Width >= 0 && !(m_lNodes[Index - m_pGameServer->Layers()->GameLayer()->m_Width].m_Flag&(COL|AVOIDABLE)))
				Index -= m_pGameServer->Layers()->GameLayer()->m_Width;
			else if(Index + m_pGameServer->Layers()->GameLayer()->m_Width < m_lMap.size() && !(m_lNodes[Index + m_pGameServer->Layers()->GameLayer()->m_Width].m_Flag&(COL|AVOIDABLE)))
				Index += m_pGameServer->Layers()->GameLayer()->m_Width;

	}
	m_lNodes[Index].m_Parent = END;

	m_EndIndex = Index; 
}


int CPathfind::GetIndex(int XPos, int YPos)
{
	return XPos + m_pGameServer->Layers()->GameLayer()->m_Width * YPos;
}

void CPathfind::FindPath()
{
	if(m_StartIndex < 0 || m_EndIndex < 0)
		return;
	
	int CurrentIndex = m_StartIndex;
	int WorkingIndex;
	
	bool UseUnhookable = false;
	
	// check direction between start and end
	if(m_StartIndex/m_LayerWidth <= m_EndIndex/m_LayerWidth)
		UseUnhookable = true;
	
	// begin the search
	while(m_lNodes[CurrentIndex].m_ID != m_EndIndex)
	{
		// cancel the pathfinding
		if(m_ClosedNodes >= g_Config.m_SvMaxWayCalc)
		{
			m_Cancled = true;
			break;
		}
		
		unsigned char AddFlag = 0;
		for(int i = 0; i < 8; i++)
		{
			WorkingIndex = -1;
			// get the working index
			
			switch(i)
			{
			case 0:
				if(CurrentIndex+1 < m_lNodes.size())
					WorkingIndex = CurrentIndex+1; // DOWN
				break;
			case 1:
				if(CurrentIndex-1 >= 0)
					WorkingIndex = CurrentIndex-1; // UP
				break;
			case 2:
				if(CurrentIndex+m_LayerWidth < m_lNodes.size())
					WorkingIndex = CurrentIndex+m_LayerWidth; // RIGHT
				break;
			case 3:
				if(CurrentIndex-m_LayerWidth >= 0)
					WorkingIndex = CurrentIndex-m_LayerWidth;	//LEFT
				break;
			case 4:
				if((CurrentIndex + m_LayerWidth) - 1 >= 0 && CurrentIndex + m_LayerWidth - 1 < m_lNodes.size())
					WorkingIndex = (CurrentIndex + m_LayerWidth) - 1;	// UP-RIGHT
				break;
			case 5:
				if(CurrentIndex + m_LayerWidth + 1 < m_lNodes.size())
					WorkingIndex = (CurrentIndex + m_LayerWidth) + 1;	// DOWN-RIGHT
				break;
			case 6:
				if((CurrentIndex - m_LayerWidth) + 1 < m_lNodes.size() && CurrentIndex - m_LayerWidth + 1 >= 0)
					WorkingIndex = (CurrentIndex - m_LayerWidth) + 1;	// DOWN-LEFT
				break;
			case 7:
				if(CurrentIndex - m_LayerWidth - 1 >= 0)
					WorkingIndex = (CurrentIndex - m_LayerWidth) - 1;	// UP-LEFT
				break;
			}
						
			if(WorkingIndex > -1 && !(m_lNodes[WorkingIndex].m_Flag&(COL|CLOSED|NOTREACHABLE|AVOIDABLE)) && (UseUnhookable || !(m_lNodes[WorkingIndex].m_Flag&UNHOOKABLE)))
			{
				// set the flag
				AddFlag |= 1<<i;
				
				if(!(m_lNodes[WorkingIndex].m_Flag&OPEN))
				{
					// set its parent
					m_lNodes[WorkingIndex].m_Parent = CurrentIndex;

					// calculate the important values
					m_lNodes[WorkingIndex].m_G = i < 4 ?  m_lNodes[CurrentIndex].m_G + 10 : m_lNodes[CurrentIndex].m_G + 14;
					m_lNodes[WorkingIndex].m_H = (abs(m_lNodes[WorkingIndex].m_Pos.x - m_lNodes[m_EndIndex].m_Pos.x) + abs(m_lNodes[WorkingIndex].m_Pos.y - m_lNodes[m_EndIndex].m_Pos.y));
					m_lNodes[WorkingIndex].m_F = m_lNodes[WorkingIndex].m_G + m_lNodes[WorkingIndex].m_H;

					m_lNodes[WorkingIndex].m_Flag |= OPEN;

					m_Open.Insert(m_lNodes[WorkingIndex]);
				}
				else
				{
					// recalculate the G and F Value
					if((i < 4 && m_lNodes[WorkingIndex].m_G > m_lNodes[CurrentIndex].m_G + 10) || (i > 3 && m_lNodes[WorkingIndex].m_G > m_lNodes[CurrentIndex].m_G + 14))
					{
						// set new parent
						m_lNodes[WorkingIndex].m_Parent = CurrentIndex;

						// important values (H value wont change)
						m_lNodes[WorkingIndex].m_G = i < 4 ?  m_lNodes[CurrentIndex].m_G + 10 : m_lNodes[CurrentIndex].m_G + 14;
						m_lNodes[WorkingIndex].m_F = m_lNodes[WorkingIndex].m_G + m_lNodes[WorkingIndex].m_H;

						m_Open.Replace(m_lNodes[WorkingIndex]);
					}
				}
			}
		}

		if(m_Open.GetSize() < 1)
			return;
		
		// get Lowest F from heap and set new CurrentIndex
		CurrentIndex = m_Open.GetMin()->m_ID;
		
		// delete it \o/
		m_Open.RemoveMin();
			
		// set the one with lowest score to closed list and begin new from there
		m_lNodes[CurrentIndex].m_Flag |= CLOSED;
		m_ClosedNodes++;
	}
	
	// go backwards and return final path :-O		
	while(m_lNodes[CurrentIndex].m_ID != m_StartIndex)
	{
		m_lFinalPath[m_FinalSize] = m_lNodes[CurrentIndex];
		m_FinalSize++;
		
		// get next node
		CurrentIndex = m_lNodes[CurrentIndex].m_Parent;
	}
}
