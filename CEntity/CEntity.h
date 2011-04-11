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

/**
* CEntity Entity Handling Framework version 2.0 by Matt "pRED*" Woodrow
*
* - Credits:
*		- This is largely (or entirely) based on a concept by voogru - http://voogru.com/
*		- The virtual function hooking is powered by the SourceHook library by Pavol "PM OnoTo" Marko.
*		- Contains code contributed by Brett "Brutal" Powell.
*		- Contains code contributed by Asher "asherkin" Baker.
*
* - About:
*		- CEntity is (and its derived classes are) designed to emulate the CBaseEntity class from the HL2 SDK.
*		- Valve code (like entire class definitions and CBaseEntity functions) from the SDK should almost just work when copied into this.
*			- References to CBaseEntity need to be changed to CEntity.
*			- Sendprops and datamaps are pointers to the actual values so references to these need to be dereferenced.
*				- Code that uses unexposed data members won't work - Though you could reimplement these manually.
*		- Virtual functions handle identically to ones in a real derived class.
*			- Calls from valve code to a virtual in CEntity (with no derived versions) fall back directly to the valve code.
*			- Calls from valve code to a virtual (with a derived version) will call that code, and the valve code can be optionally run using BaseClass::Function().
*
*			- Calls from your code to a virtual in CEntity (with no derived versions) will make a call to the valve code.
*			- Calls from your code to a virtual (with a derived version) will call that code, and that derived handler can run the valve code optionally using BaseClass::Function().
*			
*
* - Notes:
*		- If you inherit Init() or Destroy() in a derived class, I would highly recommend calling the BaseClass equivalent.
* 
* - TODO (in no particular order):
*		- Add handling of custom keyvalues commands
*			- Add datamapping to class values so keyvalues can parse to them
*		- Include more CEntity virtuals and props/datamaps
*		- Create more derived classes
*		- Include more Think/Touch etc handlers
*			- Valve code now has lists of thinks, can we access this?
*		- Forcibly deleting entities? - Implemented AcceptInput("Kill"...), UTIL_Remove sig scan would be cleaner.
*		- Support mods other than TF2 (CPlayer should only contain CBasePlayer sdk stuff and create optional CTFPlayer/CCSPlayer derives)
*
*	- Change log
*		- 1.0
*			- Initial import of basic CEntity and CPlayer
*		- 1.x
*			- Unlogged fixes/changes added after original version. TODO: Grab these from the hg changelog sometime.
*		- 2.0
*			- Improved CE_LINK_ENTITY_TO_CLASS to use DLL Classnames. tf_projectile_rocket changed to CTFProjectile_Rocket for example.
*			- Added the ability to handle entity Inputs/Outputs.
*			- Cleaned up Macros used for almost everything.
*			- Added many new hooks and props for CEntity and CPlayer.
*			- Support for custom classnames with LINK_ENTITY_TO_CUSTOM_CLASS.
*			- Added support for detours, needs CDetours folder in the parent directory.
*			- Added a helpers class that makes common functions easy (from CrimsonGT).
*			- CScriptCreatedItem and CScriptCreatedAttribute (from TF2Items).
*			- A new 'tracker', designed to get a generic pointer from a sig in a gamedata file.
*			- A lot of CPlayer defines, including PLAYERCONDs, WEAPONSLOTs and LOADOUTSLOTs.
*			- Added CAnimating with StudioFrameAdvance and Dissolve.
*			- Changed CPlayer to inherit from CAnimating.
*/

#ifndef _INCLUDE_CENTITY_H_
#define _INCLUDE_CENTITY_H_

#define NO_STRING_T

//#define GAME_DLL


#include "extension.h"
#include "CE_Define.h"
#include "CEntityBase.h"
#include "CECollisionProperty.h"
#include "CTakeDamageInfo.h"
#include "IEntityFactory.h"

#include "vector.h"
#include "server_class.h"

