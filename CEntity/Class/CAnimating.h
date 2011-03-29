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

#ifndef _INCLUDE_CANIMATING_H_
#define _INCLUDE_CANIMATING_H_

#include "CEntity.h"
#include "bone_setup.h"


class CAnimating : public CEntity
{
public:
	CE_DECLARE_CLASS(CAnimating, CEntity);

public:
	virtual void StudioFrameAdvance();
	virtual void StudioFrameAdvanceManual(float flInterval);
	virtual void Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false );

public:
	float			GetCycle() const;
	void			SetCycle(float flCycle);
	float			GetAnimTimeInterval() const;
	bool			GetIntervalMovement( float flIntervalUsed, bool &bMoveSeqFinished, Vector &newPosition, QAngle &newAngles );
	float			GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence );
	inline float	GetSequenceCycleRate( int iSequence ) { return GetSequenceCycleRate(GetModelPtr(),iSequence); }
	float			SequenceDuration( CStudioHdr *pStudioHdr, int iSequence );
	const float*	GetPoseParameterArray() { return *m_flPoseParameter.ptr; }
	bool			IsOnFire() { return ( (GetFlags() & FL_ONFIRE) != 0 ); }
	void Scorch( int rate, int floor );

	int		SelectWeightedSequence ( Activity activity );
	int		SelectWeightedSequence ( Activity activity, int curSequence );

public:
	CStudioHdr *		GetModelPtr();
	inline int			GetSequence() { return m_nSequence; }


public:
	DECLARE_DEFAULTHEADER(StudioFrameAdvance, void, ());
	DECLARE_DEFAULTHEADER(Ignite, void,( float flFlameLifetime, bool bNPCOnly, float flSize , bool bCalledByLevelDesigner));

public:
	DECLARE_DEFAULTHEADER_DETOUR(StudioFrameAdvanceManual, void, (float flInterval));


protected: //Sendprops
	DECLARE_SENDPROP(float,m_flPlaybackRate);
	DECLARE_SENDPROP(float *,m_flPoseParameter);

	
protected: //Datamaps
	DECLARE_DATAMAP(float, m_flCycle);
	DECLARE_DATAMAP_OFFSET(CStudioHdr, m_pStudioHdr, 0x414);
	DECLARE_DATAMAP(int, m_nSequence);
	DECLARE_DATAMAP(bool, m_bSequenceLoops);

};



inline float CAnimating::GetCycle() const
{
	return m_flCycle;
}

inline void CAnimating::SetCycle(float flCycle)
{
	m_flCycle = flCycle;
}

//-----------------------------------------------------------------------------
// Purpose: return a pointer to an updated studiomdl cache cache
//-----------------------------------------------------------------------------
inline CStudioHdr *CAnimating::GetModelPtr( void ) 
{ 
	return ( m_pStudioHdr.ptr && m_pStudioHdr.ptr->IsValid() ) ? m_pStudioHdr.ptr : NULL;
}


#endif // _INCLUDE_CANIMATING_H_
