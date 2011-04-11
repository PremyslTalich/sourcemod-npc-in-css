/**
* =============================================================================
* CEntity Entity Handling Framework
* Copyright (C) 2011 Matt Woodrow.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CEntity.h"
#include "shareddefs.h"
#include "CEntityManager.h"
#include "CTakeDamageInfo.h"
#include "CAI_NPC.h"
#include "model_types.h"


IHookTracker *IHookTracker::m_Head = NULL;
IPropTracker *IPropTracker::m_Head = NULL;
IDetourTracker *IDetourTracker::m_Head = NULL;
ISigOffsetTracker *ISigOffsetTracker::m_Head = NULL;

ISaveRestoreOps *eventFuncs = NULL;

SH_DECL_MANUALHOOK3_void(Teleport, 0, 0, 0, const Vector *, const QAngle *, const Vector *);
SH_DECL_MANUALHOOK0_void(UpdateOnRemove, 0, 0, 0);
SH_DECL_MANUALHOOK0_void(Spawn, 0, 0, 0);
SH_DECL_MANUALHOOK1(OnTakeDamage, 0, 0, 0, int, CEntityTakeDamageInfo &);
SH_DECL_MANUALHOOK0_void(Think, 0, 0, 0);
SH_DECL_MANUALHOOK5(AcceptInput, 0, 0, 0, bool, const char *, CBaseEntity *, CBaseEntity *, variant_t, int);
SH_DECL_MANUALHOOK0(GetDataDescMap, 0, 0, 0, datamap_t *);
SH_DECL_MANUALHOOK1_void(StartTouch, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK1_void(Touch, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK1_void(EndTouch, 0, 0, 0, CBaseEntity *);
SH_DECL_MANUALHOOK0(GetSoundEmissionOrigin, 0, 0, 0, Vector);
SH_DECL_MANUALHOOK1(VPhysicsTakeDamage, 0, 0, 0, int, const CEntityTakeDamageInfo &);
SH_DECL_MANUALHOOK2(VPhysicsGetObjectList, 0, 0, 0, int, IPhysicsObject **, int);
SH_DECL_MANUALHOOK0(GetServerClass, 0, 0, 0, ServerClass *);
SH_DECL_MANUALHOOK1_void(SetModel, 0, 0, 0, char *);
SH_DECL_MANUALHOOK0(Classify, 0, 0, 0, Class_T);
SH_DECL_MANUALHOOK2(ShouldCollide, 0, 0, 0, bool, int, int);
SH_DECL_MANUALHOOK0(ObjectCaps, 0, 0, 0, int);
SH_DECL_MANUALHOOK0(IsNPC, 0, 0, 0, bool);
SH_DECL_MANUALHOOK2_void(SetParent, 0, 0, 0, CBaseEntity *, int);
SH_DECL_MANUALHOOK2_void(DecalTrace, 0, 0, 0, trace_t *, char const *);
SH_DECL_MANUALHOOK1_void(Event_Killed, 0, 0, 0, const CTakeDamageInfo &);
SH_DECL_MANUALHOOK3_void(TraceAttack, 0, 0, 0, const CTakeDamageInfo &, const Vector &, trace_t *);
SH_DECL_MANUALHOOK2(BodyTarget, 0, 0, 0, Vector, const Vector &, bool);
SH_DECL_MANUALHOOK0(GetServerVehicle, 0, 0, 0, IServerVehicle *);
SH_DECL_MANUALHOOK0(IsAlive, 0, 0, 0, bool);
SH_DECL_MANUALHOOK0(WorldSpaceCenter, 0, 0, 0, const Vector &);
SH_DECL_MANUALHOOK0_void(PhysicsSimulate, 0, 0, 0);


DECLARE_HOOK(Teleport, CEntity);
DECLARE_HOOK(UpdateOnRemove, CEntity);
DECLARE_HOOK(Spawn, CEntity);
DECLARE_HOOK(OnTakeDamage, CEntity);
DECLARE_HOOK(Think, CEntity);
DECLARE_HOOK(AcceptInput, CEntity);
DECLARE_HOOK(GetDataDescMap, CEntity);
DECLARE_HOOK(StartTouch, CEntity);
DECLARE_HOOK(Touch, CEntity);
DECLARE_HOOK(EndTouch, CEntity);
DECLARE_HOOK(GetSoundEmissionOrigin, CEntity);
DECLARE_HOOK(VPhysicsTakeDamage, CEntity);
DECLARE_HOOK(VPhysicsGetObjectList, CEntity);
DECLARE_HOOK(GetServerClass, CEntity);
DECLARE_HOOK(SetModel, CEntity);
DECLARE_HOOK(Classify, CEntity);
DECLARE_HOOK(ShouldCollide, CEntity);
DECLARE_HOOK(ObjectCaps, CEntity);
DECLARE_HOOK(IsNPC, CEntity);
DECLARE_HOOK(SetParent, CEntity);
DECLARE_HOOK(DecalTrace, CEntity);
DECLARE_HOOK(Event_Killed, CEntity);
DECLARE_HOOK(TraceAttack, CEntity);
DECLARE_HOOK(BodyTarget, CEntity);
//DECLARE_HOOK(GetServerVehicle, CEntity);
DECLARE_HOOK(IsAlive, CEntity);
DECLARE_HOOK(WorldSpaceCenter, CEntity);
DECLARE_HOOK(PhysicsSimulate, CEntity);


DECLARE_DEFAULTHANDLER_void(CEntity, Teleport, (const Vector *origin, const QAngle* angles, const Vector *velocity), (origin, angles, velocity));
DECLARE_DEFAULTHANDLER_void(CEntity, Spawn, (), ());
DECLARE_DEFAULTHANDLER(CEntity, OnTakeDamage, int, (CEntityTakeDamageInfo &info), (info));
DECLARE_DEFAULTHANDLER(CEntity, VPhysicsTakeDamage, int, (const CEntityTakeDamageInfo &inputInfo), (inputInfo));
DECLARE_DEFAULTHANDLER(CEntity, VPhysicsGetObjectList, int, (IPhysicsObject **pList, int listMax), (pList, listMax));
DECLARE_DEFAULTHANDLER(CEntity, GetServerClass, ServerClass *, (), ());
DECLARE_DEFAULTHANDLER_void(CEntity,SetModel,(char *model),(model));
DECLARE_DEFAULTHANDLER(CEntity,Classify,Class_T,(),());
DECLARE_DEFAULTHANDLER(CEntity,ShouldCollide,bool,(int collisionGroup, int contentsMask),(collisionGroup, contentsMask));
DECLARE_DEFAULTHANDLER(CEntity,ObjectCaps,int,(),());
DECLARE_DEFAULTHANDLER(CEntity,IsNPC,bool,(),());
DECLARE_DEFAULTHANDLER_void(CEntity,SetParent,(CBaseEntity *pParentEntity, int iParentAttachment),(pParentEntity, iParentAttachment));
DECLARE_DEFAULTHANDLER_void(CEntity,DecalTrace, (trace_t *pTrace, char const *decalName) ,(pTrace, decalName));
DECLARE_DEFAULTHANDLER_void(CEntity,Event_Killed, (const CTakeDamageInfo &info), (info));
DECLARE_DEFAULTHANDLER_void(CEntity,TraceAttack, (const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr), (info, vecDir, ptr));
DECLARE_DEFAULTHANDLER(CEntity,BodyTarget, Vector, (const Vector &posSrc, bool bNoisy),(posSrc, bNoisy));
DECLARE_DEFAULTHANDLER(CEntity,IsAlive, bool, (),());
DECLARE_DEFAULTHANDLER(CEntity,WorldSpaceCenter, const Vector &, (),());
DECLARE_DEFAULTHANDLER_void(CEntity,PhysicsSimulate, (),());



//Sendprops
DEFINE_PROP(m_iTeamNum, CEntity);
DEFINE_PROP(m_vecOrigin, CEntity);
DEFINE_PROP(m_CollisionGroup, CEntity);
DEFINE_PROP(m_hOwnerEntity, CEntity);
DEFINE_PROP(m_fEffects, CEntity);
DEFINE_PROP(m_vecVelocity, CEntity);
DEFINE_PROP(m_usSolidFlags, CEntity);
DEFINE_PROP(m_clrRender, CEntity);
DEFINE_PROP(m_nRenderMode, CEntity);
DEFINE_PROP(m_nSolidType, CEntity);
DEFINE_PROP(m_nModelIndex, CEntity);
DEFINE_PROP(m_angRotation, CEntity);
DEFINE_PROP(m_vecMins, CEntity);
DEFINE_PROP(m_vecMaxs, CEntity);



//Datamaps
DEFINE_PROP(m_vecAbsOrigin, CEntity);
DEFINE_PROP(m_vecAbsVelocity, CEntity);
DEFINE_PROP(m_nNextThinkTick, CEntity);
DEFINE_PROP(m_rgflCoordinateFrame, CEntity);
DEFINE_PROP(m_vecAngVelocity, CEntity);
DEFINE_PROP(m_vecBaseVelocity, CEntity);
DEFINE_PROP(m_hMoveParent, CEntity);
DEFINE_PROP(m_iEFlags, CEntity);
DEFINE_PROP(m_pPhysicsObject, CEntity);
DEFINE_PROP(m_pParent, CEntity);
DEFINE_PROP(m_MoveType, CEntity);
DEFINE_PROP(m_MoveCollide, CEntity);
DEFINE_PROP(m_iName, CEntity);

DEFINE_PROP(m_Collision, CEntity);
DEFINE_PROP(m_vecViewOffset, CEntity);
DEFINE_PROP(m_spawnflags, CEntity);
DEFINE_PROP(m_iHealth, CEntity);
DEFINE_PROP(m_fFlags, CEntity);
DEFINE_PROP(m_takedamage, CEntity);
DEFINE_PROP(m_Network, CEntity);
DEFINE_PROP(m_flAnimTime, CEntity);
DEFINE_PROP(m_flPrevAnimTime, CEntity);
DEFINE_PROP(m_lifeState, CEntity);
DEFINE_PROP(m_nWaterLevel, CEntity);
DEFINE_PROP(m_hGroundEntity, CEntity);
DEFINE_PROP(m_angAbsRotation, CEntity);
DEFINE_PROP(m_flGravity, CEntity);
DEFINE_PROP(m_iMaxHealth, CEntity);



/* MUST BE HERE */
DEFINE_PROP(m_iClassname, CEntity);
/* MUST BE HERE */


