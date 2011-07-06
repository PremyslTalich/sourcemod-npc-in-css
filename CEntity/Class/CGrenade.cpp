
#include "CGrenade.h"


CE_LINK_ENTITY_TO_CLASS(CBaseGrenade, CE_Grenade);

SH_DECL_MANUALHOOK0_void(Detonate, 0, 0, 0);
DECLARE_HOOK(Detonate, CE_Grenade);
DECLARE_DEFAULTHANDLER_void(CE_Grenade, Detonate, (), ());


// Sendprops
DEFINE_PROP(m_flDamage, CE_Grenade);
DEFINE_PROP(m_DmgRadius, CE_Grenade);
DEFINE_PROP(m_hThrower, CE_Grenade);

void CE_Grenade::PostInit()
{
	m_hOriginalThrower.offset = m_hThrower.offset + 4;
	m_hOriginalThrower.ptr = (CFakeHandle *)(((uint8_t *)(BaseEntity())) + m_hOriginalThrower.offset);
}

CCombatCharacter *CE_Grenade::GetThrower( void )
{
	CCombatCharacter *pResult = ToBaseCombatCharacter( m_hThrower );
	if ( !pResult && GetOwnerEntity() != NULL )
	{
		pResult = ToBaseCombatCharacter( GetOwnerEntity() );
	}
	return pResult;
}

void CE_Grenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CE_Grenade::Detonate );
	SetNextThink( gpGlobals->curtime );
}


void CE_Grenade::SetThrower( CBaseEntity *pThrower )
{
	m_hThrower.ptr->Set(pThrower);

	// if this is the first thrower, set it as the original thrower
	if ( m_hOriginalThrower.ptr->Get() == NULL)
	{
		m_hOriginalThrower.ptr->Set(pThrower);
	}
}


