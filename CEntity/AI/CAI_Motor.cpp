
#include "CEntity.h"
#include "CAI_NPC.h"

float CAI_Motor::CalcIdealYaw( const Vector &vecTarget )
{
	CAI_NPC *npc = (CAI_NPC *)CEntity::Instance((CBaseEntity *)GetOuter());
	return npc->CalcIdealYaw( vecTarget );
}

void CAI_Motor::SetIdealYawToTargetAndUpdate( const Vector &target, float yawSpeed )
{ 
	SetIdealYawAndUpdate( CalcIdealYaw( target ), yawSpeed ); 
}


//-----------------------------------------------------------------------------
// Purpose: Set the ideal yaw and run the current or specified timestep 
//			worth of rotation.
//-----------------------------------------------------------------------------

void CAI_Motor::SetIdealYawAndUpdate( float idealYaw, float yawSpeed)
{
	SetIdealYaw( idealYaw );
	if (yawSpeed == AI_CALC_YAW_SPEED)
		RecalculateYawSpeed();
	else if (yawSpeed != AI_KEEP_YAW_SPEED)
		SetYawSpeed( yawSpeed );
	UpdateYaw(-1);
}



//=========================================================
// DeltaIdealYaw - returns the difference ( in degrees ) between
// npc's current yaw and ideal_yaw
//
// Positive result is left turn, negative is right turn
//=========================================================
float CAI_Motor::DeltaIdealYaw ( void )
{
	float	flCurrentYaw;

	flCurrentYaw = UTIL_AngleMod( GetLocalAngles().y );

	if ( flCurrentYaw == GetIdealYaw() )
	{
		return 0;
	}


	return UTIL_AngleDiff( GetIdealYaw(), flCurrentYaw );
}

