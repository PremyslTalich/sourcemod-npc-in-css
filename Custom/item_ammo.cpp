#include "CEntity.h"
#include "CAnimating.h"
#include "CItem.h"
#include "CPlayer.h"
#include "ItemRespawnSystem.h"
#include "CE_recipientfilter.h"
#include "ammodef.h"
#include "vphysics/constraints.h"


bool ITEM_GiveAmmo( CPlayer *pPlayer, float flCount, const char *pszAmmoName)
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
		return false;

	bool ret = pPlayer->GiveAmmo(flCount, iAmmoType);
	if(ret)
	{
		CPASAttenuationFilter filter( pPlayer, "BaseCombatCharacter.AmmoPickup" );
		CEntity::EmitSound( filter, pPlayer->entindex(), "BaseCombatCharacter.AmmoPickup" );
	}
	return ret;
}

typedef CItem<CAnimating> CAmmoItem;

class CAmmo : public CAmmoItem
{
public:
	CE_DECLARE_CLASS(CAmmo, CAmmoItem);
	DECLARE_DATADESC();
	CE_CUSTOM_ENTITY();

	virtual int	ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_WCEDIT_POSITION; };

};


BEGIN_DATADESC( CAmmo )
	DEFINE_KEYFIELD( CAmmoItem::m_bRespawn,		FIELD_BOOLEAN,	"mm_respawn" ),
END_DATADESC()


// awp
class CAmmo_338msg : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_338msg, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxSniperRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxSniperRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 10, "BULLET_PLAYER_338MAG");
	}
};

// p228
class CAmmo_357sig : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_357sig, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxSRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxSRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 13, "BULLET_PLAYER_357SIG");
	}
};

// usp mac10 ump45
class CAmmo_45acp : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_45acp, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxSRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxSRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 30, "BULLET_PLAYER_45ACP");
	}
};

// deagle
class CAmmo_50ae : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_50ae, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/357ammo.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/357ammo.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 7, "BULLET_PLAYER_50AE");
	}
};

// gail famas m4a1 sg550 sg552
class CAmmo_556mm : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_556mm, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxMRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxMRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 30, "BULLET_PLAYER_556MM");
	}
};

// m249
class CAmmo_556mm_box : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_556mm_box, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxMRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxMRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 50, "BULLET_PLAYER_556MM_BOX");
	}
};

// fiveseven p90
class CAmmo_57mm : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_57mm, CAmmo);
public:
	 bool MyTouch( CPlayer *pPlayer )
	 {
		return ITEM_GiveAmmo(pPlayer, 50, "BULLET_PLAYER_57MM");
	 }
};

// ak47 g3sg1 scout
class CAmmo_762mm : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_762mm, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxMRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxMRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 30, "BULLET_PLAYER_762MM");
	}
};

// elite glock mp5navy tmp
class CAmmo_9mm : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_9mm, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxMRounds.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxMRounds.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 30, "BULLET_PLAYER_9MM");
	}
};

// m3 xm1014
class CAmmo_buckshot : public CAmmo
{
public:
	CE_DECLARE_CLASS(CAmmo_buckshot, CAmmo);
public:
	void Spawn( void )
	{ 
		Precache();
		SetModel( "models/items/BoxBuckshot.mdl");
		BaseClass::Spawn();
	}
	void Precache( void )
	{
		BaseClass::Precache();
		PrecacheModel ("models/items/BoxBuckshot.mdl");
	}
	bool MyTouch( CPlayer *pPlayer )
	{
		return ITEM_GiveAmmo(pPlayer, 8, "BULLET_PLAYER_BUCKSHOT");
	}
};

CE_LINK_ENTITY_TO_CLASS( ammo_338mag, CAmmo_338msg ); // awp
CE_LINK_ENTITY_TO_CLASS( ammo_357sig, CAmmo_357sig ); // p228
CE_LINK_ENTITY_TO_CLASS( ammo_45acp, CAmmo_45acp ); // usp mac10 ump45
CE_LINK_ENTITY_TO_CLASS( ammo_50ae, CAmmo_50ae ); // deagle
CE_LINK_ENTITY_TO_CLASS( ammo_556mm, CAmmo_556mm ); // gail famas m4a1 sg550 sg552
CE_LINK_ENTITY_TO_CLASS( ammo_556mm_box, CAmmo_556mm_box ); // m249
CE_LINK_ENTITY_TO_CLASS( ammo_57mm, CAmmo_57mm ); // fiveseven p90
CE_LINK_ENTITY_TO_CLASS( ammo_762mm, CAmmo_762mm ); // ak47 g3sg1 scout
CE_LINK_ENTITY_TO_CLASS( ammo_9mm, CAmmo_9mm ); // elite glock mp5navy tmp
CE_LINK_ENTITY_TO_CLASS( ammo_buckshot, CAmmo_buckshot ); // m3 xm1014



