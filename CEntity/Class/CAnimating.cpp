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

#include "CAnimating.h"

CE_LINK_ENTITY_TO_CLASS(CBaseAnimating, CAnimating);


SH_DECL_MANUALHOOK0_void(StudioFrameAdvance, 0, 0, 0);
DECLARE_HOOK(StudioFrameAdvance, CAnimating);
DECLARE_DEFAULTHANDLER_void(CAnimating, StudioFrameAdvance, (), ());


DECLARE_DETOUR(StudioFrameAdvanceManual, CAnimating);
DECLARE_DEFAULTHANDLER_DETOUR_void(CAnimating, StudioFrameAdvanceManual, (float flInterval),(flInterval));


SH_DECL_MANUALHOOK4_void(Ignite, 0, 0, 0, float, bool, float, bool);
DECLARE_HOOK(Ignite, CAnimating);
DECLARE_DEFAULTHANDLER_void(CAnimating, Ignite, (float flFlameLifetime, bool bNPCOnly, float flSize , bool bCalledByLevelDesigner), (flFlameLifetime, bNPCOnly, flSize, bCalledByLevelDesigner));

DECLARE_DETOUR(GetModelPtr, CAnimating);
DECLARE_DEFAULTHANDLER_DETOUR(CAnimating, GetModelPtr, CStudioHdr *, (),());


//Sendprops
DEFINE_PROP(m_flPlaybackRate,CAnimating);
DEFINE_PROP(m_flPoseParameter,CAnimating);


//Datamaps
DEFINE_PROP(m_flCycle,CAnimating);
DEFINE_PROP(m_pStudioHdr,CAnimating);
DEFINE_PROP(m_nSequence,CAnimating);
DEFINE_PROP(m_bSequenceLoops,CAnimating);
DEFINE_PROP(m_flGroundSpeed,CAnimating);
DEFINE_PROP(m_bSequenceFinished,CAnimating);



#define MAX_ANIMTIME_INTERVAL 0.2f

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CAnimating::GetAnimTimeInterval() const
{
	float flInterval;
	if (m_flAnimTime < gpGlobals->curtime)
	{
		// estimate what it'll be this frame
		flInterval = clamp( gpGlobals->curtime - *m_flAnimTime, 0, MAX_ANIMTIME_INTERVAL );
	}
	else
	{
		// report actual
		flInterval = clamp( *m_flAnimTime - *m_flPrevAnimTime, 0, MAX_ANIMTIME_INTERVAL );
	}
	return flInterval;
}



//-----------------------------------------------------------------------------
// Purpose:
// Output :
//-----------------------------------------------------------------------------
/*bool CAnimating::GetIntervalMovement( float flIntervalUsed, bool &bMoveSeqFinished, Vector &newPosition, QAngle &newAngles )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if (! pstudiohdr || !pstudiohdr->SequencesAvailable())
		return false;

	float flComputedCycleRate = GetSequenceCycleRate( GetSequence() );
	
	float flNextCycle = GetCycle() + flIntervalUsed * flComputedCycleRate * m_flPlaybackRate;

	if ((!m_bSequenceLoops) && flNextCycle > 1.0)
	{
		flIntervalUsed = GetCycle() / (flComputedCycleRate * m_flPlaybackRate);
		flNextCycle = 1.0;
		bMoveSeqFinished = true;
	}
	else
	{
		bMoveSeqFinished = false;
	}

	Vector deltaPos;
	QAngle deltaAngles;

	if (Studio_SeqMovement( pstudiohdr, GetSequence(), GetCycle(), flNextCycle, GetPoseParameterArray(), deltaPos, deltaAngles ))
	{
		VectorYawRotate( deltaPos, GetLocalAngles().y, deltaPos );
		newPosition = GetLocalOrigin() + deltaPos;
		newAngles.Init();
		newAngles.y = GetLocalAngles().y + deltaAngles.y;
		return true;
	}
	else
	{
		newPosition = GetLocalOrigin();
		newAngles = GetLocalAngles();
		return false;
	}
}*/

