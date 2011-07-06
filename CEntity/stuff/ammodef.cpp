#include "CEntity.h"
#include "ammodef.h"


CAmmoDef *GetAmmoDef()
{
	static CAmmoDef *def = NULL;
	if(def == NULL)
		def = g_helpfunc.GetAmmoDef();
	return def;
}

Ammo_t *CAmmoDef::GetAmmoOfIndex(int nAmmoIndex)
{
	if ( nAmmoIndex >= m_nAmmoIndex )
		return NULL;

	return &m_AmmoType[ nAmmoIndex ];
}

int CAmmoDef::Index(const char *psz)
{
	int i;

	if (!psz)
		return -1;

	for (i = 1; i < m_nAmmoIndex; i++)
	{
		if (stricmp( psz, m_AmmoType[i].pName ) == 0)
			return i;
	}

	return -1;
}

float CAmmoDef::DamageForce(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	return m_AmmoType[nAmmoIndex].physicsForceImpulse;
}

int	CAmmoDef::MaxCarry(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	if ( m_AmmoType[nAmmoIndex].pMaxCarry == USE_CVAR )
	{
		if ( m_AmmoType[nAmmoIndex].pMaxCarryCVar )
			return m_AmmoType[nAmmoIndex].pMaxCarryCVar->GetInt();

		return 0;
	}
	else
	{
		return m_AmmoType[nAmmoIndex].pMaxCarry;
	}
}

int	CAmmoDef::MinSplashSize(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 4;

	return m_AmmoType[nAmmoIndex].nMinSplashSize;
}

int	CAmmoDef::MaxSplashSize(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 8;

	return m_AmmoType[nAmmoIndex].nMaxSplashSize;
}

int	CAmmoDef::PlrDamage(int nAmmoIndex)
{
	if ( nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex )
		return 0;

	if ( m_AmmoType[nAmmoIndex].pPlrDmg == USE_CVAR )
	{
		if ( m_AmmoType[nAmmoIndex].pPlrDmgCVar )
		{
			return m_AmmoType[nAmmoIndex].pPlrDmgCVar->GetInt();
		}

		return 0;
	}
	else
	{
		return m_AmmoType[nAmmoIndex].pPlrDmg;
	}
}

int	CAmmoDef::DamageType(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 0;

	return m_AmmoType[nAmmoIndex].nDamageType;
}

int CAmmoDef::Flags(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 0;

	return m_AmmoType[nAmmoIndex].nFlags;
}

int	CAmmoDef::TracerType(int nAmmoIndex)
{
	if (nAmmoIndex < 1 || nAmmoIndex >= m_nAmmoIndex)
		return 0;

	return m_AmmoType[nAmmoIndex].eTracerType;
}
