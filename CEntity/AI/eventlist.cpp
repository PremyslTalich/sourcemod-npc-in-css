
#include "CAI_NPC.h"
#include "eventlist.h"
#include "sign_func.h"

Animevent EventList_RegisterPrivateEvent( const char *pszEventName )
{
	return g_helpfunc.EventList_RegisterPrivateEvent(pszEventName);
}

