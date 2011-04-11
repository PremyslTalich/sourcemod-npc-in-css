
#include "CAI_NPC.h"
#include "stringregistry.h"

/*
#define GAME_DLL
#include "cbase.h"

#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hint.h"
#include "ai_hull.h"
#include "ai_navigator.h"
#include "ai_moveprobe.h"
#include "ai_memory.h"
*/


/*
CAI_SchedulesManager *my_g_AI_SchedulesManager = NULL;
CAI_GlobalScheduleNamespace *my_gm_SchedulingSymbols = NULL;
CAI_ClassScheduleIdSpace *my_gm_ClassScheduleIdSpace = NULL;

CAI_LocalIdSpace    *my_gm_SquadSlotIdSpace = NULL;
CAI_GlobalNamespace *my_gm_SquadSlotNamespace = NULL;

CAI_GlobalNamespace CAI_NPC::gm_SquadSlotNamespace;
CAI_LocalIdSpace    CAI_NPC::gm_SquadSlotIdSpace;
CAI_ClassScheduleIdSpace CAI_NPC::gm_ClassScheduleIdSpace;

*/

CAI_ClassScheduleIdSpace	CAI_NPC::gm_ClassScheduleIdSpace( true );
CAI_GlobalScheduleNamespace CAI_NPC::gm_SchedulingSymbols;
CAI_LocalIdSpace			CAI_NPC::gm_SquadSlotIdSpace( true );
CAI_GlobalNamespace			CAI_NPC::gm_SquadSlotNamespace;

CStringRegistry* CAI_NPC::m_pActivitySR	= NULL;
int				 CAI_NPC::m_iNumActivities = 0;

CStringRegistry* CAI_NPC::m_pEventSR	= NULL;
int				 CAI_NPC::m_iNumEvents	= 0;

CE_LINK_ENTITY_TO_CLASS(CAI_BaseNPC, CAI_NPC);

DECLARE_DETOUR(SetHullSizeNormal, CAI_NPC);
DECLARE_DEFAULTHANDLER_DETOUR_void(CAI_NPC, SetHullSizeNormal, (bool force),(force));


SH_DECL_MANUALHOOK0(GetEnemies, 0, 0, 0, CEAI_Enemies *);
DECLARE_HOOK(GetEnemies, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, GetEnemies, CEAI_Enemies *, (), ());

SH_DECL_MANUALHOOK0_void(NPCInit, 0, 0, 0);
DECLARE_HOOK(NPCInit, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, NPCInit, (), ());

DECLARE_DETOUR(SetState, CAI_NPC);
DECLARE_DEFAULTHANDLER_DETOUR_void(CAI_NPC, SetState, (NPC_STATE State), (State));

SH_DECL_MANUALHOOK1_void(SetActivity, 0, 0, 0, Activity);
DECLARE_HOOK(SetActivity, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, SetActivity, (Activity NewActivity), (NewActivity));

DECLARE_DETOUR(SetSchedule_Int, CAI_NPC);
DECLARE_DEFAULTHANDLER_DETOUR(CAI_NPC, SetSchedule_Int, bool, (int localScheduleID), (localScheduleID));

SH_DECL_MANUALHOOK1_void(RunTask, 0, 0, 0, const Task_t *);
DECLARE_HOOK(RunTask, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, RunTask, (const Task_t *pTask), (pTask));

SH_DECL_MANUALHOOK1_void(OnChangeActivity, 0, 0, 0, Activity);
DECLARE_HOOK(OnChangeActivity, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, OnChangeActivity, (Activity eNewActivity), (eNewActivity));

SH_DECL_MANUALHOOK0(MaxYawSpeed, 0, 0, 0, float);
DECLARE_HOOK(MaxYawSpeed, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, MaxYawSpeed, float,(), ());


SH_DECL_MANUALHOOK0(GetClassScheduleIdSpace, 0, 0, 0, CAI_ClassScheduleIdSpace *);
DECLARE_HOOK(GetClassScheduleIdSpace, CAI_NPC);
//DECLARE_DEFAULTHANDLER(CAI_NPC,GetClassScheduleIdSpace, CAI_ClassScheduleIdSpace*, (), ());


CAI_ClassScheduleIdSpace *CAI_NPC::GetClassScheduleIdSpace()
{
	if(!m_bInGetClassScheduleIdSpace)
	{
		return SH_MCALL(BaseEntity(), GetClassScheduleIdSpace)();
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	CAI_ClassScheduleIdSpace *ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPGetClassScheduleIdSpace(thisptr)))();
	SET_META_RESULT(MRES_SUPERCEDE);
	return ret;
}

