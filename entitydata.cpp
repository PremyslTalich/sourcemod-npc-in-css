
#include "entitydata.h"

/*CSharedEdictChangeInfo *g_pSharedChangeInfo = NULL;

void EntityDataInitialize()
{
	g_pSharedChangeInfo = engine->GetSharedEdictChangeInfo();
}

IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
	return engine->GetChangeAccessor( (const edict_t *)this );
}
*/

inline edict_t *GetEntity(int num, CBaseEntity **pData)
{
	edict_t *pEdict = engine->PEntityOfEntIndex(num);
	if (!pEdict || pEdict->IsFree())
	{
		return NULL;
	}
	if (num > 0 && num <= g_Monster.GetMaxClients())
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(num);
		if (!pPlayer || !pPlayer->IsConnected())
		{
			return NULL;
		}
	}
	IServerUnknown *pUnk;
	if ((pUnk=pEdict->GetUnknown()) == NULL)
	{
		return NULL;
	}
	*pData = pUnk->GetBaseEntity();
	return pEdict;
}

bool DispatchSpawn(edict_t *pEdict)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("DispatchSpawn", &func))
		{
			return false;
		}
	}

	if(!pEdict || pEdict->IsFree())
	{
		return false;
	}
	IServerUnknown *pUnk;
	if ((pUnk=pEdict->GetUnknown()) == NULL)
	{
		return false;
	}
	CBaseEntity *pEntity = pUnk->GetBaseEntity();
	if(!pEntity)
	{
		return false;
	}

	typedef int (*_func)(CBaseEntity *);
    _func thisfunc = (_func)(func);
    int ret = (thisfunc)(pEntity);

	return (ret == -1) ? false : true;
}

bool DispatchKeyValue(edict_t *pEdict, char *keyname, char *value)
{
	static int offset = 0;
	if(!offset)
	{
		if(!g_pGameConf->GetOffset("DispatchKeyValue", &offset))
		{
			return false;
		}
	}

	if(!pEdict || pEdict->IsFree())
	{
		return false;
	}
	IServerUnknown *pUnk;
	if ((pUnk=pEdict->GetUnknown()) == NULL)
	{
		return false;
	}
	CBaseEntity *pEntity = pUnk->GetBaseEntity();
	if(!pEntity)
	{
		return false;
	}

	void **this_ptr = *reinterpret_cast<void ***>(&pEntity);
	void **vtable = *reinterpret_cast<void ***>(pEntity);
	void *vfunc = vtable[offset];

	union
	{
		int (VEmptyClass::*mfpnew)(char *,char *);
		void *addr;
	} u;
	u.addr = vfunc;

	int ret = (reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)(keyname,value);

	return (ret) ? true : false;
}

bool DispatchKeyValueFloat(edict_t *pEdict, char *keyname, float value)
{
	static int offset = 0;
	if(!offset)
	{
		if(!g_pGameConf->GetOffset("DispatchKeyValueFloat", &offset))
		{
			return false;
		}
	}

	if(!pEdict || pEdict->IsFree())
	{
		return false;
	}
	IServerUnknown *pUnk;
	if ((pUnk=pEdict->GetUnknown()) == NULL)
	{
		return false;
	}
	CBaseEntity *pEntity = pUnk->GetBaseEntity();
	if(!pEntity)
	{
		return false;
	}

	void **this_ptr = *reinterpret_cast<void ***>(&pEntity);
	void **vtable = *reinterpret_cast<void ***>(pEntity);
	void *vfunc = vtable[offset];

	union
	{
		int (VEmptyClass::*mfpnew)(char *,float);
		void *addr;
	} u;
	u.addr = vfunc;

	int ret = (reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)(keyname,value);

	return (ret) ? true : false;
}

