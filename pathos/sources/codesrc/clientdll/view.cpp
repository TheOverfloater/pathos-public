/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "vector.h"
#include "ref_params.h"
#include "com_math.h"
#include "cl_entity.h"
#include "view.h"
#include "clientdll.h"
#include "cvar.h"
#include "usercmd.h"
#include "constants.h"
#include "contents.h"
#include "buttonbits.h"
#include "movevars.h"
#include "viewmodel.h"
#include "shake.h"
#include "ladder.h"
#include "motorbike.h"
#include "viewcontroller.h"
#include "cache_model.h"
#include "dlight.h"
#include "hud.h"
#include "mlight.h"
#include "clshared.h"

// Flashlight fade speed
const Float CDefaultView::FLASHLIGHT_FADE_SPEED = 3.5;
// Flashlight sprite file
const Char CDefaultView::FLASHLIGHT_SPRITE_FILE[] = "sprites/flare1.spr";

// Values view leaning calculations
const Float CDefaultView::VIEWMODEL_LAG_MULT = 0.4f;
const Float CDefaultView::VIEWMODEL_LAG_SPEED = 5;

// Values for view bob calculations
const Float CDefaultView::BOB_CYCLE_MIN = 1.0f;
const Float CDefaultView::BOB_CYCLE_MAX = 0.45f;
const Float CDefaultView::BOB = 0.002f;
const Float CDefaultView::BOB_UP = 0.5f;

// Values for stair step smoothing
const Float CDefaultView::V_SM_REF_VEL = 210;// DO NOT set this lower
const Float CDefaultView::V_SM_BLEND_VEL = 100;

// View origin
Vector g_viewOrigin;

// Class object definition
CDefaultView gDefaultView;

//=============================================
//
//=============================================
cl_entity_t* V_GetViewModel( void )
{
	return gViewModel.GetViewModel();
}

//====================================
//
//====================================
CDefaultView::CDefaultView( void ):
	m_bobTime(0),
	m_lastBobTime(0),
	m_verticalBob(0),
	m_lateralBob(0),
	m_flLastViewModelLagTime(0),
	m_breathingTime(0),
	m_currentViewRoll(0),
	m_leanTime(0),
	m_prevLeanButtons(0),
	m_prevPlayerFlags(0),
	m_leaningState(false),
	m_lastStepSmoothTime(0),
	m_prevStepSmoothZ(0),
	m_stepSmoothSpeed(0),
	m_lastSwimFloatTime(0),
	m_fovValue(0),
	m_fovOverrideValue(0),
	m_prevFOVValue(0),
	m_desiredFOVValue(0),
	m_fovBlendDelta(0),
	m_fovBlendTime(0),
	m_localPlayerDimLightStrength(0),
	m_pFlashlightSprite(nullptr),
	m_viewEntity(NO_ENTITY_INDEX),
	m_pCvarRollAngle(nullptr),
	m_pCvarRollSpeed(nullptr),
	m_pCvarReferenceFOV(nullptr),
	m_pCvarViewBob(nullptr)
{
	for(Uint32 i = 0; i < MAX_PLAYERS; i++)
	{
		m_tacticalLightStrengths[i] = 0;
		m_shoulderLightStrengths[i] = 0;
	}
}

//====================================
//
//====================================
CDefaultView::~CDefaultView( void )
{
}

//====================================
//
//====================================
bool CDefaultView::Init( void )
{
	m_pCvarRollAngle = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), VIEW_ROLL_CVAR_NAME, "0.5", "Controls view roll angle.");
	m_pCvarRollSpeed = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "v_rollspeed", "150", "Controls view roll speed.");
	m_pCvarViewBob = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), VIEW_BOB_CVAR_NAME, "1", "Controls view roll speed.");

	m_pCvarReferenceFOV = cl_engfuncs.pfnGetCVarPointer(REFERENCE_FOV_CVAR_NAME);
	if(!m_pCvarReferenceFOV)
	{
		cl_engfuncs.pfnErrorPopup("Failed to get cvar '%s'", REFERENCE_FOV_CVAR_NAME);
		return false;
	}

	return true;
}

//====================================
//
//====================================
bool CDefaultView::InitGame( void )
{
	// Set default FOV
	SetFOV(0);

	// Load flashlight sprite
	m_pFlashlightSprite = cl_engfuncs.pfnLoadModel(FLASHLIGHT_SPRITE_FILE);
	if(!m_pFlashlightSprite)
		m_pFlashlightSprite = cl_engfuncs.pfnLoadModel(ERROR_SPRITE_FILENAME);

	// Force reset
	m_flLastViewModelLagTime = -1;

	return true;
}

//====================================
//
//====================================
void CDefaultView::ClearGame( void )
{
	// Reset breathing
	m_breathingTime = -1;

	// Reset view roll
	m_currentViewRoll = 0;

	// Reset leaning
	m_prevLeanButtons = 0;
	m_leanTime = 0;
	m_leaningState = false;

	// Reset stair smoothing
	m_lastStepSmoothTime = 0;

	// Reset FOV related stuff
	m_fovOverrideValue = 0;
	m_prevFOVValue = 0;
	m_desiredFOVValue = 0;
	m_fovBlendDelta = 0;
	m_fovBlendTime = 0;
	m_viewModelLastFacing.Clear();

	// Reset flashlights
	for(Uint32 i = 0; i < MAX_PLAYERS; i++)
	{
		m_tacticalLightStrengths[i] = 0;
		m_shoulderLightStrengths[i] = 0;
	}

	m_localPlayerDimLightStrength = 0;

	// Reset view entity
	m_viewEntity = NO_ENTITY_INDEX;

	m_idealAutoAimVector.Clear();
	m_currentAutoAimVector.Clear();
}

//====================================
//
//====================================
void CDefaultView::ResetViewRoll( void )
{
	m_currentViewRoll = 0;
}

