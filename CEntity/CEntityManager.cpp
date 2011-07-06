/**
* =============================================================================
* CEntity Entity Handling Framework
* Copyright (C) 2011 Matt Woodrow.  All rights reserved.
* =============================================================================
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License, version 3.0, as published by the
* Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CEntityManager.h"
#include "shareddefs.h"
#include "sourcehook.h"
#include "IEntityFactory.h"
#include "ehandle.h"
class CBaseEntity;
typedef CHandle<CBaseEntity> EHANDLE;
#include "takedamageinfo.h"
#include "server_class.h"
#include "CEntity.h"
#include "usercmd.h"
#include "isaverestore.h"

//#ifdef WIN32
#include "rtti.h"
//#endif
#include "../CDetour/detours.h"


SH_DECL_HOOK1(IEntityFactoryDictionary_CE, Create, SH_NOATTRIB, 0, IServerNetworkable *, const char *);
SH_DECL_HOOK1_void(IVEngineServer, RemoveEdict, SH_NOATTRIB, 0, edict_t *);

IEntityFactoryReal *IEntityFactoryReal::m_Head;

EntityFactoryDictionaryCall EntityFactoryDictionary_CE = NULL;

CDetour *RemoveEntity_CDetour = NULL;


#ifdef _DEBUG
unsigned long long CEntityManager::count = 0;
#endif

DETOUR_DECL_MEMBER1(RemoveEntity, void, CBaseHandle, handle)
{
	CEntity *cent = CEntity::Instance(handle);
	if(cent)
	{
		cent->Destroy();
	}
	DETOUR_MEMBER_CALL(RemoveEntity)(handle);
}

CEntityManager *GetEntityManager()
{
	static CEntityManager *entityManager = new CEntityManager();
	return entityManager;
}

CEntityManager::CEntityManager()
{
	memset(pEntityData, 0, sizeof(pEntityData));
	m_bEnabled = false;
}

bool CEntityManager::Init(IGameConfig *pConfig)
{
	/* Find the IEntityFactoryDictionary_CE* */
	void *addr;
	if (!pConfig->GetMemSig("EntityFactory", &addr) || addr == NULL)
	{
		g_pSM->LogError(myself, "[CENTITY] Couldn't find sig: %s.", "EntityFactory");
		return false;
	}

	EntityFactoryDictionary_CE = (EntityFactoryDictionaryCall)addr;
	pDict = EntityFactoryDictionary_CE();

	IEntityFactoryReal *pList = IEntityFactoryReal::m_Head;
	while (pList)
	{
		pList->AddToList();
		pList = pList->m_Next;
	}

	if (!pConfig->GetMemSig("FireOutput", &addr) || addr == NULL)
	{
		g_pSM->LogError(myself, "[CENTITY] Couldn't find sig: %s.", "FireOutput");
		return false;
	}

	FireOutputFunc= (FireOutputFuncType)addr;

	if (!pConfig->GetMemSig("PhysIsInCallback", &addr) || addr == NULL)
	{
		g_pSM->LogError(myself, "[CENTITY] Couldn't find sig: %s.", "PhysIsInCallback");
		return false;
	}

	PhysIsInCallback = (PhysIsInCallbackFuncType)addr;


	/* Reconfigure all the hooks */
	IHookTracker *pTracker = IHookTracker::m_Head;
	while (pTracker)
	{
		pTracker->ReconfigureHook(pConfig);
		pTracker = pTracker->m_Next;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), pConfig);

	RemoveEntity_CDetour = DETOUR_CREATE_MEMBER(RemoveEntity, "CBaseEntityList::RemoveEntity");
	RemoveEntity_CDetour->EnableDetour();

	IDetourTracker *pDetourTracker = IDetourTracker::m_Head;
	while (pDetourTracker)
	{
		pDetourTracker->AddHook();
		pDetourTracker = pDetourTracker->m_Next;
	}

	ISigOffsetTracker *pSigOffsetTracker = ISigOffsetTracker::m_Head;
	while (pSigOffsetTracker)
	{
		pSigOffsetTracker->FindSig(pConfig);
		pSigOffsetTracker = pSigOffsetTracker->m_Next;
	}

	/* Start the creation hooks! */
	SH_ADD_HOOK(IEntityFactoryDictionary_CE, Create, pDict, SH_MEMBER(this, &CEntityManager::Create), true);
	SH_ADD_HOOK(IVEngineServer, RemoveEdict, engine, SH_MEMBER(this, &CEntityManager::RemoveEdict), true);

	srand(time(NULL));

	m_bEnabled = true;
	return true;
}

void CEntityManager::Shutdown()
{
	SH_REMOVE_HOOK(IEntityFactoryDictionary_CE, Create, pDict, SH_MEMBER(this, &CEntityManager::Create), true);
	SH_REMOVE_HOOK(IVEngineServer, RemoveEdict, engine, SH_MEMBER(this, &CEntityManager::RemoveEdict), true);

	IDetourTracker *pDetourTracker = IDetourTracker::m_Head;
	while (pDetourTracker)
	{
		pDetourTracker->RemoveHook();
		pDetourTracker = pDetourTracker->m_Next;
	}
	
	if(RemoveEntity_CDetour)
		RemoveEntity_CDetour->DisableDetour();
}

void CEntityManager::LinkEntityToClass(IEntityFactory_CE *pFactory, const char *className)
{
	assert(pFactory);
	pFactoryTrie.insert(className, pFactory);
}

