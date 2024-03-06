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
#include "r_mirror.h"
#include "r_bsp.h"
#include "r_common.h"
#include "r_main.h"
#include "texturemanager.h"
#include "cl_entity.h"
#include "cache_model.h"
#include "brushmodel.h"
#include "system.h"
#include "enginestate.h"
#include "cl_main.h"
#include "file.h"

// Mirror FBO resolution
const Uint32 CMirrorManager::MIRROR_FBO_SIZE = 1024;
// Mirror render-to-texture resolution
const Uint32 CMirrorManager::MIRROR_RTT_SIZE = 512;

// Class definition
CMirrorManager gMirrorManager;

//=============================================
//
//=============================================
CMirrorManager::CMirrorManager( void ):
	m_pCurrentMirror(nullptr),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pDepthTexture(nullptr)
{
}

//=============================================
//
//=============================================
CMirrorManager::~CMirrorManager( void )
{
	ClearGL();
	ClearGame();
}

//=============================================
//
//=============================================
bool CMirrorManager::InitGL( void ) 
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "mirrors.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_vertex = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(mirror_vertex_t), OFFSET(mirror_vertex_t, origin));
		if(!R_CheckShaderVertexAttribute(m_attribs.a_vertex, "in_position", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_dt_x = m_pShader->InitUniform("dt_x", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_dt_y = m_pShader->InitUniform("dt_y", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_mirrormatrix = m_pShader->InitUniform("mirror_matrix", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_dt_x, "dt_x", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_dt_y, "dt_y", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_mirrormatrix, "mirror_matrix", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, Sys_ErrorPopup))
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

	if(m_pDepthTexture)
		CreateDepthTexture();

	if(!m_mirrorsArray.empty())
	{
		for(Uint32 i = 0; i < m_mirrorsArray.size(); i++)
		{
			if(!AllocTextures(m_mirrorsArray[i]))
			{
				m_mirrorsArray[i]->ptexture = nullptr;
				m_mirrorsArray[i]->pfbo = nullptr;
			}
		}
	}

	return true;
}

//=============================================
//
//=============================================
void CMirrorManager::ClearGL( void ) 
{
	if(!m_mirrorsArray.empty())
	{
		for(Uint32 i = 0; i < m_mirrorsArray.size(); i++)
		{
			if(m_mirrorsArray[i]->pfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_mirrorsArray[i]->pfbo->fboid);
				delete m_mirrorsArray[i]->pfbo;
			}
		}
	}

	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(m_pVBO)
		m_pVBO->ClearGL();
}

//=============================================
//
//=============================================
bool CMirrorManager::InitGame( void ) 
{
	// Nothing here yet
	return true;
}

//=============================================
//
//=============================================
void CMirrorManager::ClearGame( void ) 
{
	if(!m_mirrorsArray.empty())
	{
		for(Uint32 i = 0; i < m_mirrorsArray.size(); i++)
		{
			if(m_mirrorsArray[i]->pfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_mirrorsArray[i]->pfbo->fboid);
				delete m_mirrorsArray[i]->pfbo;
			}

			delete m_mirrorsArray[i];
		}

		m_mirrorsArray.clear();
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

	m_pDepthTexture = nullptr;
}

//=============================================
//
//=============================================
void CMirrorManager::CreateDepthTexture( void ) 
{
	m_pDepthTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);
	glBindTexture(GL_TEXTURE_2D, m_pDepthTexture->gl_index);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, MIRROR_FBO_SIZE, MIRROR_FBO_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//=============================================
//
//=============================================
bool CMirrorManager::AllocTextures( cl_mirror_t *pmirror )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	if(rns.fboused)
	{
		if(!m_pDepthTexture)
			CreateDepthTexture();

		pmirror->pfbo = new fbobind_t;

		// Create image
		pmirror->pfbo->ptexture1 = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
		glBindTexture(GL_TEXTURE_2D, pmirror->pfbo->ptexture1->gl_index);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MIRROR_FBO_SIZE, MIRROR_FBO_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		gGLExtF.glGenFramebuffers(1, &pmirror->pfbo->fboid);
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, pmirror->pfbo->fboid);
		gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pmirror->pfbo->ptexture1->gl_index, 0);
		gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pDepthTexture->gl_index, 0);

		GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(eStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			Con_Printf("Framebuffer Object creation failed.\n");
			delete pmirror->pfbo;
			return false;
		}

		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		// Create the reflection texture
		pmirror->ptexture = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
		glBindTexture(GL_TEXTURE_2D, pmirror->ptexture->gl_index);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MIRROR_RTT_SIZE, MIRROR_RTT_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return true;
}