//====================================
//
//====================================
void CDefaultView::ResetViewIdle( void )
{
	m_breathingTime = -1;
}

//====================================
//
//====================================
Float CDefaultView::EstimateStepTime( cl_entity_t* pplayer, ref_params_t& params )
{
	Float speed = params.pl_velocity.Length2D();
	Float height = pplayer->curstate.maxs[2] - pplayer->curstate.mins[2];

	Vector center, knee, feet;
	Math::VectorCopy(params.pl_origin, center);
	Math::VectorSubtract(params.pl_origin, Vector(0, 0, 0.3 * height), knee);
	Math::VectorSubtract(params.pl_origin, Vector(0, 0, 0.5 * height), feet);

	Float steptime = 0;
	if(cl_tracefuncs.pfnPointContents(knee, nullptr) == CONTENTS_WATER)
	{
		steptime = STEPTIME_WATER;
	}
	else if(cl_tracefuncs.pfnPointContents(feet, nullptr) == CONTENTS_WATER)
	{
		steptime = STEPTIME_WATER;
	}
	else
	{
		if(pplayer->curstate.stamina > PLAYER_MIN_STAMINA && (pplayer->curstate.buttons & IN_SPRINT) && !(pplayer->curstate.flags & FL_SLOWMOVE))
			steptime = STEPTIME_SPRINT;
		else if(pplayer->curstate.stamina > PLAYER_MIN_STAMINA && (pplayer->curstate.flags & FL_SLOWMOVE) && (!(pplayer->curstate.buttons & IN_SPRINT) || pplayer->curstate.flags & FL_NO_SPRINT))
			steptime = STEPTIME_SLOWMOVE;
		else if(speed < PLAYER_NORMAL_SPEED-10)
			steptime = STEPTIME_SLOW;
		else
			steptime = STEPTIME_NORMAL;
	}

	return steptime;
}

//====================================
//
//====================================
void CDefaultView::CalcBob( cl_entity_t* pplayer, ref_params_t& params )
{
	Vector velocity = params.pl_velocity;
	Float speed = velocity.Length2D();

	// Estimate step time, we need half
	Float steptime = EstimateStepTime(pplayer, params) * 0.5;
	speed = clamp(speed, -steptime, steptime);

	Float boboffset = Common::RemapValue(speed, 0, steptime, 0.0f, 1.0f);
	m_bobTime += (params.time - m_lastBobTime) * boboffset;
	m_lastBobTime = params.time;

	// Calculate vertical bob
	Float cycle = m_bobTime - SDL_floor(m_bobTime/BOB_CYCLE_MAX)*BOB_CYCLE_MAX;
	cycle /= BOB_CYCLE_MAX;

	if(cycle < BOB_UP)
		cycle = M_PI*cycle/BOB_UP;
	else
		cycle = M_PI + M_PI*(cycle-BOB_UP)/(1.0-BOB_UP);

	Float viewBobScale = m_pCvarViewBob->GetValue();

	m_verticalBob = speed*0.005f;
	m_verticalBob = m_verticalBob*0.3 + m_verticalBob*0.7*SDL_sin(cycle);
	m_verticalBob = clamp(m_verticalBob, -7.0f, 4.0f);
	m_verticalBob *= viewBobScale;

	// Calculate lateral bob
	cycle = m_bobTime - SDL_floor(m_bobTime/BOB_CYCLE_MAX*2)*BOB_CYCLE_MAX*2;
	cycle /= BOB_CYCLE_MAX*2;

	if(cycle < BOB_UP)
		cycle = M_PI*cycle/BOB_UP;
	else
		cycle = M_PI + M_PI*(cycle-BOB_UP)/(1.0-BOB_UP);

	m_lateralBob = speed*0.005f;
	m_lateralBob = m_lateralBob*0.3f + m_lateralBob*0.7*SDL_sin(cycle);
	m_lateralBob = clamp(m_lateralBob, -7.0f, 4.0f);
	m_lateralBob *= viewBobScale;
}

//=============================================
//
//=============================================
void CDefaultView::AddViewModelBob( cl_entity_t* pviewmodel, ref_params_t& params )
{
	Vector forward, right;
	Math::AngleVectors(pviewmodel->curstate.angles, &forward, &right, nullptr);

	// Add in bob, but scaled down
	Math::VectorMA(pviewmodel->curstate.origin, m_verticalBob*0.1f, forward, pviewmodel->curstate.origin);

	// Bob z out a bit more
	pviewmodel->curstate.origin[2] += m_verticalBob*0.2f;

	// bob the angles
	pviewmodel->curstate.angles[ROLL]	+= m_verticalBob * 0.5f;
	pviewmodel->curstate.angles[PITCH]	-= m_verticalBob * 0.4f;
	pviewmodel->curstate.angles[YAW]	-= m_lateralBob  * 0.3f;

	// throw in a little tilt.
	pviewmodel->curstate.angles[YAW]   -= m_lateralBob * 0.5;
	pviewmodel->curstate.angles[ROLL]  -= m_lateralBob * 1;
	pviewmodel->curstate.angles[PITCH] -= m_lateralBob * 0.3;

	Math::VectorMA( pviewmodel->curstate.origin, m_lateralBob * 0.8f, right, pviewmodel->curstate.origin );
	Math::VectorCopy( pviewmodel->curstate.angles, pviewmodel->curstate.angles );
	Math::VectorCopy( pviewmodel->curstate.origin, pviewmodel->curstate.origin );
}

