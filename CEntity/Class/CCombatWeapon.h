
#ifndef _INCLUDE_CCOMBATWEAPON_H
#define _INCLUDE_CCOMBATWEAPON_H

#include "CEntity.h"
#include "CAnimating.h"
#include "CItem.h"
#include "weapon_proficiency.h"

class CPlayer;

//Start with a constraint in place (don't drop to floor)
#define	SF_WEAPON_START_CONSTRAINED	(1<<0)	
#define SF_WEAPON_NO_PLAYER_PICKUP	(1<<1)
#define SF_WEAPON_NO_PHYSCANNON_PUNT (1<<2)


abstract_class Template_CCombatWeapon : public CAnimating
{
public:
	CE_DECLARE_CLASS( Template_CCombatWeapon, CAnimating );

public:
	virtual void	Drop( const Vector &vecVelocity );
	virtual int		GetMaxClip1( void ) const;
	virtual int		GetSlot( void ) const;

	virtual bool	HasPrimaryAmmo( void );	
	virtual bool	UsesClipsForAmmo1( void ) const;
	virtual bool	HasSecondaryAmmo( void );
	virtual bool	UsesClipsForAmmo2( void ) const;

	virtual bool	Holster( CBaseEntity *pSwitchingTo = NULL );
	virtual bool	Deploy();

public:
	DECLARE_DEFAULTHEADER(Drop, void, (const Vector &vecVelocity));
	DECLARE_DEFAULTHEADER(GetMaxClip1, int, () const);
	DECLARE_DEFAULTHEADER(GetSlot, int, () const);

	DECLARE_DEFAULTHEADER(HasPrimaryAmmo, bool, ());
	DECLARE_DEFAULTHEADER(UsesClipsForAmmo1, bool, () const);
	DECLARE_DEFAULTHEADER(HasSecondaryAmmo, bool, ());
	DECLARE_DEFAULTHEADER(UsesClipsForAmmo2, bool, () const);

	DECLARE_DEFAULTHEADER(Holster, bool, (CBaseEntity *pSwitchingTo));
	DECLARE_DEFAULTHEADER(Deploy, bool, ());

};

class CCombatWeapon : public CItem<Template_CCombatWeapon>
{
public:
	CE_DECLARE_CLASS( CCombatWeapon, CItem<Template_CCombatWeapon> );
	DECLARE_DATADESC();
	CE_CUSTOM_ENTITY();
	CCombatWeapon();

	void		Spawn();
	bool		MyTouch( CPlayer *pPlayer );
	bool		IsRemoveable() { return m_bRemoveable; }
	void		SetRemoveable( bool value ) { m_bRemoveable = value; }
	
	int GetPrimaryAmmoType() const { return m_iPrimaryAmmoType; }

	void OnWeaponDrop( CPlayer *pOwner);
	void OnWeaponEquip( CPlayer *pOwner );

	bool GetObjectsOriginalParameters(Vector &vOriginalOrigin, QAngle &vOriginalAngles);

	bool		UsesPrimaryAmmo( void );

	int			Clip1() const { return m_iClip1; }
	int			Clip2() const { return m_iClip2; }
	int			GetSubType( void ) { return m_iSubType; }

public:
	static const	WeaponProficiencyInfo_t *GetDefaultProficiencyValues();

protected:
	bool m_bRemoveOnDrop;

protected: //Sendprops


public:
	DECLARE_SENDPROP(int, m_iPrimaryAmmoType);
	DECLARE_SENDPROP(int, m_iSecondaryAmmoType);
	DECLARE_SENDPROP(int, m_iClip1);
	DECLARE_SENDPROP(int, m_iClip2);

	
protected: //Datamaps
	DECLARE_DATAMAP(bool, m_bRemoveable);
	DECLARE_DATAMAP(IPhysicsConstraint *, m_pConstraint);
	DECLARE_DATAMAP(int, m_iSubType);

};


#endif
