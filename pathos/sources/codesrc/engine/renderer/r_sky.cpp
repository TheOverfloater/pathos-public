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
#include "r_sky.h"
#include "com_math.h"
#include "system.h"
#include "brushmodel.h"
#include "enginestate.h"
#include "file.h"
#include "r_common.h"
#include "cvar.h"
#include "r_basic_vertex.h"
#include "console.h"
#include "cl_main.h"
#include "texturemanager.h"
#include "r_bsp.h"
#include "r_vbm.h"
#include "r_water.h"
#include "r_particles.h"

// Skybox surface distance
const Float CSkyRenderer::SKYBOX_SURFACE_DISTANCE = 128;
// Skybox texture base directory
const Char CSkyRenderer::SKYBOX_TEXTURE_DIR[] = "sky/";
// Skybox texture postfixes
const Char* CSkyRenderer::SKY_TEXTURE_POSTFIXES[NB_SKYBOX_TEXTURES] = {"lf", "bk", "rt", "ft", "dn", "up"};

// Object definition
CSkyRenderer gSkyRenderer;

//====================================
//
//====================================
CSkyRenderer::CSkyRenderer( void ):
	m_skyIndexBase(0),
	m_screenQuadBase(0),
	m_pCvarDrawSky(nullptr),
	m_currentSkySet(NO_POSITION),
	m_skyBoxSkySet(NO_POSITION),
	m_skySetUsed(NO_POSITION),
	m_pShader(nullptr),
	m_pVBO(nullptr)
{
	for(Uint32 i = 0; i < NB_SKYBOX_TEXTURES; i++)
		m_pSkyboxTextures[i] = nullptr;
}