//=============================================
//
//=============================================
void CDefaultView::CalcViewModelLag( const ref_params_t& params, Vector& origin, const Vector& angles, const Vector& orig_angles )
{
	Vector forward, right, up;
	Math::AngleVectors(angles, &forward, &right, &up);

	if(m_flLastViewModelLagTime == -1)
	{
		m_flLastViewModelLagTime = params.time;
		m_viewModelLastFacing = forward;
	}

	// Set last time
	Double frametime = params.time - m_flLastViewModelLagTime;
	m_flLastViewModelLagTime = params.time;
	
	if(frametime <= 0)
	{
		frametime = 0;
		m_viewModelLastFacing = forward;
	}
	else if(frametime > 1)
		frametime = 1;

	Vector diff;
	Math::VectorSubtract(forward, m_viewModelLastFacing, diff);
	Math::VectorMA(m_viewModelLastFacing, 5*VIEWMODEL_LAG_SPEED*frametime, diff, m_viewModelLastFacing);

	Math::VectorNormalize(m_viewModelLastFacing);
	Math::VectorMA(origin, 2.5, diff*-1, origin);
	Math::AngleVectors(orig_angles, &forward, &right, &up);

	Float pitch = orig_angles[PITCH];
	if(pitch > 180.0f)
		pitch -= 360.0f;
	else if(pitch < -180.0f)
		pitch += 360.0f;

	Math::VectorMA(origin, -pitch*0.035f*VIEWMODEL_LAG_MULT, forward, origin);
	Math::VectorMA(origin, -pitch*0.03f*VIEWMODEL_LAG_MULT, right, origin);
	Math::VectorMA(origin, -pitch*0.02f*VIEWMODEL_LAG_MULT, up, origin);
}

//=============================================
//
//=============================================
Float CDefaultView::CalcRoll( const Vector& angles, const Vector& velocity, Float rollangle, Float rollspeed )
{
	Vector forward, right, up;
	Math::AngleVectors(angles, &forward, &right, &up);
	
	Float side = Math::DotProduct(velocity, right);
	Float sign = sgn(side);
	side = SDL_fabs(side);

	Float value = rollangle;
	if(side < rollspeed)
		side = side * value / rollspeed;
	else
		side = value;

	return side*sign;
}

//=============================================
//
//=============================================
void CDefaultView::CalcViewModelAngle( cl_entity_t* pviewmodel, const ref_params_t& params )
{
	if(!pviewmodel)
		return;

	pviewmodel->curstate.angles[YAW] = params.v_angles[YAW] + m_currentAutoAimVector[YAW];
	pviewmodel->curstate.angles[PITCH] = -params.v_angles[PITCH] - m_currentAutoAimVector[PITCH];

	Math::VectorCopy(pviewmodel->curstate.angles, pviewmodel->curstate.angles);
	Math::VectorCopy(pviewmodel->curstate.angles, pviewmodel->latched.angles);
}

//=============================================
//
//=============================================
void CDefaultView::AddIdle( cl_entity_t* pplayer, ref_params_t& params )
{
	if(pplayer->curstate.health <= 0 || pplayer->curstate.flags & FL_DEAD)
		return;

	if(params.onground == NO_ENTITY_INDEX || pplayer->curstate.movetype == MOVETYPE_NOCLIP || params.waterlevel > WATERLEVEL_LOW)
		return;

	if(m_breathingTime == -1)
		m_breathingTime = params.time;

	Float time = params.time - m_breathingTime;
	Float zadd = SDL_sin(time*2.0) * 0.1;
	params.v_origin.z += zadd;
}

//=============================================
//
//=============================================
Float CDefaultView::CalcSmoothRolling( Float cur_roll, Float target_roll, Float speed, Double frametime )
{
	if(cur_roll == target_roll)
		return cur_roll;

	Float moveamount = target_roll - cur_roll;
	Float turnspeed = speed * 0.1 * frametime;

	if(target_roll > cur_roll)
	{
		if(moveamount >= 180)
			moveamount = moveamount - 360;
	}
	else
	{
		if(moveamount <= -180)
			moveamount = moveamount + 360;
	}

	if(moveamount > 0)
	{
		if(moveamount > turnspeed)
			moveamount = turnspeed;
	}
	else
	{
		if(moveamount < -turnspeed)
			moveamount = -turnspeed;
	}

	return cur_roll + moveamount;
}

//=============================================
//
//=============================================
void CDefaultView::CalcViewRoll( cl_entity_t* pplayer, ref_params_t& params )
{
	if(!pplayer)
		return;

	if(pplayer->curstate.movetype == MOVETYPE_NOCLIP)
		return;

	Float targetroll = CalcRoll(params.v_angles, params.pl_velocity, m_pCvarRollAngle->GetValue(), m_pCvarRollSpeed->GetValue()) * 4;
	m_currentViewRoll = CalcSmoothRolling( m_currentViewRoll, targetroll, m_pCvarRollSpeed->GetValue(), params.frametime );
	params.v_angles[ROLL] += m_currentViewRoll;

	if(pplayer->curstate.health <= 0 && params.pl_viewoffset[2] != 0)
		params.v_angles[ROLL] = 80;
}