bool DispatchKeyValueVector(edict_t *pEdict, char *keyname, Vector vec)
{
	static int offset = 0;
	if(!offset)
	{
		if(!g_pGameConf->GetOffset("DispatchKeyValueVector", &offset))
		{
			return false;
		}
	}

	if(!pEdict || pEdict->IsFree())
	{
		return false;
	}
	IServerUnknown *pUnk;
	if ((pUnk=pEdict->GetUnknown()) == NULL)
	{
		return false;
	}
	CBaseEntity *pEntity = pUnk->GetBaseEntity();
	if(!pEntity)
	{
		return false;
	}

	void **this_ptr = *reinterpret_cast<void ***>(&pEntity);
	void **vtable = *reinterpret_cast<void ***>(pEntity);
	void *vfunc = vtable[offset];

	union
	{
		int (VEmptyClass::*mfpnew)(char *,Vector);
		void *addr;
	} u;
	u.addr = vfunc;

	int ret = (reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)(keyname,vec);

	return (ret) ? true : false;
}
bool IsValidEdict(edict_t *pEdict)
{
	if (!pEdict)
	{
		return false;
	}
	return pEdict->IsFree() ? false : true;
}
bool IsValidEntity(edict_t *pEdict)
{
	if(!pEdict)
	{
		return false;
	}

	IServerUnknown *pUnknown = pEdict->GetUnknown();
	if (!pUnknown)
	{
		return false;
	}
	CBaseEntity *pEntity = pUnknown->GetBaseEntity();
	return (pEntity != NULL) ? true : false;
}

edict_t *CheckBaseHandle(CBaseHandle &hndl)
{
	if (!hndl.IsValid())
	{
		return NULL;
	}

	int index = hndl.GetEntryIndex();

	edict_t *pStoredEdict;
	CBaseEntity *pStoredEntity;

	pStoredEdict = GetEntity(index, &pStoredEntity);

	if (pStoredEdict == NULL || pStoredEntity == NULL)
	{
		return NULL;
	}

	IServerEntity *pSE = pStoredEdict->GetIServerEntity();

	if (pSE == NULL)
	{
		return NULL;
	}

	if (pSE->GetRefEHandle() != hndl)
	{
		return NULL;
	}

	return pStoredEdict;
}
edict_t *GetEntDataEnt2(CBaseEntity *pEntity, int offset)
{
	if(!pEntity)
	{
		return NULL;
	}

	CBaseHandle &hndl = *(CBaseHandle *)((unsigned char *)pEntity + offset);

	return CheckBaseHandle(hndl);
}

edict_t *GetEntDataEnt2(edict_t *pEdict, int offset)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return NULL;
	}

	CBaseHandle &hndl = *(CBaseHandle *)((unsigned char *)pEntity + offset);

	return CheckBaseHandle(hndl);
}
void SetEntData(CBaseEntity *pEntity, int offset, int value, int size, bool changeState)
{
	if(!pEntity)
	{
		return;
	}
	if (changeState)
	{
		edict_t *pEdict = servergameents->BaseEntityToEdict(pEntity);
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}

	switch (size)
	{
	case 4:
		{
			*(int *)((unsigned char *)pEntity + offset) = value;
			break;
		}
	case 2:
		{
			*(short *)((unsigned char *)pEntity + offset) = value;
			break;
		}
	case 1:
		{
			*((unsigned char *)pEntity + offset) = value;
			break;
		}
	default:
		return;
	}
	return;
}
void SetEntData(edict_t *pEdict, int offset, int value, int size, bool changeState)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if(!pEdict || !pEntity)
	{
		return;
	}

	if(changeState)
	{
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}

	switch (size)
	{
	case 4:
		{
			*(int *)((unsigned char *)pEntity + offset) = value;
			break;
		}
	case 2:
		{
			*(short *)((unsigned char *)pEntity + offset) = value;
			break;
		}
	case 1:
		{
			*((unsigned char *)pEntity + offset) = value;
			break;
		}
	default:
		return;
	}

	return;
}

