
#include "choreoevent.h"


//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CChoreoEvent::GetEndTime( )
{
	return m_flEndTime;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CChoreoEvent::GetStartTime( )
{
	return m_flStartTime;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CChoreoEvent::HasEndTime( void )
{
	return m_flEndTime != -1.0f ? true : false;
}