CAI_ClassScheduleIdSpace *CAI_NPC::InternalGetClassScheduleIdSpace()
{
	SET_META_RESULT(MRES_SUPERCEDE);
	CAI_NPC *pEnt = dynamic_cast<CAI_NPC *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, NULL);
	}

	int index = pEnt->entindex();
	pEnt->m_bInGetClassScheduleIdSpace = true;
	CAI_ClassScheduleIdSpace *space = pEnt->GetClassScheduleIdSpace();
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInGetClassScheduleIdSpace = false;

	return space;
	//return &gm_ClassScheduleIdSpace;
}


SH_DECL_MANUALHOOK0_void(NPCThink, 0, 0, 0);
DECLARE_HOOK(NPCThink, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, NPCThink,(), ());

SH_DECL_MANUALHOOK1_void(HandleAnimEvent, 0, 0, 0, animevent_t *);
DECLARE_HOOK(HandleAnimEvent, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, HandleAnimEvent,(animevent_t *pEvent), (pEvent));

SH_DECL_MANUALHOOK0(IsActivityFinished, 0, 0, 0, bool);
DECLARE_HOOK(IsActivityFinished, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, IsActivityFinished, bool, (), ());

SH_DECL_MANUALHOOK1(CalcIdealYaw, 0, 0, 0, float, const Vector &);
DECLARE_HOOK(CalcIdealYaw, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, CalcIdealYaw, float, (const Vector &vecTarget), (vecTarget));

SH_DECL_MANUALHOOK0_void(GatherConditions, 0, 0, 0);
DECLARE_HOOK(GatherConditions, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, GatherConditions, (), ());

SH_DECL_MANUALHOOK0_void(PrescheduleThink, 0, 0, 0);
DECLARE_HOOK(PrescheduleThink, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, PrescheduleThink, (), ());

SH_DECL_MANUALHOOK0_void(IdleSound, 0, 0, 0);
DECLARE_HOOK(IdleSound, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, IdleSound, (), ());

SH_DECL_MANUALHOOK0_void(AlertSound, 0, 0, 0);
DECLARE_HOOK(AlertSound, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, AlertSound, (), ());

SH_DECL_MANUALHOOK1_void(PainSound, 0, 0, 0, const CTakeDamageInfo &);
DECLARE_HOOK(PainSound, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, PainSound, (const CTakeDamageInfo &info), (info));

SH_DECL_MANUALHOOK1_void(DeathSound, 0, 0, 0, const CTakeDamageInfo &);
DECLARE_HOOK(DeathSound, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, DeathSound, (const CTakeDamageInfo &info), (info));

SH_DECL_MANUALHOOK1(FValidateHintType, 0, 0, 0, bool, CAI_Hint *);
DECLARE_HOOK(FValidateHintType, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, FValidateHintType, bool, (CAI_Hint *pHint), (pHint));

SH_DECL_MANUALHOOK1(FInAimCone_Vector, 0, 0, 0, bool, const Vector &);
DECLARE_HOOK(FInAimCone_Vector, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, FInAimCone_Vector, bool, (const Vector &vecSpot), (vecSpot));

SH_DECL_MANUALHOOK0(BestEnemy, 0, 0, 0, CBaseEntity *);
DECLARE_HOOK(BestEnemy, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, BestEnemy, CBaseEntity *, (), ());

SH_DECL_MANUALHOOK1_void(TaskFail, 0, 0, 0, AI_TaskFailureCode_t);
DECLARE_HOOK(TaskFail, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, TaskFail, (AI_TaskFailureCode_t code), (code));

SH_DECL_MANUALHOOK1_void(StartTask, 0, 0, 0, const Task_t *);
DECLARE_HOOK(StartTask, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, StartTask, (const Task_t *pTask), (pTask));

SH_DECL_MANUALHOOK1(Event_Gibbed, 0, 0, 0, bool, const CTakeDamageInfo &);
DECLARE_HOOK(Event_Gibbed, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, Event_Gibbed, bool, (const CTakeDamageInfo &info), (info));

SH_DECL_MANUALHOOK1(TranslateSchedule, 0, 0, 0, int, int);
DECLARE_HOOK(TranslateSchedule, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, TranslateSchedule, int, (int scheduleType), (scheduleType));

SH_DECL_MANUALHOOK0(SelectSchedule, 0, 0, 0, int);
DECLARE_HOOK(SelectSchedule, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, SelectSchedule, int, (), ());

