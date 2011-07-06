
#include "CEntity.h"
#include "CPlayer.h"
#include "GameSystem.h"


class CFuncLadder : public CEntity
{
public:
	CE_DECLARE_CLASS( CFuncLadder, CEntity );

	void	Spawn();
	void	Think();

	void	GetTopPosition( Vector& org );
	void	GetBottomPosition( Vector& org );
	void	ComputeLadderDir( Vector& bottomToTopVec );

	void	LadderThink();

protected: // Sendprops
	DECLARE_SENDPROP(Vector, m_vecPlayerMountPositionTop);
	DECLARE_SENDPROP(Vector, m_vecPlayerMountPositionBottom);
	DECLARE_SENDPROP(Vector, m_vecLadderDir);


};

CE_LINK_ENTITY_TO_CLASS( func_useableladder, CFuncLadder );

// Sendprops
DEFINE_PROP(m_vecPlayerMountPositionTop, CFuncLadder);
DEFINE_PROP(m_vecPlayerMountPositionBottom, CFuncLadder);
DEFINE_PROP(m_vecLadderDir, CFuncLadder);


void CFuncLadder::Spawn()
{
	BaseClass::Spawn();

	SetNextThink(gpGlobals->curtime );
}

void CFuncLadder::Think()
{
	BaseClass::Think();

	LadderThink();

	SetNextThink(gpGlobals->curtime );
}

void CFuncLadder::GetTopPosition( Vector& org )
{
	ComputeAbsPosition( *(m_vecPlayerMountPositionTop) + GetLocalOrigin(), &org );
}

void CFuncLadder::GetBottomPosition( Vector& org )
{
	ComputeAbsPosition( *(m_vecPlayerMountPositionBottom) + GetLocalOrigin(), &org );
}

void CFuncLadder::ComputeLadderDir( Vector& bottomToTopVec )
{
	Vector top;
	Vector bottom;

	GetTopPosition( top );
	GetBottomPosition( bottom );

	bottomToTopVec = top - bottom;
}


class CLadderTraceFilterSimple : public CTraceFilter
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CLadderTraceFilterSimple );
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		CEntity *pEntity = CE_EntityFromEntityHandle( pHandleEntity );
		CPlayer *pPlayer = ToBasePlayer(pEntity);
		if(pPlayer)
		{
			pPlayer->m_bOnLadder = true;
		}
		return false;
	}
};


void CFuncLadder::LadderThink()
{
	Vector start_pos, end_pos;
	GetBottomPosition(start_pos);
	GetTopPosition(end_pos);

	Vector playerMins = Vector(-16, -16, 0 );
	Vector playerMaxs = Vector( 16,  16,  64 );

	trace_t result;
	Ray_t ray;
	ray.Init( start_pos, end_pos, playerMins, playerMaxs );
	CLadderTraceFilterSimple traceFilter;

	enginetrace->TraceRay( ray, 0, &traceFilter, &result );
}
