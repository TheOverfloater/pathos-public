/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_fbo.h"
#include "cvar.h"
#include "r_monitor.h"
#include "cl_entity.h"
#include "r_main.h"
#include "console.h"
#include "system.h"
#include "texturemanager.h"
#include "brushmodel.h"
#include "cache_model.h"
#include "enginestate.h"
#include "cl_main.h"
#include "constants.h"
#include "cl_utils.h"
#include "file.h"
#include "r_common.h"

// Scanline texture width
const Uint32 CMonitorManager::SCANLINE_TEXTURE_WIDTH = 256;
// Scanline texture height
const Uint32 CMonitorManager::SCANLINE_TEXTURE_HEIGHT = 256;

// Class definition
CMonitorManager gMonitorManager;

// Array of monitor resolutions
Uint32 g_monitorResolutions[NB_MONITOR_RESOLUTIONS][2] = 
{
	{ 128, 128 },
	{ 256, 256 },
	{ 512, 512 },
	{ 170, 128 },
	{ 341, 256 },
	{ 682, 512 },
	{ 320, 128 },
	{ 640, 256 },
	{ 1280, 512 }
};

//====================================
//
//====================================
CMonitorManager::CMonitorManager( void ):
	m_pCurrentMonitor(nullptr),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pScanlineTexture(nullptr),
	m_pCvarMonitorsDebug(nullptr),
	m_numMonitorsDrawn(0)
{
}

//====================================
//
//====================================
CMonitorManager::~CMonitorManager( void ) 
{
	Shutdown();
}

//====================================
//
//====================================
bool CMonitorManager::Init( void ) 
{
	m_pCvarMonitorsDebug = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_NONE, "r_monitors_debug", "0", "Monitor debug prints.");
	return true;
}

//====================================
//
//====================================
void CMonitorManager::Shutdown( void ) 
{
	ClearGL();
	ClearGame();
}

//====================================
//
//====================================
bool CMonitorManager::InitGame( void ) 
{
	return true;
}

//====================================
//
//====================================
void CMonitorManager::ClearGame( void ) 
{
	if(!m_monitorsArray.empty())
	{
		for(Uint32 i = 0; i < m_monitorsArray.size(); i++)
		{
			if(m_monitorsArray[i]->pfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_monitorsArray[i]->pfbo->fboid);
				gGLExtF.glDeleteRenderbuffers(1, &m_monitorsArray[i]->pfbo->rboid1);

				delete m_monitorsArray[i]->pfbo;
				m_monitorsArray[i]->pfbo = nullptr;
			}

			if(!m_monitorsArray[i]->surfaces.empty())
				m_monitorsArray[i]->surfaces.clear();

			delete m_monitorsArray[i];
		}

		m_monitorsArray.clear();
	}

	if(m_pShader)
	{
		m_pShader->SetVBO(nullptr);
		m_pShader->ResetShader();
	}

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}
}

//====================================
//
//====================================
bool CMonitorManager::InitGL( void ) 
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "monitors.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Error compiling shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_vertex = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(monitor_vertex_t), OFFSET(monitor_vertex_t, origin));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(monitor_vertex_t), OFFSET(monitor_vertex_t, texcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_vertex, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_scantexture = m_pShader->InitUniform("scantexture", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scantexture, "scantexture", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_fog = m_pShader->GetDeterminatorIndex("fog");
		m_attribs.d_grayscale = m_pShader->GetDeterminatorIndex("grayscale");

		if(!R_CheckShaderDeterminator(m_attribs.d_fog, "fog", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_grayscale, "grayscale", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(m_pVBO)
	{
		m_pVBO->RebindGL();
		m_pShader->SetVBO(m_pVBO);
	}

	// Create the scanline texture
	CreateScanlineTexture();

	return true;
}

//====================================
//
//====================================
void CMonitorManager::ClearGL( void ) 
{
	if(!m_monitorsArray.empty())
	{
		for(Uint32 i = 0; i < m_monitorsArray.size(); i++)
		{
			if(m_monitorsArray[i]->pfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_monitorsArray[i]->pfbo->fboid);
				gGLExtF.glDeleteRenderbuffers(1, &m_monitorsArray[i]->pfbo->rboid1);

				delete m_monitorsArray[i]->pfbo;
				m_monitorsArray[i]->pfbo = nullptr;
			}
		}

		m_monitorsArray.clear();
	}

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}

	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}
}

