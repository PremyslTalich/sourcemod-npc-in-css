
#include "npc_basezombie.h"
#include "CAI_utils.h"



//CE_LINK_ENTITY_TO_CLASS( npc_zombie, CE_Zombie );

#define ZOMBIE_BULLET_DAMAGE_SCALE 0.5f

int g_interactionZombieMeleeWarning;

envelopePoint_t envDefaultZombieMoanVolumeFast[] =
{
	{	1.0f, 1.0f,
		0.1f, 0.1f,
	},
	{	0.0f, 0.0f,
		0.2f, 0.3f,
	},
};

envelopePoint_t envDefaultZombieMoanVolume[] =
{
	{	1.0f, 0.1f,
		0.1f, 0.1f,
	},
	{	1.0f, 1.0f,
		0.2f, 0.2f,
	},
	{	0.0f, 0.0f,
		0.3f, 0.4f,
	},
};


// if the zombie doesn't find anything closer than this, it doesn't swat.
#define ZOMBIE_FARTHEST_PHYSICS_OBJECT	40.0*12.0
#define ZOMBIE_PHYSICS_SEARCH_DEPTH	100

// Don't swat objects unless player is closer than this.
#define ZOMBIE_PLAYER_MAX_SWAT_DIST		1000

//
// How much health a Zombie torso gets when a whole zombie is broken
// It's whole zombie's MAX Health * this value
#define ZOMBIE_TORSO_HEALTH_FACTOR 0.5

//
// When the zombie has health < m_iMaxHealth * this value, it will
// try to release its headcrab.
#define ZOMBIE_RELEASE_HEALTH_FACTOR	0.5

//
// The heaviest physics object that a zombie should try to swat. (kg)
#define ZOMBIE_MAX_PHYSOBJ_MASS		60

//
// Zombie tries to get this close to a physics object's origin to swat it
#define ZOMBIE_PHYSOBJ_SWATDIST		80

//
// Because movement code sometimes doesn't get us QUITE where we
// want to go, the zombie tries to get this close to a physics object
// Zombie will end up somewhere between PHYSOBJ_MOVE_TO_DIST & PHYSOBJ_SWATDIST
#define ZOMBIE_PHYSOBJ_MOVE_TO_DIST	48

//
// How long between physics swat attacks (in seconds). 
#define ZOMBIE_SWAT_DELAY			5


//
// After taking damage, ignore further damage for n seconds. This keeps the zombie
// from being interrupted while.
//
#define ZOMBIE_FLINCH_DELAY			3


#define ZOMBIE_BURN_TIME		10 // If ignited, burn for this many seconds
#define ZOMBIE_BURN_TIME_NOISE	2  // Give or take this many seconds.


//=========================================================
// private activities
//=========================================================
int CE_BaseZombie::ACT_ZOM_SWATLEFTMID;
int CE_BaseZombie::ACT_ZOM_SWATRIGHTMID;
int CE_BaseZombie::ACT_ZOM_SWATLEFTLOW;
int CE_BaseZombie::ACT_ZOM_SWATRIGHTLOW;
int CE_BaseZombie::ACT_ZOM_RELEASECRAB;
int CE_BaseZombie::ACT_ZOM_FALL;


//=========================================================
// For a couple of reasons, we keep a running count of how
// many zombies in the world are angry at any given time.
//=========================================================
static int s_iAngryZombies = 0;

//=========================================================
//=========================================================




int AE_ZOMBIE_ATTACK_RIGHT;
int AE_ZOMBIE_ATTACK_LEFT;
int AE_ZOMBIE_ATTACK_BOTH;
int AE_ZOMBIE_SWATITEM;
int AE_ZOMBIE_STARTSWAT;
int AE_ZOMBIE_STEP_LEFT;
int AE_ZOMBIE_STEP_RIGHT;
int AE_ZOMBIE_SCUFF_LEFT;
int AE_ZOMBIE_SCUFF_RIGHT;
int AE_ZOMBIE_ATTACK_SCREAM;
int AE_ZOMBIE_GET_UP;
int AE_ZOMBIE_POUND;
int AE_ZOMBIE_ALERTSOUND;
int AE_ZOMBIE_POPHEADCRAB;