//====================================
//
//====================================
CSkyRenderer::~CSkyRenderer( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CSkyRenderer::Init( void )
{
	m_pCvarDrawSky = gConsole.CreateCVar( CVAR_FLOAT, FL_CV_CLIENT, "r_drawsky", "1", "Toggle sky rendering." );

	return true;
}

//====================================
//
//====================================
void CSkyRenderer::Shutdown( void )
{
	ClearGL();
	ClearGame();
}

//====================================
//
//====================================
bool CSkyRenderer::InitGL( void )
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "sky.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, origin));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, texcoords));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_color, "color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_mode = m_pShader->GetDeterminatorIndex("mode");

		if(!R_CheckShaderDeterminator(m_attribs.d_mode, "mode", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(m_pVBO)
	{
		// Rebind us
		m_pVBO->RebindGL();
		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//====================================
//
//====================================
void CSkyRenderer::ClearGL( void )
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(m_pVBO)
		m_pVBO->ClearGL();
}

//====================================
//
//====================================
bool CSkyRenderer::InitGame( void )
{
	// See if we have any sky brushes at all
	for(Uint32 i = 0; i < ens.pworld->numsurfaces; i++)
	{
		const msurface_t* psurface = &ens.pworld->psurfaces[i];
		if(psurface->flags & SURF_DRAWSKY)
		{
			rns.sky.drawsky = true;
			break;
		}
	}

	if(!rns.sky.drawsky)
		return true;

	// Load base sky set
	LoadSkyTextures(cls.skyname.c_str(), m_pSkyboxTextures);

	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, true, true);
		m_pShader->SetVBO(m_pVBO);
	}

	// Create VBO
	CreateVBO();

	return true;
}

//====================================
//
//====================================
void CSkyRenderer::ClearGame( void )
{
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

	m_screenQuadBase = 0;
	m_skyIndexBase = 0;

	for(Uint32 i = 0; i < NB_SKYBOX_TEXTURES; i++)
		m_pSkyboxTextures[i] = nullptr;

	// Clear sky sets
	if(!m_skySetsArray.empty())
		m_skySetsArray.clear();

	m_currentSkySet = NO_POSITION;
	m_skyBoxSkySet = NO_POSITION;
	m_skySetUsed = NO_POSITION;
}

//====================================
//
//====================================
void CSkyRenderer::CreateVBO( void )
{
	// add in the screen quad and sky cube
	Uint32 numVertexes = 4 + 24;
	Uint32 numIndexes = 6 + 36;

	Uint32 curVertexIndex = 0;
	Uint32 curIndex = 0;

	// Set base
	m_skyIndexBase = curIndex;

	basic_vertex_t* pvertexes = new basic_vertex_t[numVertexes];
	memset(pvertexes, 0, sizeof(basic_vertex_t)*numVertexes);

	Uint32* pindexes = new Uint32[numIndexes];
	memset(pindexes, 0, sizeof(Uint32)*numIndexes);

	Vector skyboxVerts[] = 
	{
		Vector(-SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE), 
		Vector(SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE), 
		Vector(SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE), 
		Vector(-SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE),
		Vector(-SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE), 
		Vector(SKYBOX_SURFACE_DISTANCE, -SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE), 
		Vector(SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE), 
		Vector(-SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE, SKYBOX_SURFACE_DISTANCE)
	};

	// Set indexes
	Uint32 skyboxIndexes[6][4] = { {1, 2, 6, 5},{2, 3, 7, 6},{3, 0, 4, 7}, {0, 1, 5, 4},{2, 1, 0, 3},{7, 4, 5, 6}};
	for (Uint32 i = 0; i < 6; i++)
	{
		pindexes[curIndex] = curVertexIndex; pindexes[curIndex+1] = curVertexIndex+1; pindexes[curIndex+2] = curVertexIndex+2;
		pindexes[curIndex+3] = curVertexIndex; pindexes[curIndex+4] = curVertexIndex+2; pindexes[curIndex+5] = curVertexIndex+3;
		curIndex += 6;

		pvertexes[curVertexIndex].texcoords[0] = 0; pvertexes[curVertexIndex].texcoords[1] = 1;
		for(Uint32 j = 0; j < 3; j++)
			pvertexes[curVertexIndex].origin[j] = skyboxVerts[skyboxIndexes[i][0]][j];

		pvertexes[curVertexIndex].origin[3] = 1.0; 
		curVertexIndex++;

		pvertexes[curVertexIndex].texcoords[0] = 1; pvertexes[curVertexIndex].texcoords[1] = 1;
		for(Uint32 j = 0; j < 3; j++)
			pvertexes[curVertexIndex].origin[j] = skyboxVerts[skyboxIndexes[i][1]][j];

		pvertexes[curVertexIndex].origin[3] = 1.0; 
		curVertexIndex++;

		pvertexes[curVertexIndex].texcoords[0] = 1; pvertexes[curVertexIndex].texcoords[1] = 0;
		for(Uint32 j = 0; j < 3; j++)
			pvertexes[curVertexIndex].origin[j] = skyboxVerts[skyboxIndexes[i][2]][j];

		pvertexes[curVertexIndex].origin[3] = 1.0; 
		curVertexIndex++;

		pvertexes[curVertexIndex].texcoords[0] = 0; pvertexes[curVertexIndex].texcoords[1] = 0;
		for(Uint32 j = 0; j < 3; j++)
			pvertexes[curVertexIndex].origin[j] = skyboxVerts[skyboxIndexes[i][3]][j];

		pvertexes[curVertexIndex].origin[3] = 1.0; 
		curVertexIndex++;
	}

	// Create screen quad
	basic_vertex_t* pquadverts = pvertexes + curVertexIndex;
	m_screenQuadBase = curIndex;
	
	pindexes[curIndex] = curVertexIndex; pindexes[curIndex+1] = curVertexIndex+1; pindexes[curIndex+2] = curVertexIndex+2;
	pindexes[curIndex+3] = curVertexIndex; pindexes[curIndex+4] = curVertexIndex+2; pindexes[curIndex+5] = curVertexIndex+3;

	curVertexIndex += 4;
	curIndex += 6;

	pquadverts[0].origin[0] = 0; pquadverts[0].origin[1] = 1; 
	pquadverts[0].origin[2] = -1; pquadverts[0].origin[3] = 1;

	pquadverts[1].origin[0] = 0; pquadverts[1].origin[1] = 0; 
	pquadverts[1].origin[2] = -1; pquadverts[1].origin[3] = 1;

	pquadverts[2].origin[0] = 1; pquadverts[2].origin[1] = 0; 
	pquadverts[2].origin[2] = -1; pquadverts[2].origin[3] = 1;

	pquadverts[3].origin[0] = 1; pquadverts[3].origin[1] = 1; 
	pquadverts[3].origin[2] = -1; pquadverts[3].origin[3] = 1;

	m_pVBO->Append(pvertexes, sizeof(basic_vertex_t)*numVertexes, pindexes, sizeof(Uint32)*numIndexes);

	delete[] pvertexes;
	delete[] pindexes;
}

//====================================
//
//====================================
void CSkyRenderer::PreFrame( void )
{
	m_currentSkySet = m_skySetUsed;
}

//====================================
//
//====================================
bool CSkyRenderer::DrawSky( void )
{
	if(!rns.sky.drawsky || (m_pCvarDrawSky->GetValue() < 1)
		|| (!rns.sky.drawsky && !rns.sky.skybox)
		|| (rns.fog.settings.active && rns.fog.settings.affectsky 
		&& !rns.fog.blendtime && !rns.sky.skybox))
	{
		// If skybox is set, the renderer expects us to 
		// restore the leaves here
		if(rns.sky.skybox)
		{
			// Restore VIS to what we want to render
			Vector vieworigin;
			if(rns.usevisorigin)
				vieworigin = rns.view.v_visorigin;
			else
				vieworigin = rns.view.v_origin;

			R_MarkLeaves(vieworigin);
		}

		return true;
	}

	Vector vsavedorigin;
	fog_settings_t savedfog;
	bool drawskybox = false;

	if(rns.sky.skybox && !rns.fog.settings.affectsky && !rns.water_skydraw)
	{
		Vector vorigin;
		Math::VectorCopy(rns.view.v_origin, vsavedorigin);

		// We'll need to draw
		drawskybox = true;

		// Set up sky origin
		if(rns.sky.skysize)
		{
			Vector vtemp;
			Math::VectorSubtract(rns.view.v_origin, rns.sky.world_origin, vtemp);
			Math::VectorMA(rns.sky.local_origin, 1.0f/rns.sky.skysize, vtemp, vorigin);
		}
		else
		{
			Math::VectorCopy(rns.sky.local_origin, vorigin);
		}

		// Mark leaves seen by sky
		R_MarkLeaves(rns.sky.local_origin);

		// Set up basic shit
		R_SetFrustum(rns.view.frustum, vorigin, rns.view.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);
		Math::VectorCopy(vorigin, rns.view.v_origin);

		if(rns.sky.fog.active)
		{
			savedfog = rns.fog.settings;
			rns.fog.settings = rns.sky.fog;
			
			// Clear to current color
			glClearColor(rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2], GL_ONE);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	if ((!rns.fog.settings.active || !rns.fog.settings.affectsky || rns.fog.blendtime 
		&& (!rns.fog.blend1.affectsky || !rns.fog.blend2.affectsky))
		&& (!rns.sky.fog.active || !rns.sky.fog.affectsky))
	{
		m_pVBO->Bind();
		if(!m_pShader->EnableShader())
		{
			Sys_ErrorPopup("Shader error: %s.\n", m_pShader->GetError());
			return false;
		}

		m_pShader->EnableAttribute(m_attribs.a_origin);
		m_pShader->EnableAttribute(m_attribs.a_texcoord);

		if(rns.mirroring)
		{
			rns.view.projection.PushMatrix();
			R_SetProjectionMatrix(rns.view.nearclip, rns.view.fov);
		}

		rns.view.modelview.PushMatrix();
		rns.view.modelview.LoadIdentity();
		rns.view.modelview.Rotate(-90,  1, 0, 0);// put X going down
		rns.view.modelview.Rotate(90,  0, 0, 1); // put Z going up
		rns.view.modelview.Rotate(-rns.view.v_angles[2], 1, 0, 0);
		rns.view.modelview.Rotate(-rns.view.v_angles[0], 0, 1, 0);
		rns.view.modelview.Rotate(-rns.view.v_angles[1], 0, 0, 1);

		m_pShader->SetUniform1i(m_attribs.u_texture, 0);
		m_pShader->EnableAttribute(m_attribs.a_texcoord);
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

		if(!m_pShader->SetDeterminator(m_attribs.d_mode, SHADER_TEXTURE))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			return false;
		}
		
		m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

		R_ValidateShader(m_pShader);

		en_texture_t** pTexturesArray = nullptr;
		if(drawskybox && m_skyBoxSkySet != NO_POSITION 
			&& m_skyBoxSkySet >= 0 
			&& m_skyBoxSkySet < m_skySetsArray.size())
			pTexturesArray = m_skySetsArray[m_skyBoxSkySet].ptextures;
		else if(m_currentSkySet != NO_POSITION 
			&& m_currentSkySet >= 0 
			&& m_currentSkySet < m_skySetsArray.size())
			pTexturesArray = m_skySetsArray[m_currentSkySet].ptextures;
		else
			pTexturesArray = m_pSkyboxTextures;

		glDepthMask(GL_FALSE);
		for (Uint32 i = 0; i < 6; i++)
		{
			R_Bind2DTexture(GL_TEXTURE0, pTexturesArray[i]->palloc->gl_index);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, BUFFER_OFFSET(m_skyIndexBase+i*6));
		}
		glDepthMask(GL_TRUE);

		if(rns.mirroring)
		{
			rns.view.projection.PopMatrix();
			m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
		}

		rns.view.modelview.PopMatrix();

		m_pShader->DisableAttribute(m_attribs.a_texcoord);
		m_pShader->DisableAttribute(m_attribs.a_origin);

		// Disable shader and VBO
		m_pShader->DisableShader();
		m_pVBO->UnBind();
	}

	if(drawskybox && !rns.water_skydraw)
	{
		rns.view.modelview.PushMatrix();
		rns.view.modelview.LoadIdentity();
		rns.view.modelview.Rotate(-90,  1, 0, 0);// put X going down
		rns.view.modelview.Rotate(90,  0, 0, 1); // put Z going up
		rns.view.modelview.Rotate(-rns.view.v_angles[2], 1, 0, 0);
		rns.view.modelview.Rotate(-rns.view.v_angles[0], 0, 1, 0);
		rns.view.modelview.Rotate(-rns.view.v_angles[1], 0, 0, 1);
		rns.view.modelview.Translate(-rns.view.v_origin[0], -rns.view.v_origin[1], -rns.view.v_origin[2]);

		// Draw world skybox objects
		if(!gBSPRenderer.DrawSkyBox(false))
			return false;

		//Render all skybox prop entities
		if(!gVBMRenderer.DrawSky())
			return false;

		// Draw any water brushes
		if(!gWaterShader.DrawWater( true ))
			return false;

		//Render any skybox-only particles
		if(!gParticleEngine.DrawParticles(PARTICLES_SKY))
			return false;

		//Clear depth buffer for the final time
		glClear(GL_DEPTH_BUFFER_BIT);

		// Draw world skybox objects that are kept in Z buffer
		if(!gBSPRenderer.DrawSkyBox(true))
			return false;

		// restore modelview
		rns.view.modelview.PopMatrix();

		// Raise framecount
		cls.framecount++;

		// Restore original fog
		if(rns.sky.fog.active)
			rns.fog.settings = savedfog;

		Math::VectorCopy(vsavedorigin, rns.view.v_origin);

		// Restore VIS to what we want to render
		Vector vieworigin;
		if(rns.usevisorigin)
			vieworigin = rns.view.v_visorigin;
		else
			vieworigin = rns.view.v_origin;

		R_MarkLeaves(vieworigin);
	}

	// Lastly, draw a fullscreen square for fog blending
	if(rns.fog.blendtime && rns.sky.skybox)
	{
		if(!rns.fog.blend1.affectsky && !rns.fog.blend2.affectsky)
			return true;

		if(rns.fog.blend1.affectsky && rns.fog.blend2.affectsky)
			return true;

		Float flFrac = rns.fog.blendtime-rns.time;
		flFrac = Common::SplineFraction(flFrac, 1.0f/rns.fog.blend2.blend);

		if(!rns.fog.blend1.affectsky && rns.fog.blend2.affectsky)
			flFrac = 1.0-flFrac;

		if(flFrac > 1.0) 
			flFrac = 1.0;

		m_pVBO->Bind();
		if(!m_pShader->EnableShader())
		{
			Sys_ErrorPopup("Shader error: %s.\n", m_pShader->GetError());
			return false;
		}

		m_pShader->EnableAttribute(m_attribs.a_origin);
		m_pShader->DisableAttribute(m_attribs.a_texcoord);

		if(!m_pShader->SetDeterminator(m_attribs.d_mode, SHADER_COLOR))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			return false;
		}

		m_pShader->SetUniform4f(m_attribs.u_color, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2], flFrac);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		rns.view.projection.PushMatrix();
		rns.view.projection.LoadIdentity();

		rns.view.modelview.PushMatrix();
		rns.view.modelview.LoadIdentity();
		rns.view.modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

		m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

		R_ValidateShader(m_pShader);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, BUFFER_OFFSET(m_screenQuadBase));

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		rns.view.modelview.PopMatrix();
		rns.view.projection.PopMatrix();

		m_pShader->DisableAttribute(m_attribs.a_texcoord);
		m_pShader->DisableAttribute(m_attribs.a_origin);

		// Disable shader and VBO
		m_pShader->DisableShader();
		m_pVBO->UnBind();
	}

	return true;
}