/**
float CAnimating::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	float t = SequenceDuration( pStudioHdr, iSequence );

	if (t > 0.0f)
	{
		return 1.0f / t;
	}
	else
	{
		return 1.0f / 0.1f;
	}
}

//=========================================================
//=========================================================
float CAnimating::SequenceDuration( CStudioHdr *pStudioHdr, int iSequence )
{
	if ( !pStudioHdr )
	{
		DevWarning( 2, "CBaseAnimating::SequenceDuration( %d ) NULL pstudiohdr on %s!\n", iSequence, GetClassname() );
		return 0.1;
	}
	if ( !pStudioHdr->SequencesAvailable() )
	{
		return 0.1;
	}
	if (iSequence >= pStudioHdr->GetNumSeq() || iSequence < 0 )
	{
		DevWarning( 2, "CBaseAnimating::SequenceDuration( %d ) out of range\n", iSequence );
		return 0.1;
	}

	return Studio_Duration( pStudioHdr, iSequence, GetPoseParameterArray() );
}
*/

//=========================================================
//=========================================================
bool CAnimating::HasPoseParameter( int iSequence, int iParameter )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );

	if ( !pstudiohdr )
	{
		return false;
	}

	if ( !pstudiohdr->SequencesAvailable() )
	{
		return false;
	}

	if (iSequence < 0 || iSequence >= pstudiohdr->GetNumSeq())
	{
		return false;
	}

	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( iSequence );
	if (pstudiohdr->GetSharedPoseParameter( iSequence, seqdesc.paramindex[0] ) == iParameter || 
		pstudiohdr->GetSharedPoseParameter( iSequence, seqdesc.paramindex[1] ) == iParameter)
	{
		return true;
	}
	return false;
}



//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : iSequence - 
//
// Output : float - 
//-----------------------------------------------------------------------------
float CAnimating::GetSequenceMoveYaw( int iSequence )
{
	Vector				vecReturn;
	
	Assert( GetModelPtr() );
	::GetSequenceLinearMotion( GetModelPtr(), iSequence, GetPoseParameterArray(), &vecReturn );

	if (vecReturn.Length() > 0)
	{
		return UTIL_VecToYaw( vecReturn );
	}

	return NOMOTION;
}

//-----------------------------------------------------------------------------
// Make a model look as though it's burning. 
//-----------------------------------------------------------------------------
void CAnimating::Scorch( int rate, int floor )
{
	color32 color = GetRenderColor();

	if( color.r > floor )
		color.r -= rate;

	if( color.g > floor )
		color.g -= rate;

	if( color.b > floor )
		color.b -= rate;

	SetRenderColor( color.r, color.g, color.b );
}


//=========================================================
// SelectWeightedSequence
//=========================================================
int CAnimating::SelectWeightedSequence ( Activity activity )
{
	Assert( activity != ACT_INVALID );
	Assert( GetModelPtr() );
	return g_helpfunc.SelectWeightedSequence(GetModelPtr(), activity, GetSequence());
}


int CAnimating::SelectWeightedSequence ( Activity activity, int curSequence )
{
	Assert( activity != ACT_INVALID );
	Assert( GetModelPtr() );
	return g_helpfunc.SelectWeightedSequence( GetModelPtr(), activity, curSequence );
}

float CAnimating::SetPoseParameter( CStudioHdr *pStudioHdr, int iParameter, float flValue )
{
	if ( !pStudioHdr )
	{
		return flValue;
	}

	if (iParameter >= 0)
	{
		float flNewValue;
		flValue = Studio_SetPoseParameter( pStudioHdr, iParameter, flValue, flNewValue );
		*(m_flPoseParameter.ptr)[iParameter] = flNewValue;
	}

	return flValue;
}

float CAnimating::GetPoseParameter( int iParameter )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );

	if ( !pstudiohdr )
	{
		Assert(!"CBaseAnimating::GetPoseParameter: model missing");
		return 0.0;
	}

	if ( !pstudiohdr->SequencesAvailable() )
	{
		return 0;
	}

	if (iParameter >= 0)
	{
		return Studio_GetPoseParameter( pstudiohdr, iParameter, *(m_flPoseParameter.ptr)[iParameter]);
	}

	return 0.0;
}