DECLARE_DETOUR(SetLocalOrigin, CEntity);
DECLARE_DEFAULTHANDLER_DETOUR_void(CEntity, SetLocalOrigin, (const Vector& origin ), (origin));

DECLARE_DETOUR(PhysicsTouchTriggers, CEntity);
DECLARE_DEFAULTHANDLER_DETOUR_void(CEntity, PhysicsTouchTriggers, (const Vector *pPrevAbsOrigin ), (pPrevAbsOrigin));

DECLARE_DETOUR(TakeDamage, CEntity);
DECLARE_DEFAULTHANDLER_DETOUR_void(CEntity, TakeDamage, (const CTakeDamageInfo &inputInfo), (inputInfo));

DECLARE_DETOUR(SetAbsOrigin, CEntity);
DECLARE_DEFAULTHANDLER_DETOUR_void(CEntity, SetAbsOrigin, (const Vector& absOrigin), (absOrigin));

DECLARE_DETOUR(SetLocalAngles, CEntity);
DECLARE_DEFAULTHANDLER_DETOUR_void(CEntity, SetLocalAngles, (const QAngle& angles), (angles));


/* Hacked Datamap declaration to fallback to the corresponding real entities one */
datamap_t CEntity::m_DataMap = { 0, 0, "CEntity", NULL };
//DECLARE_DEFAULTHANDLER(CEntity, GetDataDescMap, datamap_t *, (), ());
datamap_t *CEntity::GetBaseMap() { return NULL; }
BEGIN_DATADESC_GUTS(CEntity)
END_DATADESC()

