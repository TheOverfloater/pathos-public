/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_main.h"
#include "cl_main.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_fbo.h"
#include "cvar.h"
#include "r_dlights.h"
#include "cache_model.h"
#include "r_fbo.h"
#include "bspv30.h"
#include "console.h"
#include "r_basic_vertex.h"
#include "r_glextf.h"
#include "system.h"
#include "texturemanager.h"
#include "cl_utils.h"
#include "enginestate.h"
#include "common.h"
#include "r_bsp.h"
#include "r_common.h"
#include "r_vbm.h"
#include "modelcache.h"
#include "r_sprites.h"
#include "tga.h"
#include "file.h"

// Number of default lightstyles
const Uint32 CDynamicLightManager::NUM_DL_DEFAULT_STYLES = 12;
// Default lightstyle framerate
const Char CDynamicLightManager::DEFAULT_LIGHTSTYLE_FRAMERATE = 10;
// Minimum shadowmap size
const Uint32 CDynamicLightManager::SHADOWMAP_MIN_SIZE = 128;
// Maximum lightstyle string length
const Uint32 CDynamicLightManager::MAX_STYLESTRING = 64;
// Time until an unused shadowmap is freed
const Float CDynamicLightManager::SHADOWMAP_RELEASE_DELAY = 15;

// Class object
CDynamicLightManager gDynamicLights;

//====================================
//
//====================================
CDynamicLightManager::CDynamicLightManager( void ):
	m_pVSMShader(nullptr),
	m_pVSMVBO(nullptr),
	m_shadowmapSize(0),
	m_cubeShadowmapSize(0),
	m_pCvarShadowmapSize(nullptr),
	m_pCvarCubeShadowmapSize(nullptr)
{
}

//====================================
//
//====================================
CDynamicLightManager::~CDynamicLightManager( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CDynamicLightManager::Init( void )
{
	// init cvars
	m_pCvarShadowmapSize = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_shadowmap_proj_size", "512", "Controls resolution of projected light shadows.", R_CheckShadowmapSizeCvarCallBack );
	m_pCvarCubeShadowmapSize = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_shadowmap_cube_size", "256", "Controls resolution of point light shadows.", R_CheckShadowmapSizeCvarCallBack );

	return true;
}

//====================================
//
//====================================
bool CDynamicLightManager::InitGL( void )
{
	// If not active, don't allocate FBOs
	if(ens.gamestate == GAME_INACTIVE)
		return true;

	// Set shadowmap sizes
	m_shadowmapSize = m_pCvarShadowmapSize->GetValue();
	m_cubeShadowmapSize = m_pCvarCubeShadowmapSize->GetValue();

	// Initialize static FBOs
	if(!InitFBOs())
		return false;

	// load shader
	if(!m_pVSMShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pVSMShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "vsm_blur.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pVSMShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pVSMShader->GetError());
			return false;
		}

		m_vsmAttribs.a_origin = m_pVSMShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, origin));
		m_vsmAttribs.a_texcoord = m_pVSMShader->InitAttribute("in_texcoord", 4, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, origin));

		if(!R_CheckShaderVertexAttribute(m_vsmAttribs.a_origin, "in_position", m_pVSMShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_vsmAttribs.a_texcoord, "in_texcoord", m_pVSMShader, Sys_ErrorPopup))
			return false;

		m_vsmAttribs.d_type = m_pVSMShader->GetDeterminatorIndex("type");

		if(!R_CheckShaderDeterminator(m_vsmAttribs.d_type, "type", m_pVSMShader, Sys_ErrorPopup))
			return false;

		m_vsmAttribs.u_modelview = m_pVSMShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_vsmAttribs.u_projection = m_pVSMShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);

		m_vsmAttribs.u_size = m_pVSMShader->InitUniform("size", CGLSLShader::UNIFORM_FLOAT1);
		m_vsmAttribs.u_texture = m_pVSMShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_vsmAttribs.u_modelview, "modelview", m_pVSMShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_vsmAttribs.u_projection, "projection", m_pVSMShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_vsmAttribs.u_size, "size", m_pVSMShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_vsmAttribs.u_texture, "texture0", m_pVSMShader, Sys_ErrorPopup))
			return false;
	}

	// create VBO
	if(!m_pVSMVBO)
	{
		// create VBO
		basic_vertex_t *pverts = new basic_vertex_t[6];

		pverts[0].origin[0] = 0; pverts[0].origin[1] = 1; 
		pverts[0].origin[2] = -1; pverts[0].origin[3] = 1;
		pverts[0].texcoords[0] = 0; pverts[0].texcoords[1] = 0;

		pverts[1].origin[0] = 0; pverts[1].origin[1] = 0; 
		pverts[1].origin[2] = -1; pverts[1].origin[3] = 1;
		pverts[1].texcoords[0] = 0; pverts[1].texcoords[1] = rns.screenheight;

		pverts[2].origin[0] = 1; pverts[2].origin[1] = 0; 
		pverts[2].origin[2] = -1; pverts[2].origin[3] = 1;
		pverts[2].texcoords[0] = rns.screenwidth; pverts[2].texcoords[1] = rns.screenheight;

		pverts[3].origin[0] = 0; pverts[3].origin[1] = 1; 
		pverts[3].origin[2] = -1; pverts[3].origin[3] = 1;
		pverts[3].texcoords[0] = 0; pverts[3].texcoords[1] = 0;

		pverts[4].origin[0] = 1; pverts[4].origin[1] = 0; 
		pverts[4].origin[2] = -1; pverts[4].origin[3] = 1;
		pverts[4].texcoords[0] = rns.screenwidth; pverts[4].texcoords[1] = rns.screenheight;

		pverts[5].origin[0] = 1; pverts[5].origin[1] = 1; 
		pverts[5].origin[2] = -1; pverts[5].origin[3] = 1;
		pverts[5].texcoords[0] = rns.screenwidth; pverts[5].texcoords[1] = 0;

		m_pVSMVBO = new CVBO(gGLExtF, pverts, sizeof(basic_vertex_t)*6, nullptr, 0);
		m_pVSMShader->SetVBO(m_pVSMVBO);
		delete[] pverts;
	}

	return true;
}

//====================================
//
//====================================
void CDynamicLightManager::ClearGL( void )
{
	DeleteFBOs();

	if(m_pVSMShader)
	{
		delete m_pVSMShader;
		m_pVSMShader = nullptr;
	}

	if(m_pVSMVBO)
	{
		delete m_pVSMVBO;
		m_pVSMVBO = nullptr;
	}

	// Clear dynlight list
	if(!m_dlightsList.empty())
	{
		m_dlightsList.begin();
		while(!m_dlightsList.end())
		{
			delete m_dlightsList.get();
			m_dlightsList.next();
		}

		m_dlightsList.clear();
	}

	// Clear any entities that point to shadowmaps
	for(Int32 i = 0; i < cls.numentities; i++)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(i);
		if(pentity->pextradata && pentity->pextradata->pshadowmap)
			pentity->pextradata->pshadowmap = nullptr;
	}
}