SH_DECL_MANUALHOOK3(SelectFailSchedule, 0, 0, 0, int, int, int, AI_TaskFailureCode_t);
DECLARE_HOOK(SelectFailSchedule, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, SelectFailSchedule, int, (int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode), (failedSchedule, failedTask, taskFailCode));

SH_DECL_MANUALHOOK2(Weapon_TranslateActivity, 0, 0, 0, Activity, Activity , bool *);
DECLARE_HOOK(Weapon_TranslateActivity, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, Weapon_TranslateActivity, Activity, (Activity baseAct, bool *pRequired), (baseAct, pRequired));

SH_DECL_MANUALHOOK1(NPC_TranslateActivity, 0, 0, 0, Activity, Activity);
DECLARE_HOOK(NPC_TranslateActivity, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, NPC_TranslateActivity, Activity, (Activity eNewActivity), (eNewActivity));

SH_DECL_MANUALHOOK2_void(PlayerHasIlluminatedNPC, 0, 0, 0, CBaseEntity *, float);
DECLARE_HOOK(PlayerHasIlluminatedNPC, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, PlayerHasIlluminatedNPC, (CBaseEntity *pPlayer, float flDot), (pPlayer, flDot));

SH_DECL_MANUALHOOK2(QuerySeeEntity, 0, 0, 0, bool, CBaseEntity *, bool);
DECLARE_HOOK(QuerySeeEntity, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, QuerySeeEntity, bool, (CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC), (pEntity, bOnlyHateOrFearIfNPC));

SH_DECL_MANUALHOOK0(GetSoundInterests, 0, 0, 0, int);
DECLARE_HOOK(GetSoundInterests, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, GetSoundInterests, int, (), ());

SH_DECL_MANUALHOOK0_void(BuildScheduleTestBits, 0, 0, 0);
DECLARE_HOOK(BuildScheduleTestBits, CAI_NPC);
DECLARE_DEFAULTHANDLER_void(CAI_NPC, BuildScheduleTestBits, (), ());


SH_DECL_MANUALHOOK1(GetSchedule, 0, 0, 0, CAI_Schedule *, int);
DECLARE_HOOK(GetSchedule, CAI_NPC);


CAI_Schedule *CAI_NPC::GetSchedule(int schedule)
{
	if(!m_bInGetSchedule)
	{
		return SH_MCALL(BaseEntity(), GetSchedule)(schedule);
	}

	SET_META_RESULT(MRES_IGNORED);
	SH_GLOB_SHPTR->DoRecall();
	SourceHook::EmptyClass *thisptr = reinterpret_cast<SourceHook::EmptyClass*>(SH_GLOB_SHPTR->GetIfacePtr());
	CAI_Schedule *ret = (thisptr->*(__SoureceHook_FHM_GetRecallMFPGetSchedule(thisptr)))(schedule);
	SET_META_RESULT(MRES_SUPERCEDE);
	return ret;
}

CAI_Schedule *CAI_NPC::InternalGetSchedule(int schedule)
{
	SET_META_RESULT(MRES_SUPERCEDE);
	
	CAI_NPC *pEnt = dynamic_cast<CAI_NPC *>(CEntity::Instance(META_IFACEPTR(CBaseEntity)));
	if (!pEnt)
	{
		RETURN_META_VALUE(MRES_IGNORED, NULL);
	}

	int index = pEnt->entindex();
	pEnt->m_bInGetSchedule = true;
	CAI_Schedule *ret = NULL;

	if (!pEnt->GetClassScheduleIdSpace()->IsGlobalBaseSet())
	{
		ret = g_AI_SchedulesManager.GetScheduleFromID(SCHED_IDLE_STAND);
		if (pEnt == CEntity::Instance(index))
			pEnt->m_bInGetSchedule = false;
		return ret;
	}
	if ( AI_IdIsLocal( schedule ) )
	{
		schedule = pEnt->GetClassScheduleIdSpace()->ScheduleLocalToGlobal(schedule);
	}
	
	ret = g_AI_SchedulesManager.GetScheduleFromID( schedule );
	if (pEnt == CEntity::Instance(index))
		pEnt->m_bInGetSchedule = false;
	return ret;
}


DECLARE_DETOUR(SetEnemy, CAI_NPC);
DECLARE_DEFAULTHANDLER_DETOUR_void(CAI_NPC, SetEnemy, (CBaseEntity *pEnemy, bool bSetCondNewEnemy), (pEnemy, bSetCondNewEnemy));

