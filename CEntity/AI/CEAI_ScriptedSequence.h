#ifndef _INCLUDE_CEAI_SCRIPTEDSEQUENCE_H_
#define _INCLUDE_CEAI_SCRIPTEDSEQUENCE_H_

#include "CEntity.h"
#include "CAI_NPC.h"

#define MAX_SCRIPT_EVENTS				8



class CEAI_ScriptedSequence : public CEntity
{
public:
	CE_DECLARE_CLASS( CEAI_ScriptedSequence, CEntity );

public:
	void Spawn( void );
	virtual void Blocked( CBaseEntity *pOther );
	virtual void Touch( CBaseEntity *pOther );
	virtual int	 ObjectCaps( void ) { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	virtual void Activate( void );
	virtual void UpdateOnRemove( void );
	void StartThink();
	void ScriptThink( void );
	void StopThink();

	DECLARE_DATADESC();

	void Pain( void );
	void Die( void );
	void DelayStart( bool bDelay );
	bool FindEntity( void );
	void StartScript( void );
	void FireScriptEvent( int nEvent );
	void OnBeginSequence( void );

	void SetTarget( CBaseEntity *pTarget ) { m_hTargetEnt = pTarget; };
	CBaseEntity *GetTarget( void ) { return m_hTargetEnt; };

	// Input handlers
	void InputBeginSequence( inputdata_t &inputdata );
	void InputCancelSequence( inputdata_t &inputdata );
	void InputMoveToPosition( inputdata_t &inputdata );

	bool IsTimeToStart( void );
	bool IsWaitingForBegin( void );
	void ReleaseEntity( CAI_NPC *pEntity );
	void CancelScript( void );
	bool StartSequence( CAI_NPC *pTarget, string_t iszSeq, bool completeOnEmpty );
	void SynchronizeSequence( CAI_NPC *pNPC );
	bool FCanOverrideState ( void );
	void SequenceDone( CAI_NPC *pNPC );
	void PostIdleDone( CAI_NPC *pNPC );
	void FixScriptNPCSchedule( CAI_NPC *pNPC, int iSavedCineFlags );
	void FixFlyFlag( CAI_NPC *pNPC, int iSavedCineFlags );
	bool CanInterrupt( void );
	void AllowInterrupt( bool fAllow );
	void RemoveIgnoredConditions( void );
	bool PlayedSequence( void ) { return m_sequenceStarted; }
	bool CanEnqueueAfter( void );

	// Entry & Action loops
	bool IsPlayingEntry( void ) { return m_bIsPlayingEntry; }
	bool IsPlayingAction( void ) { return ( m_sequenceStarted && !m_bIsPlayingEntry ); }
	bool FinishedActionSequence( CAI_NPC *pNPC );
	void SetLoopActionSequence( bool bLoop ) { m_bLoopActionSequence = bLoop; }
	bool ShouldLoopActionSequence( void ) { return m_bLoopActionSequence; }
	void StopActionLoop( bool bStopSynchronizedScenes );
	void SetSynchPostIdles( bool bSynch ) { m_bSynchPostIdles = bSynch; }
	//void SynchNewSequence( CAI_NPC::SCRIPTSTATE newState, string_t iszSequence, bool bSynchOtherScenes );

	// Dynamic scripted sequence spawning
	void ForceSetTargetEntity( CAI_NPC *pTarget, bool bDontCancelOtherSequences );

	// Dynamic interactions
	void SetupInteractionPosition( CBaseEntity *pRelativeEntity, VMatrix &matDesiredLocalToWorld );
	void ModifyScriptedAutoMovement( Vector *vecNewPos );

	bool IsTeleportingDueToMoveTo( void ) { return m_bIsTeleportingDueToMoveTo; }

	// Debug
	virtual int DrawDebugTextOverlays( void );
	virtual void DrawDebugGeometryOverlays( void );

	void InputScriptPlayerDeath( inputdata_t &inputdata );

private:
	friend class CAI_BaseNPC;	// should probably try to eliminate this relationship

	string_t m_iszEntry;		// String index for animation that must be played before entering the main action anim
	string_t m_iszPreIdle;		// String index for idle animation to play before playing the action anim (only played while waiting for the script to begin)
	string_t m_iszPlay;			// String index for scripted action animation
	string_t m_iszPostIdle;		// String index for idle animation to play before playing the action anim
	string_t m_iszCustomMove;	// String index for custom movement animation
	string_t m_iszNextScript;	// Name of the script to run immediately after this one.
	string_t m_iszEntity;		// Entity that is wanted for this script

	int m_fMoveTo;
	bool m_bIsPlayingEntry;
	bool m_bLoopActionSequence;
	bool m_bSynchPostIdles;
	bool m_bIgnoreGravity;
	bool m_bDisableNPCCollisions;	// Used when characters must interpenetrate while riding on elevators, trains, etc.

	float m_flRadius;			// Range to search for an NPC to possess.
	float m_flRepeat;			// Repeat rate

	int m_iDelay;					// A counter indicating how many scripts are NOT ready to start.

	bool m_bDelayed;				// This moderately hacky hack ensures that we don't calls to DelayStart(true) or DelayStart(false)
									// twice in succession. This is necessary because we didn't want to remove the call to DelayStart(true)
									// from StartScript, even though DelayStart(true) is called from TASK_PRE_SCRIPT.
									// All of this is necessary in case the NPCs schedule gets cleared during the script and then they
									// reselect the schedule to play the script. Without this you can get NPCs stuck with m_iDelay = -1

	float m_startTime;				// Time when script actually started, used for synchronization
	bool m_bWaitForBeginSequence;	// Set to true when we are told to MoveToPosition. Holds the actor in the pre-action idle until BeginSequence is called.

	int m_saved_effects;
	int m_savedFlags;
	int m_savedCollisionGroup;

	bool m_interruptable;
	bool m_sequenceStarted;

	EHANDLE	m_hTargetEnt;

	EHANDLE m_hNextCine;		// The script to hand the NPC off to when we finish with them.
	
	bool	m_bThinking;
	bool 	m_bInitiatedSelfDelete;

	bool	m_bIsTeleportingDueToMoveTo;

	CAI_BaseNPC *FindScriptEntity( void );
	EHANDLE m_hLastFoundEntity;

	// Code forced us to use a specific NPC
	EHANDLE m_hForcedTarget;
	bool	m_bDontCancelOtherSequences;
	bool	m_bForceSynch;

	bool	m_bTargetWasAsleep;

	COutputEvent m_OnBeginSequence;
	COutputEvent m_OnEndSequence;
	COutputEvent m_OnPostIdleEndSequence;
	COutputEvent m_OnCancelSequence;
	COutputEvent m_OnCancelFailedSequence;	// Fired when a scene is cancelled before it's ever run
	COutputEvent m_OnScriptEvent[MAX_SCRIPT_EVENTS];

	static void ScriptEntityCancel( CBaseEntity *pentCine, bool bPretendSuccess = false );

	static const char *GetSpawnPreIdleSequenceForScript( CBaseEntity *pTargetEntity );

	// Dynamic interactions
	// For now, store just a single one of these. To synchronize positions
	// with multiple other NPCs, this needs to be an array of NPCs & desired position matrices.
	VMatrix		m_matInteractionPosition;
	EHANDLE		m_hInteractionRelativeEntity;

	int			m_iPlayerDeathBehavior;

};

#endif