PhysIsInCallbackFuncType PhysIsInCallback;

IServerVehicle *CEntity::GetServerVehicle()
{
	return g_helpfunc.GetServerVehicle(BaseEntity());
}

/*IServerVehicle *CEntity::GetServerVehicle()
{
	//return SH_MCALL(META_IFACEPTR(CBaseEntity), GetServerVehicle)();
	return SH_MCALL(BaseEntity(), GetServerVehicle)();
}

IServerVehicle *CEntity::InternalGetServerVehicle()
{
	//Do absolutely nothing since the iface ptr is 0xcccccccc sometimes and we can't handle that yet
	RETURN_META_VALUE(MRES_IGNORED, NULL);
}*/

datamap_t* CEntity::GetDataDescMap()
{
	if (!m_bInGetDataDescMap)
		return SH_MCALL(BaseEntity(), GetDataDescMap)();

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	RETURN_META_VALUE(MRES_SUPERCEDE, (thisptr->*(__SoureceHook_FHM_GetRecallMFPGetDataDescMap(thisptr)))());
}

datamap_t* CEntity::InternalGetDataDescMap()
{
	SET_META_RESULT(MRES_SUPERCEDE);
	CEntity *pEnt = (CEntity *)CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
		RETURN_META_VALUE(MRES_IGNORED, (datamap_t *)0);
	int index = pEnt->entindex();
	pEnt->m_bInGetDataDescMap = true;
	datamap_t* retvalue = pEnt->GetDataDescMap();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInGetDataDescMap = false;
	return retvalue;
}

