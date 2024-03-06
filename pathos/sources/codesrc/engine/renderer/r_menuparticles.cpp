/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "window.h"
#include "system.h"
#include "config.h"
#include "enginestate.h"
#include "input.h"
#include "file.h"

#include "texturemanager.h"
#include "r_menu.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_main.h"
#include "r_common.h"
#include "r_menuparticles.h"

// Number of frames in particle texture on x axis
const Uint32 CMenuParticles::NUM_PARTICLE_FRAMES_X = 4;
// Number of frames in particle texture on y axis
const Uint32 CMenuParticles::NUM_PARTICLE_FRAMES_Y = 4;
// Menu particle max life
const Float CMenuParticles::PARTICLE_MAX_LIFE = 25;
// Particle spawn base frequency
const Uint32 CMenuParticles::PARTICLE_SPAWN_FREQ = 10;
// Particle spawn frequency variation
const Uint32 CMenuParticles::PARTICLE_SPAWN_VARIATION = 20;
// Particle fade in/fade out time
const Float CMenuParticles::PARTICLE_FADE_TIME = 0.2;

// Class object
CMenuParticles gMenuParticles;

//=============================================
// Class: CMenuParticles
// Function: CMenuParticles
//=============================================
CMenuParticles::CMenuParticles( void ):
	m_pBgAlphaTexture(nullptr),
	m_pParticleTexture(nullptr),
	m_isActive(false),
	m_lastSpawnTime(0),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_numBatchedVertexes(0)
{
}

//=============================================
// Class: CMenuParticles
// Function: ~CMenuParticles
//=============================================
CMenuParticles::~CMenuParticles( void )
{
	KillParticles();
}

//=============================================
// Class: CMenuParticles
// Function: Init
//=============================================
bool CMenuParticles::Init( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Determine which texture to use
	Float aspectRatio = (Float)gWindow.GetWidth()/(Float)gWindow.GetHeight();
	if(aspectRatio > 1.5)
		m_pBgAlphaTexture = pTextureManager->LoadTexture("menu/background_widescreen_alpha.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
	else
		m_pBgAlphaTexture = pTextureManager->LoadTexture("menu/background_alpha.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);

	if(!m_pBgAlphaTexture)
		m_pBgAlphaTexture = pTextureManager->GetDummyTexture();

	// Load particle texture
	m_pParticleTexture = pTextureManager->LoadTexture("menu/clouds.dds", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
	if(!m_pParticleTexture)
		m_pParticleTexture = pTextureManager->GetDummyTexture();

	return InitGL();
}

//=============================================
// Class: CMenuParticles
// Function: InitGL
//=============================================
bool CMenuParticles::InitGL( void )
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "menuparticles.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_vertex = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(mparticle_vertex_t), OFFSET(mparticle_vertex_t, origin));
		m_attribs.a_color = m_pShader->InitAttribute("in_color", 4, GL_FLOAT, sizeof(mparticle_vertex_t), OFFSET(mparticle_vertex_t, color));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(mparticle_vertex_t), OFFSET(mparticle_vertex_t, texcoords));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_vertex, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_color, "in_color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture1 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_texture2 = m_pShader->InitUniform("texture1", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture1, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture2, "texture1", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, m_vertexesArray, sizeof(m_vertexesArray), nullptr, 0, false);
		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//=============================================
// Class: CMenuParticles
// Function: ClearGL
//=============================================
void CMenuParticles::ClearGL( void )
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
// Class: CMenuParticles
// Function: Draw
//=============================================
bool CMenuParticles::Draw( void )
{
	if(!m_isActive)
		return true;

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pShader->EnableAttribute(m_attribs.a_vertex);
	m_pShader->EnableAttribute(m_attribs.a_color);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	m_pShader->SetUniform1i(m_attribs.u_texture1, 0);
	m_pShader->SetUniform1i(m_attribs.u_texture2, 1);

	R_Bind2DTexture(GL_TEXTURE0_ARB, m_pParticleTexture->palloc->gl_index);
	R_Bind2DTexture(GL_TEXTURE1_ARB, m_pBgAlphaTexture->palloc->gl_index);

	// Set matrices
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0f/(Float)rns.screenwidth,1.0f/(Float)rns.screenheight, 1.0);

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	// Clear this
	m_numBatchedVertexes = 0;

	m_particlesList.begin();
	while(!m_particlesList.end())
	{
		// Make sure we don't exceed the max verts
		if((m_numBatchedVertexes + 6) > MAX_PARTICLE_VERTEXES)
			break;

		mparticle_t* pParticle = m_particlesList.get();

		Float alpha = pParticle->alpha * pParticle->mainalpha;

		Float tc0x = pParticle->texcoords[0];
		Float tc1x = pParticle->texcoords[0] + ((Float)pParticle->width / (Float)m_pParticleTexture->width);
		Float tc0y = pParticle->texcoords[1];
		Float tc1y = pParticle->texcoords[1] + ((Float)pParticle->height / (Float)m_pParticleTexture->height);

		Float up = (pParticle->height * pParticle->scale) * 0.5;
		Float down = -up;

		Float right = (pParticle->width * pParticle->scale) * 0.5;
		Float left = -right;

		// First triangle
		Vector point = Vector(pParticle->origin.x + left, pParticle->origin.y + up, -1);
		BatchVertex(point, pParticle->color, alpha, tc0x, tc0y);

		point = Vector(pParticle->origin.x + right, pParticle->origin.y + up, -1);
		BatchVertex(point, pParticle->color, alpha, tc1x, tc0y);

		point = Vector(pParticle->origin.x + right, pParticle->origin.y + down, -1);
		BatchVertex(point, pParticle->color, alpha, tc1x, tc1y);

		// Second triangle
		point = Vector(pParticle->origin.x + left, pParticle->origin.y + up, -1);
		BatchVertex(point, pParticle->color, alpha, tc0x, tc0y);

		point = Vector(pParticle->origin.x + right, pParticle->origin.y + down, -1);
		BatchVertex(point, pParticle->color, alpha, tc1x, tc1y);

		point = Vector(pParticle->origin.x + left, pParticle->origin.y + down, -1);
		BatchVertex(point, pParticle->color, alpha, tc0x, tc1y);

		m_particlesList.next();
	}

	m_pVBO->VBOSubBufferData(0, m_vertexesArray, sizeof(mparticle_vertex_t)*m_numBatchedVertexes);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, m_numBatchedVertexes);

	m_pShader->DisableAttribute(m_attribs.a_vertex);
	m_pShader->DisableAttribute(m_attribs.a_color);
	m_pShader->DisableAttribute(m_attribs.a_texcoord);

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	return true;
}