//---------------------------------------------------------
//---------------------------------------------------------
int CE_BaseZombie::g_numZombies = 0;


//---------------------------------------------------------
//---------------------------------------------------------


CE_BaseZombie::CE_BaseZombie()
{
	m_fIsTorso = false;
	m_fIsHeadless = false;
	m_flNextFlinch = 0.0f;
	m_bHeadShot = false;
	m_flBurnDamage = 0.0f;
	m_flBurnDamageResetTime = 0.0f;
	m_hPhysicsEnt = NULL;
	m_flNextMoanSound = 0.0f;
	m_flNextSwat = 0.0f;
	m_flNextSwatScan = 0.0f;
	m_crabHealth = 0.0f;
	m_flMoanPitch = 0.0f;
	m_iMoanSound = 0;
	m_hObstructor = NULL;
	m_bIsSlumped= false;

	m_iMoanSound = g_numZombies;

	g_numZombies++;
}

//---------------------------------------------------------
//---------------------------------------------------------
CE_BaseZombie::~CE_BaseZombie()
{
	g_numZombies--;
}


//---------------------------------------------------------
// The closest physics object is chosen that is:
// <= MaxMass in Mass
// Between the zombie and the enemy
// not too far from a direct line to the enemy.
//---------------------------------------------------------
bool CE_BaseZombie::FindNearestPhysicsObject( int iMaxMass )
{
	CEntity		*pList[ ZOMBIE_PHYSICS_SEARCH_DEPTH ];
	CEntity		*pNearest = NULL;
	float			flDist;
	IPhysicsObject	*pPhysObj;
	int				i;
	Vector			vecDirToEnemy;
	Vector			vecDirToObject;

	if ( !CanSwatPhysicsObjects() || !GetEnemy() )
	{
		// Can't swat, or no enemy, so no swat.
		m_hPhysicsEnt = NULL;
		return false;
	}

	vecDirToEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
	float dist = VectorNormalize(vecDirToEnemy);
	vecDirToEnemy.z = 0;

	if( dist > ZOMBIE_PLAYER_MAX_SWAT_DIST )
	{
		// Player is too far away. Don't bother 
		// trying to swat anything at them until
		// they are closer.
		return false;
	}

	float flNearestDist = min( dist, ZOMBIE_FARTHEST_PHYSICS_OBJECT * 0.5 );
	Vector vecDelta( flNearestDist, flNearestDist, GetHullHeight() * 2.0 );

	class CZombieSwatEntitiesEnum : public CFlaggedEntitiesEnum
	{
	public:
		CZombieSwatEntitiesEnum( CEntity **pList, int listMax, int iMaxMass )
		 :	CFlaggedEntitiesEnum( pList, listMax, 0 ),
			m_iMaxMass( iMaxMass )
		{
		}

		virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity )
		{
			CEntity *pEntity = CE_EntityFromEntityHandle_NoStatic( pHandleEntity);
			if ( pEntity && 
				 pEntity->VPhysicsGetObject() && 
				 pEntity->VPhysicsGetObject()->GetMass() <= m_iMaxMass && 
				 pEntity->VPhysicsGetObject()->IsAsleep() && 
				 pEntity->VPhysicsGetObject()->IsMoveable() )
			{
				return CFlaggedEntitiesEnum::EnumElement( pHandleEntity );
			}
			return ITERATION_CONTINUE;
		}

		int m_iMaxMass;
	};

	CZombieSwatEntitiesEnum swatEnum( pList, ZOMBIE_PHYSICS_SEARCH_DEPTH, iMaxMass );

	int count = UTIL_EntitiesInBox( GetAbsOrigin() - vecDelta, GetAbsOrigin() + vecDelta, &swatEnum );

	// magically know where they are
	Vector vecZombieKnees;
	CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.25f ), &vecZombieKnees );

	for( i = 0 ; i < count ; i++ )
	{
		pPhysObj = pList[ i ]->VPhysicsGetObject();

		Assert( !( !pPhysObj || pPhysObj->GetMass() > iMaxMass || !pPhysObj->IsAsleep() ) );

		Vector center = pList[ i ]->WorldSpaceCenter();
		flDist = UTIL_DistApprox2D( GetAbsOrigin(), center );

		if( flDist >= flNearestDist )
			continue;

		// This object is closer... but is it between the player and the zombie?
		vecDirToObject = pList[ i ]->WorldSpaceCenter() - GetAbsOrigin();
		VectorNormalize(vecDirToObject);
		vecDirToObject.z = 0;

		if( DotProduct( vecDirToEnemy, vecDirToObject ) < 0.8 )
			continue;

		if( flDist >= UTIL_DistApprox2D( center, GetEnemy()->GetAbsOrigin() ) )
			continue;

		// don't swat things where the highest point is under my knees
		// NOTE: This is a rough test; a more exact test is going to occur below
		if ( (center.z + pList[i]->BoundingRadius()) < vecZombieKnees.z )
			continue;

		// don't swat things that are over my head.
		if( center.z > EyePosition().z )
			continue;

		vcollide_t *pCollide = modelinfo->GetVCollide( pList[i]->GetModelIndex() );
		
		Vector objMins, objMaxs;
		physcollision->CollideGetAABB( &objMins, &objMaxs, pCollide->solids[0], pList[i]->GetAbsOrigin(), pList[i]->GetAbsAngles() );

		if ( objMaxs.z < vecZombieKnees.z )
			continue;

		if ( !FVisible_Entity( pList[i]->BaseEntity() ) )
			continue;

		// Make this the last check, since it makes a string.
		// Don't swat server ragdolls!
		if ( FClassnameIs( pList[ i ], "physics_prop_ragdoll" ) )
			continue;
			
		if ( FClassnameIs( pList[ i ], "prop_ragdoll" ) )
			continue;

		// The object must also be closer to the zombie than it is to the enemy
		pNearest = pList[ i ];
		flNearestDist = flDist;
	}

	if(pNearest == NULL)
	{
		m_hPhysicsEnt = NULL;
		return false;
	} else {
		m_hPhysicsEnt = pNearest->BaseEntity();
		return true;
	}
}



