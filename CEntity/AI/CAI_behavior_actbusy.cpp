
#include "CEntity.h"
#include "CAI_NPC.h"

#include "CAI_behavior_actbusy.h"

// Anim events
static int AE_ACTBUSY_WEAPON_FIRE_ON;
static int AE_ACTBUSY_WEAPON_FIRE_OFF;


//=============================================================================================================
//=============================================================================================================

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAI_ActBusyBehavior::CAI_ActBusyBehavior()
{
	// Disabled by default. Enabled by map entity.
	m_bEnabled = false;
	m_bUseRenderBoundsForCollision = false;
	m_flDeferUntil = 0;
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if the current NPC is acting busy, or moving to an actbusy
//-----------------------------------------------------------------------------
bool CAI_ActBusyBehavior::IsActive( void )
{
	return ( m_bBusy || m_bForceActBusy || m_bNeedsToPlayExitAnim || m_bLeaving );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_ActBusyBehavior::RunTask( const Task_t *pTask )		
{ 
	switch ( pTask->iTask )
	{
	case TASK_WAIT_FOR_MOVEMENT:
		{
			// Ensure the hint node hasn't been disabled
			if ( IsCurSchedule( SCHED_ACTBUSY_START_BUSYING ) )
			{
				if ( !ActBusyNodeStillActive() )
				{
					TaskFail(FAIL_NO_HINT_NODE);
					return;
				}
			}

			BaseClass::RunTask( pTask );
			break;
		}

	case TASK_ACTBUSY_PLAY_BUSY_ANIM:
		{
			if( m_bUseRenderBoundsForCollision )
			{
				if( GetOuter()->IsSequenceFinished() && m_bNeedToSetBounds )
				{
					ComputeAndSetRenderBounds();
					m_bNeedToSetBounds = false;
				}
			}

			if( IsCombatActBusy() )
			{
				if( GetEnemy() != NULL && !HasCondition(COND_ENEMY_OCCLUDED) )
				{
					// Break a combat actbusy if an enemy gets very close.
					// I'll probably go to hell for not doing this with conditions like I should. (sjb)
					float flDistSqr = GetAbsOrigin().DistToSqr( CEntity::Instance(GetEnemy())->GetAbsOrigin() );

					if( flDistSqr < Square(12.0f * 15.0f) )
					{
						// End now.
						m_flEndBusyAt = gpGlobals->curtime;
						TaskComplete();
						return;
					}
				}
			}

			GetOuter()->AutoMovement();
			// Stop if the node's been disabled
			if ( !ActBusyNodeStillActive() || GetOuter()->IsWaitFinished() )
			{
				TaskComplete();
			}
			else
			{
				//CE_TODO
				/*CAI_PlayerAlly *pAlly = dynamic_cast<CAI_PlayerAlly*>(GetOuter());
				if ( pAlly )
				{
					pAlly->SelectInterjection();
				}*/

				if( HasCondition(COND_ACTBUSY_LOST_SEE_ENTITY) )
				{
					StopBusying();
					TaskComplete();
				}
			}
			break;
		}

	case TASK_ACTBUSY_PLAY_ENTRY:
		{
			GetOuter()->AutoMovement();
			if ( !ActBusyNodeStillActive() || GetOuter()->IsSequenceFinished() )
			{
				TaskComplete();
			}
		}
		break;

	case TASK_ACTBUSY_VERIFY_EXIT:
		{
			if( GetOuter()->IsWaitFinished() )
			{
				// Trace my normal hull over this spot to see if I'm able to stand up right now.
				trace_t tr;
				CTraceFilterOnlyNPCsAndPlayer filter( GetOuter()->BaseEntity(), COLLISION_GROUP_NONE );
				UTIL_TraceHull( GetOuter()->GetAbsOrigin(), GetOuter()->GetAbsOrigin(), NAI_Hull::Mins( HULL_HUMAN ), NAI_Hull::Maxs( HULL_HUMAN ), MASK_NPCSOLID, &filter, &tr );

				if( tr.startsolid )
				{
					// Blocked. Try again later.
					GetOuter()->SetWait( 1.0f );
				}
				else
				{
					//CE_TODO
					// Put an entity blocker here for a moment until I get into my bounding box.
					//CBaseEntity *pBlocker = CEntityBlocker::Create( GetOuter()->GetAbsOrigin(), NAI_Hull::Mins( HULL_HUMAN ), NAI_Hull::Maxs( HULL_HUMAN ), GetOuter(), true );
					//g_EventQueue.AddEvent( pBlocker, "Kill", 1.0, GetOuter(), GetOuter() );
					TaskComplete();
				}
			}
		}
		break;

	case TASK_ACTBUSY_PLAY_EXIT:
		{
			GetOuter()->AutoMovement();
			if ( GetOuter()->IsSequenceFinished() )
			{
				m_bNeedsToPlayExitAnim = false;
				GetOuter()->RemoveFlag( FL_FLY );
				NotifyBusyEnding();
				TaskComplete();
			}
		}
		break;

	default:
		BaseClass::RunTask( pTask);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CAI_ActBusyBehavior::NotifyBusyEnding( void )
{
	// Be sure to disable autofire
	m_bAutoFireWeapon = false;

	// Clear the hintnode if we're done with it
	if ( GetHintNode() )
	{
		if ( m_bBusy || m_bMovingToBusy )
		{
			GetHintNode()->NPCStoppedUsing( GetOuter() );
		}

		GetHintNode()->Unlock();

		if( IsCombatActBusy() )
		{
			// Don't allow anyone to use this node for a bit. This is so the tactical position
			// doesn't get re-occupied the moment I leave it.
			GetHintNode()->DisableForSeconds( random->RandomFloat( 10, 15) );
		}

		SetHintNode( NULL );
	}

	// Then, if we were busy, stop being busy
 	if ( m_bBusy )
	{
		m_bBusy = false;

		if ( m_hActBusyGoal )
		{
			m_hActBusyGoal->NPCFinishedBusy( GetOuter() );

			if ( m_bExitedBusyToDueLostSeeEntity )
			{
				m_hActBusyGoal->NPCLostSeeEntity( GetOuter() );
				m_bExitedBusyToDueLostSeeEntity = false;
			}

			if ( m_bExitedBusyToDueSeeEnemy )
			{
				m_hActBusyGoal->NPCSeeEnemy( GetOuter() );
				m_bExitedBusyToDueSeeEnemy = false;
			}
		}
	}
	else if ( m_bMovingToBusy && m_hActBusyGoal )
	{
		// Or if we were just on our way to be busy, let the goal know
		m_hActBusyGoal->NPCAbortedMoveTo( GetOuter() );
	}

	// Don't busy again for a while
	m_flEndBusyAt = 0;

	if( IsCombatActBusy() )
	{
		// Actbusy again soon. Real soon.
		m_flNextBusySearchTime = gpGlobals->curtime;
	}
	else
	{
		m_flNextBusySearchTime = gpGlobals->curtime + (RandomFloat(ai_actbusy_search_time.GetFloat(), ai_actbusy_search_time.GetFloat()*2));
	}
}
*/





AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_ActBusyBehavior )

	DECLARE_CONDITION( COND_ACTBUSY_LOST_SEE_ENTITY )
	DECLARE_CONDITION( COND_ACTBUSY_AWARE_OF_ENEMY_IN_SAFE_ZONE )
	DECLARE_CONDITION( COND_ACTBUSY_ENEMY_TOO_CLOSE )

	DECLARE_TASK( TASK_ACTBUSY_PLAY_BUSY_ANIM )
	DECLARE_TASK( TASK_ACTBUSY_PLAY_ENTRY )
	DECLARE_TASK( TASK_ACTBUSY_PLAY_EXIT )
	DECLARE_TASK( TASK_ACTBUSY_TELEPORT_TO_BUSY )
	DECLARE_TASK( TASK_ACTBUSY_WALK_PATH_TO_BUSY )
	DECLARE_TASK( TASK_ACTBUSY_GET_PATH_TO_ACTBUSY )
	DECLARE_TASK( TASK_ACTBUSY_VERIFY_EXIT )

	DECLARE_ANIMEVENT( AE_ACTBUSY_WEAPON_FIRE_ON )
	DECLARE_ANIMEVENT( AE_ACTBUSY_WEAPON_FIRE_OFF )

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_START_BUSYING,

		"	Tasks"
		"		TASK_SET_TOLERANCE_DISTANCE			4"
		"		TASK_ACTBUSY_GET_PATH_TO_ACTBUSY	0"
		"		TASK_ACTBUSY_WALK_PATH_TO_BUSY		0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_STOP_MOVING					0"
		"		TASK_FACE_HINTNODE					0"
		"		TASK_ACTBUSY_PLAY_ENTRY				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_ACTBUSY_BUSY"
		""
		"	Interrupts"
		"		COND_ACTBUSY_LOST_SEE_ENTITY"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_BUSY,

		"	Tasks"
		"		TASK_ACTBUSY_PLAY_BUSY_ANIM		0"
		""
		"	Interrupts"
		"		COND_PROVOKED"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_STOP_BUSYING,

		"	Tasks"
		"		TASK_ACTBUSY_VERIFY_EXIT		0"
		"		TASK_ACTBUSY_PLAY_EXIT			0"
		"		TASK_WAIT						0.1"
		""
		"	Interrupts"
		"		COND_NO_CUSTOM_INTERRUPTS"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_LEAVE,

		"	Tasks"
		"		TASK_SET_TOLERANCE_DISTANCE			4"
		"		TASK_ACTBUSY_GET_PATH_TO_ACTBUSY	0"
		"		TASK_ACTBUSY_WALK_PATH_TO_BUSY		0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		""
		"	Interrupts"
		"		COND_PROVOKED"
	)

	DEFINE_SCHEDULE
	( 
		SCHED_ACTBUSY_TELEPORT_TO_BUSY,

		"	Tasks"
		"		TASK_ACTBUSY_TELEPORT_TO_BUSY	0"
		"		TASK_ACTBUSY_PLAY_ENTRY			0"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_ACTBUSY_BUSY"
		""
		"	Interrupts"
		"		COND_PROVOKED"
	)

AI_END_CUSTOM_SCHEDULE_PROVIDER()



