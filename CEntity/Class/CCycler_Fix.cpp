
#include "CCycler_Fix.h"

CE_LINK_ENTITY_TO_CLASS(CCycler, CE_Cycler_Fix);

//extern CAI_ClassScheduleIdSpace *my_gm_ClassScheduleIdSpace;


void CE_Cycler_Fix::Spawn(void)
{
	DUMP_FUNCTION();

	/*if(my_gm_ClassScheduleIdSpace == NULL)
	{
		my_gm_ClassScheduleIdSpace = GetClassScheduleIdSpace();
	}*/

	/*CAI_LocalIdSpace *m_ConditionIds = &space->m_ConditionIds;
	m_ConditionIds->m_pParentIDSpace = NULL;
	m_ConditionIds->m_pGlobalNamespace = NULL;*/


	//BaseClass::Spawn();
	// do nothing
}

void CE_Cycler_Fix::Precache(void)
{
	DUMP_FUNCTION();

	g_helpfunc.CAI_BaseNPC_Precache(BaseEntity());
}

void CE_Cycler_Fix::Think(void)
{
	DUMP_FUNCTION();

	CBaseEntity *cbase = BaseEntity();
	datamap_t *pMap = gamehelpers->GetDataMap(cbase);
	typedescription_t *td = gamehelpers->FindInDataMap(pMap,"m_pfnThink");
	int offset = td->fieldOffset[TD_OFFSET_NORMAL];

	void * addr = *(void* *)((unsigned char *)cbase + offset);

	typedef void (__fastcall *_func)(void *,int);
	_func thisfunc = (_func)(addr);
	(thisfunc)(cbase,0);

	//BaseClass::Think();
	// NPCInitThink
}

int CE_Cycler_Fix::OnTakeDamage(CEntityTakeDamageInfo& info)
{
	DUMP_FUNCTION();

	return CCombatCharacter::OnTakeDamage(info);
}

bool CE_Cycler_Fix::IsAlive(void)
{
	DUMP_FUNCTION();

	return (m_lifeState == LIFE_ALIVE); 
}

Disposition_t CE_Cycler_Fix::IRelationType ( CBaseEntity *pTarget )
{
	//Disposition_t dd = BaseClass::IRelationType(pTarget);
	return D_HT;
}
