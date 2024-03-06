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
#include "r_portals.h"
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
#include "r_sky.h"

// Class definition
CPortalManager gPortalManager;

//====================================
//
//====================================
CPortalManager::CPortalManager( void ):
	m_pCurrentPortal(nullptr),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pCvarPortalDebug(nullptr),
	m_numPortalsDrawn(0)
{
}

//====================================
//
//====================================
CPortalManager::~CPortalManager( void ) 
{
	Shutdown();
}

//====================================
//
//====================================
bool CPortalManager::Init( void ) 
{
	m_pCvarPortalDebug = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_NONE, "r_portals_debug", "0", "Portal debug prints.");
	return true;
}

//====================================
//
//====================================
void CPortalManager::Shutdown( void ) 
{
	ClearGL();
	ClearGame();
}

//====================================
//
//====================================
bool CPortalManager::InitGame( void ) 
{
	return true;
}

//====================================
//
//====================================
void CPortalManager::ClearGame( void ) 
{
	if(!m_portalsArray.empty())
	{
		for(Uint32 i = 0; i < m_portalsArray.size(); i++)
		{
			if(!m_portalsArray[i]->surfaces.empty())
				m_portalsArray[i]->surfaces.clear();

			delete m_portalsArray[i];
		}

		m_portalsArray.clear();
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
bool CPortalManager::InitGL( void ) 
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "portals.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Error compiling shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_vertex = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(portal_vertex_t), OFFSET(portal_vertex_t, origin));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_vertex, "in_position", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_screenwidth = m_pShader->InitUniform("screenwidth", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_screenheight = m_pShader->InitUniform("screenheight", CGLSLShader::UNIFORM_FLOAT1);

		if(!R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenwidth, "screenwidth", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenheight, "screenheight", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_fog = m_pShader->GetDeterminatorIndex("fog");

		if(!R_CheckShaderDeterminator(m_attribs.d_fog, "fog", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(m_pVBO)
	{
		m_pVBO->RebindGL();
		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//====================================
//
//====================================
void CPortalManager::ClearGL( void ) 
{
	if(!m_portalsArray.empty())
		m_portalsArray.clear();

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
bool CPortalManager::CreatePortalTexture( cl_portal_t* pportal )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	
	// Allocate a new texture
	pportal->ptexture = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);

	glEnable(GL_TEXTURE_RECTANGLE);

	glBindTexture(GL_TEXTURE_RECTANGLE, pportal->ptexture->gl_index);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, rns.screenwidth, rns.screenheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glDisable(GL_TEXTURE_RECTANGLE);

	return true;
}

//====================================
//
//====================================
void CPortalManager::AllocNewPortal( cl_entity_t* pentity )
{
	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, true, false);
		m_pShader->SetVBO(m_pVBO);
	}

	cl_portal_t *pportal = new cl_portal_t;
	m_portalsArray.push_back(pportal);
	
	// Collect the drawing surfaces we'll use
	const brushmodel_t* pbrushmodel = pentity->pmodel->getBrushmodel();
	for(Uint32 i = 0; i < pbrushmodel->nummodelsurfaces; i++)
	{
		msurface_t* psurf = &pbrushmodel->psurfaces[pbrushmodel->firstmodelsurface + i];
		mtexinfo_t *ptexinfo = psurf->ptexinfo;

		if(qstrcmp(ptexinfo->ptexture->name, "portal"))
			continue;

		pportal->surfaces.push_back(psurf);
	}

	// Determine mins/maxs
	pportal->mins = NULL_MINS;
	pportal->maxs = NULL_MAXS;

	for(Uint32 k = 0; k < pportal->surfaces.size(); k++)
	{
		msurface_t* psurf = pportal->surfaces[k];

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
				if(pportal->mins[j] > vertexpos[j])
					pportal->mins[j] = vertexpos[j];

				// maxs
				if(pportal->maxs[j] < vertexpos[j])
					pportal->maxs[j] = vertexpos[j];
			}
		}
	}

	// Pad it out by one unit
	for(Uint32 i = 0; i < 3; i++)
	{
		pportal->mins[i] -= 1;
		pportal->maxs[i] += 1;
	}

	// Set extradata for entity
	pportal->pentity = pentity;
	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	pinfo->pportaldata = pportal;

	// Set render pass id
	pportal->renderpassidx = rns.nextfreerenderpassidx;
	rns.nextfreerenderpassidx++;

	// Determine origin
	pportal->origin[0] = (pportal->mins[0] + pportal->maxs[0]) * 0.5f;
	pportal->origin[1] = (pportal->mins[1] + pportal->maxs[1]) * 0.5f;
	pportal->origin[2] = (pportal->mins[2] + pportal->maxs[2]) * 0.5f;

	// Set up VBO data
	Uint32 numverts = 0;
	for(Uint32 i = 0; i < pportal->surfaces.size(); i++)
	{
		msurface_t* psurf = pportal->surfaces[i];
		numverts += 3+(psurf->numedges-3)*3;
	}

	portal_vertex_t *pvertexes = new portal_vertex_t[numverts];

	// Set the vertex data
	Vector vertexes[3];

	Uint32 dstvertindex = 0;
	for(Uint32 i = 0; i < pportal->surfaces.size(); i++)
	{
		msurface_t* psurf = pportal->surfaces[i];

		Uint32 srcvertindex = 0;
		for(Uint32 j = 0; j < 3; j++, dstvertindex++, srcvertindex++)
		{
			Vector vertexpos;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+srcvertindex];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

			Math::VectorCopy(vertexpos, vertexes[j]);

			for(Uint32 k = 0; k < 3; k++)
				pvertexes[dstvertindex].origin[k] = vertexes[j][k];

			pvertexes[dstvertindex].origin[3] = 1.0;
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
			Math::VectorCopy(vertexpos, vertexes[2]);

			for(Uint32 k = 0; k < 3; k++, dstvertindex++)
			{
				for(Uint32 l = 0; l < 3; l++)
					pvertexes[dstvertindex].origin[l] = vertexes[k][l];

				pvertexes[dstvertindex].origin[3] = 1.0; 
			}
		}
	}

	pportal->start_vertex = m_pVBO->GetVBOSize()/sizeof(portal_vertex_t);
	pportal->num_vertexes = numverts;

	// Set FBO
	m_pVBO->Append(pvertexes, sizeof(portal_vertex_t)*numverts, nullptr, 0);
	delete[] pvertexes;

	if(!CreatePortalTexture(pportal))
	{
		Con_Printf("Failed to create texture for portal.\n");
		pportal->ptexture = nullptr;
	}
}

