/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "clientdll.h"
#include "ref_params.h"
#include "viewcontroller.h"
#include "cache_model.h"
#include "studio.h"
#include "vbm_shared.h"
#include "com_math.h"
#include "viewmodel.h"
#include "view.h"
#include "constants.h"
#include "shake.h"
#include "ref_params.h"
#include "msgreader.h"
#include "matrix.h"
#include "view.h"

// Class definition
CViewController gViewController;

//=============================================
// @brief
//
//=============================================
CViewController::CViewController( void ):
	m_pControllerEntity(nullptr), 
	m_pViewHands(nullptr), 
	m_mode(CONTROLLER_MODE_OFF),
	m_blendMode(BLEND_MODE_OFF),
	m_blend(false),
	m_blendTime(0),
	m_blendDelta(0),
	m_lerpDelay(0),
	m_cameraReset(false),
	m_alwaysDraw(false)
{
}

//=============================================
// @brief
//
//=============================================
CViewController::~CViewController( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CViewController::Init( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void CViewController::Shutdown( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CViewController::InitGame( void )
{
	m_pViewHands = cl_engfuncs.pfnLoadModel(V_SEQUENCES_MODEL_NAME);
	if(!m_pViewHands)
	{
		cl_engfuncs.pfnCon_EPrintf("Failed to load model '%s'.\n", V_SEQUENCES_MODEL_NAME);
		return false;
	}

	m_viewEntity.curstate.framerate = 1.0;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CViewController::ClearGame( void )
{
	m_mode = CONTROLLER_MODE_OFF;
	m_pControllerEntity = nullptr;
	m_pViewHands = nullptr;
	m_blend = false;
	m_blendTime = 0;
	m_blendDelta = 0;
	m_alwaysDraw = false;

	m_clientEntity = cl_entity_t();
}

//=============================================
// @brief
//
//=============================================
bool CViewController::IsActive( void ) const
{
	return (m_mode != CONTROLLER_MODE_OFF) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Float CViewController::GetSequenceTime( const cache_model_t *pmodel, Int32 sequence )
{
	if(!pmodel || !pmodel->pcachedata)
	{
		cl_engfuncs.pfnCon_Printf("%s - Model not set.\n", __FUNCTION__);
		return 0;
	}

	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	return VBM_GetSequenceTime(pstudiohdr, sequence, m_viewEntity.curstate.framerate);
}

//=============================================
// @brief
//
//=============================================
Float CViewController::EstimateFrame( cl_entity_t *pentity, Int32 sequence, Float flTime ) const
{
	if(!pentity->pmodel || !pentity->pmodel->pcachedata)
	{
		cl_engfuncs.pfnCon_Printf("%s - Model not set.\n", __FUNCTION__);
		return 0;
	}

	const vbmcache_t* pcache = pentity->pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	if(sequence < 0 || sequence >= pstudiohdr->numseq)
	{
		cl_engfuncs.pfnCon_Printf("%s - Bogus sequence %d specified.\n", __FUNCTION__, sequence);
		return 0;
	}

	const mstudioseqdesc_t *pseqdesc = pstudiohdr->getSequence(sequence);
	return VBM_EstimateFrame(pseqdesc, pentity->curstate, flTime);
}

//=============================================
// @brief
//
//=============================================
void CViewController::CalcRefDef( ref_params_t& params )
{
	if(m_blend)
	{
		// Reset blending if needed
		if((m_blendTime != -1) && ((m_blendTime+m_blendDelta) <= params.time))
		{
			m_blendTime = -1;
			m_blend = false;
		}

		if(m_blend)
		{
			if(m_blendTime == -1)
			{
				Math::VectorCopy(params.v_angles, m_blendAngles);
				Math::VectorCopy(params.v_origin, m_blendOrigin);

				m_blendTime = params.time;
			}
		}
	}

	switch(m_mode)
	{
	default:
	case CONTROLLER_MODE_VIEWCAM:
		CalcView_CameraModel(params);
		break;
	}

	// Add in any view shakes
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
void CViewController::GetViewInfoFromEntity( cl_entity_t* pentity, Vector& origin, Vector& angles )
{
	Vector forward, left;
	cl_engfuncs.pfnUpdateAttachments(pentity);
	Math::VectorCopy(pentity->getAttachment(0), origin);

	// Get forward
	Math::VectorSubtract(pentity->getAttachment(1), pentity->getAttachment(0), forward);
	Math::VectorNormalize(forward);

	// Get left
	Math::VectorSubtract(pentity->getAttachment(2), pentity->getAttachment(0), left);
	Math::VectorNormalize(left);

	angles = Math::VectorToAngles(forward, left);

	for(Int32 i = 0; i < 3; i++)
		angles[i] = Math::AngleMod(angles[i]);
}

//=============================================
// @brief
//
//=============================================
void CViewController::GetViewInfo( Vector& origin, Vector& angles )
{
	if(m_mode == CONTROLLER_MODE_VIEWCAM)
	{
		// Make sure this is uniform
		Math::VectorCopy(m_pControllerEntity->curstate.origin, m_pControllerEntity->prevstate.origin);
		Math::VectorCopy(m_pControllerEntity->curstate.origin, m_pControllerEntity->latched.origin);

		// Get it from the controller entity
		GetViewInfoFromEntity(m_pControllerEntity, origin, angles);
	}
	else
	{
		entindex_t viewentindex = gDefaultView.GetViewEntity();
		if(viewentindex != NO_ENTITY_INDEX)
		{
			cl_entity_t* pentity = cl_engfuncs.pfnGetEntityByIndex(viewentindex);
			Math::VectorCopy(pentity->curstate.origin, origin);
			Math::VectorCopy(pentity->curstate.angles, angles);
		}
		else
		{
			cl_entity_t* pentity = cl_engfuncs.pfnGetLocalPlayer();
			Math::VectorAdd(pentity->curstate.origin, pentity->curstate.view_offset, origin);
			Math::VectorCopy(pentity->curstate.viewangles, angles);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CViewController::CalcView_CameraModel( ref_params_t& params )
{
	Vector vorigin, vangles;

	// Make sure this is uniform
	Math::VectorCopy(m_pControllerEntity->curstate.origin, m_pControllerEntity->prevstate.origin);
	Math::VectorCopy(m_pControllerEntity->curstate.origin, m_pControllerEntity->latched.origin);

	// Get it via shared function
	GetViewInfoFromEntity(m_pControllerEntity, vorigin, vangles);

	// Calculate blend-to
	if(m_blend)
	{
		Float time = clamp((params.time - m_blendTime), 0, m_blendDelta);
		Float flfrac = Common::SplineFraction( time, (1.0/m_blendDelta) );

		Vector diff;
		Math::VectorSubtract(vorigin, m_blendOrigin, diff);
		Math::VectorMA(m_blendOrigin, flfrac, diff, vorigin);

		for(Int32 i = 0; i < 3; i++)
		{
			Float fldiff = Math::AngleDiff(vangles[i], m_blendAngles[i]);
			vangles[i] = m_blendAngles[i]+fldiff*(flfrac);
		}
	}

	Math::VectorCopy(vorigin, params.v_origin);
	Math::VectorCopy(vangles, params.v_angles);
}

//=============================================
// @brief
//
//=============================================
Int32 CViewController::LookupSequence( const cache_model_t *pmodel, const Char *szname )
{
	if(!pmodel || !pmodel->pcachedata)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Model not set.\n", __FUNCTION__);
		return NO_SEQUENCE_VALUE;
	}

	const vbmcache_t* pcache = pmodel->getVBMCache();
	studiohdr_t* pstudiohdr = pcache->pstudiohdr;

	return VBM_FindSequence(pstudiohdr, szname);
}

//=============================================
// @brief
//
//=============================================
void CViewController::ProcessMessage( const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	m_mode = (viewcontroller_modes_t)reader.ReadByte();
	if(m_mode == CONTROLLER_MODE_OFF)
	{
		gDefaultView.ResetViewIdle();
		gDefaultView.ResetViewRoll();

		m_pControllerEntity = nullptr;
		m_alwaysDraw = false;

		gDefaultView.SetFOVOverride(0);
		return;
	}

	entindex_t entindex = reader.ReadInt16();
	m_pControllerEntity = cl_engfuncs.pfnGetEntityByIndex(entindex);
	if(!m_pControllerEntity)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Unable to get view controller entity.\n", __FUNCTION__);
		return;
	}

	Uint32 fovOverride = reader.ReadByte();
	if(fovOverride)
		gDefaultView.SetFOVOverride(fovOverride);

	m_blend = (reader.ReadByte() == 1) ? true : false;
	Float blendDelta = reader.ReadSmallFloat();
	if(m_blend)
	{
		m_blendDelta = blendDelta;
		m_blendTime = -1;
	}

	m_alwaysDraw = (reader.ReadByte() == 1) ? true : false;
	m_clientEntity.pmodel = nullptr;
	m_clientEntity.curstate.modelindex = 0;
}

//=============================================
// @brief
//
//=============================================
bool CViewController::Draw ( void )
{
	if(!IsActive())
		return true;

	// Prepare for VBM rendering
	if(!cl_renderfuncs.pfnVBMPrepareDraw())
	{
		cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
		return false;
	}

	// Draw client entity if set
	if(m_clientEntity.pmodel)
	{
		if(!cl_renderfuncs.pfnDrawVBMModel(&m_clientEntity, (VBM_RENDER|VBM_ANIMEVENTS)))
		{
			cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
			cl_renderfuncs.pfnVBMEndDraw();
			return false;
		}
	}

	// Draw controller entity
	if(m_pControllerEntity && m_pControllerEntity->pmodel)
	{
		if(!cl_renderfuncs.pfnDrawVBMModel(m_pControllerEntity, (m_blend && !m_alwaysDraw) ? (VBM_ANIMEVENTS) : (VBM_RENDER|VBM_ANIMEVENTS)))
		{
			cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
			cl_renderfuncs.pfnVBMEndDraw();
			return false;
		}
	}

	// Finish VBM rendering
	cl_renderfuncs.pfnVBMEndDraw();
	return true;
}