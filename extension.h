#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include "datamap.h"
#include "smsdk_ext.h"
#include "sh_list.h"

class CBasePlayer;

#include <eiface.h>
#include <igameevents.h>
#include <server_class.h>
#include <isoundemittersystembase.h>
#include <IEngineTrace.h>
#include <IEngineSound.h>
#include <IStaticPropMgr.h>
#include <IEffects.h>
#include <filesystem.h>
#include <inetchannelinfo.h>
#include <vphysics_interface.h>
#include <igamemovement.h>
#include <decals.h>
#include <random.h>
#include <worldsize.h>
#include <iplayerinfo.h>
#include <itempents.h>
#include <shareddefs.h>
#include <mathlib.h>
#include <itoolentity.h>

#include "ehandle.h"
class CBaseEntity;
typedef CHandle<CBaseEntity> EHANDLE;

class CBasePlayer;

//#define GAME_DLL
//#include "cbase.h"
//#include "takedamageinfo.h"
//#include "networkvar.h"
//#include <typeinfo>
//#include <variant_t.h>

//#include "macros.h"
//#include "CEntityManager.h"

//#include "CEntityManager.h"
//#include "CEntity.h"

#include "sign_func.h"


class Monster :
	public SDKExtension,
	public IConCommandBaseAccessor
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();
	virtual void SDK_OnAllLoaded();
	virtual bool QueryRunning(char *error, size_t maxlength);

	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late);
	virtual bool SDK_OnMetamodUnload(char *error, size_t maxlength);

public:
	bool RegisterConCommandBase(ConCommandBase *pCommand);
	void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax);
	bool LevelInit(char const *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background);
	void OnLevelShutdown();
	void SetCommandClient( int cmd );	
	int GetCommandClient();
	int GetMaxClients();
};

class CEntity;

extern Monster g_Monster;

extern CGlobalVars *gpGlobals;
extern INetworkStringTableContainer *netstringtables;
extern IEngineSound *engsound;
extern IEngineTrace *enginetrace;
extern IServerGameClients *gameclients;
extern IUniformRandomStream *random;
extern IStaticPropMgrServer *staticpropmgr;
extern IVModelInfo *modelinfo;
extern ISpatialPartition *partition;
extern IPhysicsSurfaceProps *physprops;
extern IPhysicsCollision *physcollision;
extern IPhysicsEnvironment *physenv;
extern IPhysics *iphysics;
extern ISoundEmitterSystemBase *soundemitterbase;
extern IFileSystem *filesystem;
extern IDecalEmitterSystem *decalsystem;
extern IEngineSound *enginesound;
extern ITempEntsSystem *te;
extern IPhysicsObjectPairHash *my_g_EntityCollisionHash;
extern IGameMovement *g_pGameMovement;
extern IServerTools *servertools;

extern IGameConfig *g_pGameConf;
extern CBaseEntityList *g_pEntityList;
extern unsigned long serverdll_addr;
extern CEntity *my_g_WorldEntity;
extern CBaseEntity *my_g_WorldEntity_cbase;

#endif

