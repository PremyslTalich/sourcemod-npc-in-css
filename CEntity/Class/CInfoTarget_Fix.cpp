
#include "CInfoTarget_Fix.h"
#include "model_types.h"

void CE_InfoTarget_Fix::Spawn(void)
{

}


int	CE_InfoTarget_Fix::ObjectCaps( void )
{ 
	return CBaseEntity_ObjectCaps();
}

void CE_InfoTarget_Fix::Think(void)
{
	VALVE_BASEPTR original_think = m_pfnThink;
	if(original_think != NULL)
	{
		(BaseEntity()->*original_think)();
		return;
	}
}
