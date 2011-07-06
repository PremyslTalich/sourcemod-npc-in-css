
#include "CAI_NPC.h"
#include "CAI_tacticalservices.h"
#include "CAI_Node.h"
#include "CAI_Network.h"


bool CAI_TacticalServices::FindLos(const Vector &threatPos, const Vector &threatEyePos, float minThreatDist, float maxThreatDist, float blockTime, Vector *pResult)
{
	return FindLos( threatPos, threatEyePos, minThreatDist, maxThreatDist, blockTime, FLANKTYPE_NONE, vec3_origin, 0, pResult );
}

bool CAI_TacticalServices::FindLos(const Vector &threatPos, const Vector &threatEyePos, float minThreatDist, float maxThreatDist, float blockTime, FlankType_t eFlankType, const Vector &vecFlankRefPos, float flFlankParam, Vector *pResult)
{
	MARK_TASK_EXPENSIVE();

	int node = FindLosNode( threatPos, threatEyePos, 
											 minThreatDist, maxThreatDist, 
											 blockTime, eFlankType, vecFlankRefPos, flFlankParam );
	
	if (node == NO_NODE)
		return false;

	*pResult = GetNodePos( node );
	return true;
}

int CAI_TacticalServices::FindLosNode(const Vector &vThreatPos, const Vector &vThreatEyePos, float flMinThreatDist, float flMaxThreatDist, float flBlockTime, FlankType_t eFlankType, const Vector &vecFlankRefPos, float flFlankParam )
{
	META_CONPRINTF("FindLosNode\n");
	return g_helpfunc.CAI_TacticalServices_FindLosNode(this, vThreatPos, vThreatEyePos, flMinThreatDist, flMaxThreatDist, flBlockTime, eFlankType, vecFlankRefPos, flFlankParam);
}

Vector CAI_TacticalServices::GetNodePos( int node )
{
	return GetNetwork()->GetNode((int)node)->GetPosition(GetHullType());
}

bool CAI_TacticalServices::FindCoverPos( const Vector &vThreatPos, const Vector &vThreatEyePos, float flMinDist, float flMaxDist, Vector *pResult )
{
	return FindCoverPos( GetLocalOrigin(), vThreatPos, vThreatEyePos, flMinDist, flMaxDist, pResult );
}

bool CAI_TacticalServices::FindCoverPos( const Vector &vNearPos, const Vector &vThreatPos, const Vector &vThreatEyePos, float flMinDist, float flMaxDist, Vector *pResult )
{
	MARK_TASK_EXPENSIVE();

	int node = FindCoverNode( vNearPos, vThreatPos, vThreatEyePos, flMinDist, flMaxDist );
	
	if (node == NO_NODE)
		return false;

	*pResult = GetNodePos( node );
	return true;
}

int CAI_TacticalServices::FindCoverNode(const Vector &vNearPos, const Vector &vThreatPos, const Vector &vThreatEyePos, float flMinDist, float flMaxDist )
{
	return g_helpfunc.CAI_TacticalServices_FindCoverNode(this, vNearPos, vThreatPos, vThreatEyePos, flMinDist, flMaxDist);
}

