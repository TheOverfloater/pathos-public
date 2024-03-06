/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "vector.h"
#include "edict.h"
#include "playermove.h"
#include "com_math.h"
#include "trace.h"
#include "entity_state.h"
#include "bspv30file.h"
#include "constants.h"
#include "cache_model.h"
#include "brushmodel_shared.h"
#include "buttonbits.h"
#include "common.h"
#include "textures_shared.h"
#include "snd_shared.h"

// Trace distance for stuck checking in either axis or direction
static const Uint32 MAX_STUCK_CHECKDISTANCE = 8;
// Air accelerate max speed
static const Float AIR_ACCELERATE_MAX_SPEED = 30;

//=============================================
// @brief Default constructor
//
//=============================================
CPlayerMovement::CPlayerMovement( void ):
	m_frameTime(0),
	m_pPMInfo(nullptr),
	m_pPlayerState(nullptr),
	m_hullIndex(HULL_AUTO),
	m_maxSpeed(0),
	m_maxForwardSpeed(0),
	m_oldWaterLevel(0),
	m_planeZCap(0),
	m_isOnLadder(false),
	m_pLadder(nullptr),
	m_isServer(false),
	m_playSounds(false),
	m_isMultiplayer(false),
	m_pTextureMaterial(nullptr),
	m_pMovevars(nullptr)
{
	// Set default material
	m_defaultMaterial.materialname = "concrete";

	memset(&m_traceInterface, 0, sizeof(m_traceInterface));
	memset(&m_pmInterface, 0, sizeof(m_pmInterface));
}

//=============================================
// @brief Destructor
//
//=============================================
CPlayerMovement::~CPlayerMovement( void )
{
}

