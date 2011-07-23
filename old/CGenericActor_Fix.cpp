
#include "CGenericActor_Fix.h"


void CE_CGenericActor_Fix::CE_PostInit()
{
	BaseClass::CE_PostInit();
	CreateBehaviors();
}

void CE_CGenericActor_Fix::Spawn()
{
	BaseClass::Spawn();

}

void CE_CGenericActor_Fix::Precache()
{
	BaseClass::Precache();

}

float CE_CGenericActor_Fix::MaxYawSpeed()
{
	return 45;
}

Class_T CE_CGenericActor_Fix::Classify( void )
{
	return CLASS_NONE;
}

int	CE_CGenericActor_Fix::GetSoundInterests ( void )
{
	return SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_PLAYER_VEHICLE | 
		SOUND_BULLET_IMPACT;
}

bool CE_CGenericActor_Fix::CreateBehaviors()
{
	return true;
}

void CE_CGenericActor_Fix::AddBehavior( CAI_BehaviorBase *pBehavior )
{
	//m_Behaviors.AddToTail( pBehavior );
	pBehavior->SetOuter( BaseEntity() );
	//pBehavior->SetBackBridge( this );
}

bool CE_CGenericActor_Fix::BehaviorSelectSchedule()
{
	return false;
}

bool CE_CGenericActor_Fix::IsRunningBehavior() const
{
	return false;
}




