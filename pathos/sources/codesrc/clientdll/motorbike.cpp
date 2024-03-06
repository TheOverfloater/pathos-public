/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "motorbike.h"
#include "clientdll.h"
#include "vbm_shared.h"
#include "cache_model.h"
#include "studio.h"
#include "input.h"
#include "com_math.h"
#include "trace.h"
#include "movevars.h"
#include "snd_shared.h"
#include "view.h"
#include "efxapi.h"
#include "dlight.h"
#include "entity_extrainfo.h"
#include "ref_params.h"
#include "shake.h"
#include "viewmodel.h"
#include "msgreader.h"
#include "viewcontroller.h"
#include "cvar.h"

// Object definition
CMotorBike gMotorBike;

// Max wheel turn angle
const Float CMotorBike::MOTORBIKE_TURN_ANGLE = 25;
// Max view deviation
const Float CMotorBike::MOTORBIKE_MAX_ADD = 45;
// Mouse move timeout
const Float CMotorBike::MOUSEMOVE_TIMEOUT = 8;
// Mouse timeout blend time
const Float CMotorBike::MOUSEMOVE_TIMEOUT_BLEND	= 1;
// Surface align time
const Double CMotorBike::ANGLE_ALIGN_TIME_ON_GROUND = 0.5;
// Surface align time
const Double CMotorBike::ANGLE_ALIGN_TIME_IN_AIR = 2.0;
// Wheel blend time
const Double CMotorBike::WHEEL_BLEND_TIME = 1.0;
// Motorbike volume
const Float CMotorBike::MOTORBIKE_VOLUME = 0.25;
// Motorbike model name
const Char CMotorBike::MOTORBIKE_MODEL_NAME[] = "models/motorbike.mdl";
// Motorbike sequence names(this needs to line up with bike_anims_t
const Char* CMotorBike::MOTORBIKE_ANIM_NAMES[NB_BIKE_ANIMS] =
{
	"standby",
	"enter",
	"exit",
	"idle",
	"forward",
	"turnleft",
	"left",
	"leftback",
	"turnright",
	"right",
	"rightback",
	"trashed",
};

//=============================================
// @brief
//
//=============================================
CMotorBike::CMotorBike( void ):
	m_pBikeEntity(nullptr),
	m_pPlayer(nullptr),
	m_pBikeModel(nullptr),
	m_time(0),
	m_pCvarTurnRoll(nullptr),
	m_acceleration(0),
	m_lastInputTime(-1),
	m_prevPlayerGroundEntity(NO_ENTITY_INDEX),
	m_serverStatus(BIKE_SV_INACTIVE),
	m_activeServerStatus(BIKE_SV_INACTIVE),
	m_nextUpdateTime(-1),
	m_animState(BIKE_STATE_STANDBY),
	m_idealAnimState(BIKE_STATE_STANDBY),
	m_nextSequenceTime(-1),
	m_isViewSaved(0),
	m_lerpBeginTime(0),
	m_lerpEndTime(0),
	m_soundTime(0),
	m_soundState(BIKE_SOUND_NONE),
	m_direction(0),
	m_prevDirection(0),
	m_wheelAngle(0),
	m_prevWheelAngle(0),
	m_playBrakeSound(false),
	m_prevForwardState(0),
	m_prevBackState(0),
	m_lastMouseMove(0),
	m_angleBlendTime(0),
	m_angleBlendDuration(0)
{
}

