
#include "player_pickup.h"
#include "CPlayer.h"


bool Pickup_GetPreferredCarryAngles( CEntity *pObject, CPlayer *pPlayer, matrix3x4_t &localToWorld, QAngle &outputAnglesWorldSpace )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pObject);
	if ( pPickup )
	{
		if ( pPickup->HasPreferredCarryAnglesForPlayer( pPlayer ) )
		{
			outputAnglesWorldSpace = TransformAnglesToWorldSpace( pPickup->PreferredCarryAngles(), localToWorld );
			return true;
		}
	}
	return false;
}


void Pickup_ForcePlayerToDropThisObject( CEntity *pTarget )
{
	if ( pTarget == NULL )
		return;

	IPhysicsObject *pPhysics = pTarget->VPhysicsGetObject();
	
	if ( pPhysics == NULL )
		return;

	if ( pPhysics->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
	{
		for(int i=1;i<=gpGlobals->maxClients;i++)
		{
			CPlayer *pPlayer = UTIL_PlayerByIndex(i);
			if(!pPlayer)
				continue;

			if(pPlayer->GetHoldEntity() == pTarget)
			{
				pPlayer->ForceDropOfCarriedPhysObjects(pTarget);
				return;
			}
		}
	}
}



void Pickup_OnPhysGunDrop( CEntity *pDroppedObject, CPlayer *pPlayer, PhysGunDrop_t Reason )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pDroppedObject);
	if ( pPickup )
	{
		pPickup->OnPhysGunDrop( pPlayer, Reason );
	}
}


void Pickup_OnPhysGunPickup( CEntity *pPickedUpObject, CPlayer *pPlayer, PhysGunPickup_t reason )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pPickedUpObject);
	if ( pPickup )
	{
		pPickup->OnPhysGunPickup( pPlayer, reason );
	}
}

extern ConVar physcannon_maxforce;
extern ConVar physcannon_minforce;


Vector Pickup_DefaultPhysGunLaunchVelocity( const Vector &vecForward, float flMass )
{
	// Calculate the velocity based on physcannon rules
	float flForceMax = physcannon_maxforce.GetFloat();
	float flForce = flForceMax;

	float mass = flMass;
	if ( mass > 100 )
	{
		mass = MIN( mass, 1000 );
		float flForceMin = physcannon_minforce.GetFloat();
		flForce = SimpleSplineRemapValClamped( mass, 100, 600, flForceMax, flForceMin );
	}

	return ( vecForward * flForce );
}

