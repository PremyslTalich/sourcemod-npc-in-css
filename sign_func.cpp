#include "sign_func.h"
#include "CEntity.h"
#include "CDetour/detours.h"
#include "CAI_NPC.h"

#include "ai_namespaces.h"
#include "ai_schedule.h"

SH_DECL_MANUALHOOK0(GameRules_FAllowNPCsHook, 0, 0, 0, bool);

//extern CAI_SchedulesManager g_AI_SchedulesManager;


HelperFunction g_helpfunc;


CDetour *GetActivityIDCDetour;


DETOUR_DECL_STATIC1(GetActivityID, int, const char *, actName)
{
	return CAI_NPC::GetActivityID(actName);//DETOUR_STATIC_CALL(GetActivityID)(actName);
}


#define RegisterHook(name, hook)\
	if(!g_pGameConf->GetOffset(name, &offset))\
	{\
		return false;\
	}\
	SH_MANUALHOOK_RECONFIGURE(hook, offset, 0, 0);


bool HelperFunction::Initialize()
{
	char *addr = NULL;
	int offset = 0;

	RegisterHook("GameRules_FAllowNPCs",GameRules_FAllowNPCsHook);

	if(!g_pGameConf->GetMemSig("CreateGameRulesObject", (void **)&addr) || !addr)
		return false;

	if(!g_pGameConf->GetOffset("g_pGameRules", &offset) || !offset)
		return false;

	my_g_pGameRules = *reinterpret_cast<void ***>(addr + offset);

	
	g_AI_SchedulesManager.LoadAllSchedules();	

	//GetActivityIDCDetour = DETOUR_CREATE_STATIC(GetActivityID, "GetActivityID");
	//GetActivityIDCDetour->EnableDetour();	

	// "UTIL_EmitAmbientSound:  Sentence emitte"
	addr = reinterpret_cast<char *>(0x001E19B2+serverdll_addr+1);
	unsigned char *data = NULL;
	memcpy(&data, reinterpret_cast<void*>(addr), sizeof(char*));
	g_SoundEmitterSystem = (void *)data;

	return true;
}

void HelperFunction::GetEntityCollisionHash()
{
	// 0x1016750B
	// "VPhysics Penetration Error (%s)!" In CCollisionEvent__ShouldCollide, at UP
	void *addr = reinterpret_cast<void *>(0x0016750B+serverdll_addr);
	unsigned char temp;
	memcpy(&temp,addr,sizeof(temp));
	if(temp == 0x8B) { //  mov ecx
		void **my_g_EntityCollisionHash = *reinterpret_cast<void ***>(0x0016750B+serverdll_addr+2);

		unsigned char *col_obj = NULL;
		memcpy(&col_obj, reinterpret_cast<void*>(my_g_EntityCollisionHash), sizeof(char*));

		myg_EntityCollisionHash = reinterpret_cast<IPhysicsObjectPairHash *>(col_obj);
	} else {
		META_CONPRINT("[*] Unable getting g_EntityCollisionHash\n");
	}
}

void HelperFunction::GetWorldEntity()
{
	//0x10219650
	// "sv_stepsize" CWorld::Precache
	void *addr = reinterpret_cast<void *>(0x00219650+serverdll_addr);
	unsigned char temp;
	unsigned char *data = NULL;
	memcpy(&temp,addr,sizeof(temp));
	if(temp == 0x89) { //  mov     dword_10572868, edi
		void **my_g_WorldEntity = *reinterpret_cast<void ***>(0x00219650+serverdll_addr+2);

		memcpy(&data, reinterpret_cast<void*>(my_g_WorldEntity), sizeof(char*));

		g_WorldEntity = reinterpret_cast<CEntity *>(data);
	} else {
		META_CONPRINT("[*] Unable getting g_WorldEntity\n");
	}
}



bool HelperFunction::GameRules_FAllowNPCs()
{
	RETURN_META_VALUE(MRES_SUPERCEDE, true);
}