//====================================
//
//====================================
void CMonitorManager::CreateScanlineTexture( void ) 
{
	Uint32 texturesize = sizeof(byte)*SCANLINE_TEXTURE_WIDTH*SCANLINE_TEXTURE_HEIGHT*4;
	byte* pscanlinetexture = new byte [texturesize];

	for(Uint32 i = 0; i < SCANLINE_TEXTURE_WIDTH; i++)
	{
		for(Uint32 j = 0; j < SCANLINE_TEXTURE_HEIGHT; j++)
		{
			byte* pdata = pscanlinetexture + SCANLINE_TEXTURE_WIDTH*i*4 + j*4;

			for(Uint32 k = 0; k < 3; k++)
				pdata[k] = 0;

			pdata[3] = ((i % 2) == 1) * 64;
		}
	}

	m_pScanlineTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_APP_LEVEL);

	glBindTexture(GL_TEXTURE_2D, m_pScanlineTexture->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCANLINE_TEXTURE_WIDTH, SCANLINE_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, pscanlinetexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gGLExtF.glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] pscanlinetexture;
}

//====================================
//
//====================================
bool CMonitorManager::CreateMonitorTextures( cl_monitor_t* pmonitor )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	if(rns.fboused)
	{
		pmonitor->pfbo = new fbobind_t;

		// Create image
		pmonitor->pfbo->ptexture1 = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
		glBindTexture(GL_TEXTURE_2D, pmonitor->pfbo->ptexture1->gl_index);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pmonitor->xresolution, pmonitor->yresolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		gGLExtF.glGenRenderbuffers(1, &pmonitor->pfbo->rboid1);
		gGLExtF.glBindRenderbuffer(GL_RENDERBUFFER, pmonitor->pfbo->rboid1);
		gGLExtF.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, pmonitor->xresolution, pmonitor->yresolution);
		gGLExtF.glBindRenderbuffer(GL_RENDERBUFFER, 0);

		gGLExtF.glGenFramebuffers(1, &pmonitor->pfbo->fboid);
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, pmonitor->pfbo->fboid);
		gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pmonitor->pfbo->ptexture1->gl_index, 0);
		gGLExtF.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pmonitor->pfbo->rboid1);

		GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(eStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			Con_Printf("Framebuffer Object creation failed.\n");
			delete pmonitor->pfbo;
			return false;
		}

		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		// Make sure we use valid sizes
		if(pmonitor->xresolution > rns.screenwidth || pmonitor->yresolution > rns.screenheight)
		{
			pmonitor->xresolution = g_monitorResolutions[0][0];
			pmonitor->yresolution = g_monitorResolutions[0][1];
		}

		// Create the capture texture
		pmonitor->ptexture = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
		glBindTexture(GL_TEXTURE_2D, pmonitor->ptexture->gl_index);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pmonitor->xresolution, pmonitor->yresolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return true;
}

