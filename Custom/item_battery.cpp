
#include "CItem.h"
#include "CSode_Fix.h"
#include "CPlayer.h"
#include "ItemRespawnSystem.h"
#include "vphysics/constraints.h"


class CItemBattery : public CItem<CSode_Fix>
{
public:
	CE_DECLARE_CLASS( CItemBattery, CItem<CSode_Fix> );

	void Spawn( void )
	{ 
		m_bRespawn = true;
		Precache();
		SetModel( "models/items/battery.mdl" );
		BaseClass::Spawn( );
		UTIL_DropToFloor( this, MASK_SOLID );
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/battery.mdl");
		PrecacheScriptSound( "ItemBattery.Touch" );

	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return pPlayer->ApplyBattery();
	}
};

LINK_ENTITY_TO_CUSTOM_CLASS( item_battery, item_sodacan, CItemBattery );