//=============================================
// Class: CMenuParticles
// Function: Think
//=============================================
void CMenuParticles::Think( void )
{
	if(!m_isActive)
		return;

	Uint32 screenWidth = gWindow.GetWidth();
	Uint32 screenHeight = gWindow.GetHeight();

	// Determine number of particles to spawn based on variation
	Uint32 spawnFrequency = PARTICLE_SPAWN_FREQ + SDL_fabs(SDL_sin(ens.time)*0.5*PARTICLE_SPAWN_VARIATION);
	Double spawnInterval = 1.0f / (Double)spawnFrequency;

	// Determine how many should've been spawned since last time
	Double elapsedTime = ens.time - m_lastSpawnTime;
	Uint32 nbSpawn = elapsedTime / spawnInterval;
	if(nbSpawn > 0)
	{
		// Spawn random particles from the top of the screen
		for(Uint32 i = 0; i < nbSpawn; i++)
		{
			if(!SpawnParticle(Common::RandomFloat(0, screenWidth), 0))
				break;
		}

		m_lastSpawnTime = ens.time;
	}

	// Update existing particles
	if(m_particlesList.empty())
		return;

	m_particlesList.begin();
	while(!m_particlesList.end())
	{
		mparticle_t* pParticle = m_particlesList.get();

		// Delete expired particles
		if(pParticle->die <= ens.time)
		{
			m_particlesList.remove(m_particlesList.get_link());
			m_particlesList.next();
			continue;
		}

		// Update alpha
		if(!pParticle->noinfade && (pParticle->spawntime + PARTICLE_FADE_TIME > ens.time))
			pParticle->alpha = (ens.time - pParticle->spawntime) / PARTICLE_FADE_TIME;
		else if(ens.time > (pParticle->die - PARTICLE_FADE_TIME))
			pParticle->alpha = 1.0 - ((ens.time - (pParticle->die - PARTICLE_FADE_TIME)) / PARTICLE_FADE_TIME);
		else
			pParticle->alpha = 1.0;

		// Add in gravity and wind
		if(pParticle->wind)
		{
			Float windintensity = SDL_fabs(SDL_sin(ens.time*0.5));
			pParticle->velocity.x += pParticle->wind * windintensity * ens.frametime;
		}

		if(pParticle->gravity)
			pParticle->velocity.y += pParticle->gravity * ens.frametime * 8;

		// Move the particle
		pParticle->origin = pParticle->origin + pParticle->velocity * ens.frametime;

		// Kill particle if it disappeared off the screen
		Float particleWidth = pParticle->width * pParticle->scale;
		Float particleHeight = pParticle->height * pParticle->scale;
		if((pParticle->origin.x + particleWidth*0.5) < 0 
			|| (pParticle->origin.x - particleWidth*0.5) > screenWidth
			|| (pParticle->origin.y - particleHeight*0.5) > screenHeight)
		{
			m_particlesList.remove(m_particlesList.get_link());
			m_particlesList.next();
			continue;
		}
#if 0
		if((pParticle->origin.y) > screenHeight)
		{
			pParticle->gravity = 0;
			pParticle->wind = 0;
			pParticle->die = ens.time + PARTICLE_FADE_TIME;
			pParticle->velocity.Clear();
		}
#endif
		m_particlesList.next();
	}
}