//====================================
//
//====================================
bool CPortalManager::DrawPortalPasses( void ) 
{
	if(m_portalsArray.empty())
		return true;

	rns.mirroring = false;
	rns.monitorpass = false;
	rns.portalpass = true;

	// number of rendered portal passes
	m_numPortalsDrawn = 0;
	Uint32 numoptimized = 0;

	// Set viewport
	glViewport(GL_ZERO, GL_ZERO, rns.view.params.screenwidth, rns.view.params.screenheight);

	CFrustum mainFrustum;
	R_SetFrustum(mainFrustum, rns.view.params.v_origin, rns.view.params.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);

	// error tracking
	bool result = true;

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_PORTALSURFACE)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pportaldata)
			continue;

		m_pCurrentPortal = pinfo->pportaldata;

		// This needs to be set every time(also, be sure to use the viewsize from params!)
		if(mainFrustum.CullBBox(m_pCurrentPortal->mins, m_pCurrentPortal->maxs))
			continue;

		if(GetMatchingPortal(i, mainFrustum) != nullptr)
		{
			numoptimized++;
			continue;
		}

		if(!SetupPortalPass())
			continue;

		// Draw the scene
		result = R_Draw(m_portalParams);
		if(!result)
			break;

		// Restore m_pCurrentPortal after R_Draw
		m_pCurrentPortal = pinfo->pportaldata;

		FinishPortalPass();
	}

	if(rns.fboused)
		R_BindFBO(nullptr);

	// Make sure this is disabled
	rns.view.frustum.DisableExtraCullBox();

	if(m_pCvarPortalDebug->GetValue() >= 1)
		Con_Printf("Number of portal scenes rendered: %d, %d optimized out.\n", m_numPortalsDrawn, numoptimized);

	rns.portalpass = false;

	return result;
}

//====================================
//
//====================================
cl_portal_t* CPortalManager::GetMatchingPortal( Uint32 currentindex, CFrustum& frustum )
{
	for(Uint32 i = 0; i < currentindex; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_PORTALSURFACE)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pportaldata)
			continue;

		cl_portal_t* pPortal = pinfo->pportaldata;

		if(frustum.CullBBox(pPortal->mins, pPortal->maxs))
			continue;

		if(pPortal->pentity->curstate.aiment == m_pCurrentPortal->pentity->curstate.aiment
			&& pPortal->pentity->curstate.scale == m_pCurrentPortal->pentity->curstate.scale)
			return pPortal;
	}

	return nullptr;
}