//====================================
//
//====================================
void CMonitorManager::AllocNewMonitor( cl_entity_t* pentity )
{
	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, true, false);
		m_pShader->SetVBO(m_pVBO);
	}

	cl_monitor_t *pmonitor = new cl_monitor_t;
	m_monitorsArray.push_back(pmonitor);
	
	// Collect the drawing surfaces we'll use
	const brushmodel_t* pbrushmodel = pentity->pmodel->getBrushmodel();
	for(Uint32 i = 0; i < pbrushmodel->nummodelsurfaces; i++)
	{
		msurface_t* psurf = &pbrushmodel->psurfaces[pbrushmodel->firstmodelsurface + i];
		mtexinfo_t *ptexinfo = psurf->ptexinfo;

		if(qstrcmp(ptexinfo->ptexture->name, "monitor"))
			continue;

		pmonitor->surfaces.push_back(psurf);
	}

	// Determine mins/maxs
	pmonitor->mins = NULL_MINS;
	pmonitor->maxs = NULL_MAXS;

	for(Uint32 k = 0; k < pmonitor->surfaces.size(); k++)
	{
		msurface_t* psurf = pmonitor->surfaces[k];

		for(Uint32 i = 0; i < psurf->numedges; i++)
		{
			Vector vertexpos;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+i];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

			for(Uint32 j = 0; j < 3; j++)
			{
				// mins
				if(pmonitor->mins[j] > vertexpos[j])
					pmonitor->mins[j] = vertexpos[j];

				// maxs
				if(pmonitor->maxs[j] < vertexpos[j])
					pmonitor->maxs[j] = vertexpos[j];
			}
		}
	}

	// Pad it out by one unit
	for(Uint32 i = 0; i < 3; i++)
	{
		pmonitor->mins[i] -= 1;
		pmonitor->maxs[i] += 1;
	}

	// Set extradata for entity
	pmonitor->pentity = pentity;
	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	pinfo->pmonitordata = pmonitor;

	// Set render pass id
	pmonitor->renderpassidx = rns.nextfreerenderpassidx;
	rns.nextfreerenderpassidx++;

	// Determine origin
	pmonitor->origin[0] = (pmonitor->mins[0] + pmonitor->maxs[0]) * 0.5f;
	pmonitor->origin[1] = (pmonitor->mins[1] + pmonitor->maxs[1]) * 0.5f;
	pmonitor->origin[2] = (pmonitor->mins[2] + pmonitor->maxs[2]) * 0.5f;

	// Set up VBO data
	Uint32 numverts = 0;
	for(Uint32 i = 0; i < pmonitor->surfaces.size(); i++)
	{
		msurface_t* psurf = pmonitor->surfaces[i];
		numverts += 3+(psurf->numedges-3)*3;
	}

	monitor_vertex_t *pvertexes = new monitor_vertex_t[numverts];

	// Set the vertex data
	Vector vertexes[3];
	Float texcoords[3][2];

	Uint32 dstvertindex = 0;
	
	for(Uint32 i = 0; i < pmonitor->surfaces.size(); i++)
	{
		msurface_t* psurf = pmonitor->surfaces[i];
		mtexinfo_t* ptexinfo = psurf->ptexinfo;

		Uint32 srcvertindex = 0;
		for(Uint32 j = 0; j < 3; j++, dstvertindex++, srcvertindex++)
		{
			Vector vertexpos;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+srcvertindex];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

			// Set texcoords
			Float scoord = Math::DotProduct(vertexpos, ptexinfo->vecs[0])+ptexinfo->vecs[0][3];
			scoord /= (Float)psurf->ptexinfo->ptexture->width;

			Float tcoord = Math::DotProduct(vertexpos, ptexinfo->vecs[1])+ptexinfo->vecs[1][3];
			tcoord /= (Float)psurf->ptexinfo->ptexture->height;

			Math::VectorCopy(vertexpos, vertexes[j]);
			texcoords[j][0] = scoord;
			texcoords[j][1] = (1.0 - tcoord);

			for(Uint32 k = 0; k < 3; k++)
				pvertexes[dstvertindex].origin[k] = vertexes[j][k];

			pvertexes[dstvertindex].origin[3] = 1.0; 
			pvertexes[dstvertindex].texcoord[0] = texcoords[j][0];
			pvertexes[dstvertindex].texcoord[1] = texcoords[j][1];
		}

		for(Uint32 j = 0; j < (psurf->numedges-3); j++, srcvertindex++)
		{
			Vector vertexpos;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+srcvertindex];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

			Math::VectorCopy(vertexes[2], vertexes[1]);
			texcoords[1][0] = texcoords[2][0];
			texcoords[1][1] = texcoords[2][1];

			// Set texcoords
			Float scoord = Math::DotProduct(vertexpos, ptexinfo->vecs[0])+ptexinfo->vecs[0][3];
			scoord /= (Float)psurf->ptexinfo->ptexture->width;

			Float tcoord = Math::DotProduct(vertexpos, ptexinfo->vecs[1])+ptexinfo->vecs[1][3];
			tcoord /= (Float)psurf->ptexinfo->ptexture->height;

			Math::VectorCopy(vertexpos, vertexes[2]);
			texcoords[2][0] = scoord;
			texcoords[2][1] = (1.0 - tcoord);

			for(Uint32 k = 0; k < 3; k++, dstvertindex++)
			{
				for(Uint32 l = 0; l < 3; l++)
					pvertexes[dstvertindex].origin[l] = vertexes[k][l];

				pvertexes[dstvertindex].origin[3] = 1.0; 
				pvertexes[dstvertindex].texcoord[0] = texcoords[k][0];
				pvertexes[dstvertindex].texcoord[1] = texcoords[k][1];
			}
		}
	}

	pmonitor->start_vertex = m_pVBO->GetVBOSize()/sizeof(monitor_vertex_t);
	pmonitor->num_vertexes = numverts;

	// Set FBO
	m_pVBO->Append(pvertexes, sizeof(monitor_vertex_t)*numverts, nullptr, 0);
	delete[] pvertexes;

	// Make sure the resolution is correct
	if(pentity->curstate.skin >= NB_MONITOR_RESOLUTIONS || pentity->curstate.skin < 0)
	{
		Con_Printf("Invalid resolution set for monitor entity.\n");
		pentity->curstate.skin = 0;
	}

	pmonitor->xresolution = g_monitorResolutions[pentity->curstate.skin][0];
	pmonitor->yresolution = g_monitorResolutions[pentity->curstate.skin][1];

	if(!CreateMonitorTextures(pmonitor))
	{
		Con_Printf("Failed to create textures for monitor.\n");
		pmonitor->ptexture = nullptr;
		pmonitor->pfbo = nullptr;
	}
}