SH_DECL_MANUALHOOK3(UpdateEnemyMemory, 0, 0, 0, bool, CBaseEntity *, const Vector &, CBaseEntity *);
DECLARE_HOOK(UpdateEnemyMemory, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, UpdateEnemyMemory, bool, (CBaseEntity *pEnemy, const Vector &position, CBaseEntity *pInformer), (pEnemy, position, pInformer));

SH_DECL_MANUALHOOK2(OverrideMoveFacing, 0, 0, 0, bool, const AILocalMoveGoal_t &, float );
DECLARE_HOOK(OverrideMoveFacing, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, OverrideMoveFacing, bool, (const AILocalMoveGoal_t &move, float flInterval), (move, flInterval));


SH_DECL_MANUALHOOK2(GetHitgroupDamageMultiplier, 0, 0, 0, float, int, const CTakeDamageInfo & );
DECLARE_HOOK(GetHitgroupDamageMultiplier, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, GetHitgroupDamageMultiplier, float, (int iHitGroup, const CTakeDamageInfo &info), (iHitGroup, info));

SH_DECL_MANUALHOOK2(OnBehaviorChangeStatus, 0, 0, 0, bool, CAI_BehaviorBase *, bool);
DECLARE_HOOK(OnBehaviorChangeStatus, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, OnBehaviorChangeStatus, bool, (CAI_BehaviorBase *pBehavior, bool fCanFinishSchedule), (pBehavior, fCanFinishSchedule));

SH_DECL_MANUALHOOK0(IsInterruptable, 0, 0, 0, bool);
DECLARE_HOOK(IsInterruptable, CAI_NPC);
DECLARE_DEFAULTHANDLER(CAI_NPC, IsInterruptable, bool, (), ());



//Datamaps
DEFINE_PROP(m_NPCState, CAI_NPC);
DEFINE_PROP(m_afCapability, CAI_NPC);
DEFINE_PROP(m_Conditions, CAI_NPC);
DEFINE_PROP(m_hEnemy, CAI_NPC);
DEFINE_PROP(m_ScheduleState, CAI_NPC);
DEFINE_PROP(m_MoveAndShootOverlay, CAI_NPC);
DEFINE_PROP(m_pMotor, CAI_NPC);
DEFINE_PROP(m_flNextFlinchTime, CAI_NPC);
DEFINE_PROP(m_pMoveProbe, CAI_NPC);
DEFINE_PROP(m_Activity, CAI_NPC);
DEFINE_PROP(m_strHintGroup, CAI_NPC);
DEFINE_PROP(m_pHintNode, CAI_NPC);
DEFINE_PROP(m_SleepState, CAI_NPC);
DEFINE_PROP(m_pSchedule, CAI_NPC);
DEFINE_PROP(m_IdealSchedule, CAI_NPC);
DEFINE_PROP(m_iInteractionState, CAI_NPC);
DEFINE_PROP(m_hCine, CAI_NPC);
DEFINE_PROP(m_IdealActivity, CAI_NPC);
DEFINE_PROP(m_pNavigator, CAI_NPC);
DEFINE_PROP(m_pSquad, CAI_NPC);
DEFINE_PROP(m_iMySquadSlot, CAI_NPC);
DEFINE_PROP(m_afMemory, CAI_NPC);
DEFINE_PROP(m_CustomInterruptConditions, CAI_NPC);
DEFINE_PROP(m_IdealNPCState, CAI_NPC);
DEFINE_PROP(m_poseMove_Yaw, CAI_NPC);
DEFINE_PROP(m_InverseIgnoreConditions, CAI_NPC);
DEFINE_PROP(m_flWaitFinished, CAI_NPC);





//-----------------------------------------------------------------------------

// ================================================================
//  Init static data
// ================================================================

//CAI_LocalIdSpace			CAI_NPC::gm_SquadSlotIdSpace( true );

CAI_NPC::CAI_NPC()
{
	//unsigned char *obj = NULL;
	//memcpy(&obj, reinterpret_cast<void*>(m_pMotor.ptr), sizeof(char*));
	//m_pMotor.ptr = (CAI_Motor *)obj;
}

