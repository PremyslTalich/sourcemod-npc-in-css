
#ifndef _INCLUDE_CAI_NPC_H_
#define _INCLUDE_CAI_NPC_H_

#include "CEntity.h"
#include "CCombatCharacter.h"
#include "CAI_Memory.h"
#include "CAI_Hint.h"
#include "CEAI_ScriptedSequence.h"
#include "CAI_Navigator.h"

#include "ai_npcstate.h"
#include "ai_activity.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_condition.h"
#include "ai_namespaces.h"
#include "ai_schedule.h"
#include "npcevent.h"
#include "ai_movetypes.h"
#include "ai_moveshoot.h"
#include "CAI_motor.h"
#include "ai_activity.h"
#include "CAI_moveprobe.h"
#include "CAI_Squad.h"
#include "ai_squadslot.h"
#include "activitylist.h"
#include "eventlist.h"


class CAI_BehaviorBase;
class CAI_Hint;

//=============================================================================
//
// Constants & enumerations
//
//=============================================================================
#define TURRET_CLOSE_RANGE	200
#define TURRET_MEDIUM_RANGE 500

#define COMMAND_GOAL_TOLERANCE	48	// 48 inches.
#define TIME_CARE_ABOUT_DAMAGE	3.0

#define ITEM_PICKUP_TOLERANCE	48.0f

// Max's of the box used to search for a weapon to pick up. 45x45x~8 ft.
#define WEAPON_SEARCH_DELTA	Vector( 540, 540, 100 )

enum Interruptability_t
{
	GENERAL_INTERRUPTABILITY,
	DAMAGEORDEATH_INTERRUPTABILITY,
	DEATH_INTERRUPTABILITY
};

//-------------------------------------
// Memory
//-------------------------------------

#define MEMORY_CLEAR					0
#define bits_MEMORY_PROVOKED			( 1 << 0 )// right now only used for houndeyes.
#define bits_MEMORY_INCOVER				( 1 << 1 )// npc knows it is in a covered position.
#define bits_MEMORY_SUSPICIOUS			( 1 << 2 )// Ally is suspicious of the player, and will move to provoked more easily
#define	bits_MEMORY_TASK_EXPENSIVE		( 1 << 3 )// NPC has completed a task which is considered costly, so don't do another task this frame
//#define	bits_MEMORY_				( 1 << 4 )
#define bits_MEMORY_PATH_FAILED			( 1 << 5 )// Failed to find a path
#define bits_MEMORY_FLINCHED			( 1 << 6 )// Has already flinched
//#define bits_MEMORY_ 					( 1 << 7 )
#define bits_MEMORY_TOURGUIDE			( 1 << 8 )// I have been acting as a tourguide.
//#define bits_MEMORY_					( 1 << 9 )// 
#define bits_MEMORY_LOCKED_HINT			( 1 << 10 )// 
//#define bits_MEMORY_					( 1 << 12 )

#define bits_MEMORY_TURNING				( 1 << 13 )// Turning, don't interrupt me.
#define bits_MEMORY_TURNHACK			( 1 << 14 )

#define bits_MEMORY_HAD_ENEMY			( 1 << 15 )// Had an enemy
#define bits_MEMORY_HAD_PLAYER			( 1 << 16 )// Had player
#define bits_MEMORY_HAD_LOS				( 1 << 17 )// Had LOS to enemy

#define bits_MEMORY_MOVED_FROM_SPAWN	( 1 << 18 )// Has moved since spawning.

#define bits_MEMORY_CUSTOM4				( 1 << 28 )	// NPC-specific memory
#define bits_MEMORY_CUSTOM3				( 1 << 29 )	// NPC-specific memory
#define bits_MEMORY_CUSTOM2				( 1 << 30 )	// NPC-specific memory
#define bits_MEMORY_CUSTOM1				( 1 << 31 )	// NPC-specific memory

//-------------------------------------
// Spawn flags
//-------------------------------------
#define SF_NPC_WAIT_TILL_SEEN			( 1 << 0  )	// spawnflag that makes npcs wait until player can see them before attacking.
#define SF_NPC_GAG						( 1 << 1  )	// no idle noises from this npc
#define SF_NPC_FALL_TO_GROUND			( 1 << 2  )	// used my NPC_Maker
#define SF_NPC_DROP_HEALTHKIT			( 1 << 3  )	// Drop a healthkit upon death
#define SF_NPC_START_EFFICIENT			( 1 << 4  ) // Set into efficiency mode from spawn
//										( 1 << 5  )
//										( 1 << 6  )
#define SF_NPC_WAIT_FOR_SCRIPT			( 1 << 7  )	// spawnflag that makes npcs wait to check for attacking until the script is done or they've been attacked
#define SF_NPC_LONG_RANGE				( 1 << 8  )	// makes npcs look far and relaxes weapon range limit 
#define SF_NPC_FADE_CORPSE				( 1 << 9  )	// Fade out corpse after death
#define SF_NPC_ALWAYSTHINK				( 1 << 10 )	// Simulate even when player isn't in PVS.
#define SF_NPC_TEMPLATE					( 1 << 11 )	// This NPC will be used as a template by an npc_maker -- do not spawn.
#define SF_NPC_ALTCOLLISION				( 1 << 12 )
#define SF_NPC_NO_WEAPON_DROP			( 1 << 13 )	// This NPC will not actually drop a weapon that can be picked up
#define SF_NPC_NO_PLAYER_PUSHAWAY		( 1 << 14 )	
//										( 1 << 15 )	
// !! Flags above ( 1 << 15 )	 are reserved for NPC sub-classes

//-------------------------------------
//
// Return codes from CanPlaySequence.
//
//-------------------------------------

enum CanPlaySequence_t
{
	CANNOT_PLAY = 0,		// Can't play for any number of reasons.
	CAN_PLAY_NOW,			// Can play the script immediately.
	CAN_PLAY_ENQUEUED,		// Can play the script after I finish playing my current script.
};

