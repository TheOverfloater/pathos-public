/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cache_model.h"
#include "clientdll.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "ladder.h"
#include "com_math.h"
#include "view.h"
#include "shake.h"
#include "viewmodel.h"
#include "vbm_shared.h"
#include "studio.h"
#include "snd_shared.h"
#include "msgreader.h"
#include "entity_extrainfo.h"

// Class definition
CLadder gLadder;

// Max angle deviation
const Float CLadder::LADDER_V_MAX_ADD = 30;
// Max add on X
const Float CLadder::LADDER_MAX_ADD_X = 20;
// Max add on Y
const Float CLadder::LADDER_MAX_ADD_Y = 30;
// Min add on Y
const Float CLadder::LADDER_MIN_ADD_Y = 10;
// Mouse move timeout
const Double CLadder::LADDER_MOUSEMOVE_TIMEOUT = 5;
// Mouse move timeout blend time
const Double CLadder::LADDER_MOUSEMOVE_TIMEOUT_BLEND = 1;
// Model name for view entity
const Char CLadder::LADDER_VIEWMODEL_NAME[] = "models/v_sequences.mdl";
// Array of sequence names to look up
const Char* CLadder::LADDER_SEQUENCE_NAMES[] = {
	"idle",
	"ladder_down_lup",
	"ladder_down_rup",
	"ladder_up_lup",
	"ladder_up_rup",
	"ladder_idle_lup",
	"ladder_idle_rup",
	"ladder_enter_bottom",
	"ladder_enter_left",
	"ladder_enter_right",
	"ladder_enter_top",
	"ladder_leave_bottom_lup",
	"ladder_leave_bottom_rup",
	"ladder_leave_left_lup",
	"ladder_leave_left_rup",
	"ladder_leave_right_lup",
	"ladder_leave_right_rup",
	"ladder_leave_top_lup",
	"ladder_leave_top_rup"
};

