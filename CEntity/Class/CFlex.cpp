
#include "CFlex.h"
#include "CAI_NPC.h"
#include "choreoevent.h"
#include "choreoscene.h"

CE_LINK_ENTITY_TO_CLASS(CBaseFlex, CFlex);


SH_DECL_MANUALHOOK5(StartSceneEvent, 0, 0, 0, bool, CSceneEventInfo *, CChoreoScene *, CChoreoEvent *, CChoreoActor *, CBaseEntity *);
DECLARE_HOOK(StartSceneEvent, CFlex);
DECLARE_DEFAULTHANDLER(CFlex, StartSceneEvent, bool, (CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget), (info, scene, event, actor, pTarget));



//Sendprops
DEFINE_PROP(m_blinktoggle,CFlex);



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

