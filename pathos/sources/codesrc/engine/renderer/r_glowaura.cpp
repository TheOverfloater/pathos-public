/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "file.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "cvar.h"
#include "r_main.h"
#include "cl_entity.h"
#include "cache_model.h"
#include "textures_shared.h"
#include "r_glowaura.h"
#include "r_vbm.h"
#include "r_bsp.h"
#include "r_common.h"
#include "console.h"
#include "constants.h"
#include "system.h"
#include "r_vbm.h"

// Radius of aura effect
const Float CGlowAura::AURA_RADIUS = 256;
// Maximum aura resolution
const Uint32 CGlowAura::AURA_RESOLUTION = 512;

// Class define
CGlowAura gGlowAura;

//====================================
//
//====================================
CGlowAura::CGlowAura( void ):
	m_iNumEntities(0),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pScreenRTT(nullptr),
	m_pWhiteRTT(nullptr),
	m_pColorsRTT(nullptr),
	m_pBlurRTT(nullptr),
	m_pCvarGlowAura(nullptr)
{
	memset(m_pEntities, 0, sizeof(m_pEntities));
}

//====================================
//
//====================================
CGlowAura::~CGlowAura( void )
{
	ClearGL();
}

//====================================
//
//====================================
bool CGlowAura::Init( void )
{
	m_pCvarGlowAura = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_glowaura", "1", "Toggles rendering of auras around objects.");

	return true;
}

//====================================
//
//====================================
void CGlowAura::Shutdown( void )
{
	ClearGL();
}

//====================================
//
//====================================
bool CGlowAura::InitGL( void )
{
	// Make sure this is called
	ClearGL();

	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "glowaura.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to load shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(aura_vertex_t), OFFSET(aura_vertex_t, origin));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(aura_vertex_t), OFFSET(aura_vertex_t, texcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture0 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_scrntexture = m_pShader->InitUniform("scrntexture", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_size = m_pShader->InitUniform("size", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_screensize = m_pShader->InitUniform("screensize", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture0, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scrntexture, "scrntexture", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_size, "size", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screensize, "screensize", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_type = m_pShader->GetDeterminatorIndex("type");

		if(!R_CheckShaderDeterminator(m_attribs.d_type, "type", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(!m_pVBO)
	{
		// create VBO
		aura_vertex_t *pverts = new aura_vertex_t[6];

		// Normal quad
		Uint32 numverts = 0;
		pverts[numverts].origin[0] = 0; pverts[numverts].origin[1] = 1; 
		pverts[numverts].origin[2] = -1; pverts[numverts].origin[3] = 1;
		pverts[numverts].texcoord[0] = 0; pverts[numverts].texcoord[1] = 0;
		numverts++;

		pverts[numverts].origin[0] = 0; pverts[numverts].origin[1] = 0; 
		pverts[numverts].origin[2] = -1; pverts[numverts].origin[3] = 1;
		pverts[numverts].texcoord[0] = 0; pverts[numverts].texcoord[1] = 1;
		numverts++;

		pverts[numverts].origin[0] = 1; pverts[numverts].origin[1] = 0; 
		pverts[numverts].origin[2] = -1; pverts[numverts].origin[3] = 1;
		pverts[numverts].texcoord[0] = 1; pverts[numverts].texcoord[1] = 1;
		numverts++;

		pverts[numverts].origin[0] = 0; pverts[numverts].origin[1] = 1; 
		pverts[numverts].origin[2] = -1; pverts[numverts].origin[3] = 1;
		pverts[numverts].texcoord[0] = 0; pverts[numverts].texcoord[1] = 0;
		numverts++;

		pverts[numverts].origin[0] = 1; pverts[numverts].origin[1] = 0; 
		pverts[numverts].origin[2] = -1; pverts[numverts].origin[3] = 1;
		pverts[numverts].texcoord[0] = 1; pverts[numverts].texcoord[1] = 1;
		numverts++;

		pverts[numverts].origin[0] = 1; pverts[numverts].origin[1] = 1; 
		pverts[numverts].origin[2] = -1; pverts[numverts].origin[3] = 1;
		pverts[numverts].texcoord[0] = 1; pverts[numverts].texcoord[1] = 0;
		numverts++;

		m_pVBO = new CVBO(gGLExtF, pverts, sizeof(aura_vertex_t)*numverts, nullptr, 0);
		m_pShader->SetVBO(m_pVBO);
		delete[] pverts;
	}

	return true;
}

//====================================
//
//====================================
void CGlowAura::ClearGL( void )
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

//====================================
//
//====================================
void CGlowAura::GetEntities( void )
{
	m_iNumEntities = 0;

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pentity = rns.objects.pvisents[i];

		if(!pentity->pmodel)
			continue;

		if(m_iNumEntities == MAX_AURA_ENTITIES)
			return;

		if(pentity->pmodel->type != MOD_VBM)
			continue;

		if(pentity->curstate.renderfx != RenderFx_GlowAura 
			&& pentity->curstate.renderfx != RenderFx_Diary)
			continue;

		if((pentity->curstate.origin - rns.view.v_origin).Length() < AURA_RADIUS)
		{
			m_pEntities[m_iNumEntities] = pentity;
			m_iNumEntities++;
		}
	}
}

//====================================
//
//====================================
bool CGlowAura::DrawSolid( void )
{
	// Clear the color buffer and begin rendering
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);

	glPolygonOffset(-1, -1);
	glEnable(GL_POLYGON_OFFSET_FILL);

	for(Uint32 i = 0; i < m_iNumEntities; i++)
	{
		if(!gVBMRenderer.DrawAura(m_pEntities[i], Vector(1.0, 1.0, 1.0), 1.0))
		{
			Sys_ErrorPopup("Rendering error: %s.", gVBMRenderer.GetShaderErrorString());
			gVBMRenderer.FinishAuraPass();
			return false;
		}
	}

	// Save it to the white RTT
	m_pWhiteRTT = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pWhiteRTT->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

	return true;
}

