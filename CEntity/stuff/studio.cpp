
#include "CEntity.h"
#include "studio.h"
#include "engine/ivmodelinfo.h"


const studiohdr_t *virtualgroup_t::GetStudioHdr( ) const
{
	return modelinfo->FindModel( this->cache );
}


virtualmodel_t *studiohdr_t::GetVirtualModel( void ) const
{
	if ( numincludemodels == 0 )
		return NULL;
	return modelinfo->GetVirtualModel( this );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int CStudioHdr::GetSharedPoseParameter( int iSequence, int iLocalPose ) const
{
	if (m_pVModel == NULL)
	{
		return iLocalPose;
	}

	if (iLocalPose == -1)
		return iLocalPose;

	Assert( m_pVModel );

	virtualgroup_t *pGroup = &m_pVModel->m_group[ m_pVModel->m_seq[iSequence].group ];

	return pGroup->masterPose[iLocalPose];
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

const mstudioposeparamdesc_t &CStudioHdr::pPoseParameter( int i ) const
{
	if (m_pVModel == NULL)
	{
		return *m_pStudioHdr->pLocalPoseParameter( i );
	}

	if ( m_pVModel->m_pose[i].group == 0)
		return *m_pStudioHdr->pLocalPoseParameter( m_pVModel->m_pose[i].index );

	const studiohdr_t *pStudioHdr = GroupStudioHdr( m_pVModel->m_pose[i].group );

	return *pStudioHdr->pLocalPoseParameter( m_pVModel->m_pose[i].index );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int CStudioHdr::iRelativeAnim( int baseseq, int relanim ) const
{
	if (m_pVModel == NULL)
	{
		return relanim;
	}

	virtualgroup_t *pGroup = &m_pVModel->m_group[ m_pVModel->m_seq[baseseq].group ];

	return pGroup->masterAnim[ relanim ];
}


mstudioanimdesc_t &CStudioHdr::pAnimdesc( int i ) const
{ 
	if (m_pVModel == NULL)
	{
		return *m_pStudioHdr->pLocalAnimdesc( i );
	}

	const studiohdr_t *pStudioHdr = GroupStudioHdr( m_pVModel->m_anim[i].group );

	return *pStudioHdr->pLocalAnimdesc( m_pVModel->m_anim[i].index );
}


const studiohdr_t *CStudioHdr::GroupStudioHdr( int i ) const
{
	if ( !this )
	{
		ExecuteNTimes( 5, Warning( "Call to NULL CStudioHdr::GroupStudioHdr()\n" ) );
	}

	if ( m_nFrameUnlockCounter != *m_pFrameUnlockCounter )
	{
		m_FrameUnlockCounterMutex.Lock();
		if ( *m_pFrameUnlockCounter != m_nFrameUnlockCounter ) // i.e., this thread got the mutex
		{
			memset( m_pStudioHdrCache.Base(), 0, m_pStudioHdrCache.Count() * sizeof(studiohdr_t *) );
			m_nFrameUnlockCounter = *m_pFrameUnlockCounter;
		}
		m_FrameUnlockCounterMutex.Unlock();
	}

	if ( !m_pStudioHdrCache.IsValidIndex( i ) )
	{
		const char *pszName = ( m_pStudioHdr ) ? m_pStudioHdr->pszName() : "<<null>>";
		ExecuteNTimes( 5, Warning( "Invalid index passed to CStudioHdr(%s)::GroupStudioHdr(): %d, but max is %d [%d]\n", pszName, i, m_pStudioHdrCache.Count() ) );
		DebuggerBreakIfDebugging();
		return m_pStudioHdr; // return something known to probably exist, certainly things will be messed up, but hopefully not crash before the warning is noticed
	}

	const studiohdr_t *pStudioHdr = m_pStudioHdrCache[ i ];

	if (pStudioHdr == NULL)
	{
	#if defined(_WIN32) && !defined(THREAD_PROFILER)
		Assert( !m_pVModel->m_Lock.GetOwnerId() );
	#endif
		virtualgroup_t *pGroup = &m_pVModel->m_group[ i ];
		pStudioHdr = pGroup->GetStudioHdr();
		m_pStudioHdrCache[ i ] = pStudioHdr;
	}

	Assert( pStudioHdr );
	return pStudioHdr;
}


const virtualmodel_t * CStudioHdr::ResetVModel( const virtualmodel_t *pVModel ) const
{
	if (pVModel != NULL)
	{
		m_pVModel = (virtualmodel_t *)pVModel;
	#if defined(_WIN32) && !defined(THREAD_PROFILER)
		Assert( !pVModel->m_Lock.GetOwnerId() );
	#endif
		m_pStudioHdrCache.SetCount( m_pVModel->m_group.Count() );

		int i;
		for (i = 0; i < m_pStudioHdrCache.Count(); i++)
		{
			m_pStudioHdrCache[ i ] = NULL;
		}
		
		return const_cast<virtualmodel_t *>(pVModel);
	}
	else
	{
		m_pVModel = NULL;
		return NULL;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int CStudioHdr::GetNumSeq( void ) const
{
	if (m_pVModel == NULL)
	{
		return m_pStudioHdr->numlocalseq;
	}

	return m_pVModel->m_seq.Count();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

mstudioseqdesc_t &CStudioHdr::pSeqdesc( int i ) const
{
	Assert( i >= 0 && i < GetNumSeq() );
	if ( i < 0 || i >= GetNumSeq() )
	{
		// Avoid reading random memory.
		i = 0;
	}
	
	if (m_pVModel == NULL)
	{
		return *m_pStudioHdr->pLocalSeqdesc( i );
	}

	const studiohdr_t *pStudioHdr = GroupStudioHdr( m_pVModel->m_seq[i].group );

	return *pStudioHdr->pLocalSeqdesc( m_pVModel->m_seq[i].index );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

bool CStudioHdr::SequencesAvailable() const
{
	if (m_pStudioHdr->numincludemodels == 0)
	{
		return true;
	}

	if (m_pVModel == NULL)
	{
		// repoll m_pVModel
		return (ResetVModel( m_pStudioHdr->GetVirtualModel() ) != NULL);
	}
	else
		return true;
}


//-----------------------------------------------------------------------------
//	CODE PERTAINING TO ACTIVITY->SEQUENCE MAPPING SUBCLASS
//-----------------------------------------------------------------------------
#define iabs(i) (( (i) >= 0 ) ? (i) : -(i) )

void CStudioHdr::CActivityToSequenceMapping::Initialize( CStudioHdr * __restrict pstudiohdr )
{
	// Algorithm: walk through every sequence in the model, determine to which activity
	// it corresponds, and keep a count of sequences per activity. Once the total count
	// is available, allocate an array large enough to contain them all, update the 
	// starting indices for every activity's section in the array, and go back through,
	// populating the array with its data.

	AssertMsg1( m_pSequenceTuples == NULL, "Tried to double-initialize sequence mapping for %s", pstudiohdr->pszName() );
	if ( m_pSequenceTuples != NULL )
		return; // don't double initialize.

	SetValidationPair(pstudiohdr);

	if ( ! pstudiohdr->SequencesAvailable() )
		return; // nothing to do.

#if STUDIO_SEQUENCE_ACTIVITY_LAZY_INITIALIZE
	m_bIsInitialized = true;
#endif
	
	// Some studio headers have no activities at all. In those
	// cases we can avoid a lot of this effort.
	bool bFoundOne = false;

	// for each sequence in the header...
	const int NumSeq = pstudiohdr->GetNumSeq();
	for ( int i = 0 ; i < NumSeq ; ++i )
	{
		const mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
#if defined(SERVER_DLL) || defined(CLIENT_DLL) || defined(GAME_DLL)
		if (!(seqdesc.flags & STUDIO_ACTIVITY))
		{
			// AssertMsg2( false, "Sequence %d on studiohdr %s didn't have its activity initialized!", i, pstudiohdr->pszName() );
			SetActivityForSequence( pstudiohdr, i );
		}
#endif

		// is there an activity associated with this sequence?
		if (seqdesc.activity >= 0)
		{
			bFoundOne = true;

			// look up if we already have an entry. First we need to make a speculative one --
			HashValueType entry(seqdesc.activity, 0, 1, iabs(seqdesc.actweight));
			UtlHashHandle_t handle = m_ActToSeqHash.Find(entry);
			if ( m_ActToSeqHash.IsValidHandle(handle) )
			{	
				// we already have an entry and must update it by incrementing count
				HashValueType * __restrict toUpdate = &m_ActToSeqHash.Element(handle);
				toUpdate->count += 1;
				toUpdate->totalWeight += iabs(seqdesc.actweight);
			}
			else
			{
				// we do not have an entry yet; create one.
				m_ActToSeqHash.Insert(entry);
			}
		}
	}

	// if we found nothing, don't bother with any other initialization!
	if (!bFoundOne)
		return;

	// Now, create starting indices for each activity. For an activity n, 
	// the starting index is of course the sum of counts [0..n-1]. 
	register int sequenceCount = 0;
	int topActivity = 0; // this will store the highest seen activity number (used later to make an ad hoc map on the stack)
	for ( UtlHashHandle_t handle = m_ActToSeqHash.GetFirstHandle() ; 
		  m_ActToSeqHash.IsValidHandle(handle) ;
		  handle = m_ActToSeqHash.GetNextHandle(handle) )
	{
		HashValueType &element = m_ActToSeqHash[handle];
		element.startingIdx = sequenceCount;
		sequenceCount += element.count;
		topActivity = max(topActivity, element.activityIdx);
	}
	

	// Allocate the actual array of sequence information. Note the use of restrict;
	// this is an important optimization, but means that you must never refer to this
	// array through m_pSequenceTuples in the scope of this function.
	SequenceTuple * __restrict tupleList = new SequenceTuple[sequenceCount];
	m_pSequenceTuples = tupleList; // save it off -- NEVER USE m_pSequenceTuples in this function!
	m_iSequenceTuplesCount = sequenceCount;



	// Now we're going to actually populate that list with the relevant data. 
	// First, create an array on the stack to store how many sequences we've written
	// so far for each activity. (This is basically a very simple way of doing a map.)
	// This stack may potentially grow very large; so if you have problems with it, 
	// go to a utlmap or similar structure.
	unsigned int allocsize = (topActivity + 1) * sizeof(int);
#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) ) //  need macro for constant expression
	allocsize = ALIGN_VALUE(allocsize,16);
	int * __restrict seqsPerAct = static_cast<int *>(stackalloc(allocsize));
	memset(seqsPerAct, 0, allocsize);

	// okay, walk through all the sequences again, and write the relevant data into 
	// our little table.
	for ( int i = 0 ; i < NumSeq ; ++i )
	{
		const mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( i );
		if (seqdesc.activity >= 0)
		{
			const HashValueType &element = m_ActToSeqHash[m_ActToSeqHash.Find(HashValueType(seqdesc.activity, 0, 0, 0))];
			
			// If this assert trips, we've written more sequences per activity than we allocated 
			// (therefore there must have been a miscount in the first for loop above).
			int tupleOffset = seqsPerAct[seqdesc.activity];
			Assert( tupleOffset < element.count );

			// You might be tempted to collapse this pointer math into a single pointer --
			// don't! the tuple list is marked __restrict above.
			(tupleList + element.startingIdx + tupleOffset)->seqnum = i; // store sequence number
			(tupleList + element.startingIdx + tupleOffset)->weight = iabs(seqdesc.actweight);

			seqsPerAct[seqdesc.activity] += 1;
		}
	}

#ifdef DBGFLAG_ASSERT
	// double check that we wrote exactly the right number of sequences.
	unsigned int chkSequenceCount = 0;
	for (int j = 0 ; j <= topActivity ; ++j)
	{
		chkSequenceCount += seqsPerAct[j];
	}
	Assert(chkSequenceCount == m_iSequenceTuplesCount);
#endif

}

void CStudioHdr::CActivityToSequenceMapping::SetValidationPair( const CStudioHdr *RESTRICT pstudiohdr ) RESTRICT
{
	m_expectedPStudioHdr = pstudiohdr->GetRenderHdr();
	m_expectedVModel = pstudiohdr->GetVirtualModel();
}



int CStudioHdr::CActivityToSequenceMapping::NumSequencesForActivity( int forActivity )
{
	// If this trips, you've called this function on something that doesn't 
	// have activities.
	//Assert(m_pSequenceTuples != NULL);
	if ( m_pSequenceTuples == NULL )
		return 0;

	HashValueType entry(forActivity, 0, 0, 0);
	UtlHashHandle_t handle = m_ActToSeqHash.Find(entry);
	if (m_ActToSeqHash.IsValidHandle(handle))
	{
		return m_ActToSeqHash[handle].count;
	}
	else
	{
		return 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int	CStudioHdr::GetNumPoseParameters( void ) const
{
	if (m_pVModel == NULL)
	{
		return m_pStudioHdr->numlocalposeparameters;
	}

	Assert( m_pVModel );

	return m_pVModel->m_pose.Count();
}