//-------------------------------------
// Weapon holstering
//-------------------------------------
enum DesiredWeaponState_t
{
	DESIREDWEAPONSTATE_IGNORE = 0,
	DESIREDWEAPONSTATE_HOLSTERED,
	DESIREDWEAPONSTATE_HOLSTERED_DESTROYED, // Put the weapon away, then destroy it.
	DESIREDWEAPONSTATE_UNHOLSTERED,
	DESIREDWEAPONSTATE_CHANGING,
	DESIREDWEAPONSTATE_CHANGING_DESTROY,	// Destroy the weapon when this change is complete.
};

//-------------------------------------
//
// Efficiency modes
//
//-------------------------------------

enum AI_Efficiency_t
{
	// Run at full tilt
	AIE_NORMAL,

	// Run decision process less often
	AIE_EFFICIENT,

	// Run decision process even less often, ignore other NPCs
	AIE_VERY_EFFICIENT,

	// Run decision process even less often, ignore other NPCs
	AIE_SUPER_EFFICIENT,

	// Don't run at all
	AIE_DORMANT,
};

enum AI_MoveEfficiency_t
{
	AIME_NORMAL,
	AIME_EFFICIENT,
};

//-------------------------------------
//
// Sleep state
//
//-------------------------------------

enum AI_SleepState_t
{
	AISS_AWAKE,
	AISS_WAITING_FOR_THREAT,
	AISS_WAITING_FOR_PVS,
	AISS_WAITING_FOR_INPUT,
	AISS_AUTO_PVS,
	AISS_AUTO_PVS_AFTER_PVS, // Same as AUTO_PVS, except doesn't activate until/unless the NPC is IN the player's PVS. 
};

#define AI_SLEEP_FLAGS_NONE					0x00000000
#define AI_SLEEP_FLAG_AUTO_PVS				0x00000001
#define AI_SLEEP_FLAG_AUTO_PVS_AFTER_PVS	0x00000002


//-------------------------------------
//
// Debug bits
//
//-------------------------------------

enum DebugBaseNPCBits_e
{
	bits_debugDisableAI = 0x00000001,		// disable AI
	bits_debugStepAI	= 0x00000002,		// step AI

};

//-------------------------------------
//
// Base Sentence index for behaviors
//
//-------------------------------------
enum SentenceIndex_t
{
	SENTENCE_BASE_BEHAVIOR_INDEX = 1000,
};

#ifdef AI_MONITOR_FOR_OSCILLATION
struct AIScheduleChoice_t 
{
	float			m_flTimeSelected;
	CAI_Schedule	*m_pScheduleSelected;
};
#endif//AI_MONITOR_FOR_OSCILLATION

#define MARK_TASK_EXPENSIVE()	\
	if ( GetOuter() ) \
	{ \
		GetOuter()->Remember( bits_MEMORY_TASK_EXPENSIVE ); \
	}

//=============================================================================
//
// Types used by CAI_BaseNPC
//
//=============================================================================

struct AIScheduleState_t
{
	int					 iCurTask;
	TaskStatus_e		 fTaskStatus;
	float				 timeStarted;
	float				 timeCurTaskStarted;
	AI_TaskFailureCode_t taskFailureCode;
	int					 iTaskInterrupt;
	bool 				 bTaskRanAutomovement;
	bool 				 bTaskUpdatedYaw;
	bool				 bScheduleWasInterrupted;

	DECLARE_SIMPLE_DATADESC();
};

// -----------------------------------------
//	An entity that this NPC can't reach
// -----------------------------------------

struct UnreachableEnt_t
{
	EHANDLE	hUnreachableEnt;	// Entity that's unreachable
	float	fExpireTime;		// Time to forget this information
	Vector	vLocationWhenUnreachable;
	
	DECLARE_SIMPLE_DATADESC();
};

//=============================================================================
// SCRIPTED NPC INTERACTIONS
//=============================================================================
// -----------------------------------------
//	Scripted NPC interaction flags
// -----------------------------------------
#define SCNPC_FLAG_TEST_OTHER_ANGLES			( 1 << 1 )
#define SCNPC_FLAG_TEST_OTHER_VELOCITY			( 1 << 2 )
#define SCNPC_FLAG_LOOP_IN_ACTION				( 1 << 3 )
#define SCNPC_FLAG_NEEDS_WEAPON_ME				( 1 << 4 )
#define SCNPC_FLAG_NEEDS_WEAPON_THEM			( 1 << 5 )
#define SCNPC_FLAG_DONT_TELEPORT_AT_END_ME		( 1 << 6 )
#define SCNPC_FLAG_DONT_TELEPORT_AT_END_THEM	( 1 << 7 )

// -----------------------------------------
//	Scripted NPC interaction trigger methods
// -----------------------------------------
enum
{
	SNPCINT_CODE = 0,
	SNPCINT_AUTOMATIC_IN_COMBAT = 1,
};

// -----------------------------------------
//	Scripted NPC interaction loop breaking trigger methods
// -----------------------------------------
#define SNPCINT_LOOPBREAK_ON_DAMAGE				( 1 << 1 )
#define SNPCINT_LOOPBREAK_ON_FLASHLIGHT_ILLUM	( 1 << 2 )

// -----------------------------------------
//	Scripted NPC interaction anim phases
// -----------------------------------------
enum
{
	SNPCINT_ENTRY = 0,
	SNPCINT_SEQUENCE,
	SNPCINT_EXIT,

	SNPCINT_NUM_PHASES
};

struct ScriptedNPCInteraction_Phases_t
{
	string_t	iszSequence;
	int			iActivity;

	DECLARE_SIMPLE_DATADESC();
};

// Allowable delta from the desired dynamic scripted sequence point
#define DSS_MAX_DIST			6
#define DSS_MAX_ANGLE_DIFF		4

// Interaction Logic States
enum
{
	NPCINT_NOT_RUNNING = 0,
	NPCINT_RUNNING_ACTIVE,		// I'm in an interaction that I initiated
	NPCINT_RUNNING_PARTNER,		// I'm in an interaction that was initiated by the other NPC
	NPCINT_MOVING_TO_MARK,		// I'm moving to a position to do an interaction
};

#define NPCINT_NONE				-1

#define MAXTACLAT_IGNORE		-1