bool CAI_NPC::LoadDefaultSchedules()
{
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_IDLE_STAND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_IDLE_WALK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_IDLE_WANDER);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_WAKE_ANGRY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ALERT_FACE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ALERT_FACE_BESTSOUND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ALERT_REACT_TO_COMBAT_SOUND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ALERT_SCAN);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ALERT_STAND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ALERT_WALK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_INVESTIGATE_SOUND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_COMBAT_FACE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_COMBAT_SWEEP);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_COMBAT_WALK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FEAR_FACE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_COMBAT_STAND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_CHASE_ENEMY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_CHASE_ENEMY_FAILED);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_VICTORY_DANCE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_TARGET_FACE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_TARGET_CHASE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SMALL_FLINCH);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_BIG_FLINCH);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_BACK_AWAY_FROM_ENEMY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MOVE_AWAY_FROM_ENEMY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_BACK_AWAY_FROM_SAVE_POSITION);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_TAKE_COVER_FROM_ENEMY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_TAKE_COVER_FROM_BEST_SOUND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FLEE_FROM_BEST_SOUND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_TAKE_COVER_FROM_ORIGIN);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FAIL_TAKE_COVER);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RUN_FROM_ENEMY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RUN_FROM_ENEMY_FALLBACK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MOVE_TO_WEAPON_RANGE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ESTABLISH_LINE_OF_FIRE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SHOOT_ENEMY_COVER);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_PRE_FAIL_ESTABLISH_LINE_OF_FIRE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FAIL_ESTABLISH_LINE_OF_FIRE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_COWER);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MELEE_ATTACK1);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MELEE_ATTACK2);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RANGE_ATTACK1);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RANGE_ATTACK2);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SPECIAL_ATTACK1);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SPECIAL_ATTACK2);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_STANDOFF);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_ARM_WEAPON);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_DISARM_WEAPON);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_HIDE_AND_RELOAD);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RELOAD);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_AMBUSH);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_DIE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_DIE_RAGDOLL);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_WAIT_FOR_SCRIPT);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SCRIPTED_WALK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SCRIPTED_RUN);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SCRIPTED_CUSTOM_MOVE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SCRIPTED_WAIT);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SCRIPTED_FACE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SCENE_GENERIC);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_NEW_WEAPON);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_NEW_WEAPON_CHEAT);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SWITCH_TO_PENDING_WEAPON);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_GET_HEALTHKIT);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MOVE_AWAY);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MOVE_AWAY_FAIL);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_MOVE_AWAY_END);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_WAIT_FOR_SPEAK_FINISH);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FORCED_GO);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FORCED_GO_RUN);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_PATROL_WALK);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_COMBAT_PATROL);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_PATROL_RUN);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RUN_RANDOM);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FAIL);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FAIL_NOSTOP);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FALL_TO_GROUND);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_DROPSHIP_DUSTOFF);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_FLINCH_PHYSICS);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_RUN_FROM_ENEMY_MOB );
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_DUCK_DODGE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_NPC_FREEZE);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_INTERACTION_MOVE_TO_PARTNER);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_INTERACTION_WAIT_FOR_PARTNER);
	AI_LOAD_DEF_SCHEDULE( CAI_NPC,					SCHED_SLEEP );

	return true;
}

void CAI_NPC::CapabilitiesClear()
{
	m_afCapability = 0;
}

int CAI_NPC::CapabilitiesAdd(int capability)
{
	m_afCapability |= capability;
	return m_afCapability;
}

int CAI_NPC::CapabilitiesRemove(int capability)
{
	m_afCapability &= ~capability;
	return m_afCapability;
}

#define InterruptFromCondition( iCondition ) \
	AI_RemapFromGlobal( ( AI_IdIsLocal( iCondition ) ? GetClassScheduleIdSpace()->ConditionLocalToGlobal( iCondition ) : iCondition ) )
	
