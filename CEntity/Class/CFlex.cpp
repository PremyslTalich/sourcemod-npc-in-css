
#include "CFlex.h"
#include "CAI_NPC.h"
#include "choreoevent.h"
#include "choreoscene.h"
#include "sceneentity_shared.h"


CE_LINK_ENTITY_TO_CLASS(CBaseFlex, CFlex);


SH_DECL_MANUALHOOK5(StartSceneEvent, 0, 0, 0, bool, CSceneEventInfo *, CChoreoScene *, CChoreoEvent *, CChoreoActor *, CBaseEntity *);
DECLARE_HOOK(StartSceneEvent, CFlex);
DECLARE_DEFAULTHANDLER(CFlex, StartSceneEvent, bool, (CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget), (info, scene, event, actor, pTarget));

SH_DECL_MANUALHOOK3(ProcessSceneEvent, 0, 0, 0, bool, CSceneEventInfo *, CChoreoScene *, CChoreoEvent *);
DECLARE_HOOK(ProcessSceneEvent, CFlex);
DECLARE_DEFAULTHANDLER(CFlex, ProcessSceneEvent, bool, (CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event), (info, scene, event));

SH_DECL_MANUALHOOK3(ClearSceneEvent, 0, 0, 0, bool, CSceneEventInfo *, bool,  bool);
DECLARE_HOOK(ClearSceneEvent, CFlex);
DECLARE_DEFAULTHANDLER(CFlex, ClearSceneEvent, bool, (CSceneEventInfo *info, bool fastKill, bool canceled), (info, fastKill, canceled));

SH_DECL_MANUALHOOK4(CheckSceneEventCompletion, 0, 0, 0, bool, CSceneEventInfo *, float, CChoreoScene *, CChoreoEvent *);
DECLARE_HOOK(CheckSceneEventCompletion, CFlex);
DECLARE_DEFAULTHANDLER(CFlex, CheckSceneEventCompletion, bool, (CSceneEventInfo *info, float currenttime, CChoreoScene *scene, CChoreoEvent *event), (info, currenttime, scene, event));

SH_DECL_MANUALHOOK1_void(SetViewtarget, 0, 0, 0, const Vector &);
DECLARE_HOOK(SetViewtarget, CFlex);
DECLARE_DEFAULTHANDLER_void(CFlex, SetViewtarget, (const Vector &viewtarget), (viewtarget));

SH_DECL_MANUALHOOK0_void(ProcessSceneEvents, 0, 0, 0);
DECLARE_HOOK(ProcessSceneEvents, CFlex);
DECLARE_DEFAULTHANDLER_void(CFlex, ProcessSceneEvents, (), ());



//Sendprops
DEFINE_PROP(m_blinktoggle,CFlex);
DEFINE_PROP(m_viewtarget,CFlex);
DEFINE_PROP(m_flexWeight,CFlex);

//DataMap
DEFINE_PROP(m_flLastFlexAnimationTime,CFlex);


CFlex::CFlex()
{
	m_vecPrevOrigin.Init();
	m_vecPrevVelocity.Init();
	m_vecLean.Init();
	m_vecShift.Init();
}

LocalFlexController_t CFlex::FindFlexController( const char *szName )
{
	for (LocalFlexController_t i = LocalFlexController_t(0); i < GetNumFlexControllers(); i++)
	{
		if (stricmp( GetFlexControllerName( i ), szName ) == 0)
		{
			return i;
		}
	}

	// AssertMsg( 0, UTIL_VarArgs( "flexcontroller %s couldn't be mapped!!!\n", szName ) );
	return LocalFlexController_t(0);
}

bool CFlex::EnterSceneSequence( CChoreoScene *scene, CChoreoEvent *event, bool bRestart )
{
	CAI_NPC *myNpc = MyNPCPointer( );

	if (!myNpc)
		return false;

	// 2 seconds past current event, or 0.2 seconds past end of scene, whichever is shorter
	float flDuration = MIN( 2.0, MIN( event->GetEndTime() - scene->GetTime() + 2.0, scene->FindStopTime() - scene->GetTime() + 0.2 ) );

	if (myNpc->IsCurSchedule( SCHED_SCENE_GENERIC ))
	{
		myNpc->AddSceneLock( flDuration );
		return true;
	}

	// for now, don't interrupt sequences that don't understand being interrupted
	if (myNpc->GetCurSchedule())
	{
		CAI_ScheduleBits testBits;
		myNpc->GetCurSchedule()->GetInterruptMask( &testBits );

		testBits.Clear( COND_PROVOKED );

		if (testBits.IsAllClear()) 
		{
			return false;
		}
	}

	if (myNpc->IsInterruptable())
	{
		if (myNpc->Get_m_hCine())
		{
			// Assert( !(myNpc->GetFlags() & FL_FLY ) );
			myNpc->ExitScriptedSequence( );
		}

		myNpc->OnStartScene();
		myNpc->SetSchedule( SCHED_SCENE_GENERIC );
		myNpc->AddSceneLock( flDuration );
		return true;
	}

	return false;
}

float CFlex::Get_m_flexWeight(int index)
{
	return *(float *)(((uint8_t *)(BaseEntity())) + m_flexWeight.offset + (index*4));
}

void CFlex::Set_m_flexWeight(int index, float value)
{
	*(float *)(((uint8_t *)(BaseEntity())) + m_flexWeight.offset + (index*4)) = value;
}