//=============================================
// @brief
//
//=============================================
CLadder::CLadder( void ):
	m_pLadderEntity(nullptr),
	m_pViewModel(nullptr),
	m_time(0),
	m_activeLadderState(LADDER_STATE_INACTIVE),
	m_idealLadderState(LADDER_STATE_INACTIVE),
	m_updateTime(-1),
	m_activeAnimState(LADDER_ANIM_NONE),
	m_idealAnimState(LADDER_ANIM_NONE),
	m_activeMovementState(LADDER_RESET),
	m_idealMovementState(LADDER_RESET),
	m_exitPoint(LADDER_EXIT_UNAVAILABLE),
	m_entryPoint(LADDER_ENTRYPOINT_UNAVAILABLE),
	m_side(SIDE_UNFEDINED),
	m_viewBlendBeginTime(0),
	m_viewBlendDelta(0),
	m_viewBlend(false),
	m_viewEntBlendBeginTime(0),
	m_viewEntBlendDelta(0),
	m_exitBlend(false),
	m_viewEntBlend(false),
	m_lastMouseMove(-1),
	m_pCvarDrawEntities(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CLadder::~CLadder( void ) 
{
}

//=============================================
// @brief
//
//=============================================
bool CLadder::Init( void ) 
{
	m_pCvarDrawEntities = cl_engfuncs.pfnGetCVarPointer("r_drawentities");
	if(!m_pCvarDrawEntities)
	{
		cl_engfuncs.pfnErrorPopup("%s - Failed to get cvar 'r_drawentities'.\n", __FUNCTION__);
		return false;
	}

	return true;
}


//=============================================
// @brief
//
//=============================================
void CLadder::Shutdown( void ) 
{
}

//=============================================
// @brief
//
//=============================================
bool CLadder::InitGame( void ) 
{
	// Set view entity flags
	m_viewEntity.curstate.entindex = m_viewEntity.entindex = LADDER_ENTITY_INDEX;
	m_viewEntity.curstate.effects |= (EF_NOLERP|EF_CLIENTENT);

	m_pViewModel = cl_engfuncs.pfnLoadModel(LADDER_VIEWMODEL_NAME);
	if(!m_pViewModel)
	{
		cl_engfuncs.pfnCon_Printf("Failed to load '%s'.\n", LADDER_VIEWMODEL_NAME);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CLadder::ClearGame( void ) 
{
	m_viewBlend = false;
	m_pLadderEntity = nullptr;

	m_activeLadderState = LADDER_STATE_INACTIVE;
	m_idealLadderState = LADDER_STATE_INACTIVE;
	m_activeAnimState = LADDER_ANIM_NONE;

	// Clear view entity
	m_viewEntity = cl_entity_t();
}

//=============================================
// @brief
//
//=============================================
void CLadder::Think( void ) 
{
	if(m_activeLadderState == LADDER_STATE_INACTIVE
		&& m_idealLadderState == LADDER_STATE_INACTIVE)
		return;

	m_time = cl_engfuncs.pfnGetClientTime();
	cl_entity_t *pplayer = cl_engfuncs.pfnGetLocalPlayer();

	if(m_idealLadderState != m_activeLadderState)
	{
		if(m_updateTime != -1 && m_updateTime <= m_time
			|| m_activeLadderState == LADDER_STATE_INACTIVE)
		{
			m_activeLadderState = m_idealLadderState;

			if(m_activeLadderState == LADDER_STATE_ENTERING)
			{
				// Flag for blend, calcrefdef will set timers
				m_viewBlendDelta = LADDER_ENTER_LERP_TIME;
				m_viewEntBlend = false;
				m_viewBlend = true;

				m_lastMouseMove = -1;
				m_deviationAngles.Clear();
				m_addDeviationAngles.Clear();

				// Clear anim state
				m_activeAnimState = LADDER_ANIM_NONE;

				// Set sequence state and update time
				if(m_entryPoint == LADDER_ENTER_BOTTOM)
				{
					m_updateTime = m_time + LADDER_ENTER_BOTTOM_TIME;
					m_idealAnimState = LADDER_ANIM_ENTER_BOTTOM;
				}
				else if(m_entryPoint == LADDER_ENTER_LEFT)
				{
					m_updateTime = m_time + LADDER_ENTER_LEFT_TIME;
					m_idealAnimState = LADDER_ANIM_ENTER_LEFT;
				}
				else if(m_entryPoint == LADDER_ENTER_RIGHT)
				{
					m_updateTime = m_time + LADDER_ENTER_RIGHT_TIME;
					m_idealAnimState = LADDER_ANIM_ENTER_RIGHT;
				}
				else if(m_entryPoint == LADDER_ENTER_TOP)
				{
					m_updateTime = m_time + LADDER_ENTER_TOP_TIME;
					m_idealAnimState = LADDER_ANIM_ENTER_TOP;
				}

				// Set movement state to default
				m_activeMovementState = LADDER_RESET;
				m_idealMovementState = LADDER_RESTING;
				m_side = SIDE_LEFT;

				// Set up viewentity
				m_viewEntity.pmodel = m_pViewModel;
				m_viewEntity.curstate.modelindex = m_pViewModel->cacheindex;

				Math::VectorCopy(m_svOrigin, m_viewEntity.curstate.origin);
				Math::VectorCopy(m_svAngles, m_viewEntity.curstate.angles);
			}
			else if(m_activeLadderState == LADDER_STATE_ACTIVE)
			{
				m_viewEntity.pmodel = m_pViewModel;
				m_viewEntity.curstate.modelindex = m_pViewModel->cacheindex;
			}
			else if(m_activeLadderState == LADDER_STATE_LEAVING)
			{
				m_viewEntBlend = true;
				m_viewEntBlendBeginTime = 0;
				m_viewEntBlendDelta = 0;

				m_lastMouseMove = m_time-LADDER_MOUSEMOVE_TIMEOUT;
				Math::VectorCopy(m_addDeviationAngles, m_deviationAngles);

				Math::VectorCopy(pplayer->curstate.origin, m_viewEntity.curstate.origin);
				Math::VectorCopy(pplayer->curstate.angles, m_viewEntity.curstate.angles);

				m_viewBlend = true;
				m_viewBlendDelta = LADDER_LEAVE_VIEW_LERP_TIME;

				Math::VectorAdd(m_svOrigin, pplayer->curstate.view_offset, m_viewBlendOrigin);
				Math::VectorCopy(m_svAngles, m_viewBlendAngles);

				for(Uint32 i = 0; i < 3; i++)
				{
					if(SDL_fabs(m_viewBlendAngles[i]) == 360.0f)
						m_viewBlendAngles[i] = 0;
				}

				switch(m_exitPoint)
				{
					case LADDER_EXIT_TOP: 
						m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_LEAVE_TOP_RUP : LADDER_ANIM_LEAVE_TOP_LUP; 
						m_viewBlendBeginTime = m_time + LADDER_LEAVE_TOP_TIME - m_viewBlendDelta;
						break;
					case LADDER_EXIT_LEFT: 
						m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_LEAVE_LEFT_RUP : LADDER_ANIM_LEAVE_LEFT_LUP; 
						m_viewBlendBeginTime = m_time + LADDER_LEAVE_LEFT_TIME - m_viewBlendDelta;
						break;
					case LADDER_EXIT_RIGHT: 
						m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_LEAVE_RIGHT_RUP : LADDER_ANIM_LEAVE_RIGHT_LUP; 
						m_viewBlendBeginTime = m_time + LADDER_LEAVE_RIGHT_TIME - m_viewBlendDelta;
						break;
					case LADDER_EXIT_BOTTOM: 
						m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_LEAVE_BOTTOM_RUP : LADDER_ANIM_LEAVE_BOTTOM_LUP; 
						m_viewBlendBeginTime = m_time + LADDER_LEAVE_BOTTOM_TIME - m_viewBlendDelta;
						break;
				}
			}
			else if(m_activeLadderState == LADDER_STATE_CLEANUP)
			{
				gDefaultView.ResetViewIdle();
				gDefaultView.ResetViewRoll();

				m_viewEntity.pmodel = nullptr;
				m_viewEntity.curstate.modelindex = 0;

				m_idealMovementState = LADDER_RESET;
				m_activeLadderState = m_idealLadderState = LADDER_STATE_INACTIVE;
				m_activeAnimState = LADDER_ANIM_NONE;
				return;
			}
		}
	}

	if(m_activeLadderState == LADDER_STATE_ACTIVE)
	{
		if(m_activeMovementState != m_idealMovementState)
		{
			m_activeMovementState = m_idealMovementState;
			Math::VectorCopy(pplayer->curstate.origin, m_viewEntity.curstate.origin);
			Math::VectorCopy(pplayer->curstate.angles, m_viewEntity.curstate.angles);

			switch(m_activeMovementState)
			{
				case LADDER_RESTING: 
					m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_IDLE_RUP : LADDER_ANIM_IDLE_LUP; 
					break;
				case LADDER_MOVE_UP:
					m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_UP_RUP : LADDER_ANIM_UP_LUP; 
					m_side = (m_side == SIDE_RIGHT) ? SIDE_LEFT : SIDE_RIGHT;
					break;
				case LADDER_MOVE_DOWN:
					m_idealAnimState = (m_side == SIDE_RIGHT) ? LADDER_ANIM_DOWN_RUP : LADDER_ANIM_DOWN_LUP; 
					m_side = (m_side == SIDE_RIGHT) ? SIDE_LEFT : SIDE_RIGHT;
					break;
			}
		}
	}

	if(m_viewEntity.pmodel)
	{
		if(m_activeAnimState != m_idealAnimState)
		{
			m_activeAnimState = m_idealAnimState;
			if(m_activeAnimState != LADDER_ANIM_NONE)
				SetSequence(LADDER_SEQUENCE_NAMES[m_activeAnimState]);
		}

		if(m_activeLadderState == LADDER_STATE_LEAVING)
		{
			if(m_viewEntBlend && !m_viewEntBlendBeginTime)
			{
				// Wait one second before blending
				m_viewEntBlendBeginTime = m_time + (m_exitPoint == LADDER_EXIT_BOTTOM ? 0 : 0.5);
				m_viewEntBlendDelta = LADDER_LEAVE_LERP_TIME;

				Math::VectorCopy(pplayer->curstate.origin, m_viewEntBlendOrigin);
				m_viewEntBlendOrigin[2] += m_exitBlend;
			}
			else if(m_viewEntBlend && m_viewEntBlendBeginTime)
			{
				// Kill blending if you can
				if((m_viewEntBlendBeginTime+m_viewEntBlendDelta) < m_time)
				{
					m_viewEntBlendBeginTime = 0;
					m_viewEntBlendDelta = 0;
					m_viewEntBlend = 0;

					Math::VectorCopy(m_viewEntBlendOrigin, m_viewEntity.curstate.origin);
				}
			}

			if(m_viewEntBlend)
			{
				Double time = clamp((m_time - m_viewEntBlendBeginTime), 0,  m_viewEntBlendDelta);
				Float fldelta = Common::SplineFraction( time, (1.0/m_viewEntBlendDelta) );
				m_viewEntity.curstate.origin = (1.0-fldelta)*pplayer->curstate.origin+fldelta*m_viewEntBlendOrigin;
			}
		}
	}

	// degrade the view add value
	if(m_lastMouseMove != -1 && (m_lastMouseMove+LADDER_MOUSEMOVE_TIMEOUT) <= m_time)
	{
		Float time = clamp((m_time-(m_lastMouseMove+LADDER_MOUSEMOVE_TIMEOUT)), 0, LADDER_MOUSEMOVE_TIMEOUT_BLEND);
		Float flfrac = Common::SplineFraction( time, (1.0/LADDER_MOUSEMOVE_TIMEOUT_BLEND) );

		if(flfrac >= 1.0)
		{
			m_deviationAngles.Clear();
			m_lastMouseMove = -1;
		}

		Math::VectorScale(m_deviationAngles, (1.0-flfrac), m_addDeviationAngles);
	}
	else if(m_lastMouseMove != -1)
	{
		Math::VectorCopy(m_deviationAngles, m_addDeviationAngles);
	}
}

//=============================================
// @brief
//
//=============================================
void CLadder::CalcRefDef( ref_params_t& params ) 
{
	// Kill blending if you can
	if(m_viewBlend && m_viewBlendBeginTime)
	{
		if((m_viewBlendBeginTime+m_viewBlendDelta) <= params.time)
		{
			m_viewBlendBeginTime = 0;
			m_viewBlendDelta = 0;
			m_viewBlend = false;
		}
	}

	// Get it from clientside entity
	Vector v_origin, v_angles;
	GetViewInfo(v_origin, v_angles);

	// Apply blend if required
	if(m_viewBlend)
	{
		if(!m_viewBlendBeginTime)
		{
			m_viewBlendBeginTime = params.time;
			Math::VectorCopy(params.v_origin, m_viewBlendOrigin);
			Math::VectorCopy(params.v_angles, m_viewBlendAngles);

			for(Uint32 i = 0; i < 3; i++)
			{
				if(SDL_fabs(m_viewBlendAngles[i]) == 360.0f)
					m_viewBlendAngles[i] = 0;
			}
		}
		
		Float time = clamp((params.time - m_viewBlendBeginTime), 0, m_viewBlendDelta);
		Float fldelta = Common::SplineFraction( time, (1.0/m_viewBlendDelta) );
		fldelta = clamp(fldelta, 0.0, 1.0);

		if(m_activeLadderState == LADDER_STATE_ENTERING)
		{
			Vector tmp = v_origin;
			Math::VectorScale(tmp, fldelta, v_origin);
			Math::VectorMA(v_origin, (1.0-fldelta), m_viewBlendOrigin, v_origin);

			for(Uint32 i = 0; i < 3; i++)
			{
				Float diff = Math::AngleDiff(v_angles[i], m_viewBlendAngles[i]);
				v_angles[i] = m_viewBlendAngles[i]+diff*(fldelta);
			}
		}
		else if(m_activeLadderState == LADDER_STATE_LEAVING)
		{
			Vector tmp = v_origin;
			Math::VectorScale(m_viewBlendOrigin, fldelta, v_origin);
			Math::VectorMA(v_origin, (1.0-fldelta), tmp, v_origin);

			for(Uint32 i = 0; i < 3; i++)
			{
				Float diff = Math::AngleDiff(v_angles[i], m_viewBlendAngles[i]);
				v_angles[i] = diff*(1.0-fldelta)+m_viewBlendAngles[i];
			}
		}
	}

	// Add the added angles
	Math::VectorAdd(v_angles, m_addDeviationAngles, v_angles);
	Common::NormalizeAngles(v_angles);

	Math::VectorCopy(v_origin, params.v_origin);
	Math::VectorCopy(v_angles, params.v_angles);

	gShake.CalcShake();
	gShake.ApplyShake( params.v_origin, params.v_angles, 1.0 );

	// View model is always present
	cl_entity_t *pviewmodel = gViewModel.GetViewModel();
	if(pviewmodel)
	{
		Math::VectorCopy(params.v_angles, pviewmodel->curstate.angles);
		Math::VectorCopy(params.v_origin, pviewmodel->curstate.origin);
		pviewmodel->curstate.origin.z = pviewmodel->curstate.origin.z-1;

		gDefaultView.CalcViewModelAngle(pviewmodel, params);
	}

	// Calculate FOV using default view
	gDefaultView.CalculateFOV(params);
}

//=============================================
// @brief
//
//=============================================
bool CLadder::IsActive( void ) const
{
	if(m_activeLadderState != LADDER_STATE_INACTIVE)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CLadder::ProcessMessage( const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	entindex_t entindex = reader.ReadInt16();
	if(entindex)
	{
		m_pLadderEntity = cl_engfuncs.pfnGetEntityByIndex(entindex);
		if(!m_pLadderEntity)
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Ladder entity not found on client.\n", __FUNCTION__);
			return;
		}
	}
	else
	{
		// Clear entity
		m_pLadderEntity = nullptr;
	}

	m_idealLadderState = (ladder_state_t)reader.ReadInt16();
	m_updateTime = cl_engfuncs.pfnGetClientTime();

	if(m_idealLadderState == LADDER_STATE_ENTERING)
	{
		m_entryPoint = (ladder_entrypoints_t)reader.ReadByte();

		for(Int32 i = 0; i < 3; i++)
			m_svOrigin[i] = reader.ReadFloat();

		for(Int32 i = 0; i < 3; i++)
			m_svAngles[i] = reader.ReadFloat();
	}
	else if(m_idealLadderState == LADDER_STATE_ACTIVE)
	{
		m_idealMovementState = (ladder_move_state_t)reader.ReadByte();
		m_activeMovementState = LADDER_RESET;
		m_updateTime = cl_engfuncs.pfnGetClientTime();
	}
	else if(m_idealLadderState == LADDER_STATE_LEAVING)
	{
		m_exitPoint = (ladder_exitpoints_t)reader.ReadByte();
		m_exitBlend = reader.ReadFloat();

		for(Int32 i = 0; i < 3; i++)
			m_svOrigin[i] = reader.ReadFloat();

		for(Int32 i = 0; i < 3; i++)
			m_svAngles[i] = reader.ReadFloat();
	}
}

//=============================================
// @brief
//
//=============================================
void CLadder::SetSequence( const Char* psequence )
{
	if(!m_viewEntity.pmodel || !m_viewEntity.pmodel->pcachedata)
	{
		cl_engfuncs.pfnCon_Printf("%s - Model not set.\n", __FUNCTION__);
		return;
	}

	const vbmcache_t* pcache = m_viewEntity.pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	Int32 sequenceIndex = VBM_FindSequence( pstudiohdr, psequence );
	if(sequenceIndex == NO_SEQUENCE_VALUE)
	{
		cl_engfuncs.pfnCon_Printf("%s - No such sequence '%s'.\n", __FUNCTION__, psequence);
		return;
	}

	m_viewEntity.latched.prevseqblending[0] = m_viewEntity.curstate.blending[0];
	m_viewEntity.latched.prevseqblending[1] = m_viewEntity.curstate.blending[1];

	m_viewEntity.latched.sequence = m_viewEntity.curstate.sequence;
	m_viewEntity.latched.animtime = m_viewEntity.curstate.animtime;

	const mstudioseqdesc_t* pseqdesc = pstudiohdr->getSequence(sequenceIndex);
	if(pseqdesc)
		m_viewEntity.latched.frame = VBM_EstimateFrame(pseqdesc, m_viewEntity.curstate, m_time);

	m_viewEntity.latched.sequencetime = m_time;

	m_viewEntity.curstate.sequence = sequenceIndex;
	m_viewEntity.curstate.animtime = cl_engfuncs.pfnGetClientTime();
	m_viewEntity.curstate.frame = 0;
	m_viewEntity.curstate.framerate = 1.0;
	m_viewEntity.eventframe = 0;
}

//=============================================
// @brief
//
//=============================================
void CLadder::PlayStepSound( void )
{
	cl_entity_t *pplayer = cl_engfuncs.pfnGetLocalPlayer();

	CString soundfile;
	switch(Common::RandomLong(0, 3))
	{
	case 0: soundfile = "player/pl_ladder1.wav"; break;
	case 1: soundfile = "player/pl_ladder3.wav"; break;
	case 2: soundfile = "player/pl_ladder2.wav"; break;
	case 3: soundfile = "player/pl_ladder4.wav"; break;
	}

	cl_engfuncs.pfnPlayAmbientSound(pplayer->entindex, pplayer->curstate.origin, SND_CHAN_STATIC, soundfile.c_str(), VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, 0);
}

//=============================================
// @brief
//
//=============================================
void CLadder::MouseMove( Float mousex, Float mousey )
{
	if(m_activeLadderState == LADDER_STATE_LEAVING || m_activeLadderState == LADDER_STATE_ENTERING)
		return;

	if(!mousex && !mousey)
		return;

	if(m_lastMouseMove+LADDER_MOUSEMOVE_TIMEOUT < cl_engfuncs.pfnGetClientTime())
		Math::VectorCopy(m_addDeviationAngles, m_deviationAngles);

	m_deviationAngles.x += mousey; // wtf this is switched for some reason
	if(m_deviationAngles.x > LADDER_MAX_ADD_X) m_deviationAngles.x = LADDER_MAX_ADD_X;
	if(m_deviationAngles.x < -LADDER_MAX_ADD_X) m_deviationAngles.x = -LADDER_MAX_ADD_X;

	m_deviationAngles.y -= mousex;
	if(m_deviationAngles.y > LADDER_MAX_ADD_Y) m_deviationAngles.y = LADDER_MAX_ADD_Y;
	if(m_deviationAngles.y < -LADDER_MIN_ADD_Y) m_deviationAngles.y = -LADDER_MIN_ADD_Y;

	m_lastMouseMove = cl_engfuncs.pfnGetClientTime();
}

//=============================================
// @brief
//
//=============================================
void CLadder::GetViewInfo( Vector& origin, Vector& angles )
{
	Vector left, forward;
	cl_engfuncs.pfnUpdateAttachments(&m_viewEntity);

	// Get forward
	Math::VectorSubtract(m_viewEntity.getAttachment(1), m_viewEntity.getAttachment(0), forward);
	Math::VectorNormalize(forward);

	// Get left
	Math::VectorSubtract(m_viewEntity.getAttachment(2), m_viewEntity.getAttachment(0), left);
	Math::VectorNormalize(left);

	Math::VectorCopy(m_viewEntity.getAttachment(0), origin);
	Math::VectorCopy(Math::VectorToAngles(forward, left), angles);
	Common::NormalizeAngles(angles);
}

//=============================================
// @brief
//
//=============================================
bool CLadder::DrawLadder( cl_entity_t *pLadder )
{
	// Remember body
	Uint64 savedBody = pLadder->curstate.body;

	// Remember origin
	Vector savedOrigin;
	Math::VectorCopy(pLadder->curstate.origin, savedOrigin);

	// Draw the first segment
	if(!cl_renderfuncs.pfnVBMPrepareDraw())
		return false;

	// Get the extrainfo and flag it
	entity_extrainfo_t* pextrainfo = cl_engfuncs.pfnGetEntityExtraData(pLadder);
	pextrainfo->plightinfo->flags |= MDL_LIGHT_NOBLEND;

	if(!cl_renderfuncs.pfnDrawVBMModel(pLadder, VBM_RENDER))
	{
		cl_renderfuncs.pfnVBMEndDraw();
		return false;
	}

	pLadder->curstate.body = 0; // Remove top part

	// Draw segments
	Uint32 numsegments = pLadder->curstate.numsegments;
	for(Uint32 i = 0; i < numsegments; i++)
	{
		// Subtract piece height
		pLadder->curstate.origin.z -= LADDER_PIECE_HEIGHT;
		if(!cl_renderfuncs.pfnDrawVBMModel(pLadder, VBM_RENDER))
		{
			cl_renderfuncs.pfnVBMEndDraw();
			return false;
		}
	}

	cl_renderfuncs.pfnVBMEndDraw();

	// Restore these
	Math::VectorCopy(savedOrigin, pLadder->curstate.origin);
	pLadder->curstate.body = savedBody;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CLadder::DrawLadderVSM( cl_entity_t *pLadder, cl_dlight_t *dl )
{
	// Use a dummy to render
	Uint64 savedBody = pLadder->curstate.body;

	// Remember origin
	Vector savedOrigin;
	Math::VectorCopy(pLadder->curstate.origin, savedOrigin);

	if(!cl_renderfuncs.pfnVBMPrepareDraw())
		return false;

	if(!cl_renderfuncs.pfnDrawVBMModelVSM(pLadder, dl))
	{
		cl_renderfuncs.pfnVBMEndVSMDraw();
		return false;
	}

	pLadder->curstate.body = 0; // Remove top part

	// Draw segments
	Uint32 numsegments = pLadder->curstate.numsegments;
	for(Uint32 i = 0; i < numsegments; i++)
	{
		// Subtract ladder piece height
		pLadder->curstate.origin.z -= LADDER_PIECE_HEIGHT;
		if(!cl_renderfuncs.pfnDrawVBMModelVSM(pLadder, dl))
		{
			cl_renderfuncs.pfnVBMEndVSMDraw();
			return false;
		}
	}

	cl_renderfuncs.pfnVBMEndVSMDraw();

	// Restore these
	Math::VectorCopy(savedOrigin, pLadder->curstate.origin);
	pLadder->curstate.body = savedBody;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CLadder::Draw( void ) 
{
	if(m_pCvarDrawEntities->GetValue() < 1)
		return true;

	Uint32 nbvisentities = cl_renderfuncs.pfnGetNumRenderEntities();
	for(Uint32 i = 0; i < nbvisentities; i++)
	{
		cl_entity_t* pentity = cl_renderfuncs.pfnGetRenderEntityByIndex(i);
		if(!pentity)
			continue;
		
		if(!(pentity->curstate.effects & EF_LADDER))
			continue;

		if(pentity->pmodel->type != MOD_VBM)
			continue;

		if(!DrawLadder(pentity))
		{
			cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
			return false;
		}
	}

	if(m_viewEntity.pmodel)
	{
		if(m_activeLadderState == LADDER_STATE_ENTERING && m_viewBlend)
			return true;

		if(!cl_renderfuncs.pfnVBMPrepareDraw())
		{
			cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
			return false;
		}

		if(!cl_renderfuncs.pfnDrawVBMModel(&m_viewEntity, (VBM_RENDER|VBM_ANIMEVENTS)))
		{
			cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
			cl_renderfuncs.pfnVBMEndDraw();
			return false;
		}

		cl_renderfuncs.pfnVBMEndDraw();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CLadder::DrawVSM( cl_dlight_t *dl ) const
{
	if(m_pCvarDrawEntities->GetValue() < 1)
		return true;

	Uint32 nbvisentities = cl_renderfuncs.pfnGetNumRenderEntities();
	for(Uint32 i = 0; i < nbvisentities; i++)
	{
		cl_entity_t* pentity = cl_renderfuncs.pfnGetRenderEntityByIndex(i);
		if(!pentity)
			continue;

		if(pentity->pmodel->type != MOD_VBM)
			continue;

		if(!(pentity->curstate.effects & EF_LADDER))
			continue;
		
		if(!DrawLadderVSM(pentity, dl))
		{
			cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
			return false;
		}
	}
	
	return true;
}