//-----------------------------------------------------------------------------
// Purpose: Returns this monster's place in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CE_BaseZombie::Classify( void )
{
	if ( IsSlumped() )
		return CLASS_NONE;

	return( CLASS_ZOMBIE ); 
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Disposition_t CE_BaseZombie::IRelationType( CBaseEntity *pTarget )
{
	// Slumping should not affect Zombie's opinion of others
	if ( IsSlumped() )
	{
		m_bIsSlumped = false;
		Disposition_t result = BaseClass::IRelationType( pTarget );
		m_bIsSlumped = true;
		return result;
	}

	return BaseClass::IRelationType( pTarget );
}


//-----------------------------------------------------------------------------
// Purpose: Returns the maximum yaw speed based on the monster's current activity.
//-----------------------------------------------------------------------------
float CE_BaseZombie::MaxYawSpeed( void )
{
	if( m_fIsTorso )
	{
		return( 60 );
	}
	else if (IsMoving() && HasPoseParameter( GetSequence(), m_poseMove_Yaw ))
	{
		return( 15 );
	}
	else
	{
		switch( GetActivity() )
		{
		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
			return 100;
			break;
		case ACT_RUN:
			return 15;
			break;
		case ACT_WALK:
		case ACT_IDLE:
			return 25;
			break;
		case ACT_RANGE_ATTACK1:
		case ACT_RANGE_ATTACK2:
		case ACT_MELEE_ATTACK1:
		case ACT_MELEE_ATTACK2:
			return 120;
		default:
			return 90;
			break;
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: turn in the direction of movement
// Output :
//-----------------------------------------------------------------------------
bool CE_BaseZombie::OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval )
{
	if (!HasPoseParameter( GetSequence(), m_poseMove_Yaw ))
	{
		return BaseClass::OverrideMoveFacing( move, flInterval );
	}

	// required movement direction
	float flMoveYaw = UTIL_VecToYaw( move.dir );
	float idealYaw = UTIL_AngleMod( flMoveYaw );

	if (GetEnemy())
	{
		float flEDist = UTIL_DistApprox2D( WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter() );

		if (flEDist < 256.0)
		{
			float flEYaw = UTIL_VecToYaw( GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter() );

			if (flEDist < 128.0)
			{
				idealYaw = flEYaw;
			}
			else
			{
				idealYaw = flMoveYaw + UTIL_AngleDiff( flEYaw, flMoveYaw ) * (2 - flEDist / 128.0);
			}

			//DevMsg("was %.0f now %.0f\n", flMoveYaw, idealYaw );
		}
	}

	GetMotor()->SetIdealYawAndUpdate( idealYaw );

	// find movement direction to compensate for not being turned far enough
	float fSequenceMoveYaw = GetSequenceMoveYaw( GetSequence() );
	float flDiff = UTIL_AngleDiff( flMoveYaw, GetLocalAngles().y + fSequenceMoveYaw );
	SetPoseParameter( m_poseMove_Yaw, GetPoseParameter( m_poseMove_Yaw ) + flDiff );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: For innate melee attack
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CE_BaseZombie::MeleeAttack1Conditions ( float flDot, float flDist )
{
	float range = GetClawAttackRange();

	if (flDist > range )
	{
		// Translate a hit vehicle into its passenger if found
		return COND_TOO_FAR_TO_ATTACK;
	}

	if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	// Build a cube-shaped hull, the same hull that ClawAttack() is going to use.
	Vector vecMins = GetHullMins();
	Vector vecMaxs = GetHullMaxs();
	vecMins.z = vecMins.x;
	vecMaxs.z = vecMaxs.x;

	Vector forward;
	GetVectors( &forward, NULL, NULL );

	trace_t	tr;
	CE_CTraceFilterNav traceFilter( this, false, this->BaseEntity(), COLLISION_GROUP_NONE );
	UTIL_TraceHull( WorldSpaceCenter(), WorldSpaceCenter() + forward * GetClawAttackRange(), vecMins, vecMaxs, MASK_NPCSOLID, &traceFilter, &tr );

	CEntity *m_pEnt = CEntity::Instance(tr.m_pEnt);

	if( tr.fraction == 1.0 || !tr.m_pEnt || !m_pEnt)
	{
		// This attack would miss completely. Trick the zombie into moving around some more.
		return COND_TOO_FAR_TO_ATTACK;
	}

	//CE_TODO
	if( m_pEnt == GetEnemy() || 
		m_pEnt->IsNPC() || 
		( m_pEnt->m_takedamage == DAMAGE_YES && (FClassnameIs(m_pEnt,"prop_dynamic") || FClassnameIs(m_pEnt,"prop_physics")) ) )
	{
		// -Let the zombie swipe at his enemy if he's going to hit them.
		// -Also let him swipe at NPC's that happen to be between the zombie and the enemy. 
		//  This makes mobs of zombies seem more rowdy since it doesn't leave guys in the back row standing around.
		// -Also let him swipe at things that takedamage, under the assumptions that they can be broken.
		return COND_CAN_MELEE_ATTACK1;
	}

	Vector vecTrace = tr.endpos - tr.startpos;
	float lenTraceSq = vecTrace.Length2DSqr();

	//CE_TODO , vehicle
	/*if ( GetEnemy() && GetEnemy()->MyCombatCharacterPointer() && tr.m_pEnt == static_cast<CCombatCharacter *>(GetEnemy())->GetVehicleEntity() )
	{
		if ( lenTraceSq < Square( GetClawAttackRange() * 0.75f ) )
		{
			return COND_CAN_MELEE_ATTACK1;
		}
	}*/

	if(m_pEnt->IsBSPModel() )
	{
		// The trace hit something solid, but it's not the enemy. If this item is closer to the zombie than
		// the enemy is, treat this as an obstruction.
		Vector vecToEnemy = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();

		if( lenTraceSq < vecToEnemy.Length2DSqr() )
		{
			return COND_ZOMBIE_LOCAL_MELEE_OBSTRUCTION;
		}
	}

	// Move around some more
	return COND_TOO_FAR_TO_ATTACK;
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define ZOMBIE_BUCKSHOT_TRIPLE_DAMAGE_DIST	96.0f // Triple damage from buckshot at 8 feet (headshot only)
float CE_BaseZombie::GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info )
{
	switch( iHitGroup )
	{
	case HITGROUP_HEAD:
		{
			if( info.GetDamageType() & DMG_BUCKSHOT )
			{
				float flDist = FLT_MAX;

				CEntity *attacker = CEntity::Instance(info.GetAttacker());
				if( attacker)
				{
					flDist = ( GetAbsOrigin() - attacker->GetAbsOrigin() ).Length();
				}

				if( flDist <= ZOMBIE_BUCKSHOT_TRIPLE_DAMAGE_DIST )
				{
					return 3.0f;
				}
			}
			else
			{
				return 2.0f;
			}
		}
	}

	return BaseClass::GetHitgroupDamageMultiplier( iHitGroup, info );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CE_BaseZombie::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	CTakeDamageInfo infoCopy = info;

	// Keep track of headshots so we can determine whether to pop off our headcrab.
	if (ptr->hitgroup == HITGROUP_HEAD)
	{
		m_bHeadShot = true;
	}

	if( infoCopy.GetDamageType() & DMG_BUCKSHOT )
	{
		// Zombie gets across-the-board damage reduction for buckshot. This compensates for the recent changes which
		// make the shotgun much more powerful, and returns the zombies to a level that has been playtested extensively.(sjb)
		// This normalizes the buckshot damage to what it used to be on normal (5 dmg per pellet. Now it's 8 dmg per pellet). 
		infoCopy.ScaleDamage( 0.625 );
	}

	BaseClass::TraceAttack( infoCopy, vecDir, ptr );
}



//-----------------------------------------------------------------------------
// Purpose: A zombie has taken damage. Determine whether he should split in half
// Input  : 
// Output : bool, true if yes.
//-----------------------------------------------------------------------------
bool CE_BaseZombie::ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold )
{
	if ( info.GetDamageType() & DMG_REMOVENORAGDOLL )
		return false;

	if ( m_fIsTorso )
	{
		// Already split.
		return false;
	}

	// Not if we're in a dss
	if ( IsRunningDynamicInteraction() )
		return false;

	// Break in half IF:
	// 
	// Take half or more of max health in DMG_BLAST
	if( (info.GetDamageType() & DMG_BLAST) && flDamageThreshold >= 0.5 )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: A zombie has taken damage. Determine whether he release his headcrab.
// Output : YES, IMMEDIATE, or SCHEDULED (see HeadcrabRelease_t)
//-----------------------------------------------------------------------------
HeadcrabRelease_t CE_BaseZombie::ShouldReleaseHeadcrab( const CTakeDamageInfo &info, float flDamageThreshold )
{
	if ( m_iHealth <= 0 )
	{
		if ( info.GetDamageType() & DMG_REMOVENORAGDOLL )
			return RELEASE_NO;

		//CE_TODO
		/*if ( info.GetDamageType() & DMG_SNIPER )
			return RELEASE_RAGDOLL;*/

		// If I was killed by a bullet...
		if ( info.GetDamageType() & DMG_BULLET )
		{
			if( m_bHeadShot ) 
			{
				if( flDamageThreshold > 0.25 )
				{
					// Enough force to kill the crab.
					return RELEASE_RAGDOLL;
				}
			}
			else
			{
				// Killed by a shot to body or something. Crab is ok!
				return RELEASE_IMMEDIATE;
			}
		}

		// If I was killed by an explosion, release the crab.
		if ( info.GetDamageType() & DMG_BLAST )
		{
			return RELEASE_RAGDOLL;
		}


		if ( m_fIsTorso && IsChopped( info ) )
		{
			return RELEASE_RAGDOLL_SLICED_OFF;
		}
	}

	return RELEASE_NO;
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : int
//-----------------------------------------------------------------------------
#define ZOMBIE_SCORCH_RATE		8
#define ZOMBIE_MIN_RENDERCOLOR	50
int CE_BaseZombie::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	if( inputInfo.GetDamageType() & DMG_BURN )
	{
		// If a zombie is on fire it only takes damage from the fire that's attached to it. (DMG_DIRECT)
		// This is to stop zombies from burning to death 10x faster when they're standing around
		// 10 fire entities.
		if( IsOnFire() && !(inputInfo.GetDamageType() & DMG_DIRECT) )
		{
			return 0;
		}
		
		Scorch( ZOMBIE_SCORCH_RATE, ZOMBIE_MIN_RENDERCOLOR );
	}

	// Take some percentage of damage from bullets (unless hit in the crab). Always take full buckshot & sniper damage
	if ( !m_bHeadShot && (info.GetDamageType() & DMG_BULLET) && !(info.GetDamageType() & (DMG_BUCKSHOT/*|DMG_SNIPER*/)) )
	{
		info.ScaleDamage( ZOMBIE_BULLET_DAMAGE_SCALE );
	}

	if ( ShouldIgnite( info ) )
	{
		Ignite( 100.0f );
	}

	int tookDamage = BaseClass::OnTakeDamage_Alive( info );

	// flDamageThreshold is what percentage of the creature's max health
	// this amount of damage represents. (clips at 1.0)
	float flDamageThreshold = min( 1, info.GetDamage() / m_iMaxHealth );
	
	// Being chopped up by a sharp physics object is a pretty special case
	// so we handle it with some special code. Mainly for 
	// Ravenholm's helicopter traps right now (sjb).
	bool bChopped = IsChopped(info);
	bool bSquashed = IsSquashed(info);
	bool bKilledByVehicle = ( ( info.GetDamageType() & DMG_VEHICLE ) != 0 );

	if( !m_fIsTorso && (bChopped || bSquashed) && !bKilledByVehicle && !(info.GetDamageType() & DMG_REMOVENORAGDOLL) )
	{
		if( bChopped )
		{
			EmitSound( "E3_Phystown.Slicer" );
		}

		DieChopped( info );
	}
	else
	{
		HeadcrabRelease_t release = ShouldReleaseHeadcrab( info, flDamageThreshold );
		
		switch( release )
		{
		case RELEASE_IMMEDIATE:
			ReleaseHeadcrab( EyePosition(), vec3_origin, true, true );
			break;

		case RELEASE_RAGDOLL:
			// Go a little easy on headcrab ragdoll force. They're light!
			ReleaseHeadcrab( EyePosition(), inputInfo.GetDamageForce() * 0.25, true, false, true );
			break;

		case RELEASE_RAGDOLL_SLICED_OFF:
			{
				EmitSound( "E3_Phystown.Slicer" );
				Vector vecForce = inputInfo.GetDamageForce() * 0.1;
				vecForce += Vector( 0, 0, 2000.0 );
				ReleaseHeadcrab( EyePosition(), vecForce, true, false, true );
			}
			break;

		case RELEASE_VAPORIZE:
			RemoveHead();
			break;

		case RELEASE_SCHEDULED:
			SetCondition( COND_ZOMBIE_RELEASECRAB );
			break;

		default:
			break;
		}

		if( ShouldBecomeTorso( info, flDamageThreshold ) )
		{
			//CE_TODO
			bool bHitByCombineCannon = false;//(inputInfo.GetAmmoType() == GetAmmoDef()->Index("CombineHeavyCannon"));

			if ( true /*CanBecomeLiveTorso()*/ )
			{
				//BecomeTorso( vec3_origin, inputInfo.GetDamageForce() * 0.50 );

				if ( ( info.GetDamageType() & DMG_BLAST) && random->RandomInt( 0, 1 ) == 0 )
				{
					Ignite( 5.0 + random->RandomFloat( 0.0, 5.0 ) );
				}

				if (flDamageThreshold >= 1.0)
				{
					m_iHealth = 0;
					//CE_TODO
					//BecomeRagdollOnClient( info.GetDamageForce() );
				}
			}
			else if ( random->RandomInt(1, 3) == 1 )
				DieChopped( info );
		}
	}

	if( tookDamage > 0 && (info.GetDamageType() & (DMG_BURN|DMG_DIRECT)) && m_ActBusyBehavior.IsActive() ) 
	{
		//!!!HACKHACK- Stuff a light_damage condition if an actbusying zombie takes direct burn damage. This will cause an
		// ignited zombie to 'wake up' and rise out of its actbusy slump. (sjb)
		SetCondition( COND_LIGHT_DAMAGE );
	}

	// IMPORTANT: always clear the headshot flag after applying damage. No early outs!
	m_bHeadShot = false;

	return tookDamage;
}


// CE_BaseZombie










//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( base_zombie, CE_BaseZombie )

	DECLARE_TASK( TASK_ZOMBIE_DELAY_SWAT )
	DECLARE_TASK( TASK_ZOMBIE_SWAT_ITEM )
	DECLARE_TASK( TASK_ZOMBIE_GET_PATH_TO_PHYSOBJ )
	DECLARE_TASK( TASK_ZOMBIE_DIE )
	DECLARE_TASK( TASK_ZOMBIE_RELEASE_HEADCRAB )
	DECLARE_TASK( TASK_ZOMBIE_WAIT_POST_MELEE )

	DECLARE_ACTIVITY( ACT_ZOM_SWATLEFTMID )
	DECLARE_ACTIVITY( ACT_ZOM_SWATRIGHTMID )
	DECLARE_ACTIVITY( ACT_ZOM_SWATLEFTLOW )
	DECLARE_ACTIVITY( ACT_ZOM_SWATRIGHTLOW )
	DECLARE_ACTIVITY( ACT_ZOM_RELEASECRAB )
	DECLARE_ACTIVITY( ACT_ZOM_FALL )

	DECLARE_CONDITION( COND_ZOMBIE_CAN_SWAT_ATTACK )
	DECLARE_CONDITION( COND_ZOMBIE_RELEASECRAB )
	DECLARE_CONDITION( COND_ZOMBIE_LOCAL_MELEE_OBSTRUCTION )

	//Adrian: events go here
	DECLARE_ANIMEVENT( AE_ZOMBIE_ATTACK_RIGHT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_ATTACK_LEFT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_ATTACK_BOTH )
	DECLARE_ANIMEVENT( AE_ZOMBIE_SWATITEM )
	DECLARE_ANIMEVENT( AE_ZOMBIE_STARTSWAT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_STEP_LEFT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_STEP_RIGHT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_SCUFF_LEFT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_SCUFF_RIGHT )
	DECLARE_ANIMEVENT( AE_ZOMBIE_ATTACK_SCREAM )
	DECLARE_ANIMEVENT( AE_ZOMBIE_GET_UP )
	DECLARE_ANIMEVENT( AE_ZOMBIE_POUND )
	DECLARE_ANIMEVENT( AE_ZOMBIE_ALERTSOUND )
	DECLARE_ANIMEVENT( AE_ZOMBIE_POPHEADCRAB )

	//CE_TODO
	//DECLARE_INTERACTION( g_interactionZombieMeleeWarning )

	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_MOVE_SWATITEM,

		"	Tasks"
		"		TASK_ZOMBIE_DELAY_SWAT			3"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_ZOMBIE_GET_PATH_TO_PHYSOBJ	0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_FACE_ENEMY					0"
		"		TASK_ZOMBIE_SWAT_ITEM			0"
		"	"
		"	Interrupts"
		"		COND_ZOMBIE_RELEASECRAB"
		"		COND_ENEMY_DEAD"
		"		COND_NEW_ENEMY"
	)

	//=========================================================
	// SwatItem
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_SWATITEM,

		"	Tasks"
		"		TASK_ZOMBIE_DELAY_SWAT			3"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY"
		"		TASK_FACE_ENEMY					0"
		"		TASK_ZOMBIE_SWAT_ITEM			0"
		"	"
		"	Interrupts"
		"		COND_ZOMBIE_RELEASECRAB"
		"		COND_ENEMY_DEAD"
		"		COND_NEW_ENEMY"
	)

	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_ATTACKITEM,

		"	Tasks"
		"		TASK_FACE_ENEMY					0"
		"		TASK_MELEE_ATTACK1				0"
		"	"
		"	Interrupts"
		"		COND_ZOMBIE_RELEASECRAB"
		"		COND_ENEMY_DEAD"
		"		COND_NEW_ENEMY"
	)

	//=========================================================
	// ChaseEnemy
	//=========================================================
#ifdef HL2_EPISODIC
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_CHASE_ENEMY,

		"	Tasks"
		"		 TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		 TASK_SET_TOLERANCE_DISTANCE	24"
		"		 TASK_GET_CHASE_PATH_TO_ENEMY	600"
		"		 TASK_RUN_PATH					0"
		"		 TASK_WAIT_FOR_MOVEMENT			0"
		"		 TASK_FACE_ENEMY				0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TASK_FAILED"
		"		COND_ZOMBIE_CAN_SWAT_ATTACK"
		"		COND_ZOMBIE_RELEASECRAB"
		"		COND_HEAVY_DAMAGE"
	)
