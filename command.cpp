
#include "command.h"
#include "CEntity.h"
#include "CAI_NetworkManager.h"
#include "npc_headcrab.h"

ConVar *sv_gravity = NULL;
ConVar *phys_pushscale = NULL;

void endround_CommandCallback(const CCommand &command)
{
	int client = g_Monster.GetCommandClient();
	if(client)
	{
		if(command.ArgC() == 2)
		{
			CBaseEntity *cbase = g_helpfunc.CreateEntityByName("npc_headcrab_fast");
			CEntity *cent = CEntity::Instance(cbase);

			cent->Spawn();
			Vector vec(501.0f,22.7f,70.21f);
			cent->Teleport(&vec, NULL,NULL);
			return;
		}		
		
		CBaseEntity *cbase = g_helpfunc.CreateEntityByName("npc_headcrab");
		CEntity *cent = CEntity::Instance(cbase);

		cent->Spawn();
		Vector vec(501.0f,22.7f,70.21f);
		cent->Teleport(&vec, NULL,NULL);

	} else {

		

		//engine->ServerCommand("developer 100\n");

		//CBaseEntity *cc = g_helpfunc.CreateEntityByName("physics_npc_solver");
	
		//CEntity *dd = CEntity::Instance(cc);
		//dd->Spawn();


		//CBaseEntity *cbase = g_helpfunc.CreateEntityByName("npc_headcrab");
		//CEntity *cent = CEntity::Instance(cbase);

		//CE_NPC_Headcrab *hc = dynamic_cast<CE_NPC_Headcrab *>(cent);

		CBaseEntity *cbase = g_helpfunc.CreateEntityByName("npc_headcrab_fast");
		CEntity *cent = CEntity::Instance(cbase);
		CE_NPC_FastHeadcrab *hc = dynamic_cast<CE_NPC_FastHeadcrab *>(cent);

		/*CBaseEntity *cbase = g_helpfunc.CreateEntityByName("npc_headcrab_black");
		CEntity *cent = CEntity::Instance(cbase);
		CE_NPC_BlackHeadcrab *hc = dynamic_cast<CE_NPC_BlackHeadcrab *>(cent);*/


		
		cent->Spawn();

		Vector vec(501.0f,22.7f,70.21f);
		cent->Teleport(&vec, NULL,NULL);

		//hc->PlayerHasIlluminatedNPC(NULL, 0);	

		edict_t *pEdict = servergameents->BaseEntityToEdict(cbase);
		//DispatchSpawn(pEdict);
		META_CONPRINTF("%p\n",cbase);
	}
}


void endround2_CommandCallback(const CCommand &command)
{
	/*for (int i=0;i<=gpGlobals->maxEntities;i++)
	{
		edict_t *pEdict = engine->PEntityOfEntIndex(i);
		if(IsValidEdict(pEdict) && IsValidEntity(pEdict))
		{
			const char *cls = pEdict->GetClassName();
			META_CONPRINTF("%s\n",cls);
		}
	}*/
	

	CEAI_NetworkManager::InitializeAINetworks();

	for (int i=1;i<=g_Monster.GetMaxClients();i++)
	{
		IGamePlayer *gPlayer = playerhelpers->GetGamePlayer(i);
		if(gPlayer == NULL || !gPlayer->IsInGame())
			continue;
		CEntity *cent = CEntity::Instance(i);
		if(cent->IsAlive())
		{
			META_CONPRINTF("%s\n",gPlayer->GetName());
		}
	}
}

void endround3_CommandCallback(const CCommand &command)
{
	engine->ServerCommand("exec 1.cfg\n");
}

void CommandInitialize()
{
	new ConCommand("e5",endround_CommandCallback, "", FCVAR_GAMEDLL);
	new ConCommand("e6",endround2_CommandCallback, "", FCVAR_GAMEDLL);
	new ConCommand("pp",endround3_CommandCallback, "", FCVAR_GAMEDLL);

	sv_gravity = g_pCVar->FindVar("sv_gravity");

	phys_pushscale = g_pCVar->FindVar("phys_pushscale");
}