// -----------------------------------------
//	A scripted interaction between NPCs
// -----------------------------------------
struct ScriptedNPCInteraction_t
{
	ScriptedNPCInteraction_t()
	{
		iszInteractionName = NULL_STRING;
		iFlags = 0;
		iTriggerMethod = SNPCINT_CODE;
		iLoopBreakTriggerMethod = 0;
		vecRelativeOrigin = vec3_origin;
		bValidOnCurrentEnemy = false;
		flDelay = 5.0;
		flDistSqr = (DSS_MAX_DIST * DSS_MAX_DIST);
		flNextAttemptTime = 0;
		iszMyWeapon = NULL_STRING;
		iszTheirWeapon = NULL_STRING;

		for ( int i = 0; i < SNPCINT_NUM_PHASES; i++)
		{
			sPhases[i].iszSequence = NULL_STRING;
			sPhases[i].iActivity = ACT_INVALID;
		}
	}

	// Fill out these when passing to AddScriptedNPCInteraction
	string_t	iszInteractionName;
	int			iFlags;
	int			iTriggerMethod;
	int			iLoopBreakTriggerMethod;
	Vector		vecRelativeOrigin;			// (forward, right, up)
	QAngle		angRelativeAngles;				
	Vector		vecRelativeVelocity;		// Desired relative velocity of the other NPC
	float		flDelay;					// Delay before interaction can be used again
	float		flDistSqr;					// Max distance sqr from the relative origin the NPC is allowed to be to trigger
	string_t	iszMyWeapon;				// Classname of the weapon I'm holding, if any
	string_t	iszTheirWeapon;				// Classname of the weapon my interaction partner is holding, if any
	ScriptedNPCInteraction_Phases_t sPhases[SNPCINT_NUM_PHASES];

	// These will be filled out for you in AddScriptedNPCInteraction
	VMatrix		matDesiredLocalToWorld;		// Desired relative position / angles of the other NPC
	bool		bValidOnCurrentEnemy;

	float		flNextAttemptTime;

	DECLARE_SIMPLE_DATADESC();
};


Vector VecCheckToss ( CEntity *pEdict, Vector vecSpot1, Vector vecSpot2, float flHeightMaxRatio, float flGravityAdj, bool bRandomize, Vector *vecMins = NULL, Vector *vecMaxs = NULL );
Vector VecCheckToss ( CEntity *pEntity, ITraceFilter *pFilter, Vector vecSpot1, Vector vecSpot2, float flHeightMaxRatio, float flGravityAdj, bool bRandomize, Vector *vecMins = NULL, Vector *vecMaxs = NULL );
Vector VecCheckThrow( CEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0f, Vector *vecMins = NULL, Vector *vecMaxs = NULL );




/*
extern CAI_SchedulesManager *my_g_AI_SchedulesManager;
extern CAI_GlobalScheduleNamespace *my_gm_SchedulingSymbols;
extern CAI_ClassScheduleIdSpace *my_gm_ClassScheduleIdSpace;
extern CAI_LocalIdSpace    *my_gm_SquadSlotIdSpace;
extern CAI_GlobalNamespace *my_gm_SquadSlotNamespace;*/



class CAI_NPC : public CCombatCharacter
{
public:
	CE_DECLARE_CLASS(CAI_NPC, CCombatCharacter);
	
	CAI_NPC();

	void		SetHullSizeNormal(bool force=false);

	void		CapabilitiesClear();
	int			CapabilitiesAdd(int capability);
	int			CapabilitiesRemove(int capability);

	void		SetCondition(int iCondition);
	bool		HasCondition( int iCondition );
	void		ClearCondition( int iCondition );

	CEntity		*GetEnemy()									{ return m_hEnemy; }
	bool		AutoMovement(CEntity *pTarget = NULL, AIMoveTrace_t *pTraceResult = NULL);
	bool		AutoMovement( float flInterval, CEntity *pTarget = NULL, AIMoveTrace_t *pTraceResult = NULL );
	
	void		TaskComplete(  bool fIgnoreSetFailedCondition = false);
	void 		SetTaskStatus( TaskStatus_e status )	{ m_ScheduleState->fTaskStatus = status; 	}

	CAI_MoveProbe *		GetMoveProbe() 				{ return m_pMoveProbe; }
	const CAI_MoveProbe *GetMoveProbe() const		{ return m_pMoveProbe; }

	CAI_Motor *			GetMotor() 					{ return m_pMotor; }
	const CAI_Motor *	GetMotor() const			{ return m_pMotor; }

	CAI_Hint			*GetHintNode()				{ return m_pHintNode; }
	const CAI_Hint		*GetHintNode() const		{ return m_pHintNode; }

	const Vector &		GetHullMins() const		{ return NAI_Hull::Mins(GetHullType()); }
	const Vector &		GetHullMaxs() const		{ return NAI_Hull::Maxs(GetHullType()); }
	float				GetHullWidth()	const	{ return NAI_Hull::Width(GetHullType()); }
	float				GetHullHeight() const	{ return NAI_Hull::Height(GetHullType()); }

	Activity			GetActivity( void ) { return m_Activity; }

	string_t			GetHintGroup( void )			{ return m_strHintGroup; }

	NPC_STATE			GetState( void )				{ return m_NPCState; }

	AI_SleepState_t		GetSleepState() const			{ return m_SleepState; }

	void SetIdealActivity( Activity NewActivity );
	bool IsCurSchedule( int schedId, bool fIdeal = true );

	inline bool SetSchedule(int localScheduleID) { return SetSchedule_Int(localScheduleID); }
	
	bool HaveSequenceForActivity( Activity activity );

	Activity TranslateActivity( Activity idealActivity, Activity *pIdealWeaponActivity = NULL );
	Activity GetIdealActivity() { return m_IdealActivity; }

	void SetHintNode( CAI_Hint *pHintNode );
	void ClearHintNode( float reuseDelay = 0.0 );

	CAI_Navigator *		GetNavigator() 				{ return m_pNavigator; }
	const CAI_Navigator *GetNavigator() const 		{ return m_pNavigator; }

	CAI_Squad *			GetSquad() 				{ return m_pSquad; }
	const CAI_Squad		*GetSquad() const 		{ return m_pSquad; }

	
	bool	FacingIdeal();

