//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "CEntity.h"
#include "CAI_utils.h"
#include "CAI_memory.h"
#include "CAI_NPC.h"
#include "CAI_moveprobe.h"
#include "vphysics/object_hash.h"

//-----------------------------------------------------------------------------
CE_CTraceFilterNav::CE_CTraceFilterNav( CAI_NPC *pProber, bool bIgnoreTransientEntities, const IServerEntity *passedict, int collisionGroup, bool bAllowPlayerAvoid ) : 
	CE_CTraceFilterSimple( passedict, collisionGroup ),
	m_pProber(pProber),
	m_bIgnoreTransientEntities(bIgnoreTransientEntities),
	m_bAllowPlayerAvoid(bAllowPlayerAvoid)
{
	//CE_TODO
	//m_bCheckCollisionTable = g_EntityCollisionHash->IsObjectInHash( pProber );
}

//-----------------------------------------------------------------------------
bool CE_CTraceFilterNav::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	IServerEntity *pServerEntity = (IServerEntity*)pHandleEntity;
	CBaseEntity *pEntity = (CBaseEntity *)pServerEntity;
	CEntity *ce_pEntity = CEntity::Instance(pEntity);

	if ( m_pProber == ce_pEntity )
		return false;

	if ( m_pProber->GetMoveProbe()->ShouldBrushBeIgnored( pEntity ) == true )
		return false;

	if ( m_bIgnoreTransientEntities && (ce_pEntity->IsPlayer() || ce_pEntity->IsNPC() ) )
		return false;

	//Adrian - If I'm flagged as using the new collision method, then ignore the player when trying
	//to check if I can get somewhere.
	//CE_TODO
	/*if ( m_bAllowPlayerAvoid && m_pProber->ShouldPlayerAvoid() && pEntity->IsPlayer() )
		return false;*/

/*	if ( pEntity->IsNavIgnored() )
		return false;*/

	if ( m_bCheckCollisionTable )
	{
		/*if ( g_EntityCollisionHash->IsObjectPairInHash( m_pProber, pEntity ) )
			return false;*/
	}

/*	if ( m_pProber->ShouldProbeCollideAgainstEntity( pEntity ) == false )
		return false;*/

	return CE_CTraceFilterSimple::ShouldHitEntity( pHandleEntity, contentsMask );
}