#include "macros.h"

#include "cutil.h"

//#include "CTakeDamageInfo.h"

#include <typeinfo>
#include <variant_t.h>
#include "EntityOutput.h"

#include "takedamageinfo.h"
#include "collisionproperty.h"
#include "ServerNetworkProperty.h"
#include "decals.h"
#include "vehicles.h"


extern variant_t g_Variant;


// For CLASSIFY
enum Class_T
{
	CLASS_NONE=0,				
	CLASS_PLAYER,			
	CLASS_PLAYER_ALLY,
	CLASS_PLAYER_ALLY_VITAL,
	CLASS_ANTLION,
	CLASS_BARNACLE,
	CLASS_BULLSEYE,
	//CLASS_BULLSQUID,	
	CLASS_CITIZEN_PASSIVE,	
	CLASS_CITIZEN_REBEL,
	CLASS_COMBINE,
	CLASS_COMBINE_GUNSHIP,
	CLASS_CONSCRIPT,
	CLASS_HEADCRAB,
	//CLASS_HOUNDEYE,
	CLASS_MANHACK,
	CLASS_METROPOLICE,		
	CLASS_MILITARY,		
	CLASS_SCANNER,		
	CLASS_STALKER,		
	CLASS_VORTIGAUNT,
	CLASS_ZOMBIE,
	CLASS_PROTOSNIPER,
	CLASS_MISSILE,
	CLASS_FLARE,
	CLASS_EARTH_FAUNA,
	CLASS_HACKED_ROLLERMINE,
	CLASS_COMBINE_HUNTER,

	NUM_AI_CLASSES
};



class CEntity;
class CEntityTakeDamageInfo;
class CECollisionProperty;
class CAI_NPC;


#undef DEFINE_INPUTFUNC
#define DEFINE_INPUTFUNC( fieldtype, inputname, inputfunc ) { fieldtype, #inputfunc, { 0, 0 }, 1, FTYPEDESC_INPUT, inputname, NULL, (inputfunc_t)(&classNameTypedef::inputfunc) }


extern CEntity *pEntityData[MAX_EDICTS+1];

extern ConVar *sv_gravity;
extern ConVar *phys_pushscale;


typedef void (CEntity::*BASEPTR)(void);
typedef void (CEntity::*ENTITYFUNCPTR)(CEntity *pOther);
typedef void (CEntity::*USEPTR)(CEntity *pActivator, CEntity *pCaller, USE_TYPE useType, float value);

#define DEFINE_THINKFUNC( function ) DEFINE_FUNCTION_RAW( function, BASEPTR )
#define DEFINE_ENTITYFUNC( function ) DEFINE_FUNCTION_RAW( function, ENTITYFUNCPTR )
#define DEFINE_USEFUNC( function ) DEFINE_FUNCTION_RAW( function, USEPTR )


//template< class T >
class CFakeHandle;

#define DECLARE_DEFAULTHEADER(name, ret, params) \
	ret Internal##name params; \
	bool m_bIn##name;