//=============================================
//
//=============================================
void CDefaultView::CalcLeaning( cl_entity_t* pplayer, cl_entity_t *pviewmodel, ref_params_t& params )
{
	Int32 flagsChanged = (m_prevPlayerFlags ^ pplayer->curstate.flags);
	m_prevPlayerFlags = pplayer->curstate.flags;

	Int32 buttonsChanged = ( m_prevLeanButtons ^ params.pcmd->buttons );
	m_prevLeanButtons = params.pcmd->buttons;

	if(pplayer->curstate.flags & FL_ON_BIKE)
		return;

	if(pplayer->curstate.health <= 0)
		return;

	if (params.pcmd->buttons & IN_LEAN)
	{
		if(!m_leaningState || (buttonsChanged & IN_MOVELEFT) || (buttonsChanged & IN_MOVERIGHT) || (buttonsChanged & IN_FORWARD) || (buttonsChanged & IN_BACK) || (flagsChanged & FL_DUCKING))
		{
			Math::VectorCopy(m_curLeanAngles, m_prevLeanAngles);
			Math::VectorCopy(m_curLeanOffset, m_prevLeanOffset);
			m_leanTime = params.time;
			m_leaningState = TRUE;
		}

		// Determine directions
		Float leanUp = 0;
		Float leanSide = 0;

		if( params.pcmd->buttons & IN_MOVELEFT )
			leanSide -= 1.0;

		if( params.pcmd->buttons & IN_MOVERIGHT )
			leanSide += 1.0;

		if(params.pcmd->buttons & IN_FORWARD)
		{
			if(!(pplayer->curstate.flags & FL_DUCKING))
				leanUp = 0.3;
			else
				leanUp = 1.0;
		}
		else if(params.pcmd->buttons & IN_BACK)
		{
			leanUp = -0.2;
		}
		
		// Calculate complete offset
		Vector v_forward, v_right, v_up;
		Math::AngleVectors(pplayer->curstate.angles, &v_forward, &v_right, &v_up);
		Math::VectorMA(v_right, -0.2*leanSide, v_up, v_right);
		Math::VectorMA(v_right, 0.3*leanSide, v_forward, v_right);

		Math::VectorScale(v_right, leanSide*LEAN_DISTANCE_SIDE, m_idealLeanOffset);
		Math::VectorMA(m_idealLeanOffset, leanUp*LEAN_DISTANCE_UP, v_up, m_idealLeanOffset);
		
		// Avoid clipping into the world
		trace_t ptrace;
		cl_tracefuncs.pfnPlayerTrace(params.v_origin, Vector(params.v_origin)+m_idealLeanOffset, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, ptrace);

		trace_t ptrace_cp;
		cl_tracefuncs.pfnPlayerTrace(params.v_origin, Vector(params.v_origin)+m_idealLeanOffset+v_forward*4, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, ptrace_cp);

		Double flTraceFraction = 1.0;
		if(ptrace.fraction != 1.0)
			flTraceFraction = ptrace.fraction*0.75;
		else if(ptrace_cp.fraction != 1.0)
			flTraceFraction = ptrace_cp.fraction*0.75;

		m_idealLeanAngles[YAW] = leanSide*4;
		m_idealLeanAngles[ROLL] = leanSide*16;
		m_idealLeanAngles[PITCH] = leanUp*6;

		// Calculate fraction
		Float time = clamp((params.time - m_leanTime), 0.0, LEAN_TIME );
		Float leanFraction = Common::SplineFraction( time, (1.0/LEAN_TIME) );

		// It must be recalculated every frame
		if(leanFraction > 1.0)
			leanFraction = 1.0;

		// Interpolate between previous offset and current one
		Math::VectorScale(m_idealLeanOffset, leanFraction*flTraceFraction, m_curLeanOffset);
		Math::VectorMA(m_curLeanOffset, (1.0-leanFraction), m_prevLeanOffset, m_curLeanOffset);

		// Interpolate tilt
		Math::VectorScale(m_idealLeanAngles, leanFraction, m_curLeanAngles);
		Math::VectorMA(m_curLeanAngles, (1.0-leanFraction), m_prevLeanAngles, m_curLeanAngles);
	}
	else
	{
		// Move back if we have to
		if(m_leaningState)
		{
			Math::VectorCopy(m_curLeanAngles, m_prevLeanAngles);
			Math::VectorClear(m_idealLeanAngles);
			Math::VectorCopy(m_curLeanOffset, m_prevLeanOffset);
			Math::VectorClear(m_idealLeanOffset);
			m_leanTime = params.time;
			m_leaningState = FALSE;
		}

		if(m_leanTime)
		{
			// Calculate lean interpolation
			Float time = clamp((params.time - m_leanTime), 0.0, LEAN_TIME );
			Float leanFraction = Common::SplineFraction( time, (1.0/LEAN_TIME) );

			if(leanFraction >= 1.0)
			{
				Math::VectorClear(m_curLeanOffset);
				Math::VectorClear(m_curLeanAngles);
				m_leanTime = 0;
				return;
			}

			Vector v_forward;
			Math::AngleVectors(pplayer->curstate.angles, &v_forward, nullptr, nullptr);

			// Avoid clipping into the world
			trace_t ptrace;
			cl_tracefuncs.pfnPlayerTrace(params.v_origin, Vector(params.v_origin)+m_idealLeanOffset, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, ptrace);

			trace_t ptrace_cp;
			cl_tracefuncs.pfnPlayerTrace(params.v_origin, Vector(params.v_origin)+m_idealLeanOffset+v_forward*4, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, ptrace_cp);

			Float flTraceFraction = 1.0;
			if(ptrace.fraction != 1.0)
				flTraceFraction = ptrace.fraction*0.75;
			else if(ptrace_cp.fraction != 1.0)
				flTraceFraction = ptrace_cp.fraction*0.75;

			// Add it all together
			Math::VectorScale(m_idealLeanOffset, leanFraction, m_curLeanOffset);
			Math::VectorMA(m_curLeanOffset, (1.0-leanFraction)*flTraceFraction, m_prevLeanOffset, m_curLeanOffset);

			// Interpolate tilt
			Math::VectorScale(m_idealLeanAngles, leanFraction, m_curLeanAngles);
			Math::VectorMA(m_curLeanAngles, (1.0-leanFraction), m_prevLeanAngles, m_curLeanAngles);
		}
	}

	// Add to view origin
	Math::VectorAdd(params.v_origin, m_curLeanOffset, params.v_origin);
	Math::VectorAdd(params.v_angles, m_curLeanAngles, params.v_angles);

	// Add 90% to view model
	Math::VectorMA(pviewmodel->curstate.origin, 0.9, m_curLeanOffset, pviewmodel->curstate.origin);
	Math::VectorCopy(pviewmodel->curstate.origin, pviewmodel->curstate.origin);

	Math::VectorMA(pviewmodel->curstate.angles, 0.4, m_curLeanAngles, pviewmodel->curstate.angles);
	Math::VectorCopy(pviewmodel->curstate.angles, pviewmodel->curstate.angles);
}

