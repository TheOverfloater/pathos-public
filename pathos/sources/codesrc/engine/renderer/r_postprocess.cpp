/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Notes:
// The black and white effect, chromatic aberration, vignette 
// and entity-customizable film grain code was done by valina354.

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
static constexpr Float BLUR_FADE = 0.1;
// Water effects fade duration
static constexpr Float WATER_FADE_TIME = 2.0;
// Default grain amount
static constexpr Float DEFAULT_GRAIN_AMOUNT = 0.05f;
// Overlay folder name
const Char CPostProcess::OVERLAY_FOLDER_PATH[] = "overlay";

//====================================
//
//====================================
Int32 R_SortOverlays( const void* p1, const void* p2 )
{
	CPostProcess::overlay_t *poverlay1 = static_cast<CPostProcess::overlay_t *>(const_cast<void*>(p1));
	CPostProcess::overlay_t *poverlay2 = static_cast<CPostProcess::overlay_t *>(const_cast<void*>(p2));

	if(poverlay1->layerindex < poverlay2->layerindex)
		return 1;
	else if(poverlay1->layerindex == poverlay2->layerindex)
		return 0;
	else
		return -1;
}

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
	m_pCvarBloom(nullptr),
	m_pCvarBloomDarkenSteps(nullptr),
	m_pCvarBloomDarkenMultiplier(nullptr),
	m_pCvarBloomBlurSteps(nullptr),
	m_pCvarBloomBrightenMultiplier(nullptr),
	m_pCvarBloomBrightnessTreshold(nullptr),
	m_pScreenRTT(nullptr),
	m_pBlurScreenTexture(nullptr),
	m_vignetteActive(false),
	m_chromaticActive(false),
	m_vignetteStrength(0),
	m_chromaticStrength(0.0f),
	m_vignetteRadius(0),
	m_blackAndWhiteActive(false),
	m_blackAndWhiteStrength(0.0f) ,
	m_filmGrainStrength(0.0f),
	m_filmGrainActive(0.0f)
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

	m_pCvarBloom = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT|FL_CV_SAVE, "r_bloom", "0", "Enable or disable bloom.");
	m_pCvarBloomDarkenMultiplier = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT | FL_CV_SAVE, "r_bloom_darken_multiplier", "1", "Controls the amount of brightness during darkening for bloom.");
	m_pCvarBloomDarkenSteps = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT | FL_CV_SAVE, "r_bloom_darken_steps", "8", "Controls the amount of darkening done for bloom.");
	m_pCvarBloomBlurSteps = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT | FL_CV_SAVE, "r_bloom_blur_steps", "1", "Controls the amount of bloom blurring steps.");
	m_pCvarBloomBrightenMultiplier = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT | FL_CV_SAVE, "r_bloom_brighten_multiplier", "1", "Controls the brightness of bloom.");
	m_pCvarBloomBrightnessTreshold = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT | FL_CV_SAVE, "r_bloom_brightness_treshold", "1", "Controls lower cutoff brightness for bloom.");

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
		m_attribs.u_color = m_pShader->InitUniform("u_color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_gamma = m_pShader->InitUniform("gamma", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_screenwidth = m_pShader->InitUniform("screenwidth", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_screenheight = m_pShader->InitUniform("screenheight", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_timer = m_pShader->InitUniform("timer", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_grainammount = m_pShader->InitUniform("grainamount", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_chromatic_strength = m_pShader->InitUniform("chromaticStrength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_bw_strength = m_pShader->InitUniform("BWStrength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_vignette_strength = m_pShader->InitUniform("vignetteStrength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_vignette_radius = m_pShader->InitUniform("vignetteRadius", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_offsetdivider = m_pShader->InitUniform("offsetdivider", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_texture1rect = m_pShader->InitUniform("texture0rect", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_texture2rect = m_pShader->InitUniform("blurtextureRect", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_texture2d = m_pShader->InitUniform("normal2dTexture", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_darken_steps = m_pShader->InitUniform("darken_steps", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_brighten_multiplier = m_pShader->InitUniform("brighten_multiplier", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_blur_brightness = m_pShader->InitUniform("blur_brightness", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_blur_radius = m_pShader->InitUniform("blur_radius", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_brightness_treshold = m_pShader->InitUniform("brightness_treshold", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_tcscale = m_pShader->InitUniform("tc_scale", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_offset, "offset", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_color, "u_color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_gamma, "gamma", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenwidth, "screenwidth", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenheight, "screenheight", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_timer, "timer", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_grainammount, "grainamount", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_chromatic_strength, "chromaticStrength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_bw_strength, "BWStrength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vignette_strength, "vignetteStrength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vignette_radius, "vignetteRadius", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_offsetdivider, "offsetdivider", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture1rect, "texture0rect", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture2rect, "blurtextureRect", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture2d, "2dTexture", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_darken_steps, "darken_steps", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_brighten_multiplier, "brighten_multiplier", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_blur_brightness, "blur_brightness", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_blur_radius, "blur_radius", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_brightness_treshold, "brightness_treshold", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_tcscale, "tc_scale", m_pShader, Sys_ErrorPopup))
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
		pverts[1].texcoords[0] = 0; pverts[1].texcoords[1] = 1;

		pverts[2].origin[0] = 1; pverts[2].origin[1] = 0; 
		pverts[2].origin[2] = -1; pverts[2].origin[3] = 1;
		pverts[2].texcoords[0] = 1; pverts[2].texcoords[1] = 1;

		pverts[3].origin[0] = 0; pverts[3].origin[1] = 1; 
		pverts[3].origin[2] = -1; pverts[3].origin[3] = 1;
		pverts[3].texcoords[0] = 0; pverts[3].texcoords[1] = 0;

		pverts[4].origin[0] = 1; pverts[4].origin[1] = 0; 
		pverts[4].origin[2] = -1; pverts[4].origin[3] = 1;
		pverts[4].texcoords[0] = 1; pverts[4].texcoords[1] = 1;

		pverts[5].origin[0] = 1; pverts[5].origin[1] = 1; 
		pverts[5].origin[2] = -1; pverts[5].origin[3] = 1;
		pverts[5].texcoords[0] = 1; pverts[5].texcoords[1] = 0;

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
	m_vignetteActive = false;
	m_filmGrainActive = false;
	m_chromaticActive = false;
	m_blackAndWhiteActive = false;
	m_blurOverride = false;
	m_lastWaterTime = -1;
	m_gaussianBlurAlpha = 1.0;

	// reset fade
	for(Uint32 i = 0; i < MAX_FADE_LAYERS; i++)
	{
		if(!(m_fadeLayersArray[i].flags & FL_FADE_PERMANENT))
			m_fadeLayersArray[i] = screenfade_t();
	}

	if(!m_screenOverlays.empty())
		m_screenOverlays.clear();

	m_pBlurScreenTexture = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: FetchScreen( rtt_texture_t** ptarget )
{
	// Grab current screen contents and copy to target
	if((*ptarget) == nullptr)
		(*ptarget) = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

	R_BindRectangleTexture(GL_TEXTURE0_ARB, (*ptarget)->palloc->gl_index, true);
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

	m_pShader->SetUniform1f(m_attribs.u_gamma, m_pCvarGamma->GetValue()/1.8);

	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_GAMMA))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

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
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

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
	m_pShader->SetUniform1f(m_attribs.u_blur_brightness, 1.0);

	// blur horizontally
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLUR_H))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// blur vertically
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
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);
	R_BindRectangleTexture(GL_TEXTURE1_ARB, m_pBlurScreenTexture->gl_index);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_MBLUR))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, 1.0 - m_blurFade);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Copy to the blur rectangle now
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pBlurScreenTexture->gl_index, true);
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
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	// If set from an entity, film grain strength is overridden by that
	Float filmGrainStrength = m_filmGrainActive ? m_filmGrainStrength : m_pCvarFilmGrain->GetValue();
	Float grainAmount = DEFAULT_GRAIN_AMOUNT * filmGrainStrength;

	m_pShader->SetUniform1f(m_attribs.u_timer, rns.time*0.1);
	m_pShader->SetUniform1f(m_attribs.u_grainammount, grainAmount);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_GRAIN))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess::DrawChromatic( void )
{
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	m_pShader->SetUniform1f(m_attribs.u_chromatic_strength, m_chromaticStrength);

	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_CHROMATIC))
		return false;

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess::DrawBlackAndWhite( void )
{
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	m_pShader->SetUniform1f(m_attribs.u_bw_strength, m_blackAndWhiteStrength);

	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BW))
		return false;

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess::DrawVignette( void )
{
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	m_pShader->SetUniform1f(m_attribs.u_vignette_strength, m_vignetteStrength);
	m_pShader->SetUniform1f(m_attribs.u_vignette_radius, m_vignetteRadius);

	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_VIGNETTE))
		return false;

	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess::DrawOverlays( void )
{
	if(m_screenOverlays.empty())
		return true;

	m_pShader->SetUniform2f(m_attribs.u_tcscale, 1.0, 1.0);

	for(Uint32 i = 0; i < m_screenOverlays.size(); i++)
	{
		overlay_t& overlay = m_screenOverlays[i];

		Float alphamultiplier = 1.0;
		if(overlay.rendermode != RENDER_NORMAL && overlay.rendermode != OVERLAY_RENDER_ALPHATEST)
		{
			if(overlay.fadebegintime)
			{
				// See if we need to set the current time
				if(overlay.fadebegintime == -1)
					overlay.fadebegintime = rns.time;

				// Calculate current value
				alphamultiplier = (rns.time - overlay.fadebegintime) / overlay.fadetime;

				if(overlay.fadeout)
				{
					alphamultiplier = (1.0 - alphamultiplier);
					if(alphamultiplier <= 0.0)
					{
						// This overlay expired, so remove it
						m_screenOverlays.erase(i);
						i--;
						continue;
					}
				}
				else
				{
					if(alphamultiplier >= 1.0)
					{
						overlay.fadebegintime = 0;
						overlay.fadetime = 0;
						alphamultiplier = 1.0;
					}
				}
			}

			// Apply any effects
			if(overlay.effect != OVERLAY_EFFECT_NONE)
			{
				if(overlay.effectbegintime == -1)
					overlay.effectbegintime = rns.time;

				switch(overlay.effect)
				{
				case OVERLAY_EFFECT_PULSATE:
					{
						Float value = SDL_fabs(SDL_sin((rns.time - overlay.effectbegintime) * overlay.effectspeed));
						alphamultiplier *= (overlay.effectminalpha + (value * (1.0 - overlay.effectminalpha)));
					}
					break;
				}
			}

			// Make sure this value is clamped
			alphamultiplier = clamp(alphamultiplier, 0, 1);
		}

		Vector renderColor;
		Float renderAlpha;
		switch(overlay.rendermode)
		{
		case OVERLAY_RENDER_ADDITIVE:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				renderAlpha = 1.0;
				Math::VectorScale(overlay.rendercolor, alphamultiplier, renderColor);
				alphamultiplier = 1.0;
			}
			break;
		case OVERLAY_RENDER_ALPHATEST:
			{
				glDisable(GL_BLEND);
				renderAlpha = 1.0;
				renderColor = overlay.rendercolor;
			}
			break;
		case OVERLAY_RENDER_ALPHABLEND:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				renderAlpha = overlay.renderamt > 0 ? overlay.renderamt : 1.0;
				renderColor = overlay.rendercolor;
			}
			break;
		case OVERLAY_RENDER_NORMAL:
		default:
			{
				glDisable(GL_BLEND);
				renderAlpha = 1.0;
				renderColor = overlay.rendercolor;
			}
			break;
		}

		m_pShader->SetUniform4f(m_attribs.u_color, renderColor.x, renderColor.y, renderColor.z, renderAlpha * alphamultiplier);

		Int32 shadertype = (overlay.rendermode == OVERLAY_RENDER_ALPHATEST) ? SHADER_2DTEXTURE_ALPHATEST : SHADER_2DTEXTURE;
		if (!m_pShader->SetDeterminator(m_attribs.d_type, shadertype))
			return false;

		R_Bind2DTexture(GL_TEXTURE0, overlay.ptexture->palloc->gl_index);
		R_ValidateShader(m_pShader);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	m_pShader->SetUniform2f(m_attribs.u_tcscale, rns.screenwidth, rns.screenheight);
	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glDisable(GL_BLEND);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: DrawBloom( void )
{
	Float bloomCvarValue = m_pCvarBloom->GetValue();
	if (bloomCvarValue < 1)
		return true;

	glDisable(GL_BLEND);

	// Step 1: Overdarken the screen contents
	FetchScreen(&m_pScreenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLOOM_DARKEN))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	Float darkenBrightness = m_pCvarBloomDarkenMultiplier->GetValue();
	darkenBrightness = clamp(darkenBrightness, 0, 1);
	m_pShader->SetUniform1f(m_attribs.u_brighten_multiplier, darkenBrightness);

	Float darkenStepsCvarValue = m_pCvarBloomDarkenSteps->GetValue();
	Uint32 darkenSteps = clamp(darkenStepsCvarValue, 1, 128);
	m_pShader->SetUniform1f(m_attribs.u_darken_steps, darkenSteps);

	Float brightnessTreshold = m_pCvarBloomBrightnessTreshold->GetValue();
	brightnessTreshold = clamp(brightnessTreshold, 0, 4);
	m_pShader->SetUniform1f(m_attribs.u_brightness_treshold, brightnessTreshold);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (bloomCvarValue == 2)
		return true;

	// Now blur horizontally
	rtt_texture_t* pDarkenRTT = nullptr;

	Float blurStepCvarValue = m_pCvarBloomBlurSteps->GetValue();
	Uint32 numBlurSteps = clamp(blurStepCvarValue, 1, 64);

	m_pShader->SetUniform1f(m_attribs.u_brighten_multiplier, 1.0);
	m_pShader->SetUniform1f(m_attribs.u_blur_brightness, 1.0);
	m_pShader->SetUniform1f(m_attribs.u_blur_radius, numBlurSteps);

	// Blur horizontally
	FetchScreen(&pDarkenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, pDarkenRTT->palloc->gl_index);

	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLOOM_BLUR_H))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	R_ValidateShader(m_pShader);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	FetchScreen(&pDarkenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, pDarkenRTT->palloc->gl_index);

	// Now blur vertically
	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLOOM_BLUR_V))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	R_ValidateShader(m_pShader);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Now draw the blurred bloom texture
	FetchScreen(&pDarkenRTT);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);

	// Restore screen contents
	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_NORMAL))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	R_ValidateShader(m_pShader);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	R_BindRectangleTexture(GL_TEXTURE0_ARB, pDarkenRTT->palloc->gl_index);

	// Now draw the final blurred image
	if (!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_BLOOM_APPLY))
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	if (bloomCvarValue != 3)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	Float brightenCvarValue = m_pCvarBloomBrightenMultiplier->GetValue();
	brightenCvarValue = clamp(brightenCvarValue, 0, 16);
	m_pShader->SetUniform1f(m_attribs.u_brighten_multiplier, brightenCvarValue);

	R_ValidateShader(m_pShader);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (bloomCvarValue != 3)
		glDisable(GL_BLEND);

	if (pDarkenRTT)
	{
		gRTTCache.Free(pDarkenRTT);
		pDarkenRTT = nullptr;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: ClearMotionBlur( void )
{
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pBlurScreenTexture->gl_index, true);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

	m_isFirstFrame = false;
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: CreateBlurScreenTexture( void )
{
	// Create the screen texture
	m_pBlurScreenTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

	GLint textureBound;
	glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &textureBound);

	glBindTexture(GL_TEXTURE_RECTANGLE, m_pBlurScreenTexture->gl_index);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, rns.screenwidth, rns.screenheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_RECTANGLE, textureBound);
}

