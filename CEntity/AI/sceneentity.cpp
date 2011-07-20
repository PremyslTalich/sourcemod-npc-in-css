
#include "CEntity.h"
#include "CFlex.h"
#include "sceneentity.h"
#include "scenefilecache/ISceneFileCache.h"
#include "CAI_Criteria.h"
#include "CE_recipientfilter.h"
#include "choreoscene.h"


class CE_CSceneEntity : public CEntity
{
public:
	CE_DECLARE_CLASS(CE_CSceneEntity, CEntity);

public:
	virtual void CancelPlayback( void );
	virtual void StartPlayback( void );
	virtual float EstimateLength( void );

public:
	void SetBackground( bool bIsBackground );
	void SetRecipientFilter( IRecipientFilter *filter );
	
	void	SetBreakOnNonIdle( bool bBreakOnNonIdle ) { m_bBreakOnNonIdle = bBreakOnNonIdle; }

public:
	DECLARE_DEFAULTHEADER(CancelPlayback, void, ());
	DECLARE_DEFAULTHEADER(StartPlayback, void, ());
	DECLARE_DEFAULTHEADER(EstimateLength, float, ());

public:
	DECLARE_SENDPROP(bool, m_bMultiplayer);

public:
	DECLARE_DATAMAP(bool, m_bBreakOnNonIdle);
	DECLARE_DATAMAP_OFFSET(CRecipientFilter *, m_pRecipientFilter);
	DECLARE_DATAMAP_OFFSET(CChoreoScene *, m_pScene);


};

CE_LINK_ENTITY_TO_CLASS(CSceneEntity, CE_CSceneEntity);


SH_DECL_MANUALHOOK0_void(CancelPlayback, 0, 0, 0);
DECLARE_HOOK(CancelPlayback, CE_CSceneEntity);
DECLARE_DEFAULTHANDLER_void(CE_CSceneEntity, CancelPlayback, (), ());

SH_DECL_MANUALHOOK0_void(StartPlayback, 0, 0, 0);
DECLARE_HOOK(StartPlayback, CE_CSceneEntity);
DECLARE_DEFAULTHANDLER_void(CE_CSceneEntity, StartPlayback, (), ());

SH_DECL_MANUALHOOK0(EstimateLength, 0, 0, 0, float);
DECLARE_HOOK(EstimateLength, CE_CSceneEntity);
DECLARE_DEFAULTHANDLER(CE_CSceneEntity, EstimateLength, float, (), ());



//Sendprops
DEFINE_PROP(m_bMultiplayer,CE_CSceneEntity);

//DataMap
DEFINE_PROP(m_bBreakOnNonIdle,CE_CSceneEntity);
DEFINE_PROP(m_pRecipientFilter,CE_CSceneEntity);
DEFINE_PROP(m_pScene,CE_CSceneEntity);




class CE_CInstancedSceneEntity : public CE_CSceneEntity
{
public:
	CE_DECLARE_CLASS(CE_CInstancedSceneEntity, CE_CSceneEntity);

public:
	// not hooked
	virtual void SetPostSpeakDelay( float flDelay ) { m_flPostSpeakDelay = flDelay; }
	virtual void SetPreDelay( float flDelay ) { m_flPreDelay = flDelay; }

public:
	DECLARE_DATAMAP(char *, m_szInstanceFilename);
	DECLARE_DATAMAP(string_t, m_iszSceneFile);
	DECLARE_DATAMAP(CFakeHandle, m_hOwner);
	DECLARE_DATAMAP(bool, m_bHadOwner);
	DECLARE_DATAMAP(float, m_flPostSpeakDelay);
	DECLARE_DATAMAP(bool, m_bIsBackground);
	DECLARE_DATAMAP(float, m_flPreDelay);

};

CE_LINK_ENTITY_TO_CLASS(CInstancedSceneEntity, CE_CInstancedSceneEntity);


//DataMap
DEFINE_PROP(m_szInstanceFilename,CE_CInstancedSceneEntity);
DEFINE_PROP(m_iszSceneFile,CE_CInstancedSceneEntity);
DEFINE_PROP(m_hOwner,CE_CInstancedSceneEntity);
DEFINE_PROP(m_bHadOwner,CE_CInstancedSceneEntity);
DEFINE_PROP(m_bIsBackground,CE_CInstancedSceneEntity);
DEFINE_PROP(m_flPreDelay,CE_CInstancedSceneEntity);








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

	CE_CInstancedSceneEntity *pScene = (CE_CInstancedSceneEntity *)CEntity::CreateNoSpawn( "instanced_scripted_scene", vec3_origin, vec3_angle );

	// This code expands any $gender tags into male or female tags based on the gender of the actor (based on his/her .mdl)
	if ( pActor )
	{
		pActor->GenderExpandString( pszScene, pScene->m_szInstanceFilename, /*sizeof( pScene->m_szInstanceFilename ) */128);
	}
	else
	{
		Q_strncpy( pScene->m_szInstanceFilename, pszScene, /*sizeof( pScene->m_szInstanceFilename ) */128);
	}
	pScene->m_iszSceneFile = MAKE_STRING( pScene->m_szInstanceFilename );

	// FIXME: I should set my output to fire something that kills me....

	// FIXME: add a proper initialization function
	pScene->m_hOwner.ptr->Set((pActor)?pActor->BaseEntity():NULL);
	pScene->m_bHadOwner = pActor != NULL;
	pScene->m_bMultiplayer = bMultiplayer;
	pScene->SetPostSpeakDelay( flPostDelay );
	DispatchSpawn( pScene->BaseEntity() );
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
		*phSceneEnt = pScene->BaseEntity();
	}

	return pScene->EstimateLength();
}

void PrecacheInstancedScene( char const *pszScene )
{
	g_helpfunc.PrecacheInstancedScene(pszScene);
}


void CE_CSceneEntity::SetRecipientFilter( IRecipientFilter *filter )
{
	// create a copy of this filter
	if ( filter )
	{
		CRecipientFilter *recp = new CRecipientFilter();
		recp->CopyFrom( (CRecipientFilter &)( *filter ) );
		*(m_pRecipientFilter.ptr) = recp;
	}
}

void CE_CSceneEntity::SetBackground( bool bIsBackground )
{
	if ( *(m_pScene.ptr) )
	{
		(*(m_pScene.ptr))->SetBackground( bIsBackground );
	}
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