//====================================
//
//====================================
void CDynamicLightManager::Shutdown( void )
{
	ClearGL();
	ClearGame();
}

//====================================
//
//====================================
bool CDynamicLightManager::InitGame( void )
{
	// Set default styles
	SetDefaultStyles();

	// Allocate FBOs
	if(!InitGL())
		return false;

	return true;
}

//====================================
//
//====================================
void CDynamicLightManager::SetDefaultStyles( void )
{
	// 0 normal
	AddLightStyle(LS_NORMAL, DEFAULT_LIGHTSTYLE_FRAMERATE, false, "m");
	
	// 1 FLICKER (first variety)
	AddLightStyle(LS_FLICKER_A, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	AddLightStyle(LS_SLOW_STRONG_PULSE, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	AddLightStyle(LS_CANDLE_A, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	AddLightStyle(LS_FAST_STROBE, DEFAULT_LIGHTSTYLE_FRAMERATE, false, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	AddLightStyle(LS_GENTLE_PULSE, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	AddLightStyle(LS_FLICKER_B, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	AddLightStyle(LS_CANDLE_B, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	AddLightStyle(LS_CANDLE_C, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	AddLightStyle(LS_SLOW_STROBE, DEFAULT_LIGHTSTYLE_FRAMERATE, false, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	AddLightStyle(LS_FLUORESCENT_FLICKER, DEFAULT_LIGHTSTYLE_FRAMERATE, false, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	AddLightStyle(LS_SLOW_PULSE_NOBLACK, DEFAULT_LIGHTSTYLE_FRAMERATE, true, "abcdefghijklmnopqrrqponmlkjihgfedcba");
}

//====================================
//
//====================================
void CDynamicLightManager::ResetStyles( void )
{
	if(m_lightStyles.empty())
		return;
		
	m_lightStyles.clear();
}

//====================================
//
//====================================
void CDynamicLightManager::AddCustomLightStyle( Uint32 index, Int32 framerate, bool interpolate, const Char* pstring )
{
	if(index < NUM_DL_DEFAULT_STYLES)
	{
		Con_Printf("%s - Invalid lightstyle index %d.\n", __FUNCTION__, index);
		return;
	}

	AddLightStyle(index, framerate, interpolate, pstring);
}

//====================================
//
//====================================
void CDynamicLightManager::AddLightStyle( Uint32 index, Int32 framerate, bool interpolate, const Char* pstring )
{
	Uint32 length = qstrlen(pstring);
	if(length+1 >= MAX_STYLESTRING)
	{
		Con_Printf("Error: Lightstyle with index %d is too long.\n", index);
		return;
	}

	if(m_lightStyles.size() <= index)
		m_lightStyles.resize(index+1);

	lightstyle_t& style = m_lightStyles[index];

	style.map.resize(length+1);
	
	for(Uint32 i = 0; i < length; i++)
		style.map[i] = pstring[i];

	style.map[length] = '\0';

	style.length = length;
	style.framerate = framerate;
	style.interp = interpolate;

	if(!style.framerate)
		style.framerate = DEFAULT_LIGHTSTYLE_FRAMERATE;
}

//====================================
//
//====================================
void CDynamicLightManager::AnimateStyles( void )
{
	for(Uint32 i = 0; i < m_lightStyles.size(); i++)
	{
		lightstyle_t& style = m_lightStyles[i];
		if(!style.length)
		{
			style.value = 1.0;
			continue;
		}

		if(style.interp)
		{
			Float frame = (rns.time*style.framerate);
			Float interp = frame - floor(frame);

			Int32 i1 = ((Int32)frame) % style.length;
			Int32 i2 = (((Int32)frame) + 1) % style.length;

			Int32 v1 = (style.map[i1] - 'a')*22;
			Int32 v2 = (style.map[i2] - 'a')*22;

			style.value = (((Float)v1)*(1.0-interp)) + (((Float)v2)*interp);
			style.value = style.value / 256.0f;
		}
		else
		{
			Float frame = (rns.time*style.framerate);
			Int32 i1 = ((Int32)frame) % style.length;
			Int32 v = (style.map[i1] - 'a')*22;

			style.value = ((Float)v)/256.0f;
		}
	}
}

//====================================
//
//====================================
void CDynamicLightManager::ApplyLightStyle( cl_dlight_t* dl, Vector& color )
{
	if(dl->lightstyle == 0)
		return;

	if(dl->lightstyle < 1 || dl->lightstyle >= m_lightStyles.size())
	{
		Con_Printf("Warning: Dynamic light at %.0f %.0f %.0f with invalid style index %d.\n", dl->origin.x, dl->origin.y, dl->origin.z, dl->lightstyle);
		return;
	}

	assert(dl->lightstyle < m_lightStyles.size());
	const lightstyle_t& style = m_lightStyles[dl->lightstyle];
	Math::VectorScale(color, style.value, color);
}

//====================================
//
//====================================
bool CDynamicLightManager::CheckFBOs( void )
{
	if(!m_cubemapPoolList.empty())
	{
		m_cubemapPoolList.begin();
		while(!m_cubemapPoolList.end())
		{
			shadowmap_t* psm = m_cubemapPoolList.get();
			if(!psm->used && psm->freetime != -1 && (rns.time - psm->freetime) > SHADOWMAP_RELEASE_DELAY)
			{
				m_cubemapPoolList.remove(m_cubemapPoolList.get_link());
				if(psm->pfbo)
				{
					gGLExtF.glDeleteFramebuffers(1, &psm->pfbo->fboid);
					delete psm->pfbo;
				}

				Con_DPrintf("%s - Released cubemap shadowmap.\n", __FUNCTION__);
				delete psm;
			}

			m_cubemapPoolList.next();
		}
	}

	if(!m_projectivePoolList.empty())
	{
		m_projectivePoolList.begin();
		while(!m_projectivePoolList.end())
		{
			shadowmap_t* psm = m_projectivePoolList.get();
			if(!psm->used && psm->freetime != -1 && (rns.time - psm->freetime) > SHADOWMAP_RELEASE_DELAY)
			{
				m_projectivePoolList.remove(m_projectivePoolList.get_link());
				if(psm->pfbo)
				{
					gGLExtF.glDeleteFramebuffers(1, &psm->pfbo->fboid);
					delete psm->pfbo;
				}

				Con_DPrintf("%s - Released projective shadowmap.\n", __FUNCTION__);
				delete psm;
			}

			m_projectivePoolList.next();
		}
	}

	// Check if cvars were changed
	if(m_shadowmapSize == m_pCvarShadowmapSize->GetValue()
		&& m_cubeShadowmapSize == m_pCvarCubeShadowmapSize->GetValue())
		return true;

	// Delete existing FBOs
	DeleteFBOs();

	// Clear any entities that point to shadowmaps
	for(Int32 i = 0; i < cls.numentities; i++)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(i);
		if(pentity->pextradata && pentity->pextradata->pshadowmap)
			pentity->pextradata->pshadowmap = nullptr;
	}

	// Create new ones
	if(!InitGL())
	{
		Sys_ErrorPopup("%s - Failed to create FBOs.\n", __FUNCTION__);
		return false;
	}

	// Clear dynlight list also
	if(!m_dlightsList.empty())
	{
		m_dlightsList.begin();
		while(!m_dlightsList.end())
		{
			delete m_dlightsList.get();
			m_dlightsList.next();
		}

		m_dlightsList.clear();
	}

	return true;
}

//====================================
//
//====================================
bool CDynamicLightManager::InitFBOs( void )
{
	if(!rns.fboused)
		return true;
	
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	//
	// Set up the ones for projective lights
	//

	// Set up blur fbo
	m_blurFBO.ptexture1 = pTextureManager->GenTextureIndex(RS_WINDOW_LEVEL);
	glBindTexture(GL_TEXTURE_2D, m_blurFBO.ptexture1->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, GetShadowmapSize(), GetShadowmapSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gGLExtF.glGenFramebuffers(1, &m_blurFBO.fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO.fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blurFBO.ptexture1->gl_index, 0);

	GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, (Int32)glGetError());
		return false;
	}

	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Set up main rendering target
	m_renderFBO.ptexture1 = pTextureManager->GenTextureIndex(RS_WINDOW_LEVEL);
	glBindTexture(GL_TEXTURE_2D, m_renderFBO.ptexture1->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, GetShadowmapSize(), GetShadowmapSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gGLExtF.glGenRenderbuffers(1, &m_renderFBO.rboid1);
	gGLExtF.glBindRenderbuffer(GL_RENDERBUFFER, m_renderFBO.rboid1);
	gGLExtF.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetShadowmapSize(), GetShadowmapSize());
	gGLExtF.glBindRenderbuffer(GL_RENDERBUFFER, 0);

	gGLExtF.glGenFramebuffers(1, &m_renderFBO.fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_renderFBO.fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderFBO.ptexture1->gl_index, 0);
	gGLExtF.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderFBO.rboid1);

	eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, (Int32)glGetError());
		return false;
	}

	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//
	// Create the FBOs for cubemaps
	//

	// Set up blur fbo
	m_cubeBlurFBO.ptexture1 = pTextureManager->GenTextureIndex(RS_WINDOW_LEVEL);
	glBindTexture(GL_TEXTURE_2D, m_cubeBlurFBO.ptexture1->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, GetCubeShadowmapSize(), GetCubeShadowmapSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gGLExtF.glGenFramebuffers(1, &m_cubeBlurFBO.fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_cubeBlurFBO.fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_cubeBlurFBO.ptexture1->gl_index, 0);

	eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, (Int32)glGetError());
		return false;
	}

	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set up main rendering target
	m_cubeRenderFBO.ptexture1 = pTextureManager->GenTextureIndex(RS_WINDOW_LEVEL);
	glBindTexture(GL_TEXTURE_2D, m_cubeRenderFBO.ptexture1->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, GetCubeShadowmapSize(), GetCubeShadowmapSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gGLExtF.glGenRenderbuffers(1, &m_cubeRenderFBO.rboid1);
	gGLExtF.glBindRenderbuffer(GL_RENDERBUFFER, m_cubeRenderFBO.rboid1);
	gGLExtF.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GetCubeShadowmapSize(), GetCubeShadowmapSize());
	gGLExtF.glBindRenderbuffer(GL_RENDERBUFFER, 0);

	gGLExtF.glGenFramebuffers(1, &m_cubeRenderFBO.fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_cubeRenderFBO.fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_cubeRenderFBO.ptexture1->gl_index, 0);
	gGLExtF.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_cubeRenderFBO.rboid1);

	eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, (Int32)glGetError());
		return false;
	}

	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

//====================================
//
//====================================
shadowmap_t *CDynamicLightManager::AllocProjectiveShadowMap( void )
{
	shadowmap_t* pshadowmap = nullptr;

	// Check if we have something in the cache
	if(!m_projectivePoolList.empty())
	{
		m_projectivePoolList.begin();
		while(!m_projectivePoolList.end())
		{
			pshadowmap = m_projectivePoolList.get();
			if(!pshadowmap->used)
			{
				pshadowmap->used = true;
				return pshadowmap;
			}

			m_projectivePoolList.next();
		}
	}

	// No free slot found, allocate a new one
	pshadowmap = new shadowmap_t();
	pshadowmap->projective = true;
	pshadowmap->used = true;

	// Fill up the FBO
	if(!CreateProjectiveFBO(*pshadowmap))
		return nullptr;

	m_projectivePoolList.add(pshadowmap);
	return pshadowmap;
}

//====================================
//
//====================================
shadowmap_t *CDynamicLightManager::AllocCubemapShadowMap( void )
{
	shadowmap_t* pshadowmap = nullptr;

	// Check if we have something in the cache
	if(!m_cubemapPoolList.empty())
	{
		m_cubemapPoolList.begin();
		while(!m_cubemapPoolList.end())
		{
			pshadowmap = m_cubemapPoolList.get();
			if(!pshadowmap->used)
			{
				pshadowmap->used = true;
				return pshadowmap;
			}

			m_cubemapPoolList.next();
		}
	}

	// No free slot found, allocate a new one
	pshadowmap = new shadowmap_t();
	pshadowmap->projective = false;
	pshadowmap->used = true;

	// Fill up the FBO
	if(!CreateCubemapFBO(*pshadowmap))
		return nullptr;

	m_cubemapPoolList.add(pshadowmap);
	return pshadowmap;
}

//====================================
//
//====================================
bool CDynamicLightManager::CreateProjectiveFBO( shadowmap_t& shadowmap ) const
{
	if(!shadowmap.pfbo)
		shadowmap.pfbo = new fbobind_t();

	if(!shadowmap.pfbo->ptexture1)
		shadowmap.pfbo->ptexture1 = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

	glBindTexture(GL_TEXTURE_2D, shadowmap.pfbo->ptexture1->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, GetShadowmapSize(), GetShadowmapSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	gGLExtF.glGenFramebuffers(1, &shadowmap.pfbo->fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, shadowmap.pfbo->fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowmap.pfbo->ptexture1->gl_index, 0);
		
	GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, (Int32)glGetError());
		return false;
	}
		
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

//====================================
//
//====================================
bool CDynamicLightManager::CreateCubemapFBO( shadowmap_t& shadowmap ) const
{
	// Create the cubemap
	if(!shadowmap.pfbo)
		shadowmap.pfbo = new fbobind_t();

	if(!shadowmap.pfbo->ptexture1)
		shadowmap.pfbo->ptexture1 = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmap.pfbo->ptexture1->gl_index);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);

	for (Int32 j = 0; j < 6; j++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGBA16, GetCubeShadowmapSize(), GetCubeShadowmapSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	gGLExtF.glGenFramebuffers(1, &shadowmap.pfbo->fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, shadowmap.pfbo->fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, shadowmap.pfbo->ptexture1->gl_index, 0);

	GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, (Int32)glGetError());
		return false;
	}

	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return true;
}