	int 				GetScheduleCurTaskIndex() const			{ return m_ScheduleState.ptr->iCurTask;	}
	CAI_Schedule *		GetCurSchedule()						{ return m_pSchedule; }

	const Task_t*		GetTask( void );


	static CAI_GlobalScheduleNamespace *GetSchedulingSymbols()		{ return &gm_SchedulingSymbols; }
	static CAI_ClassScheduleIdSpace &AccessClassScheduleIdSpaceDirect() { return gm_ClassScheduleIdSpace; }

	
	bool OccupyStrategySlotRange( int slotIDStart, int slotIDEnd );
	
	Navigation_t		GetNavType() const;
	void				SetNavType( Navigation_t navType );


	inline void			Remember( int iMemory ) 		{ m_afMemory |= iMemory; }
	inline void			Forget( int iMemory ) 			{ m_afMemory &= ~iMemory; }
	inline bool			HasMemory( int iMemory ) 		{ if ( m_afMemory & iMemory ) return TRUE; return FALSE; }
	inline bool			HasAllMemories( int iMemory ) 	{ if ( (m_afMemory & iMemory) == iMemory ) return TRUE; return FALSE; }

	void				SetCustomInterruptCondition( int nCondition );
	bool				IsCustomInterruptConditionSet( int nCondition );
	void				ClearCustomInterruptCondition( int nCondition );
	void				ClearCustomInterruptConditions( void );

	inline void			SetIdealState( NPC_STATE eIdealState );	
	inline NPC_STATE	GetIdealState();

	bool				IsMoving( void );

	void				ClearSchedule( const char *szReason );

	void				ResetScheduleCurTaskIndex();

	bool				IsWaitFinished();
	float				SetWait( float minWait, float maxWait = 0.0 );

public:
	virtual CEAI_Enemies *GetEnemies();
	virtual void NPCInit();
	virtual void SetState(NPC_STATE State);
	virtual void SetActivity(Activity NewActivity);
	virtual bool SetSchedule_Int(int localScheduleID);
	virtual void RunTask( const Task_t *pTask );
	virtual void OnChangeActivity(Activity eNewActivity);
	virtual float MaxYawSpeed();
	virtual CAI_ClassScheduleIdSpace *GetClassScheduleIdSpace();
	virtual void NPCThink();
	virtual void HandleAnimEvent(animevent_t *pEvent);
	virtual bool IsActivityFinished();
	virtual float CalcIdealYaw( const Vector &vecTarget );
	virtual void GatherConditions();
	virtual void PrescheduleThink();
	virtual void IdleSound();
	virtual void AlertSound();
	virtual void PainSound(const CTakeDamageInfo &info);
	virtual void DeathSound(const CTakeDamageInfo &info);
	virtual bool FValidateHintType(CAI_Hint *pHint);
	virtual bool FInAimCone_Vector(const Vector &vecSpot);
	virtual CBaseEntity *BestEnemy();
	void TaskFail(AI_TaskFailureCode_t code);
	void TaskFail(const char *pszGeneralFailText) { TaskFail( MakeFailCode( pszGeneralFailText ) ); }
	virtual void StartTask( const Task_t *pTask );
	virtual bool Event_Gibbed( const CTakeDamageInfo &info );
	virtual int	TranslateSchedule( int scheduleType );
	virtual int	SelectSchedule();
	virtual int	SelectFailSchedule( int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode );
	virtual Activity Weapon_TranslateActivity( Activity baseAct, bool *pRequired = NULL );
	virtual Activity NPC_TranslateActivity( Activity eNewActivity );
	virtual bool HandleInteraction(int interactionType, void *data, CCombatCharacter* sourceEnt);
	virtual CAI_Schedule *GetSchedule(int schedule);
	virtual void PlayerHasIlluminatedNPC(CBaseEntity *pPlayer, float flDot );
	virtual bool QuerySeeEntity( CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC = false );
	virtual int	GetSoundInterests();
	virtual void BuildScheduleTestBits( void );
	virtual bool UpdateEnemyMemory( CBaseEntity *pEnemy, const Vector &position, CBaseEntity *pInformer = NULL);
	virtual	bool OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	virtual float GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
	virtual bool OnBehaviorChangeStatus(  CAI_BehaviorBase *pBehavior, bool fCanFinishSchedule );
	virtual bool IsInterruptable();


public: // sign
	void CallNPCThink();
	void SetEnemy(CBaseEntity *pEnemy, bool bSetCondNewEnemy = true);

public:
	virtual bool CanBeAnEnemyOf( CEntity *pEnemy );

protected:
	static bool			LoadSchedules(void);
	virtual bool		LoadedSchedules(void);

public:
	static void			AddActivityToSR(const char *actName, int conID);
	
	static void			AddEventToSR(const char *eventName, int conID);
	static const char*	GetEventName	(int actID);
	static int			GetEventID	(const char* actName);
	
	static int			GetScheduleID	(const char* schedName);
	static int			GetActivityID	(const char* actName);
	static int			GetConditionID	(const char* condName);
	static int			GetTaskID		(const char* taskName);
	static int			GetSquadSlotID	(const char* slotName);

public:
	static void InitSchedulingTables();

	/*static CAI_GlobalNamespace gm_SquadSlotNamespace; // use the pointer
	static CAI_LocalIdSpace    gm_SquadSlotIdSpace; // use the pointer
	static CAI_ClassScheduleIdSpace		gm_ClassScheduleIdSpace;*/

	static CAI_GlobalScheduleNamespace	gm_SchedulingSymbols; // need acturl pointer?
	static CAI_ClassScheduleIdSpace		gm_ClassScheduleIdSpace;

protected:
	static CAI_GlobalNamespace gm_SquadSlotNamespace;
	static CAI_LocalIdSpace    gm_SquadSlotIdSpace;

private:
	static bool			LoadDefaultSchedules(void);

	static void			InitDefaultScheduleSR(void);
	static void			InitDefaultTaskSR(void);
	static void			InitDefaultConditionSR(void);
	static void			InitDefaultActivitySR(void);
	static void			InitDefaultSquadSlotSR(void);

	static CStringRegistry*	m_pActivitySR;
	static int			m_iNumActivities;

