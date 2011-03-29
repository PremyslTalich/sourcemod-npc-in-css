#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include "smsdk_ext.h"

#include <eiface.h>
#include <igameevents.h>
#include <server_class.h>
#include <isoundemittersystembase.h>
#include <IEngineTrace.h>
#include <IEngineSound.h>
#include <IStaticPropMgr.h>
#include <filesystem.h>
#include <inetchannelinfo.h>
#include <vphysics_interface.h>
#include <random.h>
#include <worldsize.h>
#include <iplayerinfo.h>
#include <shareddefs.h>
#include <mathlib.h>

#include "ehandle.h"
class CBaseEntity;
typedef CHandle<CBaseEntity> EHANDLE;


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
#include "entitydata.h"


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

extern IPhysicsObjectPairHash *myg_EntityCollisionHash;

extern IGameConfig *g_pGameConf;
extern CBaseEntityList *g_pEntityList;
extern unsigned long serverdll_addr;
extern CEntity *g_WorldEntity;

#endif

