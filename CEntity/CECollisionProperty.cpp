
#include "CECollisionProperty.h"

//Sendprops

CECollisionProperty::CECollisionProperty(CEntity *centity)
{
	this->centity = centity;
}

void CECollisionProperty::SetSolidFlags(int flags)
{
	g_helpfunc.SetSolidFlags(centity->CollisionProp_Actual(), flags);
}

void CECollisionProperty::SetSolid(SolidType_t val)
{
	g_helpfunc.SetSolid(centity->CollisionProp_Actual(), val);
}

void CECollisionProperty::AddSolidFlags(int flags)
{
	g_helpfunc.SetSolidFlags(centity->CollisionProp_Actual(), (centity->m_usSolidFlags & flags));
}

void CECollisionProperty::RemoveSolidFlags(int flags)
{
	g_helpfunc.SetSolidFlags(centity->CollisionProp_Actual(), (centity->m_usSolidFlags & ~flags));
}

bool CECollisionProperty::IsSolid()
{
	return ::IsSolid( (SolidType_t)(unsigned char)centity->m_nSolidType, centity->m_usSolidFlags );

}

const Vector& CECollisionProperty::GetCollisionOrigin() const
{
	return centity->CollisionProp_Actual()->GetCollisionOrigin();
}

const QAngle& CECollisionProperty::GetCollisionAngles() const
{
	return centity->CollisionProp_Actual()->GetCollisionAngles();
}

const matrix3x4_t&	CECollisionProperty::CollisionToWorldTransform() const
{
	return centity->CollisionProp_Actual()->CollisionToWorldTransform();
}

//-----------------------------------------------------------------------------
// Computes a "normalized" point (range 0,0,0 - 1,1,1) in collision space
//-----------------------------------------------------------------------------
const Vector & CECollisionProperty::NormalizedToCollisionSpace( const Vector &in, Vector *pResult ) const
{
	pResult->x = Lerp( in.x, centity->m_vecMins.ptr->x, centity->m_vecMaxs.ptr->x );
	pResult->y = Lerp( in.y, centity->m_vecMins.ptr->y, centity->m_vecMaxs.ptr->y );
	pResult->z = Lerp( in.z, centity->m_vecMins.ptr->z, centity->m_vecMaxs.ptr->z );
	return *pResult;
}


//-----------------------------------------------------------------------------
// Computes a "normalized" point (range 0,0,0 - 1,1,1) in world space
//-----------------------------------------------------------------------------
const Vector & CECollisionProperty::NormalizedToWorldSpace( const Vector &in, Vector *pResult ) const
{
	Vector vecCollisionSpace;
	NormalizedToCollisionSpace( in, &vecCollisionSpace );
	CollisionToWorldSpace( vecCollisionSpace, pResult );
	return *pResult;
}


//-----------------------------------------------------------------------------
// Selects a random point in the bounds given the normalized 0-1 bounds 
//-----------------------------------------------------------------------------
void CECollisionProperty::RandomPointInBounds( const Vector &vecNormalizedMins, const Vector &vecNormalizedMaxs, Vector *pPoint) const
{
	Vector vecNormalizedSpace;
	vecNormalizedSpace.x = random->RandomFloat( vecNormalizedMins.x, vecNormalizedMaxs.x );
	vecNormalizedSpace.y = random->RandomFloat( vecNormalizedMins.y, vecNormalizedMaxs.y );
	vecNormalizedSpace.z = random->RandomFloat( vecNormalizedMins.z, vecNormalizedMaxs.z );
	NormalizedToWorldSpace( vecNormalizedSpace, pPoint );
}



//-----------------------------------------------------------------------------
// Methods relating to solid flags
//-----------------------------------------------------------------------------
bool CECollisionProperty::IsBoundsDefinedInEntitySpace() const
{
	return (( centity->m_usSolidFlags & FSOLID_FORCE_WORLD_ALIGNED ) == 0 ) &&
			( centity->m_nSolidType != SOLID_BBOX ) && ( centity->m_nSolidType != SOLID_NONE );
}

//-----------------------------------------------------------------------------
// Transforms a point in OBB space to world space
//-----------------------------------------------------------------------------
const Vector &CECollisionProperty::CollisionToWorldSpace( const Vector &in, Vector *pResult ) const 
{
	// Makes sure we don't re-use the same temp twice
	if ( !IsBoundsDefinedInEntitySpace() || ( GetCollisionAngles() == vec3_angle ) )
	{
		VectorAdd( in, GetCollisionOrigin(), *pResult );
	}
	else
	{
		VectorTransform( in, CollisionToWorldTransform(), *pResult );
	}
	return *pResult;
}
