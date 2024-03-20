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
#include "r_main.h"
#include "cl_main.h"
#include "r_fbo.h"
#include "cvar.h"
#include "r_postprocess.h"
#include "r_rttcache.h"
#include "r_basic_vertex.h"
#include "console.h"
#include "textures_shared.h"
#include "texturemanager.h"
#include "cl_pmove.h"
#include "networking.h"
#include "system.h"
#include "file.h"
#include "r_common.h"

// Blur fade amount
static const Float BLUR_FADE = 0.1;
// Water effects fade duration
static const Float WATER_FADE_TIME = 2.0;

// Class definition
CPostProcess gPostProcess;

//=============================================
// @brief
//
//=============================================
CPostProcess::CPostProcess( void ):
	m_pCvarGamma(nullptr),
	m_gaussianBlurActive(false),
	m_motionBlurActive(false),
	m_blurOverride(false),
	m_isFirstFrame(false),
	m_blurFade(0),
	m_lastWaterTime(-1),
	m_gaussianBlurAlpha(0),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pCvarFilmGrain(nullptr),
	m_pCvarPostProcess(nullptr),
	m_pScreenRTT(nullptr),
	m_pScreenTexture(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CPostProcess::~CPostProcess( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: Init( void )
{
	// Set up the cvars
	m_pCvarGamma = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), GAMMA_CVAR_NAME, "1.8", "Controls gamma value.");
	m_pCvarFilmGrain = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_filmgrain", "1", "Toggle film grain." );
	m_pCvarPostProcess = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_postprocess", "1", "Disable post-process effects." );

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: Shutdown( void )
{
	ClearGame();
	ClearGL();
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: InitGL( void )
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		// Compile our shader
		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "postprocess.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Could not compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_offset = m_pShader->InitUniform("offset", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_gamma = m_pShader->InitUniform("gamma", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_screenwidth = m_pShader->InitUniform("screenwidth", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_screenheight = m_pShader->InitUniform("screenheight", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_timer = m_pShader->InitUniform("timer", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_texture1 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_texture2 = m_pShader->InitUniform("blurtexture", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_offset, "offset", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_color, "color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_gamma, "gamma", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenwidth, "screenwidth", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenheight, "screenheight", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_timer, "timer", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture1, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture1, "texture1", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, origin));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, texcoords));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_type = m_pShader->GetDeterminatorIndex("pp_type");
		if(!R_CheckShaderDeterminator(m_attribs.d_type, "pp_type", m_pShader, Sys_ErrorPopup))
			return false;
	}

	// Build the VBO if needed
	if(!m_pVBO)
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

		m_pVBO = new CVBO(gGLExtF, pverts, sizeof(basic_vertex_t)*6, nullptr, 0);
		m_pShader->SetVBO(m_pVBO);
		delete[] pverts;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: ClearGL( void )
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: InitGame( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: ClearGame( void )
{
	// reset blur
	m_gaussianBlurActive = false;
	m_motionBlurActive = false;
	m_blurOverride = false;
	m_lastWaterTime = -1;
	m_gaussianBlurAlpha = 1.0;

	// reset fade
	for(Uint32 i = 0; i < MAX_FADE_LAYERS; i++)
	{
		if(!(m_fadeLayersArray[i].flags & FL_FADE_PERMANENT))
			m_fadeLayersArray[i] = screenfade_t();
	}

	m_pScreenTexture = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: FetchScreen( rtt_texture_t** ptarget )
{
	// Grab current screen contents and apply gamma
	if((*ptarget) == nullptr)
		(*ptarget) = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

	R_BindRectangleTexture(GL_TEXTURE0_ARB, (*ptarget)->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawGamma( void )
{
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_NORMAL))
		return false;

	m_pShader->SetUniform1f(m_attribs.u_gamma, m_pCvarGamma->GetValue()/1.8);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_GAMMA))
		return false;

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawDistortion( void )
{
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	Float flOffset = rns.time*8;
	m_pShader->SetUniform1f(m_attribs.u_offset, flOffset);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_DISTORT))
		return false;

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawBlur( void )
{
	// blur horizontally
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLUR_H))
		return false;

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// blur horizontally
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLUR_V))
		return false;

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawMotionBlur( void )
{
	// Fetch screen contents
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	// Bind the blur rectangle texture
	gGLExtF.glActiveTexture(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_RECTANGLE);
	R_BindRectangleTexture(GL_TEXTURE1_ARB, m_pScreenTexture->gl_index);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_MBLUR))
		return false;

	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, 1.0 - m_blurFade);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	gGLExtF.glActiveTexture(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_RECTANGLE);

	// Copy to the blur FBO now
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenTexture->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawFade( screenfade_t& fade )
{
	glEnable(GL_BLEND);

	if(fade.flags & FL_FADE_OUT)
	{
		if(rns.time <= fade.end)
			fade.curfade = clamp(fade.curfade+fade.speed*cls.frametime, 0.0, 1.0);
		else
			fade.curfade = 1.0;
	}
	else
	{
		if(rns.time >= fade.end)
			fade.curfade = clamp(fade.curfade+fade.speed*cls.frametime, 0.0, 1.0);
		else
			fade.curfade = 1.0;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_ENVFADE))
		return false;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_pShader->SetUniform4f(m_attribs.u_color, fade.color.r/255.0f, fade.color.g/255.0f, fade.color.b/255.0f, (fade.alpha/255.0f)*fade.curfade);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_BLEND);

	// Check if we need to reset
	if(fade.totalend)
	{
		if(fade.totalend <= rns.time)
		{
			if((fade.flags & FL_FADE_OUT) && m_motionBlurActive)
				m_isFirstFrame = true;

			fade = screenfade_t();
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawFilmGrain( void )
{
	// Fetch screen contents
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	m_pShader->SetUniform1f(m_attribs.u_timer, rns.time*0.1);
	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_GRAIN))
		return false;

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: ClearMotionBlur( void )
{
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenTexture->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
	m_isFirstFrame = false;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: CreateScreenTexture( void )
{
	// Create the screen texture
	m_pScreenTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

	glBindTexture(GL_TEXTURE_RECTANGLE, m_pScreenTexture->gl_index);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, rns.screenwidth, rns.screenheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: Draw( void )
{
	Int32 viewcontents = CL_PointContents(rns.view.params.v_origin, 0);
	if(viewcontents == CONTENTS_WATER)
		m_lastWaterTime = rns.time;

	bool bInWater = (m_lastWaterTime == -1 || m_lastWaterTime+WATER_FADE_TIME < rns.time) ? false : true;

	if(!m_motionBlurActive && !bInWater && m_pCvarGamma->GetValue() == 1.8
		|| m_pCvarPostProcess->GetValue() <= 0 && !m_motionBlurActive)
	{
		// See if we have any active fades
		Uint32 i = 0;
		for(; i < MAX_FADE_LAYERS; i++)
		{
			if(m_fadeLayersArray[i].end)
				break;
		}

		if(i == MAX_FADE_LAYERS)
			return true;
	}

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->SetUniform1i(m_attribs.u_texture1, 0);
	m_pShader->SetUniform1i(m_attribs.u_texture2, 1);
	m_pShader->SetUniform1f(m_attribs.u_screenwidth, rns.screenwidth);
	m_pShader->SetUniform1f(m_attribs.u_screenheight, rns.screenheight);

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	glDepthMask(GL_FALSE);

	rns.view.projection.LoadIdentity();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE);

	// Set color
	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	// Create the texture if needed
	if(!m_pScreenTexture && m_motionBlurActive)
		CreateScreenTexture();

	if(m_pCvarPostProcess->GetValue() > 0)
	{
		// Apply gamma if needed
		if(m_pCvarGamma->GetValue() != 1.8)
		{
			if(!DrawGamma())
			{
				Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
				m_pShader->DisableShader();
				m_pVBO->UnBind();
				return false;
			}
		}
	}

	// Reset blur if it's the first frame of blurring
	if(m_motionBlurActive && m_isFirstFrame)
		ClearMotionBlur();

	// Water/motion blur blurring
	if(m_motionBlurActive || (bInWater || m_gaussianBlurActive) && m_pCvarPostProcess->GetValue() > 0)
	{
		if(m_pCvarPostProcess->GetValue() > 0)
		{
			Float alpha = 1.0;

			if(bInWater && !m_gaussianBlurActive)
			{
				alpha = 1.0 - ((rns.time - m_lastWaterTime)/WATER_FADE_TIME);
				alpha = clamp(alpha, 0.0, 1.0);
			}
			else if(m_gaussianBlurActive && m_gaussianBlurAlpha != 1.0)
			{
				alpha = m_gaussianBlurAlpha;
			}

			if(alpha != 1.0)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, alpha);
			}

			// Apply water distortion if needed
			if(bInWater)
			{
				if(!DrawDistortion())
				{
					Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
					m_pShader->DisableShader();
					m_pVBO->UnBind();
					return false;
				}
			}

			// Apply gaussian blurring
			if(g_pCvarGaussianBlur->GetValue() > 0)
			{
				if(!DrawBlur())
				{
					Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
					m_pShader->DisableShader();
					m_pVBO->UnBind();
					return false;
				}
			}

			if(alpha != 1.0)
			{
				glDisable(GL_BLEND);
				m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
			}
		}

		// Apply motion blur if needed
		if(m_motionBlurActive)
		{
			if(!DrawMotionBlur())
			{
				Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
				m_pShader->DisableShader();
				m_pVBO->UnBind();
				return false;
			}
		}
	}

	// Draw fade before film grain is applied
	for(Uint32 i = 0; i < 3; i++)
	{
		if(m_fadeLayersArray[i].end)
			DrawFade(m_fadeLayersArray[i]);
	}

	// Render film grain
	if(m_pCvarPostProcess->GetValue() > 0 && m_pCvarFilmGrain->GetValue() > 0)
	{
		if(!DrawFilmGrain())
		{
			Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_RECTANGLE);
	glDepthMask(GL_TRUE);

	if(m_pScreenRTT)
	{
		gRTTCache.Free(m_pScreenRTT);
		m_pScreenRTT = nullptr;
	}

	m_pShader->DisableAttribute(m_attribs.a_origin);
	m_pShader->DisableAttribute(m_attribs.a_texcoord);
	m_pShader->DisableShader();

	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: SetGaussianBlur( bool active, Float alpha )
{
	m_gaussianBlurActive = active;

	if(m_gaussianBlurActive)
		m_gaussianBlurAlpha = alpha;
	else
		m_gaussianBlurAlpha = 1.0;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: SetMotionBlur( bool active, Float blurfade, bool override )
{
	if(!active)
	{
		if(override && m_motionBlurActive && !m_blurOverride)
			return;

		m_motionBlurActive = false;
		return;
	}

	m_blurFade = blurfade;
	if(!m_blurFade)
		m_blurFade = BLUR_FADE;

	if(override && m_motionBlurActive)
		return;
	else if(override)
		m_blurOverride = true;
	else
		m_blurOverride = false;

	m_motionBlurActive = true;
	m_isFirstFrame = true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: SetFade( Uint32 layerindex, Float duration, Float holdtime, Int32 flags, const color24_t& color, byte alpha, Float timeoffset )
{
	// Don't apply if we're over the full duration
	if(timeoffset > duration+holdtime)
		return;
	
	if(layerindex >= MAX_FADE_LAYERS)
	{
		Con_EPrintf("%s - Bogus layer index %d specified.\n", __FUNCTION__, layerindex);
		return;
	}

	screenfade_t& fade = m_fadeLayersArray[layerindex];
	if(fade.flags & FL_FADE_CLEARGAME)
		return;

	fade.flags = flags;
	fade.color = color;
	fade.alpha = alpha;
	fade.speed = 1.0f/duration;

	if(fade.flags & FL_FADE_OUT)
	{
		if(timeoffset < duration)
		{
			fade.end = cls.cl_time+(duration-timeoffset);
			fade.curfade = (timeoffset/duration);
		}
		else
		{
			fade.end = cls.cl_time;
			fade.curfade = 1.0;
		}
	}
	else
	{
		fade.speed *= -1;

		if(timeoffset < holdtime)
		{
			fade.end = cls.cl_time+(holdtime-timeoffset);
			fade.curfade = 1.0;
		}
		else
		{
			fade.end = cls.cl_time;
			fade.curfade = 1.0 - (timeoffset/duration);
		}
	}

	if(!(fade.flags & FL_FADE_STAYOUT))
		fade.totalend = cls.cl_time+(duration+holdtime-timeoffset);
}