//=============================================
//
//=============================================
void CDefaultView::SmoothSteps( cl_entity_t* pplayer, cl_entity_t* pviewmodel, ref_params_t& params )
{
	if((params.pl_origin - m_lastStepSmoothOrigin).Length() > params.pmovevars->stepsize
		|| !Math::VectorCompare(m_lastStepSmoothViewOffset, params.pl_viewoffset))
		m_prevStepSmoothZ = params.pl_origin[2];

	// Get groundent
	cl_entity_t *pground = nullptr;
	if(pplayer->curstate.groundent != NO_ENTITY_INDEX)
		pground = cl_engfuncs.pfnGetEntityByIndex(pplayer->curstate.groundent);

	// Add movement from groundent if it's moving
	if(pground && pground->prevstate.origin.z != pground->curstate.origin.z
		&& pground->curstate.msg_time == pplayer->curstate.msg_time)
	{
		Float flzdiff = pground->curstate.origin.z - pground->prevstate.origin.z;
		Float moveDist = (pground->curstate.origin-pground->prevstate.origin).Length();

		// Skip if it's gone beyond STEPSIZE*2, probably the groundent teleported
		if( abs(flzdiff) > params.pmovevars->stepsize || moveDist > params.pmovevars->stepsize*2 )
			m_prevStepSmoothZ = params.pl_origin[2];
		else
			m_prevStepSmoothZ += flzdiff;
	}

	// Only if we allow smoothing at all
	if (params.smoothsteps && params.onground != NO_ENTITY_INDEX && params.pl_origin[2] != m_prevStepSmoothZ)
	{
		Float steptime = params.time - m_lastStepSmoothTime;
		if (steptime < 0)
			steptime = 0;

		if(steptime > 1)
		{
			m_prevStepSmoothZ = params.pl_origin[2];
			m_stepSmoothSpeed = 0;
			return;
		}

		// Determine speed factor
		float flspeed = params.pl_velocity.Length()/V_SM_REF_VEL;
		if(flspeed || m_stepSmoothSpeed)
		{
			// Moving ahead properly
			m_stepSmoothSpeed = flspeed;
			if(m_stepSmoothSpeed < 80.0f/V_SM_REF_VEL)
				m_stepSmoothSpeed = 80.0f/V_SM_REF_VEL;
		}
		else
		{
			// Probably on an elevator or something
			m_prevStepSmoothZ = params.pl_origin[2];
			return;
		}
	
		int dir = ( params.pl_origin[2] > m_prevStepSmoothZ ) ? 1 : -1;
		m_prevStepSmoothZ += steptime * m_stepSmoothSpeed * V_SM_BLEND_VEL * dir;

		// Cap it
		if ( dir > 0 )
		{
			if (m_prevStepSmoothZ > params.pl_origin[2])
				m_prevStepSmoothZ = params.pl_origin[2];

			if (params.pl_origin[2] - m_prevStepSmoothZ > params.pmovevars->stepsize)
				m_prevStepSmoothZ = params.pl_origin[2] - params.pmovevars->stepsize;
		}
		else
		{
			if (m_prevStepSmoothZ < params.pl_origin[2])
				m_prevStepSmoothZ = params.pl_origin[2];

			if (params.pl_origin[2] - m_prevStepSmoothZ < -params.pmovevars->stepsize)
				m_prevStepSmoothZ = params.pl_origin[2] + params.pmovevars->stepsize;
		}

		// Add to both view and view origin
		params.v_origin[2] += m_prevStepSmoothZ - params.pl_origin[2];
		pviewmodel->curstate.origin[2] += m_prevStepSmoothZ - params.pl_origin[2];
	}
	else
	{
		// Reset
		m_prevStepSmoothZ = params.pl_origin[2];
		m_stepSmoothSpeed = 0;
	}

	// Remember the lasttime
	m_lastStepSmoothTime = params.time;

	// Save for later
	Math::VectorCopy(params.pl_origin, m_lastStepSmoothOrigin);
	Math::VectorCopy(params.pl_viewoffset, m_lastStepSmoothViewOffset);
}

//=============================================
//
//=============================================
void CDefaultView::CalcSwimFloat( cl_entity_t* pplayer, ref_params_t& params )
{
	if(params.waterlevel < WATERLEVEL_MID || params.waterlevel >= WATERLEVEL_FULL || params.onground != NO_ENTITY_INDEX || pplayer->curstate.movetype == MOVETYPE_NOCLIP)
	{
		// Reset
		m_lastSwimFloatTime = 0;
		return;
	}
	else if(m_lastSwimFloatTime == 0)
	{
		// Set begin time
		m_lastSwimFloatTime = params.time;
	}

	// Add in as a sine wave
	Double floatTime = params.time - m_lastSwimFloatTime;
	Double floatAdd = SDL_sin(floatTime*2)*2;
	if(floatAdd < 0)
		floatAdd *= 0.25;

	params.v_origin[2] += floatAdd;
}