//====================================
//
//====================================
void CDynamicLightManager::ClearProjectiveShadowMap( shadowmap_t *psm )
{
	psm->freetime = rns.time;
	psm->used = false;
}

//====================================
//
//====================================
void CDynamicLightManager::ClearCubemapShadowMap( shadowmap_t *psm )
{
	psm->freetime = rns.time;
	psm->used = false;
}

//====================================
//
//====================================
void CDynamicLightManager::DeleteFBOs( void )
{
	if(m_blurFBO.fboid)
		gGLExtF.glDeleteFramebuffers(1, &m_blurFBO.fboid);

	if(m_renderFBO.fboid)
	{
		gGLExtF.glDeleteFramebuffers(1, &m_renderFBO.fboid);
		gGLExtF.glDeleteRenderbuffers(1, &m_renderFBO.rboid1);
	}

	if(m_cubeBlurFBO.fboid)
		gGLExtF.glDeleteFramebuffers(1, &m_cubeBlurFBO.fboid);

	if(m_cubeRenderFBO.fboid)
	{
		gGLExtF.glDeleteFramebuffers(1, &m_cubeRenderFBO.fboid);
		gGLExtF.glDeleteRenderbuffers(1, &m_cubeRenderFBO.rboid1);
	}

	ClearShadowMaps();
}