#define DECLARE_DEFAULTHEADER_DETOUR(name, ret, params) \
	ret Internal##name params; \
	static ret (ThisClass::* name##_Actual) params;

#define SetThink(a) ThinkSet(static_cast <void (CEntity::*)(void)> (a), 0, NULL)
#define SetContextThink( a, b, context ) ThinkSet( static_cast <void (CEntity::*)(void)> (a), (b), context )

#define SetTouch( a ) m_pfnTouch = static_cast <void (CEntity::*)(CEntity *)> (a)

class CCombatCharacter;

class CEntity // : public CBaseEntity  - almost.
{
public: // CEntity
	CE_DECLARE_CLASS_NOBASE(CEntity);
	DECLARE_DATADESC();
	DECLARE_DEFAULTHEADER(GetDataDescMap, datamap_t *, ());

	virtual void Init(edict_t *pEdict, CBaseEntity *pBaseEntity);
	void InitHooks();
	void InitProps();
	void ClearAllFlags();
	virtual void Destroy();
	CBaseEntity *BaseEntity();
	CBaseEntity *BaseEntity() const;

	operator CBaseEntity* ()
	{
		if (this == NULL)
		{
			return NULL;
		}

		return BaseEntity();
	}
	CEntity *operator=(CBaseEntity *rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CEntity *operator=(CBaseHandle &rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CEntity *operator=(CBaseHandle rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CEntity *operator=(Redirect<CBaseHandle> &rhs)
	{
		return CEntityLookup::Instance(rhs);
	}
	CBaseEntity *operator=(CEntity *const pEnt)
	{
		return pEnt->BaseEntity();
	}

	/* Bcompat and it's just easier to refer to these as CEntity:: */
	static CEntity *Instance(const CBaseHandle &hEnt) { return CEntityLookup::Instance(hEnt); }
	static CEntity *Instance(const edict_t *pEnt)  { return CEntityLookup::Instance(pEnt); }
	static CEntity *Instance(edict_t *pEnt)  { return CEntityLookup::Instance(pEnt); }
	static CEntity* Instance(int iEnt)  { return CEntityLookup::Instance(iEnt); }
	static CEntity* Instance(CBaseEntity *pEnt)  { return CEntityLookup::Instance(pEnt); }

	static HSOUNDSCRIPTHANDLE PrecacheScriptSound( const char *soundname );

public: // CBaseEntity virtuals
	virtual void Teleport(const Vector *origin, const QAngle* angles, const Vector *velocity);
	virtual void UpdateOnRemove();
	virtual void Spawn();
	virtual int OnTakeDamage(CEntityTakeDamageInfo &info);
	virtual void Think();
	virtual bool AcceptInput(const char *szInputName, CEntity *pActivator, CEntity *pCaller, variant_t Value, int outputID);
	virtual void StartTouch(CEntity *pOther);
	virtual void Touch(CEntity *pOther); 
	virtual void EndTouch(CEntity *pOther);
	virtual Vector GetSoundEmissionOrigin();
	virtual int VPhysicsTakeDamage(const CEntityTakeDamageInfo &inputInfo);
	virtual int	VPhysicsGetObjectList(IPhysicsObject **pList, int listMax);
	virtual ServerClass *GetServerClass();
	virtual void SetModel(char *model);
	virtual Class_T Classify();
	virtual bool ShouldCollide(int collisionGroup, int contentsMask);
	virtual int ObjectCaps();
	virtual bool IsNPC();
	virtual void TakeDamage( const CTakeDamageInfo &inputInfo );
	virtual void SetParent(CBaseEntity* pNewParent, int iAttachment = -1);
	virtual void DecalTrace( trace_t *pTrace, char const *decalName );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual Vector BodyTarget( const Vector &posSrc, bool bNoisy = true);
	virtual IServerVehicle *GetServerVehicle();
	virtual bool IsAlive();
	virtual const Vector &WorldSpaceCenter();
	virtual void PhysicsSimulate();


public:
	void SetLocalOrigin(const Vector& origin);
	void PhysicsTouchTriggers(const Vector *pPrevAbsOrigin = NULL);
	void SetAbsOrigin(const Vector& absOrigin);
	void SetLocalAngles( const QAngle& angles );

public:
	virtual CCombatCharacter *MyCombatCharacterPointer( void ) { return NULL; }

public: // CBaseEntity non virtual helpers
	BASEPTR	ThinkSet(BASEPTR func, float thinkTime, const char *szContext);
	void SetNextThink(float thinkTime, const char *szContext = NULL);
	void CheckHasThinkFunction(bool isThinking);
	bool WillThink();

	void AddEFlags(int nEFlagMask);
	void RemoveEFlags(int nEFlagMask);
	bool IsEFlagSet(int nEFlagMask) const;

	const char* GetClassname();
	void SetClassname(const char *pClassName);
	const char* GetTargetName();
	void SetTargetName(const char *pTargetName);
	CEntity *GetOwnerEntity();
	void SetOwner(CEntity *pOwnerEntity);

	int GetTeamNumber()  const;
	virtual void ChangeTeam(int iTeamNum);
	bool InSameTeam(CEntity *pEntity) const;

	int GetMoveType() const;
	void SetMoveType(int MoveType);
	int GetMoveCollide() const;
	void SetMoveCollide(int MoveCollide);

	const Vector &GetAbsOrigin() const;
	const Vector &GetLocalOrigin() const;
	const Vector &GetAbsVelocity() const;
	const Vector &GetVelocity() const;

	CEntity *GetMoveParent();
	CEntity *GetRootMoveParent();

	edict_t *edict();
	int entindex();

	inline IPhysicsObject *VPhysicsGetObject(void) const { return m_pPhysicsObject; }
	inline CECollisionProperty *CollisionProp(void) const { return col_ptr; }
	inline CCollisionProperty *CollisionProp_Actual(void) const { return m_Collision; }
	inline ICollideable *GetCollideable() const { return m_Collision; }

	void SetCollisionGroup(int collisionGroup);
	void CollisionRulesChanged();

	inline int GetWaterLevel() const { return m_nWaterLevel; }

	inline bool	IsWorld() { return entindex() == 0; }
	
	const Vector&			WorldAlignMins( ) const;
	const Vector&			WorldAlignMaxs( ) const;
	const Vector&			WorldAlignSize( ) const;

	float BoundingRadius() const;

	virtual	bool IsPlayer();

	int GetTeam();

	const color32 GetRenderColor() const;
	void SetRenderColor( byte r, byte g, byte b );
	void SetRenderColor( byte r, byte g, byte b, byte a );
	void SetRenderColorR( byte r );
	void SetRenderColorG( byte g );
	void SetRenderColorB( byte b );
	void SetRenderColorA( byte a );

	int GetSpawnFlags() const;
	void AddSpawnFlags(int nFlags);
	void RemoveSpawnFlags(int nFlags) ;
	void ClearSpawnFlags();
	bool HasSpawnFlags( int nFlags ) const;

	void GetVectors(Vector* pForward, Vector* pRight, Vector* pUp) const;

	matrix3x4_t &EntityToWorldTransform() ;
	const matrix3x4_t &EntityToWorldTransform() const;

	void SetGravity(float flGravity);
	float GetGravity() const;

	int		GetMaxHealth()  const	{ return m_iMaxHealth; }
	void	SetMaxHealth( int amt )	{ m_iMaxHealth = amt; }

	int		GetHealth() const		{ return m_iHealth; }
	void	SetHealth( int amt )	{ m_iHealth = amt; }

	void ApplyAbsVelocityImpulse( const Vector &vecImpulse );

	void EmitSound( const char *soundname, float soundtime = 0.0f, float *duration = NULL );

public: //custom
	CECollisionProperty *col_ptr;

public: // custom
	SolidType_t GetSolid() const;
	void SetSolid(SolidType_t val);
	void SetSolidFlags(int flags);
	void AddSolidFlags(int nFlags );
	void RemoveSolidFlags(int nFlags);
	bool IsSolid() const;

	void SetViewOffset(const Vector& v);
	
	void AddEffects(int nEffects);
	void RemoveEffects(int nEffects);

	void AddFlag(int flags);
	void RemoveFlag(int flagsToRemove);
	void ToggleFlag(int flagToToggle);
	int GetFlags() const;
	void ClearFlags();
	
	int GetModelIndex( void ) const;

	bool IsTransparent() const;

	int GetCollisionGroup() const;

	IHandleEntity *GetIHandle() const;

	model_t	*GetModel();

	int DispatchUpdateTransmitState();

	const QAngle& GetLocalAngles() const;
	const QAngle& GetAbsAngles() const;
	
	const Vector& GetViewOffset() const;

	void SetGroundEntity(CBaseEntity *ground);
	void SetAbsVelocity(const Vector &vecAbsVelocity);

	void SetAbsAngles(const QAngle& absAngles);

	Vector EyePosition();

	CAI_NPC	*MyNPCPointer();

	CServerNetworkProperty *NetworkProp() const;
	
	CEntity *GetGroundEntity();

	bool IsBSPModel() const;

public: // All the internal hook implementations for the above virtuals
	DECLARE_DEFAULTHEADER(Teleport, void, (const Vector *origin, const QAngle* angles, const Vector *velocity));
	DECLARE_DEFAULTHEADER(UpdateOnRemove, void, ());
	DECLARE_DEFAULTHEADER(Spawn, void, ());
	DECLARE_DEFAULTHEADER(OnTakeDamage, int, (CEntityTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(Think, void, ());
	DECLARE_DEFAULTHEADER(AcceptInput, bool, (const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller,variant_t Value, int outputID));
	DECLARE_DEFAULTHEADER(StartTouch, void, (CBaseEntity *pOther));
	DECLARE_DEFAULTHEADER(Touch, void, (CBaseEntity *pOther));
	DECLARE_DEFAULTHEADER(EndTouch, void, (CBaseEntity *pOther));
	DECLARE_DEFAULTHEADER(GetSoundEmissionOrigin, Vector, ());
	DECLARE_DEFAULTHEADER(VPhysicsTakeDamage, int, (const CEntityTakeDamageInfo &inputInfo));
	DECLARE_DEFAULTHEADER(VPhysicsGetObjectList, int, (IPhysicsObject **pList, int listMax));
	DECLARE_DEFAULTHEADER(GetServerClass, ServerClass *, ());
	DECLARE_DEFAULTHEADER(SetModel, void, (char *model));
	DECLARE_DEFAULTHEADER(Classify, Class_T, ());
	DECLARE_DEFAULTHEADER(ShouldCollide, bool, (int collisionGroup, int contentsMask));
	DECLARE_DEFAULTHEADER(ObjectCaps, int, ());
	DECLARE_DEFAULTHEADER(IsNPC, bool, ());
	DECLARE_DEFAULTHEADER(SetParent, void, (CBaseEntity* pNewParent, int iAttachment));
	DECLARE_DEFAULTHEADER(DecalTrace, void, (trace_t *pTrace, char const *decalName));
	DECLARE_DEFAULTHEADER(Event_Killed, void, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(TraceAttack, void, (const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr));
	DECLARE_DEFAULTHEADER(BodyTarget, Vector, (const Vector &posSrc, bool bNoisy));
	DECLARE_DEFAULTHEADER(GetServerVehicle, IServerVehicle *, ());
	DECLARE_DEFAULTHEADER(IsAlive, bool, ());
	DECLARE_DEFAULTHEADER(WorldSpaceCenter, const Vector &, ());
	DECLARE_DEFAULTHEADER(PhysicsSimulate, void, ());

	
public:
	DECLARE_DEFAULTHEADER_DETOUR(SetLocalOrigin, void, (const Vector& origin));
	DECLARE_DEFAULTHEADER_DETOUR(PhysicsTouchTriggers, void, (const Vector *pPrevAbsOrigin));
	DECLARE_DEFAULTHEADER_DETOUR(TakeDamage, void, (const CTakeDamageInfo &inputInfo));
	DECLARE_DEFAULTHEADER_DETOUR(SetAbsOrigin, void, (const Vector& absOrigin));
	DECLARE_DEFAULTHEADER_DETOUR(SetLocalAngles, void, (const QAngle& angles));

protected: // CEntity	
	CBaseEntity *m_pEntity;
	edict_t *m_pEdict;

protected: //Sendprops
	DECLARE_SENDPROP(uint8_t, m_iTeamNum);
	DECLARE_SENDPROP(Vector, m_vecOrigin);
	DECLARE_SENDPROP(int, m_CollisionGroup);
	DECLARE_SENDPROP(CFakeHandle, m_hOwnerEntity);
	DECLARE_SENDPROP(unsigned short, m_usSolidFlags);
	DECLARE_SENDPROP(color32, m_clrRender);
	DECLARE_SENDPROP(unsigned char, m_nRenderMode);
	DECLARE_SENDPROP(unsigned char, m_nSolidType);
	DECLARE_SENDPROP(Vector, m_vecMins);
	DECLARE_SENDPROP(Vector, m_vecMaxs);	
	DECLARE_SENDPROP(short, m_nModelIndex);
	DECLARE_SENDPROP(QAngle, m_angRotation);

	
protected: //Datamaps
	DECLARE_DATAMAP(Vector, m_vecAbsOrigin);
	DECLARE_DATAMAP(Vector, m_vecAbsVelocity);
	DECLARE_DATAMAP(string_t, m_iClassname);
	DECLARE_DATAMAP(matrix3x4_t, m_rgflCoordinateFrame);
	DECLARE_DATAMAP(Vector, m_vecVelocity);
	DECLARE_DATAMAP(Vector, m_vecAngVelocity);
	DECLARE_DATAMAP(Vector, m_vecBaseVelocity);
	DECLARE_DATAMAP(CFakeHandle, m_hMoveParent);
	DECLARE_DATAMAP(int, m_iEFlags);
	DECLARE_DATAMAP(IPhysicsObject *, m_pPhysicsObject);
	DECLARE_DATAMAP(int, m_nNextThinkTick);
	DECLARE_DATAMAP(CFakeHandle, m_pParent);
	DECLARE_DATAMAP(int, m_MoveType);
	DECLARE_DATAMAP(int, m_MoveCollide);
	DECLARE_DATAMAP(string_t, m_iName);
	
	DECLARE_DATAMAP(CCollisionProperty, m_Collision);
	DECLARE_DATAMAP(Vector, m_vecViewOffset);
	DECLARE_DATAMAP(int, m_spawnflags);
	DECLARE_DATAMAP(int, m_iHealth);
	DECLARE_DATAMAP(int, m_fFlags);
	DECLARE_DATAMAP(CServerNetworkProperty , m_Network);
	DECLARE_DATAMAP(char, m_lifeState);
	DECLARE_DATAMAP(unsigned char, m_nWaterLevel);
	DECLARE_DATAMAP(CFakeHandle, m_hGroundEntity);
	DECLARE_DATAMAP(QAngle, m_angAbsRotation);
	DECLARE_DATAMAP(float, m_flGravity);
	DECLARE_DATAMAP(int, m_iMaxHealth);
	DECLARE_DATAMAP(int, m_fEffects);

public:
	DECLARE_DATAMAP(char, m_takedamage);
	DECLARE_DATAMAP(float , m_flAnimTime);
	DECLARE_DATAMAP(float , m_flPrevAnimTime);


	/* Thinking Stuff */
	void (CEntity::*m_pfnThink)(void);
	void (CEntity::*m_pfnTouch)(CEntity *pOther);

	friend class CECollisionProperty;
};

/**
 * Fake definition of CBaseEntity, as long as we don't declare any data member we should be fine with this.
 * Also gives us access to IServerEntity and below without explicit casting.
 */
class CBaseEntity : public IServerEntity
{
public:
	CBaseEntity* operator= (CEntity* const input)
	{
		return input->BaseEntity();
	}
	operator CEntity* ()
	{
		return CEntityLookup::Instance(this);
	}
};

inline const color32 CEntity::GetRenderColor() const
{
	return m_clrRender;
}

inline void CEntity::SetRenderColor( byte r, byte g, byte b )
{
	color32 clr = { r, g, b, m_clrRender->a };
	m_clrRender = clr;
}

inline void CEntity::SetRenderColor( byte r, byte g, byte b, byte a )
{
	color32 clr = { r, g, b, a };
	m_clrRender = clr;
}

inline void CEntity::SetRenderColorR( byte r )
{
	SetRenderColor( r, GetRenderColor().g, GetRenderColor().b );
}

inline void CEntity::SetRenderColorG( byte g )
{
	SetRenderColor( GetRenderColor().r, g, GetRenderColor().b );
}

inline void CEntity::SetRenderColorB( byte b )
{
	SetRenderColor( GetRenderColor().r, GetRenderColor().g, b );
}

inline void CEntity::SetRenderColorA( byte a )
{
	SetRenderColor( GetRenderColor().r, GetRenderColor().g, GetRenderColor().b, a );
}

inline int CEntity::GetFlags() const
{
	return m_fFlags;
}

inline int CEntity::GetSpawnFlags() const
{ 
	return m_spawnflags; 
}

inline void CEntity::AddSpawnFlags(int nFlags) 
{ 
	m_spawnflags |= nFlags; 
}

inline void CEntity::RemoveSpawnFlags(int nFlags) 
{ 
	m_spawnflags &= ~nFlags; 
}

inline void CEntity::ClearSpawnFlags() 
{ 
	m_spawnflags = 0; 
}

inline bool CEntity::HasSpawnFlags( int nFlags ) const
{ 
	return (m_spawnflags & nFlags) != 0; 
}

inline int CEntity::GetCollisionGroup() const
{
	return m_CollisionGroup;
}

inline int CEntity::GetModelIndex( void ) const
{
	return m_nModelIndex;
}

inline bool CEntity::IsTransparent() const
{
	return m_nRenderMode != kRenderNormal;
}

inline const QAngle& CEntity::GetLocalAngles() const
{
	return m_angRotation;
}

inline const Vector& CEntity::GetViewOffset() const 
{ 
	return m_vecViewOffset; 
}

inline const Vector& CEntity::WorldAlignMins( ) const
{
	return CollisionProp_Actual()->OBBMins();
}

inline const Vector& CEntity::WorldAlignMaxs( ) const
{
	return CollisionProp_Actual()->OBBMaxs();
}

inline const Vector& CEntity::WorldAlignSize( ) const
{
	return CollisionProp_Actual()->OBBSize();
}

inline float CEntity::BoundingRadius() const
{
	return CollisionProp_Actual()->BoundingRadius();
}

inline void CEntity::SetGravity(float flGravity) 
{ 
	m_flGravity = flGravity; 
}

inline float CEntity::GetGravity() const 
{ 
	return m_flGravity; 
}

inline CEntity *GetWorldEntity()
{
	return g_WorldEntity;
}



void UTIL_PatchOutputRestoreOps(datamap_t *pMap);

inline bool FClassnameIs( CEntity *pEntity, const char *szClassname )
{ 
	Assert( pEntity );
	if ( pEntity == NULL )
		return false;

	return !strcmp( pEntity->GetClassname(), szClassname ) ? true : false; 
}




class CPointEntity : public CEntity
{
public:
	CE_DECLARE_CLASS( CPointEntity, CEntity );

	//void	Spawn( void );
	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};


// Has a position + size
class CServerOnlyEntity : public CEntity
{
public:
	CE_DECLARE_CLASS(CServerOnlyEntity, CEntity);

public:
	virtual int ObjectCaps( void ) { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
};

/*
// Has only a position, no size
class CServerOnlyPointEntity : public CServerOnlyEntity
{
	CE_DECLARE_CLASS( CServerOnlyPointEntity, CServerOnlyEntity );

public:
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
};*/


#endif // _INCLUDE_CENTITY_H_
