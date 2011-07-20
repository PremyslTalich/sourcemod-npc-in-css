
#include "CEntity.h"
#include "CAI_NPC.h"
#include "CAI_speech.h"
#include "sceneentity.h"
#include "CE_recipientfilter.h"


CAI_TimedSemaphore *g_AIFriendliesTalkSemaphore = NULL;
CAI_TimedSemaphore *g_AIFoesTalkSemaphore = NULL;




BEGIN_SIMPLE_DATADESC( CAI_Expresser )
	//									m_pSink		(reconnected on load)
//	DEFINE_FIELD( m_pOuter, CHandle < CBaseFlex > ),
//	DEFINE_CUSTOM_FIELD( m_ConceptHistories,	&g_ConceptHistoriesSaveDataOps ),
	DEFINE_FIELD(		m_flStopTalkTime,		FIELD_TIME		),
	DEFINE_FIELD(		m_flStopTalkTimeWithoutDelay, FIELD_TIME		),
	DEFINE_FIELD(		m_flBlockedTalkTime, 	FIELD_TIME		),
	DEFINE_FIELD(		m_voicePitch,			FIELD_INTEGER	),
	DEFINE_FIELD(		m_flLastTimeAcceptedSpeak, 	FIELD_TIME		),
END_DATADESC()

CAI_Expresser::CAI_Expresser( CBaseEntity *pOuter )
 :	m_pSink( NULL ),
	m_flStopTalkTime( 0 ),
	m_flStopTalkTimeWithoutDelay( 0 ),
	m_flBlockedTalkTime( 0 ),
	m_voicePitch( 100 ),
	m_pOuter( pOuter )
{
}


bool CAI_Expresser::IsValidResponse( ResponseType_t type, const char *pszValue )
{
	if ( type == RESPONSE_SCENE )
	{
		char szInstanceFilename[256];
		GetOuter()->GenderExpandString( pszValue, szInstanceFilename, sizeof( szInstanceFilename ) );
		return ( GetSceneDuration( szInstanceFilename ) > 0 );
	}
	return true;
}

int CAI_Expresser::SpeakRawSentence( const char *pszSentence, float delay, float volume, soundlevel_t soundlevel, CBaseEntity *pListener )
{
	int sentenceIndex = -1;

	if ( !pszSentence )
		return sentenceIndex;

	if ( pszSentence[0] == AI_SP_SPECIFIC_SENTENCE )
	{
		sentenceIndex = SENTENCEG_Lookup( pszSentence );

		if( sentenceIndex == -1 )
		{
			// sentence not found
			return -1;
		}

		CPASAttenuationFilter filter( GetOuter(), soundlevel );
		CEntity::EmitSentenceByIndex( filter, GetOuter()->entindex(), CHAN_VOICE, sentenceIndex, volume, soundlevel, 0, GetVoicePitch());
	}
	else
	{
		sentenceIndex = SENTENCEG_PlayRndSz( GetOuter()->NetworkProp()->edict(), pszSentence, volume, soundlevel, 0, GetVoicePitch() );
	}

	NoteSpeaking( engine->SentenceLength( sentenceIndex ), delay );

	return sentenceIndex;
}




bool CAI_Expresser::IsSpeaking( void )
{
	if ( m_flLastTimeAcceptedSpeak == gpGlobals->curtime ) // only one speak accepted per think
		return true;

	return ( m_flStopTalkTime > gpGlobals->curtime );
}

void CAI_Expresser::NoteSpeaking( float duration, float delay )
{
	duration += delay;
	
	GetSink()->OnStartSpeaking();

	if ( duration <= 0 )
	{
		// no duration :( 
		m_flStopTalkTime = gpGlobals->curtime + 3;
		duration = 0;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->curtime + duration;
	}

	m_flStopTalkTimeWithoutDelay = m_flStopTalkTime - delay;

	if ( GetSink()->UseSemaphore() )
	{
		CAI_TimedSemaphore *pSemaphore = GetMySpeechSemaphore( GetOuter() );
		if ( pSemaphore )
		{
			pSemaphore->Acquire( duration, GetOuter()->BaseEntity() );
		}
	}
}

int CAI_Expresser::GetVoicePitch() const
{
	return m_voicePitch + enginerandom->RandomInt(0,3);
}

CAI_TimedSemaphore *CAI_Expresser::GetMySpeechSemaphore( CEntity *pNpc ) 
{
	if ( !pNpc->MyNPCPointer() )
		return false;

	return (pNpc->MyNPCPointer()->IsPlayerAlly() ? g_AIFriendliesTalkSemaphore : g_AIFoesTalkSemaphore );
}






ConceptHistory_t::~ConceptHistory_t()
{
	if ( response )
	{
		delete response;
	}
	response = NULL;
}