//=============================================
//
//=============================================
void CMirrorManager::AllocNewMirror( cl_entity_t* pentity )
{
	const brushmodel_t* pbrushmodel = pentity->pmodel->getBrushmodel();

	if(pbrushmodel->nummodelsurfaces > 1)
	{
		Con_Printf("ERROR: Mirror bmodel has more than 1 polygon!\n");
		return;
	}

	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, true, false);
		m_pShader->SetVBO(m_pVBO);
	}

	cl_mirror_t *pmirror = new cl_mirror_t;
	m_mirrorsArray.push_back(pmirror);

	msurface_t *psurf = &pbrushmodel->psurfaces[pbrushmodel->firstmodelsurface];

	pmirror->mins = NULL_MINS;
	pmirror->maxs = NULL_MAXS;

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
			if(pmirror->mins[j] > vertexpos[j])
				pmirror->mins[j] = vertexpos[j];

			if(pmirror->maxs[j] < vertexpos[j])
				pmirror->maxs[j] = vertexpos[j];
		}
	}

	pmirror->pentity = pentity;
	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	pinfo->pmirrordata = pmirror;

	// Set render pass id
	pmirror->renderpassidx = rns.nextfreerenderpassidx;
	rns.nextfreerenderpassidx++;

	pmirror->origin[0] = (pmirror->mins[0] + pmirror->maxs[0]) * 0.5f;
	pmirror->origin[1] = (pmirror->mins[1] + pmirror->maxs[1]) * 0.5f;
	pmirror->origin[2] = (pmirror->mins[2] + pmirror->maxs[2]) * 0.5f;
	pmirror->psurface = psurf;

	// Set up VBO data
	Uint32 numverts = 3+(psurf->numedges-3)*3;
	mirror_vertex_t *pvertexes = new mirror_vertex_t[numverts];

	Vector vertexes[3];
	Uint32 dstvertindex = 0;
	Uint32 srcvertindex = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		Vector vertexpos;
		Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+srcvertindex];
		if(e_index > 0)
			Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
		else
			Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

		Math::VectorCopy(vertexpos, vertexes[i]);

		for(Uint32 j = 0; j < 3; j++)
			pvertexes[dstvertindex].origin[j] = vertexes[i][j];

		pvertexes[dstvertindex].origin[3] = 1.0;

		dstvertindex++;
		srcvertindex++;
	}

	for(Uint32 i = 0; i < (psurf->numedges-3); i++)
	{
		Vector vertexpos;
		Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+srcvertindex];
		if(e_index > 0)
			Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
		else
			Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

		Math::VectorCopy(vertexes[2], vertexes[1]);
		Math::VectorCopy(vertexpos, vertexes[2]);

		for(Uint32 j = 0; j < 3; j++)
		{
			for(Uint32 k = 0; k < 3; k++)
				pvertexes[dstvertindex].origin[k] = vertexes[j][k];

			pvertexes[dstvertindex].origin[3] = 1.0;
			dstvertindex++;
		}

		srcvertindex++;
	}

	pmirror->start_vertex = m_pVBO->GetVBOSize()/sizeof(mirror_vertex_t);
	pmirror->num_vertexes = numverts;

	m_pVBO->Append(pvertexes, sizeof(mirror_vertex_t)*numverts, nullptr, 0);
	delete[] pvertexes;

	if(!AllocTextures(pmirror))
	{
		Con_Printf("Failed to allocate FBO for mirror");
		pmirror->ptexture = nullptr;
		pmirror->pfbo = nullptr;
	}
}

