/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "viewmodel.h"
#include "clientdll.h"
#include "cache_model.h"
#include "studio.h"
#include "vbm_shared.h"
#include "matrix.h"
#include "view.h"
#include "msgreader.h"
#include "com_math.h"

// View model default FOV value
const Uint32 CViewModel::VIEWMODEL_DEFAULT_FOV_VALUE = 75;

// Class definition
CViewModel gViewModel;

//====================================
//
//====================================
CViewModel::CViewModel ( void ):
	m_pCvarDrawViewModel(nullptr),
	m_pModel(nullptr),
	m_iIdealSequence(0),
	m_bBlendSequence(false),
	m_bUpdateSequence(false),
	m_bForceDontBlend(false),
	m_applyOffsets(true),
	m_pCvarReferenceFOV(nullptr),
	m_pCvarViewModelFOV(nullptr)
{
}

//====================================
//
//====================================
CViewModel::~CViewModel ( void )
{
}

//====================================
//
//====================================
bool CViewModel::Init ( void )
{
	CString fovValue;
	fovValue << (Int32)VIEWMODEL_DEFAULT_FOV_VALUE;

	// Create new cvars
	m_pCvarViewModelFOV = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "viewmodel_fov", fovValue.c_str(), "Viewmodel FOV.");
	m_pCvarDrawViewModel = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_drawviewmodel", "1", "Toggle view model rendering.");

	// Get default fov cvar
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
bool CViewModel::InitGame ( void )
{
	// Set basics
	m_viewModelEntity.entindex = VIEWMODEL_ENTITY_INDEX;
	m_viewModelEntity.curstate.framerate = 1;
	m_viewModelEntity.curstate.effects |= EF_FASTINTERP|EF_CLIENTENT;
	// Default to true
	m_applyOffsets = true;

	return true;
}

//====================================
//
//====================================
void CViewModel::ClearGame ( void )
{
	// Reset vmodel
	m_viewModelEntity = cl_entity_t();
	m_idealModel.clear();
	m_pModel = nullptr;

	m_bForceDontBlend = true;
}

//====================================
//
//====================================
const mstudioseqdesc_t* CViewModel::GetSequenceInfo ( Int32 sequence )
{
	if(!m_pModel || !m_pModel->pcachedata)
		return nullptr;

	const vbmcache_t* pcache = m_pModel->getVBMCache();
	const studiohdr_t* phdr = pcache->pstudiohdr;
	if(!phdr)
		return nullptr;

	if(sequence < 0 || sequence > phdr->numseq)
		sequence = 0;

	const mstudioseqdesc_t* pseqdesc = phdr->getSequence(sequence);
	return pseqdesc;
}