void HelperFunction::HookGameRules()
{
	void *rules = NULL;
	memcpy(&rules, reinterpret_cast<void*>(my_g_pGameRules), sizeof(char*));

	SH_ADD_MANUALHOOK_MEMFUNC(GameRules_FAllowNPCsHook, rules, &g_helpfunc, &HelperFunction::GameRules_FAllowNPCs, false);
}

void HelperFunction::UnHookGameRules()
{
	void *rules = NULL;
	memcpy(&rules, reinterpret_cast<void*>(my_g_pGameRules), sizeof(char*));
	if(rules == NULL)
		return;

	SH_REMOVE_MANUALHOOK_MEMFUNC(GameRules_FAllowNPCsHook, rules, &g_helpfunc, &HelperFunction::GameRules_FAllowNPCs, false);

}

bool HelperFunction::GameRules_ShouldCollide(int collisionGroup, int contentsMask)
{
	static int offset = NULL;
	if(!offset)
	{
		if(!g_pGameConf->GetOffset("GameRules_ShouldCollide", &offset))
		{
			assert(0);
			return false;
		}
	}
	
	unsigned char *rules = NULL;
	memcpy(&rules, reinterpret_cast<void*>(my_g_pGameRules), sizeof(char*));

	void **this_ptr = *reinterpret_cast<void ***>(&rules);
	void **vtable = *reinterpret_cast<void ***>(rules);
	void *vfunc = vtable[offset];

	union
	{
		bool (VEmptyClass::*mfpnew)(int, int);
		void *addr;
	} u;
	u.addr = vfunc;

	return (reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)(collisionGroup, contentsMask);
}

Activity HelperFunction::ActivityList_RegisterPrivateActivity( const char *pszActivityName )
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("ActivityList_RegisterPrivateActivity", &func))
		{
			assert(0);
			return ACT_INVALID;
		}
	}

	typedef Activity (*_func)(char const *);
    _func thisfunc = (_func)func;
    return thisfunc(pszActivityName);
}

Animevent HelperFunction::EventList_RegisterPrivateEvent( const char *pszEventName )
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("EventList_RegisterPrivateEvent", &func))
		{
			assert(0);
			return AE_INVALID;
		}
	}

	typedef Animevent (*_func)(char const *);
    _func thisfunc = (_func)func;
    return thisfunc(pszEventName);
}

CBaseEntity *HelperFunction::CreateEntityByName(char *entityname,int ForceEdictIndex)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("CreateEntityByName", &func))
		{
			assert(0);
			return NULL;
		}
	}

	CBaseEntity *pEntity = NULL;
	typedef CBaseEntity* (*_func)(char const *, int);
    _func thisfunc = (_func)func;
    pEntity = (CBaseEntity*)thisfunc(entityname, ForceEdictIndex);

	return pEntity;
}


CBaseEntity *HelperFunction::NPCPhysics_CreateSolver(CBaseEntity *pNPC, CBaseEntity *c, bool disableCollisions, float separationDuration)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("NPCPhysics_CreateSolver", &func))
		{
			assert(0);
			return NULL;
		}
	}

	CBaseEntity *pEntity = NULL;
	typedef CBaseEntity* (*_func)(CBaseEntity *, CBaseEntity *, bool, float);
    _func thisfunc = (_func)func;
    pEntity = (CBaseEntity*)thisfunc(pNPC, pNPC, disableCollisions, separationDuration);
	return pEntity;
}

HSOUNDSCRIPTHANDLE HelperFunction::PrecacheScriptSound(const char *soundname)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("PrecacheScriptSound", &func))
		{
			assert(0);
			return SOUNDEMITTER_INVALID_HANDLE;
		}
	}

	typedef HSOUNDSCRIPTHANDLE (__fastcall *_func)(void *, int , const char *);
    _func thisfunc = (_func)func;
    return thisfunc(g_SoundEmitterSystem, 0, soundname);
}


int HelperFunction::SelectWeightedSequence(void *pstudiohdr, int activity, int curSequence)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SelectWeightedSequence", &func))
		{
			assert(0);
			return 0;
		}
	}

	CBaseEntity *pEntity = NULL;
	typedef int (*_func)(void *, int, int);
    _func thisfunc = (_func)func;
    return thisfunc(pstudiohdr, activity, curSequence);	

}