void CAI_NPC::SetCondition(int iCondition)
{
	int interrupt = InterruptFromCondition( iCondition );
	if ( interrupt == -1 )
	{
		Assert(0);
		return;
	}	
	m_Conditions->Set(interrupt);
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_NPC::HasCondition( int iCondition )
{
	int interrupt = InterruptFromCondition( iCondition );
	
	if ( interrupt == -1 )
	{
		Assert(0);
		return false;
	}
	
	bool bReturn = m_Conditions->IsBitSet(interrupt);
	return (bReturn);
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_NPC::ClearCondition( int iCondition )
{
	int interrupt = InterruptFromCondition( iCondition );
	
	if ( interrupt == -1 )
	{
		Assert(0);
		return;
	}
	
	m_Conditions->Clear(interrupt);
}

bool CAI_NPC::AutoMovement(CEntity *pTarget, AIMoveTrace_t *pTraceResult)
{
	return AutoMovement( GetAnimTimeInterval(), pTarget, pTraceResult );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//			 - 
//			*pTraceResult - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_NPC::AutoMovement( float flInterval, CEntity *pTarget, AIMoveTrace_t *pTraceResult )
{
	return g_helpfunc.AutoMovement(BaseEntity(), flInterval, pTarget->BaseEntity(), pTraceResult);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAI_NPC::TaskComplete(  bool fIgnoreSetFailedCondition )
{
	EndTaskOverlay();

	if ( fIgnoreSetFailedCondition || !HasCondition(COND_TASK_FAILED) )
	{
		SetTaskStatus( TASKSTATUS_COMPLETE );
	}
}

void CAI_NPC::EndTaskOverlay()
{
	g_helpfunc.EndTaskOverlay(BaseEntity());
}

void CAI_NPC::SetIdealActivity( Activity NewActivity )
{
	g_helpfunc.SetIdealActivity(BaseEntity(), NewActivity);
}


bool CAI_NPC::CanBeAnEnemyOf( CEntity *pEnemy )	
{ 
	if ( GetSleepState() > AISS_WAITING_FOR_THREAT )
		return false;

	return true; 
}


bool CAI_NPC::IsCurSchedule( int schedId, bool fIdeal )	
{ 
	if ( !m_pSchedule )
		return ( schedId == SCHED_NONE || schedId == AI_RemapToGlobal(SCHED_NONE) );

	schedId = ( AI_IdIsLocal( schedId ) ) ? 
							GetClassScheduleIdSpace()->ScheduleLocalToGlobal(schedId) : 
							schedId;
	if ( fIdeal )
		return ( schedId == m_IdealSchedule );

	return ( m_pSchedule->GetId() == schedId ); 
}


//=========================================================
// FacingIdeal - tells us if a npc is facing its ideal
// yaw. Created this function because many spots in the
// code were checking the yawdiff against this magic
// number. Nicer to have it in one place if we're gonna
// be stuck with it.
//=========================================================
bool CAI_NPC::FacingIdeal( void )
{
	if ( fabs( GetMotor()->DeltaIdealYaw() ) <= 0.006 )//!!!BUGBUG - no magic numbers!!!
	{
		return true;
	}

	return false;
}

//=========================================================
// GetTask - returns a pointer to the current 
// scheduled task. NULL if there's a problem.
//=========================================================
const Task_t *CAI_NPC::GetTask( void ) 
{
	int iScheduleIndex = GetScheduleCurTaskIndex();
	if ( !GetCurSchedule() ||  iScheduleIndex < 0 || iScheduleIndex >= GetCurSchedule()->NumTasks() )
		// iScheduleIndex is not within valid range for the NPC's current schedule.
		return NULL;

	return &GetCurSchedule()->GetTaskList()[ iScheduleIndex ];
}


//-----------------------------------------------------------------------------

Activity CAI_NPC::TranslateActivity( Activity idealActivity, Activity *pIdealWeaponActivity )
{
	const int MAX_TRIES = 5;
	int count = 0;

	bool bIdealWeaponRequired = false;
	Activity idealWeaponActivity;
	Activity baseTranslation;
	bool bWeaponRequired = false;
	Activity weaponTranslation;
	Activity last;
	Activity current;

	idealWeaponActivity = Weapon_TranslateActivity( idealActivity, &bIdealWeaponRequired );
	if ( pIdealWeaponActivity )
		*pIdealWeaponActivity = idealWeaponActivity;

	baseTranslation	  = idealActivity;
	weaponTranslation = idealActivity;
	last			  = idealActivity;
	while ( count++ < MAX_TRIES )
	{
		current = NPC_TranslateActivity( last );
		if ( current != last )
			baseTranslation = current;

		weaponTranslation = Weapon_TranslateActivity( current, &bWeaponRequired );

		if ( weaponTranslation == last )
			break;

		last = weaponTranslation;
	}
	AssertMsg( count < MAX_TRIES, "Circular activity translation!" );

	if ( last == ACT_SCRIPT_CUSTOM_MOVE )
		return ACT_SCRIPT_CUSTOM_MOVE;
	
	if ( HaveSequenceForActivity( weaponTranslation ) )
		return weaponTranslation;
	
	if ( bWeaponRequired )
	{
		// only complain about an activity once
		static CUtlVector< Activity > sUniqueActivities;

		if (!sUniqueActivities.Find( weaponTranslation))
		{
			sUniqueActivities.AddToTail( weaponTranslation );
		}
	}

	if ( baseTranslation != weaponTranslation && HaveSequenceForActivity( baseTranslation ) )
		return baseTranslation;

	if ( idealWeaponActivity != baseTranslation && HaveSequenceForActivity( idealWeaponActivity ) )
		return idealActivity;

	if ( idealActivity != idealWeaponActivity && HaveSequenceForActivity( idealActivity ) )
		return idealActivity;

	Assert( !HaveSequenceForActivity( idealActivity ) );
	if ( idealActivity == ACT_RUN )
	{
		idealActivity = ACT_WALK;
	}
	else if ( idealActivity == ACT_WALK )
	{
		idealActivity = ACT_RUN;
	}

	return idealActivity;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_NPC::HandleInteraction(int interactionType, void *data, CCombatCharacter* sourceEnt)
{
	//CE_WTF
#ifdef HL2_DLL
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		// Make the victim stop thinking so they're as good as dead without 
		// shocking the system by destroying the entity.
		StopLoopingSounds();
		BarnacleDeathSound();
 		SetThink( NULL );

		// Gag the NPC so they won't talk anymore
		AddSpawnFlags( SF_NPC_GAG );

		// Drop any weapon they're holding
		if ( GetActiveWeapon() )
		{
			Weapon_Drop( GetActiveWeapon() );
		}

		return true;
	}
#endif // HL2_DLL

	return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
}

//-----------------------------------------------------------------------------
// TASK_CLEAR_HINTNODE
//-----------------------------------------------------------------------------
void CAI_NPC::ClearHintNode( float reuseDelay )
{
	if ( m_pHintNode )
	{
		if ( m_pHintNode->IsLockedBy(this) )
			m_pHintNode->Unlock(reuseDelay);
		m_pHintNode.ptr = NULL;
	}
}


void CAI_NPC::SetHintNode( CAI_Hint *pHintNode )
{
	m_pHintNode.ptr = pHintNode;
}


//-----------------------------------------------------------------------------
// Purpose: Written by subclasses macro to load schedules
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CAI_NPC::LoadSchedules(void)
{
	return true;
}

//-----------------------------------------------------------------------------

bool CAI_NPC::LoadedSchedules(void)
{
	return true;
}


void CAI_NPC::AddActivityToSR(const char *actName, int actID)
{
	//g_helpfunc.AddActivityToSR(actName,actID);
	Assert( m_pActivitySR );
	if ( !m_pActivitySR )
		return;

	// technically order isn't dependent, but it's too damn easy to forget to add new ACT_'s to all three lists.

	// NOTE: This assertion generally means that the activity enums are out of order or that new enums were not added to all
	//		 relevant tables.  Make sure that you have included all new enums in:
	//			game_shared/ai_activity.h
	//			game_shared/activitylist.cpp
	//			dlls/ai_activity.cpp
	MEM_ALLOC_CREDIT();

	static int lastActID = -2;
	Assert( actID >= LAST_SHARED_ACTIVITY || actID == lastActID + 1 || actID == ACT_INVALID );
	lastActID = actID;

	m_pActivitySR->AddString(actName, actID);
	m_iNumActivities++;
}

void CAI_NPC::AddEventToSR(const char *eventName, int eventID) 
{
	//g_helpfunc.AddEventToSR(eventName,eventID);
	MEM_ALLOC_CREDIT();
	Assert( m_pEventSR );

	m_pEventSR->AddString( eventName, eventID );
	m_iNumEvents++;
}

//-----------------------------------------------------------------------------
// Purpose: Given and schedule name, return the schedule ID
//-----------------------------------------------------------------------------
int CAI_NPC::GetScheduleID(const char* schedName)
{
	return GetSchedulingSymbols()->ScheduleSymbolToId(schedName);
}

//-----------------------------------------------------------------------------
// Purpose: Given and condition name, return the condition ID
//-----------------------------------------------------------------------------
int CAI_NPC::GetConditionID(const char* condName)
{
	return GetSchedulingSymbols()->ConditionSymbolToId(condName);
}

//-----------------------------------------------------------------------------
// Purpose: Given and activity name, return the activity ID
//-----------------------------------------------------------------------------
int CAI_NPC::GetActivityID(const char* actName) 
{
	Assert( m_pActivitySR );
	if ( !m_pActivitySR )
		return ACT_INVALID;

	return m_pActivitySR->GetStringID(actName);
}

//-----------------------------------------------------------------------------
// Purpose: Given and task name, return the task ID
//-----------------------------------------------------------------------------
int CAI_NPC::GetTaskID(const char* taskName)
{
	return GetSchedulingSymbols()->TaskSymbolToId( taskName );
}

//-----------------------------------------------------------------------------

bool CAI_NPC::OccupyStrategySlotRange( int slotIDStart, int slotIDEnd )
{
	//CE_TODO untest
	// If I'm not in a squad a I don't fill slots
	return ( !GetSquad() || GetSquad()->OccupyStrategySlotRange( GetEnemy(), slotIDStart, slotIDEnd, m_iMySquadSlot.ptr ) );

}

void CAI_NPC::CallNPCThink()
{
	DUMP_FUNCTION();

	g_helpfunc.CallNPCThink(BaseEntity());

	SetThink(NULL);
}

Navigation_t CAI_NPC::GetNavType() const
{
	return GetNavigator()->GetNavType();
}

void CAI_NPC::SetNavType( Navigation_t navType )
{
	GetNavigator()->SetNavType( navType );
}

//-----------------------------------------------------------------------------
bool CAI_NPC::IsCustomInterruptConditionSet( int nCondition )
{
	int interrupt = InterruptFromCondition( nCondition );
	
	if ( interrupt == -1 )
	{
		Assert(0);
		return false;
	}
	
	return m_CustomInterruptConditions->IsBitSet( interrupt );
}

//-----------------------------------------------------------------------------
// Purpose: Sets a flag in the custom interrupt flags, translating the condition
//			to the proper global space, if necessary
//-----------------------------------------------------------------------------
void CAI_NPC::SetCustomInterruptCondition( int nCondition )
{
	int interrupt = InterruptFromCondition( nCondition );
	
	if ( interrupt == -1 )
	{
		Assert(0);
		return;
	}
	
	m_CustomInterruptConditions->Set( interrupt );
}

//-----------------------------------------------------------------------------
// Purpose: Clears a flag in the custom interrupt flags, translating the condition
//			to the proper global space, if necessary
//-----------------------------------------------------------------------------
void CAI_NPC::ClearCustomInterruptCondition( int nCondition )
{
	int interrupt = InterruptFromCondition( nCondition );
	
	if ( interrupt == -1 )
	{
		Assert(0);
		return;
	}
	
	m_CustomInterruptConditions->Clear( interrupt );
}


//-----------------------------------------------------------------------------
// Purpose: Clears all the custom interrupt flags.
//-----------------------------------------------------------------------------
void CAI_NPC::ClearCustomInterruptConditions()
{
	m_CustomInterruptConditions->ClearAll();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CAI_NPC::IsMoving( void ) 
{ 
	return GetNavigator()->IsGoalSet(); 
}


//=========================================================
// ClearSchedule - blanks out the caller's schedule pointer
// and index.
//=========================================================
void CAI_NPC::ClearSchedule( const char *szReason )
{
	m_ScheduleState->timeCurTaskStarted = m_ScheduleState->timeStarted = 0;
	m_ScheduleState->bScheduleWasInterrupted = true;
	SetTaskStatus( TASKSTATUS_NEW );
	m_IdealSchedule = SCHED_NONE;
	m_pSchedule.ptr =  NULL;
	ResetScheduleCurTaskIndex();
	m_InverseIgnoreConditions->SetAll();

}

bool CAI_NPC::IsWaitFinished()
{
	return ( gpGlobals->curtime >= m_flWaitFinished );
}

//-----------------------------------------------------------------------------

float CAI_NPC::SetWait( float minWait, float maxWait )
{
	int minThinks = Ceil2Int( minWait * 10 );

	if ( maxWait == 0.0 )
	{
		m_flWaitFinished = gpGlobals->curtime + ( 0.1 * minThinks );
	}
	else
	{
		if ( minThinks == 0 ) // random 0..n is almost certain to not return 0
			minThinks = 1;
		int maxThinks = Ceil2Int( maxWait * 10 );

		m_flWaitFinished = gpGlobals->curtime + ( 0.1 * random->RandomInt( minThinks, maxThinks ) );
	}
	return m_flWaitFinished;
}







//-------------------------------------------------------

bool NPC_CheckBrushExclude( CEntity *pEntity, CEntity *pBrush )
{
	CAI_NPC *pNPC = pEntity->MyNPCPointer();

	if ( pNPC )
	{
		return pNPC->GetMoveProbe()->ShouldBrushBeIgnored( pBrush->BaseEntity() );
	}

	return false;
}




