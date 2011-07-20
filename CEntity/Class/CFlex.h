
#ifndef _INCLUDE_CFLEX_H_
#define _INCLUDE_CFLEX_H_

#include "CAnimatingOverlay.h"

class CSceneEventInfo;
class CChoreoScene;
class CChoreoEvent;
class CChoreoActor;


class CFlex : public CAnimatingOverlay
{
public:
	CE_DECLARE_CLASS(CFlex, CAnimatingOverlay);

public:
	CFlex();

	LocalFlexController_t	FindFlexController( const char *szName );
	
	void				SetFlexWeight( char *szName, float value );
	void				SetFlexWeight( LocalFlexController_t index, float value );
	float				GetFlexWeight( char *szName );
	float				GetFlexWeight( LocalFlexController_t index );

	void				Blink( );

	bool EnterSceneSequence( CChoreoScene *scene, CChoreoEvent *event, bool bRestart = false );
	
	const Vector		&GetViewtarget( void ) const;

	void				DoBodyLean( void );

	float				Get_m_flexWeight(int index);
	void				Set_m_flexWeight(int index, float value);

	bool				IsSuppressedFlexAnimation( CSceneEventInfo *info );

protected:
	const void				*FindSceneFile( const char *filename );
	LocalFlexController_t	FlexControllerLocalToGlobal( const flexsettinghdr_t *pSettinghdr, int key );

public:
	virtual bool StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget );
	virtual	bool ProcessSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event );
	virtual	bool ClearSceneEvent( CSceneEventInfo *info, bool fastKill, bool canceled );
	virtual bool CheckSceneEventCompletion( CSceneEventInfo *info, float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual	void SetViewtarget( const Vector &viewtarget );
	virtual void ProcessSceneEvents( void );

public:
	DECLARE_DEFAULTHEADER(StartSceneEvent, bool, ( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget ));
	DECLARE_DEFAULTHEADER(ProcessSceneEvent, bool, (CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event ));
	DECLARE_DEFAULTHEADER(ClearSceneEvent, bool, (CSceneEventInfo *info, bool fastKill, bool canceled));
	DECLARE_DEFAULTHEADER(CheckSceneEventCompletion, bool, (CSceneEventInfo *info, float currenttime, CChoreoScene *scene, CChoreoEvent *event ));
	DECLARE_DEFAULTHEADER(SetViewtarget, void, (const Vector &viewtarget));
	DECLARE_DEFAULTHEADER(ProcessSceneEvents, void, ());

public:
	DECLARE_SENDPROP(int, m_blinktoggle);
	
protected:
	DECLARE_SENDPROP(Vector, m_viewtarget);
	DECLARE_SENDPROP(float *, m_flexWeight);

protected:
	DECLARE_DATAMAP(float, m_flLastFlexAnimationTime);

public:
	Vector m_vecPrevOrigin;
	Vector m_vecPrevVelocity;
	Vector m_vecLean;
	Vector m_vecShift;


};


inline void CFlex::Blink()
{
	*(m_blinktoggle) = !m_blinktoggle;
}

inline const Vector &CFlex::GetViewtarget( ) const
{
	return *(m_viewtarget);	// bah
}

inline float CFlex::GetFlexWeight( char *szName )
{
	return GetFlexWeight( FindFlexController( szName ) );
}

inline void CFlex::SetFlexWeight( char *szName, float value )
{
	SetFlexWeight( FindFlexController( szName ), value );
}

#endif

