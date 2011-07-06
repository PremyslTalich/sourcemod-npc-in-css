#include "CAI_Hint.h"
#include "CPlayer.h"
#include "CAI_Network.h"
#include "CAI_node.h"


//==================================================
// CHintCriteria
//==================================================

CHintCriteria::CHintCriteria( void )
{
	m_iFirstHintType = HINT_NONE;
	m_iLastHintType = HINT_NONE;
	m_strGroup		= NULL_STRING;
	m_iFlags		= 0;
	m_HintTypes.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHintCriteria::~CHintCriteria( void )
{
	m_zoneInclude.Purge();
	m_zoneExclude.Purge();
	m_HintTypes.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Sets the hint type for this search criteria
// Input  : nHintType - the hint type for this search criteria
//-----------------------------------------------------------------------------
void CHintCriteria::SetHintType( int nHintType )
{
	m_iFirstHintType = nHintType;
	m_iLastHintType = HINT_NONE;
	m_HintTypes.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Add another type of hint that matches the search criteria
//-----------------------------------------------------------------------------
void CHintCriteria::AddHintType( int hintType )
{
	m_HintTypes.AddToTail( hintType );
}

int CHintCriteria::NumHintTypes() const
{
	return m_HintTypes.Count();
}

int	CHintCriteria::GetHintType( int idx ) const
{
	return m_HintTypes[ idx ];
}

bool CHintCriteria::MatchesSingleHintType() const
{ 
	if ( m_HintTypes.Count() != 0 )
	{
		return false;
	}

	if ( m_iFirstHintType != HINT_ANY && 
		    m_iLastHintType == HINT_NONE )
	{
		return true;
	}
	
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHintCriteria::MatchesHintType( int hintType ) const
{
	int c = m_HintTypes.Count();
 	for ( int i = 0; i < c; ++i )
	{
		if ( m_HintTypes[i] == hintType )
			return true;
	}

	// See if we're trying to filter the nodes
	if ( GetFirstHintType() != HINT_ANY )
	{
		if( GetLastHintType() == HINT_NONE )
		{
			// Searching for a single type of hint.
			if( GetFirstHintType() != hintType )
				return false;
		}
		else
		{
			// This search is for a range of hint types.
			if( hintType < GetFirstHintType() || hintType > GetLastHintType() )
				return false;
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Allows us to search for nodes within a range of consecutive types.
//-----------------------------------------------------------------------------
void CHintCriteria::SetHintTypeRange( int firstType, int lastType )
{
	if( lastType < firstType )
	{
		DevMsg( 2, "Hint Type Range is backwards - Fixing up.\n" );

		int temp;

		temp = firstType;
		firstType = lastType;
		lastType = temp;
	}

	m_iFirstHintType = firstType;
	m_iLastHintType = lastType;
	m_HintTypes.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bitmask - 
//-----------------------------------------------------------------------------
void CHintCriteria::SetFlag( int bitmask )
{
	m_iFlags |= bitmask;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bitmask - 
//-----------------------------------------------------------------------------
void CHintCriteria::ClearFlag( int bitmask )
{
	m_iFlags &= ~bitmask;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : group - 
//-----------------------------------------------------------------------------
void CHintCriteria::SetGroup( string_t group )
{
	m_strGroup = group;
}

//-----------------------------------------------------------------------------
// Purpose: Adds a zone to a zone list
// Input  : list - the list of zones to add the new zone to
//			&position - the origin point of the zone
//			radius - the radius of the zone
//-----------------------------------------------------------------------------
void CHintCriteria::AddZone( zoneList_t &list, const Vector &position, float radius )
{
	int id = list.AddToTail();
	list[id].position	= position;
	list[id].radiussqr	= radius*radius;
}

//-----------------------------------------------------------------------------
// Purpose: Adds an include zone to the search criteria
// Input  : &position - the origin point of the zone
//			radius - the radius of the zone
//-----------------------------------------------------------------------------
void CHintCriteria::AddIncludePosition( const Vector &position, float radius )
{
	AddZone( m_zoneInclude, position, radius );
}

//-----------------------------------------------------------------------------
// Purpose: Adds an exclude zone to the search criteria
// Input  : &position - the origin point of the zone
//			radius - the radius of the zone
//-----------------------------------------------------------------------------
void CHintCriteria::AddExcludePosition( const Vector &position, float radius )
{
	AddZone( m_zoneExclude, position, radius );
}

//-----------------------------------------------------------------------------
// Purpose: Test to see if this position falls within any of the zones in the list
// Input  : *zone - list of zones to test against
//			&testPosition - position to test with
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CHintCriteria::InZone( const zoneList_t &zone, const Vector &testPosition ) const
{
	int	numZones = zone.Count();

	//Iterate through all zones in the list
	for ( int i = 0; i < numZones; i++ )
	{
		if ( ((zone[i].position) - testPosition).LengthSqr() < (zone[i].radiussqr) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Determine if a point within our include list
// Input  : &testPosition - position to test with
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHintCriteria::InIncludedZone( const Vector &testPosition ) const
{
	return InZone( m_zoneInclude, testPosition );
}

//-----------------------------------------------------------------------------
// Purpose: Determine if a point within our exclude list
// Input  : &testPosition - position to test with
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHintCriteria::InExcludedZone( const Vector &testPosition ) const
{
	return InZone( m_zoneExclude, testPosition );
}



DEFINE_PROP(m_NodeData, CE_AI_Hint);
DEFINE_PROP(m_flNextUseTime, CE_AI_Hint);
DEFINE_PROP(m_hHintOwner, CE_AI_Hint);
DEFINE_PROP(m_nTargetNodeID, CE_AI_Hint);


bool CE_AI_Hint::IsLocked( void )
{
	if (m_NodeData.ptr->iDisabled)
	{
		return true;
	}

	if (gpGlobals->curtime < m_flNextUseTime)
	{
		return true;
	}
	
	if (m_hHintOwner.ptr != NULL)
	{
		return true;
	}
	return false;
}

bool CE_AI_Hint::Lock( CBaseEntity* pNPC )
{
	if ( *(m_hHintOwner.ptr) != pNPC && *(m_hHintOwner.ptr) != NULL )
		return false;
	*(m_hHintOwner.ptr) = pNPC;
	return true;
}

void CE_AI_Hint::GetPosition(CCombatCharacter *pBCC, Vector *vPosition)
{
	if ( m_NodeData.ptr->nNodeID != NO_NODE )
	{
		*vPosition = g_pBigAINet->GetNodePosition( pBCC, m_NodeData.ptr->nNodeID );
	}
	else
	{
		*vPosition = GetAbsOrigin();
	}
}

void CE_AI_Hint::Unlock( float delay )
{
	*(m_hHintOwner.ptr)	= NULL;
	m_flNextUseTime = gpGlobals->curtime + delay;
}

bool CE_AI_Hint::IsLockedBy( CEntity *pNPC )
{
	CBaseEntity *cbase = (pNPC) ? pNPC->BaseEntity() : NULL;
	return (m_hHintOwner == cbase);
};

void CE_AI_Hint::DisableForSeconds( float flSeconds )
{
	Unlock( flSeconds );
}

Vector CE_AI_Hint::GetDirection()
{
	return UTIL_YawToVector( Yaw() );
}

float CE_AI_Hint::Yaw(void)
{
	if (m_NodeData->nNodeID != NO_NODE)
	{
		return g_pBigAINet->GetNodeYaw(m_NodeData->nNodeID );
	}
	else
	{
		return GetLocalAngles().y;
	}
}
/*
//-----------------------------------------------------------------------------
// Purpose: Returns true is hint node is open for use
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CAI_Hint::IsLocked( void )
{
	if (m_NodeData.iDisabled)
	{
		return true;
	}

	if (gpGlobals->curtime < m_flNextUseTime)
	{
		return true;
	}
	
	if (m_hHintOwner != NULL)
	{
		return true;
	}
	return false;
}



//-----------------------------------------------------------------------------
// Purpose: Locks the node for use by an AI for hints
// Output : Returns true if the node was available for locking, false on failure.
//-----------------------------------------------------------------------------
bool CAI_Hint::Lock( CBaseEntity* pNPC )
{
	if ( m_hHintOwner != pNPC && m_hHintOwner != NULL )
		return false;
	m_hHintOwner = pNPC;
	return true;
}

//------------------------------------------------------------------------------
// Purpose :  If connected to a node returns node position, otherwise
//			  returns local hint position
//
//			  NOTE: Assumes not using multiple AI networks  
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CAI_Hint::GetPosition(CCombatCharacter *pBCC, Vector *vPosition)
{
	if ( m_NodeData.nNodeID != NO_NODE )
	{
		*vPosition = g_pBigAINet->GetNodePosition( pBCC, m_NodeData.nNodeID );
	}
	else
	{
		*vPosition = GetAbsOrigin();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Unlocks the node, making it available for hint use by other AIs.
//			after the given delay time
//-----------------------------------------------------------------------------
void CAI_Hint::Unlock( float delay )
{
	m_hHintOwner	= NULL;
	m_flNextUseTime = gpGlobals->curtime + delay;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true is hint node is open for use
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CAI_Hint::IsLockedBy( CEntity *pNPC )
{
	CBaseEntity *cbase = (pNPC) ? pNPC->BaseEntity() : NULL;
	return (m_hHintOwner == cbase);
}
*/




//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *token - 
// Output : int
//-----------------------------------------------------------------------------
int CAI_HintManager::GetFlags( const char *token )
{
	int len = strlen( token );
	if ( len <= 0 )
	{
		return bits_HINT_NODE_NONE;
	}

	char *lowercase = (char *)_alloca( len + 1 );
	Q_strncpy( lowercase, token, len+1 );
	Q_strlower( lowercase );

	if ( strstr( "none", lowercase ) )
	{
		return bits_HINT_NODE_NONE;
	}

	int bits = 0;

	if ( strstr( "visible", lowercase ) )
	{
		bits |= bits_HINT_NODE_VISIBLE;
	}

	if ( strstr( "nearest", lowercase ) )
	{
		bits |= bits_HINT_NODE_NEAREST;
	}

	if ( strstr( "random", lowercase ) )
	{
		bits |= bits_HINT_NODE_RANDOM;
	}

	// Can't be nearest and random, defer to nearest
	if ( ( bits & bits_HINT_NODE_NEAREST ) &&
		 ( bits & bits_HINT_NODE_RANDOM ) )
	{
		// Remove random
		bits &= ~bits_HINT_NODE_RANDOM;

		DevMsg( "HINTFLAGS:%s, inconsistent, the nearest node is never a random hint node, treating as nearest request!\n",
			token );
	}

	return bits;
}


//-----------------------------------------------------------------------------
// Purpose: Searches for a hint node that this NPC cares about. If one is
//			claims that hint node for this NPC so that no other NPCs
//			try to use it.
//
// Input  : nFlags - Search criterea. Can currently be one or more of the following:
//				bits_HINT_NODE_VISIBLE - searches for visible hint nodes.
//				bits_HINT_NODE_RANDOM - calls through the FindHintRandom and builds list of all matching
//				nodes and picks randomly from among them.  Note:  Depending on number of hint nodes, this
//				could be slower, so use with care.
//
// Output : Returns pointer to hint node if available hint node was found that matches the
//			given criterea that this NPC also cares about. Otherwise, returns NULL
//-----------------------------------------------------------------------------
CE_AI_Hint* CAI_HintManager::FindHint( CAI_NPC *pNPC, Hint_e nHintType, int nFlags, float flMaxDist, const Vector *pMaxDistFrom )
{
	assert( pNPC != NULL );
	if ( pNPC == NULL )
		return NULL;

	CHintCriteria	hintCriteria;
	hintCriteria.SetHintType( nHintType );
	hintCriteria.SetFlag( nFlags );

	// Using the NPC's hint group?
	if ( nFlags & bits_HINT_NODE_USE_GROUP )
	{
		hintCriteria.SetGroup( pNPC->GetHintGroup() );
	}

	// Add the search position
	Vector vecPosition = ( pMaxDistFrom != NULL ) ? (*pMaxDistFrom) : pNPC->GetAbsOrigin();
	hintCriteria.AddIncludePosition( vecPosition, flMaxDist );

	// If asking for a random node, use random logic instead
	if ( nFlags & bits_HINT_NODE_RANDOM )
		return FindHintRandom( pNPC, vecPosition, hintCriteria );

	return FindHint( pNPC, vecPosition, hintCriteria );
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *hintCriteria - 
// Output : CAI_Hint
//-----------------------------------------------------------------------------
CE_AI_Hint *CAI_HintManager::FindHint( CAI_NPC *pNPC, const Vector &position, const CHintCriteria &hintCriteria )
{
	return (CE_AI_Hint *)g_helpfunc.CAI_HintManager_FindHint( pNPC->BaseEntity(), position, hintCriteria );
}

CE_AI_Hint *CAI_HintManager::FindHint( CAI_NPC *pNPC, const CHintCriteria &hintCriteria )
{
	assert( pNPC != NULL );
	if ( pNPC == NULL )
		return NULL;

	return FindHint( pNPC, pNPC->GetAbsOrigin(), hintCriteria );
}

CE_AI_Hint *CAI_HintManager::FindHint( const Vector &position, const CHintCriteria &hintCriteria )
{
	return FindHint( NULL, position, hintCriteria );
}


//-----------------------------------------------------------------------------
// Purpose: Finds a random hint within the requested radious of the npc
//  Builds a list of all suitable hints and chooses randomly from amongst them.
// Input  : *pNPC - 
//			nHintType - 
//			nFlags - 
//			flMaxDist - 
// Output : CAI_Hint
//-----------------------------------------------------------------------------
CE_AI_Hint *CAI_HintManager::FindHintRandom( CAI_NPC *pNPC, const Vector &position, const CHintCriteria &hintCriteria )
{
	return (CE_AI_Hint *)g_helpfunc.CAI_HintManager_FindHintRandom( pNPC->BaseEntity(), position, hintCriteria );
}







CE_LINK_ENTITY_TO_CLASS(CAI_Hint, CE_AI_Hint);


