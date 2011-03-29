
#include "command.h"
#include "CEntity.h"
#include "npc_headcrab.h"


void endround_CommandCallback(const CCommand &command)
{
	int client = g_Monster.GetCommandClient();
	if(client)
	{
		if(command.ArgC() == 2)
		{
			return;
		}		

		

	} else {

		engine->ServerCommand("developer 100\n");

		CBaseEntity *cbase = g_helpfunc.CreateEntityByName("mm_npc_headcrab");
		CEntity *cent = CEntity::Instance(cbase);
		cent->Spawn();

		//Vector vec(6542,2974,1090);
		Vector vec(589,128,-100);
		cent->Teleport(&vec, NULL,NULL);
		//CBaseEntity *cbase = g_helpfunc.CreateEntityByName("cycler");
		
		CE_NPC_Headcrab *hc = dynamic_cast<CE_NPC_Headcrab *>(cent);
		//hc->SelectFailSchedule(1,1,1);
		Task_t tempTask = { 152, 48 };
		//hc->RunTask((const Task_t *)	&tempTask);
		
		//const char *ss = hc->TaskName(1);

		//hc->PrescheduleThink();

		edict_t *pEdict = servergameents->BaseEntityToEdict(cbase);
		//DispatchSpawn(pEdict);
		META_CONPRINTF("%p\n",cbase);
	}
}


void endround2_CommandCallback(const CCommand &command)
{
	for (int i=0;i<=gpGlobals->maxEntities;i++)
	{
		edict_t *pEdict = engine->PEntityOfEntIndex(i);
		if(IsValidEdict(pEdict) && IsValidEntity(pEdict))
		{
			const char *cls = pEdict->GetClassName();
			META_CONPRINTF("%s\n",cls);
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
}