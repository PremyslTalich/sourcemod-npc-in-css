
#include "CAI_NPC.h"
#include "CAI_Squad.h"

//-----------------------------------------------------------------------------
// Purpose: Try to get one of a contiguous range of slots
// Input  : slotIDStart - start of slot range
//			slotIDEnd - end of slot range
//			hEnemy - enemy this slot is for
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_Squad::OccupyStrategySlotRange( CEntity *pEnemy, int slotIDStart, int slotIDEnd, int *pSlot )
{
#ifndef PER_ENEMY_SQUADSLOTS
	// FIXME: combat slots need to be per enemy, not per squad.  
	// As it is, once a squad is occupied it stops making even simple attacks to other things nearby.
	// This code may make soldiers too aggressive
	if (GetLeader() && pEnemy != GetLeader()->GetEnemy())
	{
		*pSlot = SQUAD_SLOT_NONE;
		return true;
	}
#endif

	// If I'm already occupying this slot
	if ( *pSlot >= slotIDStart && *pSlot <= slotIDEnd)
		return true;

	for ( int i = slotIDStart; i <= slotIDEnd; i++ )
	{
		// Check enemy to see if slot already occupied
		if (!IsSlotOccupied(pEnemy, i))
		{
			// Clear any previous spot;
			if (*pSlot != SQUAD_SLOT_NONE)
			{
				// As a debug measure check to see if slot was filled
				if (!IsSlotOccupied(pEnemy, *pSlot))
				{
					DevMsg( "ERROR! Vacating an empty slot!\n");
				}

				// Free the slot
				VacateSlot(pEnemy, *pSlot);
			}

			// Fill the slot
			OccupySlot(pEnemy, i);
			*pSlot = i;
			return true;
		}
	}
	return false;
}


//------------------------------------------------------------------------------

bool CAI_Squad::IsSlotOccupied( CEntity *pEnemy, int i ) const	
{ 
	const AISquadEnemyInfo_t *pInfo = FindEnemyInfo( pEnemy );
	return pInfo->slots.IsBitSet(i);
}


//------------------------------------------------------------------------------

void CAI_Squad::VacateSlot( CEntity *pEnemy, int i )			
{ 
	AISquadEnemyInfo_t *pInfo = FindEnemyInfo( pEnemy );
	pInfo->slots.Clear(i);
}

//------------------------------------------------------------------------------

void CAI_Squad::OccupySlot( CEntity *pEnemy, int i )			
{ 
	AISquadEnemyInfo_t *pInfo = FindEnemyInfo( pEnemy );
	pInfo->slots.Set(i);

}

AISquadEnemyInfo_t *CAI_Squad::FindEnemyInfo( CEntity *pEnemy )
{
	int i;
	if ( gpGlobals->curtime > m_flEnemyInfoCleanupTime )
	{
		if ( m_EnemyInfos.Count() )
		{
			m_pLastFoundEnemyInfo = NULL;
			CUtlRBTree<CBaseEntity *> activeEnemies;
			SetDefLessFunc( activeEnemies );

			// Gather up the set of active enemies

			for ( i = 0; i < m_SquadMembers.Count(); i++ )
			{
				CAI_NPC *npc = CEntity::Instance(m_SquadMembers[i])->MyNPCPointer();
				CEntity *pMemberEnemy = npc->GetEnemy();
				if ( pMemberEnemy && activeEnemies.Find( pMemberEnemy->BaseEntity() ) == activeEnemies.InvalidIndex() )
				{
					activeEnemies.Insert( pMemberEnemy->BaseEntity() );
				}
			}
			
			// Remove the records for deleted or unused enemies
			for ( i = m_EnemyInfos.Count() - 1; i >= 0; --i )
			{
				if ( m_EnemyInfos[i].hEnemy == NULL || activeEnemies.Find( m_EnemyInfos[i].hEnemy ) == activeEnemies.InvalidIndex() )
				{
					m_EnemyInfos.FastRemove( i );
				}
			}
		}
		
		m_flEnemyInfoCleanupTime = gpGlobals->curtime + 30;
	}

	if ( m_pLastFoundEnemyInfo && m_pLastFoundEnemyInfo->hEnemy == pEnemy->BaseEntity())
		return m_pLastFoundEnemyInfo;

	for ( i = 0; i < m_EnemyInfos.Count(); i++ )
	{
		if ( m_EnemyInfos[i].hEnemy == pEnemy->BaseEntity() )
		{
			m_pLastFoundEnemyInfo = &m_EnemyInfos[i];
			return &m_EnemyInfos[i];
		}
	}

	m_pLastFoundEnemyInfo = NULL;
	i = m_EnemyInfos.AddToTail();
	m_EnemyInfos[i].hEnemy = pEnemy->BaseEntity();

	m_pLastFoundEnemyInfo = &m_EnemyInfos[i];
	return &m_EnemyInfos[i];
}