//=============================================
// @brief
//
//=============================================
CMotorBike::~CMotorBike( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CMotorBike::Init( void )
{
	m_pCvarTurnRoll = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_SAVE|FL_CV_CLIENT), "cl_bikeroll", "1", "Toggle bike view roll");
	return true;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::Shutdown( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CMotorBike::InitGame( void )
{
	// Set up motorbike entity
	m_clientBikeEntity.curstate.framerate = 1;
	m_clientBikeEntity.entindex = m_clientBikeEntity.curstate.entindex = BIKE_ENTITY_INDEX;
	m_clientBikeEntity.curstate.effects |= (EF_CLIENTENT|EF_QUAKEBUG_FIX);

	if(!m_pBikeModel)
	{
		m_pBikeModel = cl_engfuncs.pfnLoadModel(MOTORBIKE_MODEL_NAME);
		if(!m_pBikeModel)
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Failed to load '%s'.\n", __FUNCTION__, MOTORBIKE_MODEL_NAME);
			return false;
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::ClearGame( void )
{
	// Clear motorbike entity
	m_clientBikeEntity = cl_entity_t();

	// Reset every state
	m_serverStatus			= BIKE_SV_INACTIVE;
	m_activeServerStatus	= BIKE_SV_INACTIVE;
	m_animState				= BIKE_STATE_STANDBY;
	m_idealAnimState		= BIKE_STATE_STANDBY;
	m_soundState			= BIKE_SOUND_NONE;

	m_nextUpdateTime		= -1;
	m_acceleration			= 0;
	m_lastInputTime			= -1;
	m_nextSequenceTime		= -1;
	m_pBikeEntity			= nullptr;
	m_lerpBeginTime 		= 0;
	m_lerpEndTime 			= 0;
	m_soundTime				= 0;
	m_direction				= 0;
	m_prevDirection			= 0;
	m_wheelAngle			= 0;
	m_prevWheelAngle		= 0;
	m_lastMouseMove			= 0;
	m_angleBlendTime		= 0;
	m_angleBlendDuration	= 0;
	m_pBikeModel			= nullptr;
	m_isViewSaved 			= false;
	m_playBrakeSound		= false;

	m_exitOrigin.Clear();
	m_exitOrigin.Clear();
	m_exitAngles.Clear();
	m_viewAngles.Clear();
	m_viewOrigin.Clear();
	m_savedVelocity.Clear();
	m_punchAngle.Clear();
	m_punchAmount.Clear();
	m_lerpBeginOrigin.Clear();
	m_lerpBeginAngles.Clear();
	m_lerpEndAngles.Clear();
	m_lerpEndOrigin.Clear();
	m_deviationAngles.Clear();
}

//=============================================
// @brief
//
//=============================================
bool CMotorBike::IsActive( void ) const
{
	if(!m_pBikeEntity)
		return false;

	if(m_activeServerStatus == BIKE_SV_INACTIVE 
		|| m_activeServerStatus == BIKE_SV_CLEANUP
		|| m_activeServerStatus == BIKE_SV_RESTORE)
		return false;
	
	if(!m_clientBikeEntity.pmodel)
		return false;

	return true;
};

//=============================================
// @brief
//
//=============================================
cl_entity_t *CMotorBike::GetBikeEntity ( void )
{
	if(m_activeServerStatus == BIKE_SV_CLEANUP)
		return nullptr;

	if(m_activeServerStatus == BIKE_SV_INACTIVE)
		return nullptr;

	if(!m_clientBikeEntity.pmodel)
		return nullptr;

	return &m_clientBikeEntity;
};

//=============================================
// @brief
//
//=============================================
Float CMotorBike::GetTurnAmount( void )
{
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!pplayer)
		return 0;

	Double fltime = cl_engfuncs.pfnGetClientTime();

	switch(m_animState)
	{
	case BIKE_STATE_TURNLEFT:
		{
			Double prevFrac = 1.0 - ((fltime-m_lastInputTime)/SequenceTime(BIKE_ANIM_TURNLEFT));
			m_wheelAngle = m_prevWheelAngle*prevFrac + (fltime-m_lastInputTime)/SequenceTime(BIKE_ANIM_TURNLEFT);
		}
		break;
	case BIKE_STATE_LEFT:
			m_wheelAngle = 1.0;
		break;
	case BIKE_STATE_LEFTBACK:
		m_wheelAngle = m_prevWheelAngle*(1.0-((fltime-m_lastInputTime)/SequenceTime(BIKE_ANIM_LEFTBACK)));
		break;
	case BIKE_STATE_TURNRIGHT:
		{
			Float prevFrac = 1.0 - ((fltime-m_lastInputTime)/SequenceTime(BIKE_ANIM_TURNRIGHT));
			m_wheelAngle = m_prevWheelAngle*prevFrac + (fltime-m_lastInputTime)/SequenceTime(BIKE_ANIM_TURNRIGHT) * -1.0;
		}
		break;
	case BIKE_STATE_RIGHT:
		m_wheelAngle = -1.0;
		break;
	case BIKE_STATE_RIGHTBACK:
		m_wheelAngle = m_prevWheelAngle*(1.0-((fltime-m_lastInputTime)/SequenceTime(BIKE_ANIM_RIGHTBACK)));
		break;
	default:
		if(m_wheelAngle)
			m_wheelAngle = 0;
		break;
	}

	Double frametime = cl_engfuncs.pfnGetFrameTime();
	Float wheelangle = ((m_wheelAngle*MOTORBIKE_TURN_ANGLE*pplayer->curstate.velocity.Length())/90.0f)*m_direction*frametime;

	return wheelangle;
}

//=============================================
// @brief
//
//=============================================
Float CMotorBike::GetAcceleration( void ) const
{
	return m_acceleration;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::SetSequence( Int32 sequence )
{
	if(!m_pBikeModel || !m_pBikeModel->pcachedata)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Bike model not set.\n", __FUNCTION__);
		return;
	}

	if(sequence >= NB_BIKE_ANIMS || sequence < 0)
	{
		cl_engfuncs.pfnCon_Printf("%s - Bogus sequence '%d' specified.\n", __FUNCTION__, sequence);
		return;
	}

	const vbmcache_t* pcache = m_pBikeModel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	// Find the sequence
	Int32 sequenceindex = VBM_FindSequence(pstudiohdr, MOTORBIKE_ANIM_NAMES[sequence]);
	if(sequenceindex == NO_SEQUENCE_VALUE)
	{
		cl_engfuncs.pfnCon_Printf("%s - Unknown sequence '%s' specified.\n", __FUNCTION__, MOTORBIKE_ANIM_NAMES[sequence]);
		return;
	}

	m_clientBikeEntity.latched.prevseqblending[0] = m_clientBikeEntity.curstate.blending[0];
	m_clientBikeEntity.latched.prevseqblending[1] = m_clientBikeEntity.curstate.blending[1];

	m_clientBikeEntity.latched.sequence = m_clientBikeEntity.curstate.sequence;
	m_clientBikeEntity.latched.animtime = m_clientBikeEntity.curstate.animtime;

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(m_clientBikeEntity.curstate.sequence);
	m_clientBikeEntity.latched.frame = VBM_EstimateFrame(pseqdesc, m_clientBikeEntity.curstate, m_time);
	m_clientBikeEntity.latched.sequencetime = m_time;

	m_clientBikeEntity.curstate.sequence = sequenceindex;
	m_clientBikeEntity.curstate.animtime = m_time;
	m_clientBikeEntity.curstate.frame = 0;
}

//=============================================
// @brief
//
//=============================================
Float CMotorBike::SequenceTime( Int32 sequence ) const
{
	if(!m_pBikeModel || !m_pBikeModel->pcachedata)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Bike model not set.\n", __FUNCTION__);
		return 0;
	}

	if(sequence >= NB_BIKE_ANIMS || sequence < 0)
	{
		cl_engfuncs.pfnCon_Printf("%s - Bogus sequence '%d' specified.\n", __FUNCTION__, sequence);
		return 0;
	}

	const vbmcache_t* pcache = m_pBikeModel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	// Find the sequence
	Int32 sequenceindex = VBM_FindSequence(pstudiohdr, MOTORBIKE_ANIM_NAMES[sequence]);
	if(sequenceindex == NO_SEQUENCE_VALUE)
	{
		cl_engfuncs.pfnCon_Printf("%s - Unknown sequence '%s' specified.\n", __FUNCTION__, MOTORBIKE_ANIM_NAMES[sequence]);
		return 0;
	}

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequenceindex);
	return ((Float)pseqdesc->numframes/(Float)pseqdesc->fps);
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::HandleInput( void )
{
	if(m_activeServerStatus != BIKE_SV_ACTIVE)
		return;

	if(cmd_moveleft.state & 1 && cmd_moveright.state & 1 && m_animState != BIKE_STATE_READY && m_animState != BIKE_STATE_FORWARD && m_animState != BIKE_STATE_BACK)
	{
		if(m_animState == BIKE_STATE_TURNLEFT || m_animState == BIKE_STATE_LEFT)
		{
			m_prevWheelAngle = m_wheelAngle;
			m_idealAnimState = BIKE_STATE_LEFTBACK;
			m_nextSequenceTime = m_lastInputTime = m_time;
		}
		else if(m_animState == BIKE_STATE_TURNRIGHT || m_animState == BIKE_STATE_RIGHT)
		{
			m_prevWheelAngle = m_wheelAngle;
			m_idealAnimState = BIKE_STATE_RIGHTBACK;
			m_nextSequenceTime = m_lastInputTime = m_time;
		}
	}
	else
	{
		if(cmd_moveleft.state & 1)
		{
			if(m_animState == BIKE_STATE_READY || m_animState == BIKE_STATE_FORWARD || m_animState == BIKE_STATE_TURNRIGHT || m_animState == BIKE_STATE_RIGHT || m_animState == BIKE_STATE_RIGHTBACK)
			{
				m_prevWheelAngle = m_wheelAngle;
				m_idealAnimState = BIKE_STATE_TURNLEFT;
				m_nextSequenceTime = m_lastInputTime = m_time;
			}
			else if(m_nextSequenceTime <= m_time && m_animState == BIKE_STATE_TURNLEFT)
			{
				m_prevWheelAngle = m_wheelAngle;
				m_animState = m_idealAnimState = BIKE_STATE_LEFT;
				m_nextSequenceTime = m_lastInputTime = m_time;
				SetSequence(BIKE_ANIM_LEFT);
			}
		}
		else
		{
			if(m_animState == BIKE_STATE_TURNLEFT || m_animState == BIKE_STATE_LEFT)
			{
				m_animState = BIKE_STATE_LEFTBACK;
				m_idealAnimState = BIKE_STATE_READY;
				m_nextSequenceTime = m_time+SequenceTime(BIKE_ANIM_LEFTBACK);
				m_prevWheelAngle = m_wheelAngle;
				m_lastInputTime = m_time;
				SetSequence(BIKE_ANIM_LEFTBACK);
			}
		}
		if(cmd_moveright.state & 1)
		{
			if(m_animState == BIKE_STATE_READY || m_animState == BIKE_STATE_FORWARD || m_animState == BIKE_STATE_TURNLEFT || m_animState == BIKE_STATE_LEFT || m_animState == BIKE_STATE_LEFTBACK)
			{
				m_prevWheelAngle = m_wheelAngle;
				m_idealAnimState = BIKE_STATE_TURNRIGHT;
				m_nextSequenceTime = m_lastInputTime = m_time;
			}
			else if(m_nextSequenceTime <= m_time)
			{
				if(m_animState == BIKE_STATE_TURNRIGHT)
				{
					m_prevWheelAngle = m_wheelAngle;
					m_animState = m_idealAnimState = BIKE_STATE_RIGHT;
					m_nextSequenceTime = m_lastInputTime = m_time;
					SetSequence(BIKE_ANIM_RIGHT);
				}
			}
		}
		else
		{
			if(m_animState == BIKE_STATE_TURNRIGHT || m_animState == BIKE_STATE_RIGHT)
			{
				m_animState = BIKE_STATE_RIGHTBACK;
				m_idealAnimState = BIKE_STATE_READY;
				m_nextSequenceTime = m_time+SequenceTime(BIKE_ANIM_LEFTBACK);
				m_prevWheelAngle = m_wheelAngle;
				m_lastInputTime = m_time;
				SetSequence(BIKE_ANIM_RIGHTBACK);
			}
		}

		if(m_nextSequenceTime <= m_time)
		{
			cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();

			if(m_idealAnimState == BIKE_STATE_READY)
			{
				if(!pplayer->curstate.velocity.IsZero() && m_acceleration)
				{
					m_prevWheelAngle = m_wheelAngle;
					m_lastInputTime = m_time;
					m_idealAnimState = BIKE_STATE_FORWARD;
					m_nextSequenceTime = m_time;
				}
			}
			else if(m_animState == BIKE_STATE_FORWARD)
			{
				if(pplayer->curstate.velocity.IsZero() && !m_acceleration)
				{
					m_prevWheelAngle = m_wheelAngle;
					m_lastInputTime = m_time;
					m_idealAnimState = BIKE_STATE_READY;
					m_nextSequenceTime = m_time;
				}
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::MouseMove( Float mousex, Float mousey )
{
	if(m_activeServerStatus == BIKE_SV_LEAVING || m_activeServerStatus == BIKE_SV_LEAVING_LERP)
		return;

	if(!mousex && !mousey)
		return;

	Double time = cl_engfuncs.pfnGetClientTime();
	if(m_lastMouseMove+MOUSEMOVE_TIMEOUT < time)
		Math::VectorCopy(m_addDeviationAngles, m_deviationAngles);

	m_deviationAngles.x += mousey; // wtf this is switched for some reason
	if(m_deviationAngles.x > MOTORBIKE_MAX_ADD*0.75) m_deviationAngles.x = MOTORBIKE_MAX_ADD*0.75;
	if(m_deviationAngles.x < -MOTORBIKE_MAX_ADD) m_deviationAngles.x = -MOTORBIKE_MAX_ADD;

	m_deviationAngles.y -= mousex;
	if(m_deviationAngles.y > MOTORBIKE_MAX_ADD) m_deviationAngles.y = MOTORBIKE_MAX_ADD;
	if(m_deviationAngles.y < -MOTORBIKE_MAX_ADD) m_deviationAngles.y = -MOTORBIKE_MAX_ADD;

	m_lastMouseMove = time;
};

//=============================================
// @brief
//
//=============================================
void CMotorBike::RestoreBike( void )
{
	m_clientBikeEntity.pmodel = m_pBikeModel;
	m_clientBikeEntity.curstate.modelindex = m_pBikeModel->cacheindex;

	Math::VectorCopy(m_pPlayer->curstate.velocity, m_savedVelocity);
	Math::VectorCopy(m_pPlayer->curstate.origin, m_clientBikeEntity.curstate.origin);
	Math::VectorAdd(m_clientBikeEntity.curstate.origin, Vector(0, 0, VEC_HULL_MIN[2]), m_clientBikeEntity.curstate.origin);
	Math::VectorCopy(m_clientBikeEntity.curstate.origin, m_clientBikeEntity.prevstate.origin);
	
	m_prevPlayerGroundEntity = m_pPlayer->curstate.groundent;

	m_nextSequenceTime = m_nextUpdateTime = m_time;
	m_idealAnimState = BIKE_STATE_READY;
	m_serverStatus = BIKE_SV_ACTIVE;

	if(cmd_forward.state & 1)
		m_direction = 1;
	else if(cmd_back.state & 1) 
		m_direction = -1;
	else
		m_direction = 0;

	m_prevDirection = m_direction;

	if(m_acceleration)
	{
		m_soundState = BIKE_SOUND_ACCEL_BEGIN;
		m_clientBikeEntity.curstate.sequence = BIKE_ANIM_FORWARD;
	}
	else
	{
		m_clientBikeEntity.curstate.sequence = BIKE_ANIM_IDLE;
	}

	m_clientBikeEntity.latched.sequence = m_clientBikeEntity.curstate.sequence;
	m_clientBikeEntity.latched.animtime = m_time;
	m_clientBikeEntity.latched.frame = 0;
	m_clientBikeEntity.latched.sequencetime = 0;

	m_clientBikeEntity.curstate.body = 1;
	m_clientBikeEntity.curstate.animtime = m_time;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::UpdateSequenceStates( void )
{
	if(m_nextSequenceTime == -1)
		return;

	if(m_animState == m_idealAnimState)
		return;

	if(m_nextSequenceTime > m_time)
		return;
			
	m_animState = m_idealAnimState;
	m_nextSequenceTime = -1;

	if(m_animState == BIKE_STATE_ENTER)
	{
		SetSequence(BIKE_ANIM_ENTER);
		m_nextSequenceTime = m_time + SequenceTime(BIKE_ANIM_ENTER);
		m_idealAnimState = BIKE_STATE_READY;
	}
	else if(m_animState == BIKE_STATE_LEAVE_LERP)
	{
		SetSequence(BIKE_ANIM_STANDBY);
	}
	else if(m_animState == BIKE_STATE_ENTER_LERP)
	{
		SetSequence(BIKE_ANIM_STANDBY);
	}
	else if(m_animState == BIKE_STATE_READY)
	{
		SetSequence(BIKE_ANIM_IDLE);
	}
	else if(m_animState == BIKE_STATE_FORWARD)
	{
		SetSequence(BIKE_ANIM_FORWARD);
	}
	else if(m_animState == BIKE_STATE_TURNLEFT)
	{
		SetSequence(BIKE_ANIM_TURNLEFT);
		m_nextSequenceTime = m_time + SequenceTime(BIKE_ANIM_TURNLEFT);
	}
	else if(m_animState == BIKE_STATE_TURNRIGHT)
	{
		SetSequence(BIKE_ANIM_TURNRIGHT);
		m_nextSequenceTime = m_time + SequenceTime(BIKE_ANIM_TURNRIGHT);
	}
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::ActiveThink( void )
{
	Float flLastVelocity = m_savedVelocity.Length();
	Float flVelocity = m_pPlayer->curstate.velocity.Length();
	Float flDiff = SDL_fabs(flLastVelocity - flVelocity);

	// Get engine stuff
	Double frametime = cl_engfuncs.pfnGetFrameTime();
	const movevars_t* pmovevars = cl_engfuncs.pfnGetMoveVars();

	// Play appropriate sounds, and disable acceleration
	if(flDiff > MOTORBIKE_MINIMUM_KNOCK_SPEED)
	{
		// Test the acceleration dest
		Vector testOrigin = m_pPlayer->curstate.origin + m_savedVelocity * frametime;

		trace_t tr;
		cl_tracefuncs.pfnPlayerTrace(m_pPlayer->curstate.origin, testOrigin, FL_TRACE_NORMAL, HULL_HUMAN, NO_ENTITY_INDEX, tr);

		// Get direction of movement
		Vector velDir = m_savedVelocity;
		velDir.Normalize();

		// Determine if we can slide off a bit
		if(Math::DotProduct(velDir, tr.plane.normal) < 0.6 && tr.fraction != 1.0)
		{
			// Save previous fraction
			Double groundFraction = tr.fraction;
			Vector raisedOrigin = m_pPlayer->curstate.origin + Vector(0, 0, pmovevars->stepsize);
			testOrigin = raisedOrigin + m_savedVelocity * frametime;

			cl_tracefuncs.pfnPlayerTrace(raisedOrigin, testOrigin, FL_TRACE_NORMAL, HULL_HUMAN, NO_ENTITY_INDEX, tr);

			if(groundFraction >= tr.fraction)
			{
				if(flDiff >= MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)
				{
					Float impactDamage = (flDiff-MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)/(MOTORBIKE_FATAL_COLLISON_SPEED-MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)*100;

					// Reset the sound to idle
					m_soundState = BIKE_SOUND_IDLE;
					
					// Remove acceleration and add punch
					m_acceleration = m_acceleration * -0.01;
					m_punchAmount[0] -= 300*clamp(impactDamage/100, 0, 1);

					CString soundfile;
					switch(Common::RandomLong(0, 3))
					{
					case 0: soundfile = "bike/bike_crash1.wav"; break;
					case 1: soundfile = "bike/bike_crash2.wav"; break;
					case 2: soundfile = "bike/bike_crash3.wav"; break;
					case 3: soundfile = "bike/bike_crash4.wav"; break;
					}

					cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, soundfile.c_str(), MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
				}
				// Add in a little punch
				if(flDiff > MOTORBIKE_MINIMUM_KNOCK_SPEED)
				{
					Float amount = flDiff/MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED;
					if(amount < 0.1)
						amount = 0.1;

					m_punchAmount[0] += 90*amount;
					m_acceleration *= 0.8;

					if(flDiff < MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)
					{
						CString soundfile;
						switch(Common::RandomLong(0, 1))
						{
						case 0: soundfile = "bike/bike_bump1.wav"; break;
						case 1: soundfile = "bike/bike_bump2.wav"; break;
						}

						cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_ITEM, soundfile.c_str(), MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
					}
				}
			}
		}
	}

	// Save for bump check
	Math::VectorCopy(m_pPlayer->curstate.velocity, m_savedVelocity);

	if(cmd_jump.state & 1)
	{
		if(m_acceleration)
		{
			if(m_acceleration < 0)
			{
				m_acceleration += MOTORBIKE_BRAKE_DECELERATION*frametime;
				if(m_acceleration > 0) m_acceleration = 0;
			}
			else
			{
				m_acceleration -= MOTORBIKE_BRAKE_DECELERATION*frametime;
				if(m_acceleration < 0) m_acceleration = 0;
			}
		}
	}
	else
	{
		// Calculate acceleration
		if(cmd_forward.state & 1) 
			m_direction = 1;
		else if(cmd_back.state & 1) 
			m_direction = -1;
		else if(!m_acceleration)
			m_direction = 0;

		if(cmd_forward.state & 1 || cmd_back.state & 1)
		{
			if(m_acceleration < 0 && cmd_forward.state & 1 || m_acceleration > 0 && cmd_back.state & 1)
				m_acceleration += MOTORBIKE_ACCELERATION*frametime*4*m_direction;
			else
				m_acceleration += MOTORBIKE_ACCELERATION*frametime*m_direction;

			if(cmd_forward.state & 1)
			{
				if(m_acceleration > MOTORBIKE_MAX_SPEED)
					m_acceleration = MOTORBIKE_MAX_SPEED;
			}
			else
			{
				if(m_acceleration < -MOTORBIKE_MAX_SPEED*0.5)
					m_acceleration = -MOTORBIKE_MAX_SPEED*0.5;
			}
		}
		else if(m_acceleration)
		{
			if(m_acceleration < 0)
			{
				m_acceleration += MOTORBIKE_DECELERATION*frametime;
				if(m_acceleration > 0) m_acceleration = 0;
			}
			else
			{
				m_acceleration -= MOTORBIKE_DECELERATION*frametime;
				if(m_acceleration < 0) m_acceleration = 0;
			}
		}
	}

	if(m_pPlayer->curstate.groundent != NO_ENTITY_INDEX)
		Math::VectorCopy(m_clientBikeEntity.curstate.origin, m_clientBikeEntity.prevstate.origin);

	// Set origin with the offset
	Math::VectorAdd(m_pPlayer->curstate.origin, Vector(0, 0, VEC_HULL_MIN[2]), m_clientBikeEntity.curstate.origin);
	
	// Update angles
	UpdateAngles();

	if(m_pPlayer->curstate.groundent != NO_ENTITY_INDEX)
	{
		Float zdiff = m_clientBikeEntity.curstate.origin.z-m_clientBikeEntity.prevstate.origin.z;

		trace_t tr1, tr2;
		cl_tracefuncs.pfnPlayerTrace(m_clientBikeEntity.prevstate.origin+Vector(0, 0, 4), m_clientBikeEntity.prevstate.origin - Vector(0, 0, 32), FL_TRACE_NO_MODELS, HULL_POINT, -1, tr1);
		cl_tracefuncs.pfnPlayerTrace(m_clientBikeEntity.curstate.origin+Vector(0, 0, 4), m_clientBikeEntity.curstate.origin - Vector(0, 0, 32), FL_TRACE_NO_MODELS, HULL_POINT, -1, tr2);

		if(Math::VectorCompare(tr1.plane.normal, tr2.plane.normal) 
			&& tr2.plane.normal[2] >= 0.99 && tr1.plane.normal[2] >= 0.99
			&& abs(zdiff) > 4 || m_prevPlayerGroundEntity == NO_ENTITY_INDEX && abs(zdiff) > 4)
		{
			Math::VectorCopy(m_clientBikeEntity.curstate.origin, m_clientBikeEntity.prevstate.origin);
			m_acceleration *= 0.8; // take down a bit

			CString soundfile;
			switch(Common::RandomLong(0, 1))
			{
			case 0: soundfile = "bike/bike_bump1.wav"; break;
			case 1: soundfile = "bike/bike_bump2.wav"; break;
			}

			cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_ITEM, soundfile.c_str(), MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);

			if(zdiff < 0)
				m_punchAmount[0] += 90;
			else
				m_punchAmount[0] -= 90;
		}
	}

	// Update this
	m_prevPlayerGroundEntity = m_pPlayer->curstate.groundent;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::UpdateSoundStates( void )
{
	// manage sound states
	if(m_activeServerStatus == BIKE_SV_ENTERING)
	{
		if(m_soundState != BIKE_SOUND_START)
		{
			cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "bike/bike_start.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
			m_soundTime = m_time + 8;
			m_soundState = BIKE_SOUND_START;
			m_playBrakeSound = false;
		}
	}
	else if(m_activeServerStatus == BIKE_SV_LEAVING)
	{
		if(m_soundState != BIKE_SOUND_OFF)
		{
			cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "bike/bike_exit.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
			m_soundState = BIKE_SOUND_OFF;
			m_playBrakeSound = false;
		}
	}
	else if(m_activeServerStatus == BIKE_SV_ACTIVE)
	{
		if(m_acceleration && ((cmd_forward.state & 1) || (cmd_back.state& 1)) && !(cmd_jump.state & 1) )
		{
			if(m_soundState != BIKE_SOUND_RUN && m_soundState != BIKE_SOUND_ACCEL_BEGIN || cmd_back.state != m_prevBackState || cmd_forward.state != m_prevForwardState )
			{
				cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "bike/bike_accelerate_begin.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
				m_soundState = BIKE_SOUND_ACCEL_BEGIN;
				m_soundTime = m_time + 4.6;
			}
			else if(m_soundTime < m_time)
			{
				if(m_soundState != BIKE_SOUND_RUN)
				{
					cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "bike/bike_accelerate.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
					m_soundState = BIKE_SOUND_RUN;
				}
			}

			// reset this
			m_playBrakeSound = false;
		}
		else if(m_acceleration)
		{
			if(!m_playBrakeSound && cmd_jump.state & 1)
			{
				cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_STATIC, "bike/bike_skid.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
				m_playBrakeSound = true;
			}

			if(m_soundState != BIKE_SOUND_DECELERATE)
			{
				cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "bike/bike_decelerate.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
				m_soundState = BIKE_SOUND_DECELERATE;
				m_soundTime = m_time + 7.5;
			}
		}
		else if(m_soundTime <= m_time)
		{
			if(m_soundState != BIKE_SOUND_IDLE)
			{
				cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "bike/bike_idle_loop.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
				m_soundState = BIKE_SOUND_IDLE;
				m_playBrakeSound = false;
			}
		}

		m_prevDirection = m_direction;
	}
	else if(m_soundState != BIKE_SOUND_NONE)
	{
		cl_engfuncs.pfnPlayEntitySound(m_pPlayer->entindex, SND_CHAN_BODY, "common/null.wav", MOTORBIKE_VOLUME, ATTN_NORM, PITCH_NORM, SND_FL_MOTORBIKE, 0);
		m_soundState = BIKE_SOUND_NONE;
	}

	// Save button states for sound changes
	m_prevForwardState = cmd_forward.state;
	m_prevBackState = cmd_back.state;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::HandleStates( void )
{
	if(m_nextUpdateTime == -1)
		return;

	if(m_activeServerStatus == m_serverStatus && m_activeServerStatus != BIKE_SV_ACTIVE)
		return;

	if(m_nextUpdateTime > m_time
		&& m_activeServerStatus != BIKE_SV_INACTIVE)
		return;

	m_nextUpdateTime = -1;
	m_activeServerStatus = m_serverStatus;

	if(m_activeServerStatus == BIKE_SV_ENTERING_LERP)
	{
		// Copy origin and angles from the entity
		Math::VectorCopy(m_pBikeEntity->curstate.origin, m_clientBikeEntity.curstate.origin);
		Math::VectorCopy(m_pBikeEntity->curstate.angles, m_clientBikeEntity.curstate.angles);
		
		m_clientBikeEntity.pmodel = m_pBikeModel;
		m_clientBikeEntity.curstate.modelindex = m_pBikeModel->cacheindex;

		m_lastMouseMove = 0;
		m_deviationAngles.Clear();
		m_addDeviationAngles.Clear();

		m_idealAnimState = BIKE_STATE_ENTER_LERP;
		SetSequence(BIKE_ANIM_STANDBY);
		m_isViewSaved = false;
		m_nextSequenceTime = m_time;
	}
	else if(m_activeServerStatus == BIKE_SV_ENTERING)
	{
		Math::VectorAdd(m_pPlayer->curstate.origin, Vector(0, 0, VEC_HULL_MIN[2]), m_clientBikeEntity.curstate.origin);
		Math::VectorCopy(m_clientBikeEntity.curstate.origin, m_clientBikeEntity.prevstate.origin);
		m_clientBikeEntity.curstate.body = 1;

		m_nextSequenceTime = m_time + SequenceTime(BIKE_ANIM_ENTER);
		m_idealAnimState = BIKE_STATE_READY;

		m_animState = BIKE_STATE_ENTER;
		SetSequence(BIKE_ANIM_ENTER);
	}
	else if(m_activeServerStatus == BIKE_SV_ACTIVE)
	{
		ActiveThink();
		m_nextUpdateTime = m_time;
	}
	else if(m_activeServerStatus == BIKE_SV_LEAVING)
	{
		m_nextSequenceTime = m_time + SequenceTime(BIKE_ANIM_EXIT);
		m_lastMouseMove = m_time-MOUSEMOVE_TIMEOUT;
		Math::VectorCopy(m_addDeviationAngles, m_deviationAngles);

		m_idealAnimState = BIKE_STATE_LEAVE_LERP;
		m_animState = BIKE_STATE_LEAVE;
		SetSequence(BIKE_ANIM_EXIT);
	}
	else if(m_activeServerStatus == BIKE_SV_LEAVING_LERP)
	{
		m_idealAnimState = m_animState = BIKE_STATE_LEAVE_LERP;
		SetSequence(BIKE_ANIM_STANDBY);
		m_clientBikeEntity.curstate.body = 0;
		m_isViewSaved = false;
	}
	else if(m_activeServerStatus == BIKE_SV_CLEANUP)
	{
		gDefaultView.ResetViewIdle();
		gDefaultView.ResetViewRoll();

		SetSequence(BIKE_ANIM_STANDBY);
		m_idealAnimState = m_animState = BIKE_STATE_STANDBY;
		m_serverStatus = m_activeServerStatus = BIKE_SV_INACTIVE;

		m_clientBikeEntity.pmodel = nullptr;
		m_clientBikeEntity.curstate.modelindex = 0;
	}
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::UpdateAngles( void )
{
	// Get ideal angles
	Double blendtime = 0;
	Vector idealUpVector;
	GetIdealUpVector(idealUpVector, &blendtime);

	// Check if the angle differs
	if(!m_angleBlendTime && !Math::VectorCompare(m_currentBikeUpVector, idealUpVector))
	{
		m_angleBlendTime = m_time;
		m_angleBlendDuration = blendtime;

		m_idealBikeUpVector = idealUpVector;
		m_prevBikeUpVector = m_currentBikeUpVector;
	}

	// Update blend angle
	if(m_angleBlendTime)
	{
		if(m_angleBlendTime + m_angleBlendDuration <= m_time)
		{
			m_currentBikeUpVector = m_idealBikeUpVector;
			m_angleBlendTime = 0;
		}
		else
		{
			Double flfrac = (m_time-m_angleBlendTime)/m_angleBlendDuration;
			Math::VectorScale(m_prevBikeUpVector, (1.0 - flfrac), m_currentBikeUpVector);
			Math::VectorMA(m_currentBikeUpVector, flfrac, m_idealBikeUpVector, m_currentBikeUpVector);
		}
	}

	// Check if the angle differs
	if(m_angleBlendTime && !Math::VectorCompare(m_idealBikeUpVector, idealUpVector))
	{
		Float deltaleft = (m_angleBlendTime+m_angleBlendDuration)-m_time;
		m_angleBlendDuration = deltaleft;
		m_angleBlendTime = m_time;

		m_idealBikeUpVector = idealUpVector;
		m_prevBikeUpVector = m_currentBikeUpVector;
	}

	// Get the surface-aligned angles
	m_clientBikeEntity.curstate.angles = Math::AdjustAnglesToNormal(m_currentBikeUpVector, m_pPlayer->curstate.angles);
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::GetIdealUpVector( Vector& outup, Double* pblendtime )
{
	// If not onground, use 
	if(m_pPlayer->curstate.groundent == NO_ENTITY_INDEX)
	{
		outup = Vector(0, 0, 1);
		if(pblendtime)(*pblendtime) = ANGLE_ALIGN_TIME_IN_AIR;
		return;
	}

	// Get ground right below
	trace_t tr1;
	Vector vecEnd = m_clientBikeEntity.curstate.origin - Vector(0, 0, 128);
	cl_tracefuncs.pfnPlayerTrace(m_clientBikeEntity.curstate.origin, vecEnd, FL_TRACE_NO_MODELS, HULL_POINT, m_pPlayer->entindex, tr1);
	if(tr1.noHit() || tr1.startSolid() || tr1.allSolid())
	{
		outup = Vector(0, 0, 1);
		if(pblendtime)(*pblendtime) = ANGLE_ALIGN_TIME_ON_GROUND;
		return;
	}

	Vector playerangles = m_clientBikeEntity.curstate.angles;
	playerangles[PITCH] = playerangles[ROLL] = 0;

	Vector forward;
	Math::AngleVectors(playerangles, &forward);
	Vector refNormal = Vector(0, 0, 1);

#if 0
	// Original method: average normals between wheels
	Vector blendNormal = tr.plane.normal;
	for(Int32 i = -MOTORBIKE_BACK_WHEEL_DISTANCE; i <= MOTORBIKE_FRONT_WHEEL_DISTANCE; i += 4)
	{
		if(i == 0)
			continue;

		// Start begin and up a bit
		Vector traceBeginPosition = m_clientBikeEntity.curstate.origin + refNormal*32 + forward*i;
		Vector traceEndPosition = traceBeginPosition - refNormal*512;

		cl_tracefuncs.pfnPlayerTrace(traceBeginPosition, traceEndPosition, FL_TRACE_NO_MODELS, HULL_POINT, m_pPlayer->entindex, tr);
		if(tr.noHit() || tr.startSolid() || tr.allSolid())
			continue;

		Math::VectorAdd(blendNormal, tr.plane.normal, blendNormal);
	}

	blendNormal.Normalize();

#else
	// Current implementation: Use wheel positions and vector to front from back
	Vector backTracePos = m_clientBikeEntity.curstate.origin - forward*MOTORBIKE_BACK_WHEEL_DISTANCE + refNormal * 18;
	Vector backTraceEndPos = backTracePos - refNormal * 128;

	trace_t tr2;
	cl_tracefuncs.pfnPlayerTrace(backTracePos, backTraceEndPos, FL_TRACE_NO_MODELS, HULL_POINT, m_pPlayer->entindex, tr2);
	if(tr2.noHit() || tr2.startSolid() || tr2.allSolid())
	{
		outup = tr1.plane.normal;
		if(pblendtime)(*pblendtime) = ANGLE_ALIGN_TIME_ON_GROUND;
		return;
	}

	Vector frontTracePos = m_clientBikeEntity.curstate.origin + forward*MOTORBIKE_FRONT_WHEEL_DISTANCE + refNormal * 18;
	Vector frontTraceEndPos = frontTracePos - refNormal * 128;
	
	trace_t tr3;
	cl_tracefuncs.pfnPlayerTrace(frontTracePos, frontTraceEndPos, FL_TRACE_NO_MODELS, HULL_POINT, m_pPlayer->entindex, tr3);
	if(tr3.noHit() || tr3.startSolid() || tr3.allSolid())
	{
		outup = tr1.plane.normal;
		if(pblendtime)(*pblendtime) = ANGLE_ALIGN_TIME_ON_GROUND;
		return;
	}

	// Get vector from front to back
	forward = (tr3.endpos - tr2.endpos).Normalize();
	Vector angles = Math::VectorToAngles(forward);
	angles[PITCH] = -angles[PITCH];

	Vector blendNormal;
	Math::AngleVectors(angles, nullptr, nullptr, &blendNormal);
#endif
	
	outup = blendNormal;
	if(pblendtime)(*pblendtime) = ANGLE_ALIGN_TIME_ON_GROUND;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::Think( void )
{
	// Has to be iserverstatus
	if(m_activeServerStatus == BIKE_SV_INACTIVE && m_serverStatus == BIKE_SV_INACTIVE)
		return;
	
	m_time = cl_engfuncs.pfnGetClientTime();
	m_pPlayer = cl_engfuncs.pfnGetLocalPlayer();

	if(m_serverStatus == BIKE_SV_RESTORE)
	{
		// if we got save restored, do it here
		RestoreBike();
	}

	// Handle input first
	HandleInput();

	// main state handle
	HandleStates();

	// update sequences after input
	UpdateSequenceStates();

	// update sounds
	UpdateSoundStates();

	// drop punch angles
	DropPunchAngle();

	// Calc turn angles
	UpdateTurnAngles();

	// Set dynamic light
	SetupFlashlight();
};

//=============================================
// @brief
//
//=============================================
void CMotorBike::UpdateTurnAngles ( void )
{
	if(!m_lastMouseMove)
		return;

	// degrate the view add value
	if((m_lastMouseMove+MOUSEMOVE_TIMEOUT) <= m_time)
	{
		Double time = clamp((m_time-(m_lastMouseMove+MOUSEMOVE_TIMEOUT)), 0, MOUSEMOVE_TIMEOUT_BLEND);
		Double flfrac = Common::SplineFraction( time, (1.0/MOUSEMOVE_TIMEOUT_BLEND) );

		if(flfrac >= 1.0)
		{
			Math::VectorClear(m_deviationAngles);
			m_lastMouseMove = 0;
		}

		Math::VectorScale(m_deviationAngles, (1.0-flfrac), m_addDeviationAngles);
	}
	else
	{
		Math::VectorCopy(m_deviationAngles, m_addDeviationAngles);
	}
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::SetupFlashlight ( void )
{
	// Set up dynamic light
	if(m_activeServerStatus != BIKE_SV_ENTERING && m_activeServerStatus != BIKE_SV_ACTIVE)
		return;

	if(!(m_pPlayer->curstate.effects & EF_DIMLIGHT))
		return;

	Vector forward, right, up;
	cl_engfuncs.pfnUpdateAttachments(&m_clientBikeEntity);

	Math::VectorSubtract(m_clientBikeEntity.getAttachment(3), m_clientBikeEntity.getAttachment(2), forward);
	Math::VectorNormalize(forward); 
	Math::GetUpRight(forward, up, right);
	Math::VectorScale(right, -1, right);

	Vector angles;
	angles = Math::VectorToAngles(forward, right);

	cl_dlight_t* pdlight = cl_efxapi.pfnAllocDynamicSpotLight(BIKE_ENTITY_INDEX, 0, false, false, nullptr);
	Math::VectorCopy(m_clientBikeEntity.getAttachment(2), pdlight->origin);
	pdlight->color = Vector(1.0, 1.0, 1.0);
	pdlight->die = m_time+0.001;
	pdlight->textureindex = 0;
	pdlight->angles = angles;
	pdlight->cone_size = 90;
	pdlight->radius = 800;
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::CalcRefDef( ref_params_t& params )
{
	// Get vectors from entity
	Vector bikeViewOrigin, bikeViewAngles;
	GetViewInfo(bikeViewOrigin, bikeViewAngles);

	if(m_animState == BIKE_STATE_ENTER_LERP || m_animState == BIKE_STATE_LEAVE_LERP)
	{
		if(!m_isViewSaved)
		{
			if(m_animState == BIKE_STATE_ENTER_LERP)
			{
				// Set start point
				Math::VectorCopy(params.v_angles, m_lerpBeginAngles);
				Math::VectorCopy(params.v_origin, m_lerpBeginOrigin);
				Common::NormalizeAngles(m_lerpBeginAngles);

				// Set end point
				m_lerpEndOrigin = bikeViewOrigin;
				m_lerpEndAngles = bikeViewAngles;

				m_lerpBeginTime = params.time;
				m_lerpEndTime = params.time+BIKE_ENTER_TIME;
				m_isViewSaved = true;
			}
			else
			{
				// Set start point
				m_lerpBeginOrigin = bikeViewOrigin;
				m_lerpBeginAngles = bikeViewAngles;

				// Set end point
				Math::VectorCopy(m_exitAngles, m_lerpEndAngles);
				Math::VectorCopy(m_exitOrigin, m_lerpEndOrigin);
				Common::NormalizeAngles(m_lerpEndAngles);

				m_lerpBeginTime = params.time;
				m_lerpEndTime = params.time+BIKE_LEAVE_TIME;
				m_isViewSaved = true;
			}
		}

		Double lerptime = (m_animState == BIKE_STATE_ENTER_LERP) ? BIKE_ENTER_TIME : BIKE_LEAVE_TIME;
		Double time = clamp((params.time - m_lerpBeginTime), 0, lerptime);
		Double flfrac = Common::SplineFraction( time, (1.0/lerptime) );

		Vector vdiff, vangles;
		Math::VectorSubtract(m_lerpEndOrigin, m_lerpBeginOrigin, vdiff);
		Math::VectorMA(m_lerpBeginOrigin, flfrac, vdiff, m_viewOrigin);

		for(Int32 i = 0; i < 3; i++)
		{
			Float diff = Math::AngleDiff(m_lerpEndAngles[i], m_lerpBeginAngles[i]);
			m_viewAngles[i] = m_lerpBeginAngles[i]+diff*(flfrac);
		}
	}
	else
	{
		m_viewOrigin = bikeViewOrigin;
		m_viewAngles = bikeViewAngles;
	}

	// sometimes during entering/leaving, the bikes might try to coexist for a fraction
	// do this here, or the bike will disappear for a single frame when entering/leaving
	if(m_pBikeEntity->curstate.msg_num == m_pPlayer->curstate.msg_num)
		m_pBikeEntity->curstate.effects |= EF_NODRAW;

	// Add the added angles
	Math::VectorAdd(m_viewAngles, m_addDeviationAngles, m_viewAngles);
	Common::NormalizeAngles(m_viewAngles);
	
	// Add in view punches
	Math::VectorAdd(m_viewAngles, m_punchAngle, m_viewAngles);

	Math::VectorCopy(m_viewAngles, params.v_angles);
	Math::VectorCopy(m_viewOrigin, params.v_origin);

	gShake.CalcShake();
	gShake.ApplyShake( params.v_origin, params.v_angles, 1.0 );

	// Add in a bit of roll from turning
	if(m_pCvarTurnRoll->GetValue() >= 1)
	{
		Float roll = clamp(m_pPlayer->curstate.velocity.Length()/MOTORBIKE_MAX_SPEED, 0.2, 1.0);
		params.v_angles[2] -= m_wheelAngle*MOTORBIKE_TURN_ANGLE*roll;
	}

	// Bit of sine wave to the view origin
	params.v_origin[2] -= 4*SDL_fabs(SDL_sin(params.time*1.7))*(m_pPlayer->curstate.velocity.Length()/MOTORBIKE_MAX_SPEED);

	// Set up viewmodel too
	cl_entity_t *pviewmodel = gViewModel.GetViewModel();

	m_viewOrigin.z = m_viewOrigin.z-1;
	Math::VectorCopy(m_viewOrigin, pviewmodel->curstate.origin);
	Math::VectorCopy(m_viewAngles, pviewmodel->curstate.angles);
	gDefaultView.CalcViewModelAngle(pviewmodel, params);

	// Calculate FOV using default view
	gDefaultView.CalculateFOV(params);
};

//=============================================
// @brief
//
//=============================================
void CMotorBike::DropPunchAngle ( void )
{
	if( m_punchAngle.Length() <= 0.001 && m_punchAmount.Length() < 0.001)
		return;

	Double frametime = cl_engfuncs.pfnGetFrameTime();
	Math::VectorMA(m_punchAngle, frametime, m_punchAmount, m_punchAngle);
	Double damping = 1.0 - (VIEW_PUNCH_DAMPING * frametime);
	if(damping < 0)
		damping = 0;

	Math::VectorScale(m_punchAmount, damping, m_punchAmount);

	// Toroidal spring
	Float springmagnitude = VIEW_PUNCH_SPRING_CONSTANT * frametime;
	springmagnitude = clamp(springmagnitude, 0, 2);

	Math::VectorMA(m_punchAmount, -springmagnitude, m_punchAngle, m_punchAmount);

	m_punchAngle[0] = clamp(m_punchAngle[0], -89, 89);
	m_punchAngle[1] = clamp(m_punchAngle[1], -179, 179);
	m_punchAngle[2] = clamp(m_punchAngle[2], -89, 89);
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::ProcessMessage( const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	m_serverStatus = (bike_sv_states_t)reader.ReadByte();
	if(m_serverStatus == BIKE_SV_ENTERING_LERP || m_serverStatus == BIKE_SV_RESTORE)
	{
		m_pPlayer = cl_engfuncs.pfnGetLocalPlayer();

		Vector idealUpVector;
		GetIdealUpVector(idealUpVector, nullptr);

		m_prevPlayerGroundEntity = m_pPlayer->curstate.groundent;
		m_currentBikeUpVector = idealUpVector;
		m_idealBikeUpVector = idealUpVector;
		m_angleBlendTime = 0;

		entindex_t entindex = reader.ReadInt16();
		m_pBikeEntity = cl_engfuncs.pfnGetEntityByIndex(entindex);
		if(!m_pBikeEntity)
		{
			cl_engfuncs.pfnCon_Printf("%s - Unable to get bike entity.\n", __FUNCTION__);
			return;
		}

		// Ensure sequences line up
		if(m_serverStatus == BIKE_SV_ENTERING_LERP)
			m_clientBikeEntity.curstate.sequence = m_pBikeEntity->curstate.sequence;
	}

	if(m_serverStatus == BIKE_SV_LEAVING_LERP)
	{
		for(Int32 i = 0; i < 3; i++)
			m_exitOrigin[i] = reader.ReadFloat();

		for(Int32 i = 0; i < 3; i++)
			m_exitAngles[i] = reader.ReadFloat();
	}
	else if(m_serverStatus == BIKE_SV_RESTORE)
	{
		m_acceleration = reader.ReadFloat();
	}
	else if(m_serverStatus == BIKE_SV_CLEANUP)
	{
		m_pBikeEntity = nullptr;
	}

	// update immediately
	m_nextUpdateTime = cl_engfuncs.pfnGetClientTime();
}

//=============================================
// @brief
//
//=============================================
void CMotorBike::GetViewInfo( Vector& origin, Vector& angles )
{
	// Set start point
	Vector forward, right, up;
	cl_engfuncs.pfnUpdateAttachments(&m_clientBikeEntity);
	Math::VectorCopy(m_clientBikeEntity.getAttachment(0), origin);

	Math::VectorSubtract(m_clientBikeEntity.getAttachment(1), m_clientBikeEntity.getAttachment(0), forward);
	Math::VectorNormalize(forward); 

	Math::GetUpRight(forward, up, right);
	Math::VectorScale(right, -1, right);

	angles = Math::VectorToAngles(forward, right);
	Common::NormalizeAngles(angles);
}

//=============================================
// @brief
//
//=============================================
bool CMotorBike::Draw ( void )
{
	if(m_activeServerStatus == BIKE_SV_CLEANUP)
		return true;

	if(m_activeServerStatus == BIKE_SV_INACTIVE)
		return true;

	// Sync any decals
	entity_extrainfo_t* pextrainfo = cl_engfuncs.pfnGetEntityExtraData(m_pBikeEntity);
	entity_extrainfo_t* pclextrainfo = cl_engfuncs.pfnGetEntityExtraData(&m_clientBikeEntity);
	if(pextrainfo && pclextrainfo && pextrainfo->pvbmdecalheader)
		pclextrainfo->pvbmdecalheader = pextrainfo->pvbmdecalheader;

	// Depthrange hack
	if(!cl_renderfuncs.pfnVBMPrepareDraw())
	{
		cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
		return false;
	}

	if(!cl_renderfuncs.pfnDrawVBMModel(&m_clientBikeEntity, (VBM_RENDER|VBM_ANIMEVENTS)))
	{
		cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
		cl_renderfuncs.pfnVBMEndDraw();
		return false;
	}

	cl_renderfuncs.pfnVBMEndDraw();

	return true;
}