void HelperFunction::SetNextThink(CBaseEntity *pEntity, float thinkTime, const char *szContext)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetNextThink", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, float, const char *);
    _func thisfunc = (_func)(func);

	(thisfunc)(pEntity,0,thinkTime,szContext);
}

void HelperFunction::SimThink_EntityChanged(CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SimThink_EntityChanged", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (*_func)(CBaseEntity *);
    _func thisfunc = (_func)(func);
	(thisfunc)(pEntity);
}

void HelperFunction::SetGroundEntity(CBaseEntity *pEntity, CBaseEntity *ground)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetGroundEntity", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, CBaseEntity *);
    _func thisfunc = (_func)(func);

	(thisfunc)(pEntity,0,ground);
}

void HelperFunction::SetAbsVelocity(CBaseEntity *pEntity, const Vector &vecAbsVelocity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetAbsVelocity", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, const Vector &);
    _func thisfunc = (_func)(func);

	(thisfunc)(pEntity,0,vecAbsVelocity);
}

void HelperFunction::SetAbsAngles(CBaseEntity *pEntity, const QAngle& absAngles)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetAbsAngles", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, const QAngle &);
    _func thisfunc = (_func)(func);

	(thisfunc)(pEntity,0,absAngles);
}

IServerVehicle *HelperFunction::GetServerVehicle(CBaseEntity *pEntity)
{
	static int offset = 0;
	if(!offset)
	{
		if(!g_pGameConf->GetOffset("GetServerVehicle", &offset))
		{
			assert(0);
			return NULL;
		}
	}

	if(!pEntity)
		return NULL;

	void **this_ptr = *reinterpret_cast<void ***>(&pEntity);
	void **vtable = *reinterpret_cast<void ***>(pEntity);
	void *vfunc = vtable[offset];

	union
	{
		IServerVehicle *(VEmptyClass::*mfpnew)();
		void *addr;
	} u;
	u.addr = vfunc;

	return (reinterpret_cast<VEmptyClass *>(this_ptr)->*u.mfpnew)();
}

void HelperFunction::EmitSound(CBaseEntity *pEntity, const char *soundname, float soundtime, float *duration)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("EmitSound_char_float_pfloat", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, const char *, float, float *);
	_func thisfunc = (_func)(func);
	(thisfunc)(pEntity,0,soundname, soundtime, duration);
}

void HelperFunction::SetSolid(void *Collision_ptr, SolidType_t val)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetSolid", &func))
		{
			assert(0);
			return;
		}
	}

	if(!Collision_ptr)
		return;

	typedef void (__fastcall *_func)(void *,int, int);
	_func thisfunc = (_func)(func);
	(thisfunc)(Collision_ptr,0,val);
}


void HelperFunction::SetSolidFlags(void *Collision_ptr, int flags)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetSolidFlags", &func))
		{
			assert(0);
			return;
		}
	}

	if(!Collision_ptr)
		return;

	typedef void (__fastcall *_func)(void *,int, int);
	_func thisfunc = (_func)(func);
	(thisfunc)(Collision_ptr,0,flags);
}

void HelperFunction::ReportEntityFlagsChanged(CBaseEntity *pEntity, unsigned int flagsOld, unsigned int flagsNow)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("ReportEntityFlagsChanged", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, CBaseEntity *, unsigned int, unsigned int);
	_func thisfunc = (_func)(func);
	(thisfunc)(g_pEntityList,0,pEntity, flagsOld, flagsNow);
}


void HelperFunction::AddPostClientMessageEntity(CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("AddPostClientMessageEntity", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, CBaseEntity *);
	_func thisfunc = (_func)(func);
	(thisfunc)(g_pEntityList,0,pEntity);
}


CEntity *HelperFunction::FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("FindEntityByClassname", &func))
		{
			assert(0);
			return NULL;
		}
	}

	typedef CBaseEntity *(__fastcall *_func)(void *,int, CBaseEntity *, const char *);
	_func thisfunc = (_func)(func);
	return CEntity::Instance((thisfunc)(g_pEntityList,0,pStartEntity, szName));
}


