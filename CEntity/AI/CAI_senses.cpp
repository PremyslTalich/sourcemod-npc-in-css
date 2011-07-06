
#include "CAI_NPC.h"
#include "CAI_senses.h"


#pragma pack(push, 1)

struct AISightIterVal_t
{
	char  array;
	short iNext;
	char  SeenArray;
};

#pragma pack(pop)



//-----------------------------------------------------------------------------

CEntity *CAI_Senses::GetNextSeenEntity( AISightIter_t *pIter ) const	
{ 
	if ( ((int)*pIter) != -1 )
	{
		AISightIterVal_t *pIterVal = (AISightIterVal_t *)pIter;
		
		for ( int i = pIterVal->array;  i < (int)ARRAYSIZE( m_SeenArrays ); i++ )
		{
			for ( int j = pIterVal->iNext; j < m_SeenArrays[i]->Count(); j++ )
			{
				if ( (*m_SeenArrays[i])[j].Get() != NULL )
				{
					pIterVal->array = i;
					pIterVal->iNext = j+1;
					return CEntity::Instance((*m_SeenArrays[i])[j]);
				}
			}
			pIterVal->iNext = 0;

			// If we're searching for a specific type, don't move to the next array
			if ( pIterVal->SeenArray != SEEN_ALL )
				break;
		}
		(*pIter) = (AISightIter_t)(-1); 
	}
	return NULL;
}


//-----------------------------------------------------------------------------

CEntity *CAI_Senses::GetFirstSeenEntity( AISightIter_t *pIter, seentype_t iSeenType ) const
{ 
	COMPILE_TIME_ASSERT( sizeof( AISightIter_t ) == sizeof( AISightIterVal_t ) );
	
	AISightIterVal_t *pIterVal = (AISightIterVal_t *)pIter;
	
	// If we're searching for a specific type, start in that array
	pIterVal->SeenArray = (char)iSeenType;
	int iFirstArray = ( iSeenType == SEEN_ALL ) ? 0 : iSeenType;

	for ( int i = iFirstArray; i < (int)ARRAYSIZE( m_SeenArrays ); i++ )
	{
		if ( m_SeenArrays[i]->Count() != 0 )
		{
			pIterVal->array = i;
			pIterVal->iNext = 1;
			return CEntity::Instance((*m_SeenArrays[i])[0]);
		}
	}
	
	(*pIter) = (AISightIter_t)(-1); 
	return NULL;
}

bool CAI_Senses::DidSeeEntity( CEntity *pSightEnt ) const
{
	AISightIter_t iter;
	CEntity *pTestEnt;

	pTestEnt = GetFirstSeenEntity( &iter );

	while( pTestEnt )
	{
		if ( pSightEnt == pTestEnt )
			return true;
		pTestEnt = GetNextSeenEntity( &iter );
	}
	return false;
}

bool CAI_Senses::CanSeeEntity( CBaseEntity *pSightEnt )
{
	return ( GetOuter()->FInViewCone_Entity( pSightEnt ) && GetOuter()->FInViewCone_Entity( pSightEnt ) );
}