//====================================
//
//====================================
void CDynamicLightManager::ClearShadowMaps( void )
{
	if(!m_cubemapPoolList.empty())
	{
		m_cubemapPoolList.begin();
		while(!m_cubemapPoolList.end())
		{
			shadowmap_t* psm = m_cubemapPoolList.get();
			m_cubemapPoolList.remove(m_cubemapPoolList.get_link());
			if(psm->pfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &psm->pfbo->fboid);
				delete psm->pfbo;
			}

			delete psm;
			m_cubemapPoolList.next();
		}

		m_cubemapPoolList.clear();
	}

	if(!m_projectivePoolList.empty())
	{
		m_projectivePoolList.begin();
		while(!m_projectivePoolList.end())
		{
			shadowmap_t* psm = m_projectivePoolList.get();
			m_projectivePoolList.remove(m_projectivePoolList.get_link());
			if(psm->pfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &psm->pfbo->fboid);
				delete psm->pfbo;
			}

			delete psm;
			m_projectivePoolList.next();
		}

		m_projectivePoolList.clear();
	}
}

//====================================
//
//====================================
void CDynamicLightManager::SetVIS( void )
{
	if( g_pCvarShadows->GetValue() < 1 )
		return;
	
	// Mark leaves
	R_MarkLeaves(rns.view.params.v_origin);

	m_dlightsList.begin();
	while(!m_dlightsList.end())
	{
		cl_dlight_t *plight = m_dlightsList.get();

		if(!DL_CanShadow(plight) || plight->key == CL_GetLocalPlayer()->entindex 
			|| plight->isstatic && plight->pstaticinfo->drawframe != rns.framecount_main)
		{
			m_dlightsList.next();
			continue;
		}

		// Make sure the spotlight sees it's polygons
		const mleaf_t *pleaf = Mod_PointInLeaf(plight->origin, *ens.pworld);
		R_ForceMarkLeaves(pleaf, rns.view.pviewleaf->visframe);

		// Advance
		m_dlightsList.next();
	}
}

//====================================
//
//====================================
void CDynamicLightManager::UpdateStaticLights( void )
{
	// Loop through dynlights
	m_dlightsList.begin();
	while(!m_dlightsList.end())
	{
		CLinkedList<cl_dlight_t*>::link_t *plink = m_dlightsList.get_link();
		cl_dlight_t *dl = plink->_val;

		if(!dl->isstatic)
		{
			m_dlightsList.next();
			continue;
		}

		// Build mins/maxs
		Vector mins, maxs;
		for(Int32 i = 0; i < 3; i++)
		{
			mins[i] = dl->origin[i] - dl->radius;
			maxs[i] = dl->origin[i] + dl->radius;
		}

		// Check frustum
		R_SetFrustum(rns.view.frustum, rns.view.v_origin, rns.view.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);
		if(rns.view.frustum.CullBBox(mins, maxs))
		{
			m_dlightsList.next();
			continue;
		}

		if(ShouldRedrawStaticMap(dl))
		{
			// Redraw the shadow map for this frame
			dl->pstaticinfo->drawframe = rns.framecount_main;
		}

		// Reset this
		if(dl->cone_size && dl->pshadowmap && dl->pshadowmap->reset)
			dl->pshadowmap->reset = false;
		else if(dl->psmcubemap && dl->psmcubemap->reset)
			dl->psmcubemap->reset = false;

		m_dlightsList.next();
	}
}

