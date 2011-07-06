#ifndef _INCLUDE_GAMESYSTEM_H_
#define _INCLUDE_GAMESYSTEM_H_

#ifdef _WIN32
#pragma once
#endif

#include "CEntity.h"

abstract_class IGameSystem
{
public:
	virtual bool SDKInit() =0;
	virtual void LevelInitPreEntity() =0;
	virtual void LevelInitPostEntity() =0;
	virtual void LevelShutdown() =0;
	virtual void SDKShutdown() =0;
	virtual char *GetSystemName();

public:
	static void Add(IGameSystem* pSys);
	static void SDKInitAllSystems();
	static void LevelInitPreEntityAllSystems();
	static void LevelInitPostEntityAllSystems();
	static void LevelShutdownAllSystems();
	static void SDKShutdownAllSystem();
	static void InitAllSystems();

protected:
	char system_name[65];
};


class CBaseGameSystem : public IGameSystem
{
public:
	CBaseGameSystem(const char *name);
	virtual bool SDKInit() { return true; };
	virtual void LevelInitPreEntity() {};
	virtual void LevelInitPostEntity() {};
	virtual void LevelShutdown() {};
	virtual void SDKShutdown() {};

public:
	CBaseGameSystem		*m_pNext;

};

#endif

