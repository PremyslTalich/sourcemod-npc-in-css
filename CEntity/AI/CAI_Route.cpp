
#include "CAI_Route.h"

//-----------------------------------------------------------------------------

Activity CAI_Path::SetMovementActivity(Activity activity)
{ 
	Assert( activity != ACT_RESET && activity != ACT_INVALID );
	//Msg("Set movement to %s\n", ActivityList_NameForIndex(activity) );

	m_sequence = ACT_INVALID;
	return (m_activity = activity);	
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
GoalType_t	CAI_Path::GoalType(void) const
{
	return m_goalType;
}