//====================================
//
//====================================
bool CGlowAura::DrawColors( void )
{
	// Now clear again and draw the colors
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(Uint32 i = 0; i < m_iNumEntities; i++)
	{
		Float alpha = 1.0;
		Float dist = (rns.view.v_origin-m_pEntities[i]->curstate.origin).Length();
		if(dist >= 172) 
		{
			alpha = 1.0-((dist-172.0f)/64.0f);
		}

		if(alpha < 0)
			alpha = 0;

		Vector color;
		if(m_pEntities[i]->curstate.rendercolor.x || m_pEntities[i]->curstate.rendercolor.y || m_pEntities[i]->curstate.rendercolor.z)
		{
			color[0] = (Float)m_pEntities[i]->curstate.rendercolor.x/255.0f;
			color[1] = (Float)m_pEntities[i]->curstate.rendercolor.y/255.0f;
			color[2] = (Float)m_pEntities[i]->curstate.rendercolor.z/255.0f;
		}
		else
		{
			color[0] = 0.1;
			color[1] = 0.25;
			color[2] = 0.9;
		}

		if(!gVBMRenderer.DrawAura(m_pEntities[i], color, alpha))
		{
			Sys_ErrorPopup("Rendering error: %s.", gVBMRenderer.GetShaderErrorString());
			gVBMRenderer.FinishAuraPass();
			return false;
		}
	}

	glDisable(GL_BLEND);
	glDisable(GL_POLYGON_OFFSET_FILL);	

	gVBMRenderer.FinishAuraPass();

	// Save it to the color RTT
	m_pColorsRTT = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pColorsRTT->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

	return true;
}

//====================================
//
//====================================
bool CGlowAura::BlurTexture( void )
{
	// Resize the color texture to the proper resolution
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, glowaura_texture))
		return false;

	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pColorsRTT->palloc->gl_index);
	m_pShader->SetUniform1i(m_attribs.u_scrntexture, 0);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Allocate the blur 1 target and fetch to it
	m_pBlurRTT = gRTTCache.Alloc(AURA_RESOLUTION, AURA_RESOLUTION);
	R_Bind2DTexture(GL_TEXTURE0_ARB, m_pBlurRTT->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, AURA_RESOLUTION, AURA_RESOLUTION, 0);

	//
	// Blur the resized texture vertically
	//
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, glowaura_blur_v))
		return false;

	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Fetch to the RTT
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	R_Bind2DTexture(GL_TEXTURE0_ARB, m_pBlurRTT->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, AURA_RESOLUTION, AURA_RESOLUTION, 0);

	//
	// Blur the resized texture horizontally
	//
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);

	m_pShader->SetDeterminator(m_attribs.d_type, glowaura_blur_h);
	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Bind it to the blur1 target
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	R_Bind2DTexture(GL_TEXTURE0_ARB, m_pBlurRTT->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, AURA_RESOLUTION, AURA_RESOLUTION, 0);

	return true;
}