//====================================
//
//====================================
void CDefaultView::CalcRefDef( ref_params_t& params )
{
	// Retreive local player
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!pplayer)
		return;

	// Get viewmodel
	cl_entity_t* pviewmodel = gViewModel.GetViewModel();
	if(!pviewmodel)
		return;

	// Get view entity
	if(m_viewEntity != NO_ENTITY_INDEX)
	{
		cl_entity_t* pviewentity = cl_engfuncs.pfnGetEntityByIndex(m_viewEntity);
		if(pviewentity)
		{
			Math::VectorCopy(pviewentity->curstate.origin, params.v_origin);
			Math::VectorCopy(pviewentity->curstate.angles, params.v_angles);
		}

		// Add in view shaking
		gShake.CalcShake();
		gShake.ApplyShake(params.v_origin, params.v_angles, 1.0);

		// Calculate FOV
		CalculateFOV(params);
		return;
	}

	// Add in basics
	CalcBob(pplayer, params);

	Vector v_right, v_up;
	Math::AngleVectors(params.v_angles, nullptr, &v_right, &v_up);

	// Refresh position
	params.v_origin = params.pl_origin + params.pl_viewoffset;

	// Add bob
	if(params.onground != NO_ENTITY_INDEX)
	{
		Math::VectorMA(params.v_origin, -m_lateralBob*0.75, v_right*0.8+v_up*0.2, params.v_origin);
		params.v_origin[2] += m_verticalBob*3;
	}

	// Add in view shaking
	gShake.CalcShake();
	gShake.ApplyShake(params.v_origin, params.v_angles, 1.0);

	// Shift origin a bit
	for(Uint32 i = 0; i < 3; i++)
		params.v_origin[i] += 1.0/32;

	// Check for waterdist
	if(params.waterlevel >= WATERLEVEL_MID)
	{
		Float wateroffset = 0;
		Vector point = params.v_origin;

		// Use pointcontents to determine water pos
		for(Float dist = 0.1; dist < params.pmovevars->waterdist; dist += 0.1)
		{
			if(cl_tracefuncs.pfnPointContents(point, nullptr) == CONTENTS_WATER)
				break;

			point[2] = params.v_origin[2] - dist;
		}

		// Round to one digit
		Float planeheight = SDL_floor(point[2]*10)/10.0f;
		if(params.waterlevel < WATERLEVEL_FULL)
		{
			Float minheight = planeheight + params.pmovevars->waterdist;
			if(minheight > params.v_origin[2])
				wateroffset = minheight - params.v_origin[2];
		}
		else
		{
			Float maxheight = planeheight - params.pmovevars->waterdist;
			if(maxheight < params.v_origin[2])
				wateroffset = -(params.v_origin[2] - maxheight);
		}

		// Add in the offset
		params.v_origin[2] += wateroffset;
	}

	// Add in swim floating
	CalcSwimFloat(pplayer, params);

	// Add view roll
	CalcViewRoll(pplayer, params);

	// Add in breathing
	AddIdle(pplayer, params);

	// Give the viewmodel our angles
	pviewmodel->curstate.angles = params.pl_viewangles;
	Vector origangles = pviewmodel->curstate.angles;

	// Update autoaim
	CL_UpdateAutoAim(params.frametime, m_idealAutoAimVector, m_currentAutoAimVector);

	// Set angles and position
	CalcViewModelAngle(pviewmodel, params);
	pviewmodel->curstate.origin = params.v_origin;

	// Only apply viewmodel effects if set
	if(gViewModel.CanApplyOffsets())
	{
		// Add bob to view model
		AddViewModelBob(pviewmodel, params);

		// Shift origin by -1 on z
		pviewmodel->curstate.origin[2] -= 1;

		// Calculate view model lag
		CalcViewModelLag(params, pviewmodel->curstate.origin, pviewmodel->curstate.angles, origangles);

		// Add in leaning
		CalcLeaning(pplayer, pviewmodel, params);
	}

	// Add in punchangles
	Math::VectorAdd(params.v_angles, params.pl_punchangle, params.v_angles);

	// Add stair smoothing
	SmoothSteps(pplayer, pviewmodel, params);

	// Calculate FOV
	CalculateFOV(params);
}

//=============================================
//
//=============================================
Vector CDefaultView::GetLeanOffset( void ) const
{
	return m_curLeanOffset;
}

//=============================================
//
//=============================================
void CDefaultView::SetFOV( Float newFOV )
{
	if(!newFOV)
		m_fovValue = m_pCvarReferenceFOV->GetValue();
	else
		m_fovValue = newFOV;
}

//=============================================
//
//=============================================
void CDefaultView::SetFOVOverride( Float overrideFOV )
{
	// Set override value
	m_fovOverrideValue = overrideFOV;
}

//=============================================
//
//=============================================
void CDefaultView::CalculateFOV( ref_params_t& params )
{
	if(m_desiredFOVValue)
	{
		if(m_fovBlendTime == -1)
			m_fovBlendTime = params.time;

		if(!m_fovBlendDelta)
		{
			// Just set the override
			SetFOVOverride(m_desiredFOVValue);
			m_desiredFOVValue = 0;
			m_prevFOVValue = 0;
		}
		else
		{
			// Calculate fraction
			Float frac = params.time - m_fovBlendTime;
			frac /= m_fovBlendDelta;

			if(frac >= 1.0)
			{
				// Clear everything
				SetFOVOverride(0);
				m_desiredFOVValue = 0;
				m_prevFOVValue = 0;
			}
			else
			{
				// Calculate FOV value
				Float fovValue = m_prevFOVValue*(1.0-frac)+m_desiredFOVValue*frac;
				SetFOVOverride(fovValue);
			}
		}
	}

	// Set final FOV value
	params.viewsize = GetFOV();
}

//=============================================
//
//=============================================
void CDefaultView::SetFOVZoom( Float desiredFOV, Float blenddelta )
{
	// Save previous FOV
	if(m_fovOverrideValue)
		m_prevFOVValue = m_fovOverrideValue;
	else
		m_prevFOVValue = m_fovValue;

	m_desiredFOVValue = desiredFOV;

	m_fovBlendDelta = blenddelta;
	m_fovBlendTime = -1;
}