//=============================================
// @brief Destructor
//
//=============================================
void CPlayerMovement::Init( const trace_interface_t& traceFuncs, const pm_interface_t& pmFuncs, bool isServer )
{
	m_traceInterface = traceFuncs;
	m_pmInterface = pmFuncs;
	m_isServer = isServer;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Reset( void )
{
	m_pPlayerState = nullptr;
	m_pTextureMaterial = nullptr;
	m_pLadder = nullptr;
	m_isOnLadder = false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::DropPunchAngle( void )
{
	if( m_pPlayerState->punchangles.Length() <= 0.001 && m_pPlayerState->punchamount.Length() < 0.001)
		return;

	Math::VectorMA(m_pPlayerState->punchangles, m_frameTime, m_pPlayerState->punchamount, m_pPlayerState->punchangles);
	Float damping = 1.0 - (VIEW_PUNCH_DAMPING * m_frameTime);
	if(damping < 0)
		damping = 0;

	Math::VectorScale(m_pPlayerState->punchamount, damping, m_pPlayerState->punchamount);

	// Toroidal spring
	Float springmagnitude = VIEW_PUNCH_SPRING_CONSTANT * m_frameTime;
	springmagnitude = clamp(springmagnitude, 0, 2);

	Math::VectorMA(m_pPlayerState->punchamount, -springmagnitude, m_pPlayerState->punchangles, m_pPlayerState->punchamount);

	m_pPlayerState->punchangles[0] = clamp(m_pPlayerState->punchangles[0], -89, 89);
	m_pPlayerState->punchangles[1] = clamp(m_pPlayerState->punchangles[1], -179, 179);
	m_pPlayerState->punchangles[2] = clamp(m_pPlayerState->punchangles[2], -89, 89);
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::CheckParameters( void )
{
	float speed = (m_userCmd.sidemove * m_userCmd.sidemove) 
		+ (m_userCmd.upmove * m_userCmd.upmove);

	// Cap forward velocity to main velocity limit when walking backwards
	// No sprinting backwards basically
	if(m_userCmd.forwardmove < 0)
		speed += (m_userCmd.forwardmove * m_userCmd.forwardmove);

	speed = SDL_sqrt(speed);
	if(speed != 0 && speed > m_maxSpeed)
	{
		Float ratio = m_maxSpeed / speed;

		// Cap forward velocity to main velocity limit when walking backwards
		// No sprinting backwards basically
		if(m_userCmd.forwardmove < 0)
			m_userCmd.forwardmove *= ratio;

		m_userCmd.sidemove *= ratio;
		m_userCmd.upmove *= ratio;
	}

	// If we're moving forward, then cap at m_maxForwardSpeed, which changes
	// with sprint states
	if(m_userCmd.forwardmove > 0)
	{
		Float fwishspeed = m_userCmd.forwardmove * m_userCmd.forwardmove;
		fwishspeed = SDL_sqrt(fwishspeed);

		if(fwishspeed != 0 && fwishspeed > m_maxForwardSpeed)
		{
			Float ratio = m_maxForwardSpeed / fwishspeed;
			m_userCmd.forwardmove *= ratio;
		}
	}

	if(m_pPlayerState->flags & (FL_FROZEN|FL_ON_LADDER|FL_PARALYZED) || m_userCmd.buttons & IN_LEAN)
	{
		m_userCmd.forwardmove = 0;
		m_userCmd.sidemove = 0;
		m_userCmd.upmove = 0;
	}

	if(m_pPlayerState->flags & (FL_FROZEN))
	{
		Math::VectorClear(m_pPlayerState->velocity);
		m_pPlayerState->fallvelocity = 0;
	}

	// Drop punch angles
	DropPunchAngle();

	if(!(m_pPlayerState->flags & FL_DEAD))
	{
		Vector viewangles = m_pPlayerState->viewangles;
		if(viewangles[0] < 89 && (viewangles[0] + m_pPlayerState->punchangles[0]) > 89)
			m_pPlayerState->punchangles[0] = 0;
	}
	else
	{
		// Just copy the old view angles
		Math::VectorCopy(m_pPMInfo->oldangles, m_pPlayerState->viewangles);
		m_pPlayerState->view_offset[2] = DEAD_VIEWHEIGHT;
	}

	// Cap yaw
	if(m_pPlayerState->viewangles[YAW] > 180.0f)
		m_pPlayerState->viewangles[YAW] -= 360.0f;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::ReduceTimers( void )
{
	if(m_pPlayerState->timestepsound > 0)
	{
		m_pPlayerState->timestepsound -= m_userCmd.msec;
		if(m_pPlayerState->timestepsound < 0)
			m_pPlayerState->timestepsound = 0;
	}

	if(m_pPlayerState->ducktime > 0)
	{
		m_pPlayerState->ducktime -= m_userCmd.msec;
		if(m_pPlayerState->ducktime < 0)
			m_pPlayerState->ducktime = 0;
	}

	if(m_pPlayerState->swimtime > 0)
	{
		m_pPlayerState->swimtime -= m_userCmd.msec;
		if(m_pPlayerState->swimtime < 0)
			m_pPlayerState->swimtime = 0;
	}

	if(m_pPlayerState->waterjumptime > 0)
	{
		m_pPlayerState->waterjumptime -= m_userCmd.msec;
		if(m_pPlayerState->waterjumptime < 0)
			m_pPlayerState->waterjumptime = 0;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::FixCrouchStuck( const Vector& direction )
{
	entindex_t hitent = m_traceInterface.pfnTestPlayerPosition(m_hullIndex, FL_TRACE_NORMAL, m_pPlayerState->origin);
	if(hitent == NO_ENTITY_INDEX)
		return;

	Vector baseorg = m_pPlayerState->origin;
	for(Int32 i = 0; i < 36; i++)
	{
		m_pPlayerState->origin += direction;
		hitent = m_traceInterface.pfnTestPlayerPosition(m_hullIndex, FL_TRACE_NORMAL, m_pPlayerState->origin);
		if(hitent == NO_ENTITY_INDEX)
			return;
	}

	m_pPlayerState->origin = baseorg;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::CategorizePosition( void )
{
	// Re-check water
	CheckWater();

	if(m_pPlayerState->velocity[2] > 180)
	{
		// We're going up real fast, not onground
		m_pPlayerState->groundent = NO_ENTITY_INDEX;
	}
	else
	{
		// Try to lay us exactly on the ground
		Vector point = m_pPlayerState->origin;
		point[2] -= 2;

		trace_t trace;
		m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, point, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);
		if(trace.plane.normal[2] < m_planeZCap)
			m_pPlayerState->groundent = NO_ENTITY_INDEX;
		else
			m_pPlayerState->groundent = trace.hitentity;

		if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
		{
			// Clear waterjump time
			m_pPlayerState->waterjumptime = 0;

			// Drop us exactly onground if we hit something
			if(m_pPlayerState->waterlevel < WATERLEVEL_MID 
				&& !(trace.flags & FL_TR_ALLSOLID)
				&& !(m_pPlayerState->flags & (FL_ON_LADDER|FL_ON_BIKE)) 
				&& m_pPlayerState->movetype != MOVETYPE_NOCLIP 
				&& m_pPlayerState->movetype != MOVETYPE_FLY)
				Math::VectorCopy(trace.endpos, m_pPlayerState->origin);
		}

		// Signal that the entity is being touched
		if(trace.hitentity > 0)
			m_pmInterface.pfnAddToTouched(trace.hitentity, trace, m_pPlayerState->velocity);
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Ladder( void )
{
	if(m_pPlayerState->movetype == MOVETYPE_NOCLIP)
		return;

	if(!m_pLadder)
		return;

	const cache_model_t* pmodel = m_pmInterface.pfnGetModel(m_pLadder->modelindex);
	if(!pmodel)
		return;

	// Calculate center
	Vector mins, maxs, center;
	m_pmInterface.pfnGetModelBounds(*pmodel, mins, maxs);
	Math::VectorAdd(mins, maxs, center);
	Math::VectorScale(center, 0.5, center);

	// Change our movetype to flying
	m_pPlayerState->movetype = MOVETYPE_FLY;

	Vector floor = m_pPlayerState->origin;
	floor[2] += m_pPMInfo->player_mins[m_hullIndex][2] - 1;

	bool onfloor = false;
	if(m_traceInterface.pfnPointContents(floor, nullptr) == CONTENTS_SOLID)
		onfloor = true;

	m_pPlayerState->gravity = 0;

	trace_t trace;
	m_traceInterface.pfnTraceModel(m_pLadder->entindex, m_pPlayerState->origin, center, HULL_POINT, FL_TRACE_NORMAL, trace);
	if(trace.fraction != 1.0)
	{
		Float forward = 0;
		Float right = 0;

		if(m_userCmd.buttons & IN_BACK)
			forward -= PLAYER_CLIMB_SPEED;
		if(m_userCmd.buttons & IN_FORWARD)
			forward += PLAYER_CLIMB_SPEED;

		if(m_userCmd.buttons & IN_MOVELEFT)
			right -= PLAYER_CLIMB_SPEED;
		if(m_userCmd.buttons & IN_MOVERIGHT)
			right += PLAYER_CLIMB_SPEED;

		if(m_userCmd.buttons & IN_JUMP)
		{
			// Dislodge if jump key was pressed
			m_pPlayerState->movetype = MOVETYPE_WALK;
			Math::VectorScale(trace.plane.normal, 270, m_pPlayerState->velocity);
		}
		else
		{
			if(forward != 0 || right != 0)
			{
				Vector v_forward, v_right;
				Math::AngleVectors(m_pPlayerState->viewangles, &v_forward, &v_right, nullptr);

				Vector velocity;
				Math::VectorScale(v_forward, forward, velocity);
				Math::VectorMA(velocity, right, v_right, velocity);

				Vector perpendicular;
				Math::CrossProduct(Vector(0, 0, 1), trace.plane.normal, perpendicular);
				Math::VectorNormalize(perpendicular);

				Vector cross;
				Float dp = Math::DotProduct(velocity, trace.plane.normal);
				Math::VectorScale(trace.plane.normal, dp, cross);

				Vector lateral;
				Math::VectorSubtract(velocity, cross, lateral);

				Vector tmp;
				Math::CrossProduct(trace.plane.normal, perpendicular, tmp);
				Math::VectorMA(lateral, -dp, tmp, m_pPlayerState->velocity);

				if(onfloor && dp > 0)
					Math::VectorMA(m_pPlayerState->velocity, PLAYER_CLIMB_SPEED, trace.plane.normal, m_pPlayerState->velocity);
			}
			else
			{
				// Just clear any velocity we had
				m_pPlayerState->velocity.Clear();
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Toss_Bounce( void )
{
	if(m_pPlayerState->velocity[2] > 0)
		m_pPlayerState->groundent = NO_ENTITY_INDEX;

	// Do not simulate if on the ground and not moving
	if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
	{
		if(m_pPlayerState->velocity.IsZero() && m_pPlayerState->basevelocity.IsZero())
			return;
	}

	CheckVelocity();

	// Add gravity if needed
	if(m_pPlayerState->movetype != MOVETYPE_FLY)
		AddGravity();

	// Add base velocity to the entity
	Math::VectorAdd(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);

	CheckVelocity();

	Vector delta;
	Math::VectorScale(m_pPlayerState->velocity, m_frameTime, delta);
	Math::VectorSubtract(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);

	trace_t trace;
	PushEntity(delta, trace);

	CheckVelocity();

	if(trace.flags & FL_TR_ALLSOLID)
	{
		m_pPlayerState->groundent = trace.hitentity;
		m_pPlayerState->velocity.Clear();
		return;
	}
	else if(trace.fraction == 1.0)
	{
		CheckWater();
		return;
	}

	Float backoff;
	if(m_pPlayerState->movetype == MOVETYPE_BOUNCE)
		backoff = 2.0 - m_pPlayerState->friction;
	else
		backoff = 1.0;

	ClipVelocity(m_pPlayerState->velocity, trace.plane.normal, m_pPlayerState->velocity, backoff);

	// Stop if we're on the ground
	if(trace.plane.normal[2] >= m_planeZCap)
	{
		if(m_pPlayerState->velocity[2] < m_pMovevars->gravity * m_frameTime)
		{
			// add static fiction to rolling
			m_pPlayerState->groundent = trace.hitentity;
			m_pPlayerState->velocity[2] = 0;
		}

		Float velocity = Math::DotProduct(m_pPlayerState->velocity, m_pPlayerState->velocity);
		if(velocity < 900)
		{
			m_pPlayerState->groundent = trace.hitentity;
			m_pPlayerState->velocity.Clear();
		}
		else
		{
			Math::VectorScale(m_pPlayerState->velocity, (1.0 - trace.fraction) * m_frameTime * 0.9, delta);
			PushEntity(delta, trace);
		}
	}

	CheckWater();
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerMovement::Move_Fly( void )
{
	Int32 blocked = 0;
	Int32 numplanes = 0;
	Vector planenormals[MAX_CLIP_PLANES];

	// Save these
	Vector orig_velocity, primal_velocity;
	orig_velocity = primal_velocity = m_pPlayerState->velocity;

	Float allfraction = 1.0;
	Float timeleft = m_frameTime;

	// Perform movement
	for(Uint32 i = 0; i < NUM_FLYMOVE_BUMPS; i++)
	{
		if(m_pPlayerState->velocity.IsZero())
			break;

		Vector end;
		Math::VectorMA(m_pPlayerState->origin, timeleft, m_pPlayerState->velocity, end);

		trace_t trace;
		m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, end, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);
		allfraction += trace.fraction;

		if(trace.flags & FL_TR_ALLSOLID)
		{
			Math::VectorClear(m_pPlayerState->velocity);
			return BLOCKED_WALL;
		}

		// Copy endpos if trace succeeded
		if(trace.fraction > 0)
		{
			m_pPlayerState->origin = trace.endpos;
			orig_velocity = m_pPlayerState->velocity;
			numplanes = 0;
		}

		// Don't check further if we made it all the way
		if(trace.fraction == 1.0)
			break;

		m_pmInterface.pfnAddToTouched(trace.hitentity, trace, m_pPlayerState->velocity);

		// See if we'be blocked by the floor
		if(trace.plane.normal[2] >= m_planeZCap)
			blocked |= BLOCKED_FLOOR;

		// See if we're blocked by a wall
		if(trace.plane.normal[2] <= 0)
			blocked |= BLOCKED_WALL;

		// Reduce the amount of time
		timeleft -= trace.fraction*timeleft;

		if(numplanes >= MAX_CLIP_PLANES)
		{
			// We ran out of planes to collide against
			m_pPlayerState->velocity.Clear();
			break;
		}

		// Remember this plane
		planenormals[numplanes] = trace.plane.normal;
		numplanes++;

		// Reflect player velocity
		if(m_pPlayerState->movetype == MOVETYPE_WALK && (m_pPlayerState->groundent == NO_ENTITY_INDEX || m_pPlayerState->friction != 1))
		{
			Vector newvelocity;
			for(Int32 j = 0; j < numplanes; j++)
			{
				if(planenormals[j][2] >= m_planeZCap)
					ClipVelocity(orig_velocity, planenormals[j], newvelocity, 1.0f);
				else
					ClipVelocity(orig_velocity, planenormals[j], newvelocity, 1.0 + m_pMovevars->bounce * (1.0 - m_pPlayerState->friction));
			}

			// Set new velocity
			m_pPlayerState->velocity = newvelocity;
			orig_velocity = newvelocity;
		}
		else
		{
			Int32 j = 0;
			for(; j < numplanes; j++)
			{
				ClipVelocity(orig_velocity, planenormals[j], m_pPlayerState->velocity, 1.0f);

				Int32 k = 0;
				for(; k < numplanes; k++)
				{
					if(k == j)
						continue;

					if(Math::DotProduct(m_pPlayerState->velocity, planenormals[k]) < 0.0)
						break;
				}

				if(k == numplanes)
					break;
			}

			// Check if we went through all the planes
			if(j == numplanes)
			{
				if(numplanes != 2)
				{
					m_pPlayerState->velocity.Clear();
					break;
				}

				Vector dir;
				Math::CrossProduct(planenormals[0], planenormals[1], dir);
				Float dp = Math::DotProduct(dir, m_pPlayerState->velocity);
				Math::VectorScale(dir, dp, m_pPlayerState->velocity);
			}

			// If the velocity is against the primal velocity, remove velocity
			if(Math::DotProduct(m_pPlayerState->velocity, primal_velocity) <= 0)
			{
				m_pPlayerState->velocity.Clear();
				break;
			}
		}
	}

	// See if we didn't move at all
	if(allfraction == 0)
		m_pPlayerState->velocity.Clear();

	return blocked;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerMovement::ClipVelocity( const Vector& in, const Vector& normal, Vector& out, Float overbounce )
{
	Float angle = normal[2];

	Int32 blocked = BLOCKED_NOT;
	if(angle > 0)
		blocked |= BLOCKED_FLOOR;
	if(!angle)
		blocked |= BLOCKED_WALL;

	Float backoff = Math::DotProduct(in, normal)*overbounce;

	for(Int32 i = 0; i < 3; i++)
	{
		Float change = normal[i]*backoff;
		out[i] = in[i] - change;

		if(out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	return blocked;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::PreventMegaBunnyJumping( void )
{
	if(m_maxSpeed <= 0)
		return;

	Float speed = m_pPlayerState->velocity.Length();
	if(speed <= m_maxSpeed)
		return;

	Float frac = (m_maxSpeed/speed) * 0.65;
	Math::VectorScale(m_pPlayerState->velocity, frac, m_pPlayerState->velocity);
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Walk( void )
{
	Vector vforward = m_vForward;
	Vector vright = m_vRight;

	// Clear this
	vforward[2] = 0;
	vforward = vforward.Normalize();

	// Do not add in wishvel when on bike and not onground
	Vector wishvel;
	for(Int32 i = 0; i < 2; i++)
		wishvel[i] = vforward[i]*m_userCmd.forwardmove + vright[i]*m_userCmd.sidemove;

	wishvel[2] = 0;

	Vector wishdir = wishvel;
	Float wishspeed = Math::VectorNormalize(wishdir);

	// Speed-capping depends on direction
	Float otherDot;
	Float forwardMaxDot = Math::DotProduct(wishdir, vforward);

	if(forwardMaxDot <= 0)
	{
		// Do not consider backwards velocity as forward limit
		otherDot = 1.0;
		forwardMaxDot = 0;
	}
	else
	{
		// Comes from alignment to right side
		otherDot = SDL_fabs(Math::DotProduct(wishdir, vright));
	}

	// Normalize values at 1.0
	Float dotRatio = (forwardMaxDot + otherDot);
	if(dotRatio > 1.0)
	{
		dotRatio = 1.0f/dotRatio;
		forwardMaxDot *= dotRatio;
		otherDot *= dotRatio;
	}

	Float maxSpeedCheck = forwardMaxDot * m_maxForwardSpeed
		+ otherDot * m_maxSpeed;

	// Clamp to maxspeed
	if(wishspeed > maxSpeedCheck)
	{
		Math::VectorScale(wishvel, maxSpeedCheck/wishspeed, wishvel);
		wishspeed = maxSpeedCheck;
	}

	// Set pmove velocity Z component to zero
	m_pPlayerState->velocity[2] = 0;
	Accelerate(wishdir, wishspeed, m_pMovevars->accelerate);
	m_pPlayerState->velocity[2] = 0;

	// Add in basevelocity
	Math::VectorAdd(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);
	
	// If speed is too low, don't bother
	Float speed = m_pPlayerState->velocity.Length();
	if(speed < 1.0f)
	{
		m_pPlayerState->velocity.Clear();
		return;
	}

	// Remember groundent
	Int32 oldgroundent = m_pPlayerState->groundent;

	// Try moving to the destination
	Vector dest;
	Math::VectorMA(m_pPlayerState->origin, m_frameTime, m_pPlayerState->velocity, dest);
	dest[2] = m_pPlayerState->origin[2];

	// Try tracing
	trace_t trace;
	m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, dest, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);
	if(trace.fraction == 1.0)
	{
		// Copy endpos to player origin
		m_pPlayerState->origin = trace.endpos;

		// Try and move down stairs
		if(!m_pPlayerState->waterjumptime && oldgroundent != NO_ENTITY_INDEX)
		{
			dest = m_pPlayerState->origin;
			dest[2] -= m_pMovevars->stepsize;

			// Only care if there's actually a stair step below
			m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, dest, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);
			if(trace.fraction < 1.0 && trace.fraction > 0.0 && trace.plane.normal[2] >= m_planeZCap && !(trace.flags &(FL_TR_STARTSOLID|FL_TR_ALLSOLID)))
			{
				// We stepped down a ledge or stair step
				m_pPlayerState->origin = trace.endpos;
			}
		}

		// Nothing to do
		return;
	}

	if(oldgroundent == NO_ENTITY_INDEX && m_pPlayerState->waterlevel == WATERLEVEL_NONE)
		return;

	if(m_pPlayerState->waterjumptime > 0)
		return;

	// Remember original velocity and origin
	Vector orig_pos = m_pPlayerState->origin;
	Vector orig_vel = m_pPlayerState->velocity;

	// Try sliding
	Move_Fly();

	// Copy the results out
	Vector down = m_pPlayerState->origin;
	Vector downvel = m_pPlayerState->velocity;

	// Reset original values
	m_pPlayerState->origin = orig_pos;
	m_pPlayerState->velocity = orig_vel;

	// Try stepping up some stairs
	dest = m_pPlayerState->origin;
	dest[2] += m_pMovevars->stepsize;

	m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, dest, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);

	// If we made it, then copy origin
	if(!(trace.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID)))
	{
		orig_pos = m_pPlayerState->origin;
		m_pPlayerState->origin = trace.endpos;
	}

	// Slide move the rest of the way
	// (I really need to take the time to learn how this logic fucking works)
	Move_Fly();

	// I really have trouble following this logic...
	dest = m_pPlayerState->origin;
	dest[2] -= m_pMovevars->stepsize;

	m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, dest, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);
	if(trace.plane.normal[2] < m_planeZCap)
	{
		m_pPlayerState->origin = down;
		m_pPlayerState->velocity = downvel;
		return;
	}

	// if the trace succeeded, copy it
	if(!(trace.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID)))
		m_pPlayerState->origin = trace.endpos;

	// I have no idea what the fuck was this about in Valve's code
	// so I just did the same... need to figure out wtf this code does
	Vector up = m_pPlayerState->origin;

	// Ugly as sin even in this form
	Float dd0 = down[0]-orig_pos[0];
	Float dd1 = down[1]-orig_pos[1];
	Float downdist = dd0*dd0 + dd1*dd1;

	// Good lord
	Float ud0 = up[0]-orig_pos[0];
	Float ud1 = up[1]-orig_pos[1];
	Float updist = ud0*ud0 + ud1*ud1;

	if(downdist > updist)
	{
		m_pPlayerState->origin = down;
		m_pPlayerState->velocity = downvel;
	}
	else
	{
		// Only set Z velocity
		m_pPlayerState->velocity[2] = downvel[2];
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::PushEntity( const Vector& push, trace_t& trace )
{
	Vector end;
	Math::VectorAdd(m_pPlayerState->origin, push, end);

	m_traceInterface.pfnPlayerTrace(m_pPlayerState->origin, end, FL_TRACE_NORMAL, m_hullIndex, -1, trace );
	Math::VectorCopy(trace.endpos, m_pPlayerState->origin);

	if(trace.fraction < 1.0 && !(trace.flags & FL_TR_ALLSOLID))
		m_pmInterface.pfnAddToTouched(trace.hitentity, trace, m_pPlayerState->velocity);
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::AddGravity( Float multiplier )
{
	if(m_pPlayerState->flags & FL_FROZEN)
		return;

	Float entityGravity;
	if(m_pPlayerState->gravity)
		entityGravity = m_pPlayerState->gravity;
	else
		entityGravity = 1.0;

	m_pPlayerState->velocity[2] -= (entityGravity * m_pMovevars->gravity * multiplier * m_frameTime);
	m_pPlayerState->velocity[2] += m_pPlayerState->basevelocity[2] * m_frameTime;
	m_pPlayerState->basevelocity[2] = 0;

	CheckVelocity();
}

//=============================================
// @brief
//
//=============================================
bool CPlayerMovement::CheckWater( void )
{
	// Check for leg submersion
	Vector checkpos;
	checkpos[0] = m_pPlayerState->origin[0] + (m_pPMInfo->player_mins[m_hullIndex][0] + m_pPMInfo->player_maxs[m_hullIndex][0])*0.5;
	checkpos[1] = m_pPlayerState->origin[1] + (m_pPMInfo->player_mins[m_hullIndex][1] + m_pPMInfo->player_maxs[m_hullIndex][1])*0.5;
	checkpos[2] = m_pPlayerState->origin[2] + m_pPMInfo->player_mins[m_hullIndex][2] + 1.0f;

	m_pPlayerState->waterlevel = WATERLEVEL_NONE;
	m_pPlayerState->watertype = CONTENTS_EMPTY;

	Int32 contents = m_traceInterface.pfnPointContents(checkpos, nullptr);
	if(contents <= CONTENTS_WATER && contents  >= CONTENTS_LAVA)
	{
		m_pPlayerState->watertype = contents;
		m_pPlayerState->waterlevel = WATERLEVEL_LOW;

		// Check for half submersion
		Float height = (m_pPMInfo->player_mins[m_hullIndex][2] + m_pPMInfo->player_maxs[m_hullIndex][2]);
		Float heighthalf = height* 0.5f;

		checkpos[2] = m_pPlayerState->origin[2] + heighthalf;
		contents = m_traceInterface.pfnPointContents(checkpos, nullptr);
		if(contents <= CONTENTS_WATER && contents  >= CONTENTS_LAVA)
		{
			m_pPlayerState->waterlevel = WATERLEVEL_MID;

			// Check for head submersion
			checkpos[2] = m_pPlayerState->origin[2] + m_pPlayerState->view_offset[2];

			contents = m_traceInterface.pfnPointContents(checkpos, nullptr);
			if(contents <= CONTENTS_WATER && contents  >= CONTENTS_LAVA)
				m_pPlayerState->waterlevel = WATERLEVEL_FULL;
		}
	}

	return (m_pPlayerState->waterlevel > WATERLEVEL_LOW) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::CheckVelocity( void )
{
	for(Int32 i = 0; i < 3; i++)
	{
		if(m_pPlayerState->velocity.IsNAN(i))
		{
			m_pmInterface.pfnCon_Printf("%s - Got a NAN velocity on axis %d, player %d.\n", __FUNCTION__, i, (m_pPlayerState->entindex-1));
			m_pPlayerState->velocity[i] = 0;
		}
		if(m_pPlayerState->origin.IsNAN(i))
		{
			m_pmInterface.pfnCon_Printf("%s - Got a NAN origin on axis %d, player %d.\n", __FUNCTION__, i, (m_pPlayerState->entindex-1));
			m_pPlayerState->velocity[i] = 0;
		}

		if(m_pPlayerState->velocity[i] > m_pMovevars->maxvelocity)
		{
			m_pmInterface.pfnCon_Printf("%s - Got a velocity too high on axis %d, player %d.\n", __FUNCTION__, i, (m_pPlayerState->entindex-1));
			m_pPlayerState->velocity[i] = m_pMovevars->maxvelocity;
		}
		else if(m_pPlayerState->velocity[i] < -m_pMovevars->maxvelocity)
		{
			m_pmInterface.pfnCon_Printf("%s - Got a velocity too low on axis %d, player %d.\n", __FUNCTION__, i, (m_pPlayerState->entindex-1));
			m_pPlayerState->velocity[i] = -m_pMovevars->maxvelocity;
		}
	}
}

//=============================================
// @brief
//
//=============================================
const entity_state_t* CPlayerMovement::GetLadder( void )
{
	Int32 numentities = m_pmInterface.pfnGetNumEntities();
	for(Int32 i = 0; i < numentities; i++)
	{
		const entity_state_t* pentity = m_pmInterface.pfnGetEntityState(i);
		if(!pentity)
			continue;

		if(!pentity->modelindex)
			continue;

		const cache_model_t* pmodel = m_pmInterface.pfnGetModel(pentity->modelindex);
		if(pmodel->type != MOD_BRUSH)
			continue;

		if(pentity->skin == CONTENTS_LADDER)
		{
			Vector offset, test;
			const hull_t* phull = m_traceInterface.pfnHullForBSP(pentity->entindex, m_hullIndex, &offset);
			Int32 firstclipnode = phull->firstclipnode;

			Math::VectorSubtract( m_pPlayerState->origin, offset, test );
			if(m_traceInterface.pfnHullPointContents(phull, firstclipnode, test) == CONTENTS_EMPTY)
				continue;

			return pentity;
		}
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerMovement::CheckWaterJump( bool dojump )
{
	if(dojump)
	{
		// Already water jumping
		if(m_pPlayerState->waterjumptime)
			return false;

		// Don't hop out if we just jumped in
		if(m_pPlayerState->velocity[2] < -180)
			return false;
	}

	Vector flatvelocity;
	Math::VectorCopy(m_pPlayerState->velocity, flatvelocity);
	flatvelocity[2] = 0;

	// Get speed of flat movement
	Float speed = flatvelocity.Length();

	Vector flatforward;
	Math::VectorCopy(m_vForward, flatforward);
	flatforward[2] = 0;
	Math::VectorNormalize(flatforward);

	if(speed != 0 && Math::DotProduct(flatforward, flatvelocity) < 0)
		return false;

	Vector start, end;
	Math::VectorCopy(m_pPlayerState->origin, start);
	start[2] += WATERJUMP_HEIGHT;
	Math::VectorMA(start, 24, flatforward, end);

	trace_t tr;

	// Trace in front of us
	m_traceInterface.pfnPlayerTrace(start, end, FL_TRACE_NORMAL, HULL_SMALL, NO_ENTITY_INDEX, tr);
	Double firstTraceFraction = tr.fraction;
	if(tr.fraction < 1.0 && SDL_fabs(tr.plane.normal[2] < 0.5f))
	{
		start[2] += m_pPMInfo->player_maxs[m_hullIndex][2] + WATERJUMP_HEIGHT;

		Math::VectorMA(start, 24, flatforward, end);
		Math::VectorScale(tr.plane.normal, -50, m_pPlayerState->movedir);

		// Trace against jumped height
		m_traceInterface.pfnPlayerTrace(start, end, FL_TRACE_NORMAL, HULL_SMALL, NO_ENTITY_INDEX, tr);
		if(!tr.startSolid() && !tr.allSolid() && tr.fraction > firstTraceFraction)
		{
			if(dojump)
			{
				m_pPlayerState->waterjumptime = WATERJUMP_WAIT_TIME;
				m_pPlayerState->velocity[2] = 400;
				m_pPlayerState->oldbuttons |= IN_JUMP;
				m_pPlayerState->flags |= FL_WATERJUMP;
			}

			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::FixupGravityVelocity( void )
{
	if(m_pPlayerState->waterjumptime)
		return;

	m_pPlayerState->velocity[2] -= (m_pMovevars->gravity * m_frameTime * 0.5);
	CheckVelocity();
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Jump( void )
{
	// Don't jump if on bike, dead, or out of stamina
	if(m_pPlayerState->flags & (FL_DEAD|FL_ON_BIKE) || m_pPlayerState->stamina < PLAYER_MIN_STAMINA
		|| (m_pPlayerState->waterlevel == WATERLEVEL_NONE && m_pPlayerState->groundent == NO_ENTITY_INDEX))
	{
		m_pPlayerState->oldbuttons |= IN_JUMP;
		return;
	}

	// Handle water movement
	if(m_pPlayerState->waterlevel >= WATERLEVEL_MID)
	{
		if(m_pPlayerState->groundent == NO_ENTITY_INDEX && !CheckWaterJump(false))
		{
			Vector testpos = m_pPlayerState->origin;
			Math::VectorAdd(testpos, (m_pPlayerState->flags & FL_DUCKING) ? VEC_DUCK_VIEW : VEC_VIEW, testpos);

			// Remove waterdist
			testpos[2] -= m_pMovevars->waterdist;

			if(m_traceInterface.pfnPointContents(testpos, nullptr) != CONTENTS_WATER)
			{
				m_pPlayerState->velocity[2] = 0;
				return;
			}
		}

		// Remove groundent if we're in water
		m_pPlayerState->groundent = NO_ENTITY_INDEX;

		switch(m_pPlayerState->watertype)
		{
		case CONTENTS_WATER:
			m_pPlayerState->velocity[2] = 100;
			break;
		case CONTENTS_SLIME:
			m_pPlayerState->velocity[2] = 80;
			break;
		default:
			m_pPlayerState->velocity[2] = 50;
			break;
		}

		if(m_playSounds && m_pPlayerState->swimtime <= 0 && m_pPlayerState->waterlevel < 3)
		{
			// Wait one second before making new sounds
			m_pPlayerState->swimtime = SWIM_SOUND_DELAY;

			CString soundname;
			switch(Common::RandomLong(0, 3))
			{
			case 0: 
				soundname = "player/pl_wade1.wav";
				break;
			case 1: 
				soundname = "player/pl_wade2.wav";
				break;
			case 2: 
				soundname = "player/pl_wade3.wav";
				break;
			case 3: 
				soundname = "player/pl_wade4.wav";
				break;
			}

			m_pmInterface.pfnPlaySound(m_pPlayerState->entindex, SND_CHAN_BODY, soundname.c_str(), VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE);
		}
	}

	// Don't jump if we're not on ground
	if(m_pPlayerState->groundent == NO_ENTITY_INDEX)
	{
		m_pPlayerState->oldbuttons |= IN_JUMP;
		return;
	}

	if(m_pPlayerState->oldbuttons & IN_JUMP)
		return;

	// We're no longer on ground
	m_pPlayerState->groundent = NO_ENTITY_INDEX;

	PreventMegaBunnyJumping();

	// Play step sounds
	UpdateStepSound();

	if(m_playSounds && m_pTextureMaterial)
	{
		// only play the actual sound when it's the current cmd
		m_pmInterface.pfnPlayStepSound(m_pPlayerState->entindex, m_pTextureMaterial->materialname.c_str(), m_pPlayerState->stepleft, VOL_NORM, m_pPlayerState->origin);
	}

	// Add a punch to the player's view angles
	m_pPlayerState->punchamount[0] = Common::RandomFloat(25, 50);

	Vector direction = m_pPlayerState->velocity;
	Math::VectorNormalize(direction);

	// Add in jump velocity
	m_pPlayerState->velocity[2] += sqrt(2*800*55.0);

	// Add in forward velocity
	Vector vforward = m_vForward;
	Vector vright = m_vRight;

	// Clear this
	vforward[2] = 0;
	vforward = vforward.Normalize();

	// Do not add in wishvel when on bike and not onground
	Vector wishvel;
	for(Int32 i = 0; i < 2; i++)
		wishvel[i] = vforward[i]*m_userCmd.forwardmove + vright[i]*m_userCmd.sidemove;

	Vector wishdir = wishvel;
	Float wishspeed = Math::VectorNormalize(wishdir);

	// Clamp to maxspeed
	if(wishspeed > m_maxSpeed)
	{
		Math::VectorScale(wishvel, m_maxSpeed/wishspeed, wishvel);
		wishspeed = m_maxSpeed;
	}

	// Add in movement direction velocity
	for(Int32 i = 0; i < 2; i++)
		m_pPlayerState->velocity[i] += direction[i]*sqrt(2*wishspeed*55.0);

	// NOTES: Do we really need this?
	FixupGravityVelocity();

	m_pPlayerState->oldbuttons |= IN_JUMP;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::WaterJump( void )
{
	if(m_pPlayerState->waterjumptime > 10000)
		m_pPlayerState->waterjumptime = 10000;

	if(!m_pPlayerState->waterjumptime)
		return;

	m_pPlayerState->waterjumptime -= m_userCmd.msec;

	if(m_pPlayerState->waterjumptime < 0 || m_pPlayerState->waterlevel == WATERLEVEL_NONE)
	{
		m_pPlayerState->flags &= ~FL_WATERJUMP;
		m_pPlayerState->waterjumptime = 0;
	}

	m_pPlayerState->velocity[0] = m_pPlayerState->movedir[0];
	m_pPlayerState->velocity[1] = m_pPlayerState->movedir[1];
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Water( void )
{
	if(m_pPlayerState->waterlevel >= WATERLEVEL_LOW)
	{
		// Cap swim speed
		if(m_pPlayerState->waterlevel >= 3)
		{
			m_maxForwardSpeed = PLAYER_SWIM_SPEED;
			m_maxSpeed = PLAYER_SWIM_SPEED;
		}

		if((m_pPlayerState->waterlevel >= WATERLEVEL_FULL && m_pPlayerState->groundent != NO_ENTITY_INDEX
			|| m_pPlayerState->waterlevel >= WATERLEVEL_MID && m_pPlayerState->groundent == NO_ENTITY_INDEX)
			&& !(m_pPlayerState->flags & FL_DUCKING) && !m_pPlayerState->induck)
		{
			m_pPlayerState->ducktime = PLAYER_DUCK_WAIT_TIME;
			m_pPlayerState->induck = true;
		}
	}

	Vector wishvel;
	for(Int32 i = 0; i < 3; i++)
		wishvel[i] = m_vForward[i]*m_userCmd.forwardmove + m_vRight[i]*m_userCmd.sidemove;

	if(!m_userCmd.forwardmove && !m_userCmd.sidemove && !m_userCmd.upmove)
	{
		if(m_pPlayerState->waterlevel >= 3)
		{
			// Drift downwards slowly
			wishvel[2] -= PLAYER_DOWNWARD_DRIFT_SPEED;
		}
		else if(SDL_fabs(m_pPlayerState->velocity[2]) > 60)
		{
			wishvel[2] = m_pPlayerState->velocity[2];
		}
		else
		{
			Vector submergemin = m_pPlayerState->origin;
			submergemin += m_pPlayerState->view_offset;
			Vector submergemax = submergemin;
			submergemax[2] -= m_pMovevars->waterdist*3;

			Int32 contentsmin = m_traceInterface.pfnPointContents(submergemin, nullptr);
			Int32 contentsmax = m_traceInterface.pfnPointContents(submergemax, nullptr);

			// Wobble on the surface
			if(contentsmin == CONTENTS_WATER && contentsmax != CONTENTS_WATER)
				wishvel[2] += 8;
			else if(contentsmin == CONTENTS_WATER && contentsmax == CONTENTS_WATER)
				wishvel[2] += 12;
			else
				wishvel[2] += 2;
		}
	}
	else
	{
		// Add in upwards movement
		wishvel[2] += m_userCmd.upmove;
	}

	// Determine speed
	Vector wishdir = wishvel;
	Float wishspeed = Math::VectorNormalize(wishdir);
	if(wishspeed > m_maxSpeed)
	{
		Math::VectorScale(wishvel, m_maxSpeed/wishspeed, wishvel);
		wishspeed = m_maxSpeed;
	}

	// Slow it down a bit
	wishspeed *= 0.8;
	Math::VectorAdd(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);

	Vector temp = m_pPlayerState->velocity;
	Float speed = Math::VectorNormalize(temp);

	Float newspeed = 0;
	if(speed)
	{
		newspeed = speed - m_frameTime * speed * m_pMovevars->waterfriction * m_pPlayerState->waterlevel;
		if(newspeed < 0)
			newspeed = 0;

		Math::VectorScale(m_pPlayerState->velocity, newspeed/speed, m_pPlayerState->velocity);
	}

	if(wishspeed < 0.1f)
		return;

	Float addspeed = wishspeed - newspeed;
	if(addspeed > 0)
	{
		Math::VectorNormalize(wishvel);
		Float accelspeed = m_pMovevars->accelerate * wishspeed * m_frameTime * m_pPlayerState->friction;
		if(accelspeed > addspeed)
			accelspeed = addspeed;

		Math::VectorMA(m_pPlayerState->velocity, accelspeed, wishvel, m_pPlayerState->velocity);
	}
	
	trace_t tr;
	Vector dest;
	Math::VectorMA(m_pPlayerState->origin, m_frameTime, m_pPlayerState->velocity, dest);
	Vector start = dest;

	start[2] += m_pMovevars->stepsize + 1;
	m_traceInterface.pfnPlayerTrace(start, dest, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, tr);
	if(!(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID)))
		m_pPlayerState->origin = tr.endpos;
	else
		Move_Fly();
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::AirAccelerate( const Vector& wishdir, Float wishspeed, Float acceleration )
{
	if(m_pPlayerState->flags & FL_DEAD || m_pPlayerState->waterjumptime)
		return;

	if(wishspeed > AIR_ACCELERATE_MAX_SPEED)
		wishspeed = AIR_ACCELERATE_MAX_SPEED;

	Float currspeed = Math::DotProduct(m_pPlayerState->velocity, wishdir);
	Float addspeed = wishspeed - currspeed;
	if(addspeed <= 0)
		return;

	// Determine acceleration with veering
	Float accelspeed = acceleration * wishspeed * m_frameTime * m_pPlayerState->friction;
	if(accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust player velocity
	Math::VectorMA(m_pPlayerState->velocity, accelspeed, wishdir, m_pPlayerState->velocity);
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Accelerate( const Vector& wishdir, Float wishspeed, Float acceleration )
{
	if(m_pPlayerState->flags & FL_DEAD || m_pPlayerState->waterjumptime > 0)
		return;

	// See if we're changing direction
	Float curspeed = Math::DotProduct(m_pPlayerState->velocity, wishdir);
	Float addspeed = wishspeed - curspeed;
	
	// Only do anything if we're adding speed;
	if(addspeed <= 0)
		return;

	// Determine acceleration
	Float accelspeed = acceleration * m_frameTime * wishspeed * m_pPlayerState->friction;
	if(accelspeed > addspeed)
		accelspeed = addspeed;

	Math::VectorMA(m_pPlayerState->velocity, accelspeed, wishdir, m_pPlayerState->velocity);
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Air( void )
{
	Vector forward = m_vForward;
	Vector right = m_vRight;

	// Zero out z
	forward[2] = right[2] = 0;

	Vector wishvel;
	for(Int32 i = 0; i < 2; i++)
		wishvel[i] = forward[i]*m_userCmd.forwardmove + right[i]*m_userCmd.sidemove;

	// Zero out Z
	wishvel[2] = 0;

	Vector wishdir = wishvel;
	Float wishspeed = Math::VectorNormalize(wishdir);

	if(wishspeed > m_maxSpeed)
	{
		Math::VectorScale(wishvel, m_maxSpeed/wishspeed, wishvel);
		wishspeed = m_maxSpeed;
	}

	AirAccelerate(wishdir, wishspeed, m_pMovevars->airaccelerate);

	// Add in basevelocity
	Math::VectorAdd(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);

	// Run flymove
	Move_Fly();
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::DetermineTextureType( void )
{
	Vector sideOffset;
	sideOffset = m_vRight * 16;
	if(m_pPlayerState->stepleft)
		Math::VectorScale(sideOffset, -1, sideOffset);

	Vector start = m_pPlayerState->origin + sideOffset;
	Vector end = start - Vector(0, 0, 64);

	if(m_pPlayerState->movetype == MOVETYPE_FLY)
	{
		m_pTextureMaterial = &m_defaultMaterial;
		return;
	}

	// Clear this
	m_pTextureMaterial = nullptr;

	trace_t tr;
	m_traceInterface.pfnPlayerTrace(start, end, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.fraction == 1.0)
	{
		if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
			m_pTextureMaterial = &m_defaultMaterial;
		
		return;
	}

	const entity_state_t* pstate = m_pmInterface.pfnGetEntityState(tr.hitentity);
	if(!pstate->modelindex)
	{
		m_pTextureMaterial = &m_defaultMaterial;
		return;
	}

	const cache_model_t* pmodel = m_pmInterface.pfnGetModel(pstate->modelindex);
	if(pmodel->type == MOD_VBM)
	{
		m_pTextureMaterial = &m_defaultMaterial;
		return;
	}

	Math::VectorAdd(start, tr.plane.normal, start);
	Math::VectorSubtract(end, tr.plane.normal, end);

	const Char* pstrTextureName = m_traceInterface.pfnTraceTexture(tr.hitentity, start, end);
	if(!pstrTextureName)
		return;

	// Retreive material script
	m_pTextureMaterial = m_pmInterface.pfnGetMapTextureMaterialScript(pstrTextureName);
	if(!m_pTextureMaterial)
		m_pTextureMaterial = &m_defaultMaterial;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::UpdateStepSound( void )
{
	if(m_pPlayerState->timestepsound > 0 
		|| m_pPlayerState->movetype == MOVETYPE_NOCLIP
		|| m_pPlayerState->flags & (FL_FROZEN|FL_ON_BIKE))
		return;

	// Find out what texture we're on
	DetermineTextureType();

	// Failed to get material script
	if(!m_pTextureMaterial)
		return;

	// Don't play sounds on no-sound textures
	if(m_pTextureMaterial->flags & TX_FL_NO_STEPSOUND)
		return;

	Float speed = m_pPlayerState->velocity.Length() + 10;
	bool onladder = (m_pPlayerState->movetype == MOVETYPE_FLY);

	// Don't play step sounds when not onground
	if(m_pPlayerState->groundent == NO_ENTITY_INDEX && !onladder)
		return;

	// Don't play sounds if not moving
	if(m_pPlayerState->velocity.Length2D() <= 0)
		return;

	// Determine individual positions
	Float height = m_pPMInfo->player_maxs[m_hullIndex][2] - m_pPMInfo->player_mins[m_hullIndex][2];

	Vector center, knee, feet;
	Math::VectorCopy(m_pPlayerState->origin, center);
	Math::VectorSubtract(m_pPlayerState->origin, Vector(0, 0, 0.25 * height), knee);
	Math::VectorSubtract(m_pPlayerState->origin, Vector(0, 0, 0.5 * height), feet);

	CString material;
	Float volume = 0;
	Float nextStepTime = 0;

	if(onladder)
	{
		volume = 0.4;
		nextStepTime = STEPTIME_LADDER;
		material = "ladder";
	}
	else if(m_traceInterface.pfnPointContents(knee, nullptr) == CONTENTS_WATER)
	{
		volume = 0.7;
		nextStepTime = STEPTIME_WATER;
		material = "wade";
	}
	else if(m_traceInterface.pfnPointContents(feet, nullptr) == CONTENTS_WATER)
	{
		volume = (speed < PLAYER_NORMAL_SPEED) ? 0.25 : 0.5;
		nextStepTime = STEPTIME_WATER;
		material = "slosh";
	}
	else
	{
		// Name comes from material used by face under us
		material = m_pTextureMaterial->materialname;

		if(m_pPlayerState->stamina > PLAYER_MIN_STAMINA && (m_userCmd.buttons & IN_SPRINT) && !(m_pPlayerState->flags & FL_SLOWMOVE) && m_userCmd.forwardmove > 0)
		{
			nextStepTime = STEPTIME_SPRINT;
			volume = 1.0;
		}
		else if(speed < PLAYER_NORMAL_SPEED && m_pPlayerState->stamina > PLAYER_MIN_STAMINA && (m_pPlayerState->flags & FL_SLOWMOVE) && (!(m_userCmd.buttons & IN_SPRINT) || (m_pPlayerState->flags & FL_NO_SPRINT)))
		{
			nextStepTime = STEPTIME_SLOWMOVE;
			volume = 0.5;
		}
		else if(speed < PLAYER_NORMAL_SPEED)
		{
			nextStepTime = STEPTIME_SLOW;
			volume = 0.25;
		}
		else
		{
			nextStepTime = STEPTIME_NORMAL;
			volume = 0.75;
		}
	}

	// Next time we make a step sound
	m_pPlayerState->timestepsound = nextStepTime;

	// Don't play sounds or punch viewangles underwater
	if(m_pPlayerState->waterlevel >= WATERLEVEL_FULL)
		return;

	// Add in view punch
	if((m_pPlayerState->flags & FL_DUCKING || m_userCmd.forwardmove > 0 && m_userCmd.buttons & IN_SPRINT && !(m_pPlayerState->flags & FL_NO_SPRINT)
		&& m_pPlayerState->stamina > PLAYER_MIN_STAMINA) && m_pPlayerState->groundent != NO_ENTITY_INDEX && m_pPlayerState->waterlevel < WATERLEVEL_MID)
	{
		Int32 punchx = 0;
		Int32 punchz = 0;

		if(m_pPlayerState->flags & FL_DUCKING)
		{
			punchx = 7;
			punchz = 8;
		}
		else if(m_userCmd.buttons & IN_SPRINT && !(m_pPlayerState->flags & FL_NO_SPRINT) && m_pPlayerState->stamina > PLAYER_MIN_STAMINA)
		{
			punchx = 15;
			punchz = 7.5;
		}

		m_pPlayerState->punchamount.x += punchx;
		if(m_pPlayerState->stepleft)
			m_pPlayerState->punchamount.z += punchz;
		else
			m_pPlayerState->punchamount.z -= punchz;
	}

	if(m_playSounds)
	{
		// only play the actual sound when it's the current cmd
		m_pmInterface.pfnPlayStepSound(m_pPlayerState->entindex, material.c_str(), m_pPlayerState->stepleft, volume, m_pPlayerState->origin);
	}

	// Alternate step sounds
	m_pPlayerState->stepleft = !m_pPlayerState->stepleft;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerMovement::CanUnDuck( void )
{
	// Do not unduck if we're in water and not on the ground
	if(!(m_pPlayerState->flags & FL_ON_LADDER) && (m_pPlayerState->waterlevel >= WATERLEVEL_FULL 
		|| m_pPlayerState->waterlevel >= WATERLEVEL_LOW && m_pPlayerState->groundent == NO_ENTITY_INDEX))
		return false;

	// Test if we can unduck actually
	Vector neworigin = m_pPlayerState->origin;
	if(m_pPlayerState->groundent != NO_ENTITY_INDEX && m_pPlayerState->flags & FL_DUCKING)
	{
		for(Int32 i = 0; i < 3; i++)
			neworigin[i] += (m_pPMInfo->player_mins[HULL_SMALL][i] - m_pPMInfo->player_mins[HULL_HUMAN][i]);
	}

	Int32 result = m_traceInterface.pfnTestPlayerPosition(HULL_HUMAN, FL_TRACE_NORMAL, neworigin);
	return (result == NO_ENTITY_INDEX) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::UnDuck( void )
{
	if(!CanUnDuck())
		return;

	// Test if we can unduck actually
	Vector neworigin = m_pPlayerState->origin;
	if(m_pPlayerState->groundent != NO_ENTITY_INDEX && m_pPlayerState->flags & FL_DUCKING)
	{
		for(Int32 i = 0; i < 3; i++)
			neworigin[i] += (m_pPMInfo->player_mins[HULL_SMALL][i] - m_pPMInfo->player_mins[HULL_HUMAN][i]);
	}

	m_pPlayerState->flags &= ~FL_DUCKING;
	m_pPlayerState->induck = false;
	m_pPlayerState->view_offset = VEC_VIEW;
	m_pPlayerState->ducktime = 0;
	m_pPlayerState->origin = neworigin;
	m_hullIndex = HULL_HUMAN;

	// Re-check position
	CategorizePosition();
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Duck( void )
{
	if(m_pPlayerState->flags & FL_ON_BIKE)
		return;

	Int32 buttonschanged = m_pPlayerState->oldbuttons ^ m_userCmd.buttons;
	Int32 buttonspressed = buttonschanged & m_userCmd.buttons;

	// No ducking while dead or on ladder
	if(m_pPlayerState->flags & (FL_DEAD|FL_ON_LADDER))
	{
		// Unduck
		if(m_pPlayerState->flags & FL_DUCKING)
			UnDuck();

		// Do not allow ducking in these states
		return;
	}

	// Lower velocity if we're ducking
	if(m_pPlayerState->flags & FL_DUCKING && m_pPlayerState->waterlevel < WATERLEVEL_MID)
	{
		m_userCmd.forwardmove *= DUCKING_SPEED_MULTIPLIER;
		m_userCmd.sidemove *= DUCKING_SPEED_MULTIPLIER;
		m_userCmd.upmove *= DUCKING_SPEED_MULTIPLIER;
	}

	if((!m_pMovevars->holdtoduck && (m_userCmd.buttons & IN_DUCK) || m_pMovevars->holdtoduck && ((m_pPlayerState->flags & FL_DUCKING) || (m_userCmd.buttons & IN_DUCK))) 
		|| m_pPlayerState->induck || (m_pPlayerState->flags & FL_DUCKING))
	{
		// Try ducking if we can
		if(!m_pPlayerState->induck)
		{
			if(!m_pMovevars->holdtoduck)
			{
				if((buttonspressed & IN_DUCK && !(m_pPlayerState->flags & FL_DUCKING) && !m_pPlayerState->induck)
					|| (m_userCmd.buttons & IN_DUCK && !(m_pPlayerState->oldbuttons & IN_DUCK) && m_pPlayerState->flags & FL_DUCKING))
				{
					m_pPlayerState->ducktime = PLAYER_DUCK_WAIT_TIME;
					m_pPlayerState->induck = true;
				}
			}
			else
			{
				if(!(m_userCmd.buttons & IN_DUCK) && (m_pPlayerState->flags & FL_DUCKING)
					|| (buttonspressed & IN_DUCK) && !(m_pPlayerState->flags & FL_DUCKING))
				{
					m_pPlayerState->ducktime = PLAYER_DUCK_WAIT_TIME;
					m_pPlayerState->induck = true;
				}
			}
		}

		// Unduck if jumping
		if(m_pPlayerState->waterlevel < WATERLEVEL_MID && m_pPlayerState->groundent == NO_ENTITY_INDEX
			&& (m_pPlayerState->flags & FL_DUCKING) && !(m_userCmd.buttons & IN_DUCK) && CanUnDuck())
			UnDuck();
	}

	// Manage duck finish/start
	if(m_pPlayerState->induck)
	{
		Float time = (float)m_pPlayerState->ducktime / PLAYER_DUCK_WAIT_TIME;
		Float timeto = clamp((1.0 - (float)m_pPlayerState->ducktime / PLAYER_DUCK_WAIT_TIME), 0.0, 1.0);
		Float ducktime = PLAYER_DUCK_TIME*MILLISECONDS_TO_SECONDS;

		if(!(m_pPlayerState->flags & FL_DUCKING))
		{
			// Finish duck if time is up, or if we're not onground or in water
			if(time <= (1.0 - ducktime) 
				|| (m_pPlayerState->groundent == NO_ENTITY_INDEX
				&& m_pPlayerState->waterlevel <= WATERLEVEL_LOW))
			{
				m_pPlayerState->view_offset = VEC_DUCK_VIEW;
				m_pPlayerState->flags |= FL_DUCKING;
				m_pPlayerState->induck = false;
				m_hullIndex = HULL_SMALL;

				if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
				{
					Vector offset;
					Math::VectorSubtract(m_pPMInfo->player_mins[HULL_SMALL], m_pPMInfo->player_mins[HULL_HUMAN], offset);
					Math::VectorSubtract(m_pPlayerState->origin, offset, m_pPlayerState->origin);

					// Try to unfudge the player if he's stuck
					FixCrouchStuck(Vector(0, 0, 1));

					// Set this stuff
					CategorizePosition();
				}
			}
			else
			{
				// Calculate offset
				Float add = (VEC_DUCK_HULL_MIN[2] - VEC_HULL_MIN[2]);

				// Calculate fraction
				Float duckfrac = Common::SplineFraction(timeto, (1.0/ducktime));
				m_pPlayerState->view_offset[2] = ((VEC_DUCK_VIEW[2] - add)*duckfrac) + (VEC_VIEW[2]*(1.0 - duckfrac));
			}
		}
		else
		{
			if(time <= (1.0 - ducktime) || m_pPlayerState->groundent == NO_ENTITY_INDEX)
			{
				// Unduck if not onground or time's up
				UnDuck();
			}
			else
			{
				// Calculate difference
				Float add = (VEC_HULL_MAX[2] - VEC_DUCK_HULL_MAX[2]);

				// Calculate fraction
				Float duckfrac = Common::SplineFraction(timeto, (1.0/ducktime));
				m_pPlayerState->view_offset[2] = (VEC_DUCK_VIEW[2] * (1.0 - duckfrac)) + ((VEC_VIEW[2]+add) * duckfrac);
			}

			if(!CanUnDuck())
			{
				// Can't unduck, go back to ducking
				m_pPlayerState->view_offset = VEC_DUCK_VIEW;
				m_pPlayerState->flags |= FL_DUCKING;
				m_pPlayerState->induck = false;
				m_hullIndex = HULL_SMALL;

				if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
				{
					Vector offset;
					Math::VectorSubtract(m_pPMInfo->player_mins[HULL_SMALL], m_pPMInfo->player_mins[HULL_HUMAN], offset);
					Math::VectorSubtract(m_pPlayerState->origin, offset, m_pPlayerState->origin);			
				
					// Try to unfudge the player if he's stuck
					FixCrouchStuck(Vector(0, 0, 1));

					// Set this stuff
					CategorizePosition();
				}
			}
		}
	}

	if(m_userCmd.buttons & IN_DUCK)
		m_pPlayerState->oldbuttons |= IN_DUCK;
	else
		m_pPlayerState->oldbuttons &= ~IN_DUCK;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Friction( void )
{
	if(m_pPlayerState->waterjumptime)
		return;

	Vector velocity = m_pPlayerState->velocity;
	Float speed = velocity.Length();

	if(speed < 0.1)
		return;

	Float drop = 0;
	if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
	{
		Vector start, stop;
		for(Int32 i = 0; i < 2; i++)
			start[i] = stop[i] = m_pPlayerState->origin[i] + velocity[i]/speed*16;

		start[2] = m_pPlayerState->origin[2] + m_pPMInfo->player_mins[m_hullIndex][2];
		stop[2] = start[2] - 34;

		trace_t trace;
		m_traceInterface.pfnPlayerTrace(start, stop, FL_TRACE_NORMAL, m_hullIndex, NO_ENTITY_INDEX, trace);

		Float friction = 0;
		if(trace.fraction == 1.0)
			friction = m_pMovevars->friction*m_pMovevars->edgefriction;
		else
			friction = m_pMovevars->friction;

		// Add player friction
		friction *= m_pPlayerState->friction;
		Float control = (speed < m_pMovevars->stopspeed) ? m_pMovevars->stopspeed : speed;
		drop += control*friction*m_frameTime;
	}

	Float newspeed = speed - drop;
	if(newspeed < 0)
		newspeed = 0;

	newspeed /= speed;

	Math::VectorScale(velocity, newspeed, m_pPlayerState->velocity);

	CheckVelocity();
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::CheckFalling( void )
{
	if(m_pPlayerState->groundent != NO_ENTITY_INDEX 
		&& !(m_pPlayerState->flags & FL_DEAD) 
		&& m_pPlayerState->fallvelocity >= PLAYER_FALL_TRESHOLD)
	{
		m_pPlayerState->timestepsound = 0;

		// Update now
		UpdateStepSound();

		// Play the sound always
		if(m_playSounds && m_pTextureMaterial)
		{
			Float volume = 0.5;
			if(m_pPlayerState->waterlevel == WATERLEVEL_NONE)
			{
				if(m_pPlayerState->fallvelocity > PLAYER_SAFE_FALL_SPEED)
					volume = 1.0;
				else if(m_pPlayerState->fallvelocity > PLAYER_SAFE_FALL_SPEED/2.0f)
					volume = 0.75;
			}

			// only play the actual sound when it's the current cmd
			m_pmInterface.pfnPlayStepSound(m_pPlayerState->entindex, m_pTextureMaterial->materialname.c_str(), m_pPlayerState->stepleft, volume, m_pPlayerState->origin);
		}

		// Punch the view
		Float punchforce = ((m_pPlayerState->fallvelocity > PLAYER_SAFE_FALL_SPEED) ? PLAYER_SAFE_FALL_SPEED : m_pPlayerState->fallvelocity) * 0.25;

		m_pPlayerState->punchamount[0] += punchforce;
		m_pPlayerState->punchamount[1] += Common::RandomFloat(punchforce/-10, punchforce/10);
		m_pPlayerState->punchamount[2] += Common::RandomFloat(punchforce/-10, punchforce/10);
	}

	if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
		m_pPlayerState->fallvelocity = 0;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Frozen( void )
{
	if(m_pPlayerState->flags & FL_DUCKING)
		UnDuck();

	m_pPlayerState->movetype = MOVETYPE_FLY;
	m_pPlayerState->velocity.Clear();
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::RunPlayerMove( void )
{
	CheckParameters();
	ReduceTimers();

	Math::AngleVectors(m_pPlayerState->viewangles, &m_vForward, &m_vRight, &m_vUp);

	// Try to unstick us if we're stuck
	if(m_pPlayerState->movetype != MOVETYPE_NOCLIP 
		&& m_pPlayerState->movetype != MOVETYPE_NONE
		&& !(m_pPlayerState->flags & FL_FROZEN))
	{
		if(!CheckStuck())
			return;
	}

	// Determine position related info
	CategorizePosition();

	// Remember previous water level
	m_oldWaterLevel = m_pPlayerState->waterlevel;

	// Store fall velocity if not onground
	if(m_pPlayerState->groundent == NO_ENTITY_INDEX)
		m_pPlayerState->fallvelocity = -m_pPlayerState->velocity[2];

	// Set to default
	m_isOnLadder = false;
	if(!(m_pPlayerState->flags & (FL_DEAD|FL_ON_BIKE)))
	{
		m_pLadder = GetLadder();
		if(m_pLadder)
			m_isOnLadder = true;
	}

	// Try to play step sounds
	UpdateStepSound();

	// Crouch if needed
	Duck();

	if(!(m_pPlayerState->flags & (FL_DEAD|FL_ON_BIKE)))
	{
		if(m_pPlayerState->flags & FL_ON_LADDER)
		{
			Move_Frozen();
		}
		else if(m_pLadder)
		{
			Move_Ladder();
		}
		else if( m_pPlayerState->movetype != MOVETYPE_WALK 
			&& m_pPlayerState->movetype != MOVETYPE_NOCLIP )
		{
			// Reset movetype after ladder use
			m_pPlayerState->movetype = MOVETYPE_WALK;
		}
	}

	switch(m_pPlayerState->movetype)
	{
	case MOVETYPE_NONE:
		break;

	case MOVETYPE_NOCLIP:
		Move_Noclip();
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		Move_Toss_Bounce();
		break;

	case MOVETYPE_WALK:
		{
			// Add in gravity (Why is this 0.5?)
			if(m_pPlayerState->waterlevel <= WATERLEVEL_LOW)
				AddGravity();

			// If we're leaping out of water, perform flymove
			if(m_pPlayerState->waterjumptime > 0)
			{
				WaterJump();
				Move_Fly();

				// Set waterlevel after movement
				CheckWater();
				return;
			}
			else if(m_pPlayerState->flags & FL_WATERJUMP)
			{
				// Make sure this is cleared
				m_pPlayerState->flags &= ~FL_WATERJUMP;
			}

			if(m_pPlayerState->waterlevel >= WATERLEVEL_MID)
			{
				// Jump out of the water if we can
				if(m_pPlayerState->waterlevel == WATERLEVEL_MID)
					CheckWaterJump(true);

				if(m_pPlayerState->velocity[2] < 0 && m_pPlayerState->waterjumptime > 0)
					m_pPlayerState->waterjumptime = 0;

				// Jump if possible
				if(m_userCmd.buttons & IN_JUMP)
					Jump();
				else
					m_pPlayerState->oldbuttons &= ~IN_JUMP;

				// Perform water movement
				Move_Water();

				Math::VectorSubtract(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);
				
				// Get a final position
				CategorizePosition();
			}
			else
			{
				// Jump if possible
				if(m_userCmd.buttons & IN_JUMP)
					Jump();
				else
					m_pPlayerState->oldbuttons &= ~IN_JUMP;

				// Add friction
				if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
				{
					m_pPlayerState->velocity[2] = 0;
					Friction();
				}

				// Make sure velocity is valid
				CheckVelocity();

				if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
					Move_Walk();
				else
					Move_Air();

				// Set position related stuff
				CategorizePosition();

				// Remove base velocity
				Math::VectorSubtract(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);

				// Check velocity again
				CheckVelocity();

				// Correct gravity
				if(m_pPlayerState->waterlevel > WATERLEVEL_LOW)
					FixupGravityVelocity();

				// Clear z velocity again if onground
				if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
					m_pPlayerState->velocity[2] = 0;

				// Play landing sounds if possible
				CheckFalling();
			}
		}
		break;
	case MOVETYPE_FLY:
		{
			// Set water level
			CheckWater();

			if( m_userCmd.buttons & IN_JUMP )
			{
				if(!m_pLadder)
					Jump();
			}
			else
			{
				m_pPlayerState->oldbuttons &= ~IN_JUMP;
			}

			// Account for basevelocity in flymove
			Math::VectorAdd(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);

			// Perform flying movement
			Move_Fly();

			// Subtract basevelocity after flymove
			Math::VectorSubtract(m_pPlayerState->velocity, m_pPlayerState->basevelocity, m_pPlayerState->velocity);
		}
		break;

	case MOVETYPE_STEP:
	default:
		m_pmInterface.pfnCon_EPrintf("Invalid movetype %d for client %d.\n", (Int32)m_pPlayerState->movetype);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::RunMovement( const usercmd_t& cmd, pm_info_t* pminfo, bool playSounds, bool isMultiplayer )
{
	// set basics
	m_pPMInfo = pminfo;
	m_userCmd = cmd;
	m_playSounds = playSounds;
	m_isMultiplayer = isMultiplayer;

	m_frameTime = cmd.lerp_msec*MILLISECONDS_TO_SECONDS;
	m_pPlayerState = &pminfo->playerstate;
	m_pMovevars = &pminfo->movevars;

	// Set the Z plane cap
	if(m_pPlayerState->planezcap != 0)
		m_planeZCap = m_pPlayerState->planezcap;
	else
		m_planeZCap = DEFAULT_ONGROUND_LOWER_CAP;

	// Determine hull used
	if(m_pPlayerState->flags & FL_DUCKING)
		m_hullIndex = HULL_SMALL;
	else
		m_hullIndex = HULL_HUMAN;

	if(m_pPlayerState->flags & FL_ON_BIKE)
	{
		m_maxSpeed = MOTORBIKE_MAX_SPEED;
		m_maxForwardSpeed = MOTORBIKE_MAX_SPEED;

		// Save client acceleration
		m_pPlayerState->fuser2 = cmd.forwardmove;
	}
	else
	{
		if(m_pPlayerState->movetype == MOVETYPE_NOCLIP)
		{
			m_maxSpeed = PLAYER_NOCLIP_SPEED;
			m_maxForwardSpeed = PLAYER_NOCLIP_SPEED;
		}
		else
		{
			if(m_pPlayerState->stamina <= PLAYER_MIN_STAMINA)
			{
				m_maxSpeed = PLAYER_EXHAUST_SPEED;
				m_maxForwardSpeed = PLAYER_EXHAUST_SPEED;
			}
			else if(!(m_pPlayerState->flags & FL_NO_SPRINT) && m_userCmd.buttons & IN_SPRINT)
			{
				m_maxSpeed = PLAYER_NORMAL_SPEED;
				m_maxForwardSpeed = PLAYER_SPRINT_SPEED;
			}
			else if(!(m_pPlayerState->flags & FL_SLOWMOVE) || (m_pPlayerState->flags & FL_DUCKING))
			{
				m_maxSpeed = PLAYER_NORMAL_SPEED;
				m_maxForwardSpeed = PLAYER_NORMAL_SPEED;
			}
			else
			{
				m_maxSpeed = PLAYER_SNEAK_SPEED;
				m_maxForwardSpeed = PLAYER_SNEAK_SPEED;
			}
		}
	}

	// Perform movement
	RunPlayerMove();

	if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
		m_pPlayerState->flags |= FL_ONGROUND;
	else
		m_pPlayerState->flags &= ~FL_ONGROUND;
	
	if(m_pPlayerState->movetype == MOVETYPE_WALK)
		m_pPlayerState->friction = 1.0f;

	// Always reset this
	if(m_pPlayerState->groundent != NO_ENTITY_INDEX)
		m_pPlayerState->planezcap = 0;
}

//=============================================
// @brief
//
//=============================================
void CPlayerMovement::Move_Noclip( void )
{
	// Calculate velocity
	Vector velocity;
	Math::VectorScale(m_vForward, m_userCmd.forwardmove, velocity);
	Math::VectorMA(velocity, m_userCmd.sidemove, m_vRight, velocity);
	Math::VectorMA(velocity, m_userCmd.upmove, m_vUp, velocity);

	// Add velocity to origin
	Vector origin;
	Math::VectorMA(m_pPlayerState->origin, m_frameTime, velocity, origin);
	Math::VectorCopy(origin, m_pPlayerState->origin);
	Math::VectorCopy(velocity, m_pPlayerState->velocity);
}

//=============================================
// @brief
//
//=============================================
bool CPlayerMovement::CheckStuck( void )
{
	// See if we're inside anything
	entindex_t hitent = m_traceInterface.pfnTestPlayerPosition(m_hullIndex, FL_TRACE_NORMAL, m_pPMInfo->playerstate.origin);
	if(hitent == NO_ENTITY_INDEX)
		return true;

	if(m_pPMInfo->playerstate.flags & (FL_FROZEN|FL_ON_LADDER)
		|| (m_pPMInfo->playerstate.movetype == MOVETYPE_NOCLIP
		|| m_pPMInfo->playerstate.movetype == MOVETYPE_FLY))
		return false;

	Vector origin, testorigin;
	Math::VectorCopy(m_pPMInfo->playerstate.origin, origin);

	// Do a round trace for possible unsticking
	for(Uint32 i = 0; i < MAX_STUCK_CHECKDISTANCE; i++)
	{
		for(Uint32 j = 0; j < MAX_STUCK_CHECKDISTANCE; j++)
		{
			for(Uint32 k = 0; k < MAX_STUCK_CHECKDISTANCE; k++)
			{
				testorigin[0] = origin[0]+i;
				testorigin[1] = origin[1]+j;
				testorigin[2] = origin[2]+k;
				if(m_traceInterface.pfnTestPlayerPosition(m_hullIndex, FL_TRACE_NORMAL, testorigin) == NO_ENTITY_INDEX)
				{
					Math::VectorCopy(testorigin, m_pPMInfo->playerstate.origin);
					return true;
				}

				testorigin[0] = origin[0]-i;
				testorigin[1] = origin[1]-j;
				testorigin[2] = origin[2]-k;
				if(m_traceInterface.pfnTestPlayerPosition(m_hullIndex, FL_TRACE_NORMAL, testorigin) == NO_ENTITY_INDEX)
				{
					Math::VectorCopy(testorigin, m_pPMInfo->playerstate.origin);
					return true;
				}
			}
		}
	}

	return false;
}