int HelperFunction::DispatchUpdateTransmitState(CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("DispatchUpdateTransmitState", &func))
		{
			assert(0);
			return 0;
		}
	}

	if(!pEntity)
		return 0;

	typedef int (__fastcall *_func)(void *,int);
	_func thisfunc = (_func)(func);
	return (thisfunc)(pEntity,0);
}


void HelperFunction::CAI_BaseNPC_Precache(CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("CAI_BaseNPC_Precache", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int);
	_func thisfunc = (_func)(func);
	(thisfunc)(pEntity,0);
}


bool HelperFunction::AutoMovement(CBaseEntity *pEntity, float flInterval, CBaseEntity *pTarget, AIMoveTrace_t *pTraceResult)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("CAI_BaseNPC_Precache", &func))
		{
			assert(0);
			return false;
		}
	}

	if(!pEntity)
		return false;

	typedef bool (__fastcall *_func)(void *,int, float , CBaseEntity *, AIMoveTrace_t *);
	_func thisfunc = (_func)(func);
	return (thisfunc)(pEntity,0, flInterval, pTarget, pTraceResult);
}


void HelperFunction::EndTaskOverlay(CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("EndTaskOverlay", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int);
	_func thisfunc = (_func)(func);
	(thisfunc)(pEntity,0);
}

void HelperFunction::SetIdealActivity(CBaseEntity *pEntity, Activity NewActivity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("SetIdealActivity", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int, Activity);
	_func thisfunc = (_func)(func);
	(thisfunc)(pEntity,0, NewActivity);
}

void HelperFunction::CallNPCThink(CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("CallNPCThink", &func))
		{
			assert(0);
			return;
		}
	}

	if(!pEntity)
		return;

	typedef void (__fastcall *_func)(void *,int);
	_func thisfunc = (_func)(func);
	(thisfunc)(pEntity,0);
}

bool HelperFunction::HaveSequenceForActivity(CBaseEntity *pEntity, Activity activity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("HaveSequenceForActivity", &func))
		{
			assert(0);
			return false;
		}
	}

	if(!pEntity)
		return false;

	typedef bool (__fastcall *_func)(void *,int, Activity);
	_func thisfunc = (_func)(func);
	return (thisfunc)(pEntity,0, activity);
}

int HelperFunction::CBaseCombatCharacter_OnTakeDamage(CBaseEntity *pEntity, CEntityTakeDamageInfo &info)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("CBaseCombatCharacter_OnTakeDamage", &func))
		{
			assert(0);
			return 0;
		}
	}

	if(!pEntity)
		return 0;

	typedef int (__fastcall *_func)(CBaseEntity *,int, CEntityTakeDamageInfo &);
	_func thisfunc = (_func)(func);
	return (thisfunc)(pEntity,0, info);
}


bool HelperFunction::ShouldBrushBeIgnored(void *ptr, CBaseEntity *pEntity)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("ShouldBrushBeIgnored", &func))
		{
			assert(0);
			return false;
		}
	}
	typedef bool (__fastcall *_func)(void *,int, CBaseEntity *);
	_func thisfunc = (_func)(func);
	return (thisfunc)(ptr,0, pEntity);
}

bool HelperFunction::MoveLimit(void *ptr, Navigation_t navType, const Vector &vecStart, 
		const Vector &vecEnd, unsigned int collisionMask, const CBaseEntity *pTarget, 
		float pctToCheckStandPositions, unsigned flags, AIMoveTrace_t* pTrace)
{
	static void *func = NULL;
	if(!func)
	{
		if(!g_pGameConf->GetMemSig("MoveLimit", &func))
		{
			assert(0);
			return false;
		}
	}
	typedef bool (__fastcall *_func)(void *,int, Navigation_t, const Vector &, const Vector &, unsigned int, const CBaseEntity *, float, unsigned, AIMoveTrace_t*);
	_func thisfunc = (_func)(func);
	return (thisfunc)(ptr,0, navType, vecStart, vecEnd, collisionMask, pTarget, pctToCheckStandPositions, flags, pTrace);
}


