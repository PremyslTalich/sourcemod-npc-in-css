
#include "CEntity.h"
#include "CFlex.h"
#include "sceneentity.h"
#include "scenefilecache/ISceneFileCache.h"



class CE_CSceneEntity : public CEntity
{
public:
	CE_DECLARE_CLASS(CE_CSceneEntity, CEntity);

public:
	virtual void CancelPlayback( void );

public:
	DECLARE_DEFAULTHEADER(CancelPlayback, void, ());

};


SH_DECL_MANUALHOOK0_void(CancelPlayback, 0, 0, 0);
DECLARE_HOOK(CancelPlayback, CE_CSceneEntity);
DECLARE_DEFAULTHANDLER_void(CE_CSceneEntity, CancelPlayback, (), ());



CE_LINK_ENTITY_TO_CLASS(CSceneEntity, CE_CSceneEntity);

void StopScriptedScene( CFlex *pActor, EHANDLE hSceneEnt )
{
	CEntity *pEntity = CEntity::Instance(hSceneEnt);
	CE_CSceneEntity *pScene = dynamic_cast<CE_CSceneEntity *>(pEntity);

	if ( pScene )
	{
		pScene->CancelPlayback();
	}
}



//-----------------------------------------------------------------------------
// Purpose: create a one-shot scene, no movement, sequences, etc.
// Input  :
// Output :
//-----------------------------------------------------------------------------
float InstancedScriptedScene( CFlex *pActor, const char *pszScene, EHANDLE *phSceneEnt,
							 float flPostDelay, bool bIsBackground, AI_Response *response,
							 bool bMultiplayer, IRecipientFilter *filter /* = NULL */ )
{
	return 0;
	//CE_TODO
#if 0
	CInstancedSceneEntity *pScene = (CInstancedSceneEntity *)CBaseEntity::CreateNoSpawn( "instanced_scripted_scene", vec3_origin, vec3_angle );

	// This code expands any $gender tags into male or female tags based on the gender of the actor (based on his/her .mdl)
	if ( pActor )
	{
		pActor->GenderExpandString( pszScene, pScene->m_szInstanceFilename, sizeof( pScene->m_szInstanceFilename ) );
	}
	else
	{
		Q_strncpy( pScene->m_szInstanceFilename, pszScene, sizeof( pScene->m_szInstanceFilename ) );
	}
	pScene->m_iszSceneFile = MAKE_STRING( pScene->m_szInstanceFilename );

	// FIXME: I should set my output to fire something that kills me....

	// FIXME: add a proper initialization function
	pScene->m_hOwner = pActor;
	pScene->m_bHadOwner = pActor != NULL;
	pScene->m_bMultiplayer = bMultiplayer;
	pScene->SetPostSpeakDelay( flPostDelay );
	DispatchSpawn( pScene );
	pScene->Activate();
	pScene->m_bIsBackground = bIsBackground;

	pScene->SetBackground( bIsBackground );
	pScene->SetRecipientFilter( filter );
	
	if ( response )
	{
		float flPreDelay = response->GetPreDelay();
		if ( flPreDelay )
		{
			pScene->SetPreDelay( flPreDelay );
		}
	}

	pScene->StartPlayback();

	if ( response )
	{
		// If the response wants us to abort on NPC state switch, remember that
		pScene->SetBreakOnNonIdle( response->ShouldBreakOnNonIdle() );
	}

	if ( phSceneEnt )
	{
		*phSceneEnt = pScene;
	}

	return pScene->EstimateLength();

#endif

}



//-----------------------------------------------------------------------------
// Purpose: Used for precaching instanced scenes
// Input  : *pszScene - 
//-----------------------------------------------------------------------------
void PrecacheInstancedScene( char const *pszScene )
{
	//CE_TODO
#if 0
	static int nMakingReslists = -1;
	
	if ( nMakingReslists == -1 )
	{
		nMakingReslists = CommandLine()->FindParm( "-makereslists" ) > 0 ? 1 : 0;
	}

	if ( nMakingReslists == 1 )
	{
		// Just stat the file to add to reslist
		g_pFullFileSystem->Size( pszScene );
	}

	// verify existence, cache is pre-populated, should be there
	SceneCachedData_t sceneData;
	if ( !scenefilecache->GetSceneCachedData( pszScene, &sceneData ) )
	{
		// Scenes are sloppy and don't always exist.
		// A scene that is not in the pre-built cache image, but on disk, is a true error.
		if ( developer.GetInt() && ( IsX360() && ( g_pFullFileSystem->GetDVDMode() != DVDMODE_STRICT ) && g_pFullFileSystem->FileExists( pszScene, "GAME" ) ) )
		{
			Warning( "PrecacheInstancedScene: Missing scene '%s' from scene image cache.\nRebuild scene image cache!\n", pszScene );
		}
	}
	else
	{
		for ( int i = 0; i < sceneData.numSounds; ++i )
		{
			short stringId = scenefilecache->GetSceneCachedSound( sceneData.sceneId, i );
			CBaseEntity::PrecacheScriptSound( scenefilecache->GetSceneString( stringId ) );
		}
	}

	g_pStringTableClientSideChoreoScenes->AddString( CBaseEntity::IsServer(), pszScene );
#endif
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszScene - 
// Output : float
//-----------------------------------------------------------------------------
float GetSceneDuration( char const *pszScene )
{
	unsigned int msecs = 0;

	SceneCachedData_t cachedData;
	if ( scenefilecache->GetSceneCachedData( pszScene, &cachedData ) )
	{
		msecs = cachedData.msecs;
	}

	return (float)msecs * 0.001f;
}