//====================================
//
//====================================
bool CMonitorManager::DrawMonitorPasses( void ) 
{
	if(m_monitorsArray.empty())
		return true;

	rns.mirroring = false;
	rns.monitorpass = true;

	// number of rendered monitor passes
	m_numMonitorsDrawn = 0;
	Uint32 numoptimized = 0;

	CFrustum mainFrustum;
	R_SetFrustum(mainFrustum, rns.view.params.v_origin, rns.view.params.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);

	// error tracking
	bool result = true;

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_MONITORENTITY)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pmonitordata)
			continue;

		m_pCurrentMonitor = pinfo->pmonitordata;

		// This needs to be set every time(also, be sure to use the viewsize from params!)
		if(mainFrustum.CullBBox(m_pCurrentMonitor->mins, m_pCurrentMonitor->maxs))
			continue;

		if(GetMatchingMonitor(i) != nullptr)
		{
			numoptimized++;
			continue;
		}

		if(!SetupMonitorPass())
			continue;

		// Draw the scene
		result = R_Draw(m_monitorParams);
		if(!result)
			break;

		// Restore after R_Draw
		m_pCurrentMonitor = pinfo->pmonitordata;

		FinishMonitorPass();
	}

	rns.monitorpass = false;

	if(rns.fboused)
		R_BindFBO(nullptr);

	glViewport(GL_ZERO, GL_ZERO, rns.screenwidth, rns.screenheight);

	// Make sure this is disabled
	rns.view.frustum.DisableExtraCullBox();

	if(m_pCvarMonitorsDebug->GetValue() >= 1)
		Con_Printf("Number of monitor scenes rendered: %d, %d optimized out.\n", m_numMonitorsDrawn, numoptimized);

	return result;
}

//====================================
//
//====================================
cl_monitor_t* CMonitorManager::GetMatchingMonitor( Uint32 currentindex )
{
	for(Uint32 i = 0; i < currentindex; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_MONITORENTITY)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pmonitordata)
			continue;

		cl_monitor_t* pMonitor = pinfo->pmonitordata;

		if(rns.view.frustum.CullBBox(pMonitor->mins, pMonitor->maxs))
			continue;

		if(pMonitor->pentity->curstate.aiment == m_pCurrentMonitor->pentity->curstate.aiment
			&& pMonitor->pentity->curstate.body == m_pCurrentMonitor->pentity->curstate.body
			&& pMonitor->pentity->curstate.scale == m_pCurrentMonitor->pentity->curstate.scale
			&& pMonitor->xresolution == m_pCurrentMonitor->xresolution
			&& pMonitor->yresolution == m_pCurrentMonitor->yresolution)
			return pMonitor;
	}

	return nullptr;
}