void SetEntDataEnt(edict_t *pEdict, int offset, edict_t *pEdict_Target, bool changeState)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return;
	}

	CBaseHandle &hndl = *(CBaseHandle *)((unsigned char *)pEntity + offset);

	if (pEdict_Target == NULL)
	{
		hndl.Set(NULL);
	} else {
		IServerEntity *pEntOther = pEdict_Target->GetIServerEntity();
		hndl.Set(pEntOther);
	}
	if(changeState)
	{
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}
	return;
}

void SetEntDataVector(CBaseEntity *pEntity, int offset, const Vector &vec, bool changeState)
{
	if (!pEntity)
	{
		return;
	}

	Vector *v = (Vector *)((unsigned char *)pEntity + offset);
	v->x = vec.x;
	v->y = vec.y;
	v->z = vec.z;

	if(changeState)
	{
		edict_t *pEdict = servergameents->BaseEntityToEdict(pEntity);
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}
	return;

}
void SetEntDataVector(edict_t *pEdict, int offset, const Vector &vec, bool changeState)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return;
	}

	Vector *v = (Vector *)((unsigned char *)pEntity + offset);
	v->x = vec.x;
	v->y = vec.y;
	v->z = vec.z;

	if(changeState)
	{
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}
	return;

}
int GetEntData(edict_t *pEdict, int offset, int size)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return -2;
	}
	switch (size)
	{
		case 4:
			return *(int *)((unsigned char *)pEntity + offset);
		case 2:
			return *(short *)((unsigned char *)pEntity + offset);
		case 1:
			return *((unsigned char *)pEntity + offset);
		default:
			return -1;
	}
}
int GetEntData(CBaseEntity *pEntity, int offset, int size)
{
	if(!pEntity)
	{
		return -2;
	}
	switch (size)
	{
		case 4:
			return *(int *)((unsigned char *)pEntity + offset);
		case 2:
			return *(short *)((unsigned char *)pEntity + offset);
		case 1:
			return *((unsigned char *)pEntity + offset);
		default:
			return -1;
	}
}
void GetEntDataVector(CBaseEntity *pEntity, int offset, Vector *vec)
{
	if (!pEntity)
	{
		return;
	}

	*vec = *(Vector *)((unsigned char *)pEntity + offset);
	return;
}

void GetEntDataVector(edict_t *pEdict, int offset, Vector *vec)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return;
	}

	*vec = *(Vector *)((unsigned char *)pEntity + offset);
	return;
}

void SetEntDataArray(edict_t *pEdict, int offset, const int array[], int arraySize, int dataSize, bool changeState)
{
	for (int i=0; i<arraySize; i++)
	{
		SetEntData(pEdict,offset + i*dataSize, array[i], dataSize, changeState);
	}
}

void SetEntDataArray(CBaseEntity *pEntity, int offset, const int array[], int arraySize, int dataSize, bool changeState)
{
	for (int i=0; i<arraySize; i++)
	{
		SetEntData(pEntity,offset + i*dataSize, array[i], dataSize, changeState);
	}
}

void SetEntDataFloat(CBaseEntity *pEntity, int offset, float value, bool changeState)
{
	if(!pEntity)
	{
		return;
	}
	*(float *)((unsigned char *)pEntity + offset) = value;
	if(changeState)
	{
		edict_t *pEdict = servergameents->BaseEntityToEdict(pEntity);
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}
	return;
}
void SetEntDataFloat(edict_t *pEdict, int offset, float value, bool changeState)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return ;
	}

	*(float *)((unsigned char *)pEntity + offset) = value;
	if (changeState)
	{
		//pEdict->StateChanged(offset);
		gamehelpers->SetEdictStateChanged(pEdict,offset);
	}
	return;
}
float GetEntDataFloat(edict_t *pEdict, int offset)
{
	CBaseEntity *pEntity = servergameents->EdictToBaseEntity(pEdict);
	if (!pEdict || !pEntity)
	{
		return -1;
	}
	return *(float *)((unsigned char *)pEntity + offset);
}
float GetEntDataFloat(CBaseEntity *pEntity, int offset)
{
	if(!pEntity)
	{
		return -1;
	}
	return *(float *)((unsigned char *)pEntity + offset);
}