//=============================================
//
//=============================================
void CDefaultView::SetupFlashlightForType( const ref_params_t& params, Float* pstrengths, Int32 effectbit )
{
	Vector vAngles, vOrigin;

	Uint32 maxclients = cl_engfuncs.pfnGetMaxClients();
	for(Uint32 i = 0; i < maxclients; i++)
	{
		cl_entity_t *pEntity = cl_engfuncs.pfnGetEntityByIndex(i+1);

		if(!pEntity)
			return;

		if(!pEntity->player)
			continue;

		if(!pEntity->pmodel)
			continue;
	
		// Decay the light
		if(pEntity->curstate.effects & effectbit 
			&& pEntity->curstate.aiment == NO_ENTITY_INDEX)
		{
			if(!(pEntity->prevstate.effects & effectbit))
				pstrengths[i] = 0;

			if(pstrengths[i] != 1.0)
			{
				pstrengths[i] += FLASHLIGHT_FADE_SPEED*params.frametime;

				if(pstrengths[i] > 1)
					pstrengths[i] = 1;
			}
		}
		else if(pstrengths[i] <= 0 || pEntity->curstate.aiment != NO_ENTITY_INDEX)
			continue;

		// Decay the light
		if(!(pEntity->curstate.effects & effectbit))
		{
			if(pstrengths[i] != 0)
			{
				pstrengths[i] -= FLASHLIGHT_FADE_SPEED*params.frametime;

				if(pstrengths[i] <= 0)
					pstrengths[i] = 0;
			}
		}

		if(pEntity == cl_engfuncs.pfnGetLocalPlayer())
		{
			Vector vdir, up, right;
			if(effectbit & EF_SHOULDERLIGHT)
			{
				Math::VectorCopy( params.v_angles, vAngles );
				Math::AngleVectors( vAngles, &vdir, &right, &up );

				vOrigin = params.v_origin - right * 4 - up * 3;
			}
			else if(effectbit & EF_DIMLIGHT)
			{
				cl_entity_t *pView = gViewModel.GetViewModel();
				if(!pView || !pView->pmodel)
					continue;

				cl_engfuncs.pfnUpdateAttachments(pView);
				
				Math::VectorSubtract(pView->getAttachment(1), pView->getAttachment(0), vdir);
				vdir.Normalize();

				Math::GetUpRight(vdir, up, right);
				Math::VectorScale(right, -1, right);
				vAngles = Math::VectorToAngles(vdir, right);
				Math::VectorCopy(pView->getAttachment(3), vOrigin);

				if(m_pFlashlightSprite)
				{
					cl_entity_t* psprite = cl_efxapi.pfnAllocTempSpriteEntity(pEntity->entindex, 0.1);

					Math::VectorCopy( pView->getAttachment(3), psprite->curstate.origin );

					psprite->pmodel = m_pFlashlightSprite;
					psprite->curstate.modelindex = m_pFlashlightSprite->cacheindex;

					psprite->curstate.rendercolor.x = 192;
					psprite->curstate.rendercolor.y = 192;
					psprite->curstate.rendercolor.z = 230;
					psprite->curstate.rendermode = RENDER_TRANSADDITIVE;
					psprite->curstate.renderamt = 155*pstrengths[i];
					psprite->curstate.scale = 0.05;
				}
			}
		}
		else
		{
			Math::VectorAdd(pEntity->curstate.origin, Vector(0, 0, 64), vOrigin);
			Math::VectorCopy(pEntity->curstate.angles, vAngles); 
			vAngles[PITCH] = -vAngles[PITCH];
		}

		Float cone_size = (effectbit & EF_SHOULDERLIGHT) ? 50 : 40;
		Float radius = (effectbit & EF_SHOULDERLIGHT) ? 1400 : 800;

		Int32 lightkey = pEntity->entindex;
		Int32 lightsubkey = (effectbit & EF_SHOULDERLIGHT) ? 0 : 1;
		cl_dlight_t *pdlight = cl_efxapi.pfnAllocDynamicSpotLight(lightkey, lightsubkey, false, false, pEntity);

		pdlight->origin = vOrigin;
		pdlight->angles = vAngles;
		pdlight->color = Vector(0.8, 0.8, 0.9)*pstrengths[i];
		pdlight->textureindex = 0;
		pdlight->cone_size = cone_size;
		pdlight->radius = radius;

		if(pEntity->curstate.effects & (EF_DIMLIGHT|EF_SHOULDERLIGHT))
			pdlight->die = params.time+(1/FLASHLIGHT_FADE_SPEED);
		
		cl_dlight_t *pdlight_dim = nullptr;
		if(pEntity == cl_engfuncs.pfnGetLocalPlayer())
		{
			Int32 dimLightSubkey = (effectbit & EF_SHOULDERLIGHT) ? 2 : 3;
			pdlight_dim = cl_efxapi.pfnAllocDynamicPointLight(lightkey, dimLightSubkey, false, true, pEntity);
			pdlight_dim->origin = params.v_origin;
			pdlight_dim->color = pdlight->color * 0.15; // Set base color
			pdlight_dim->radius = radius * 0.25; // Radius is 25% of base radius

			Vector forward;
			Math::AngleVectors(vAngles, &forward);

			// Set base target value
			Float dimLightTargetStrength = 0;

			// First check if view to flashlight origin is blocked. If it is, then intensity is at maximum
			trace_t tr;
			cl_tracefuncs.pfnPlayerTrace(params.v_origin, vOrigin, FL_TRACE_NORMAL, HULL_POINT, pEntity->entindex, tr);
			if(tr.startSolid() || tr.allSolid() || !tr.noHit())
			{
				// Set target value to 1.0
				dimLightTargetStrength = 1.0;
			}
			else
			{
				// Reset trace
				tr = trace_t();

				// Trace half the distance of the flashlight's max reach
				Vector traceEnd = vOrigin + forward*pdlight->radius*0.5;
				cl_tracefuncs.pfnPlayerTrace(vOrigin, traceEnd, FL_TRACE_NORMAL, HULL_POINT, pEntity->entindex, tr);
				if(!tr.noHit())
				{
					Float fraction = tr.fraction;
					if(fraction > 1.0)
						fraction = 1.0;

					static const Float DIMLIGHT_MIN_DISTANCE = 64.0f;
					Float minFraction = (DIMLIGHT_MIN_DISTANCE / radius);
					// Set target strength
					dimLightTargetStrength = (1.0 - fraction) / (1.0 - minFraction);
					if(dimLightTargetStrength > 1.0)
						dimLightTargetStrength = 1.0;

					// Set die time
					pdlight_dim->die = params.time+(1/FLASHLIGHT_FADE_SPEED);
				}
				else
				{
					// This becomes zero
					dimLightTargetStrength = 0;
				}
			}

			// Set die time
			if(dimLightTargetStrength > 0)
				pdlight_dim->die = params.time+(1/FLASHLIGHT_FADE_SPEED);

			// Reduce or increase strength smoothly
			if(m_localPlayerDimLightStrength < dimLightTargetStrength)
			{
				m_localPlayerDimLightStrength += FLASHLIGHT_FADE_SPEED*params.frametime;
				if(m_localPlayerDimLightStrength > dimLightTargetStrength)
					m_localPlayerDimLightStrength = dimLightTargetStrength;
			}
			else if(m_localPlayerDimLightStrength > dimLightTargetStrength)
			{
				m_localPlayerDimLightStrength -= FLASHLIGHT_FADE_SPEED*params.frametime;
				if(m_localPlayerDimLightStrength < dimLightTargetStrength)
					m_localPlayerDimLightStrength = dimLightTargetStrength;
			}

			// Multiply the color with the strength
			Math::VectorScale(pdlight_dim->color, m_localPlayerDimLightStrength, pdlight_dim->color);
		}

		if(pEntity == cl_engfuncs.pfnGetLocalPlayer() && gHUD.GetFlashlightBattery() < 25.0f)
		{
			pdlight->lightstyle = 6;
			if(pdlight_dim)
				pdlight_dim->lightstyle = 6;

			Float strength = gHUD.GetFlashlightBattery()/25.0f;
			Math::VectorScale(pdlight->color, strength, pdlight->color);
		}
		else
		{
			pdlight->lightstyle = 0;
			if(pdlight_dim)
				pdlight_dim->lightstyle = 0;
		}

		if(pEntity == cl_engfuncs.pfnGetLocalPlayer())
		{
			cl_entity_t *pView = gViewModel.GetViewModel();
			if(!pView || !pView->pmodel)
				continue;

			mlight_t* pel = cl_efxapi.pfnAllocEntityLight(pView->entindex, 0.1, 3);

			pel->origin = vOrigin;
			pel->color = Vector(0.75, 0.75, 0.9)*pstrengths[i];
			pel->radius = (effectbit & EF_SHOULDERLIGHT) ? 32 : 16;
		}
	}
}