CE_LINK_ENTITY_TO_CLASS(baseentity, CEntity);

variant_t g_Variant;


void CEntity::Init(edict_t *pEdict, CBaseEntity *pBaseEntity)
{
	m_pEntity = pBaseEntity;
	m_pEdict = pEdict;

	char vtable[20];
	_snprintf(vtable, sizeof(vtable), "%x", gamehelpers->EntityToReference(m_pEntity));

	cell_t s = gamehelpers->EntityToReference(m_pEntity);
	cell_t p = gamehelpers->EntityToBCompatRef(m_pEntity);
	
	//pEntityData_lookup.insert(vtable, this);

	/*char vtable[20];
	_snprintf(vtable, sizeof(vtable), "%x", (unsigned int)m_pEntity);
	CEntity *cent = this;
	pEntityData_lookup.insert(vtable, cent);*/

	/*CEntity_Data *data = new CEntity_Data();
	data->centity = this;
	data->cbase = m_pEntity;
	CEntity_lookup.push_back(data);*/

	assert(!pEntityData[entindex()]);

	pEntityData[entindex()] = this;
	if(!m_pEntity || !m_pEdict)
		return;

	m_pfnThink = NULL;
	m_pfnTouch = NULL;

	col_ptr = new CECollisionProperty(this);
}

void CEntity::Destroy()
{
//	META_CONPRINTF("Destroy %s\n", GetClassname());
	pEntityData[entindex()] = NULL;
	delete col_ptr;
	delete this;

}

CBaseEntity * CEntity::BaseEntity()
{
	return m_pEntity;
}

CBaseEntity * CEntity::BaseEntity() const
{
	return m_pEntity;
}

IHandleEntity *CEntity::GetIHandle() const
{
	return reinterpret_cast<IHandleEntity *>(m_pEntity);
}

/* Expanded handler for readability and since this one actually does something */
void CEntity::UpdateOnRemove()
{
	if (!m_bInUpdateOnRemove)
	{
		SH_MCALL(BaseEntity(), UpdateOnRemove);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);

	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPUpdateOnRemove(thisptr)))();

	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalUpdateOnRemove()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInUpdateOnRemove = true;
	pEnt->UpdateOnRemove();
	if (pEnt == CEntity::Instance(index))
	{
		pEnt->m_bInUpdateOnRemove = false;
		pEnt->Destroy();
	}
}


void CEntity::StartTouch(CEntity *pOther)
{
	if (!m_bInStartTouch)
	{
		SH_MCALL(BaseEntity(), StartTouch)(*pOther);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPStartTouch(thisptr)))(*pOther);
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalStartTouch(CBaseEntity *pOther)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = *META_IFACEPTR(CBaseEntity);
	CEntity *pEntOther = *pOther;
	if (!pEnt || !pEntOther)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInStartTouch = true;
	pEnt->StartTouch(pEntOther);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInStartTouch = false;
}

