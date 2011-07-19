
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
	LocalFlexController_t	FindFlexController( const char *szName );
	
	void				SetFlexWeight( char *szName, float value );
	void				SetFlexWeight( LocalFlexController_t index, float value );
	float				GetFlexWeight( char *szName );
	float				GetFlexWeight( LocalFlexController_t index );

	void				Blink( );

	bool EnterSceneSequence( CChoreoScene *scene, CChoreoEvent *event, bool bRestart = false );

public:
	virtual bool StartSceneEvent( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget );

public:
	DECLARE_DEFAULTHEADER(StartSceneEvent, bool, ( CSceneEventInfo *info, CChoreoScene *scene, CChoreoEvent *event, CChoreoActor *actor, CBaseEntity *pTarget ));

public:
	DECLARE_SENDPROP(int, m_blinktoggle);


};


inline void CFlex::Blink()
{
	*(m_blinktoggle) = !m_blinktoggle;
}



#endif