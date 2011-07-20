
#include "CEntity.h"
#include "soundflags.h"
#include "CAI_Criteria.h"

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *criteria - 
//			"" - 
//			1.0f - 
//-----------------------------------------------------------------------------
void AI_CriteriaSet::AppendCriteria( const char *criteria, const char *value /*= ""*/, float weight /*= 1.0f*/ )
{
	int idx = FindCriterionIndex( criteria );
	if ( idx == -1 )
	{
		CritEntry_t entry;
		entry.criterianame = criteria;
		MEM_ALLOC_CREDIT();
		idx = m_Lookup.Insert( entry );
	}

	CritEntry_t *entry = &m_Lookup[ idx ];

	entry->SetValue( value );
	entry->weight = weight;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : int
//-----------------------------------------------------------------------------
int AI_CriteriaSet::FindCriterionIndex( const char *name ) const
{
	CritEntry_t search;
	search.criterianame = name;
	int idx = m_Lookup.Find( search );
	if ( idx == m_Lookup.InvalidIndex() )
		return -1;

	return idx;
}


AI_Response::~AI_Response()
{
	g_pMemAlloc->Free(m_pCriteria);
	g_pMemAlloc->Free(m_szContext);
}
