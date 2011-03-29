
#ifndef _INCLUDE_CECOLLISIONPROPERTY_H_
#define _INCLUDE_CECOLLISIONPROPERTY_H_

#include "extension.h"
#include "CEntity.h"


class CECollisionProperty
{
public:
	CECollisionProperty(CEntity *centity);
	void SetSolidFlags(int flags);
	void SetSolid(SolidType_t val);
	void AddSolidFlags(int flags);
	void RemoveSolidFlags(int flags);
	bool IsSolid();

public:
	const Vector&	GetCollisionOrigin() const;
	const QAngle&	GetCollisionAngles() const;
	const matrix3x4_t&	CollisionToWorldTransform() const;

public:
	void				RandomPointInBounds( const Vector &vecNormalizedMins, const Vector &vecNormalizedMaxs, Vector *pPoint) const;
	const Vector &		NormalizedToWorldSpace( const Vector &in, Vector *pResult ) const;
	const Vector &		NormalizedToCollisionSpace( const Vector &in, Vector *pResult ) const;
	const Vector &		CollisionToWorldSpace( const Vector &in, Vector *pResult ) const;
	bool				IsBoundsDefinedInEntitySpace() const;


protected:
	CEntity *centity;

protected: //Sendprops
};



#endif

