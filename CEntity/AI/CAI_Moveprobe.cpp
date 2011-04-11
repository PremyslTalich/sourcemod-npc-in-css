
#include "CAI_NPC.h"
#include "CAI_moveprobe.h"


bool CAI_MoveProbe::MoveLimit( Navigation_t navType, const Vector &vecStart, 
	const Vector &vecEnd, unsigned int collisionMask, const CBaseEntity *pTarget, 
	float pctToCheckStandPositions, unsigned flags, AIMoveTrace_t* pTrace)
{
	return g_helpfunc.MoveLimit(this, navType, vecStart, vecEnd, collisionMask, pTarget, pctToCheckStandPositions, flags, pTrace);
}


bool CAI_MoveProbe::ShouldBrushBeIgnored( CBaseEntity *pEntity )
{
	return g_helpfunc.ShouldBrushBeIgnored(this, pEntity);
}