void CEntity::EndTouch(CEntity *pOther)
{
	if (!m_bInEndTouch)
	{
		SH_MCALL(BaseEntity(), EndTouch)(*pOther);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPEndTouch(thisptr)))(*pOther);
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalEndTouch(CBaseEntity *pOther)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = *META_IFACEPTR(CBaseEntity);
	CEntity *pEntOther = *pOther;
	if (!pEnt || !pEntOther)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInEndTouch = true;
	pEnt->EndTouch(pEntOther);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInEndTouch = false;
}

void CEntity::Touch(CEntity *pOther)
{
	if ( m_pfnTouch ) 
	{
		SET_META_RESULT(MRES_SUPERCEDE);
		(this->*m_pfnTouch)(pOther);
		return;
	}

	//if (m_pParent)
	//	m_pParent->Touch(pOther);

	if (!m_bInTouch)
	{
		SH_MCALL(BaseEntity(), Touch)(*pOther);
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPTouch(thisptr)))(*pOther);
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalTouch(CBaseEntity *pOther)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = *META_IFACEPTR(CBaseEntity);
	CEntity *pEntOther = *pOther;
	if (!pEnt || !pEntOther)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInTouch = true;
	pEnt->Touch(pEntOther);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInTouch = false;
}

Vector CEntity::GetSoundEmissionOrigin()
{
	if (!m_bInGetSoundEmissionOrigin)
	{
		Vector ret = SH_MCALL(BaseEntity(), GetSoundEmissionOrigin)();
		return ret;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	Vector ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPGetSoundEmissionOrigin(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);

	return ret;
}

Vector CEntity::InternalGetSoundEmissionOrigin()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = *META_IFACEPTR(CBaseEntity);
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, NULL);
	}

	int index = pEnt->entindex();
	pEnt->m_bInGetSoundEmissionOrigin = true;
	Vector ret = pEnt->GetSoundEmissionOrigin();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInGetSoundEmissionOrigin = false;

	return ret;
}