//====================================
//
//====================================
bool CPortalManager::SetupPortalPass( void ) 
{
	if(m_pCurrentPortal->pentity->curstate.aiment == NO_ENTITY_INDEX)
		return false;

	// Get local player
	cl_entity_t* pplayer = CL_GetLocalPlayer();

	// Get the envpos_portal entity
	Int32 envPosPortalEntityIndex = m_pCurrentPortal->pentity->curstate.aiment;
	cl_entity_t* pEnvPosPortalEntity = CL_GetEntityByIndex(envPosPortalEntityIndex);
	if(!pEnvPosPortalEntity || pEnvPosPortalEntity->curstate.msg_num != pplayer->curstate.msg_num)
		return false;
	
	entity_extrainfo_t* pEnvPosPortalInfo = CL_GetEntityExtraData(pEnvPosPortalEntity);
	if(!pEnvPosPortalInfo || !pEnvPosPortalInfo->ppvsdata)
		return false;

	// View origin to be used
	Vector viewOrigin;

	if(pEnvPosPortalEntity->curstate.scale != 0)
	{
		if(pEnvPosPortalEntity->curstate.aiment == NO_ENTITY_INDEX)
			return false;

		// Get the envpos_portal_world entity
		Int32 envPosPortalWorldEntityIndex = pEnvPosPortalEntity->curstate.aiment;
		cl_entity_t* pEnvPosPortalWorldEntity = CL_GetEntityByIndex(envPosPortalWorldEntityIndex);
		if(!pEnvPosPortalWorldEntity || pEnvPosPortalWorldEntity->curstate.msg_num != pplayer->curstate.msg_num)
			return false;

		// Calculate position with offset
		Vector vtemp;
		Math::VectorSubtract(rns.view.params.v_origin, pEnvPosPortalWorldEntity->curstate.origin, vtemp);
		Math::VectorMA(pEnvPosPortalEntity->curstate.origin, 1.0f/pEnvPosPortalEntity->curstate.scale, vtemp, viewOrigin);
	}
	else
	{
		Math::VectorCopy(pEnvPosPortalEntity->curstate.origin, viewOrigin);
	}

	// Set from camera entity
	Math::VectorCopy(viewOrigin, m_portalParams.v_origin);
	Math::VectorCopy(rns.view.params.v_angles, m_portalParams.v_angles);

	// Set FOV as well
	m_portalParams.viewsize = rns.view.params.viewsize;
	m_portalParams.screenwidth = rns.view.params.screenwidth;
	m_portalParams.screenheight = rns.view.params.screenheight;

	// Set custom skybox if set
	if(pEnvPosPortalEntity->curstate.body != NO_POSITION)
		gSkyRenderer.SetCurrentSkySet(pEnvPosPortalEntity->curstate.body);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Alter bbox if fog bbox is larger, or if there's no fog
	if(pEnvPosPortalEntity->curstate.renderamt > 0)
	{
		Vector vmins, vmaxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			vmins[i] = pEnvPosPortalEntity->curstate.origin[i] - pEnvPosPortalEntity->curstate.renderamt;
			vmaxs[i] = pEnvPosPortalEntity->curstate.origin[i] + pEnvPosPortalEntity->curstate.renderamt;
		}

		rns.view.frustum.SetExtraCullBox(vmins, vmaxs);
	}

	// Set renderpass id
	rns.renderpassidx = m_pCurrentPortal->renderpassidx;

	return true;
}

//====================================
//
//====================================
void CPortalManager::FinishPortalPass( void ) 
{
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE);

	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pCurrentPortal->ptexture->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.view.params.screenwidth, rns.view.params.screenheight, 0);

	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_RECTANGLE);

	// Get the aiment
	Uint32 envPosPortalEntityIndex = m_pCurrentPortal->pentity->curstate.aiment;
	cl_entity_t* pEnvPosPortalEntity = CL_GetEntityByIndex(envPosPortalEntityIndex);
	if(!pEnvPosPortalEntity)
		return;

	if(m_pCvarPortalDebug->GetValue() >= 1)
		Con_Printf("Portal %d renderpass: %d wpolys(%d batches), %d studio polys, %d particles.\n", m_numPortalsDrawn, rns.counters.brushpolies, rns.counters.batches, rns.counters.modelpolies, rns.counters.particles);

	// Ensure this gets reset
	gSkyRenderer.SetCurrentSkySet(NO_POSITION);

	m_numPortalsDrawn++;
}

//====================================
//
//====================================
bool CPortalManager::DrawPortals( void ) 
{
	if(m_portalsArray.empty())
		return true;

	if(rns.portalpass)
		return true;

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_vertex);

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

	// Enable rectangle textures
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE);

	m_pShader->SetUniform1i(m_attribs.u_texture, 0);
	m_pShader->SetUniform1f(m_attribs.u_screenwidth, rns.view.params.screenwidth);
	m_pShader->SetUniform1f(m_attribs.u_screenheight, rns.view.params.screenheight);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_PORTALSURFACE)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pportaldata)
			continue;

		m_pCurrentPortal = pinfo->pportaldata;

		if(rns.view.frustum.CullBBox(m_pCurrentPortal->mins, m_pCurrentPortal->maxs))
			continue;

		cl_portal_t* pBindPortal = GetMatchingPortal(i, rns.view.frustum);
		if(!pBindPortal)
			pBindPortal = m_pCurrentPortal;

		R_BindRectangleTexture(GL_TEXTURE0, pBindPortal->ptexture->gl_index);
		R_ValidateShader(m_pShader);

		glDrawArrays(GL_TRIANGLES, m_pCurrentPortal->start_vertex, m_pCurrentPortal->num_vertexes);

		// Set framecount for decals
		for(Uint32 j = 0; j < m_pCurrentPortal->surfaces.size(); j++)
			m_pCurrentPortal->surfaces[j]->visframe = cls.framecount;
	}

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	// Disable rectangle textures
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_RECTANGLE);

	return result;
}
