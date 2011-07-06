
#include "CCombatWeapon.h"
#include "ItemRespawnSystem.h"
#include "CPlayer.h"
#include "CE_recipientfilter.h"
#include "ammodef.h"
#include "vphysics/constraints.h"


CE_LINK_ENTITY_TO_CLASS(CBaseCombatWeapon, CCombatWeapon);

ConVar sv_css_weapon_respawn_time( "sv_css_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );


SH_DECL_MANUALHOOK1_void(Drop, 0, 0, 0, const Vector &);
DECLARE_HOOK(Drop, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER_void(Template_CCombatWeapon, Drop, (const Vector &vecVelocity), (vecVelocity));

SH_DECL_MANUALHOOK0(GetMaxClip1, 0, 0, 0, int);
DECLARE_HOOK(GetMaxClip1, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, GetMaxClip1, int, () const, ());

SH_DECL_MANUALHOOK0(GetSlot, 0, 0, 0, int);
DECLARE_HOOK(GetSlot, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, GetSlot, int, () const, ());

SH_DECL_MANUALHOOK0(HasPrimaryAmmo, 0, 0, 0, bool);
DECLARE_HOOK(HasPrimaryAmmo, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, HasPrimaryAmmo, bool, (), ());

SH_DECL_MANUALHOOK0(UsesClipsForAmmo1, 0, 0, 0, bool);
DECLARE_HOOK(UsesClipsForAmmo1, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, UsesClipsForAmmo1, bool, () const, ());

SH_DECL_MANUALHOOK0(HasSecondaryAmmo, 0, 0, 0, bool);
DECLARE_HOOK(HasSecondaryAmmo, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, HasSecondaryAmmo, bool, (), ());

SH_DECL_MANUALHOOK0(UsesClipsForAmmo2, 0, 0, 0, bool);
DECLARE_HOOK(UsesClipsForAmmo2, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, UsesClipsForAmmo2, bool, () const, ());

SH_DECL_MANUALHOOK1(Holster, 0, 0, 0, bool, CBaseEntity *);
DECLARE_HOOK(Holster, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, Holster, bool, (CBaseEntity *pSwitchingTo), (pSwitchingTo));

SH_DECL_MANUALHOOK0(Deploy, 0, 0, 0, bool);
DECLARE_HOOK(Deploy, Template_CCombatWeapon);
DECLARE_DEFAULTHANDLER(Template_CCombatWeapon, Deploy, bool, (), ());




// Sendprops
DEFINE_PROP(m_iPrimaryAmmoType, CCombatWeapon);
DEFINE_PROP(m_iSecondaryAmmoType, CCombatWeapon);
DEFINE_PROP(m_iClip1, CCombatWeapon);
DEFINE_PROP(m_iClip2, CCombatWeapon);

//Datamaps
DEFINE_PROP(m_bRemoveable, CCombatWeapon);
DEFINE_PROP(m_pConstraint, CCombatWeapon);
DEFINE_PROP(m_iSubType, CCombatWeapon);


BEGIN_DATADESC( CCombatWeapon )
	DEFINE_KEYFIELD( CItem<Template_CCombatWeapon>::m_bRespawn,		FIELD_BOOLEAN,	"mm_respawn" ),
END_DATADESC()

CCombatWeapon::CCombatWeapon()
{
	m_bRemoveOnDrop = false;
}

void CCombatWeapon::Spawn()
{
	if(m_bRemoveOnDrop)
	{
		m_bRespawn = false;
	}
	BaseClass::Spawn();
	if(m_bRespawn)
	{
		UTIL_DropToFloor( this, MASK_SOLID );
	}
}

bool CCombatWeapon::MyTouch( CPlayer *pPlayer )
{
	if ( !pPlayer->IsAllowedToPickupWeapons() )
		return false;

	if(pPlayer->HaveThisWeaponType(this))
	{
		// give sound
		int nCount = GetMaxClip1();
		if(nCount == -1)
			nCount = 1;
		bool ret = pPlayer->GiveAmmo(nCount, GetPrimaryAmmoType());
		if(ret)
		{
			CPASAttenuationFilter filter( pPlayer, "BaseCombatCharacter.AmmoPickup" );
			EmitSound( filter, pPlayer->entindex(), "BaseCombatCharacter.AmmoPickup" );
		}
		return ret;
	}

	CCombatWeapon *pWeapon = (CCombatWeapon *)CEntity::Instance(pPlayer->Weapon_GetSlot(GetSlot()));
	if(pWeapon)
		return false;

	//give sound
	pWeapon = (CCombatWeapon *)CEntity::Instance(pPlayer->GiveNamedItem(GetClassname()));
	if(pWeapon)
	{
		pWeapon->m_bRemoveOnDrop = true;
	}

	return true;
}


void CCombatWeapon::OnWeaponDrop(CPlayer *pOwner)
{
	if(m_bRemoveOnDrop)
	{
		UTIL_Remove(this);
	}
}

void CCombatWeapon::OnWeaponEquip( CPlayer *pOwner)
{

}


bool CCombatWeapon::GetObjectsOriginalParameters(Vector &vOriginalOrigin, QAngle &vOriginalAngles)
{
	if ( m_flNextResetCheckTime > gpGlobals->curtime )
		return false;

	vOriginalOrigin = GetOriginalSpawnOrigin();
	vOriginalAngles = GetOriginalSpawnAngles();

	m_flNextResetCheckTime = gpGlobals->curtime + sv_css_weapon_respawn_time.GetFloat();
	return true;
}

bool CCombatWeapon::UsesPrimaryAmmo( void )
{
	if ( m_iPrimaryAmmoType < 0 )
		return false;
	return true;
}

