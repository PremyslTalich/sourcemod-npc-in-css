
#include "CAI_Memory.h"

#define	EMEMORY_POOL_SIZE		  64
#define AI_FREE_KNOWLEDGE_DURATION 1.75


CEAI_Enemies::CEAI_Enemies(void)
{
	m_flFreeKnowledgeDuration = AI_FREE_KNOWLEDGE_DURATION;
	m_flEnemyDiscardTime = AI_DEF_ENEMY_DISCARD_TIME;
	m_vecDefaultLKP = vec3_invalid;
	m_vecDefaultLSP = vec3_invalid;
	m_serial = 0;
	SetDefLessFunc( m_Map );
}


CEAI_Enemies::~CEAI_Enemies()
{
	for ( CMemMap::IndexType_t i = m_Map.FirstInorder(); i != m_Map.InvalidIndex(); i = m_Map.NextInorder( i ) )
	{
		delete m_Map[i];
	}
}

void CEAI_Enemies::SetFreeKnowledgeDuration( float flDuration )
{ 
	m_flFreeKnowledgeDuration = flDuration;	

	if ( m_flFreeKnowledgeDuration >= m_flEnemyDiscardTime )
	{
		// If your free knowledge time is greater than your discard time,
		// you'll forget about secondhand enemies passed to you by squadmates
		// as soon as you're given them.
		//Assert( m_flFreeKnowledgeDuration < m_flEnemyDiscardTime );

		m_flFreeKnowledgeDuration = m_flEnemyDiscardTime - .1;
	}
}



//-----------------------------------------------------------------------------
// Purpose: Returns last known posiiton of given enemy
//-----------------------------------------------------------------------------
const Vector &CEAI_Enemies::LastKnownPosition( CBaseEntity *pEnemy )
{
	AI_EnemyInfo_t *pMemory = Find( pEnemy, true );
	if ( pMemory )
	{
		m_vecDefaultLKP = pMemory->vLastKnownLocation;
	}
	else
	{
		DevWarning( 2,"Asking LastKnownPosition for enemy that's not in my memory!!\n");
	}
	return m_vecDefaultLKP;
}


//-----------------------------------------------------------------------------

AI_EnemyInfo_t *CEAI_Enemies::Find( CBaseEntity *pEntity, bool bTryDangerMemory )
{
	if ( pEntity == AI_UNKNOWN_ENEMY )
		pEntity = NULL;

	CMemMap::IndexType_t i = m_Map.Find( pEntity );
	if ( i == m_Map.InvalidIndex() )
	{
		if ( !bTryDangerMemory || ( i = m_Map.Find( NULL ) ) == m_Map.InvalidIndex() )
			return NULL;
		Assert(m_Map[i]->bDangerMemory == true);
	}
	return m_Map[i];
}


void CEAI_Enemies::SetEnemyDiscardTime( float flTime )
{ 
	m_flEnemyDiscardTime = flTime;			

	if ( m_flFreeKnowledgeDuration >= m_flEnemyDiscardTime )
	{
		// If your free knowledge time is greater than your discard time,
		// you'll forget about secondhand enemies passed to you by squadmates
		// as soon as you're given them.
		Assert( m_flFreeKnowledgeDuration < m_flEnemyDiscardTime );

		m_flFreeKnowledgeDuration = m_flEnemyDiscardTime - .1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Notes that the given enemy has eluded me
//-----------------------------------------------------------------------------
void CEAI_Enemies::MarkAsEluded( CBaseEntity *pEnemy )
{
	AI_EnemyInfo_t *pMemory = Find( pEnemy );
	if ( pMemory )
	{
		pMemory->bEludedMe = true;
	}
}

float CEAI_Enemies::LastTimeSeen( CBaseEntity *pEnemy, bool bCheckDangerMemory /*= true*/ )
{
	// I've never seen something that doesn't exist
	if (!pEnemy)
		return 0;

	AI_EnemyInfo_t *pMemory = Find( pEnemy, bCheckDangerMemory );
	if ( pMemory )
		return pMemory->timeLastSeen;

	if ( pEnemy != AI_UNKNOWN_ENEMY )
		DevWarning( 2,"Asking LastTimeSeen for enemy that's not in my memory!!\n");
	return AI_INVALID_TIME;
}


//-----------------------------------------------------------------------------
// Purpose:	Clear information about our enemy
//-----------------------------------------------------------------------------
void CEAI_Enemies::ClearMemory(CBaseEntity *pEnemy)
{
	CMemMap::IndexType_t i = m_Map.Find( pEnemy );
	if ( i != m_Map.InvalidIndex() )
	{
		g_pMemAlloc->Free( m_Map[i] );
		m_Map.RemoveAt( i );
	}
}

AI_EnemyInfo_t *CEAI_Enemies::GetNext( AIEnemiesIter_t *pIter )
{
	CMemMap::IndexType_t i = (CMemMap::IndexType_t)((unsigned)(*pIter));

	if ( i == m_Map.InvalidIndex() )
		return NULL;

	i = m_Map.NextInorder( i );
	*pIter = (AIEnemiesIter_t)(unsigned)i;
	if ( i == m_Map.InvalidIndex() )
		return NULL;

	if ( m_Map[i]->hEnemy == NULL )
		return GetNext( pIter );

	return m_Map[i];
}


//-----------------------------------------------------------------------------
// Purpose:	Purges any dead enemies from memory
//-----------------------------------------------------------------------------

AI_EnemyInfo_t *CEAI_Enemies::GetFirst( AIEnemiesIter_t *pIter )
{
	CMemMap::IndexType_t i = m_Map.FirstInorder();
	*pIter = (AIEnemiesIter_t)(unsigned)i;

	if ( i == m_Map.InvalidIndex() )
		return NULL;

	if ( m_Map[i]->hEnemy == NULL )
		return GetNext( pIter );

	return m_Map[i];
}

//------------------------------------------------------------------------------
// Purpose : Returns true if this enemy is part of my memory
//------------------------------------------------------------------------------
bool CEAI_Enemies::HasMemory( CBaseEntity *pEnemy )
{
	return ( Find( pEnemy ) != NULL );
}

//------------------------------------------------------------------------------
// Purpose : Returns true if this enemy is part of my memory
//------------------------------------------------------------------------------
void CEAI_Enemies::OnTookDamageFrom( CBaseEntity *pEnemy )
{
	AI_EnemyInfo_t *pMemory = Find( pEnemy, true );
	if ( pMemory )
		pMemory->timeLastReceivedDamageFrom = gpGlobals->curtime;
}