//====================================
//
//====================================
bool CDynamicLightManager::DrawPasses( void )
{
	if(m_dlightsList.empty())
		return true;

	if(g_pCvarShadows->GetValue() < 1)
		return true;

	if(g_pCvarDynamicLights->GetValue() < 1)
		return true;

	// Update static lights and set vis too
	UpdateStaticLights();
	SetVIS();

	// Animate lightstyles
	AnimateStyles();

	// Holds our main view frustum params
	CFrustum mainFrustum;
	R_SetFrustum(mainFrustum, rns.view.params.v_origin, rns.view.params.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);

	// Error tracking
	bool result = true;

	m_dlightsList.begin();
	while(!m_dlightsList.end())
	{
		cl_dlight_t* dl = m_dlightsList.get();

		if (!DL_CanShadow(dl) || dl->isstatic && dl->pstaticinfo->drawframe != rns.framecount_main)
		{
			m_dlightsList.next();
			continue;
		}

		if(!dl->nomaincull)
		{
			// Build mins/maxs
			Vector mins, maxs;
			for(Int32 i = 0; i < 3; i++)
			{
				mins[i] = dl->origin[i] - dl->radius;
				maxs[i] = dl->origin[i] + dl->radius;
			}

			if(!DL_IsLightVisible(mainFrustum, mins, maxs, dl))
			{
				m_dlightsList.next();
				continue;
			}
		}

		Int32 numentities = 0;
		cl_entity_t** pentityarray = nullptr;

		if(dl->isstatic)
		{
			pentityarray = dl->pstaticinfo->pvisents;
			numentities = dl->pstaticinfo->numvisents;
		}
		else
		{
			pentityarray = &rns.objects.pvisents[0];
			numentities = rns.objects.numvisents;
		}

		if(dl->cone_size)
		{
			glViewport(GL_ZERO, GL_ZERO, GetShadowmapSize(), GetShadowmapSize());
			if(!DrawProjectivePass(dl, pentityarray, numentities))
			{
				result = false;
				break;
			}
		}
		else
		{
			glViewport(GL_ZERO, GL_ZERO, GetCubeShadowmapSize(), GetCubeShadowmapSize());
			if(!DrawCubemapPass(dl, Vector(0, -90, 180), 0, pentityarray, numentities) ||
				!DrawCubemapPass(dl, Vector(0, 90, 180), 1, pentityarray, numentities) ||
				!DrawCubemapPass(dl, Vector(-90, 0, 0), 2, pentityarray, numentities) ||
				!DrawCubemapPass(dl, Vector(90, 0, 0), 3, pentityarray, numentities) ||
				!DrawCubemapPass(dl, Vector(0, 180, 180), 4, pentityarray, numentities) ||
				!DrawCubemapPass(dl, Vector(0, 0, 180), 5, pentityarray, numentities))
			{
				result = false;
				break;
			}
		}

		m_dlightsList.next();
	}

	if(rns.fboused)
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

	glViewport(GL_ZERO, GL_ZERO, rns.screenwidth, rns.screenheight);

	// Clear any binds
	R_ClearBinds();

	return result;
}

//====================================
//
//====================================
bool CDynamicLightManager::DrawProjectivePass( cl_dlight_t *dl, cl_entity_t** pvisents, Int32 numentities )
{
	rns.view.frustum.SetFrustum(dl->angles, dl->origin, dl->cone_size, dl->radius);
	Math::VectorCopy(dl->angles, rns.view.v_angles);
	Math::VectorCopy(dl->origin, rns.view.v_origin);

	if(rns.fboused)
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_renderFBO.fboid);

	//Completely clear everything
	glClearColor(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_FRONT);
	glDisable(GL_BLEND);

	Float flSize = tan((M_PI/360) * dl->cone_size);
	rns.view.projection.PushMatrix();
	rns.view.projection.LoadIdentity();
	rns.view.projection.SetFrustum(-flSize, flSize, -flSize, flSize, 1, dl->radius);

	// Asscawks
	Vector vforward, vtarget;
	Vector vangles = dl->angles;
	Common::FixVector(vangles);

	Math::AngleVectors(vangles, &vforward, nullptr, nullptr);
	Math::VectorMA(dl->origin, dl->radius, vforward, vtarget);

	rns.view.modelview.LoadIdentity();
	rns.view.modelview.LookAt(dl->origin[0], dl->origin[1], dl->origin[2], vtarget[0], vtarget[1], vtarget[2], 0, 0, Common::IsPitchReversed(dl->angles[PITCH]) ? -1 : 1);

	// Advance frame count here
	rns.framecount++;

	// Render everything
	rns.visframe = rns.view.pviewleaf->visframe;

	if(!gBSPRenderer.DrawVSM(dl, pvisents, numentities))
		return false;

	if(!gVBMRenderer.DrawVSM(dl, pvisents, numentities))
		return false;

	// Draw any ladders on client
	if(!cls.dllfuncs.pfnDrawLaddersForVSM(dl))
		return false;

	// Draw any view objects for shadows
	entindex_t localPlayerIndex = CL_GetLocalPlayer()->entindex;
	if(!dl->isStatic() && SDL_abs(dl->key) != localPlayerIndex)
	{
		if(!cls.dllfuncs.pfnDrawViewObjectsForVSM(dl))
			return false;
	}

	// blur the map now
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	m_pVSMVBO->Bind();
	if(!m_pVSMShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
		return false;
	}

	rns.view.projection.LoadIdentity();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

	m_pVSMShader->SetUniformMatrix4fv(m_vsmAttribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pVSMShader->SetUniformMatrix4fv(m_vsmAttribs.u_projection, rns.view.projection.GetMatrix());

	m_pVSMShader->SetUniform1f(m_vsmAttribs.u_size, GetShadowmapSize());
	m_pVSMShader->SetUniform1i(m_vsmAttribs.u_texture, 0);

	m_pVSMShader->EnableAttribute(m_vsmAttribs.a_origin);
	m_pVSMShader->EnableAttribute(m_vsmAttribs.a_texcoord);

	if(g_pCvarGaussianBlur->GetValue() > 0)
	{
		// blur vertically
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO.fboid);

		if(!m_pVSMShader->SetDeterminator(m_vsmAttribs.d_type, VSM_SHADER_VBLUR))
		{
			Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
			m_pVSMShader->DisableShader();
			m_pVSMVBO->UnBind();
			return false;
		}

		R_Bind2DTexture(GL_TEXTURE0, m_renderFBO.ptexture1->gl_index);

		glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		R_ValidateShader(m_pVSMShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// blur vertically
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, dl->pshadowmap->pfbo->fboid);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!m_pVSMShader->SetDeterminator(m_vsmAttribs.d_type, VSM_SHADER_HBLUR))
		{
			Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
			m_pVSMShader->DisableShader();
			m_pVSMVBO->UnBind();
			return false;
		}

		R_Bind2DTexture(GL_TEXTURE0, m_blurFBO.ptexture1->gl_index);
		R_ValidateShader(m_pVSMShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	else
	{
		// just copy
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, dl->pshadowmap->pfbo->fboid);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!m_pVSMShader->SetDeterminator(m_vsmAttribs.d_type, VSM_SHADER_COPY))
		{
			Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
			m_pVSMShader->DisableShader();
			m_pVSMVBO->UnBind();
			return false;
		}

		R_Bind2DTexture(GL_TEXTURE0, m_renderFBO.ptexture1->gl_index);
		R_ValidateShader(m_pVSMShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	rns.view.projection.PopMatrix();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	m_pVSMShader->DisableShader();
	m_pVSMVBO->UnBind();

	return true;
}

