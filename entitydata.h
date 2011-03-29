#ifndef _INCLUDE_SOURCEMOD_ZX2PLUGIN_ENTITYDATA_H_
#define _INCLUDE_SOURCEMOD_ZX2PLUGIN_ENTITYDATA_H_

#include "extension.h"

void EntityDataInitialize();

bool DispatchSpawn(edict_t *pEdict);
bool DispatchKeyValue(edict_t *pEdict, char *keyname, char *value);
bool DispatchKeyValueFloat(edict_t *pEdict, char *keyname, float value);
bool DispatchKeyValueVector(edict_t *pEdict, char *keyname, Vector vec);

bool IsValidEdict(edict_t *pEdict);
bool IsValidEntity(edict_t *pEdict);
edict_t *GetEntDataEnt2(edict_t *pEdict, int offset);
edict_t *GetEntDataEnt2(CBaseEntity *pEntity, int offset);

void SetEntData(edict_t *pEdict, int offset, int value, int size=4, bool changeState=false);
void SetEntData(CBaseEntity *pEntity, int offset, int value, int size, bool changeState=false);

void SetEntDataEnt(edict_t *pEdict, int offset, edict_t *pEdict_Target, bool changeState=false);

void SetEntDataVector(edict_t *pEdict, int offset, const Vector &vec, bool changeState=false);
void SetEntDataVector(CBaseEntity *pEntity, int offset, const Vector &vec, bool changeState=false);

int GetEntData(edict_t *pEdict, int offset, int size=4);
int GetEntData(CBaseEntity *pEntity, int offset, int size=4);


void GetEntDataVector(edict_t *pEdict, int offset, Vector *vec);
void GetEntDataVector(CBaseEntity *pEntity, int offset, Vector *vec);

void SetEntDataArray(edict_t *pEdict, int offset, const int array[], int arraySize, int dataSize=4, bool changeState=false);
void SetEntDataArray(CBaseEntity *pEntity, int offset, const int array[], int arraySize, int dataSize=4, bool changeState=false);


void SetEntDataFloat(edict_t *pEdict, int offset, float value, bool changeState=false);
void SetEntDataFloat(CBaseEntity *pEntity, int offset, float value, bool changeState);

float GetEntDataFloat(edict_t *pEdict, int offset);
float GetEntDataFloat(CBaseEntity *pEntity, int offset);


#endif