//=============================================
//
//=============================================
void CDefaultView::SetupFlashlights( const ref_params_t& params )
{
	// Set up for tactical flashlights
	SetupFlashlightForType(params, m_tacticalLightStrengths, EF_DIMLIGHT);
	// Set up for shoulder flashlights
	SetupFlashlightForType(params, m_shoulderLightStrengths, EF_SHOULDERLIGHT);	
}

//=============================================
//
//=============================================
Float CDefaultView::GetFOV( void ) const
{
	if(m_fovOverrideValue)
		return m_fovOverrideValue;
	else
		return m_fovValue;
}

//=============================================
//
//=============================================
void CDefaultView::SetViewEntity( entindex_t viewentity )
{
	m_viewEntity = viewentity;
}

//=============================================
//
//=============================================
entindex_t CDefaultView::GetViewEntity( void ) const
{
	return m_viewEntity;
}

//=============================================
//
//=============================================
void CDefaultView::SetAutoAim( Float autoAimX, Float autoAimY )
{
	m_idealAutoAimVector[0] = autoAimX;
	m_idealAutoAimVector[1] = autoAimY;
}

//=============================================
//
//=============================================
void V_CalcRefDef( ref_params_t& params )
{
	if(gLadder.IsActive())
	{
		// Calculate ladder refdef
		gLadder.CalcRefDef(params);
	}
	else if(gMotorBike.IsActive())
	{
		// Calculate motorbike refdef
		gMotorBike.CalcRefDef(params);
	}
	else if(gViewController.IsActive())
	{
		// Calculate view controller refdef
		gViewController.CalcRefDef(params);
	}
	else
	{
		// Calculate normal refdef
		gDefaultView.CalcRefDef(params);
	}

	// Always set these
	gDefaultView.SetupFlashlights(params);
	g_viewOrigin = params.v_origin;
}

//=============================================
//
//=============================================
void V_GetViewInfo( Vector& vOrigin, Vector& vAngles )
{
	vOrigin.Clear();
	vAngles.Clear();

	if(gLadder.IsActive())
	{
		// Get it from the ladder entity
		gLadder.GetViewInfo(vOrigin, vAngles);
	}
	else if(gMotorBike.IsActive())
	{
		// Get it from motorbike entity
		gMotorBike.GetViewInfo(vOrigin, vAngles);
	}
	else if(gViewController.IsActive())
	{
		// Get it from view controller
		gViewController.GetViewInfo(vOrigin, vAngles);
	}
	else
	{
		entindex_t viewentityindex = gDefaultView.GetViewEntity();
		if(viewentityindex != NO_ENTITY_INDEX)
		{
			// Retrieve view entity
			cl_entity_t* pviewentity = cl_engfuncs.pfnGetEntityByIndex(viewentityindex);
			if(!pviewentity)
				return;

			vOrigin = pviewentity->curstate.origin;
			vAngles = pviewentity->curstate.angles;
		}
		else
		{
			// Retreive local player
			cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
			if(!pplayer)
				return;

			// Comes from normal player
			vOrigin = pplayer->curstate.origin + pplayer->curstate.view_offset;
			vAngles = pplayer->curstate.viewangles;
		}
	}
}