/**
* =============================================================================
* CEntity Entity Handling Framework
* Copyright (C) 2011 Matt Woodrow.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _INCLUDE_CUTIL_H_
#define _INCLUDE_CUTIL_H_

#include "extension.h"
#include "CEntity.h"
#include "networkvar.h"



//-----------------------------------------------------------------------------
// These are inlined for backwards compatibility
//-----------------------------------------------------------------------------
inline float UTIL_Approach( float target, float value, float speed )
{
	return Approach( target, value, speed );
}

inline float UTIL_ApproachAngle( float target, float value, float speed )
{
	return ApproachAngle( target, value, speed );
}

inline float UTIL_AngleDistance( float next, float cur )
{
	return AngleDistance( next, cur );
}

inline float UTIL_AngleMod(float a)
{
	return anglemod(a);
}

inline float UTIL_AngleDiff( float destAngle, float srcAngle )
{
	return AngleDiff( destAngle, srcAngle );
}



// make this a fixed size so it just sits on the stack
#define MAX_SPHERE_QUERY	512
class CEntitySphereQuery
{
public:
	// currently this builds the list in the constructor
	// UNDONE: make an iterative query of ISpatialPartition so we could
	// make queries like this optimal
	CEntitySphereQuery( const Vector &center, float radius, int flagMask=0 );
	CEntity *GetCurrentEntity();
	inline void NextEntity() { m_listIndex++; }

private:
	int			m_listIndex;
	int			m_listCount;
	CEntity		*m_pList[MAX_SPHERE_QUERY];
};


//-----------------------------------------------------------------------------
// class CFlaggedEntitiesEnum
//-----------------------------------------------------------------------------
// enumerate entities that match a set of edict flags into a static array
class CFlaggedEntitiesEnum : public IPartitionEnumerator
{
public:
	CFlaggedEntitiesEnum( CEntity **pList, int listMax, int flagMask );

	// This gets called	by the enumeration methods with each element
	// that passes the test.
	virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity );
	
	int GetCount() { return m_count; }
	bool AddToList( CEntity *pEntity );
	
private:
	CEntity			**m_pList;
	int				m_listMax;
	int				m_flagMask;
	int				m_count;
};




// Pass in an array of pointers and an array size, it fills the array and returns the number inserted
int			UTIL_EntitiesInBox( const Vector &mins, const Vector &maxs, CFlaggedEntitiesEnum *pEnum  );
int			UTIL_EntitiesAlongRay( const Ray_t &ray, CFlaggedEntitiesEnum *pEnum  );
int			UTIL_EntitiesInSphere( const Vector &center, float radius, CFlaggedEntitiesEnum *pEnum  );

inline int UTIL_EntitiesInBox( CEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask )
{
	CFlaggedEntitiesEnum boxEnum( pList, listMax, flagMask );
	return UTIL_EntitiesInBox( mins, maxs, &boxEnum );
}

inline int UTIL_EntitiesAlongRay( CEntity **pList, int listMax, const Ray_t &ray, int flagMask )
{
	CFlaggedEntitiesEnum rayEnum( pList, listMax, flagMask );
	return UTIL_EntitiesAlongRay( ray, &rayEnum );
}

inline int UTIL_EntitiesInSphere( CEntity **pList, int listMax, const Vector &center, float radius, int flagMask )
{
	CFlaggedEntitiesEnum sphereEnum( pList, listMax, flagMask );
	return UTIL_EntitiesInSphere( center, radius, &sphereEnum );
}





class CE_CTraceFilterSimple : public CTraceFilter
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CE_CTraceFilterSimple );
	
	CE_CTraceFilterSimple( const IHandleEntity *passentity, int collisionGroup );
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );
	virtual void SetPassEntity( const IHandleEntity *pPassEntity ) { m_pPassEnt = pPassEntity; }
	virtual void SetCollisionGroup( int iCollisionGroup ) { m_collisionGroup = iCollisionGroup; }

	const IHandleEntity *GetPassEntity( void ){ return m_pPassEnt;}

private:
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
};


class CPlayer;

class CEntityLookup;

inline CEntity *CE_EntityFromEntityHandle( IHandleEntity *pHandleEntity )
{
	if ( staticpropmgr->IsStaticProp( pHandleEntity ) )
		return NULL;

	IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
	return CEntityLookup::Instance(pUnk->GetBaseEntity());
}

inline CEntity *CE_EntityFromEntityHandle( const IHandleEntity *pConstHandleEntity )
{
	IHandleEntity *pHandleEntity = const_cast<IHandleEntity*>(pConstHandleEntity);

	if ( staticpropmgr->IsStaticProp( pHandleEntity ) )
		return NULL;

	IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
	return CEntityLookup::Instance(pUnk->GetBaseEntity());
}


void UTIL_TraceLine( const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, 
					 const IHandleEntity *ignore, int collisionGroup, trace_t *ptr );

void UTIL_TraceLine( const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, 
					 ITraceFilter *pFilter, trace_t *ptr );

void UTIL_TraceHull( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, ITraceFilter *pFilter, trace_t *ptr );

void UTIL_TraceHull( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin, 
					 const Vector &hullMax,	unsigned int mask, const IHandleEntity *ignore, 
					 int collisionGroup, trace_t *ptr );

void UTIL_TraceEntity( CEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, 
					  unsigned int mask, const IHandleEntity *pIgnore, int nCollisionGroup, trace_t *ptr );

void UTIL_SetOrigin(CEntity *entity, const Vector &vecOrigin, bool bFireTriggers = false);

bool UTIL_EntityHasMatchingRootParent( CEntity *pRootParent, CEntity *pEntity );

float UTIL_VecToYaw( const Vector &vec );

bool NPC_CheckBrushExclude( CEntity *pEntity, CEntity *pBrush );

CPlayer	*UTIL_PlayerByIndex( int playerIndex );

float UTIL_ScaleForGravity( float desiredGravity );

void UTIL_DecalTrace( trace_t *pTrace, char const *decalName );

inline bool FStrEq(const char *sz1, const char *sz2)
{
	return ( sz1 == sz2 || stricmp(sz1, sz2) == 0 );
}


#endif // _INCLUDE_UTIL_H_