//=============================================
// @brief
//
//=============================================
bool CPostProcess :: Draw( bool noFilmGrain )
{
	Int32 viewcontents = CL_PointContents(rns.view.params.v_origin, 0);
	if(viewcontents == CONTENTS_WATER)
		m_lastWaterTime = rns.time;

	bool bInWater = (m_lastWaterTime == -1 || m_lastWaterTime+WATER_FADE_TIME < rns.time) ? false : true;

	if(!m_motionBlurActive && !bInWater && m_pCvarGamma->GetValue() == 1.8
		|| m_pCvarPostProcess->GetValue() <= 0 && !m_motionBlurActive
		&& !m_screenOverlays.empty() && !m_vignetteActive && !m_blackAndWhiteActive
		&& !m_filmGrainActive && !m_chromaticActive)
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

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		return false;
	}

	m_pShader->SetUniform1i(m_attribs.u_texture1rect, 0);
	m_pShader->SetUniform1i(m_attribs.u_texture2rect, 1);

	m_pShader->SetUniform1f(m_attribs.u_screenwidth, rns.screenwidth);
	m_pShader->SetUniform1f(m_attribs.u_screenheight, rns.screenheight);

	m_pShader->SetUniform2f(m_attribs.u_tcscale, rns.screenwidth, rns.screenheight);
	m_pShader->SetUniform2f(m_attribs.u_offsetdivider, 1.0f, 1.0f);

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	rns.view.projection.LoadIdentity();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	// Create the texture if needed
	if (m_motionBlurActive && !m_pBlurScreenTexture)
		CreateBlurScreenTexture();

	// Set color
	m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	// Render overlays
	if(!DrawOverlays())
	{
		glEnable(GL_DEPTH_TEST);
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return false;
	}

	if(m_pCvarPostProcess->GetValue() > 0)
	{
		// Apply gamma if needed
		if(m_pCvarGamma->GetValue() != 1.8)
		{
			if(!DrawGamma())
			{
				glEnable(GL_DEPTH_TEST);
				m_pShader->DisableShader();
				m_pVBO->UnBind();
				return false;
			}
		}

		// Draw bloom if set
		if(!DrawBloom())
		{
			glEnable(GL_DEPTH_TEST);
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
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
					glEnable(GL_DEPTH_TEST);
					m_pShader->DisableShader();
					m_pVBO->UnBind();
					return false;
				}
			}

			// Apply gaussian blurring
			if(g_pCvarGaussianBlur->GetValue() > 0)
			{
				if (!DrawBlur())
				{
					glEnable(GL_DEPTH_TEST);
					m_pShader->DisableShader();
					m_pVBO->UnBind();
					return false;
				}
			}

			if(alpha != 1.0)
			{
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				m_pShader->SetUniform4f(m_attribs.u_color, GL_ONE, GL_ONE, GL_ONE, GL_ONE);
			}
		}

		// Apply motion blur if needed
		if(m_motionBlurActive)
		{
			if(!DrawMotionBlur())
			{
				glEnable(GL_DEPTH_TEST);
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
	if (!noFilmGrain && m_pCvarPostProcess->GetValue() > 0 && (m_pCvarFilmGrain->GetValue() > 0 || m_filmGrainActive))
	{
		if(!DrawFilmGrain())
		{
			glEnable(GL_DEPTH_TEST);
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	// Render chromatic
	if (m_pCvarPostProcess->GetValue() > 0 && m_chromaticActive)
	{
		if (!DrawChromatic())
		{
			glEnable(GL_DEPTH_TEST);
			Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	// Render BW
	if (m_pCvarPostProcess->GetValue() > 0 && m_blackAndWhiteActive)
	{
		if (!DrawBlackAndWhite())
		{
			glEnable(GL_DEPTH_TEST);
			Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	// Apply vignette if needed
	if (m_pCvarPostProcess->GetValue() > 0 && m_vignetteActive)
	{
		if (!DrawVignette())
		{
			glEnable(GL_DEPTH_TEST);
			m_pShader->DisableShader();
			m_pVBO->UnBind();
			return false;
		}
	}

	if (m_pScreenRTT)
	{
		gRTTCache.Free(m_pScreenRTT);
		m_pScreenRTT = nullptr;
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

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
void CPostProcess::SetVignette(bool active, Float strength, Float radius)
{
	if (!active)
	{
		m_vignetteActive = false;
		return;
	}

	m_vignetteActive = active;
	m_vignetteStrength = strength;
	m_vignetteRadius = radius;
}
//=============================================
// @brief
//
//=============================================
void CPostProcess::SetFilmGrain(bool active, Float strength)
{
	if (!active)
	{
		m_filmGrainActive = false;
		return;
	}
	m_filmGrainActive = active;
	m_filmGrainStrength = strength;
}
//=============================================
// @brief
//
//=============================================
void CPostProcess::SetBlackAndWhite(bool active, Float strength)
{
	if (!active)
	{
		m_blackAndWhiteActive = false;
		return;
	}
	m_blackAndWhiteActive = active;
	m_blackAndWhiteStrength = strength;
}
//=============================================
// @brief
//
//=============================================
void CPostProcess::SetChromatic(bool active, Float strength)
{
	if (!active)
	{
		m_chromaticActive = false;
		return;
	}
	m_chromaticActive = active;
	m_chromaticStrength = strength;
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

//=============================================
// @brief
//
//=============================================
void CPostProcess :: SetOverlay( Int32 layerindex, const Char* pstrtexturename, overlay_rendermode_t rendermode, const Vector& rendercolor, Float renderamt, 
	overlay_effect_t effect, Float effectspeed, Float effectminalpha, Float fadetime )
{
	if(!pstrtexturename || !qstrlen(pstrtexturename))
	{
		for(Uint32 i = 0; i < m_screenOverlays.size(); i++)
		{
			if(m_screenOverlays[i].layerindex == layerindex)
			{
				m_screenOverlays.erase(i);
				return;
			}
		}
	}

	// Remove the ratio tag
	CString formattag;
	CString texturename(pstrtexturename);

	for(Uint32 i = 0; i < NB_SCREENRATIO_STRINGS; i++)
	{
		Int32 tagpos = texturename.find(0, SCREEN_RATIO_STRINGS[i]);
		if(tagpos != CString::CSTRING_NO_POSITION)
		{
			texturename.erase(tagpos, qstrlen(SCREEN_RATIO_STRINGS[i]));
			break;
		}
	}

	// Find the dot and separate it off
	Int32 dotpos = texturename.find(0, ".");
	if(dotpos == CString::CSTRING_NO_POSITION)
	{
		// Shouldn't really happen
		formattag = ".dds";
	}
	else
	{
		// Put it into the format tag string
		Uint32 formatlen = texturename.length() - (dotpos + 1);
		formattag.assign(texturename.c_str() + dotpos + 1, formatlen);

		// Erase from name
		texturename.erase(dotpos, texturename.length() - dotpos);
	}

	// Get aspect ratio string
	CString aspectRatioStr = R_GetAspectRatio(rns.screenwidth, rns.screenheight);
	if(!texturename.empty() && texturename[texturename.length()-1] == '_')
		texturename << aspectRatioStr << "." << formattag;
	else
		texturename << "_" << aspectRatioStr << "." << formattag;

	CString filepath;
	filepath << OVERLAY_FOLDER_PATH << PATH_SLASH_CHAR << texturename;

	en_texture_t* ptexture = CTextureManager::GetInstance()->LoadTexture(filepath.c_str(), RS_GAME_LEVEL, (TX_FL_NOMIPMAPS|TX_FL_CLAMP_S|TX_FL_CLAMP_T));
	if(!ptexture)
	{
		// If failed, just load the original texture specified
		filepath.clear();
		filepath << OVERLAY_FOLDER_PATH << PATH_SLASH_CHAR << pstrtexturename;

		ptexture = CTextureManager::GetInstance()->LoadTexture(filepath.c_str(), RS_GAME_LEVEL, (TX_FL_NOMIPMAPS|TX_FL_CLAMP_S|TX_FL_CLAMP_T));
		if(!ptexture)
		{
			Con_EPrintf("%s - Failed to load texture '%s'.\n", __FUNCTION__, filepath.c_str());
			return;
		}
	}

	Uint32 prevsize = m_screenOverlays.size();
	overlay_t* poverlay = nullptr;
	for(Uint32 i = 0; i < m_screenOverlays.size(); i++)
	{
		if(m_screenOverlays[i].layerindex == layerindex)
		{
			poverlay = &m_screenOverlays[i];
			break;
		}
	}

	if(!poverlay)
	{
		Int32 insertindex = m_screenOverlays.size();
		m_screenOverlays.resize(m_screenOverlays.size()+1);
		poverlay = &m_screenOverlays[insertindex];
	}

	poverlay->layerindex = layerindex;
	poverlay->ptexture = ptexture;
	poverlay->rendermode = static_cast<overlay_rendermode_t>(rendermode);
	poverlay->rendercolor = rendercolor;
	poverlay->renderamt = renderamt;
	poverlay->effect = static_cast<overlay_effect_t>(effect);
	poverlay->effectspeed = effectspeed;
	poverlay->effectminalpha = effectminalpha;
	poverlay->fadeout = false;
	poverlay->fadetime = fadetime;

	if(poverlay->rendercolor.IsZero())
		poverlay->rendercolor = Vector(1, 1, 1);

	// Mark for effect timer set
	if(poverlay->effect != OVERLAY_EFFECT_NONE && poverlay->effectbegintime == 0)
		poverlay->effectbegintime = -1;

	// Mark for fade timer set
	if(poverlay->fadetime > 0 && poverlay->fadebegintime == 0)
		poverlay->fadebegintime = -1;

	// Sort overlays in order if we added a new one
	if(prevsize != m_screenOverlays.size())
		qsort(&m_screenOverlays[0], m_screenOverlays.size(), sizeof(overlay_t), R_SortOverlays);
}

//=============================================
// @brief
//
//=============================================
void CPostProcess :: ClearOverlay( Int32 layerindex, Float fadetime )
{
	if(m_screenOverlays.empty())
		return;

	for(Uint32 i = 0; i < m_screenOverlays.size(); i++)
	{
		if(m_screenOverlays[i].layerindex == layerindex)
		{
			if(fadetime <= 0)
			{
				// Just remove it
				m_screenOverlays.erase(i);
			}
			else
			{
				// Set it to fade out
				m_screenOverlays[i].fadebegintime = -1;
				m_screenOverlays[i].fadetime = fadetime;
				m_screenOverlays[i].fadeout = true;
			}

			return;
		}
	}
}