//====================================
//
//====================================
void CSkyRenderer::LoadSkyTextures( const Char* pstrName, en_texture_t** pArray )
{
	// check if texture is present
	Int32 j = 0;
	for(; j < NB_SKYBOX_TEXTURES; j++)
	{
		CString filepath;
		filepath << TEXTURE_BASE_DIRECTORY_PATH << SKYBOX_TEXTURE_DIR << pstrName << SKY_TEXTURE_POSTFIXES[j] << ".dds";
		if(!FL_FileExists(filepath.c_str()))
		{
			filepath.clear();
			filepath << TEXTURE_BASE_DIRECTORY_PATH << SKYBOX_TEXTURE_DIR << pstrName << SKY_TEXTURE_POSTFIXES[j] << ".tga";
			if(!FL_FileExists(filepath.c_str()))
				break;
		}
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Re-set skyname to default if missing
	if(j != NB_SKYBOX_TEXTURES)
	{
		for(Uint32 i = 0; i < NB_SKYBOX_TEXTURES; i++)
			pArray[i] = pTextureManager->GetDummyTexture();

		Con_EPrintf("%s - Skybox texture set incomplete or missing for '%s'\n", __FUNCTION__, pstrName);
		return;
	}

	for(Uint32 i = 0; i < NB_SKYBOX_TEXTURES; i++)
	{
		CString path;
		path << SKYBOX_TEXTURE_DIR << pstrName << SKY_TEXTURE_POSTFIXES[i] << ".dds";

		en_texture_t* ptexture = pTextureManager->LoadTexture(path.c_str(), RS_GAME_LEVEL, (TX_FL_CLAMP_S|TX_FL_CLAMP_T));
		if(!ptexture)
		{
			rns.sky.drawsky = false;
			return;
		}

		pArray[i] = ptexture;
	}
}

//====================================
//
//====================================
Int32 CSkyRenderer::FindSkySet( Int32 skysetindex )
{
	Uint32 i = 0;
	for(; i < m_skySetsArray.size(); i++)
	{
		if(m_skySetsArray[i].serverindex == skysetindex)
			break;
	}

	if(i == m_skySetsArray.size())
		return NO_POSITION;
	else
		return (Int32)i;
}

//====================================
//
//====================================
void CSkyRenderer::AddSkyTextureSet( const Char* pstrSkyTextureName, Int32 skysetindex )
{
	for(Uint32 i = 0; i < m_skySetsArray.size(); i++)
	{
		if(m_skySetsArray[i].serverindex == skysetindex)
		{
			Con_Printf("%s - Sky set '%s' with server index %d already registered.\n", __FUNCTION__, pstrSkyTextureName, skysetindex);
			return;
		}
	}

	for(Uint32 i = 0; i < m_skySetsArray.size(); i++)
	{
		if(!qstrcmp(pstrSkyTextureName, m_skySetsArray[i].texturename))
		{
			Con_Printf("%s - Sky set with name '%s' already registered.\n", __FUNCTION__, pstrSkyTextureName);
			return;
		}
	}

	skytextureset_t newSet;
	newSet.texturename = pstrSkyTextureName;
	newSet.serverindex = skysetindex;

	LoadSkyTextures(pstrSkyTextureName, newSet.ptextures);

	// Add to the list of sets
	m_skySetsArray.push_back(newSet);
}

//====================================
//
//====================================
void CSkyRenderer::SetSkyTexture( Int32 skysetindex )
{
	if(skysetindex == NO_POSITION)
	{
		m_skySetUsed = NO_POSITION;
		return;
	}

	m_skySetUsed = FindSkySet(skysetindex);
	if(m_skySetUsed == NO_POSITION)
		Con_EPrintf("%s - No such set with index '%d' registered.\n", __FUNCTION__, skysetindex);
}

//====================================
//
//====================================
void CSkyRenderer::SetSkyBoxSkySet( Int32 skysetindex )
{
	if(skysetindex == NO_POSITION)
	{
		m_skyBoxSkySet = NO_POSITION;
		return;
	}

	m_skyBoxSkySet = FindSkySet(skysetindex);
	if(m_skyBoxSkySet == NO_POSITION)
		Con_EPrintf("%s - No such set with index '%d' registered.\n", __FUNCTION__, skysetindex);
}

//====================================
//
//====================================
void CSkyRenderer::SetCurrentSkySet( Int32 skysetindex )
{
	if(skysetindex == NO_POSITION)
	{
		m_currentSkySet = m_skySetUsed;
		return;
	}

	m_currentSkySet = FindSkySet(skysetindex);
	if(m_currentSkySet == NO_POSITION)
		Con_EPrintf("%s - No such set with index '%d' registered.\n", __FUNCTION__, skysetindex);
}