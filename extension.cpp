
#include "extension.h"
#include "command.h"
#include "CAI_NetworkManager.h"

Monster g_Monster;

SMEXT_LINK(&g_Monster);

CGlobalVars *gpGlobals = NULL;
INetworkStringTableContainer *netstringtables = NULL;
IEngineSound *engsound = NULL;
IEngineTrace *enginetrace = NULL;
IServerGameClients *gameclients = NULL;
ICvar *icvar = NULL;
IUniformRandomStream *random = NULL;
IStaticPropMgrServer *staticpropmgr = NULL;
IVModelInfo *modelinfo = NULL;
IPhysicsObjectPairHash *myg_EntityCollisionHash = NULL;
ISpatialPartition *partition = NULL;
IPhysicsSurfaceProps *physprops = NULL;

IGameConfig *g_pGameConf = NULL;

CBaseEntityList *g_pEntityList = NULL;

CEntity *g_WorldEntity = NULL;

int gCmdIndex;
int gMaxClients;

unsigned long serverdll_addr;

SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, 0, int);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);

bool Monster::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("monster.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			g_pSM->Format(error, maxlength, "Could not read tfdodgeball.games: %s", conf_error);
		}
		return false;
	}

	if (!GetEntityManager()->Init(g_pGameConf))
		g_pSM->LogError(myself, "CEntity failed to init.");

	g_pEntityList = (CBaseEntityList *)gamehelpers->GetGlobalEntityList();
	
	void *__laddr = reinterpret_cast<void *>(g_SMAPI->GetServerFactory(false));
	MEMORY_BASIC_INFORMATION mem;
	if(!VirtualQuery(__laddr, &mem, sizeof(MEMORY_BASIC_INFORMATION))) {
		return false;
	}
	if(mem.AllocationBase == NULL) {
		return false;
	}
	HMODULE dll = (HMODULE)mem.AllocationBase;
	serverdll_addr = (unsigned long)dll;


	if(!g_helpfunc.Initialize())
	{
		g_pSM->LogError(myself, "Helper failed to init.");
	}

	CommandInitialize();


	//0EB829DE 


	META_CONPRINTF("%p\n",reinterpret_cast<void *>(0x00031380+serverdll_addr));


	return true;
}

void Monster::SDK_OnUnload()
{
	
}

void Monster::SDK_OnAllLoaded()
{
}

bool Monster::QueryRunning(char *error, size_t maxlength)
{
	return true;
}

bool Monster::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	gpGlobals = ismm->GetCGlobals();

	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	GET_V_IFACE_CURRENT(GetEngineFactory, netstringtables, INetworkStringTableContainer, INTERFACENAME_NETWORKSTRINGTABLESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, engsound, IEngineSound, IENGINESOUND_SERVER_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, enginetrace, IEngineTrace, INTERFACEVERSION_ENGINETRACE_SERVER);
	GET_V_IFACE_CURRENT(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, random, IUniformRandomStream, VENGINE_SERVER_RANDOM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, staticpropmgr, IStaticPropMgrServer, INTERFACEVERSION_STATICPROPMGR_SERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, modelinfo, IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, partition, ISpatialPartition, INTERFACEVERSION_SPATIALPARTITION);
	GET_V_IFACE_CURRENT(GetPhysicsFactory, physprops, IPhysicsSurfaceProps,  VPHYSICS_SURFACEPROPS_INTERFACE_VERSION);


	g_pCVar = icvar;
	
	ConVar_Register(0, this);

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, gamedll, this, &Monster::ServerActivate, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverclients, this, &Monster::SetCommandClient, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &Monster::OnLevelShutdown, false);

	return true;
}

bool Monster::RegisterConCommandBase(ConCommandBase *pCommand)
{
	META_REGCVAR(pCommand);

	return true;
}

bool Monster::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, gamedll, this, &Monster::ServerActivate, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverclients, this, &Monster::SetCommandClient, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &Monster::OnLevelShutdown, false);
	return true;
}

void Monster::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	gMaxClients = clientMax;

	g_helpfunc.HookGameRules();
	g_helpfunc.GetEntityCollisionHash();
	g_helpfunc.GetWorldEntity();

	CEAI_NetworkManager::InitializeAINetworks();

	engine->PrecacheModel("models/headcrabclassic.mdl",true);

	engine->PrecacheModel("models/roller_vehicledriver.mdl",true);


	RETURN_META(MRES_IGNORED);
}

void Monster::OnLevelShutdown()
{
	g_helpfunc.UnHookGameRules();
}

void Monster::SetCommandClient( int cmd )
{
	gCmdIndex = cmd + 1;
}

int Monster::GetCommandClient()
{
	return gCmdIndex;
}

int Monster::GetMaxClients()
{
	return gMaxClients;
}