//=============================================
//
//=============================================
bool CMirrorManager::DrawMirrorPasses( void ) 
{
	if(m_mirrorsArray.empty())
		return true;

	rns.mirroring = true;

	// Set viewport
	if(rns.fboused)
		glViewport(GL_ZERO, GL_ZERO, MIRROR_FBO_SIZE, MIRROR_FBO_SIZE);
	else
		glViewport(GL_ZERO, GL_ZERO, MIRROR_RTT_SIZE, MIRROR_RTT_SIZE);

	CFrustum mainFrustum;
	R_SetFrustum(mainFrustum, rns.view.params.v_origin, rns.view.params.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);

	// set sizes for screen
	rns.view.viewsize_x = rns.screenwidth;
	rns.view.viewsize_y = rns.screenheight;

	// error tracking
	bool result = true;

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_MIRROR)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pmirrordata)
			continue;

		m_pCurrentMirror = pinfo->pmirrordata;
		if(!m_pCurrentMirror->pfbo && !m_pCurrentMirror->ptexture)
			continue;

		if(mainFrustum.CullBBox(m_pCurrentMirror->mins, m_pCurrentMirror->maxs))
			continue;

		SetupMirrorPass();

		// Draw reflection scene
		result = R_Draw(m_mirrorParams);
		if(!result)
			break;

		// Restore after R_Draw
		m_pCurrentMirror = pinfo->pmirrordata;

		FinishMirrorPass();
	}

	rns.mirroring = false;

	if(rns.fboused)
		R_BindFBO(nullptr);

	glViewport(GL_ZERO, GL_ZERO, rns.screenwidth, rns.screenheight);
	return result;
}

//=============================================
//
//=============================================
void CMirrorManager::SetupClipping( void )
{
	Float	dot;
	Float	eq1[4];
	Float	eq2[4];
	Float	projection[16];

	Vector	vDist;
	Vector	vForward;
	Vector	vRight;
	Vector	vUp;

	Math::AngleVectors(m_mirrorParams.v_angles, &vForward, &vRight, &vUp);
	Math::VectorSubtract(m_pCurrentMirror->origin, m_mirrorParams.v_origin, vDist);
	
	Math::VectorScale(vRight, -1, vRight);
	Math::VectorScale(vUp, -1, vUp);

	if(m_pCurrentMirror->psurface->flags & SURF_PLANEBACK)
	{
		eq1[0] = Math::DotProduct(vRight, m_pCurrentMirror->psurface->pplane->normal);
		eq1[1] = Math::DotProduct(vUp, m_pCurrentMirror->psurface->pplane->normal);
		eq1[2] = Math::DotProduct(vForward, m_pCurrentMirror->psurface->pplane->normal);
		eq1[3] = Math::DotProduct(vDist, m_pCurrentMirror->psurface->pplane->normal);
	}
	else
	{
		eq1[0] = Math::DotProduct(vRight, (-m_pCurrentMirror->psurface->pplane->normal));
		eq1[1] = Math::DotProduct(vUp, (-m_pCurrentMirror->psurface->pplane->normal));
		eq1[2] = Math::DotProduct(vForward, (-m_pCurrentMirror->psurface->pplane->normal));
		eq1[3] = Math::DotProduct(vDist, (-m_pCurrentMirror->psurface->pplane->normal));
	}

	// Set projection matrix first
	R_SetProjectionMatrix(rns.view.nearclip, rns.view.fov);

	// Change current projection matrix into an oblique projection matrix
	memcpy(projection, rns.view.projection.Transpose(), sizeof(Float)*16);

	eq2[0] = (sgn(eq1[0]) + projection[8]) / projection[0];
	eq2[1] = (sgn(eq1[1]) + projection[9]) / projection[5];
	eq2[2] = -1.0F;
	eq2[3] = (1.0F + projection[10]) / projection[14];

	dot = eq1[0]*eq2[0] + eq1[1]*eq2[1] + eq1[2]*eq2[2] + eq1[3]*eq2[3];

    projection[2] = eq1[0]*(2.0f/dot);
    projection[6] = eq1[1]*(2.0f/dot);
    projection[10] = eq1[2]*(2.0f/dot) + 1.0F;
    projection[14] = eq1[3]*(2.0f/dot);

	rns.view.projection.PushMatrix();
	rns.view.projection.SetMatrix(projection);
}

