
#include "CAI_NPC.h"
#include "eventqueue.h"

ConVar *sv_gravity = NULL;
ConVar *phys_pushscale = NULL;
ConVar *npc_height_adjust = NULL;
ConVar *ai_path_adjust_speed_on_immediate_turns = NULL;
ConVar *ai_path_insert_pause_at_obstruction = NULL;
ConVar *ai_path_insert_pause_at_est_end = NULL;
ConVar *scene_flatturn = NULL;
ConVar *ai_use_clipped_paths = NULL;
ConVar *ai_moveprobe_usetracelist = NULL;
ConVar *ai_use_visibility_cache = NULL;
ConVar *ai_strong_optimizations = NULL;

ConVar *violence_hblood = NULL;
ConVar *violence_ablood = NULL;
ConVar *violence_hgibs = NULL;
ConVar *violence_agibs = NULL;

ConVar *sv_suppress_viewpunch = NULL;
ConVar *ai_navigator_generate_spikes = NULL;
ConVar *ai_navigator_generate_spikes_strength = NULL;

ConVar *ai_no_node_cache = NULL;
ConVar *sv_stepsize = NULL;
ConVar *hl2_episodic = NULL;
ConVar *ai_follow_use_points = NULL;
ConVar *ai_LOS_mode = NULL;
ConVar *ai_follow_use_points_when_moving = NULL;

ConVar *ammo_hegrenade_max = NULL;

ConVar *sv_strict_notarget = NULL;

ConVar *ai_shot_bias_min = NULL;
ConVar *ai_shot_bias_max = NULL;
ConVar *ai_shot_bias = NULL;
ConVar *ai_spread_pattern_focus_time = NULL;

ConVar *ai_lead_time = NULL;

ConVar *scene_clamplookat = NULL;
ConVar *scene_showfaceto = NULL;
ConVar *flex_maxawaytime = NULL;
ConVar *flex_minawaytime = NULL;
ConVar *flex_maxplayertime = NULL;
ConVar *flex_minplayertime = NULL;

ConVar *ai_find_lateral_los = NULL;

ConVar *npc_sentences = NULL;


void cmd1_CommandCallback(const CCommand &command)
{
	int client = g_Monster.GetCommandClient();
	if(client)
	{
		Vector vec(501.0f,22.7f,70.21f);

		g_pEffects->MuzzleFlash(vec,vec3_angle, 1.0, 1);

	} else {
		Vector vec(501.0f,22.7f,70.21f);

		//vec.Init(6220.0f, 2813.0f, 1090.0f);

		//vec.Init(73.18,-54.81,-60.0);

		//vec.Init(952.65466,61.566082,-58.339985);


		//CEntity *cent = CreateEntityByName("npc_headcrab");
		//CEntity *cent = CreateEntityByName("npc_headcrab_fast");
		//CEntity *cent = CreateEntityByName("npc_headcrab_black");

		//CEntity *cent = CreateEntityByName("npc_fastzombie");
		//CEntity cent = CreateEntityByName("npc_fastzombie_torso");
		//CEntity *cent = CreateEntityByName("npc_zombie_torso");
		//CEntity *cent = CreateEntityByName("npc_zombie");
		//CEntity *cent = CreateEntityByName("npc_poisonzombie");
		
		//CEntity *cent = CreateEntityByName("npc_manhack");
		//CEntity *cent = CreateEntityByName("npc_antlionguard");

		//CEntity *cent = CreateEntityByName("npc_stalker");

		//CEntity *cent = CreateEntityByName("npc_antlion");

		//CEntity *cent = CreateEntityByName("npc_vortigaunt");

		//CEntity *cent = CreateEntityByName("npc_rollermine");
		
		//CEntity *cent = CreateEntityByName("npc_test");
		
		/*CEntity *cent = CreateEntityByName("env_headcrabcanister");
	
		cent->DispatchKeyValue("HeadcrabType", "0");
		cent->DispatchKeyValue("HeadcrabCount", "10");
		
		cent->DispatchKeyValue("SmokeLifetime","60");
		cent->DispatchKeyValue("SkyboxCannisterCount","1");
		cent->DispatchKeyValue("DamageRadius","0");
		cent->DispatchKeyValue("Damage","100");*/

		//CEntity *cent = CreateEntityByName("npc_turret_floor");

		CEntity *cent = CreateEntityByName("npc_combine");
		
		CBaseEntity *cbase = cent->BaseEntity();

		CAI_NPC *hc = dynamic_cast<CAI_NPC *>(cent);
		
		cent->Teleport(&vec, NULL,NULL);

		//cent->AddSpawnFlags(4096);

		cent->Spawn();
		cent->Activate();

		//g_CEventQueue->AddEvent( cbase, "SelfDestruct", 0.5f, cbase,cbase );

		//hc->Dissolve(NULL, gpGlobals->curtime, false, 0 );

		edict_t *pEdict = servergameents->BaseEntityToEdict(cbase);
		META_CONPRINTF("%p %d %d\n",cbase, cent->entindex_non_network(), engine->IndexOfEdict(pEdict));

	}
}