//====================================
//
//====================================
bool CDynamicLightManager::DrawCubemapPass( cl_dlight_t *dl, Vector vangles, Int32 index, cl_entity_t** pvisents, Int32 numentities )
{
	rns.view.frustum.SetFrustum(vangles, dl->origin, 90, dl->radius);
	Math::VectorCopy(dl->origin, rns.view.v_origin);
	Math::VectorCopy(vangles, rns.view.v_angles);
	
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_cubeRenderFBO.fboid);

	//Completely clear everything
	glClearColor(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_FRONT);
	glDisable(GL_BLEND);

	Float flSize = tan((M_PI/360) * 90);
	rns.view.projection.PushMatrix();
	rns.view.projection.LoadIdentity();
	rns.view.projection.SetFrustum(-flSize, flSize, -flSize, flSize, 1, dl->radius);

	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Rotate(-90,  1, 0, 0);// put X going down
	rns.view.modelview.Rotate(90,  0, 0, 1); // put Z going up
	rns.view.modelview.Rotate(-vangles[2], 1, 0, 0);
	rns.view.modelview.Rotate(-vangles[0], 0, 1, 0);
	rns.view.modelview.Rotate(-vangles[1], 0, 0, 1);
	rns.view.modelview.Translate(-dl->origin[0], -dl->origin[1], -dl->origin[2]);

	// Advance frame count here
	rns.framecount++;

	// Render everything
	rns.visframe = rns.view.pviewleaf->visframe;

	// draw world
	if(!gBSPRenderer.DrawVSM(dl, pvisents, numentities))
		return false;

	// draw models
	if(!gVBMRenderer.DrawVSM(dl, pvisents, numentities))
		return false;

	// Draw any ladders on client
	if(!cls.dllfuncs.pfnDrawLaddersForVSM(dl))
		return false;

	// Draw any view objects for shadows
	if(!cls.dllfuncs.pfnDrawViewObjectsForVSM(dl))
		return false;

	// blur the map now
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	m_pVSMVBO->Bind();
	if(!m_pVSMShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
		return false;
	}

	rns.view.projection.LoadIdentity();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

	m_pVSMShader->SetUniformMatrix4fv(m_vsmAttribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pVSMShader->SetUniformMatrix4fv(m_vsmAttribs.u_projection, rns.view.projection.GetMatrix());

	m_pVSMShader->SetUniform1f(m_vsmAttribs.u_size, GetCubeShadowmapSize());
	m_pVSMShader->SetUniform1i(m_vsmAttribs.u_texture, 0);

	m_pVSMShader->EnableAttribute(m_vsmAttribs.a_origin);
	m_pVSMShader->EnableAttribute(m_vsmAttribs.a_texcoord);

	if(g_pCvarGaussianBlur->GetValue() > 0)
	{
		// blur vertically
		if(!m_pVSMShader->SetDeterminator(m_vsmAttribs.d_type, VSM_SHADER_VBLUR))
		{
			Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
			return false;
		}

		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, m_cubeBlurFBO.fboid);
		R_Bind2DTexture(GL_TEXTURE0, m_cubeRenderFBO.ptexture1->gl_index);

		glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		R_ValidateShader(m_pVSMShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		// blur horizontally

		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, dl->psmcubemap->pfbo->fboid);
		gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, dl->psmcubemap->pfbo->ptexture1->gl_index, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!m_pVSMShader->SetDeterminator(m_vsmAttribs.d_type, VSM_SHADER_HBLUR))
		{
			Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
			return false;
		}

		R_Bind2DTexture(GL_TEXTURE0, m_cubeBlurFBO.ptexture1->gl_index);
		R_ValidateShader(m_pVSMShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	else
	{
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, dl->psmcubemap->pfbo->fboid);
		gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, dl->psmcubemap->pfbo->ptexture1->gl_index, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!m_pVSMShader->SetDeterminator(m_vsmAttribs.d_type, VSM_SHADER_COPY))
		{
			Sys_ErrorPopup("Shader error: %s.", m_pVSMShader->GetError());
			return false;
		}

		R_Bind2DTexture(GL_TEXTURE0, m_cubeRenderFBO.ptexture1->gl_index);
		R_ValidateShader(m_pVSMShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	rns.view.projection.PopMatrix();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	m_pVSMShader->DisableShader();
	m_pVSMVBO->UnBind();
	
	return true;
}

//====================================
//
//====================================
void CDynamicLightManager::FreeDynamicLight( cl_dlight_t* pdlight, bool ignoreStatic )
{
	if(!pdlight->isstatic || ignoreStatic)
	{
		if(pdlight->pshadowmap)
			ClearProjectiveShadowMap(pdlight->pshadowmap);

		if(pdlight->psmcubemap)
			ClearCubemapShadowMap(pdlight->psmcubemap);
	}

	if(pdlight->pstaticinfo)
		delete pdlight->pstaticinfo;

	if(pdlight->pfrustum)
		delete pdlight->pfrustum;

	delete pdlight;
}

//====================================
//
//====================================
bool CDynamicLightManager::Update( void )
{
	// Check for any FBO updates
	if(!CheckFBOs())
		return false;

	if(m_dlightsList.empty())
		return true;

	m_dlightsList.begin();
	while(!m_dlightsList.end())
	{
		cl_dlight_t* pdl = m_dlightsList.get();

		if(pdl->followentity)
		{
			cl_entity_t* pentity = CL_GetEntityByIndex(pdl->key);
			if(!pentity || !pentity->pmodel)
			{
				FreeDynamicLight(pdl);

				m_dlightsList.remove(m_dlightsList.get_link());
				m_dlightsList.next();
				continue;
			}

			if(pdl->attachment == NO_POSITION || pentity->pmodel->type != MOD_VBM)
			{
				// Take just origin directly
				Vector mins, maxs;
				Math::VectorAdd(pentity->pmodel->mins, pentity->curstate.origin, mins);
				Math::VectorAdd(pentity->pmodel->maxs, pentity->curstate.origin, maxs);

				Vector center;
				Math::VectorAdd(mins, maxs, center);
				Math::VectorScale(center, 0.5, center);

				pdl->origin = center;
			}
			else
			{
				gVBMRenderer.UpdateAttachments(pentity);
				pdl->origin = pentity->getAttachment(pdl->attachment);
			}
		}

		// Decay lights
		if(!pdl->decay_delay || ((pdl->spawntime + pdl->decay_delay) < rns.time))
			pdl->radius -= rns.frametime*pdl->decay;

		// Update dlight frustum
		if(pdl->pfrustum && pdl->cone_size > 0)
			pdl->pfrustum->SetFrustum(pdl->angles, pdl->origin, pdl->cone_size, 
			Vector(pdl->radius, pdl->radius, pdl->radius).Length());

		if(( pdl->die == -1 
			&& pdl->lastframe == rns.framecount_main 
			|| pdl->die >= rns.time )
			&& pdl->radius > 0)
		{
			m_dlightsList.next();
			continue;
		}

		FreeDynamicLight(pdl);

		m_dlightsList.remove(m_dlightsList.get_link());
		m_dlightsList.next();
	}

	return true;
}

