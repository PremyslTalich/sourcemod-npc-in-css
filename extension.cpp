
#include "extension.h"
#include "command.h"
#include "CAI_NetworkManager.h"
#include "GameSystem.h"
#include "tier3.h"
#include "datacache/imdlcache.h"

Monster g_Monster;

bool g_bUseNetworkVars = false;

SMEXT_LINK(&g_Monster);

CGlobalVars *gpGlobals = NULL;
INetworkStringTableContainer *netstringtables = NULL;
INetworkStringTable *g_pStringTableParticleEffectNames = NULL;
IEngineSound *engsound = NULL;
IEngineTrace *enginetrace = NULL;
IServerGameClients *gameclients = NULL;
ICvar *icvar = NULL;
IUniformRandomStream *random = NULL;
IStaticPropMgrServer *staticpropmgr = NULL;
IVModelInfo *modelinfo = NULL;
IPhysicsObjectPairHash *my_g_EntityCollisionHash = NULL;
ISpatialPartition *partition = NULL;
IPhysicsSurfaceProps *physprops = NULL;
IPhysicsCollision *physcollision = NULL;
IPhysicsEnvironment *physenv;
IPhysics *iphysics = NULL;
ISoundEmitterSystemBase *soundemitterbase = NULL;
IFileSystem *filesystem = NULL;
IEffects *g_pEffects = NULL;
IDecalEmitterSystem *decalsystem = NULL;
IEngineSound *enginesound = NULL;
ITempEntsSystem *te = NULL;
CSharedEdictChangeInfo *g_pSharedChangeInfo = NULL;
IGameMovement *g_pGameMovement = NULL;
IGameConfig *g_pGameConf = NULL;
IServerTools *servertools = NULL;

CBaseEntityList *g_pEntityList = NULL;

CEntity *my_g_WorldEntity = NULL;
CBaseEntity *my_g_WorldEntity_cbase = NULL;

int gCmdIndex;
int gMaxClients;

unsigned long serverdll_addr;
int g_sModelIndexSmoke;
short g_sModelIndexBubbles;

bool CommandInitialize();

//extern sp_nativeinfo_t g_MonsterNatives[];


SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, false, bool, const char *, const char *, const char *, const char *, bool, bool);
SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int, int);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, 0, int);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);


#define	TEST_SIGNATURE		1

#if TEST_SIGNATURE
size_t UTIL_DecodeHexString(unsigned char *buffer, size_t maxlength, const char *hexstr)
{
	size_t written = 0;
	size_t length = strlen(hexstr);

	for (size_t i = 0; i < length; i++)
	{
		if (written >= maxlength)
			break;
		buffer[written++] = hexstr[i];
		if (hexstr[i] == '\\' && hexstr[i + 1] == 'x')
		{
			if (i + 3 >= length)
				continue;
			/* Get the hex part. */
			char s_byte[3];
			int r_byte;
			s_byte[0] = hexstr[i + 2];
			s_byte[1] = hexstr[i + 3];
			s_byte[2] = '\0';
			/* Read it as an integer */
			sscanf(s_byte, "%x", &r_byte);
			/* Save the value */
			buffer[written - 1] = r_byte;
			/* Adjust index */
			i += 3;
		}
	}

	return written;
}

class Test_Signature : public ITextListener_SMC
{
	virtual void ReadSMC_ParseStart()
	{
		has_error = false;
		addrInBase = (void *)g_SMAPI->GetServerFactory(false);
		ignore = true;
	}

	virtual SMCResult ReadSMC_NewSection(const SMCStates *states, const char *name)
	{
		if(strcmp(name,"Signatures") == 0)
		{
			ignore = false;
		}

		strncpy(current_name,name, strlen(name));
		current_name[strlen(name)] = '\0';

		return SMCResult_Continue;
	}

	virtual SMCResult ReadSMC_KeyValue(const SMCStates *states, const char *key, const char *value)
	{
		if(!ignore && strcmp(key,"windows") == 0)
		{
			unsigned char real_sig[511];
			size_t real_bytes;
			size_t length;

			real_bytes = 0;
			length = strlen(value);

			real_bytes = UTIL_DecodeHexString(real_sig, sizeof(real_sig), value);
			if (real_bytes >= 1)
			{
				void *addr = memutils->FindPattern(addrInBase,(char *)real_sig,real_bytes);
				if(addr == NULL)
				{
					has_error = true;
					META_CONPRINTF("[%s DEBUG] %s - FAIL\n",g_Monster.GetLogTag(), current_name);
				}
			}
		}
		return SMCResult_Continue;
	}

public:
	bool HasError() { return has_error; }

private:
	bool ignore;
	void *addrInBase;
	char current_name[128];
	bool has_error;
} g_Test_Signature;
#endif


