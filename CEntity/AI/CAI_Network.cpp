
#include "CAI_Network.h"
#include "CCombatCharacter.h"
#include "CAI_Node.h"



CAI_Network * 		g_pBigAINet;


//-----------------------------------------------------------------------------

Vector CAI_Network::GetNodePosition( Hull_t hull, int nodeID )
{
	if ( !m_pAInode )
	{
		Assert( 0 );
		return vec3_origin;
	}
	
	if ( ( nodeID < 0 ) || ( nodeID > m_iNumNodes ) )
	{
		Assert( 0 );
		return vec3_origin;
	}

	return m_pAInode[nodeID]->GetPosition( hull );
}

//-----------------------------------------------------------------------------

Vector CAI_Network::GetNodePosition( CCombatCharacter *pNPC, int nodeID )
{
	if ( pNPC == NULL )
	{
		Assert( 0 );
		return vec3_origin;
	}

	return GetNodePosition( pNPC->GetHullType(), nodeID );
}