	static CStringRegistry*	m_pEventSR;
	static int			m_iNumEvents;

public:
	DECLARE_DEFAULTHEADER_DETOUR(SetHullSizeNormal, void, (bool force));
	DECLARE_DEFAULTHEADER(GetEnemies, CEAI_Enemies *, ());
	DECLARE_DEFAULTHEADER(NPCInit, void, ());
	DECLARE_DEFAULTHEADER_DETOUR(SetState, void, (NPC_STATE State));
	DECLARE_DEFAULTHEADER(SetActivity, void, (Activity NewActivity));
	DECLARE_DEFAULTHEADER(RunTask, void, (const Task_t *pTask));
	DECLARE_DEFAULTHEADER(OnChangeActivity, void, (Activity eNewActivity));
	DECLARE_DEFAULTHEADER(MaxYawSpeed, float, ());
	DECLARE_DEFAULTHEADER(GetClassScheduleIdSpace, CAI_ClassScheduleIdSpace *, ());
	DECLARE_DEFAULTHEADER(NPCThink, void, ());
	DECLARE_DEFAULTHEADER(HandleAnimEvent, void, (animevent_t *pEvent));
	DECLARE_DEFAULTHEADER(IsActivityFinished, bool, ());
	DECLARE_DEFAULTHEADER(CalcIdealYaw, float, (const Vector &vecTarget));
	DECLARE_DEFAULTHEADER(GatherConditions, void, ());
	DECLARE_DEFAULTHEADER(PrescheduleThink, void, ());
	DECLARE_DEFAULTHEADER(IdleSound, void, ());
	DECLARE_DEFAULTHEADER(AlertSound, void, ());
	DECLARE_DEFAULTHEADER(PainSound, void, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(DeathSound, void, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(FValidateHintType, bool, (CAI_Hint *pHint));
	DECLARE_DEFAULTHEADER(FInAimCone_Vector, bool, (const Vector &vecSpot));
	DECLARE_DEFAULTHEADER(BestEnemy, CBaseEntity *, ());
	DECLARE_DEFAULTHEADER(TaskFail, void, (AI_TaskFailureCode_t code));
	DECLARE_DEFAULTHEADER(StartTask, void, (const Task_t *pTask));
	DECLARE_DEFAULTHEADER(Event_Gibbed, bool, (const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(TranslateSchedule, int, (int scheduleType));
	DECLARE_DEFAULTHEADER(SelectSchedule, int, ());
	DECLARE_DEFAULTHEADER(SelectFailSchedule, int, (int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode));
	DECLARE_DEFAULTHEADER(Weapon_TranslateActivity, Activity, (Activity baseAct, bool *pRequired));
	DECLARE_DEFAULTHEADER(NPC_TranslateActivity, Activity, (Activity eNewActivity));
	DECLARE_DEFAULTHEADER(GetSchedule, CAI_Schedule *, (int schedule));
	DECLARE_DEFAULTHEADER(TaskName, const char *, (int taskID));
	DECLARE_DEFAULTHEADER(PlayerHasIlluminatedNPC, void, (CBaseEntity *pPlayer, float flDot));
	DECLARE_DEFAULTHEADER(QuerySeeEntity, bool, (CBaseEntity *pEntity, bool bOnlyHateOrFearIfNPC));
	DECLARE_DEFAULTHEADER(GetSoundInterests, int, ());
	DECLARE_DEFAULTHEADER(BuildScheduleTestBits, void, ());
	DECLARE_DEFAULTHEADER(UpdateEnemyMemory, bool, (CBaseEntity *pEnemy, const Vector &position, CBaseEntity *pInformer));
	DECLARE_DEFAULTHEADER(OverrideMoveFacing, bool, (const AILocalMoveGoal_t &move, float flInterval));
	DECLARE_DEFAULTHEADER(GetHitgroupDamageMultiplier , float, (int iHitGroup, const CTakeDamageInfo &info));
	DECLARE_DEFAULTHEADER(OnBehaviorChangeStatus, bool, (CAI_BehaviorBase *pBehavior, bool fCanFinishSchedule));
	DECLARE_DEFAULTHEADER(IsInterruptable, bool, ());


public:
	DECLARE_DEFAULTHEADER_DETOUR(SetSchedule_Int, bool, (int localScheduleID));
	DECLARE_DEFAULTHEADER_DETOUR(SetEnemy, void, (CBaseEntity *pEnemy, bool bSetCondNewEnemy));


protected: // sign
	void EndTaskOverlay();


protected:
	bool IsRunningDynamicInteraction( void ) { return (m_iInteractionState != NPCINT_NOT_RUNNING && (m_hCine != NULL)); }


protected: //Sendprops


protected: //Datamaps
	DECLARE_DATAMAP(NPC_STATE, m_NPCState);
	DECLARE_DATAMAP(int, m_afCapability);
	DECLARE_DATAMAP_OFFSET(CAI_ScheduleBits, m_Conditions,0x834);
	DECLARE_DATAMAP(CFakeHandle, m_hEnemy);
	DECLARE_DATAMAP(AIScheduleState_t, m_ScheduleState);
	DECLARE_DATAMAP(CAI_MoveAndShootOverlay, m_MoveAndShootOverlay);
	DECLARE_DATAMAP(CAI_Motor *, m_pMotor);
	DECLARE_DATAMAP(float, m_flNextFlinchTime);
	DECLARE_DATAMAP(CAI_MoveProbe *, m_pMoveProbe);
	DECLARE_DATAMAP(Activity, m_Activity);
	DECLARE_DATAMAP(string_t, m_strHintGroup);
	DECLARE_DATAMAP(CAI_Hint, m_pHintNode);
	DECLARE_DATAMAP(AI_SleepState_t, m_SleepState);
	DECLARE_DATAMAP_OFFSET(CAI_Schedule, m_pSchedule, 0x7F0); // "Schedule cleared: %s\"
	DECLARE_DATAMAP(int, m_IdealSchedule);
	DECLARE_DATAMAP(int, m_iInteractionState);
	DECLARE_DATAMAP(Activity, m_IdealActivity);
	DECLARE_DATAMAP(CAI_Navigator *, m_pNavigator);
	DECLARE_DATAMAP_OFFSET(CAI_Squad *, m_pSquad,0xA80); // "Found %s that isn't in a squad\n"
	DECLARE_DATAMAP(int, m_iMySquadSlot);
	DECLARE_DATAMAP(int, m_afMemory);
	DECLARE_DATAMAP_OFFSET(CAI_ScheduleBits, m_CustomInterruptConditions,0x854);
	DECLARE_DATAMAP(NPC_STATE, m_IdealNPCState);
	DECLARE_DATAMAP_OFFSET(int, m_poseMove_Yaw,0x830); // move_yaw
	DECLARE_DATAMAP_OFFSET(CAI_ScheduleBits, m_InverseIgnoreConditions,0x894);
	DECLARE_DATAMAP(float, m_flWaitFinished);

public:
	DECLARE_DATAMAP(CEAI_ScriptedSequence, m_hCine);

	friend class CAI_SchedulesManager;
};



inline bool	CAI_NPC::HaveSequenceForActivity( Activity activity )				
{
	return g_helpfunc.HaveSequenceForActivity(BaseEntity(), activity);
}

//-----------------------------------------------------------------------------
// Purpose: Sets the ideal state of this NPC.
//-----------------------------------------------------------------------------
inline void	CAI_NPC::SetIdealState( NPC_STATE eIdealState )
{
	if (eIdealState != m_IdealNPCState)
	{
		m_IdealNPCState = eIdealState;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns the current ideal state the NPC will try to achieve.
//-----------------------------------------------------------------------------
inline NPC_STATE CAI_NPC::GetIdealState()
{
	return m_IdealNPCState;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline void CAI_NPC::ResetScheduleCurTaskIndex()
{
	m_ScheduleState->iCurTask = 0;
	m_ScheduleState->iTaskInterrupt = 0;
	m_ScheduleState->bTaskRanAutomovement = false;
	m_ScheduleState->bTaskUpdatedYaw = false;
}

inline const QAngle &CAI_Component::GetLocalAngles( void ) const
{
	return CEntity::Instance((CBaseEntity *)GetOuter())->GetLocalAngles();
}

inline const Vector &CAI_Component::GetLocalOrigin() const
{
	return CEntity::Instance((CBaseEntity *)GetOuter())->GetLocalOrigin();
}

//-----------------------------------------------------------------------------

inline void CAI_Component::TaskComplete( bool fIgnoreSetFailedCondition )
{
	//CAI_NPC *npc = (CAI_NPC *)CEntity::Instance((CBaseEntity *)GetOuter());	
	//GetOuter()->TaskComplete( fIgnoreSetFailedCondition );
}

inline void CAI_Component::TaskFail( AI_TaskFailureCode_t code )
{
	//CAI_NPC *npc = (CAI_NPC *)CEntity::Instance((CBaseEntity *)GetOuter());	
	//GetOuter()->TaskFail( code );
}

//-------------------------------------

struct AI_NamespaceAddInfo_t
{
	AI_NamespaceAddInfo_t( const char *pszName, int localId )
	 :	pszName( pszName ),
		localId( localId )
	{
	}
	
	const char *pszName;
	int			localId;
};


class CAI_NamespaceInfos : public CUtlVector<AI_NamespaceAddInfo_t>
{
public:
	void PushBack(  const char *pszName, int localId )
	{
		AddToTail( AI_NamespaceAddInfo_t( pszName, localId ) );
	}

	void Sort()
	{
		CUtlVector<AI_NamespaceAddInfo_t>::Sort( Compare );
	}
	
private:
	static int __cdecl Compare(const AI_NamespaceAddInfo_t *pLeft, const AI_NamespaceAddInfo_t *pRight )
	{
		return pLeft->localId - pRight->localId;
	}
	
};


//-------------------------------------

// Declares the static variables that hold the string registry offset for the new subclass
// as well as the initialization in schedule load functions

struct AI_SchedLoadStatus_t
{
	bool fValid;
	int  signature;
};


// Load schedules pulled out to support stepping through with debugger
inline bool AI_DoLoadSchedules( bool (*pfnBaseLoad)(), void (*pfnInitCustomSchedules)(),
								AI_SchedLoadStatus_t *pLoadStatus )
{
	(*pfnBaseLoad)();
	
	if (pLoadStatus->signature != g_AI_SchedulesManager.GetScheduleLoadSignature())
	{
		(*pfnInitCustomSchedules)();
		pLoadStatus->fValid	   = true;
		pLoadStatus->signature = g_AI_SchedulesManager.GetScheduleLoadSignature();
	}
	return pLoadStatus->fValid;
}


//-------------------------------------


typedef bool (*AIScheduleLoadFunc_t)();






//-------------------------------------

#define ADD_CUSTOM_SCHEDULE_NAMED(derivedClass,schedName,schedEN)\
	if ( !derivedClass::AccessClassScheduleIdSpaceDirect().AddSchedule( schedName, schedEN, derivedClass::gm_pszErrorClassName ) ) return;

#define ADD_CUSTOM_SCHEDULE(derivedClass,schedEN) ADD_CUSTOM_SCHEDULE_NAMED(derivedClass,#schedEN,schedEN)

#define ADD_CUSTOM_TASK_NAMED(derivedClass,taskName,taskEN)\
	if ( !derivedClass::AccessClassScheduleIdSpaceDirect().AddTask( taskName, taskEN, derivedClass::gm_pszErrorClassName ) ) return;

#define ADD_CUSTOM_TASK(derivedClass,taskEN) ADD_CUSTOM_TASK_NAMED(derivedClass,#taskEN,taskEN)

#define ADD_CUSTOM_CONDITION_NAMED(derivedClass,condName,condEN)\
	if ( !derivedClass::AccessClassScheduleIdSpaceDirect().AddCondition( condName, condEN, derivedClass::gm_pszErrorClassName ) ) return;

#define ADD_CUSTOM_CONDITION(derivedClass,condEN) ADD_CUSTOM_CONDITION_NAMED(derivedClass,#condEN,condEN)

//-------------------------------------

#define INIT_CUSTOM_AI(derivedClass)\
	derivedClass::AccessClassScheduleIdSpaceDirect().Init( #derivedClass, BaseClass::GetSchedulingSymbols(), &BaseClass::AccessClassScheduleIdSpaceDirect() ); \
	derivedClass::gm_SquadSlotIdSpace.Init( &CAI_BaseNPC::gm_SquadSlotNamespace, &BaseClass::gm_SquadSlotIdSpace);

#define	ADD_CUSTOM_INTERACTION( interaction )	{ interaction = CBaseCombatCharacter::GetInteractionID(); }

#define ADD_CUSTOM_SQUADSLOT_NAMED(derivedClass,squadSlotName,squadSlotEN)\
	if ( !derivedClass::gm_SquadSlotIdSpace.AddSymbol( squadSlotName, squadSlotEN, "squadslot", derivedClass::gm_pszErrorClassName ) ) return;

#define ADD_CUSTOM_SQUADSLOT(derivedClass,squadSlotEN) ADD_CUSTOM_SQUADSLOT_NAMED(derivedClass,#squadSlotEN,squadSlotEN)

#define ADD_CUSTOM_ACTIVITY_NAMED(derivedClass,activityName,activityEnum)\
	REGISTER_PRIVATE_ACTIVITY(activityEnum);\
	CAI_NPC::AddActivityToSR(activityName,activityEnum);

#define ADD_CUSTOM_ACTIVITY(derivedClass,activityEnum) ADD_CUSTOM_ACTIVITY_NAMED(derivedClass,#activityEnum,activityEnum)


#define ADD_CUSTOM_ANIMEVENT_NAMED(derivedClass,eventName,eventEnum)\
	REGISTER_PRIVATE_ANIMEVENT(eventEnum);\
	CAI_NPC::AddEventToSR(eventName,eventEnum);

#define ADD_CUSTOM_ANIMEVENT(derivedClass,eventEnum) ADD_CUSTOM_ANIMEVENT_NAMED(derivedClass,#eventEnum,eventEnum)




//-------------------------------------

#define DEFINE_CUSTOM_SCHEDULE_PROVIDER\
	static AI_SchedLoadStatus_t 		gm_SchedLoadStatus; \
	static CAI_ClassScheduleIdSpace 	gm_ClassScheduleIdSpace; \
	static const char *					gm_pszErrorClassName;\
	\
	static CAI_ClassScheduleIdSpace &AccessClassScheduleIdSpaceDirect() 	{ return gm_ClassScheduleIdSpace; } \
	virtual CAI_ClassScheduleIdSpace *	GetClassScheduleIdSpace()			{ return &gm_ClassScheduleIdSpace; } \
	virtual const char *				GetSchedulingErrorName()			{ return gm_pszErrorClassName; } \
	\
	static void							InitCustomSchedules(void);\
	\
	static bool							LoadSchedules(void);\
	virtual bool						LoadedSchedules(void); \
	\
	friend class ScheduleLoadHelperImpl;	\
	\
	class CScheduleLoader \
	{ \
	public: \
		CScheduleLoader(); \
	} m_ScheduleLoader; \
	\
	friend class CScheduleLoader;


//-------------------------------------

#define DEFINE_CUSTOM_AI\
	DEFINE_CUSTOM_SCHEDULE_PROVIDER \
	\
	static CAI_LocalIdSpace gm_SquadSlotIdSpace; \
	\
	const char*				SquadSlotName	(int squadSlotID);


//-------------------------------------

#define IMPLEMENT_CUSTOM_SCHEDULE_PROVIDER(derivedClass)\
	AI_SchedLoadStatus_t		derivedClass::gm_SchedLoadStatus = { true, -1 }; \
	CAI_ClassScheduleIdSpace 	derivedClass::gm_ClassScheduleIdSpace; \
	const char *				derivedClass::gm_pszErrorClassName = #derivedClass; \
	\
	derivedClass::CScheduleLoader::CScheduleLoader()\
	{ \
		derivedClass::LoadSchedules(); \
	} \
	\
	/* --------------------------------------------- */ \
	/* Load schedules for this type of NPC           */ \
	/* --------------------------------------------- */ \
	bool derivedClass::LoadSchedules(void)\
	{\
		return AI_DoLoadSchedules( derivedClass::BaseClass::LoadSchedules, \
								   derivedClass::InitCustomSchedules, \
								   &derivedClass::gm_SchedLoadStatus ); \
	}\
	\
	bool derivedClass::LoadedSchedules(void) \
	{ \
		return derivedClass::gm_SchedLoadStatus.fValid;\
	} 


//-------------------------------------

// Initialize offsets and implement methods for loading and getting squad info for the subclass
#define IMPLEMENT_CUSTOM_AI(className, derivedClass)\
	IMPLEMENT_CUSTOM_SCHEDULE_PROVIDER(derivedClass)\
	\
	CAI_LocalIdSpace 	derivedClass::gm_SquadSlotIdSpace; \
	\
	/* -------------------------------------------------- */ \
	/* Given squadSlot enumeration return squadSlot name */ \
	/* -------------------------------------------------- */ \
	const char* derivedClass::SquadSlotName(int slotEN)\
	{\
		return "";\
	}


#define AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( derivedClass ) \
	IMPLEMENT_CUSTOM_SCHEDULE_PROVIDER(derivedClass ) \
	void derivedClass::InitCustomSchedules( void ) \
	{ \
		typedef derivedClass CNpc; \
		const char *pszClassName = #derivedClass; \
		\
		CUtlVector<char *> schedulesToLoad; \
		CUtlVector<AIScheduleLoadFunc_t> reqiredOthers; \
		CAI_NamespaceInfos scheduleIds; \
		CAI_NamespaceInfos taskIds; \
		CAI_NamespaceInfos conditionIds;
		

//-----------------

#define AI_BEGIN_CUSTOM_NPC( className, derivedClass ) \
	IMPLEMENT_CUSTOM_AI(className, derivedClass ) \
	void derivedClass::InitCustomSchedules( void ) \
	{ \
		typedef derivedClass CNpc; \
		const char *pszClassName = #derivedClass; \
		\
		CUtlVector<char *> schedulesToLoad; \
		CUtlVector<AIScheduleLoadFunc_t> reqiredOthers; \
		CAI_NamespaceInfos scheduleIds; \
		CAI_NamespaceInfos taskIds; \
		CAI_NamespaceInfos conditionIds; \
		CAI_NamespaceInfos squadSlotIds;
		



//-------------------------------------

// IDs are stored and then added in order due to constraints in the namespace implementation
#define AI_END_CUSTOM_NPC() \
		\
		int i; \
		\
		CNpc::AccessClassScheduleIdSpaceDirect().Init( pszClassName, BaseClass::GetSchedulingSymbols(), &BaseClass::AccessClassScheduleIdSpaceDirect() ); \
		CNpc::gm_SquadSlotIdSpace.Init( &BaseClass::gm_SquadSlotNamespace, &BaseClass::gm_SquadSlotIdSpace); \
		\
		scheduleIds.Sort(); \
		taskIds.Sort(); \
		conditionIds.Sort(); \
		squadSlotIds.Sort(); \
		\
		for ( i = 0; i < scheduleIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_SCHEDULE_NAMED( CNpc, scheduleIds[i].pszName, scheduleIds[i].localId );  \
		} \
		\
		for ( i = 0; i < taskIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_TASK_NAMED( CNpc, taskIds[i].pszName, taskIds[i].localId );  \
		} \
		\
		for ( i = 0; i < conditionIds.Count(); i++ ) \
		{ \
			if ( ValidateConditionLimits( conditionIds[i].pszName ) ) \
			{ \
				ADD_CUSTOM_CONDITION_NAMED( CNpc, conditionIds[i].pszName, conditionIds[i].localId );  \
			} \
		} \
		\
		for ( i = 0; i < squadSlotIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_SQUADSLOT_NAMED( CNpc, squadSlotIds[i].pszName, squadSlotIds[i].localId );  \
		} \
		\
		for ( i = 0; i < reqiredOthers.Count(); i++ ) \
		{ \
			(*reqiredOthers[i])();  \
		} \
		\
		for ( i = 0; i < schedulesToLoad.Count(); i++ ) \
		{ \
			if ( CNpc::gm_SchedLoadStatus.fValid ) \
			{ \
				CNpc::gm_SchedLoadStatus.fValid = g_AI_SchedulesManager.LoadSchedulesFromBuffer( pszClassName, schedulesToLoad[i], &AccessClassScheduleIdSpaceDirect() ); \
			} \
			else \
				break; \
		} \
	}

//-------------------------------------


// IDs are stored and then added in order due to constraints in the namespace implementation
#define AI_END_CUSTOM_SCHEDULE_PROVIDER() \
		\
		int i; \
		\
		CNpc::AccessClassScheduleIdSpaceDirect().Init( pszClassName, BaseClass::GetSchedulingSymbols(), &BaseClass::AccessClassScheduleIdSpaceDirect() ); \
		\
		scheduleIds.Sort(); \
		taskIds.Sort(); \
		conditionIds.Sort(); \
		\
		for ( i = 0; i < scheduleIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_SCHEDULE_NAMED( CNpc, scheduleIds[i].pszName, scheduleIds[i].localId );  \
		} \
		\
		for ( i = 0; i < taskIds.Count(); i++ ) \
		{ \
			ADD_CUSTOM_TASK_NAMED( CNpc, taskIds[i].pszName, taskIds[i].localId );  \
		} \
		\
		for ( i = 0; i < conditionIds.Count(); i++ ) \
		{ \
			if ( ValidateConditionLimits( conditionIds[i].pszName ) ) \
			{ \
				ADD_CUSTOM_CONDITION_NAMED( CNpc, conditionIds[i].pszName, conditionIds[i].localId );  \
			} \
		} \
		\
		for ( i = 0; i < reqiredOthers.Count(); i++ ) \
		{ \
			(*reqiredOthers[i])();  \
		} \
		\
		for ( i = 0; i < schedulesToLoad.Count(); i++ ) \
		{ \
			if ( CNpc::gm_SchedLoadStatus.fValid ) \
			{ \
				CNpc::gm_SchedLoadStatus.fValid = g_AI_SchedulesManager.LoadSchedulesFromBuffer( pszClassName, schedulesToLoad[i], &AccessClassScheduleIdSpaceDirect() ); \
			} \
			else \
				break; \
		} \
	}


inline bool ValidateConditionLimits( const char *pszNewCondition )
{
	int nGlobalConditions = CAI_NPC::GetSchedulingSymbols()->NumConditions();
	if ( nGlobalConditions >= MAX_CONDITIONS )
	{ 
		AssertMsg2( 0, "Exceeded max number of conditions (%d), ignoring condition %s\n", MAX_CONDITIONS, pszNewCondition ); 
		DevWarning( "Exceeded max number of conditions (%d), ignoring condition %s\n", MAX_CONDITIONS, pszNewCondition ); 
		return false;
	}
	return true;
}


		
//-----------------

#define EXTERN_SCHEDULE( id ) \
	scheduleIds.PushBack( #id, id ); \
	extern const char * g_psz##id; \
	schedulesToLoad.AddToTail( (char *)g_psz##id );

//-----------------

#define DEFINE_SCHEDULE( id, text ) \
	scheduleIds.PushBack( #id, id ); \
	const char * g_psz##id = \
			"\n	Schedule" \
			"\n		" #id \
			text \
			"\n"; \
	schedulesToLoad.AddToTail( (char *)g_psz##id );
	
//-----------------

#define DECLARE_CONDITION( id ) \
	conditionIds.PushBack( #id, id );

//-----------------

#define DECLARE_TASK( id ) \
	taskIds.PushBack( #id, id );

//-----------------

#define DECLARE_ACTIVITY( id ) \
	ADD_CUSTOM_ACTIVITY( CNpc, id );

//-----------------

#define DECLARE_SQUADSLOT( id ) \
	squadSlotIds.PushBack( #id, id );

//-----------------

#define DECLARE_INTERACTION( interaction ) \
	ADD_CUSTOM_INTERACTION( interaction );

//-----------------

#define DECLARE_ANIMEVENT( id ) \
	ADD_CUSTOM_ANIMEVENT( CNpc, id );

//-----------------

#define DECLARE_USES_SCHEDULE_PROVIDER( classname )	reqiredOthers.AddToTail( ScheduleLoadHelper(classname) );

//-----------------




#endif