//=============================================
//
//=============================================
void CMirrorManager::SetupMirrorPass( void ) 
{
	Vector forward;
	Math::AngleVectors(rns.view.params.v_angles, &forward, nullptr, nullptr);

	Float flDist = Math::DotProduct(rns.view.params.v_origin, m_pCurrentMirror->psurface->pplane->normal) -  m_pCurrentMirror->psurface->pplane->dist;
	Math::VectorMA(rns.view.params.v_origin, -2*flDist, m_pCurrentMirror->psurface->pplane->normal, m_mirrorParams.v_origin);

	if (m_pCurrentMirror->psurface->flags & SURF_PLANEBACK)
	{
		flDist = Math::DotProduct(forward, m_pCurrentMirror->psurface->pplane->normal);
		Math::VectorMA(forward, -2*flDist, m_pCurrentMirror->psurface->pplane->normal, forward);
	}
	else
	{
		flDist = Math::DotProduct(forward, -m_pCurrentMirror->psurface->pplane->normal);
		Math::VectorMA(forward, -2*flDist, -m_pCurrentMirror->psurface->pplane->normal, forward);
	}

	m_mirrorParams.v_angles[0] = -asin(forward[2])/M_PI*180;
	m_mirrorParams.v_angles[1] = atan2(forward[1], forward[0])/M_PI*180;
	m_mirrorParams.v_angles[2] = -rns.view.params.v_angles[2];

	m_mirrorParams.screenwidth = rns.view.params.screenwidth;
	m_mirrorParams.screenheight = rns.view.params.screenheight;
	m_mirrorParams.viewsize = rns.view.params.viewsize;

	rns.view.nearclip = NEAR_CLIP_DISTANCE;
	rns.view.fov = R_GetRenderFOV(rns.view.params.viewsize);

	// Set up clipping
	SetupClipping();

	// Bind FBO
	if(rns.fboused && m_pCurrentMirror->pfbo)
		R_BindFBO(m_pCurrentMirror->pfbo);

	//Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set renderpass id
	rns.renderpassidx = m_pCurrentMirror->renderpassidx;
}

//=============================================
//
//=============================================
void CMirrorManager::FinishMirrorPass( void ) 
{
	rns.view.projection.PopMatrix();

	if(!rns.fboused || !m_pCurrentMirror->pfbo)
	{
		R_Bind2DTexture(GL_TEXTURE0_ARB, m_pCurrentMirror->ptexture->gl_index);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, MIRROR_RTT_SIZE, MIRROR_RTT_SIZE, 0);
	}
}

//=============================================
//
//=============================================
bool CMirrorManager::DrawMirrors( void ) 
{
	if(m_mirrorsArray.empty())
		return true;

	if(rns.mirroring)
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

	m_pShader->SetUniform1i(m_attribs.u_texture, 0);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_mirrormatrix, rns.view.modelview.GetMatrix());

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];
		if(pentity->curstate.rendertype != RT_MIRROR)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pmirrordata)
			continue;

		m_pCurrentMirror = pinfo->pmirrordata;

		if(!m_pCurrentMirror->ptexture && !m_pCurrentMirror->pfbo)
			continue;

		if(rns.view.frustum.CullBBox(m_pCurrentMirror->mins, m_pCurrentMirror->maxs))
			continue;

		if(m_pCurrentMirror->psurface->pplane->normal[2] == 1)
		{
			m_pShader->SetUniform1f(m_attribs.u_dt_x, 1);
			m_pShader->SetUniform1f(m_attribs.u_dt_y, -1);
		}
		else
		{
			m_pShader->SetUniform1f(m_attribs.u_dt_x, -1);
			m_pShader->SetUniform1f(m_attribs.u_dt_y, 1);
		}

		if(rns.fboused && m_pCurrentMirror->pfbo)
			R_Bind2DTexture(GL_TEXTURE0, m_pCurrentMirror->pfbo->ptexture1->gl_index);
		else
			R_Bind2DTexture(GL_TEXTURE0, m_pCurrentMirror->ptexture->gl_index);

		R_ValidateShader(m_pShader);

		glDrawArrays(GL_TRIANGLES, m_pCurrentMirror->start_vertex, m_pCurrentMirror->num_vertexes);

		// Set framecount for decals
		const brushmodel_t *pbrushmodel = pentity->pmodel->getBrushmodel();
		msurface_t *psurf = &pbrushmodel->psurfaces[pbrushmodel->firstmodelsurface];

		psurf->visframe = cls.framecount;
	}

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	return true;
}
