
#include "GameSystem.h"


static CUtlVector<IGameSystem*> s_my_GameSystems( 0, 4 );
static	CBaseGameSystem *s_my_pSystemList = NULL;
static bool s_bSystemsInitted = false; 

typedef void (IGameSystem::*GameSystemFunc_t)();

CUtlVector<IValveGameSystem*> *s_GameSystems;

void InvokeMethod( GameSystemFunc_t f )
{
	int i;
	int c = s_my_GameSystems.Count();
	for ( i = 0; i < c ; ++i )
	{
		IGameSystem *sys = s_my_GameSystems[i];
		(sys->*f)();
	}
}

void IGameSystem::InitAllSystems()
{
	{
		// first add any auto systems to the end
		CBaseGameSystem *pSystem = s_my_pSystemList;
		while ( pSystem )
		{
			if ( s_my_GameSystems.Find( pSystem ) == s_my_GameSystems.InvalidIndex() )
			{
				Add( pSystem );
			}
			pSystem = pSystem->m_pNext;
		}
		s_my_pSystemList = NULL;
	}
	s_bSystemsInitted = true;
}

void IGameSystem::Add( IGameSystem* pSys )
{
	s_my_GameSystems.AddToTail( pSys );
}

void IGameSystem::SDKInitAllSystems()
{
	int i;
	int c = s_my_GameSystems.Count();
	for ( i = 0; i < c ; ++i )
	{
		IGameSystem *sys = s_my_GameSystems[i];
		if(!sys->SDKInit())
		{
			META_CONPRINTF("[*] %s SDKInit failed to Initialize.",sys->GetSystemName());
		}
	}
}

void IGameSystem::LevelInitPreEntityAllSystems()
{
	InvokeMethod(&IGameSystem::LevelInitPreEntity);
}

void IGameSystem::LevelInitPostEntityAllSystems()
{
	InvokeMethod(&IGameSystem::LevelInitPostEntity);
}

void IGameSystem::LevelShutdownPreEntityAllSystems()
{
	InvokeMethod(&IGameSystem::LevelShutdownPreEntity);
}

void IGameSystem::LevelShutdownPostEntityAllSystems()
{
	InvokeMethod(&IGameSystem::LevelShutdownPostEntity);
}

void IGameSystem::SDKShutdownAllSystem()
{
	InvokeMethod(&IGameSystem::SDKShutdown);
}

char *IGameSystem::GetSystemName()
{
	return system_name;
}

CBaseGameSystem::CBaseGameSystem(const char *name)
{
	memset(system_name,0,sizeof(system_name));
	strncpy(system_name, name, strlen(name));

	if ( s_bSystemsInitted )
	{
		Add( this );
	}
	else
	{
		m_pNext = s_my_pSystemList;
		s_my_pSystemList = this;
	}
}
