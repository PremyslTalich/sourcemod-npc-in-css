
#ifndef _INCLUDE_CCOMBATCHARACTER_H_
#define _INCLUDE_CCOMBATCHARACTER_H_

#include "CFlex.h"
#include "ai_hull.h"

enum Capability_t 
{
	bits_CAP_MOVE_GROUND			= 0x00000001, // walk/run
	bits_CAP_MOVE_JUMP				= 0x00000002, // jump/leap
	bits_CAP_MOVE_FLY				= 0x00000004, // can fly, move all around
	bits_CAP_MOVE_CLIMB				= 0x00000008, // climb ladders
	bits_CAP_MOVE_SWIM				= 0x00000010, // navigate in water			// UNDONE - not yet implemented
	bits_CAP_MOVE_CRAWL				= 0x00000020, // crawl						// UNDONE - not yet implemented
	bits_CAP_MOVE_SHOOT				= 0x00000040, // tries to shoot weapon while moving
	bits_CAP_SKIP_NAV_GROUND_CHECK	= 0x00000080, // optimization - skips ground tests while computing navigation
	bits_CAP_USE					= 0x00000100, // open doors/push buttons/pull levers
	//bits_CAP_HEAR					= 0x00000200, // can hear forced sounds
	bits_CAP_AUTO_DOORS				= 0x00000400, // can trigger auto doors
	bits_CAP_OPEN_DOORS				= 0x00000800, // can open manual doors
	bits_CAP_TURN_HEAD				= 0x00001000, // can turn head, always bone controller 0
	bits_CAP_WEAPON_RANGE_ATTACK1	= 0x00002000, // can do a weapon range attack 1
	bits_CAP_WEAPON_RANGE_ATTACK2	= 0x00004000, // can do a weapon range attack 2
	bits_CAP_WEAPON_MELEE_ATTACK1	= 0x00008000, // can do a weapon melee attack 1
	bits_CAP_WEAPON_MELEE_ATTACK2	= 0x00010000, // can do a weapon melee attack 2
	bits_CAP_INNATE_RANGE_ATTACK1	= 0x00020000, // can do a innate range attack 1
	bits_CAP_INNATE_RANGE_ATTACK2	= 0x00040000, // can do a innate range attack 1
	bits_CAP_INNATE_MELEE_ATTACK1	= 0x00080000, // can do a innate melee attack 1
	bits_CAP_INNATE_MELEE_ATTACK2	= 0x00100000, // can do a innate melee attack 1
	bits_CAP_USE_WEAPONS			= 0x00200000, // can use weapons (non-innate attacks)
	//bits_CAP_STRAFE					= 0x00400000, // strafe ( walk/run sideways)
	bits_CAP_ANIMATEDFACE			= 0x00800000, // has animated eyes/face
	bits_CAP_USE_SHOT_REGULATOR		= 0x01000000, // Uses the shot regulator for range attack1
	bits_CAP_FRIENDLY_DMG_IMMUNE	= 0x02000000, // don't take damage from npc's that are D_LI
	bits_CAP_SQUAD					= 0x04000000, // can form squads
	bits_CAP_DUCK					= 0x08000000, // cover and reload ducking
	bits_CAP_NO_HIT_PLAYER			= 0x10000000, // don't hit players
	bits_CAP_AIM_GUN				= 0x20000000, // Use arms to aim gun, not just body
	bits_CAP_NO_HIT_SQUADMATES		= 0x40000000, // none
	bits_CAP_SIMPLE_RADIUS_DAMAGE	= 0x80000000, // Do not use robust radius damage model on this character.
};


enum Disposition_t 
{
	D_ER,		// Undefined - error
	D_HT,		// Hate
	D_FR,		// Fear
	D_LI,		// Like
	D_NU		// Neutral
};




class CCombatCharacter : public CFlex
{
public:
	CE_DECLARE_CLASS(CCombatCharacter, CFlex);

public:
	Hull_t	GetHullType() const				{ return m_eHull; }
	void	SetHullType( Hull_t hullType )	{ m_eHull = hullType; }

	void SetBloodColor(int nBloodColor);

	virtual void ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut ) { *vecForceOut = vecForceIn; } // Base class does nothing.

public:
	virtual Disposition_t IRelationType(CBaseEntity *pTarget);
	virtual bool FInAimCone_Entity(CBaseEntity *pEntity);
	virtual bool FVisible_Entity(CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );
	virtual Vector EyeDirection3D();
	virtual bool CorpseGib( const CTakeDamageInfo &info );
	virtual int	OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual int	OnTakeDamage_Dying( const CTakeDamageInfo &info );
	virtual int	OnTakeDamage_Dead( const CTakeDamageInfo &info );
	virtual bool HandleInteraction( int interactionType, void *data, CCombatCharacter* sourceEnt);
	virtual int OnTakeDamage(CEntityTakeDamageInfo &info);

public:
	DECLARE_DEFAULTHEADER(IRelationType, Disposition_t, (CBaseEntity *pTarget));
	DECLARE_DEFAULTHEADER(FInAimCone_Entity, bool, (CBaseEntity *pEntity));
	DECLARE_DEFAULTHEADER(FVisible_Entity, bool, (CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL));
	DECLARE_DEFAULTHEADER(EyeDirection3D, Vector, ());
	DECLARE_DEFAULTHEADER(CorpseGib, bool, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(OnTakeDamage_Alive, int, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(OnTakeDamage_Dying, int, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(OnTakeDamage_Dead, int, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(HandleInteraction, bool, (int interactionType, void *data, CCombatCharacter* sourceEnt));



protected: //Sendprops
	DECLARE_SENDPROP(float, m_flNextAttack);

protected: //Datamaps
	DECLARE_DATAMAP(Hull_t, m_eHull);
	DECLARE_DATAMAP(int, m_bloodColor);
	DECLARE_DATAMAP(float, m_flFieldOfView);
};

#endif