void CEntity::Think()
{
	if (m_pfnThink)
	{
		SET_META_RESULT(MRES_SUPERCEDE);
		(this->*m_pfnThink)();
		return;
	}

	if (!m_bInThink)
	{
		SH_MCALL(BaseEntity(), Think)();
		return;
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	(thisptr->*(__SoureceHook_FHM_GetRecallMFPThink(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);
}

void CEntity::InternalThink()
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = CEntity::Instance(META_IFACEPTR(CBaseEntity));
	if (!pEnt)
	{
		RETURN_META(MRES_IGNORED);
	}

	int index = pEnt->entindex();
	pEnt->m_bInThink = true;
	pEnt->Think();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInThink = false;
}


BASEPTR	CEntity::ThinkSet(BASEPTR func, float thinkTime, const char *szContext)
{
	if ( !szContext )
	{
		m_pfnThink = func;
		return m_pfnThink;
	}

	return NULL;
}

void CEntity::SetNextThink(float thinkTime, const char *szContext)
{
	g_helpfunc.SetNextThink(m_pEntity, thinkTime, szContext);
}

void CEntity::AddEFlags(int nEFlagMask)
{
	m_iEFlags |= nEFlagMask;

	if ( nEFlagMask & ( EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX ) )
	{
		DispatchUpdateTransmitState();
	}
}

void CEntity::RemoveEFlags(int nEFlagMask)
{
	m_iEFlags &= ~nEFlagMask;
	if ( nEFlagMask & ( EFL_FORCE_CHECK_TRANSMIT | EFL_IN_SKYBOX ) )
		DispatchUpdateTransmitState();
}

bool CEntity::IsEFlagSet(int nEFlagMask) const
{
	return (m_iEFlags & nEFlagMask) != 0;
}

void CEntity::CheckHasThinkFunction(bool isThinking)
{
	if ( IsEFlagSet( EFL_NO_THINK_FUNCTION ) && isThinking )
	{
		RemoveEFlags( EFL_NO_THINK_FUNCTION );
	}
	else if ( !isThinking && !IsEFlagSet( EFL_NO_THINK_FUNCTION ) && !WillThink() )
	{
		AddEFlags( EFL_NO_THINK_FUNCTION );
	}

	g_helpfunc.SimThink_EntityChanged(m_pEntity);
}

bool CEntity::WillThink()
{
	if (m_nNextThinkTick > 0)
		return true;

	return false;
}

const char* CEntity::GetClassname()
{
	return STRING(m_iClassname);
}

void CEntity::SetClassname(const char *pClassName)
{
	m_iClassname = MAKE_STRING(pClassName);
}

const char* CEntity::GetTargetName()
{
	return STRING(m_iName);
}

void CEntity::SetTargetName(const char *pTargetName)
{
	m_iName = MAKE_STRING(pTargetName);
}

void CEntity::ChangeTeam(int iTeamNum)
{
	m_iTeamNum = iTeamNum;
}

int CEntity::GetTeamNumber(void) const
{
	return m_iTeamNum;
}

bool CEntity::InSameTeam(CEntity *pEntity) const
{
	if (!pEntity)
		return false;

	return (pEntity->GetTeamNumber() == GetTeamNumber());
}

const Vector& CEntity::GetLocalOrigin(void) const
{
	return m_vecOrigin;
}

const Vector& CEntity::GetAbsOrigin(void) const
{
	return m_vecAbsOrigin;
}

const Vector &CEntity::GetAbsVelocity() const
{
	if (IsEFlagSet(EFL_DIRTY_ABSVELOCITY))
	{
		//const_cast<CEntity*>(this)->CalcAbsoluteVelocity();
	}
	return m_vecAbsVelocity;
}

const Vector & CEntity::GetVelocity() const
{
	return m_vecVelocity;
}

CEntity *CEntity::GetMoveParent(void)
{
	return Instance(m_hMoveParent); 
}

CEntity *CEntity::GetRootMoveParent()
{
	CEntity *pEntity = this;
	CEntity *pParent = m_hMoveParent;
	while ( pParent )
	{
		pEntity = pParent;
		pParent = pEntity->GetMoveParent();
	}

	return pEntity;
}

edict_t *CEntity::edict()
{
	return m_pEdict;
}

int CEntity::entindex()
{
	return BaseEntity()->GetRefEHandle().GetEntryIndex();
}

bool CEntity::IsPlayer()
{
	return false;
}

int CEntity::GetTeam()
{
	return m_iTeamNum;
}

bool CEntity::AcceptInput(const char *szInputName, CEntity *pActivator, CEntity *pCaller, variant_t Value, int outputID)
{
	if (!m_bInAcceptInput)
	{
		return SH_MCALL(BaseEntity(), AcceptInput)(szInputName, *pActivator, *pCaller, Value, outputID);
	}

	/**
	 * This gets the award for the worst hack so far. Detects the end of waiting for players and probably lots of other things.
	 * Forces players out of vehicles.
	 */
	/*if (strcmp(szInputName, "ShowInHUD") == 0 || strcmp(szInputName, "RoundSpawn") == 0 || strcmp(szInputName, "RoundWin") == 0)
	{
		CEntity *pEnt;
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			pEnt = CEntity::Instance(i);
			if (!pEnt)
			{
				continue;
			}

			CPlayer *pPlayer = dynamic_cast<CPlayer *>(pEnt);
			assert(pPlayer);

			IServerVehicle *pVehicle = pPlayer->GetVehicle();
			if (pVehicle && !pVehicle->IsPassengerExiting())
			{
				pPlayer->LeaveVehicle();
			}
		}
	}*/

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	bool ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPAcceptInput(thisptr)))(szInputName, *pActivator, *pCaller, Value, outputID);
	SET_META_RESULT(MRES_SUPERCEDE);
	return ret;
}

bool CEntity::InternalAcceptInput(const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID)
{
	SET_META_RESULT(MRES_SUPERCEDE);

	CEntity *pEnt = *META_IFACEPTR(CBaseEntity);
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, false);
	}

	int index = pEnt->entindex();
	pEnt->m_bInAcceptInput = true;
	bool ret = pEnt->AcceptInput(szInputName, *pActivator, *pCaller, Value, outputID);
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInAcceptInput = false;

	return ret;
}

void CEntity::InitHooks()
{
	IHookTracker *pTracker = IHookTracker::m_Head;
	while (pTracker)
	{
		pTracker->AddHook(this);
		pTracker = pTracker->m_Next;
	}
}

