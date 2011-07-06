
#include "CAI_NPC.h"
#include "activitylist.h"
#include "sign_func.h"


Activity ActivityList_RegisterPrivateActivity( const char *pszActivityName )
{
	return g_helpfunc.ActivityList_RegisterPrivateActivity(pszActivityName);
}