//====================================
//
//====================================
bool CGlowAura::DrawFinal( void )
{
	// Enable rectangles, GLSL needs it
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE);

	//
	// Restore the screen contents
	//
	if(!m_pShader->SetDeterminator(m_attribs.d_type, glowaura_texture))
		return false;

	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);
	m_pShader->SetUniform1i(m_attribs.u_scrntexture, 0);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	//
	// Draw the auras
	//
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, glowaura_combine))
		return false;

	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_pWhiteRTT->palloc->gl_index);
	m_pShader->SetUniform1i(m_attribs.u_scrntexture, 0);

	R_Bind2DTexture(GL_TEXTURE1_ARB, m_pBlurRTT->palloc->gl_index);
	m_pShader->SetUniform1i(m_attribs.u_texture0, 1);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);

	return true;
}

//====================================
//
//====================================
bool CGlowAura::DrawAuras( void )
{
	if(m_pCvarGlowAura->GetValue() < 1)
		return true;

	// Get entity list
	GetEntities();

	if(!m_iNumEntities)
		return true;

	// Enable the rectangle texture
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE);

	// Save the screen texture and clear the color buffer
	m_pScreenRTT = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);
	R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenRTT->palloc->gl_index);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);

	// Prep the vbm renderer
	if(!gVBMRenderer.PrepAuraPass())
	{
		Sys_ErrorPopup("Rendering error: %s.", gVBMRenderer.GetShaderErrorString());
		gRTTCache.Free(m_pScreenRTT);
		glDisable(GL_TEXTURE_RECTANGLE);
		glDepthMask(GL_TRUE);
		return false;
	}

	// Draw solid white colors
	if(!DrawSolid())
	{
		gRTTCache.Free(m_pScreenRTT);
		glDisable(GL_TEXTURE_RECTANGLE);
		glDepthMask(GL_TRUE);
		return false;
	}

	// Draw colors now
	if(!DrawColors())
	{
		gRTTCache.Free(m_pScreenRTT);
		glDisable(GL_TEXTURE_RECTANGLE);
		glDepthMask(GL_TRUE);
		return false;
	}

	// Begin rendering
	glDisable(GL_DEPTH_TEST);

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
		return false;

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	if(!m_pShader->SetDeterminator(m_attribs.d_type, glowaura_texture))
	{
		m_pShader->DisableShader();
		return false;
	}

	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);
	m_pShader->SetUniform1i(m_attribs.u_scrntexture, 0);
	m_pShader->SetUniform1f(m_attribs.u_size, AURA_RESOLUTION);
	m_pShader->SetUniform2f(m_attribs.u_screensize, rns.screenwidth, rns.screenheight);

	rns.view.modelview.PushMatrix();
	rns.view.modelview.LoadIdentity();

	rns.view.projection.PushMatrix();
	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	rns.view.modelview.PopMatrix();
	rns.view.projection.PopMatrix();

	// Resize and blur
	glViewport(0, 0, AURA_RESOLUTION, AURA_RESOLUTION);
	if(!BlurTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", m_pShader->GetError());
		m_pShader->DisableShader();
		return false;
	}

	// Restore color buffer and finalize
	glViewport(0, 0, rns.screenwidth, rns.screenheight);
	if(!DrawFinal())
	{
		Sys_ErrorPopup("Shader error: %s.\n", m_pShader->GetError());
		m_pShader->DisableShader();
		return false;
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Disable the rectangle texture
	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_RECTANGLE);

	// Free the RTT textures for reuse
	if(m_pScreenRTT)
		gRTTCache.Free(m_pScreenRTT);

	if(m_pWhiteRTT)
		gRTTCache.Free(m_pWhiteRTT);

	if(m_pColorsRTT)
		gRTTCache.Free(m_pColorsRTT);

	if(m_pBlurRTT)
		gRTTCache.Free(m_pBlurRTT);

	// Clear any binds
	R_ClearBinds();

	return true;
}