//====================================
//
//====================================
void  CViewModel::Think ( void )
{
	// Check for viewmodel changes
	if(m_idealModel.empty())
	{
		m_pModel = nullptr;
		m_viewModelEntity.pmodel = nullptr;
		cl_efxapi.pfnFreeEntityData(m_viewModelEntity.entindex, FREE_MSG_FL_NONE);
	}
	else if(!m_pModel)
	{
		cl_efxapi.pfnFreeEntityData(m_viewModelEntity.entindex, FREE_MSG_FL_NONE);
		m_pModel = cl_engfuncs.pfnLoadModel(m_idealModel.c_str());
		m_bForceDontBlend = true;
	}
	else
	{
		if(qstrcmp(m_idealModel, m_pModel->name))
		{
			cl_efxapi.pfnFreeEntityData(m_viewModelEntity.entindex, FREE_MSG_FL_NONE);
			m_pModel = cl_engfuncs.pfnLoadModel(m_idealModel.c_str());
			m_bForceDontBlend = true;
		}
	}

	if(!m_pModel)
		return;

	// Reset viewmodel
	m_viewModelEntity.pmodel = m_pModel;
	if(m_viewModelEntity.pmodel)
		m_viewModelEntity.curstate.modelindex = m_viewModelEntity.pmodel->cacheindex;

	Double time = cl_engfuncs.pfnGetClientTime();

	// Change sequence if requested
	if(m_bUpdateSequence)
	{
		if(m_bBlendSequence && !m_bForceDontBlend)
		{
			m_viewModelEntity.latched.prevseqblending[0] = m_viewModelEntity.curstate.blending[0];
			m_viewModelEntity.latched.prevseqblending[1] = m_viewModelEntity.curstate.blending[1];

			m_viewModelEntity.latched.sequence = m_viewModelEntity.curstate.sequence;
			m_viewModelEntity.latched.animtime = m_viewModelEntity.curstate.animtime;

			const mstudioseqdesc_t* pseq = GetSequenceInfo(m_viewModelEntity.curstate.sequence);
			if(pseq)
				m_viewModelEntity.latched.frame = VBM_EstimateFrame(pseq, m_viewModelEntity.curstate, time);

			m_viewModelEntity.latched.sequencetime = time;
		}
		else
		{
			m_viewModelEntity.latched.sequencetime = 0;
		}

		m_viewModelEntity.curstate.sequence = m_iIdealSequence;
		m_viewModelEntity.curstate.animtime = time;
		m_viewModelEntity.curstate.frame = 0;
		m_viewModelEntity.curstate.framerate = 1.0;

		m_bUpdateSequence = false;
		m_bForceDontBlend = false;
	}

	// Copy previous frame state
	m_viewModelEntity.prevstate = m_viewModelEntity.curstate;

	// Make sure msg_num is set
	cl_entity_t* pPlayer = cl_engfuncs.pfnGetLocalPlayer();
	if(pPlayer)
	{
		m_viewModelEntity.curstate.msg_num = pPlayer->curstate.msg_num;
		m_viewModelEntity.curstate.msg_time = pPlayer->curstate.msg_time;
	}

	// Set groundentity for ground entity
	Vector start, end;
	Math::VectorAdd(m_viewModelEntity.curstate.origin, Vector(0, 0, 16), start);
	Math::VectorSubtract(m_viewModelEntity.curstate.origin, Vector(0, 0, 16384), end);

	trace_t tr;
	cl_tracefuncs.pfnPlayerTrace(start, end, FL_TRACE_NO_MODELS, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(!tr.allSolid() && !tr.startSolid() && !tr.noHit() && tr.hitentity != NO_ENTITY_INDEX)
		m_viewModelEntity.curstate.groundent = m_viewModelEntity.prevstate.groundent = tr.hitentity;
	else
		m_viewModelEntity.curstate.groundent = m_viewModelEntity.prevstate.groundent = WORLDSPAWN_ENTITY_INDEX;
}

//====================================
//
//====================================
cl_entity_t *CViewModel::GetViewModel ( void )
{
	return &m_viewModelEntity;
}

//====================================
//
//====================================
void CViewModel::SetSequence ( Int32 sequence, Int64 body, Int32 skin, bool blendsequence )
{
	// Set body
	m_viewModelEntity.curstate.body = body;
	m_viewModelEntity.curstate.skin = skin;

	m_bBlendSequence = blendsequence;
	m_iIdealSequence = sequence;
	m_bUpdateSequence = true;
}

//====================================
//
//====================================
bool CViewModel::Draw ( void )
{
	if(!m_viewModelEntity.pmodel)
		return true;

	// Depth range hack
	glDepthRange(0.0, 0.1);

	CMatrix& projection = cl_renderfuncs.pfnGetProjectionMatrix();

	Uint32 currentFOV = gDefaultView.GetFOV();
	Uint32 vmFovValue = m_pCvarViewModelFOV->GetValue()*(currentFOV/m_pCvarReferenceFOV->GetValue());

	// Set up a projection matrix for view model fox
	projection.PushMatrix();
	cl_renderfuncs.pfnSetProjectionMatrix(1.0f, vmFovValue);

	bool result = cl_renderfuncs.pfnVBMPrepareDraw();

	// Only draw if setup went fine
	if(result)
		result = cl_renderfuncs.pfnDrawVBMModel(&m_viewModelEntity, (VBM_ANIMEVENTS|VBM_RENDER));

	cl_renderfuncs.pfnVBMEndDraw();

	// Only draw if rendering went fine
	if(result)
	{
		// Draw viewmodel particles
		result = cl_renderfuncs.pfnDrawViewModelParticles();
	}
	
	projection.PopMatrix();

	// Restore glDepthRange
	glDepthRange(0.0, 1.0);

	if(!result)
		cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());

	return result;
}

//====================================
//
//====================================
void CViewModel::ProcessMessage( const byte* pdata, Uint32 msgsize )
{
	CMSGReader reader(pdata, msgsize);

	// Read in msg type
	vm_msg_types_t msgtype = (vm_msg_types_t)reader.ReadByte();
	switch(msgtype)
	{
	case VMODEL_SET_SEQUENCE:
		{
			Int32 sequence = reader.ReadByte();
			Int64 body = reader.ReadInt64();
			Int32 skin = reader.ReadInt32();
			bool blend = (reader.ReadByte() == 1) ? true : false;

			// Set sequence
			SetSequence(sequence, body, skin, blend);
		}
		break;
	case VMODEL_SET_MODEL:
		{
			// Read model name
			const Char* pstrmodelname = reader.ReadString();
			if(!pstrmodelname || !qstrlen(pstrmodelname))
			{
				m_idealModel.clear();
				m_pModel = nullptr;
				return;
			}

			// Set ideal model
			m_idealModel = pstrmodelname;
		}
		break;
	case VMODEL_SET_SKIN:
		m_viewModelEntity.curstate.skin = reader.ReadInt32();
		break;
	case VMODEL_SET_BODY:
		m_viewModelEntity.curstate.body = reader.ReadInt64();
		break;
	case VMODEL_SET_OFFSETS_ENABLE:
		m_applyOffsets = (reader.ReadByte() == 0) ? false : true;
		break;
	default:
		cl_engfuncs.pfnCon_Printf("%s - Unknown message type %d specified.\n", __FUNCTION__, (Int32)msgtype);
		break;
	}
}
