
#include "CSode_Fix.h"


void CSode_Fix::Spawn(void)
{

}

void CSode_Fix::Precache(void)
{

}

void CSode_Fix::Think(void)
{
	VALVE_BASEPTR original_think = m_pfnThink;
	if(original_think != NULL)
	{
		(BaseEntity()->*original_think)();
		return;
	}
}

