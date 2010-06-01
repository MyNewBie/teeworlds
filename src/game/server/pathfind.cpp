/* Pathfind class by Sushi */
#include <engine/shared/config.h>
#include "gamecontroller.h"

CPathfind::CPathfind(CGameContext *pGameServer) : m_pGameServer(pGameServer)
{
	m_ClosedNodes = 0;
	m_FinalSize = 0;
	m_StartIndex = -1;
	m_EndIndex = -1;
	
	m_LayerWidth = m_pGameServer->Layers()->GameLayer()->m_Width;
	m_LayerHeight = m_pGameServer->Layers()->GameLayer()->m_Height;
	
	// set size for array
	m_lMap.set_size(m_LayerWidth*m_LayerHeight);
	m_lNodes.set_size(m_LayerWidth*m_LayerHeight);
	m_lFinalPath.set_size(g_Config.m_SvMaxWayCalc);
	
	// init size for heap
	m_Open.SetSize(4*g_Config.m_SvMaxWayCalc);
	
	int ID = 0;
	for(int i = 0; i < m_LayerHeight; i++)
	{
		for(int j = 0; j < m_LayerWidth; j++)
		{
			CNode Node;
			Node.m_Pos = vec2(j,i);
			Node.m_Parent = -1;
			Node.m_ID = ID;
			Node.m_G = 0;
			Node.m_H = 0;
			Node.m_F = 0;
			Node.m_IsClosed = false;
			Node.m_IsOpen = false;
			if(m_pGameServer->Collision()->CheckPoint(j*32+16, i*32+16))
				Node.m_IsCol = true;
			else
				Node.m_IsCol = false;
			
			// add the node to the list
			m_lMap[i*m_LayerWidth+j] = Node;
			
			ID++;
		}
	}
}

void CPathfind::Init()
{
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
	m_lNodes[Index].m_IsClosed = true;
	//m_lClosed[m_ClosedSize] = m_lNodes[Index];
	m_ClosedNodes++;
	m_StartIndex = Index;
}

void CPathfind::SetEnd(vec2 Pos)
{	
	int Index = GetIndex((int)(Pos.x/32), (int)(Pos.y/32));
	
	m_lNodes[Index].m_Parent = END;
	m_EndIndex = Index; 
}


int CPathfind::GetIndex(int XPos, int YPos)
{
	return XPos + m_pGameServer->Layers()->GameLayer()->m_Width * YPos;
}

void CPathfind::FindPath()
{
	int CurrentIndex = m_StartIndex;
	
	// begin the search
	if(m_StartIndex > -1 && m_EndIndex > -1)
	{
		while(m_ClosedNodes < g_Config.m_SvMaxWayCalc && m_lNodes[CurrentIndex].m_ID != m_EndIndex)
		{
			for(int i = 0; i < 4; i++)
			{
				// get the working index
				int WorkingIndex = -1;
				
				switch(i)
				{
				case 0:
					if(CurrentIndex+1 < m_lNodes.size())
						WorkingIndex = CurrentIndex+1;
					break;
				case 1:
					if(CurrentIndex-1 >= 0)
						WorkingIndex = CurrentIndex-1;
					break;
				case 2:
					if(CurrentIndex+m_LayerWidth < m_lNodes.size())
						WorkingIndex = CurrentIndex+m_LayerWidth;
					break;
				case 3:
					if(CurrentIndex-m_LayerWidth >= 0)
						WorkingIndex = CurrentIndex-m_LayerWidth;
				}
				
				if(WorkingIndex > -1 && !m_lNodes[WorkingIndex].m_IsCol && !m_lNodes[WorkingIndex].m_IsClosed)
				{			
					if(!m_lNodes[WorkingIndex].m_IsOpen)
					{
						// set its parent
						m_lNodes[WorkingIndex].m_Parent = CurrentIndex;
						
						// calculate the important values
						m_lNodes[WorkingIndex].m_G = m_lNodes[CurrentIndex].m_G + 1;
						m_lNodes[WorkingIndex].m_H = (abs(m_lNodes[WorkingIndex].m_Pos.x - m_lNodes[m_EndIndex].m_Pos.x) + abs(m_lNodes[WorkingIndex].m_Pos.y - m_lNodes[m_EndIndex].m_Pos.y));
						m_lNodes[WorkingIndex].m_F = m_lNodes[WorkingIndex].m_G + m_lNodes[WorkingIndex].m_H;
						
						m_lNodes[WorkingIndex].m_IsOpen = true;

						m_Open.Insert(m_lNodes[WorkingIndex]);
					}
					else
					{
						// recalculate the G and F Value
						if(m_lNodes[WorkingIndex].m_G > m_lNodes[CurrentIndex].m_G + 1)
						{
							// set new parent
							m_lNodes[WorkingIndex].m_Parent = CurrentIndex;
							
							// important values (H value wont change)
							m_lNodes[WorkingIndex].m_G = m_lNodes[CurrentIndex].m_G + 1;
							m_lNodes[WorkingIndex].m_F = m_lNodes[WorkingIndex].m_G + m_lNodes[WorkingIndex].m_H;
							
							m_Open.Replace(m_lNodes[WorkingIndex]);
						}
					}
				}
			}
			
			if(m_Open.GetSize() < 1)
			{
				dbg_msg("pathfind", "No way can be found");
				return;
			}
			
			// get Lowest F from heap and set new CurrentIndex
			CurrentIndex = m_Open.GetMin()->m_ID;
			
			// delete it \o/
			m_Open.RemoveMin();
				
			// set the one with lowest score to closed list and begin new from there
			m_lNodes[CurrentIndex].m_IsClosed = true;
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
}