//====================================
//
//====================================
void CDynamicLightManager::ClearGame( void )
{
	ResetStyles();

	// Clear out dynlights
	if(!m_dlightsList.empty())
	{
		m_dlightsList.begin();
		while(!m_dlightsList.end())
		{
			FreeDynamicLight(m_dlightsList.get(), true);
			m_dlightsList.next();
		}

		m_dlightsList.clear();
	}

	ClearShadowMaps();
}

//====================================
//
//====================================
void CDynamicLightManager::SetupLight( cl_dlight_t* pdlight, bool spotlight, bool noshadow, cl_entity_t *pentity )
{
	entity_extrainfo_t* pextrainfo = CL_GetEntityExtraData(pentity);

	if(!noshadow)
	{
		if(!pextrainfo || !pextrainfo->pshadowmap)
		{
			// prevent cross-use
			if(spotlight)
			{
				if(pdlight->psmcubemap)
					ClearCubemapShadowMap(pdlight->psmcubemap);

				if(!pdlight->pshadowmap)
					pdlight->pshadowmap = AllocProjectiveShadowMap();
			}
			else
			{
				if(pdlight->pshadowmap)
					ClearProjectiveShadowMap(pdlight->pshadowmap);

				if(!pdlight->psmcubemap)
					pdlight->psmcubemap = AllocCubemapShadowMap();
			}
		}
		else
		{
			// Set the pointer if the entity
			if(spotlight)
				pdlight->pshadowmap = reinterpret_cast<shadowmap_t *>(pextrainfo->pshadowmap);
			else
				pdlight->psmcubemap = reinterpret_cast<shadowmap_t *>(pextrainfo->pshadowmap);
		}
	}
	else
	{
		if(pdlight->pshadowmap)
		{
			ClearProjectiveShadowMap(pdlight->pshadowmap);
			pdlight->pshadowmap = nullptr;
		}

		if(pdlight->psmcubemap)
		{
			ClearCubemapShadowMap(pdlight->psmcubemap);
			pdlight->psmcubemap = nullptr;
		}

		if(pdlight->isstatic)
			pdlight->isstatic = false;
	}

	if(spotlight)
	{
		if(!pdlight->pfrustum)
			pdlight->pfrustum = new CFrustum();
	}
	else
	{
		if(pdlight->pfrustum)
		{
			delete pdlight->pfrustum;
			pdlight->pfrustum = nullptr;
		}
	}
}

//====================================
//
//====================================
cl_dlight_t* CDynamicLightManager::AllocDynamicSpotlight( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity )
{
	// First see if the current list has a dlight with the key in it
	if(key)
	{
		cl_dlight_t* pdlight = GetLightByKey(key, subkey);
		if(pdlight)
		{
			// Reset these
			pdlight->cone_size = 0;
			pdlight->radius = 0;
			pdlight->decay = 0;
			pdlight->textureindex = 0;
			pdlight->noshadow = noshadow;
			pdlight->isstatic = isstatic;
			pdlight->nomaincull = false;
			pdlight->decay_delay = 0;

			if(!pdlight->isstatic && pdlight->pstaticinfo)
			{
				// Remove it
				delete pdlight->pstaticinfo;
				pdlight->pstaticinfo = nullptr;
			}
			else if(pdlight->isstatic && !pdlight->pstaticinfo)
			{
				pdlight->pstaticinfo = new dlight_staticinfo_t();
				pdlight->pstaticinfo->drawframe = -1; // Set initial value
			}

			// Set the dynlight info
			SetupLight(pdlight, true, noshadow, pentity);
			return pdlight;
		}
	}

	// then allocate a new one
	cl_dlight_t *pdlight = new cl_dlight_t();
	pdlight->key = key;
	pdlight->subkey = subkey;
	pdlight->isstatic = isstatic;
	pdlight->noshadow = noshadow;
	pdlight->nomaincull = false;
	pdlight->spawntime = cls.cl_time;
	pdlight->decay_delay = 0;

	// Link it
	m_dlightsList.add(pdlight);

	// Set the dynlight infos
	SetupLight(pdlight, true, noshadow, pentity);

	// Set pointers in entity
	if(pdlight->isstatic && pentity)
	{
		entity_extrainfo_t* pextrainfo = CL_GetEntityExtraData(pentity);
		pextrainfo->pshadowmap = pdlight->pshadowmap;
	}
	else
	{
		// Can't be static without an entity
		pdlight->isstatic = false;
	}

	// Allocate static dlight info if needed
	if(pdlight->isstatic && !pdlight->pstaticinfo)
	{
		pdlight->pstaticinfo = new dlight_staticinfo_t();
		pdlight->pstaticinfo->drawframe = -1; // Set initial value
	}

	return pdlight;
}

//====================================
//
//====================================
cl_dlight_t* CDynamicLightManager::AllocDynamicPointLight( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity )
{
	// First see if the current list has a dlight with the key in it
	if(key)
	{
		cl_dlight_t* pdlight = GetLightByKey(key, subkey);
		if(pdlight)
		{
			// Reset these
			pdlight->decay = 0;
			pdlight->cone_size = 0;
			pdlight->radius = 0;
			pdlight->textureindex = 0;
			pdlight->noshadow = noshadow;
			pdlight->isstatic = isstatic;
			pdlight->nomaincull = false;
			pdlight->decay_delay = 0;

			if(!pdlight->isstatic && pdlight->pstaticinfo)
			{
				// Remove it
				delete pdlight->pstaticinfo;
				pdlight->pstaticinfo = nullptr;
			}
			else if(pdlight->isstatic && !pdlight->pstaticinfo)
			{
				pdlight->pstaticinfo = new dlight_staticinfo_t();
				pdlight->pstaticinfo->drawframe = -1; // Set initial value
			}

			// Set the dynlight info
			SetupLight(pdlight, false, noshadow, pentity);
			return pdlight;
		}
	}

	// then allocate a new one
	cl_dlight_t *pdlight = new cl_dlight_t();
	pdlight->key = key;
	pdlight->subkey = subkey;
	pdlight->isstatic = isstatic;
	pdlight->noshadow = noshadow;
	pdlight->nomaincull = false;
	pdlight->spawntime = cls.cl_time;
	pdlight->decay_delay = 0;

	// Link it
	m_dlightsList.add(pdlight);

	// Set the dynlight infos
	SetupLight(pdlight, false, noshadow, pentity);

	// Set pointers in entity
	if(pdlight->isstatic && pentity)
	{
		entity_extrainfo_t* pextrainfo = CL_GetEntityExtraData(pentity);
		if(!pextrainfo->pshadowmap)
			pextrainfo->pshadowmap = pdlight->psmcubemap;
	}
	else
	{
		// Can't be static without an entity
		pdlight->isstatic = false;
	}

	// Allocate static dlight info if needed
	if(pdlight->isstatic && !pdlight->pstaticinfo)
	{
		pdlight->pstaticinfo = new dlight_staticinfo_t();
		pdlight->pstaticinfo->drawframe = -1; // Set initial value
	}

	return pdlight;
}