#else 
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_CHASE_ENEMY,

		"	Tasks"
		"		 TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_CHASE_ENEMY_FAILED"
		"		 TASK_SET_TOLERANCE_DISTANCE	24"
		"		 TASK_GET_CHASE_PATH_TO_ENEMY	600"
		"		 TASK_RUN_PATH					0"
		"		 TASK_WAIT_FOR_MOVEMENT			0"
		"		 TASK_FACE_ENEMY				0"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_UNREACHABLE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TOO_CLOSE_TO_ATTACK"
		"		COND_TASK_FAILED"
		"		COND_ZOMBIE_CAN_SWAT_ATTACK"
		"		COND_ZOMBIE_RELEASECRAB"
	)
#endif // HL2_EPISODIC


	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_RELEASECRAB,

		"	Tasks"
		"		TASK_PLAY_PRIVATE_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_ZOM_RELEASECRAB"
		"		TASK_ZOMBIE_RELEASE_HEADCRAB				0"
		"		TASK_ZOMBIE_DIE								0"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
	)


	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_MOVE_TO_AMBUSH,

		"	Tasks"
		"		TASK_WAIT						1.0" // don't react as soon as you see the player.
		"		TASK_FIND_COVER_FROM_ENEMY		0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		"		TASK_TURN_LEFT					180"
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_ZOMBIE_WAIT_AMBUSH"
		"	"
		"	Interrupts"
		"		COND_TASK_FAILED"
		"		COND_NEW_ENEMY"
	)


	//=========================================================
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_WAIT_AMBUSH,

		"	Tasks"
		"		TASK_WAIT_FACE_ENEMY	99999"
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
	)

	//=========================================================
	// Wander around for a while so we don't look stupid. 
	// this is done if we ever lose track of our enemy.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_WANDER_MEDIUM,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_WANDER						480384" // 4 feet to 32 feet
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		"		TASK_WAIT_PVS					0" // if the player left my PVS, just wait.
		"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_ZOMBIE_WANDER_MEDIUM" // keep doing it
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_WANDER_STANDOFF,

		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_WANDER						480384" // 4 feet to 32 feet
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_STOP_MOVING				0"
		"		TASK_WAIT_PVS					0" // if the player left my PVS, just wait.
		"	"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_ENEMY_DEAD"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_ZOMBIE_RELEASECRAB"
	)

	//=========================================================
	// If you fail to wander, wait just a bit and try again.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_WANDER_FAIL,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_WAIT				1"
		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_ZOMBIE_WANDER_MEDIUM"
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_ENEMY_DEAD"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_RANGE_ATTACK2"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_ZOMBIE_RELEASECRAB"
	)

	//=========================================================
	// Like the base class, only don't stop in the middle of 
	// swinging if the enemy is killed, hides, or new enemy.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_MELEE_ATTACK1,

		"	Tasks"
		"		TASK_STOP_MOVING		0"
		"		TASK_FACE_ENEMY			0"
		"		TASK_ANNOUNCE_ATTACK	1"	// 1 = primary attack
		"		TASK_MELEE_ATTACK1		0"
		"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_ZOMBIE_POST_MELEE_WAIT"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)

	//=========================================================
	// Make the zombie wait a frame after a melee attack, to
	// allow itself & it's enemy to test for dynamic scripted sequences.
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_ZOMBIE_POST_MELEE_WAIT,

		"	Tasks"
		"		TASK_ZOMBIE_WAIT_POST_MELEE		0"
	)

AI_END_CUSTOM_NPC()

