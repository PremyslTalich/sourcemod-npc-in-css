
#include "CAI_NPC.h"
#include "CAI_Navigator.h"
#include "CAI_Route.h"


//-----------------------------------------------------------------------------

bool CAI_Navigator::IsGoalActive() const
{
	return ( GetPath() && !( const_cast<CAI_Path *>(GetPath())->IsEmpty() ) );
}


//-----------------------------------------------------------------------------

Activity CAI_Navigator::SetMovementActivity(Activity activity)
{
	return GetPath()->SetMovementActivity( activity );
}


//-----------------------------------------------------------------------------
// Purpose: Sets navigation type, maintaining any necessary invariants
//-----------------------------------------------------------------------------
void CAI_Navigator::SetNavType( Navigation_t navType )
{ 
	m_navType = navType;
}


bool CAI_Navigator::IsGoalSet() const
{
	return ( GetPath()->GoalType() != GOALTYPE_NONE );
}

