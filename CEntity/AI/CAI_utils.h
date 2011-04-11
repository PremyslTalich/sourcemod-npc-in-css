//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Simple, small, free-standing tools for building AIs
//
//=============================================================================//

#ifndef AI_UTILS_H
#define AI_UTILS_H

#include "simtimer.h"
#include "ai_hull.h"
#include "CAI_component.h"

#if defined( _WIN32 )
#pragma once
#endif


//-----------------------------------------------------------------------------

class CE_CTraceFilterNav : public CE_CTraceFilterSimple
{
public:
	CE_CTraceFilterNav( CAI_NPC *pProber, bool bIgnoreTransientEntities, const IServerEntity *passedict, int collisionGroup, bool m_bAllowPlayerAvoid = true );
	bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask );

private:
	CAI_NPC *m_pProber;
	bool m_bIgnoreTransientEntities;
	bool m_bCheckCollisionTable;
	bool m_bAllowPlayerAvoid;
};

extern string_t g_iszFuncBrushClassname;

#endif // AI_UTILS_H
