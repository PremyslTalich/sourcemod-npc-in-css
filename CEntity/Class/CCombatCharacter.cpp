
#include "CCombatCharacter.h"

CE_LINK_ENTITY_TO_CLASS(CBaseCombatCharacter, CCombatCharacter);


SH_DECL_MANUALHOOK1(IRelationType, 0, 0, 0, Disposition_t, CBaseEntity *);
DECLARE_HOOK(IRelationType, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, IRelationType, Disposition_t, (CBaseEntity *pTarget), (pTarget));

SH_DECL_MANUALHOOK1(FInAimCone_Entity, 0, 0, 0, bool, CBaseEntity *);
DECLARE_HOOK(FInAimCone_Entity, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, FInAimCone_Entity, bool, (CBaseEntity *pEntity), (pEntity));

SH_DECL_MANUALHOOK3(FVisible_Entity, 0, 0, 0, bool, CBaseEntity *, int, CBaseEntity **);
DECLARE_HOOK(FVisible_Entity, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, FVisible_Entity, bool, (CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker), (pEntity,  traceMask, ppBlocker));

SH_DECL_MANUALHOOK0(EyeDirection3D, 0, 0, 0, Vector);
DECLARE_HOOK(EyeDirection3D, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, EyeDirection3D, Vector, (), ());

SH_DECL_MANUALHOOK1(CorpseGib, 0, 0, 0, bool, const CTakeDamageInfo &);
DECLARE_HOOK(CorpseGib, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, CorpseGib, bool, (const CTakeDamageInfo &info), (info));

SH_DECL_MANUALHOOK1(OnTakeDamage_Alive, 0, 0, 0, int, const CTakeDamageInfo &);
DECLARE_HOOK(OnTakeDamage_Alive, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, OnTakeDamage_Alive, int, (const CTakeDamageInfo &info), (info));

SH_DECL_MANUALHOOK3(HandleInteraction, 0, 0, 0, bool, int, void *, CCombatCharacter*);
DECLARE_HOOK(HandleInteraction, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, HandleInteraction, bool, (int interactionType, void *data, CCombatCharacter* sourceEnt), (interactionType, data, sourceEnt));

SH_DECL_MANUALHOOK1(OnTakeDamage_Dying, 0, 0, 0, int, const CTakeDamageInfo &);
DECLARE_HOOK(OnTakeDamage_Dying, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, OnTakeDamage_Dying, int, (const CTakeDamageInfo &info), (info));

SH_DECL_MANUALHOOK1(OnTakeDamage_Dead, 0, 0, 0, int, const CTakeDamageInfo &);
DECLARE_HOOK(OnTakeDamage_Dead, CCombatCharacter);
DECLARE_DEFAULTHANDLER(CCombatCharacter, OnTakeDamage_Dead, int, (const CTakeDamageInfo &info), (info));



// Sendprops
DEFINE_PROP(m_flNextAttack, CCombatCharacter);


//Datamaps
DEFINE_PROP(m_eHull, CCombatCharacter);
DEFINE_PROP(m_bloodColor, CCombatCharacter);
DEFINE_PROP(m_flFieldOfView, CCombatCharacter);


void CCombatCharacter::SetBloodColor(int nBloodColor)
{
	m_bloodColor = nBloodColor;
}

int CCombatCharacter::OnTakeDamage(CEntityTakeDamageInfo &info)
{
	return g_helpfunc.CBaseCombatCharacter_OnTakeDamage(BaseEntity(), info);
}

Disposition_t IRelationType(CBaseEntity *pTarget)
{
	return D_HT;
}