void CEntityManager::LinkEntityToClass(IEntityFactory_CE *pFactory, const char *className, const char *replaceName)
{
	LinkEntityToClass(pFactory, className);
	pSwapTrie.insert(className, replaceName);
}

static const char hhh[] = "info_player_counterterrorist";

IServerNetworkable *CEntityManager::Create(const char *pClassName)
{
	/*if(strcmp(pClassName,"grenade") == 0)
	{
		//META_CONPRINTF("CEntityManager::Create %s\n",pClassName);
	}*/

	//META_CONPRINTF("CEntityManager::Create %s\n",pClassName);

	IServerNetworkable *pNetworkable = META_RESULT_ORIG_RET(IServerNetworkable *);

	if (!pNetworkable)
	{
		return NULL;
	}

	CBaseEntity *pEnt = pNetworkable->GetBaseEntity();
	edict_t *pEdict = pNetworkable->GetEdict();

	if (/*!pEdict || */!pEnt)
	{
		return NULL;
	}

	if(strcmp(pClassName,"player") == 0 && engine->IndexOfEdict(pEdict) == 0)
	{
		//META_CONPRINTF("CEntityManager::Create Skip! \"%s\"\n",pClassName);
		return NULL;
	}


	IEntityFactory_CE **value = NULL;
	value = pFactoryTrie.retrieve(pClassName);

	if (!value)
	{
		/* Attempt to do an RTTI lookup for C++ class links */
		IType *pType = GetType(pEnt);
		IBaseType *pBase = pType->GetBaseType();

		do 
		{
			const char *classname = GetTypeName(pBase->GetTypeInfo());
			value = pFactoryTrie.retrieve(classname);

			if (value)
			{
				break;
			}

		} while (pBase->GetNumBaseClasses() && (pBase = pBase->GetBaseClass(0)));

		pType->Destroy();
	}

	if (!value)
	{
		/* No specific handler for this entity */
		value = pFactoryTrie.retrieve("baseentity");
		assert(value);
	}

	IEntityFactory_CE *pFactory = *value;
	assert(pFactory);

	CEntity *pEntity = pFactory->Create(pEdict, pEnt);

	char vtable[20];
	_snprintf(vtable, sizeof(vtable), "%x", (unsigned int) *(void **)pEnt);

	pEntity->ClearAllFlags();
	pEntity->InitProps();
	
	if (!pHookedTrie.retrieve(vtable))
	{
		pEntity->InitHooks();
		pHookedTrie.insert(vtable, true);
	}
	
	pEntity->InitDataMap();


	pEntity->SetClassname(pClassName);

	pEntity->PostInit();

	return NULL;
}

void CEntityManager::RemoveEdict(edict_t *e)
{
	CEntity *pEnt = CEntity::Instance(e);
	if (pEnt)
	{
		assert(0);
		g_pSM->LogMessage(myself, "Edict Removed, removing CEntity");
		pEnt->Destroy();
	}
}


void our_trie_iterator(KTrie<IEntityFactory_CE *> *pTrie, const char *name, IEntityFactory_CE *& obj, void *data)
{
	SourceHook::List<CEntity *> *cent_list = (SourceHook::List<CEntity *> *)data;
	int count = 0;
	SourceHook::List<CEntity *>::iterator _iter;
	CEntity *pInfo;
	for(_iter=cent_list->begin(); _iter!=cent_list->end(); _iter++)
	{
		pInfo = (*_iter);

		if(strcmp(name, pInfo->GetClassname()) == 0)
		{
			count++;
			continue;
		}
		IType *pType = GetType(pInfo->BaseEntity());
		IBaseType *pBase = pType->GetBaseType();

		do 
		{
			const char *classname = GetTypeName(pBase->GetTypeInfo());
			if(strcmp(classname, name) == 0)
			{
				count++;
			}
		} while (pBase->GetNumBaseClasses() && (pBase = pBase->GetBaseClass(0)));
		pType->Destroy();
	}

	if(strlen(name) < 7)
		META_CONPRINTF("%s:\t\t\t\t%d\n",name,count);
	else if(strlen(name) < 15)
		META_CONPRINTF("%s:\t\t\t%d\n",name,count);
	else if(strlen(name) < 25)
		META_CONPRINTF("%s:\t\t%d\n",name,count);
	else
		META_CONPRINTF("%s:\t%d\n",name,count);
}

void CEntityManager::PrintDump()
{
	META_CONPRINTF("=====================================\n");
	
	SourceHook::List<CEntity *> cent_list;

	CEntity *cent;
	int networked_count = 0;
	int non_networked_count = 0;
	for(int i=0;i<NUM_ENT_ENTRIES;i++)
	{
		cent = pEntityData[i];
		if(cent != NULL)
		{
			cent_list.push_back(cent);
			if(i < MAX_EDICTS)
				networked_count++;
			else
				non_networked_count++;
		}
	}

#ifdef _DEBUG
	META_CONPRINTF("CEntity Instance:\t\t%lld\n",CEntityManager::count);
#endif
	META_CONPRINTF("CEntity Networked:\t\t%d\n",networked_count);
	META_CONPRINTF("CEntity Non Networked:\t\t%d\n",non_networked_count);
	META_CONPRINTF("CEntity Total:\t\t\t%d\n",networked_count+non_networked_count);

	char buffer[128];
	pFactoryTrie.bad_iterator(buffer,128, &cent_list,our_trie_iterator);

	cent_list.clear();
	META_CONPRINTF("=====================================\n");
}