float CFlex::GetFlexWeight( LocalFlexController_t index )
{
	if (index >= 0 && index < GetNumFlexControllers())
	{
		CStudioHdr *pstudiohdr = GetModelPtr( );
		if (! pstudiohdr)
			return 0;

		mstudioflexcontroller_t *pflexcontroller = pstudiohdr->pFlexcontroller( index );

		if (pflexcontroller->max != pflexcontroller->min)
		{
			return Get_m_flexWeight(index) * (pflexcontroller->max - pflexcontroller->min) + pflexcontroller->min;
		}
				
		return Get_m_flexWeight(index);
	}
	return 0.0;
}

void CFlex::SetFlexWeight( LocalFlexController_t index, float value )
{
	if (index >= 0 && index < GetNumFlexControllers())
	{
		CStudioHdr *pstudiohdr = GetModelPtr( );
		if (! pstudiohdr)
			return;

		mstudioflexcontroller_t *pflexcontroller = pstudiohdr->pFlexcontroller( index );

		if (pflexcontroller->max != pflexcontroller->min)
		{
			value = (value - pflexcontroller->min) / (pflexcontroller->max - pflexcontroller->min);
			value = clamp( value, 0.0, 1.0 );
		}

		Set_m_flexWeight( index, value );
	}
}


bool CFlex::IsSuppressedFlexAnimation( CSceneEventInfo *info )
{
	// check for suppression if the current info is a background
	if (info->m_pScene && info->m_pScene->IsBackground())
	{
		// allow for slight jitter
		return m_flLastFlexAnimationTime > gpGlobals->curtime - GetAnimTimeInterval() * 1.5;
	}
	// keep track of last non-suppressable flex animation
	m_flLastFlexAnimationTime = gpGlobals->curtime;
	return false;
}

LocalFlexController_t CFlex::FlexControllerLocalToGlobal( const flexsettinghdr_t *pSettinghdr, int key )
{
	//CE_TODO
	LocalFlexController_t index;
	return index;
}

const void *CFlex::FindSceneFile( const char *filename )
{
	//CE_TODO
	// Ask manager to get the globally cached scene instead.
	return NULL;//g_FlexSceneFileManager.FindSceneFile( this, filename, false );
}



float CSceneEventInfo::UpdateWeight( CFlex *pActor )
{
	// decay if this is a background scene and there's other flex animations playing
	if (pActor->IsSuppressedFlexAnimation( this ))
	{
		m_flWeight = MAX( m_flWeight - 0.2, 0.0 );
	}
	else
	{
		m_flWeight = MIN( m_flWeight + 0.1, 1.0 );
	}
	return m_flWeight;
}

void CSceneEventInfo::InitWeight( CFlex *pActor )
{
	if (pActor->IsSuppressedFlexAnimation( this ))
	{
		m_flWeight = 0.0;
	}
	else
	{
		m_flWeight = 1.0;
	}
}

void CFlex::DoBodyLean( void )
{
	CAI_NPC *myNpc = MyNPCPointer( );

	if (myNpc)
	{
		Vector vecDelta;
		Vector vecPos;
		Vector vecOrigin = GetAbsOrigin();

		if (m_vecPrevOrigin == vec3_origin)
		{
			m_vecPrevOrigin = vecOrigin;
		}

		vecDelta = vecOrigin - m_vecPrevOrigin;
		vecDelta.x = clamp( vecDelta.x, -50, 50 );
		vecDelta.y = clamp( vecDelta.y, -50, 50 );
		vecDelta.z = clamp( vecDelta.z, -50, 50 );

		float dt = gpGlobals->curtime - GetLastThink();
		bool bSkip = ((GetFlags() & (FL_FLY | FL_SWIM)) != 0) || (GetMoveParent() != NULL) || (GetGroundEntity() == NULL) || (GetGroundEntity()->IsMoving());
		bSkip |= myNpc->TaskRanAutomovement() || (myNpc->GetVehicleEntity() != NULL);

		if (!bSkip)
		{
			if (vecDelta.LengthSqr() > m_vecPrevVelocity.LengthSqr())
			{
				float decay =  ExponentialDecay( 0.6, 0.1, dt );
				m_vecPrevVelocity = m_vecPrevVelocity * (decay) + vecDelta * (1.f - decay);
			}
			else
			{
				float decay =  ExponentialDecay( 0.4, 0.1, dt );
				m_vecPrevVelocity = m_vecPrevVelocity * (decay) + vecDelta * (1.f - decay);
			}

			vecPos = m_vecPrevOrigin + m_vecPrevVelocity;

			float decay =  ExponentialDecay( 0.5, 0.1, dt );
			m_vecShift = m_vecShift * (decay) + (vecOrigin - vecPos) * (1.f - decay); // FIXME: Scale this
			m_vecLean = (vecOrigin - vecPos) * 1.0; // FIXME: Scale this
		}
		else
		{
			m_vecPrevVelocity = vecDelta;
			float decay =  ExponentialDecay( 0.5, 0.1, dt );
			m_vecShift = m_vecLean * decay;
			m_vecLean = m_vecShift * decay;
 		}

		m_vecPrevOrigin = vecOrigin;

		/*
		DevMsg( "%.2f %.2f %.2f  (%.2f %.2f %.2f)\n", 
			m_vecLean.Get().x, m_vecLean.Get().y, m_vecLean.Get().z,
			vecDelta.x, vecDelta.y, vecDelta.z );
		*/
	}
}






