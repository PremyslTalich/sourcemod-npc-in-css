
#ifndef _INCLUDE_CGRENADE_H_
#define _INCLUDE_CGRENADE_H_

#include "CEntity.h"
#include "CAnimating.h"

class CE_Grenade : public CAnimating
{
public:
	CE_DECLARE_CLASS( CE_Grenade, CAnimating );

	void PostInit();

	CCombatCharacter *GetThrower( void );
	void DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void SetThrower( CBaseEntity *pThrower );

public:
	virtual void Detonate( void );

public:
	DECLARE_DEFAULTHEADER(Detonate, void, ());

protected: //Sendprops
	DECLARE_SENDPROP(float, m_flDamage);
	DECLARE_SENDPROP(float, m_DmgRadius);
	DECLARE_SENDPROP(CFakeHandle, m_hThrower);

public: //Datamaps
	DECLARE_DATAMAP_OFFSET(CFakeHandle,m_hOriginalThrower);

};


#endif