//=============================================
// Class: CMenuParticles
// Function: StartParticles
//=============================================
void CMenuParticles::StartParticles( void )
{
	if(m_isActive)
		return;

	m_isActive = true;

	Uint32 screenWidth = gWindow.GetWidth();
	Uint32 screenHeight = gWindow.GetHeight();

	// Spawn a random amount of particles all over the screen
	Uint32 numSpawn = Common::RandomLong(40, 60);
	for(Uint32 i = 0; i < numSpawn; i++)
	{
		Float xCoord = Common::RandomLong(0, screenWidth);
		Float yCoord = Common::RandomLong(0, screenHeight);

		SpawnParticle(xCoord, yCoord, true);
	}

	m_lastSpawnTime = ens.time;
}

//=============================================
// Class: CMenuParticles
// Function: KillParticles
//=============================================
void CMenuParticles::KillParticles( void )
{
	if(!m_isActive)
		return;

	m_isActive = false;

	if(m_particlesList.empty())
		return;

	m_particlesList.begin();
	while(!m_particlesList.end())
	{
		delete m_particlesList.get();
		m_particlesList.next();
	}
	m_particlesList.clear();
}

//=============================================
// Class: CMenuParticles
// Function: SpawnParticle
//=============================================
bool CMenuParticles::SpawnParticle( Float xcoord, Float ycoord, bool noinfade )
{
	if(m_particlesList.size() >= MAX_PARTICLES)
		return false;

	mparticle_t* pParticle = new mparticle_t();
	pParticle->alpha = 1.0;
	pParticle->origin.x = xcoord;
	pParticle->origin.y = ycoord;
	pParticle->noinfade = noinfade;
	pParticle->die = ens.time + PARTICLE_MAX_LIFE;
	pParticle->color = Vector(1.0, 1.0, 1.0);
	pParticle->mainalpha = Common::RandomFloat(0.3, 0.7);
	pParticle->gravity = Common::RandomFloat(0.5, 2.0);
	pParticle->wind = Common::RandomFloat(-2.0, 15.0);

	pParticle->width = m_pParticleTexture->width / NUM_PARTICLE_FRAMES_X;
	pParticle->height = m_pParticleTexture->height / NUM_PARTICLE_FRAMES_X;

	pParticle->scale = Common::RandomFloat(0.5, 4.5);
	pParticle->spawntime = ens.time;
	pParticle->velocity.x = Common::RandomFloat(-20, 20);
	pParticle->velocity.y = Common::RandomFloat(-10, 10);

	pParticle->texcoords[0] = (pParticle->width * Common::RandomLong(0, 3))/(Float)m_pParticleTexture->width;
	pParticle->texcoords[1] = (pParticle->height * Common::RandomLong(0, 3))/(Float)m_pParticleTexture->height;

	m_particlesList.add(pParticle);
	return true;
}

//=============================================
// Class: CMenuParticles
// Function: BatchVertex
//=============================================
void CMenuParticles::BatchVertex( const Vector& origin, const Vector& color, Float alpha, Float tcx, Float tcy )
{
	if(m_numBatchedVertexes >= MAX_PARTICLE_VERTEXES)
		return;

	mparticle_vertex_t& vertex = m_vertexesArray[m_numBatchedVertexes];
	m_numBatchedVertexes++;

	Math::VectorCopy(origin, vertex.origin);
	vertex.origin[3] = 1.0;

	Math::VectorCopy(color, vertex.color);
	vertex.color[3] = alpha;

	vertex.texcoords[0] = tcx;
	vertex.texcoords[1] = tcy;
}

//=============================================
// Class: CMenuParticles
// Function: GetShaderError
//=============================================
const Char* CMenuParticles::GetShaderError( void )
{
	if(m_pShader)
		return m_pShader->GetError();
	else
		return "";
}