//====================================
//
//====================================
cl_dlight_t* CDynamicLightManager::GetLightByKey( Int32 key, Int32 subkey )
{
	m_dlightsList.begin();
	while(!m_dlightsList.end())
	{
		cl_dlight_t* pdlight = m_dlightsList.get();

		if(pdlight->key == key 
			&& pdlight->subkey == subkey)
			return pdlight;

		m_dlightsList.next();
	}

	return nullptr;
}

//====================================
//
//====================================
bool CDynamicLightManager::ShouldRedrawStaticMap( cl_dlight_t *dl )
{
	Vector lightmins, lightmaxs;
	for(Int32 i = 0; i < 3; i++)
	{
		lightmins[i] = dl->origin[i] - dl->radius;
		lightmaxs[i] = dl->origin[i] + dl->radius;
	}

	Int32 numAdded = 0;
	bool bRedraw = false;
	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		if(!rns.objects.pvisents[i]->pmodel)
			continue;

		cl_entity_t* pvisentity = rns.objects.pvisents[i];

		// Only static objects
		if(pvisentity->curstate.movetype != MOVETYPE_NONE
			&& !(pvisentity->curstate.effects & EF_STATICENTITY))
			continue;

		// No transparents
		if(R_IsEntityTransparent((*pvisentity)))
			continue;

		// No entities without models
		if(!pvisentity->pmodel)
			continue;

		// Skip tempents
		if(pvisentity->entindex < 1)
			continue;

		// Only vbm and brush models
		if(pvisentity->pmodel->type != MOD_VBM 
			&& pvisentity->pmodel->type != MOD_BRUSH)
			continue;
		
		// Check bbox
		Vector mins, maxs;
		if(pvisentity->pmodel->type == MOD_BRUSH)
		{
			Math::VectorAdd(pvisentity->pmodel->mins, pvisentity->curstate.origin, mins);
			Math::VectorAdd(pvisentity->pmodel->maxs, pvisentity->curstate.origin, maxs);
		}
		else
		{
			entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pvisentity);

			Math::VectorCopy(pinfo->absmin, mins);
			Math::VectorCopy(pinfo->absmax, maxs);
		}

		if(dl->cone_size && dl->pfrustum)
		{
			if(dl->pfrustum->CullBBox(mins, maxs))
				continue;
		}
		else
		{
			if(Math::CheckMinsMaxs(mins, maxs, lightmins, lightmaxs))
				continue;
		}

		// Check if this entity is already present in the list
		Uint32 j = 0;
		for(; j < dl->pstaticinfo->numvisents; j++)
		{
			if(pvisentity == dl->pstaticinfo->pvisents[j])
				break;
		}
		
		if(j == dl->pstaticinfo->numvisents)
		{
			if(dl->pstaticinfo->numvisents == MAX_DLIGHT_VISENTS)
				return (numAdded > 0) ? true : false;

			// Add the missing entity to the list
			dlight_staticinfo_t *pinfo = dl->pstaticinfo;
			pinfo->pvisents[pinfo->numvisents] = pvisentity;
			pinfo->numvisents++;

			// We'll need to redraw this light
			bRedraw = true;
			numAdded++;
		}
	}

	// If we haven't drawn a shadowmap yet
	if(dl->pstaticinfo->drawframe == -1)
		return true;

	// Check for resets
	shadowmap_t* pshadowmap = dl->cone_size ? dl->pshadowmap : dl->psmcubemap;
	if(pshadowmap->reset)
		return true;

	return bRedraw;
}

//====================================
//
//====================================
Int32 CDynamicLightManager::GetShadowmapSize( void ) const
{
	return m_shadowmapSize;
}

//====================================
//
//====================================
Int32 CDynamicLightManager::GetCubeShadowmapSize( void ) const
{
	return m_cubeShadowmapSize;
}

//====================================
//
//====================================
void CDynamicLightManager::ReleaseEntityDynamicLights( entindex_t entindex )
{
	// Release any dynamic lights tied to this entity
	m_dlightsList.begin();
	while(!m_dlightsList.end())
	{
		cl_dlight_t* pdl = m_dlightsList.get();

		if(pdl->key == entindex)
		{
			FreeDynamicLight(pdl);
			m_dlightsList.remove(m_dlightsList.get_link());
		}

		m_dlightsList.next();
	}

	// Release any shadowmaps tied to this entity
	cl_entity_t* pentity = CL_GetEntityByIndex(entindex);
	if(pentity && pentity->pextradata)
	{
		entity_extrainfo_t* pextradata = CL_GetEntityExtraData(pentity);
		if(pextradata && pextradata->pshadowmap)
		{
			// Clear the shadow map
			if(pextradata->pshadowmap->projective)
				gDynamicLights.ClearProjectiveShadowMap(pextradata->pshadowmap);
			else
				gDynamicLights.ClearCubemapShadowMap(pextradata->pshadowmap);

			// Remove the pointer
			pextradata->pshadowmap = nullptr;
		}
	}
}

//====================================
//
//====================================
bool DL_IsLightVisible( const CFrustum& mainFrustum, const Vector& mins, const Vector& maxs, cl_dlight_t* pdl )
{
	if(pdl->followentity)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(pdl->key);
		if(!pentity)
			return false;

		cl_entity_t* plocalplayer = CL_GetLocalPlayer();
		if(!plocalplayer)
			return false;

		if(plocalplayer->curstate.msg_num != pentity->curstate.msg_num)
			return false;
	}

	// Check against main frustum first
	return mainFrustum.CullBBox(mins, maxs) == true ? false : true;
}

//====================================
//
//====================================
bool DL_CanShadow( const cl_dlight_t *dl )
{
	if(!rns.fboused)
		return false;

	if(g_pCvarShadows->GetValue() < 1)
		return false;

	if(g_pCvarDynamicLights->GetValue() < 1)
		return false;

	if(dl->noShadow())
		return false;

	return true;
}

//====================================
//
//====================================
void R_CheckShadowmapSizeCvarCallBack( CCVar* pCVar )
{
	if(Common::IsPowerOfTwo(pCVar->GetValue()) && pCVar->GetValue() >= CDynamicLightManager::SHADOWMAP_MIN_SIZE)
		return;

	Con_Printf("Invalid size %f for %s, minimum is %d\n", pCVar->GetValue(), pCVar->GetName(), (Int32)CDynamicLightManager::SHADOWMAP_MIN_SIZE);
	gConsole.CVarSetFloatValue(pCVar->GetName(), CDynamicLightManager::SHADOWMAP_MIN_SIZE);
}