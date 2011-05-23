/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "pickup.h"

CPickup::CPickup(CGameWorld *pGameWorld, int Type, int SubType)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Type = Type;
	m_Subtype = SubType;
	m_ProximityRadius = PickupPhysSize;

	Reset();

	GameWorld()->InsertEntity(this);
}

void CPickup::Reset()
{
	for(int i = 0; i < 2; i++)
	{
		if(g_pData->m_aPickups[m_Type].m_Spawndelay > 0)
			m_aSpawnTick[i] = Server()->Tick() + Server()->TickSpeed() * g_pData->m_aPickups[m_Type].m_Spawndelay;
		else
			m_aSpawnTick[i] = -1;
	}
}

void CPickup::Tick()
{
	// wait for respawn
	for(int i = 0; i < 2; i++)
	{
		if(m_aSpawnTick[i] > 0)
		{
			if(Server()->Tick() > m_aSpawnTick[i])
			{
				// respawn
				m_aSpawnTick[i] = -1;

				if(m_Type == POWERUP_WEAPON)
					GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN, CmaskCatching(GameServer(), (bool)i));
			}
		}
	}

	// Check if a player intersected us
	CCharacter *apChrs[MAX_CLIENTS];
	int Num = GameServer()->m_World.FindEntities(m_Pos, 20.0f, (CEntity**)apChrs, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	for(int i = 0; i < Num; i++)
	{
		CCharacter *pChr = apChrs[i];
		int Joined = (pChr->GetPlayer()->IsJoined() || pChr->GetPlayer()->GetTeam() == TEAM_SPECTATORS) ? 1 : 0;

		if(pChr->IsAlive() && m_aSpawnTick[Joined] == -1)
		{
			// player picked us up, is someone was hooking us, let them go
			int RespawnTime = -1;
			switch (m_Type)
			{
				case POWERUP_HEALTH:
					if(pChr->IncreaseHealth(1))
					{
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, CmaskCatching(GameServer(), (bool)Joined));
						RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					}
					break;

				case POWERUP_ARMOR:
					if(pChr->IncreaseArmor(1))
					{
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, CmaskCatching(GameServer(), (bool)Joined));
						RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
					}
					break;

				case POWERUP_WEAPON:
					if(m_Subtype >= 0 && m_Subtype < NUM_WEAPONS)
					{
						if(pChr->GiveWeapon(m_Subtype, 10))
						{
							RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

							if(m_Subtype == WEAPON_GRENADE)
								GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, CmaskCatching(GameServer(), (bool)Joined));
							else if(m_Subtype == WEAPON_SHOTGUN)
								GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, CmaskCatching(GameServer(), (bool)Joined));
							else if(m_Subtype == WEAPON_RIFLE)
								GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, CmaskCatching(GameServer(), (bool)Joined));

							if(pChr->GetPlayer())
								GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
						}
					}
					break;

				case POWERUP_NINJA:
					{
						// activate ninja on target player
						pChr->GiveNinja();
						RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

						// loop through all players, setting their emotes
						CCharacter *pC = static_cast<CCharacter *>(GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
						for(; pC; pC = (CCharacter *)pC->TypeNext())
						{
							if(pC != pChr && pChr->GetPlayer()->IsJoined() == (bool)Joined)
								pC->SetEmote(EMOTE_SURPRISE, Server()->Tick() + Server()->TickSpeed());
						}

						pChr->SetEmote(EMOTE_ANGRY, Server()->Tick() + 1200 * Server()->TickSpeed() / 1000);
						break;
					}

				default:
					break;
			};

			if(RespawnTime >= 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "pickup player='%d:%s' item=%d/%d",
					pChr->GetPlayer()->GetCID(), Server()->ClientName(pChr->GetPlayer()->GetCID()), m_Type, m_Subtype);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
				m_aSpawnTick[Joined] = Server()->Tick() + Server()->TickSpeed() * RespawnTime;
			}
		}
	}
}

void CPickup::Snap(int SnappingClient)
{
	int joined = (GameServer()->m_apPlayers[SnappingClient]->IsJoined() || GameServer()->m_apPlayers[SnappingClient]->GetTeam() == TEAM_SPECTATORS) ? 1 : 0;
	if(m_aSpawnTick[joined] != -1 || NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
	pP->m_Subtype = m_Subtype;
}