bool Monster::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
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

	//0EB829DE 
	//META_CONPRINTF("%p\n",reinterpret_cast<void *>(0x0EAE91F0  -serverdll_addr));
	//META_CONPRINTF("%p\n",reinterpret_cast<void *>(0x000D5090 +serverdll_addr));

	const char *game_foler = g_pSM->GetGameFolderName();

	/*if(stricmp(game_foler,"hl2mp") == 0 || 
	   stricmp(game_foler,"garrysmod") == 0 ||
	   stricmp(game_foler,"obsidian") == 0
	)*/

	if(stricmp(game_foler,"cstrike") != 0)
	{
		g_pSM->Format(error, maxlength, "NOT allow load this extension: %s", game_foler);
		return false;
	}

	char conf_error[255] = "";
	char config_path[255];
	snprintf(config_path, sizeof(config_path),"monster/monster.%s.games",game_foler);
	if (!gameconfs->LoadGameConfigFile(config_path, &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			g_pSM->Format(error, maxlength, "Could not read monster.%s.games: %s", game_foler, conf_error);
		}
		return false;
	}

#if TEST_SIGNATURE

	char path[512];
	g_pSM->BuildPath(Path_SM,path, sizeof(path),"gamedata/monster/monster.%s.games.txt",game_foler);
	SMCStates state = {0, 0};
	textparsers->ParseFile_SMC(path, &g_Test_Signature, &state);

	if(g_Test_Signature.HasError())
	{
		g_pSM->LogError(myself, "Some Signature counld not found.");
		return false;
	}

#endif

	if (!GetEntityManager()->Init(g_pGameConf))
	{
		g_pSM->LogError(myself, "CEntity failed to Initialize.");
		return false;
	}

	g_pSharedChangeInfo = engine->GetSharedEdictChangeInfo();

	g_pEntityList = (CBaseEntityList *)gamehelpers->GetGlobalEntityList();
	
	if(!CommandInitialize())
	{
		g_pSM->LogError(myself, "Command failed to Initialize. Server may Crash!");
		return false;
	}

	if(!g_helpfunc.Initialize())
	{
		g_pSM->LogError(myself, "Helper failed to Initialize. Server may Crash!");
		return false;
	}

	IGameSystem::SDKInitAllSystems();

	//sharesys->AddNatives(myself, g_MonsterNatives);

 	return true;
}



void Monster::SDK_OnUnload()
{
	if(g_pGameConf != NULL)
	{
		gameconfs->CloseGameConfigFile(g_pGameConf);
		g_pGameConf = NULL;
	}
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
	CreateInterfaceFn appSystemFactory = ismm->GetEngineFactory();
	ConnectTier1Libraries(&appSystemFactory, 1);
	ConnectTier3Libraries(&appSystemFactory, 1);

	MathLib_Init(2.2f, 2.2f, 0.0f, 2);

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
	GET_V_IFACE_CURRENT(GetEngineFactory, soundemitterbase, ISoundEmitterSystemBase,  SOUNDEMITTERSYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetPhysicsFactory, physcollision, IPhysicsCollision,  VPHYSICS_COLLISION_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pEffects, IEffects, IEFFECTS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetPhysicsFactory, iphysics, IPhysics,  VPHYSICS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, enginesound, IEngineSound,  IENGINESOUND_SERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, mdlcache, IMDLCache,  MDLCACHE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pGameMovement, IGameMovement, INTERFACENAME_GAMEMOVEMENT);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);

	g_pCVar = icvar;
	
	ConVar_Register(0, this);

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, ServerActivate, gamedll, this, &Monster::ServerActivate, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, serverclients, this, &Monster::SetCommandClient, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &Monster::OnLevelShutdown, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, this, &Monster::LevelInit, false);

	IGameSystem::InitAllSystems();

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
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelShutdown, gamedll, this, &Monster::OnLevelShutdown, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, LevelInit, gamedll, this, &Monster::LevelInit, false);

	IGameSystem::SDKShutdownAllSystem();

	GetEntityManager()->Shutdown();

	DisconnectTier1Libraries();
	DisconnectTier3Libraries();

	return true;
}

void Monster::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	gMaxClients = clientMax;

	IGameSystem::LevelInitPostEntityAllSystems();

	CEAI_NetworkManager::InitializeAINetworks();

	g_sModelIndexSmoke = engine->PrecacheModel("sprites/steam1.vmt",true);
	
	g_sModelIndexBubbles = engine->PrecacheModel("sprites/bubble.vmt",true);

	soundemitterbase->AddSoundOverrides("scripts/sm_monster/game_sounds_BaseNpc.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_headcrab.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_fastheadcrab.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_blackheadcrab.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_zombie.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_fastzombie.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_poisonzombie.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_manhack.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_antlionguard.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_antlionguard_episodic2.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_antlionguard_episodic.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_stalker.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_antlion.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/game_sounds_weapons.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/game_sounds_items.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_vortigaunt.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_rollermine.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_antlion_episodic.txt");
	soundemitterbase->AddSoundOverrides("scripts/sm_monster/npc_sounds_combine_cannon.txt");

	RETURN_META(MRES_IGNORED);
}

static bool g_LevelEndBarrier = false;
bool Monster::LevelInit(char const *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background)
{
	g_LevelEndBarrier = false;

	RETURN_META_VALUE(MRES_IGNORED, true);
}

void Monster::OnLevelShutdown()
{
	if(g_LevelEndBarrier)
		RETURN_META(MRES_IGNORED);

	g_LevelEndBarrier = true;
	IGameSystem::LevelShutdownAllSystems();

	RETURN_META(MRES_IGNORED);
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