//====================================
//
//====================================
bool CMonitorManager::SetupMonitorPass( void ) 
{
	if(m_pCurrentMonitor->pentity->curstate.aiment == NO_ENTITY_INDEX)
		return false;

	// Get local player
	cl_entity_t* pplayer = CL_GetLocalPlayer();

	// Get the aiment
	Int32 cameraEntityIndex = m_pCurrentMonitor->pentity->curstate.aiment;
	cl_entity_t* pCameraEntity = CL_GetEntityByIndex(cameraEntityIndex);
	if(!pCameraEntity || pCameraEntity->curstate.msg_num != pplayer->curstate.msg_num)
		return false;
	
	entity_extrainfo_t* pcamerainfo = CL_GetEntityExtraData(pCameraEntity);
	if(!pcamerainfo || !pcamerainfo->ppvsdata)
		return false;

	// Set from camera entity
	Math::VectorCopy(pCameraEntity->curstate.origin, m_monitorParams.v_origin);
	Math::VectorCopy(pCameraEntity->curstate.angles, m_monitorParams.v_angles);

	// Set FOV as well
	m_monitorParams.viewsize = pCameraEntity->curstate.scale;

	// Bind FBO
	if(rns.fboused && m_pCurrentMonitor->pfbo)
		R_BindFBO(m_pCurrentMonitor->pfbo);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_monitorParams.screenwidth = m_pCurrentMonitor->xresolution;
	m_monitorParams.screenheight = m_pCurrentMonitor->yresolution;

	// Set viewport
	glViewport(GL_ZERO, GL_ZERO, m_monitorParams.screenwidth, m_monitorParams.screenheight);

	// Alter bbox if fog bbox is larger, or if there's no fog
	if(pCameraEntity->curstate.renderamt > 0)
	{
		Vector vmins, vmaxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			vmins[i] = pCameraEntity->curstate.origin[i] - pCameraEntity->curstate.renderamt;
			vmaxs[i] = pCameraEntity->curstate.origin[i] + pCameraEntity->curstate.renderamt;
		}

		rns.view.frustum.SetExtraCullBox(vmins, vmaxs);
	}

	// Set renderpass id
	rns.renderpassidx = m_pCurrentMonitor->renderpassidx;

	return true;
}

//====================================
//
//====================================
void CMonitorManager::FinishMonitorPass( void ) 
{
	if(!rns.fboused || !m_pCurrentMonitor->pfbo)
	{
		R_Bind2DTexture(GL_TEXTURE0_ARB, m_pCurrentMonitor->ptexture->gl_index);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, m_pCurrentMonitor->xresolution, m_pCurrentMonitor->yresolution, 0);
	}

	// Get the aiment
	Uint32 cameraEntityIndex = m_pCurrentMonitor->pentity->curstate.aiment;
	cl_entity_t* pCameraEntity = CL_GetEntityByIndex(cameraEntityIndex);
	if(!pCameraEntity)
		return;

	if(m_pCvarMonitorsDebug->GetValue() >= 1)
		Con_Printf("Monitor %d renderpass: %d wpolys(%d batches), %d studio polys, %d particles.\n", m_numMonitorsDrawn, rns.counters.brushpolies, rns.counters.batches, rns.counters.modelpolies, rns.counters.particles);

	m_numMonitorsDrawn++;
}

//====================================
//
//====================================
bool CMonitorManager::DrawMonitors( void ) 
{
	if(m_monitorsArray.empty())
		return true;

	if(rns.monitorpass)
		return true;

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_vertex);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	bool result = true;

	if(rns.fog.settings.active)
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, 1);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/((Float)rns.fog.settings.end-(Float)rns.fog.settings.start));
	}
	else
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, 0);
	}

	if(!result)
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->SetUniform1i(m_attribs.u_texture, 0);
	m_pShader->SetUniform1i(m_attribs.u_scantexture, 1);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	R_Bind2DTexture(GL_TEXTURE1, m_pScanlineTexture->gl_index);

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_MONITORENTITY)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pmonitordata)
			continue;

		m_pCurrentMonitor = pinfo->pmonitordata;

		if(rns.view.frustum.CullBBox(m_pCurrentMonitor->mins, m_pCurrentMonitor->maxs))
			continue;

		m_pShader->SetDeterminator(m_attribs.d_grayscale, (pentity->curstate.body == 1) ? 1 : 0);

		cl_monitor_t* pBindMonitor = GetMatchingMonitor(i);
		if(!pBindMonitor)
			pBindMonitor = m_pCurrentMonitor;

		if(rns.fboused && pBindMonitor->pfbo)
			R_Bind2DTexture(GL_TEXTURE0, pBindMonitor->pfbo->ptexture1->gl_index);
		else
			R_Bind2DTexture(GL_TEXTURE0, pBindMonitor->ptexture->gl_index);

		R_ValidateShader(m_pShader);

		glDrawArrays(GL_TRIANGLES, m_pCurrentMonitor->start_vertex, m_pCurrentMonitor->num_vertexes);

		// Set framecount for decals
		for(Uint32 j = 0; j < m_pCurrentMonitor->surfaces.size(); j++)
			m_pCurrentMonitor->surfaces[j]->visframe = cls.framecount;
	}

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	return result;
}