void CEntity::InitProps()
{
	IPropTracker *pTracker = IPropTracker::m_Head;
	while (pTracker)
	{
		pTracker->InitProp(this);
		pTracker = pTracker->m_Next;
	}
}

void CEntity::ClearAllFlags()
{
	IHookTracker *pTracker = IHookTracker::m_Head;
	while (pTracker)
	{
		pTracker->ClearFlag(this);
		pTracker = pTracker->m_Next;
	}
}

CEntity *CEntity::GetOwnerEntity()
{
	return m_hOwnerEntity;
}

void CEntity::SetOwner(CEntity *pOwnerEntity)
{
	(*m_hOwnerEntity.ptr).Set(pOwnerEntity->edict()->GetIServerEntity());
}

void CEntity::SetCollisionGroup(int collisionGroup)
{
	if ((int)m_CollisionGroup != collisionGroup)
	{
		m_CollisionGroup = collisionGroup;
		CollisionRulesChanged();
	}
}

#define VPHYSICS_MAX_OBJECT_LIST_COUNT	1024
void CEntity::CollisionRulesChanged()
{
	// ivp maintains state based on recent return values from the collision filter, so anything
	// that can change the state that a collision filter will return (like m_Solid) needs to call RecheckCollisionFilter.
	if (VPhysicsGetObject())
	{
		if (PhysIsInCallback())
		{
			Warning("Changing collision rules within a callback is likely to cause crashes!\n");
			Assert(0);
		}
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
		for (int i = 0; i < count; i++)
		{
			if (pList[i] != NULL) //this really shouldn't happen, but it does >_<
				pList[i]->RecheckCollisionFilter();
		}
	}
}

int CEntity::GetMoveType() const
{
	return *m_MoveType;
}

void CEntity::SetMoveType( int MoveType )
{
	*m_MoveType = MoveType;
}

int CEntity::GetMoveCollide() const
{
	return *m_MoveCollide;
}

void CEntity::SetMoveCollide( int MoveCollide )
{
	*m_MoveCollide = MoveCollide;
}

SolidType_t CEntity::GetSolid() const
{
	return (SolidType_t)*m_nSolidType;
}

void CEntity::SetSolid(SolidType_t val)
{
	CollisionProp()->SetSolid(val);
}

void CEntity::SetSolidFlags(int flags)
{
	CollisionProp()->SetSolidFlags(flags);
}

void CEntity::AddSolidFlags(int nFlags)
{
	CollisionProp()->AddSolidFlags(nFlags);
}

void CEntity::RemoveSolidFlags(int nFlags)
{
	CollisionProp()->RemoveSolidFlags(nFlags);
}

bool CEntity::IsSolid() const
{
	return CollisionProp()->IsSolid( );
}

void CEntity::SetViewOffset(const Vector& v) 
{ 
	m_vecViewOffset = v; 
}

void CEntity::AddEffects(int nEffects)
{
	m_fEffects |= nEffects;

	if(nEffects & ( EF_NOINTERP ) )
	{
		g_helpfunc.AddPostClientMessageEntity(m_pEntity);
	}

	if(nEffects & EF_NODRAW)
	{
		DispatchUpdateTransmitState();
	}
}

void CEntity::RemoveEffects(int nEffects)
{
	m_fEffects &= ~nEffects;
	if(nEffects & EF_NODRAW)
	{
		NetworkProp()->MarkPVSInformationDirty();
		DispatchUpdateTransmitState();
	}
}

#define CHANGE_FLAGS(flags,newFlags) { unsigned int old = flags; flags = (newFlags); g_helpfunc.ReportEntityFlagsChanged( m_pEntity, old, flags ); }

void CEntity::AddFlag(int flags)
{
	CHANGE_FLAGS( m_fFlags, m_fFlags | flags );
}

void CEntity::RemoveFlag(int flagsToRemove)
{
	CHANGE_FLAGS( m_fFlags, m_fFlags & ~flagsToRemove );
}

void CEntity::ToggleFlag(int flagToToggle)
{
	CHANGE_FLAGS( m_fFlags, m_fFlags ^ flagToToggle );
}

