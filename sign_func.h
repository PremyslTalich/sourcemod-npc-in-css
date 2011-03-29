#ifndef _INCLUDE_SOURCEMOD_EXTENSION_SIGN_FUNC_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_SIGN_FUNC_H_

#include "extension.h"
//#include "entityinput.h"
//#include "entitydata.h"
#include "ai_activity.h"
#include "eventlist.h"

class CEntity;
class CEntityTakeDamageInfo;
class VEmptyClass {};
struct AIMoveTrace_t;

class HelperFunction
{
public:
	bool Initialize();
	void GetEntityCollisionHash();
	void GetWorldEntity();
	void HookGameRules();
	void UnHookGameRules();
	CBaseEntity *CreateEntityByName(char *entityname,int ForceEdictIndex = -1);
	int SelectWeightedSequence(void *pstudiohdr, int activity, int curSequence = -1);
	Activity ActivityList_RegisterPrivateActivity( const char *pszActivityName );
	Animevent EventList_RegisterPrivateEvent( const char *pszEventName );

public: // entity
	void SetNextThink(CBaseEntity *pEntity, float thinkTime, const char *szContext);
	void SimThink_EntityChanged(CBaseEntity *pEntity);
	void SetGroundEntity(CBaseEntity *pEntity, CBaseEntity *ground);
	void SetAbsVelocity(CBaseEntity *pEntity, const Vector &vecAbsVelocity);
	int DispatchUpdateTransmitState(CBaseEntity *pEntity);
	void SetAbsAngles(CBaseEntity *pEntity, const QAngle& absAngles);

public: // collision
	void SetSolid(void *Collision_ptr, SolidType_t val);
	void SetSolidFlags(void *Collision_ptr, int flags);

public: // gamerules
	bool GameRules_ShouldCollide(int collisionGroup, int contentsMask);

public: // entlist
	void ReportEntityFlagsChanged(CBaseEntity *pEntity, unsigned int flagsOld, unsigned int flagsNow);
	void AddPostClientMessageEntity(CBaseEntity *pEntity);
	CEntity *FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName);

public: // basenpc
	void CAI_BaseNPC_Precache(CBaseEntity *pEntity);
	bool AutoMovement(CBaseEntity *pEntity, float flInterval, CBaseEntity *pTarget, AIMoveTrace_t *pTraceResult);
	void EndTaskOverlay(CBaseEntity *pEntity);
	void SetIdealActivity(CBaseEntity *pEntity, Activity NewActivity);

public: // combatcharacter
	int CBaseCombatCharacter_OnTakeDamage(CBaseEntity *pEntity, CEntityTakeDamageInfo &info);

private:
	bool GameRules_FAllowNPCs();

private:
	void **my_g_pGameRules;
};

extern HelperFunction g_helpfunc;


#endif
