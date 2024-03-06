/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "r_common.h"
#include "r_main.h"
#include "texturemanager.h"
#include "cl_entity.h"
#include "system.h"
#include "enginestate.h"
#include "cl_main.h"
#include "r_blackhole.h"
#include "r_glsl.h"
#include "file.h"
#include "r_rttcache.h"
#include "r_particles.h"
#include "cl_tempentities.h"
#include "blackhole_shared.h"
#include "console.h"

// Class object
CBlackHoleRenderer gBlackHoleRenderer;

// Black hole reference size
const Float CBlackHoleRenderer::BLACK_HOLE_REFERENCE_SIZE = 1024;

//=============================================
//
//=============================================
CBlackHoleRenderer::CBlackHoleRenderer( void ):
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pCvarDrawBlackHoles(nullptr),
	m_pStrengthDebugCvar(nullptr)
{
}

//=============================================
//
//=============================================
CBlackHoleRenderer::~CBlackHoleRenderer( void )
{
	ClearGL();
}

//=============================================
//
//=============================================
bool CBlackHoleRenderer::Init( void ) 
{
	m_pCvarDrawBlackHoles = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT), "r_blackholes", "1", "Toggles black hole rendering");
	m_pStrengthDebugCvar = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT), "r_blackhole_strength", "1", "For debugging black hole strength");
	
	return true;
}