void cmd2_CommandCallback(const CCommand &command)
{
	Vector vec(501.0f,22.7f,70.21f);
	CEntity *cent = CreateEntityByName("npc_stalker");

	CBaseEntity *cbase = cent->BaseEntity();
	CAI_NPC *hc = dynamic_cast<CAI_NPC *>(cent);
	
	cent->Teleport(&vec, NULL,NULL);

	cent->Spawn();

	/*for (int i=0;i<ENTITY_ARRAY_SIZE;i++)
	{
		CEntity *cent = CEntity::Instance(i);
		if(cent != NULL)
		{
			META_CONPRINTF("%s\n",cent->GetClassname());

		}
	}*/
}

void monster_dump_CommandCallback(const CCommand &command)
{
	GetEntityManager()->PrintDump();
}

#define GET_CONVAR(name) \
	name = g_pCVar->FindVar(#name); \
	if(name == NULL) { \
		META_CONPRINTF("[%s] %s - FAIL\n",g_Monster.GetLogTag(), #name); \
		return false; \
	}


bool CommandInitialize()
{
#if 1
	new ConCommand("e5",cmd1_CommandCallback, "", FCVAR_GAMEDLL);
	new ConCommand("e6",cmd2_CommandCallback, "", 0);
	new ConCommand("pp",monster_dump_CommandCallback, "", 0);
#endif
	new ConCommand("monster_dump",monster_dump_CommandCallback, "", 0);

	GET_CONVAR(sv_gravity);
	GET_CONVAR(phys_pushscale);
	GET_CONVAR(npc_height_adjust);

	GET_CONVAR(ai_path_adjust_speed_on_immediate_turns);
	GET_CONVAR(ai_path_insert_pause_at_obstruction);
	GET_CONVAR(ai_path_insert_pause_at_est_end);
	GET_CONVAR(ai_use_clipped_paths);
	GET_CONVAR(ai_moveprobe_usetracelist);
	GET_CONVAR(scene_flatturn);

	GET_CONVAR(violence_hblood);
	GET_CONVAR(violence_ablood);
	GET_CONVAR(violence_hgibs);
	GET_CONVAR(violence_agibs);

	GET_CONVAR(sv_suppress_viewpunch);
	GET_CONVAR(ai_use_visibility_cache);
	GET_CONVAR(ai_strong_optimizations);
	GET_CONVAR(ai_navigator_generate_spikes);
	GET_CONVAR(ai_navigator_generate_spikes_strength);
	GET_CONVAR(ai_no_node_cache);

	GET_CONVAR(sv_stepsize);
	GET_CONVAR(hl2_episodic);
	GET_CONVAR(ai_follow_use_points);
	GET_CONVAR(ai_LOS_mode);
	GET_CONVAR(ai_follow_use_points_when_moving);

	GET_CONVAR(ammo_hegrenade_max);
	
	GET_CONVAR(sv_strict_notarget);

	GET_CONVAR(ai_shot_bias_min);
	GET_CONVAR(ai_shot_bias_max);
	GET_CONVAR(ai_shot_bias);
	GET_CONVAR(ai_spread_pattern_focus_time);

	GET_CONVAR(ai_lead_time);

	GET_CONVAR(scene_clamplookat);
	GET_CONVAR(scene_showfaceto);
	GET_CONVAR(flex_maxawaytime);
	GET_CONVAR(flex_minawaytime);
	GET_CONVAR(flex_maxplayertime);
	GET_CONVAR(flex_minplayertime);

	GET_CONVAR(ai_find_lateral_los);
	GET_CONVAR(npc_sentences);

	return true;
}
