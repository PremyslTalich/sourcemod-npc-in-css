
#include "CWorld.h"
#include "GameSystem.h"
#include "CESoundEnt.h"

CE_LINK_ENTITY_TO_CLASS(CWorld, CE_CWorld);

void CE_CWorld::Precache()
{
	BaseClass::Precache();

	physenv = iphysics->GetActiveEnvironmentByIndex(0);

	CE_CSoundEnt::InitSoundEnt();
}