//=============================================
//
//=============================================
bool CBlackHoleRenderer::InitGL( void ) 
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "blackhole.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_vertex = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(blackhole_vertex_t), OFFSET(blackhole_vertex_t, origin));
		if(!R_CheckShaderVertexAttribute(m_attribs.a_vertex, "in_position", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);

		m_attribs.u_screensize = m_pShader->InitUniform("screensize", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_screenpos = m_pShader->InitUniform("screenpos", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_distance = m_pShader->InitUniform("distance", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_size = m_pShader->InitUniform("size", CGLSLShader::UNIFORM_FLOAT1);

		if(!R_CheckShaderUniform(m_attribs.u_screensize, "screensize", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_screenpos, "screenpos", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_distance, "distance", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_size, "size", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, m_blackHoleVertexes, sizeof(m_blackHoleVertexes), nullptr, 0, false);
		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//=============================================
//
//=============================================
void CBlackHoleRenderer::ClearGL( void ) 
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
//
//=============================================
void CBlackHoleRenderer::ClearGame( void ) 
{
	if(!m_blackHolesList.empty())
		m_blackHolesList.clear();
}

//=============================================
//
//=============================================
void CBlackHoleRenderer::Think( void ) 
{
	if(cls.paused)
		return;

	if(m_blackHolesList.empty())
		return;

	m_blackHolesList.begin();
	while(!m_blackHolesList.end())
	{
		const blackhole_t& blackhole = m_blackHolesList.get();
		if(blackhole.life == -1)
		{
			m_blackHolesList.next();
			continue;
		}

		// If black hole has expired, remove it
		if(blackhole.spawntime + blackhole.life < cls.cl_time)
			m_blackHolesList.remove(m_blackHolesList.get_link());

		m_blackHolesList.next();
	}
}

//=============================================
//
//=============================================
bool CBlackHoleRenderer::AffectObject( const Vector& origin, Vector& velocity, Float gravity )
{
	if(m_blackHolesList.empty())
		return true;

	m_blackHolesList.begin();
	while(!m_blackHolesList.end())
	{
		const blackhole_t& blackhole = m_blackHolesList.get();
		if(blackhole.life != -1 && blackhole.spawntime + blackhole.life < cls.cl_time)
		{
			m_blackHolesList.next();
			continue;
		}

		if(!blackhole.strength)
		{
			m_blackHolesList.next();
			continue;
		}

		// Use inverse square radius
		Float scale = GetBlackHoleScale(blackhole);
		Float radius = BLACK_HOLE_SIZE*scale;
		Float radiusSquared = radius*radius;
		Vector direction = blackhole.origin - origin;
		Float distance = Math::DotProduct(direction, direction);
		if(distance > radiusSquared)
		{
			m_blackHolesList.next();
			continue;
		}

		Float attenuation = ((distance/radiusSquared) - 1.0) * -1.0;
		attenuation = clamp(attenuation, 0.0, 1.0);

		// Calculate strength of pull by black hole
		Float pullStrength = attenuation*BLACK_HOLE_SUCK_SPEED*blackhole.strength*m_pStrengthDebugCvar->GetValue();
		direction.Normalize();

		Vector velocityDirection = velocity;
		velocityDirection.Normalize();

		Vector awayDirection = velocityDirection - direction;
		awayDirection.Normalize();

		// Remove from velocity slowly the direction pointing away from the hole
		velocity = velocity - awayDirection*pullStrength*cls.frametime;

		if(blackhole.rotation)
		{
			Vector right, up;
			Math::GetUpRight(direction, up, right);

			Float orbitAtten = radiusSquared/(distance*sqrt(distance));
			orbitAtten = clamp(orbitAtten, 0.0, 1.0);

			// Orbit direction should pull towards the black hole the closer we are
			velocity = velocity + right * BLACK_HOLE_SUCK_SPEED * blackhole.rotation * (1.0 - orbitAtten) * cls.frametime;
		}

		// Add velocity moving towards black hole
		velocity = velocity + direction*pullStrength*cls.frametime;

		// Kill it if it's too close
		if( sqrt(distance) < BLACK_HOLE_KILL_DISTANCE*scale )
			return false;

		m_blackHolesList.next();
	}

	return true;
}

//=============================================
//
//=============================================
void CBlackHoleRenderer::CreateBlackHole( Int32 key, const Vector& origin, Float life, Float scale, Float strength, Float rotation, Float growthtime, Float shrinktime )
{
	// Try to find a black hole with the same key if key is a non-zero value
	blackhole_t* pblackhole = nullptr;
	if(key != 0 && !m_blackHolesList.empty())
	{
		m_blackHolesList.begin();
		while(!m_blackHolesList.end())
		{
			blackhole_t& blackhole = m_blackHolesList.get();
			if(blackhole.key == key)
			{
				pblackhole = &blackhole;
				break;
			}

			m_blackHolesList.next();
		}
	}

	// If none was found, spawn a new one
	if(!pblackhole)
		pblackhole = &m_blackHolesList.add(blackhole_t())->_val;

	// Fill in the values
	pblackhole->key = key;
	pblackhole->origin = origin;
	pblackhole->life = life;
	pblackhole->scale = scale;
	pblackhole->strength = strength;
	pblackhole->growthtime = growthtime;
	pblackhole->shrinktime = shrinktime;
	pblackhole->rotation = rotation;

	// Mark time of spawn
	pblackhole->spawntime = cls.cl_time;
}

//=============================================
//
//=============================================
void CBlackHoleRenderer::KillBlackHole( Int32 key )
{
	if(!key)
		return;

	m_blackHolesList.begin();
	while(!m_blackHolesList.end())
	{
		blackhole_t& blackhole = m_blackHolesList.get();
		if(blackhole.key == key)
		{
			if(blackhole.shrinktime)
			{
				// Modify the life so it fades out
				blackhole.life = (rns.time - blackhole.spawntime) + blackhole.shrinktime;
			}
			else
			{
				// Otherwise, just kill it
				m_blackHolesList.remove(m_blackHolesList.get_link());
			}
		}

		m_blackHolesList.next();
	}
}

//=============================================
//
//=============================================
Float CBlackHoleRenderer::GetBlackHoleScale( const blackhole_t& blackhole ) const
{
	Float scale = blackhole.scale;

	// Calculate growth period
	if(blackhole.growthtime && cls.cl_time < (blackhole.spawntime + blackhole.growthtime))
	{
		Float growthFactor = (cls.cl_time - blackhole.spawntime) / blackhole.growthtime;
		if(growthFactor > 1.0)
			growthFactor = 1.0;
		else if(growthFactor > 1.0)
			growthFactor = 1.0;

		scale *= growthFactor;
	}

	// Calculate shrinking
	if(blackhole.life != -1 && blackhole.shrinktime)
	{
		Double shrinkBeginTime = (blackhole.spawntime + blackhole.life - blackhole.shrinktime);
		if(shrinkBeginTime < cls.cl_time)
		{
			Float shrinkFactor = (cls.cl_time - shrinkBeginTime) / blackhole.shrinktime;
			if(shrinkFactor < 0)
				shrinkFactor = 0;
			else if(shrinkFactor > 1.0)
				shrinkFactor = 1.0;

			scale *= (1.0 - shrinkFactor);
		}
	}

	return scale;
}

//=============================================
//
//=============================================
bool CBlackHoleRenderer::DrawBlackHoles( void )
{
	if(m_pCvarDrawBlackHoles->GetValue() < 1)
		return true;

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);

	m_pShader->EnableAttribute(m_attribs.a_vertex);

	m_pShader->SetUniform1i(m_attribs.u_texture, 0);
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniform2f(m_attribs.u_screensize, rns.screenwidth, rns.screenheight);

	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	rtt_texture_t* pTexture = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

	m_blackHolesList.begin();
	while(!m_blackHolesList.end())
	{
		const blackhole_t& blackhole = m_blackHolesList.get();
		m_blackHolesList.next();

		const Vector& origin = blackhole.origin;
		Float vOrigin [] = { origin[0], origin[1], origin[2], 1.0 };

		// Multiply with modelview
		Float viewPos[4];
		Math::MatMult4(rns.view.modelview.Transpose(), vOrigin, viewPos);

		// Multiply with projection
		Float screenCoords[4];
		Math::MatMult4(rns.view.projection.Transpose(), viewPos, screenCoords);

		// See if it's z-clipped
		if(screenCoords[3] <= 0)
			continue;

		// Get current screen contents
		R_BindRectangleTexture(GL_TEXTURE0_ARB, pTexture->palloc->gl_index);
		glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

		// Calculate uniform values
		Float distance = (rns.view.v_origin-origin).Length();
		Float coordX = (screenCoords[0]/screenCoords[3])*0.5 + 0.5;
		Float coordY = (screenCoords[1]/screenCoords[3])*0.5 + 0.5;

		// Calculate size based on time from spawn/time to death
		Float scale = GetBlackHoleScale(blackhole);

		m_pShader->SetUniform2f(m_attribs.u_screenpos, coordX, coordY);
		m_pShader->SetUniform1f(m_attribs.u_distance, distance);
		m_pShader->SetUniform1f(m_attribs.u_size, scale);

		// Triangle 1
		Vector vertexOrigin;
		vertexOrigin = origin - rns.view.v_forward * 0.01 + rns.view.v_up * scale * BLACK_HOLE_REFERENCE_SIZE;
		vertexOrigin = vertexOrigin - rns.view.v_right * scale * BLACK_HOLE_REFERENCE_SIZE;
		Math::VectorCopy(vertexOrigin, m_blackHoleVertexes[0].origin);
		m_blackHoleVertexes[0].origin[3] = 1.0;

		vertexOrigin = origin - rns.view.v_forward * 0.01 + rns.view.v_up * scale * BLACK_HOLE_REFERENCE_SIZE;
		vertexOrigin = vertexOrigin + rns.view.v_right * scale * BLACK_HOLE_REFERENCE_SIZE;
		Math::VectorCopy(vertexOrigin, m_blackHoleVertexes[1].origin);
		m_blackHoleVertexes[1].origin[3] = 1.0;

		vertexOrigin = origin - rns.view.v_forward * 0.01 - rns.view.v_up * scale * BLACK_HOLE_REFERENCE_SIZE;
		vertexOrigin = vertexOrigin + rns.view.v_right * scale * BLACK_HOLE_REFERENCE_SIZE;
		Math::VectorCopy(vertexOrigin, m_blackHoleVertexes[2].origin);
		m_blackHoleVertexes[2].origin[3] = 1.0;

		// Triangle 2
		vertexOrigin = origin - rns.view.v_forward * 0.01 + rns.view.v_up * scale * BLACK_HOLE_REFERENCE_SIZE;
		vertexOrigin = vertexOrigin - rns.view.v_right * scale * BLACK_HOLE_REFERENCE_SIZE;
		Math::VectorCopy(vertexOrigin, m_blackHoleVertexes[3].origin);
		m_blackHoleVertexes[3].origin[3] = 1.0;

		vertexOrigin = origin - rns.view.v_forward * 0.01 - rns.view.v_up * scale * BLACK_HOLE_REFERENCE_SIZE;
		vertexOrigin = vertexOrigin + rns.view.v_right * scale * BLACK_HOLE_REFERENCE_SIZE;
		Math::VectorCopy(vertexOrigin, m_blackHoleVertexes[4].origin);
		m_blackHoleVertexes[4].origin[3] = 1.0;

		vertexOrigin = origin - rns.view.v_forward * 0.01 - rns.view.v_up * scale * BLACK_HOLE_REFERENCE_SIZE;
		vertexOrigin = vertexOrigin - rns.view.v_right * scale * BLACK_HOLE_REFERENCE_SIZE;
		Math::VectorCopy(vertexOrigin, m_blackHoleVertexes[5].origin);
		m_blackHoleVertexes[5].origin[3] = 1.0;

		m_pVBO->VBOSubBufferData(0, m_blackHoleVertexes, sizeof(blackhole_vertex_t)*NUM_BLACKHOLE_VERTEXES);

		R_ValidateShader(m_pShader);

		glDrawArrays(GL_TRIANGLES, 0, NUM_BLACKHOLE_VERTEXES);
	}

	gRTTCache.Free(pTexture);

	gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	glDepthMask(GL_TRUE);

	return true;
}