void CEntity::ClearFlags()
{
	CHANGE_FLAGS( m_fFlags, 0 );
}

int CEntity::DispatchUpdateTransmitState()
{
	return g_helpfunc.DispatchUpdateTransmitState(m_pEntity);
}

CServerNetworkProperty *CEntity::NetworkProp() const
{
	return m_Network;
}

model_t	*CEntity::GetModel()
{
	return (model_t *)modelinfo->GetModel( GetModelIndex() );
}

void CEntity::SetGroundEntity(CBaseEntity *ground)
{
	g_helpfunc.SetGroundEntity(m_pEntity, ground);
}

void CEntity::SetAbsVelocity(const Vector &vecAbsVelocity)
{
	g_helpfunc.SetAbsVelocity(m_pEntity, vecAbsVelocity);
}

Vector CEntity::EyePosition(void)
{ 
	return GetAbsOrigin() + m_vecViewOffset; 
}

CAI_NPC	*CEntity::MyNPCPointer()
{
	if(!IsNPC()) 
		return NULL;

	CAI_NPC *ce = dynamic_cast<CAI_NPC *>(this);
	return ce;
}

CEntity *CEntity::GetGroundEntity()
{
	return m_hGroundEntity;
}

const QAngle& CEntity::GetAbsAngles() const
{
	/*Assert( CBaseEntity::IsAbsQueriesValid() );

	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		const_cast<CBaseEntity*>(this)->CalcAbsolutePosition();
	}*/
	return m_angAbsRotation;
}

matrix3x4_t &CEntity::EntityToWorldTransform() 
{ 
	/*Assert( CBaseEntity::IsAbsQueriesValid() );

	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		CalcAbsolutePosition();
	}*/
	return m_rgflCoordinateFrame; 
}

const matrix3x4_t &CEntity::EntityToWorldTransform() const
{ 
	/*Assert( CBaseEntity::IsAbsQueriesValid() );

	if (IsEFlagSet(EFL_DIRTY_ABSTRANSFORM))
	{
		const_cast<CBaseEntity*>(this)->CalcAbsolutePosition();
	}*/
	return m_rgflCoordinateFrame; 
}


void CEntity::SetAbsAngles(const QAngle& absAngles)
{
	g_helpfunc.SetAbsAngles(BaseEntity(), absAngles);
}

void CEntity::GetVectors(Vector* pForward, Vector* pRight, Vector* pUp) const
{
	const matrix3x4_t &entityToWorld = EntityToWorldTransform();

	if (pForward != NULL)
	{
		MatrixGetColumn( entityToWorld, 0, *pForward ); 
	}

	if (pRight != NULL)
	{
		MatrixGetColumn( entityToWorld, 1, *pRight ); 
		*pRight *= -1.0f;
	}

	if (pUp != NULL)
	{
		MatrixGetColumn( entityToWorld, 2, *pUp ); 
	}
}

void CEntity::ApplyAbsVelocityImpulse( const Vector &vecImpulse )
{
	if (vecImpulse != vec3_origin )
	{
		if ( GetMoveType() == MOVETYPE_VPHYSICS )
		{
			VPhysicsGetObject()->AddVelocity( &vecImpulse, NULL );
		}
		else
		{
			// NOTE: Have to use GetAbsVelocity here to ensure it's the correct value
			Vector vecResult;
			VectorAdd( GetAbsVelocity(), vecImpulse, vecResult );
			SetAbsVelocity( vecResult );
		}
	}
}


HSOUNDSCRIPTHANDLE CEntity::PrecacheScriptSound( const char *soundname )
{
	return g_helpfunc.PrecacheScriptSound(soundname);
}

void CEntity::EmitSound( const char *soundname, float soundtime, float *duration)
{
	return g_helpfunc.EmitSound(BaseEntity(), soundname, soundtime, duration);
}

bool CEntity::IsBSPModel() const
{
	if ( GetSolid() == SOLID_BSP )
		return true;
	
	const model_t *model = modelinfo->GetModel( GetModelIndex() );

	if ( GetSolid() == SOLID_VPHYSICS && modelinfo->GetModelType( model ) == mod_brush )
		return true;

	return false;
}
