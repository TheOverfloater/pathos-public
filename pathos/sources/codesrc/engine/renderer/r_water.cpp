/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Notes:
// Flow mapping related code was done by valina354.

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "textures_shared.h"
#include "cl_main.h"
#include "r_main.h"
#include "texturemanager.h"
#include "cvar.h"
#include "r_water.h"
#include "r_bsp.h"
#include "r_common.h"
#include "console.h"
#include "system.h"
#include "cl_utils.h"
#include "file.h"
#include "brushmodel.h"
#include "cache_model.h"
#include "enginestate.h"
#include "enginefuncs.h"
#include "r_rttcache.h"
#include "r_fbocache.h"
#include "tga.h"
#include "r_lightstyles.h"
#include "modelcache.h"

// Default phong exponent value
const Float CWaterShader::DEFAULT_PHONG_EXPONENT = 16.0f;
// Default phong exponent value
const Float CWaterShader::DEFAULT_SPECULAR_FACTOR = 2.0;

// Water shader default normalmap texture path
const Char CWaterShader::WATER_DEFAULT_NORMALMAP_PATH[] = "general/watershader.tga";
// Script base path
const Char CWaterShader::WATER_SCRIPT_BASEPATH[] = "scripts/water/";
// Default water script name
const Char CWaterShader::DEFAULT_WATER_SCRIPT_FILENAME[] = "water_default.txt";

// Lightmap X resolution
const Uint32 CWaterShader::WATER_LIGHTMAP_DEFAULT_WIDTH = 128;
// Lightmap Y resolution
const Uint32 CWaterShader::WATER_LIGHTMAP_DEFAULT_HEIGHT = 128;

// FBO resolution for water
const Uint32 CWaterShader::WATER_FBO_SIZE = 1024;
// RTT resolution for water
const Uint32 CWaterShader::WATER_RTT_SIZE = 512;

//=============================================
//
//=============================================
void R_WaterQualityCvarCallBack( CCVar* pCVar )
{
	Int32 waterQuality = static_cast<Int32>(pCVar->GetValue());
	switch(waterQuality)
	{
	case 0:
	case 1:
	case 2:
		if(ens.gamestate != GAME_INACTIVE)
			Con_Printf("Changes for this cvar will only take effect after level reload.\n");
		break;
	default:
		Con_Printf("Invalid cvar value specified, will default to full quality.\n");
		break;
	}
}

// Class definition
CWaterShader gWaterShader;

//====================================
//
//====================================
CWaterShader::CWaterShader( void ):
	m_pCurrentWater(nullptr),
	m_pCvarWaterDebug(nullptr),
	m_pCvarWaterQuality(nullptr),
	m_pCvarWaterReflectionSetting(nullptr),
	m_numPasses(0),
	m_drawCounter(0),
	m_waterQuality(WATER_QUALITY_FULL),
	m_pDefaultNormalTexture(nullptr),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pDepthTexture(nullptr)
{
}

//====================================
//
//====================================
CWaterShader::~CWaterShader( void )
{
	ClearGame();
	ClearGL();
}

//====================================
//
//====================================
bool CWaterShader::Init( void ) 
{
	// Set up cvar
	m_pCvarWaterDebug = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_water_debug", "0", "Enable printing of water shader debug info." );
	m_pCvarWaterQuality = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SAVE, "r_water_quality", "2", "Water quality level(0 = no reflection/refraction passes, 1 = refraction only, 2 = refractions and reflections)", R_WaterQualityCvarCallBack);
	m_pCvarWaterReflectionSetting = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_SAVE, "r_water_reflection_quality", "2", "Water reflections quality(0 = world only, 1 = world and models, 2 = world, models and particles");
	return true;
}

//====================================
//
//====================================
void CWaterShader::Shutdown( void ) 
{
	ClearGame();
	ClearGL();
}

//====================================
//
//====================================
bool CWaterShader::InitGL( void ) 
{
	// Set up shader
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "water.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, origin));
		m_attribs.a_normal = m_pShader->InitAttribute("in_normal", 3, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, normal));
		m_attribs.a_tangent = m_pShader->InitAttribute("in_tangent", 3, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, tangent));
		m_attribs.a_binormal = m_pShader->InitAttribute("in_binormal", 3, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, binormal));
		m_attribs.a_texcoords = m_pShader->InitAttribute("in_texcoords", 2, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, texcoords));
		m_attribs.a_lightcoords = m_pShader->InitAttribute("in_lightcoords", 2, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, lightcoords));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_normal, "in_normal", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_tangent, "in_tangent", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_binormal, "in_binormal", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_lightcoords, "in_lightcoords", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_fog = m_pShader->GetDeterminatorIndex("fog");
		m_attribs.d_side = m_pShader->GetDeterminatorIndex("side");
		m_attribs.d_rectrefract = m_pShader->GetDeterminatorIndex("rectrefract");
		m_attribs.d_specular = m_pShader->GetDeterminatorIndex("specular");
		m_attribs.d_flowmap = m_pShader->GetDeterminatorIndex("flowmap");
		m_attribs.d_lightonly = m_pShader->GetDeterminatorIndex("lightonly");

		if(!R_CheckShaderDeterminator(m_attribs.d_fog, "fog", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_side, "side", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_rectrefract, "rectrefract", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_specular, "specular", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_flowmap, "flowmap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_lightonly, "lightonly", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_fresnel = m_pShader->InitUniform("fresnel", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_scroll = m_pShader->InitUniform("scroll", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_time = m_pShader->InitUniform("time", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_strength = m_pShader->InitUniform("strength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_texscale = m_pShader->InitUniform("texscale", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_rectscale = m_pShader->InitUniform("rectscale", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_lightstrength = m_pShader->InitUniform("lightstrength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_specularstrength = m_pShader->InitUniform("specularstrength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_wavefresnelstrength = m_pShader->InitUniform("wavefresnelstrength", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_flowspeed = m_pShader->InitUniform("flowSpeed", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_phongexponent = m_pShader->InitUniform("phongexponent", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_normalmatrix = m_pShader->InitUniform("normalmatrix", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_normalmatrix_v = m_pShader->InitUniform("normalmatrix_v", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_normalmap = m_pShader->InitUniform("normalMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_flowmap = m_pShader->InitUniform("flowMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_lightmap = m_pShader->InitUniform("lightMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_refract = m_pShader->InitUniform("refractMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_reflect = m_pShader->InitUniform("reflectMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_rectrefract = m_pShader->InitUniform("rectangleRefractMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_diffusemap = m_pShader->InitUniform("diffuseMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_lightvecsmap = m_pShader->InitUniform("lightvecsMap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_stylestrength = m_pShader->InitUniform("stylestrength", CGLSLShader::UNIFORM_FLOAT1);

		if(!R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fresnel, "fresnel", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scroll, "scroll", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_time, "time", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_strength, "strength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texscale, "texscale", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_rectscale, "rectscale", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_lightstrength, "lightstrength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_specularstrength, "specularstrength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_wavefresnelstrength, "wavefresnelstrength", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_flowspeed, "flowSpeed", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_phongexponent, "phongexponent", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmatrix, "normalmatrix", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmatrix_v, "normalmatrix_v", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmap, "normalMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_flowmap, "flowMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_lightmap, "lightMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_refract, "refractMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_reflect, "reflectMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_rectrefract, "rectangleRefractMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_diffusemap, "diffuseMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_lightvecsmap, "lightvecsMap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_stylestrength, "stylestrength", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(m_pDepthTexture)
	{
		// Create depth texture
		CreateDepthTexture();
	}

	// If we have water entities, restore them also
	if(!m_waterEntitiesArray.empty())
	{
		for(Uint32 i = 0; i < m_waterEntitiesArray.size(); i++)
		{
			if(!CreateRenderToTexture(m_waterEntitiesArray[i]))
			{
				m_waterEntitiesArray[i]->preflectfbo = nullptr;
				m_waterEntitiesArray[i]->preflect_texture = nullptr;
				m_waterEntitiesArray[i]->prefractfbo = nullptr;
				m_waterEntitiesArray[i]->prefract_texture = nullptr;
			}

			CreateLightmapTexture(m_waterEntitiesArray[i]);
		}
	}

	if(m_pVBO)
	{
		// Rebind the VBO
		m_pVBO->RebindGL();
		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//====================================
//
//====================================
void CWaterShader::ClearGL( void ) 
{
	if(!m_waterEntitiesArray.empty())
	{
		for(Uint32 i = 0; i < m_waterEntitiesArray.size(); i++)
		{
			if(m_waterEntitiesArray[i]->preflectfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_waterEntitiesArray[i]->preflectfbo->fboid);
				delete m_waterEntitiesArray[i]->preflectfbo;
				m_waterEntitiesArray[i]->preflectfbo = nullptr;
			}
		
			if(m_waterEntitiesArray[i]->prefractfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_waterEntitiesArray[i]->prefractfbo->fboid);
				delete m_waterEntitiesArray[i]->prefractfbo;
				m_waterEntitiesArray[i]->prefractfbo = nullptr;
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

	if (m_pDepthTexture)
		m_pDepthTexture = nullptr;
}

//====================================
//
//====================================
bool CWaterShader::InitGame( void ) 
{
	Int32 waterQuality = static_cast<Int32>(m_pCvarWaterQuality->GetValue());
	switch(waterQuality)
	{
	case 0:
		m_waterQuality = WATER_QUALITY_NO_REFLECT_REFRACT;
		break;
	case 1:
		m_waterQuality = WATER_QUALITY_NO_REFLECT;
		break;
	case 2:
	default:
		m_waterQuality = WATER_QUALITY_FULL;
		break;
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Load texture
	m_pDefaultNormalTexture = pTextureManager->LoadTexture(WATER_DEFAULT_NORMALMAP_PATH, RS_GAME_LEVEL);
	if(!m_pDefaultNormalTexture)
	{
		m_pDefaultNormalTexture = pTextureManager->GetDummyTexture();
		Con_EPrintf("%s - Couldn't load '%s'.\n", __FUNCTION__, WATER_DEFAULT_NORMALMAP_PATH);
	}

	// Load scripts
	LoadScripts();
	return true;
}

//====================================
//
//====================================
void CWaterShader::ClearGame( void ) 
{
	if(!m_waterEntitiesArray.empty())
	{
		for(Uint32 i = 0; i < m_waterEntitiesArray.size(); i++)
		{
			if(m_waterEntitiesArray[i]->preflectfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_waterEntitiesArray[i]->preflectfbo->fboid);
				delete m_waterEntitiesArray[i]->preflectfbo;
				m_waterEntitiesArray[i]->preflectfbo = nullptr;
			}
		
			if(m_waterEntitiesArray[i]->prefractfbo)
			{
				gGLExtF.glDeleteFramebuffers(1, &m_waterEntitiesArray[i]->prefractfbo->fboid);
				delete m_waterEntitiesArray[i]->prefractfbo;
				m_waterEntitiesArray[i]->prefractfbo = nullptr;
			}

			delete m_waterEntitiesArray[i];
		}

		m_waterEntitiesArray.clear();
	}

	if(!m_waterSettingsArray.empty())
		m_waterSettingsArray.clear();

	if(m_pShader)
	{
		m_pShader->SetVBO(nullptr);
		m_pShader->ResetShader();
	}

	if(m_pVBO)
	{
		// Make sure this is reset
		m_pShader->SetVBO(nullptr);

		delete m_pVBO;
		m_pVBO = nullptr;
	}

	m_drawCounter = 0;

	// Reset these
	m_pDepthTexture = nullptr;
}

//====================================
//
//====================================
void CWaterShader::CreateLightmapTexture( cl_water_t* pwater ) 
{
	// Get from cache, as we need a non-const model
	cache_model_t* pcachemodel = gModelCache.GetModelByIndex(pwater->pentity->pmodel->cacheindex);
 	if(!pcachemodel)
		return;

	brushmodel_t* pbrushmodel = pcachemodel->getBrushmodel();
	msurface_t *psurfaces = pbrushmodel->psurfaces + pbrushmodel->firstmodelsurface;

	// Holds texture data
	for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
	{
		color32_t* plightmapdata = new color32_t[pwater->lightmaptextureheights[i] * pwater->lightmaptexturewidths[i]];
		Uint32 lightmapdatasize = 0;

		color32_t* pdiffusemaptexture = new color32_t[pwater->lightmaptextureheights[i] * pwater->lightmaptexturewidths[i]];
		Uint32 diffuselightdatasize = 0;

		color32_t* plightvecstexture = new color32_t[pwater->lightmaptextureheights[i] * pwater->lightmaptexturewidths[i]];
		Uint32 lightvecsdatasize = 0;

		// Get overdarken treshold
		Float overdarken;
		if(i <= 0)
		{
			overdarken = g_pCvarOverdarkenTreshold->GetValue();
			if(overdarken < 0)
				overdarken = 0;
		}
		else
		{
			// No overdarkening on other styles
			overdarken = 0;
		}

		for(Uint32 j = 0; j < pbrushmodel->nummodelsurfaces; j++)
		{
			msurface_t* psurf = &psurfaces[j];
			if(!(psurf->flags & SURF_SHADERWATER))
				continue;
		
			// Skip empty styles
			if(i > BASE_LIGHTMAP_INDEX && psurf->styles[i] == NULL_LIGHTSTYLE_INDEX)
				continue;

			// Determine sizes
			Uint32 xsize = (psurf->extents[0] / psurf->lightmapdivider)+1;
			Uint32 ysize = (psurf->extents[1] / psurf->lightmapdivider)+1;
			Uint32 size = xsize*ysize;

			color24_t *psrc;
			if(pbrushmodel->plightdata_water[SURF_LIGHTMAP_DEFAULT])
				psrc = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(pbrushmodel->plightdata_water[SURF_LIGHTMAP_DEFAULT]) + psurf->lightoffset_water);
			else
				psrc = nullptr;

			R_BuildLightmap(psurf->light_s[i], psurf->light_t[i], psrc, psurf, plightmapdata, i, pwater->lightmaptexturewidths[i], overdarken, 0);
			lightmapdatasize += size * sizeof(color32_t);

			// See if we have anything to bind
			if(pbrushmodel->plightdata_water[SURF_LIGHTMAP_DIFFUSE] && pbrushmodel->plightdata_water[SURF_LIGHTMAP_AMBIENT])
			{
				// Grab diffuse lightmap data
				psrc = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(pbrushmodel->plightdata_water[SURF_LIGHTMAP_DIFFUSE]) + psurf->lightoffset_water);
				R_BuildLightmap(psurf->light_s[i], psurf->light_t[i], psrc, psurf, pdiffusemaptexture, i, pwater->lightmaptexturewidths[i], 0);
				diffuselightdatasize += size * sizeof(color32_t);

				// Grab vectors lightmap data
				psrc = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(pbrushmodel->plightdata_water[SURF_LIGHTMAP_VECTORS]) + psurf->lightoffset_water);
				R_BuildLightmap(psurf->light_s[i], psurf->light_t[i], psrc, psurf, plightvecstexture, i, pwater->lightmaptexturewidths[i], 0, true);
				lightvecsdatasize += size * sizeof(color32_t);
			}
		}

		if(i > BASE_LIGHTMAP_INDEX && lightmapdatasize <= 0)
		{
			delete[] plightmapdata;
			delete[] pdiffusemaptexture;
			delete[] plightvecstexture;
			continue;
		}

		if(g_pCvarDumpLightmaps->GetValue() >= 1)
		{
			CString basename;
			Common::Basename(ens.pworld->name.c_str(), basename);

			CString basedirpath;
			basedirpath << "dumps" << PATH_SLASH_CHAR << basename << "_water" << PATH_SLASH_CHAR;
			if(FL_CreateDirectory(basedirpath.c_str()))
			{
				CString filepath;
				filepath << basedirpath << "water_" << pwater->index << "_lightmap_default_style_" << i << ".tga";

				const byte* pwritedata = reinterpret_cast<const byte*>(plightmapdata);
				TGA_Write(pwritedata, 4, pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], filepath.c_str(), FL_GetInterface(), Con_Printf);
			}
		}

		pwater->plightmap_textures[i] = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

		glBindTexture(GL_TEXTURE_2D, pwater->plightmap_textures[i]->gl_index);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, plightmapdata);

		if(diffuselightdatasize > 0 && lightvecsdatasize > 0)
		{
			// Diffuse
			pwater->plightmap_diffuse_textures[i] = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

			glBindTexture(GL_TEXTURE_2D, pwater->plightmap_diffuse_textures[i]->gl_index);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, pdiffusemaptexture);

			// Light vectors
			pwater->plightmap_lightvecs_textures[i] = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

			glBindTexture(GL_TEXTURE_2D, pwater->plightmap_lightvecs_textures[i]->gl_index);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, plightvecstexture);

			if(g_pCvarDumpLightmaps->GetValue() >= 1)
			{
				CString basename;
				Common::Basename(ens.pworld->name.c_str(), basename);

				CString basedirpath;
				basedirpath << "dumps" << PATH_SLASH_CHAR << basename << "_water" << PATH_SLASH_CHAR;

				if(FL_CreateDirectory(basedirpath.c_str()))
				{
					// Diffuse
					CString filepath;
					filepath << basedirpath << "water_" << pwater->index << "_lightmap_diffuse_style_" << i << "tga";

					const byte* pwritedata = reinterpret_cast<const byte*>(pdiffusemaptexture);
					TGA_Write(pwritedata, 4, pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], filepath.c_str(), FL_GetInterface(), Con_Printf);

					// Light vectors
					filepath.clear();
					filepath << basedirpath << "water_" << pwater->index << "_lightmap_vectors_style_" << i << "tga";

					pwritedata = reinterpret_cast<const byte*>(plightvecstexture);
					TGA_Write(pwritedata, 4, pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], filepath.c_str(), FL_GetInterface(), Con_Printf);
				}
			}
		}

		delete[] plightmapdata;
		delete[] plightvecstexture;
		delete[] pdiffusemaptexture;
	}

	// Remove lighting data, as we don't need it now
	for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		if(!pbrushmodel->plightdata_water[i])
			break;

		delete[] pbrushmodel->plightdata_water[i];
		pbrushmodel->plightdata_water[i] = nullptr;
	}
}

//====================================
//
//====================================
void CWaterShader::CreateDepthTexture( void ) 
{
	if (m_pDepthTexture)
		return;

	m_pDepthTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);
	glBindTexture(GL_TEXTURE_2D, m_pDepthTexture->gl_index);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WATER_FBO_SIZE, WATER_FBO_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

//====================================
//
//====================================
void CWaterShader::Restore( void ) const
{
	if(m_waterQuality <= WATER_QUALITY_NO_REFLECT_REFRACT)
		return;

	if(m_waterEntitiesArray.empty())
		return;

	if(!rns.inwater)
		return;

	// End of frame, so reset
	rns.fog.settings = m_savedFog;
	rns.inwater = false;
}

//====================================
//
//====================================
void CWaterShader::ParseScript( const Char* pstrFilename, water_settings_t *psettings, const Char* pfile )
{
	CString normalmappath;
	CString flowmappath;

	const Char *pscan = pfile;
	while(pscan)
	{
		static Char szField[MAX_PARSE_LENGTH];
		static Char szValue[MAX_PARSE_LENGTH];

		pscan = Common::Parse(pscan, szField);

		// Some fields don't need values
		if(qstrcmp(szField, "refractonly")
			&& qstrcmp(szField, "cheaprefraction"))
		{
			if(!pscan)
				break;

			pscan = Common::Parse(pscan, szValue);
		}	

		// Set defaults
		psettings->wavefresnelstrength = 1.0;
		psettings->lightstrength = 0.2;
		psettings->phongexponent = DEFAULT_PHONG_EXPONENT;
		psettings->specularstrength = DEFAULT_SPECULAR_FACTOR;

		if(!qstrcmp(szField, "fresnel"))
			psettings->fresnel = atof(szValue);
		else if(!qstrcmp(szField, "colr"))
			psettings->fogparams.color[0] = atof(szValue)/255.0;
		else if(!qstrcmp(szField, "colg"))
			psettings->fogparams.color[1] = atof(szValue)/255.0;
		else if(!qstrcmp(szField, "colb"))
			psettings->fogparams.color[2] = atof(szValue)/255.0;
		else if(!qstrcmp(szField, "fogend"))
			psettings->fogparams.end = atof(szValue);
		else if(!qstrcmp(szField, "fogstart"))
			psettings->fogparams.start = atof(szValue);
		else if(!qstrcmp(szField, "causticscale"))
			psettings->causticscale = atof(szValue);
		else if(!qstrcmp(szField, "causticstrength"))
			psettings->causticstrength = atof(szValue);
		else if(!qstrcmp(szField, "causticstimescale"))
			psettings->causticstimescale = atof(szValue);
		else if(!qstrcmp(szField, "lightstrength"))
			psettings->lightstrength = atof(szValue);
		else if(!qstrcmp(szField, "specularstrength"))
			psettings->specularstrength = atof(szValue);
		else if(!qstrcmp(szField, "phongexponent"))
			psettings->phongexponent = atof(szValue);
		else if(!qstrcmp(szField, "wavefresnelstrength"))
			psettings->wavefresnelstrength = atof(szValue);
		else if(!qstrcmp(szField, "scrollu"))
			psettings->scrollu = atof(szValue);
		else if(!qstrcmp(szField, "scrollv"))
			psettings->scrollv = atof(szValue);
		else if(!qstrcmp(szField, "strength"))
			psettings->strength = atof(szValue);
		else if(!qstrcmp(szField, "timescale"))
			psettings->timescale = atof(szValue);
		else if(!qstrcmp(szField, "texturescale"))
			psettings->texscale = atof(szValue);
		else if(!qstrcmp(szField, "refractonly"))
			psettings->refractonly = true;
		else if(!qstrcmp(szField, "cheaprefraction"))
			psettings->cheaprefraction = true;
		else if (!qstrcmp(szField, "normalmap"))
			normalmappath = szValue;
		else if (!qstrcmp(szField, "flowmap"))
			flowmappath = szValue;
		else if (!qstrcmp(szField, "flowmapspeed"))
			psettings->flowmapspeed = atof(szValue);
		else
			Con_Printf("%s - Unknown field '%s' in '%s'\n", __FUNCTION__, szField, pstrFilename);
	}

	// always true
	psettings->fogparams.affectsky = true;

	if(psettings->fogparams.end < 1 && psettings->fogparams.start < 1)
		psettings->fogparams.active = false;
	else 
		psettings->fogparams.active = true;

	if(psettings->fresnel <= 0) 
		psettings->fresnel = 1;

	if(psettings->strength <= 0)
		psettings->strength = 1;

	if(psettings->timescale <= 0)
		psettings->timescale = 1;

	if(psettings->texscale <= 0)
		psettings->texscale = 1;

	if(psettings->cheaprefraction && (psettings->fogparams.end > 0 || psettings->fogparams.start > 0))
	{
		Con_Printf("Fog cannot be used with water script '%s' as it is set for cheap refraction.\n", pstrFilename);
		psettings->fogparams = fog_settings_t();
	}

	if(psettings->cheaprefraction && (psettings->causticscale > 0 || psettings->causticstrength > 0
		|| psettings->causticstimescale > 0))
	{
		Con_Printf("Water caustics cannot be used with water script '%s' as it is set for cheap refraction.\n", pstrFilename);
		psettings->causticscale = 0;
		psettings->causticstrength = 0;
		psettings->causticstimescale = 0;
	}
	else
	{
		if(psettings->causticscale <= 0)
			psettings->causticscale = 1;

		if(psettings->causticstrength <= 0)
			psettings->causticstrength = 1;

		if(psettings->causticstimescale <= 0)
			psettings->causticstimescale = 1;
	}

	if(!normalmappath.empty())
	{
		psettings->pnormalmap = CTextureManager::GetInstance()->LoadTexture(normalmappath.c_str(), RS_GAME_LEVEL);
		if(!psettings->pnormalmap)
		{
			psettings->pnormalmap = m_pDefaultNormalTexture;
			Con_EPrintf("%s - Couldn't load '%s'.\n", __FUNCTION__, normalmappath.c_str());
		}
	}

	if(!flowmappath.empty())
	{
		psettings->pflowmap = CTextureManager::GetInstance()->LoadTexture(flowmappath.c_str(), RS_GAME_LEVEL);
		if(!psettings->pflowmap)
			Con_EPrintf("%s - Couldn't load flow map texture '%s'.\n", __FUNCTION__, normalmappath.c_str());
	}
}

//====================================
//
//====================================
void CWaterShader::ReloadLightmapData( void )
{
	// If we have water entities, restore them also
	if (m_waterEntitiesArray.empty())
		return;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for (Uint32 i = 0; i < m_waterEntitiesArray.size(); i++)
	{
		cl_water_t* pwater = m_waterEntitiesArray[i];

		for(Uint32 j = 0; j < MAX_SURFACE_STYLES; j++)
		{
			if (pwater->plightmap_textures[j])
			{
				pTextureManager->DeleteAllocation(pwater->plightmap_textures[j]);
				pwater->plightmap_textures[j] = nullptr;
			}

			if (pwater->plightmap_diffuse_textures[j])
			{
				pTextureManager->DeleteAllocation(pwater->plightmap_diffuse_textures[j]);
				pwater->plightmap_diffuse_textures[j] = nullptr;
			}

			if (pwater->plightmap_lightvecs_textures[j])
			{
				pTextureManager->DeleteAllocation(pwater->plightmap_lightvecs_textures[j]);
				pwater->plightmap_lightvecs_textures[j] = nullptr;
			}
		}

		CreateLightmapTexture(pwater);
	}
}

//====================================
//
//====================================
void CWaterShader::LoadScripts( void ) 
{
	if(!m_waterSettingsArray.empty())
		m_waterSettingsArray.clear();

	CString filename;
	CString mapname;

	// Retreive level name
	const Char *levelname = Engine_GetLevelName();
	Common::Basename(levelname, mapname);

	while(1)
	{
		const Char *pFile = nullptr;
		if(rns.daystage == DAYSTAGE_NIGHTSTAGE)
		{
			filename.clear();
			filename << WATER_SCRIPT_BASEPATH << "water_" << mapname << "_" << static_cast<Int32>(m_waterSettingsArray.size()) << "_n.txt";
			pFile = reinterpret_cast<const Char *>(FL_LoadFile(filename.c_str(), nullptr));
		}
		
		if(!pFile)
		{
			filename.clear();
			filename << WATER_SCRIPT_BASEPATH << "water_" << mapname << "_" << static_cast<Int32>(m_waterSettingsArray.size()) << ".txt";
			pFile = reinterpret_cast<const Char *>(FL_LoadFile(filename.c_str(), nullptr));
		}

		if(!pFile)
			break;

		m_waterSettingsArray.resize(m_waterSettingsArray.size()+1);
		water_settings_t *pSettings = &m_waterSettingsArray[m_waterSettingsArray.size()-1];

		ParseScript(filename.c_str(), pSettings, pFile);
		FL_FreeFile(pFile);
	}

	if(m_waterSettingsArray.empty())
	{
		CString filepath;
		filepath << WATER_SCRIPT_BASEPATH << DEFAULT_WATER_SCRIPT_FILENAME;

		const Char *pFile = reinterpret_cast<const Char *>(FL_LoadFile(filepath.c_str(), nullptr));

		m_waterSettingsArray.resize(m_waterSettingsArray.size()+1);
		water_settings_t *pSettings = &m_waterSettingsArray[m_waterSettingsArray.size()-1];

		if(!pFile)
		{
			pSettings->fogparams = fog_settings_t();
			pSettings->causticscale = 1;
			pSettings->causticstrength = 1;
			pSettings->causticstimescale = 1;
			pSettings->fresnel = 1;
			pSettings->strength = 1;
			pSettings->texscale = 1;
			pSettings->timescale = 1;
			pSettings->refractonly = false;
			pSettings->cheaprefraction = false;
			pSettings->lightstrength = 0.2;
			pSettings->specularstrength = DEFAULT_SPECULAR_FACTOR;
			pSettings->phongexponent = DEFAULT_PHONG_EXPONENT;
			pSettings->wavefresnelstrength = 1.0;
			pSettings->flowmapspeed = 1.0;

			Con_Printf("Could not load default water definition file '%s'!\n", filepath.c_str());
			return;
		}

		ParseScript(filepath.c_str(), pSettings, pFile);
		FL_FreeFile(pFile);
	}
}

//====================================
//
//====================================
bool CWaterShader::ShouldReflect( Uint32 index, const water_settings_t* psettings ) const
{
	if(m_waterQuality <= WATER_QUALITY_NO_REFLECT || psettings->refractonly)
		return false;

	Vector viewOrg = GetViewOrigin();
	Vector waterOrg = GetWaterOrigin();

	if(waterOrg[2] > viewOrg[2])
		return false;

	// Check if we can draw anything
	for(Uint32 i = 0; i < index; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_WATERSHADER)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pwaterdata)
			continue;

		cl_water_t *pwater = pinfo->pwaterdata;

		if(!psettings->refractonly && pwater->framecount == m_drawCounter)
		{
			if(GetWaterOrigin(pwater).z == GetWaterOrigin().z)
				return false;
		}
	}

	return true;
}

//====================================
//
//====================================
void CWaterShader::AddEntity( cl_entity_t *pentity ) 
{
	for(Uint32 i = 0; i < m_waterEntitiesArray.size(); i++)
	{
		if(m_waterEntitiesArray[i]->pentity == pentity)
			return;// Already in cache
	}

	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, true, true);
		m_pShader->SetVBO(m_pVBO);
	}

	Uint32 vertexcount = 0;
	Uint32 indexcount = 0;

	const brushmodel_t* pbrushmodel = pentity->pmodel->getBrushmodel();
	msurface_t *psurfaces = pbrushmodel->psurfaces + pbrushmodel->firstmodelsurface;

	// First clear any flags and find the highest Z value
	Float highestzvalue = NULL_MAXS[2];
	for(Uint32 i = 0; i < pbrushmodel->nummodelsurfaces; i++)
	{
		msurface_t* psurf = &psurfaces[i];

		if(psurf->flags & SURF_SHADERWATER)
			psurf->flags &= ~SURF_SHADERWATER;

		for(Uint32 j = 0; j < psurf->numedges; j++)
		{
			Vector vertexpos;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+j];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

			if(vertexpos[2] > highestzvalue)
				highestzvalue = vertexpos[2];
		}
	}

	// Determine vertex and index count, and which surface is relevant
	for(Uint32 i = 0; i < pbrushmodel->nummodelsurfaces; i++)
	{
		msurface_t* psurf = &psurfaces[i];

		Uint32 j = 0;
		for(; j < psurf->numedges; j++)
		{
			Vector vertexpos;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+j];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertexpos);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertexpos);

			if(vertexpos[2] != highestzvalue)
				break;
		}
		
		if(j != psurf->numedges)
			continue;

		if(psurf->flags & SURF_PLANEBACK)
			continue;

		if (psurf->pplane->normal[2] != 1)
			continue;

		// Mark the surface
		psurf->flags |= SURF_SHADERWATER;

		// Add to vertex count
		vertexcount += psurfaces[i].numedges;
		indexcount += 3+(psurfaces[i].numedges-3)*3;
	}

	if(!vertexcount)
		return;

	cl_water_t *pwater = new cl_water_t;
	pwater->index = m_waterEntitiesArray.size();
	m_waterEntitiesArray.push_back(pwater);

	// Set render pass id
	pwater->renderpassidx = rns.nextfreerenderpassidx;
	rns.nextfreerenderpassidx += 2;

	// Allocate array of pointers
	water_vertex_t *pvertexes = new water_vertex_t[vertexcount];
	Uint32 vertexindex = 0;
	
	Uint32 *pindexes = new Uint32[indexcount];
	Uint32 indexoffset = 0;

	pwater->mins = NULL_MINS;
	pwater->maxs = NULL_MAXS;

	Int32 vertex_base = m_pVBO->GetVBOSize()/sizeof(water_vertex_t);
	Int32 index_base = m_pVBO->GetIBOSize()/sizeof(Uint32);

	pwater->start_index = index_base;
	pwater->num_indexes = indexcount;

	// For tracking lightmap allocations
	for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
	{
		pwater->lightmaptexturewidths[i] = WATER_LIGHTMAP_DEFAULT_WIDTH;
		pwater->lightmaptextureheights[i] = WATER_LIGHTMAP_DEFAULT_HEIGHT;

		Uint32* pallocations = new Uint32[pwater->lightmaptexturewidths[i]];
		for(Uint32 j = 0; j < pwater->lightmaptexturewidths[i]; j++)
			pallocations[j] = 0;

		for(Uint32 j = 0; j < pbrushmodel->nummodelsurfaces; j++)
		{
			msurface_t* psurf = &psurfaces[j];
			if(!(psurf->flags & SURF_SHADERWATER))
				continue;

			// Skip empty styles
			if(i > BASE_LIGHTMAP_INDEX && psurf->styles[i] == NULL_LIGHTSTYLE_INDEX)
				continue;

			Uint32 xsize = (psurf->extents[0] / psurf->lightmapdivider)+1;
			Uint32 ysize = (psurf->extents[1] / psurf->lightmapdivider)+1;

			R_AllocBlock(xsize, ysize, psurf->light_s[i], psurf->light_t[i], pwater->lightmaptexturewidths[i], pwater->lightmaptextureheights[i], pallocations);
		}

		delete[] pallocations;
	}

	for(Uint32 i = 0; i < pbrushmodel->nummodelsurfaces; i++)
	{
		msurface_t* psurf = &psurfaces[i];
		if(!(psurf->flags & SURF_SHADERWATER))
			continue;

		water_vertex_t *pcurvert = pvertexes + vertexindex;
		for(Uint32 j = 0; j < psurf->numedges; j++)
		{
			Vector vertex_origin;
			Int32 e_index = ens.pworld->psurfedges[psurf->firstedge+j];
			if(e_index > 0)
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, vertex_origin);
			else
				Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, vertex_origin);

			for(Uint32 k = 0; k < 3; k++)
				pcurvert->origin[k] = vertex_origin[k];

			pcurvert->origin[3] = 1.0;

			mtexinfo_t* ptexinfo = psurf->ptexinfo;
			Math::VectorCopy(ptexinfo->vecs[0], pcurvert->tangent);
			Math::VectorNormalize(pcurvert->tangent);
			Math::VectorCopy(ptexinfo->vecs[1], pcurvert->binormal);
			Math::VectorNormalize(pcurvert->binormal);

			// Store normal
			Math::VectorCopy(psurf->pplane->normal, pcurvert->normal);
			if(psurf->flags & SURF_PLANEBACK)
			{
				for(Uint32 l = 0; l < 3; l++)
					pcurvert->normal[l] *= -1;
			}

			pcurvert->texcoords[0] = Math::DotProduct(pcurvert->origin, ptexinfo->vecs[0]) + ptexinfo->vecs[0][3];
			pcurvert->texcoords[0] /= static_cast<Float>(ptexinfo->ptexture->width);

			pcurvert->texcoords[1] = Math::DotProduct(pcurvert->origin, ptexinfo->vecs[1]) + ptexinfo->vecs[1][3];
			pcurvert->texcoords[1] /= static_cast<Float>(ptexinfo->ptexture->height);

			for(Uint32 k = 0; k < MAX_SURFACE_STYLES; k++)
			{
				pcurvert->lightcoords[k][0] = Math::DotProduct(pcurvert->origin, ptexinfo->vecs[0]) + ptexinfo->vecs[0][3];
				pcurvert->lightcoords[k][0] -= psurf->texturemins[0];
				pcurvert->lightcoords[k][0] += psurf->light_s[k]*psurf->lightmapdivider + (psurf->lightmapdivider / 2.0f);
				pcurvert->lightcoords[k][0] /= pwater->lightmaptexturewidths[k]*psurf->lightmapdivider;

				pcurvert->lightcoords[k][1] = Math::DotProduct(pcurvert->origin, ptexinfo->vecs[1]) + ptexinfo->vecs[1][3];
				pcurvert->lightcoords[k][1] -= psurf->texturemins[1];
				pcurvert->lightcoords[k][1] += psurf->light_t[k]*psurf->lightmapdivider + (psurf->lightmapdivider / 2.0f);
				pcurvert->lightcoords[k][1] /= pwater->lightmaptextureheights[k]*psurf->lightmapdivider;
			}

			for(Uint32 k = 0; k < 3; k++)
			{
				if(pwater->mins[k] > pcurvert->origin[k])
					pwater->mins[k] = pcurvert->origin[k];

				if(pwater->maxs[k] < pcurvert->origin[k])
					pwater->maxs[k] = pcurvert->origin[k];
			}

			vertexindex++;
			pcurvert++;
		}

		Uint32 surf_start_index = index_base + indexoffset;

		Uint32 indexes[3];
		for(Uint32 j = 0; j < 3; j++)
		{
			indexes[j] = vertex_base+j;
			pindexes[indexoffset] = indexes[j];
			indexoffset++;
		}

		for(Uint32 j = 0, k = 3; j < (psurf->numedges-3); j++, k++)
		{
			indexes[1] = indexes[2];
			indexes[2] = vertex_base+k;

			pindexes[indexoffset] = indexes[0]; indexoffset++;
			pindexes[indexoffset] = indexes[1]; indexoffset++;
			pindexes[indexoffset] = indexes[2]; indexoffset++;
		}

		Uint32 surf_end_index = index_base + indexoffset;

		// Set up any styles
		for(Uint32 j = 1; j < MAX_SURFACE_STYLES; j++)
		{
			if(psurf->styles[j] == NULL_LIGHTSTYLE_INDEX)
				break;

			Int32 styleindex = psurf->styles[j];

			cl_water_style_batches_t* pstylebatch = nullptr;
			for(Uint32 k = 0; k < pwater->stylebatches.size(); k++)
			{
				if(pwater->stylebatches[k].styleindex == styleindex)
				{
					pstylebatch = &pwater->stylebatches[k];
					break;
				}
			}

			if(!pstylebatch)
			{
				Uint32 insertposition = pwater->stylebatches.size();
				pwater->stylebatches.resize(pwater->stylebatches.size()+1);
				pstylebatch = &pwater->stylebatches[insertposition];
				pstylebatch->styleindex = styleindex;
			}

			Uint32 k;
			for(k = 0; k < pstylebatch->batches[j].size(); k++)
			{
				cl_water_style_batch_t& batch = pstylebatch->batches[j][k];
				if(batch.start_index == surf_end_index)
				{
					batch.start_index = surf_start_index;
					break;
				}
				else if((batch.start_index+batch.num_indexes) == surf_start_index)
				{
					batch.num_indexes = (surf_end_index-batch.start_index);
					break;
				}
			}

			if(k == pstylebatch->batches[j].size())
			{
				Uint32 insertposition = pstylebatch->batches[j].size();
				pstylebatch->batches[j].resize(pstylebatch->batches[j].size()+1);

				cl_water_style_batch_t& batch = pstylebatch->batches[j][insertposition];
				batch.start_index = surf_start_index;
				batch.num_indexes = (surf_end_index - batch.start_index);
			}
		}

		vertex_base += psurf->numedges;
	}

	m_pVBO->Append(pvertexes, sizeof(water_vertex_t)*vertexcount, pindexes, sizeof(Uint32)*indexcount);
	delete[] pvertexes;
	delete[] pindexes;

	Math::VectorSubtract(pwater->mins, Vector(1, 1, 1), pwater->mins);
	Math::VectorAdd(pwater->maxs, Vector(1, 1, 1), pwater->maxs);

	pwater->pentity = pentity;
	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	pinfo->pwaterdata = pwater;

	pwater->origin[0] = (pwater->mins[0] + pwater->maxs[0]) * 0.5f;
	pwater->origin[1] = (pwater->mins[1] + pwater->maxs[1]) * 0.5f;
	pwater->origin[2] = (pwater->mins[2] + pwater->maxs[2]) * 0.5f;

	Int64 settingindex = pwater->pentity->curstate.body;
	if(settingindex >= m_waterSettingsArray.size())
		settingindex = 0;
	if(settingindex < 0)
		settingindex = 0;

	// Set this for later
	pwater->settingsindex = settingindex;
	// Retrieve settings tied to this water entity
	const water_settings_t* psettings = GetWaterSettings(pwater);

	// Rebuild lightmap
	CreateLightmapTexture(pwater);

	if(m_waterQuality > WATER_QUALITY_NO_REFLECT_REFRACT
		&& (!psettings->cheaprefraction
		|| !psettings->refractonly))
	{
		// Create render-to-texture objects
		if(!CreateRenderToTexture(pwater))
		{
			Con_Printf("%s - Failed to create render-to-texture for water entity.\n", __FUNCTION__);
			pwater->preflectfbo = nullptr;
			pwater->prefractfbo = nullptr;
			pwater->prefract_texture = nullptr;
			pwater->preflect_texture = nullptr;
		}
	}
}

//====================================
//
//====================================
bool CWaterShader::CreateRenderToTexture( cl_water_t* pwater ) 
{
	// Get ptr to texture manager
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	// Get water entity's settings
	const water_settings_t* psettings = GetWaterSettings(pwater);

	if(rns.fboused)
	{
		if(!m_pDepthTexture)
			CreateDepthTexture();

		if(m_waterQuality > WATER_QUALITY_NO_REFLECT_REFRACT 
			&& !psettings->cheaprefraction)
		{
			// Create refraction image
			pwater->prefractfbo = new fbobind_t;

			pwater->prefractfbo->ptexture1 = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
			glBindTexture(GL_TEXTURE_2D, pwater->prefractfbo->ptexture1->gl_index);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WATER_FBO_SIZE, WATER_FBO_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			gGLExtF.glGenFramebuffers(1, &pwater->prefractfbo->fboid);
			gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, pwater->prefractfbo->fboid);
			gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pwater->prefractfbo->ptexture1->gl_index, 0);
			gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pDepthTexture->gl_index, 0);

			GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(eStatus != GL_FRAMEBUFFER_COMPLETE)
			{
				Con_Printf("%s - Framebuffer Object creation failed.\n", __FUNCTION__);
				delete pwater->prefractfbo;
				return false;
			}
		}

		if(m_waterQuality > WATER_QUALITY_NO_REFLECT 
			&& !psettings->refractonly)
		{
			// Create reflection image
			pwater->preflectfbo = new fbobind_t;

			pwater->preflectfbo->ptexture1 = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
			glBindTexture(GL_TEXTURE_2D, pwater->preflectfbo->ptexture1->gl_index);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WATER_FBO_SIZE, WATER_FBO_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			gGLExtF.glGenFramebuffers(1, &pwater->preflectfbo->fboid);
			gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, pwater->preflectfbo->fboid);
			gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pwater->preflectfbo->ptexture1->gl_index, 0);
			gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_pDepthTexture->gl_index, 0);

			GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if(eStatus != GL_FRAMEBUFFER_COMPLETE)
			{
				Con_Printf("%s - Framebuffer Object creation failed.\n", __FUNCTION__);
				if(pwater->prefractfbo)
					delete pwater->prefractfbo;

				delete pwater->preflectfbo;
				return false;
			}
		}

		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		if(m_waterQuality > WATER_QUALITY_NO_REFLECT
			&& !psettings->refractonly)
		{
			// Create the reflection texture
			pwater->preflect_texture = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);

			glBindTexture(GL_TEXTURE_2D, pwater->preflect_texture->gl_index);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WATER_RTT_SIZE, WATER_RTT_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}

		if(m_waterQuality > WATER_QUALITY_NO_REFLECT_REFRACT 
			&& !psettings->cheaprefraction)
		{
			// Create the reflection texture
			pwater->prefract_texture = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);

			glBindTexture(GL_TEXTURE_2D, pwater->prefract_texture->gl_index);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WATER_RTT_SIZE, WATER_RTT_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return true;
}

//====================================
//
//====================================
void CWaterShader::SetupClipping( const ref_params_t *pparams, bool negative ) const
{
	Vector vForward, vRight, vUp, vDist;
	Math::AngleVectors(pparams->v_angles, &vForward, &vRight, &vUp);

	if(negative)
	{
		Math::VectorSubtract(GetWaterOrigin()-Vector(0, 0, 4), pparams->v_origin, vDist);
	}
	else
	{
		Math::VectorSubtract(GetWaterOrigin()+Vector(0, 0, 4), pparams->v_origin, vDist);
	}

	Math::VectorScale(vRight, -1, vRight);
	Math::VectorScale(vUp, -1, vUp);

	Float eq1[4];
	if(negative)
	{
		eq1[0] = Math::DotProduct(vRight, -Vector(0, 0, 1));
		eq1[1] = Math::DotProduct(vUp, -Vector(0, 0, 1));
		eq1[2] = Math::DotProduct(vForward, -Vector(0, 0, 1));
		eq1[3] = Math::DotProduct(vDist, -Vector(0, 0, 1));
	}
	else
	{
		eq1[0] = Math::DotProduct(vRight, Vector(0, 0, 1));
		eq1[1] = Math::DotProduct(vUp, Vector(0, 0, 1));
		eq1[2] = Math::DotProduct(vForward, Vector(0, 0, 1));
		eq1[3] = Math::DotProduct(vDist, Vector(0, 0, 1));
	}

	// Set projection matrix first
	R_SetProjectionMatrix(rns.view.nearclip, rns.view.fov);

	// Change current projection matrix into an oblique projection matrix
	Float projection[16];
	memcpy(projection, rns.view.projection.Transpose(), sizeof(Float)*16);

	Float eq2[4];
	eq2[0] = (sgn(eq1[0]) + projection[8]) / projection[0];
	eq2[1] = (sgn(eq1[1]) + projection[9]) / projection[5];
	eq2[2] = -1.0F;
	eq2[3] = (1.0F + projection[10]) / projection[14];

	Float dot = eq1[0]*eq2[0] + eq1[1]*eq2[1] + eq1[2]*eq2[2] + eq1[3]*eq2[3];

    projection[2] = eq1[0]*(2.0f/dot);
    projection[6] = eq1[1]*(2.0f/dot);
    projection[10] = eq1[2]*(2.0f/dot) + 1.0F;
    projection[14] = eq1[3]*(2.0f/dot);

	rns.view.projection.PushMatrix();
	rns.view.projection.SetMatrix(projection);
}

//====================================
//
//====================================
bool CWaterShader::ViewInWater( void )
{
	Vector vMins, vMaxs;
	Math::VectorAdd(m_pCurrentWater->pentity->curstate.mins, m_pCurrentWater->pentity->curstate.origin, vMins);
	Math::VectorAdd(m_pCurrentWater->pentity->curstate.maxs, m_pCurrentWater->pentity->curstate.origin, vMaxs);

	Vector viewOrg = GetViewOrigin();
	if(viewOrg[0] > vMins[0] && viewOrg[1] > vMins[1] && viewOrg[2] > vMins[2]
	&& viewOrg[0] < vMaxs[0] && viewOrg[1] < vMaxs[1] && viewOrg[2] < vMaxs[2])
		return true;

	return false;
}

//====================================
//
//====================================
bool CWaterShader::DrawWaterPasses( void ) 
{
	if(m_waterEntitiesArray.empty())
		return true;

	//Completely clear everything
	if(rns.fboused)
		glViewport(GL_ZERO, GL_ZERO, WATER_FBO_SIZE, WATER_FBO_SIZE);
	else
		glViewport(GL_ZERO, GL_ZERO, WATER_RTT_SIZE, WATER_RTT_SIZE);

	// Raise this at start
	m_drawCounter++;

	m_numPasses = 0;
	rns.mirroring = true;

	// save fog params
	m_savedFog = rns.fog.settings;

	// set size of view
	rns.view.viewsize_x = rns.screenwidth;
	rns.view.viewsize_y = rns.screenheight;

	CFrustum mainFrustum;
	R_SetFrustum(mainFrustum, rns.view.params.v_origin, rns.view.params.v_angles, rns.view.fov, rns.view.viewsize_x, rns.view.viewsize_y, true);

	// error state tracking
	bool result = true;

	// Check if we can draw anything
	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_WATERSHADER
			&& pentity->curstate.rendertype != RT_SKYWATERENT)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pwaterdata)
			continue;

		m_pCurrentWater = pinfo->pwaterdata;

		if(!m_pCurrentWater->preflectfbo && m_pCurrentWater->prefractfbo
			&& !m_pCurrentWater->preflect_texture && m_pCurrentWater->prefract_texture)
			continue;

		if(m_pCurrentWater->pentity->curstate.rendertype == RT_SKYWATERENT)
			rns.water_skydraw = true;
		else
			rns.water_skydraw = false;

		if(pentity->curstate.rendertype != RT_SKYWATERENT)
		{
			// This needs to be set every time(also, be sure to use the viewsize from view_params!)
			Vector vmins, vmaxs;
			Math::VectorAdd(m_pCurrentWater->pentity->curstate.mins, m_pCurrentWater->pentity->curstate.origin, vmins);
			Math::VectorAdd(m_pCurrentWater->pentity->curstate.maxs, m_pCurrentWater->pentity->curstate.origin, vmaxs);

			if(mainFrustum.CullBBox(vmins, vmaxs))
				continue;
		}

		// Get water entity's settings
		const water_settings_t* psettings = GetWaterSettings(m_pCurrentWater);

		if(m_waterQuality == WATER_QUALITY_NO_REFLECT_REFRACT 
			|| psettings->cheaprefraction
			&& psettings->refractonly)
		{
			m_pCurrentWater->framecount = m_drawCounter;
			continue;
		}

		if(m_waterQuality > WATER_QUALITY_NO_REFLECT_REFRACT 
			&& !psettings->cheaprefraction)
		{
			SetupRefract(psettings);

			// Draw the refraction scene
			result = DrawScene(m_waterParams, true);
			if(!result)
				break;

			// Restore after DrawScene
			m_pCurrentWater = pinfo->pwaterdata;

			FinishRefract();
		}

		if(ShouldReflect(i, psettings))
		{
			SetupReflect();

			// Draw the reflection scene
			result = DrawScene(m_waterParams, false);
			if(!result)
				break;

			// Restore after DrawScene
			m_pCurrentWater = pinfo->pwaterdata;

			FinishReflect();
		}

		// for optimization
		m_pCurrentWater->framecount = m_drawCounter;
	}

	if(result && m_waterQuality > WATER_QUALITY_NO_REFLECT_REFRACT)
	{
		for(Uint32 i = 0; i < m_waterEntitiesArray.size(); i++)
		{
			m_pCurrentWater = m_waterEntitiesArray[i];
			if(m_pCurrentWater->pentity->curstate.rendertype == RT_SKYWATERENT)
				continue;

			const water_settings_t* psettings = GetWaterSettings(m_pCurrentWater);
			if(!psettings->cheaprefraction && ViewInWater() && !rns.inwater)
			{
				rns.fog.settings = psettings->fogparams;
				rns.inwater = true;
				break;
			}
		}
	}

	if(m_pCvarWaterDebug->GetValue() >= 1)
		Con_Printf("A total of %d passes drawn for water shader.\n", m_numPasses);

	rns.mirroring = false;
	rns.water_skydraw = false;

	if(rns.fboused)
		R_BindFBO(nullptr);

	glViewport(GL_ZERO, GL_ZERO, rns.screenwidth, rns.screenheight);

	return result;
}

//====================================
//
//====================================
bool CWaterShader::DrawScene( const ref_params_t& pparams, bool isrefracting ) 
{
	// Set renderpass id
	rns.renderpassidx = isrefracting ? m_pCurrentWater->renderpassidx : (m_pCurrentWater->renderpassidx+1);

	if(!R_Draw(pparams))
		return false;

	if(m_pCvarWaterDebug->GetValue())
	{
		if(isrefracting)
		{
			Con_Printf("Water No %d Refract: %d wpolys, %d studio polys drawn\n", 
				m_pCurrentWater->index, rns.counters.brushpolies, rns.counters.modelpolies);
		}
		else
		{
			Con_Printf("Water No %d Reflect: %d wpolys, %d studio polys drawn\n", 
				m_pCurrentWater->index, rns.counters.brushpolies, rns.counters.modelpolies);
		}
	}

	m_numPasses++;
	return true;
}

//====================================
//
//====================================
Vector CWaterShader::GetWaterOrigin( cl_water_t *pwater ) const
{
	if(pwater)
		return pwater->origin+pwater->pentity->curstate.origin;
	else 
		return m_pCurrentWater->origin+m_pCurrentWater->pentity->curstate.origin;
}

//====================================
//
//====================================
Vector CWaterShader::GetViewOrigin( void ) const
{
	if(m_pCurrentWater->pentity->curstate.rendertype == RT_SKYWATERENT)
	{
		// Set up sky origin
		Vector vorigin;
		if(rns.sky.skysize)
		{
			Vector vtemp;
			Math::VectorSubtract(rns.view.params.v_origin, rns.sky.world_origin, vtemp);
			Math::VectorMA(rns.sky.local_origin, 1.0f/rns.sky.skysize, vtemp, vorigin);
		}
		else
		{
			Math::VectorCopy(rns.sky.local_origin, vorigin);
		}

		return vorigin;
	}
	else
		return rns.view.params.v_origin;
}

//====================================
//
//====================================
void CWaterShader::SetupRefract( const water_settings_t* psettings ) 
{
	// Copy values from main view params
	m_waterParams = rns.view.params;

	m_waterParams.screenwidth = rns.view.params.screenwidth;
	m_waterParams.screenheight = rns.view.params.screenheight;

	rns.view.nearclip = NEAR_CLIP_DISTANCE;
	rns.view.fov = R_GetRenderFOV(rns.view.params.viewsize);

	Vector viewOrg = GetViewOrigin();
	Math::VectorCopy(viewOrg, m_waterParams.v_origin);

	if(GetWaterOrigin().z < viewOrg[2])
	{
		SetupClipping(&m_waterParams, false);

		Vector vMins, vMaxs;
		Math::VectorAdd(m_pCurrentWater->pentity->curstate.mins, m_pCurrentWater->pentity->curstate.origin, vMins);
		Math::VectorAdd(m_pCurrentWater->pentity->curstate.maxs, m_pCurrentWater->pentity->curstate.origin, vMaxs);

		rns.view.frustum.SetExtraCullBox(vMins, vMaxs);
		rns.fog.settings = psettings->fogparams;
		rns.inwater = true;
	}
	else
	{
		Vector vMins, vMaxs;
		Math::VectorCopy(ens.pworld->maxs, vMaxs);
		Math::VectorCopy(ens.pworld->mins, vMins); 
		vMins.z = GetWaterOrigin().z;
		
		rns.view.frustum.SetExtraCullBox(vMins, vMaxs);
		SetupClipping(&m_waterParams, true);
	}

	if(rns.fboused && m_pCurrentWater->prefractfbo)
		R_BindFBO(m_pCurrentWater->prefractfbo);

	// Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//====================================
//
//====================================
void CWaterShader::FinishRefract( void ) 
{
	rns.fog.settings = m_savedFog;

	// Disable culling
	rns.view.frustum.DisableExtraCullBox();
	rns.view.projection.PopMatrix();
	rns.inwater = false;

	if(!rns.fboused || !m_pCurrentWater->prefractfbo)
	{
		R_Bind2DTexture(GL_TEXTURE0_ARB, m_pCurrentWater->prefract_texture->gl_index);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, WATER_RTT_SIZE, WATER_RTT_SIZE, 0);
	}
}

//====================================
//
//====================================
void CWaterShader::SetupReflect( void ) 
{
	Vector forward;
	Math::AngleVectors(rns.view.params.v_angles, &forward, nullptr, nullptr);

	Vector viewOrg = GetViewOrigin();
	Float flDist = abs((m_pCurrentWater->pentity->curstate.origin[2]+m_pCurrentWater->origin[2])-viewOrg[2]);
	Math::VectorMA(viewOrg, -2*flDist, Vector(0, 0, 1), m_waterParams.v_origin);

	flDist = Math::DotProduct(forward, Vector(0, 0, 1));
	Math::VectorMA(forward, -2*flDist, Vector(0, 0, 1), forward);

	m_waterParams.v_angles[0] = -asin(forward[2])/M_PI*180;
	m_waterParams.v_angles[1] = atan2(forward[1], forward[0])/M_PI*180;
	m_waterParams.v_angles[2] = -rns.view.params.v_angles[2];

	m_waterParams.screenwidth = rns.view.params.screenwidth;
	m_waterParams.screenheight = rns.view.params.screenheight;
	m_waterParams.viewsize = rns.view.params.viewsize;

	rns.view.nearclip = NEAR_CLIP_DISTANCE;
	rns.view.fov = R_GetRenderFOV(rns.view.params.viewsize);

	m_waterParams.renderflags = RENDERER_FL_NONE;
	if(m_pCvarWaterReflectionSetting->GetValue() <= 1.0)
		m_waterParams.renderflags |= RENDERER_FL_DONT_DRAW_PARTICLES;
	
	if(m_pCvarWaterReflectionSetting->GetValue() <= 0.0)
		m_waterParams.renderflags |= RENDERER_FL_DONT_DRAW_MODELS;

	// Cull everything below the water plane
	Vector vMins, vMaxs;
	Math::VectorCopy(ens.pworld->maxs, vMaxs);
	Math::VectorCopy(ens.pworld->mins, vMins); 
	vMins.z = GetWaterOrigin().z;
	
	rns.view.frustum.SetExtraCullBox(vMins, vMaxs);
	SetupClipping(&m_waterParams, true);

	if(rns.fboused && m_pCurrentWater->preflectfbo)
		R_BindFBO(m_pCurrentWater->preflectfbo);

	// Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(m_pCurrentWater->pentity->curstate.rendertype == RT_SKYWATERENT)
		rns.view.v_visorigin = GetViewOrigin();
	else
		rns.view.v_visorigin = rns.view.params.v_origin;

	rns.usevisorigin = true;
}

//====================================
//
//====================================
void CWaterShader::FinishReflect( void ) 
{
	// Turn culling off
	rns.view.frustum.DisableExtraCullBox();
	rns.view.projection.PopMatrix();

	if(!rns.fboused || !m_pCurrentWater->preflectfbo)
	{
		R_Bind2DTexture(GL_TEXTURE0_ARB, m_pCurrentWater->preflect_texture->gl_index);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, WATER_RTT_SIZE, WATER_RTT_SIZE, 0);
	}

	rns.usevisorigin = false;
}

//====================================
//
//====================================
bool CWaterShader::DrawWater( bool skybox ) 
{
	if(m_waterEntitiesArray.empty())
		return true;

	if(rns.mirroring)
		return true;

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_normal);
	m_pShader->EnableAttribute(m_attribs.a_tangent);
	m_pShader->EnableAttribute(m_attribs.a_binormal);
	m_pShader->EnableAttribute(m_attribs.a_texcoords);
	m_pShader->EnableAttribute(m_attribs.a_lightcoords);

	bool result = true;
	if(rns.fog.settings.active)
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, TRUE, false);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));
	}
	else
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, FALSE, false);
	}

	if(!result)
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix_v, rns.view.modelview.GetInverse());

	rtt_texture_t* pRTT = nullptr;
	CFBOCache::cache_fbo_t* pScreenFBO = nullptr;

	// Check if we can draw anything
	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t *pentity = rns.objects.pvisents_unsorted[i];

		if(pentity->curstate.rendertype != RT_WATERSHADER
			&& pentity->curstate.rendertype != RT_SKYWATERENT)
			continue;

		if(pentity->curstate.rendertype == RT_SKYWATERENT && !skybox
			|| pentity->curstate.rendertype != RT_SKYWATERENT && skybox)
			continue;

		entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
		if(!pinfo->pwaterdata)
			continue;

		// Get ptr from entity
		m_pCurrentWater = pinfo->pwaterdata;
		// get water settings ptr
		const water_settings_t* psettings = GetWaterSettings(m_pCurrentWater);

		if(!m_pCurrentWater->preflectfbo && m_pCurrentWater->prefractfbo
			&& !m_pCurrentWater->preflect_texture && m_pCurrentWater->prefract_texture)
			continue;

		if(m_pCurrentWater->framecount != m_drawCounter)
			continue;

		// load modelview
		if(R_IsEntityMoved(*pentity))
		{
			rns.view.modelview.PushMatrix();
			rns.view.modelview.Translate(pentity->curstate.origin[0], pentity->curstate.origin[1], pentity->curstate.origin[2]);
			m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
			rns.view.modelview.PopMatrix();
		}

		if(!psettings->refractonly && m_waterQuality > WATER_QUALITY_NO_REFLECT
			&& rns.view.v_origin[2] > GetWaterOrigin().z)
		{
			glCullFace(GL_FRONT);
			result = m_pShader->SetDeterminator(m_attribs.d_side, WATERSURF_SIDE_ABOVE, false);
			m_pShader->SetUniform1f(m_attribs.u_fresnel, psettings->fresnel);
		}
		else
		{
			if((psettings->refractonly || m_waterQuality <= WATER_QUALITY_NO_REFLECT)
				&& rns.view.v_origin[2] > GetWaterOrigin().z)
				glCullFace(GL_FRONT);
			else
				glCullFace(GL_BACK);

			result = m_pShader->SetDeterminator(m_attribs.d_side, WATERSURF_SIDE_UNDER, false);
		}

		if(!result)
			break;

		// Set the normal map
		Int32 textureUnit = 0;
		en_texture_t* pnormaltex = (psettings->pnormalmap) ? psettings->pnormalmap : m_pDefaultNormalTexture;
		R_Bind2DTexture(GL_TEXTURE0+textureUnit, pnormaltex->palloc->gl_index);
		m_pShader->SetUniform1i(m_attribs.u_normalmap, textureUnit);
		textureUnit++;

		// We want to keep the normal map used
		Int32 resetUnit = textureUnit;

		if (psettings->pflowmap) 
		{
			R_Bind2DTexture(GL_TEXTURE0 + textureUnit, psettings->pflowmap->palloc->gl_index);
			m_pShader->SetUniform1i(m_attribs.u_flowmap, textureUnit);
			textureUnit++;

			result = m_pShader->SetDeterminator(m_attribs.d_flowmap, TRUE, false);
			if (!result)
				break;
		}
		else
		{
			result = m_pShader->SetDeterminator(m_attribs.d_flowmap, FALSE, false);
			if (!result)
				break;
		}

		m_pShader->SetUniform1f(m_attribs.u_texscale, psettings->texscale);
		m_pShader->SetUniform1f(m_attribs.u_strength, psettings->strength);
		m_pShader->SetUniform1f(m_attribs.u_time, rns.time*psettings->timescale);
		m_pShader->SetUniform2f(m_attribs.u_scroll, psettings->scrollu*rns.time, psettings->scrollv*rns.time);
		m_pShader->SetUniform1f(m_attribs.u_lightstrength, psettings->lightstrength);
		m_pShader->SetUniform1f(m_attribs.u_specularstrength, psettings->specularstrength);
		m_pShader->SetUniform1f(m_attribs.u_phongexponent, psettings->phongexponent*g_pCvarPhongExponent->GetValue());
		m_pShader->SetUniform1f(m_attribs.u_wavefresnelstrength, psettings->wavefresnelstrength);
		m_pShader->SetUniform1f(m_attribs.u_flowspeed, psettings->flowmapspeed);

		Int32 rectangleUnit = NO_POSITION;
		if(psettings->cheaprefraction || m_waterQuality <= WATER_QUALITY_NO_REFLECT_REFRACT)
		{
			result = m_pShader->SetDeterminator(m_attribs.d_rectrefract, TRUE, false);
			if (!result)
				break;

			rectangleUnit = textureUnit;
			textureUnit++;

			m_pShader->SetUniform2f(m_attribs.u_rectscale, rns.screenwidth, rns.screenheight);
			m_pShader->SetUniform1i(m_attribs.u_rectrefract, rectangleUnit);

			if (!pRTT)
				pRTT = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

			R_BindRectangleTexture(GL_TEXTURE0+rectangleUnit, pRTT->index, true);
			glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
		}
		else
		{
			result = m_pShader->SetDeterminator(m_attribs.d_rectrefract, FALSE, false);
			if(!result)
				break;

			m_pShader->SetUniform1i(m_attribs.u_refract, textureUnit);

			if(rns.fboused && m_pCurrentWater->prefractfbo)
				R_Bind2DTexture(GL_TEXTURE0+textureUnit, m_pCurrentWater->prefractfbo->ptexture1->gl_index);
			else
				R_Bind2DTexture(GL_TEXTURE0+textureUnit, m_pCurrentWater->prefract_texture->gl_index);

			textureUnit++;
		}

		if(!psettings->refractonly && m_waterQuality > WATER_QUALITY_NO_REFLECT)
		{
			// Set reflect unit now
			m_pShader->SetUniform1i(m_attribs.u_reflect, textureUnit);

			// Optimization: Try and find a water entity on the same z coord
			Uint32 j = 0;
			for(; j < i; j++)
			{
				pentity = rns.objects.pvisents_unsorted[j];

				if(pentity->curstate.rendertype != RT_WATERSHADER)
					continue;

				pinfo = CL_GetEntityExtraData(pentity);
				if(!pinfo->pwaterdata)
					continue;

				cl_water_t *pwater = pinfo->pwaterdata;

				if(pwater->framecount == m_drawCounter)
				{
					if(GetWaterOrigin(pwater).z == GetWaterOrigin().z)
					{
						if(rns.fboused && pwater->preflectfbo)
							R_Bind2DTexture(GL_TEXTURE0+textureUnit, pwater->preflectfbo->ptexture1->gl_index);
						else
							R_Bind2DTexture(GL_TEXTURE0+textureUnit, pwater->preflect_texture->gl_index);

						break;
					}
				}
			}

			if(j == i)
			{
				if(rns.fboused && m_pCurrentWater->preflectfbo)
					R_Bind2DTexture(GL_TEXTURE0+textureUnit, m_pCurrentWater->preflectfbo->ptexture1->gl_index);
				else
					R_Bind2DTexture(GL_TEXTURE0+textureUnit, m_pCurrentWater->preflect_texture->gl_index);
			}

			textureUnit++;
		}

		m_pShader->SetUniform1i(m_attribs.u_lightmap, textureUnit);
		R_Bind2DTexture(GL_TEXTURE0 + textureUnit, m_pCurrentWater->plightmap_textures[BASE_LIGHTMAP_INDEX]->gl_index);

		if(g_pCvarSpecular->GetValue() >= 1 && psettings->specularstrength 
			&& m_pCurrentWater->plightmap_diffuse_textures[BASE_LIGHTMAP_INDEX] 
			&& m_pCurrentWater->plightmap_lightvecs_textures[BASE_LIGHTMAP_INDEX])
		{
			result = m_pShader->SetDeterminator(m_attribs.d_specular, TRUE);
			if(!result)
				break;

			m_pShader->SetUniform1i(m_attribs.u_diffusemap, textureUnit);
			R_Bind2DTexture(GL_TEXTURE0 + textureUnit, m_pCurrentWater->plightmap_diffuse_textures[BASE_LIGHTMAP_INDEX]->gl_index);
			textureUnit++;

			m_pShader->SetUniform1i(m_attribs.u_lightvecsmap, textureUnit);
			R_Bind2DTexture(GL_TEXTURE0 + textureUnit, m_pCurrentWater->plightmap_lightvecs_textures[BASE_LIGHTMAP_INDEX]->gl_index);
			textureUnit++;
		}
		else
		{
			result = m_pShader->SetDeterminator(m_attribs.d_specular, FALSE);
			if(!result)
				break;
		}
		
		R_ValidateShader(m_pShader);

		glDrawElements(GL_TRIANGLES, m_pCurrentWater->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pCurrentWater->start_index));

		if(rectangleUnit != NO_POSITION)
			R_BindRectangleTexture(GL_TEXTURE0+rectangleUnit, 0);

		if(!m_pCurrentWater->stylebatches.empty())
		{
			result = m_pShader->SetDeterminator(m_attribs.d_rectrefract, FALSE, false);
			if(!result)
				break;

			result = m_pShader->SetDeterminator(m_attribs.d_lightonly, TRUE);
			if(!result)
				break;

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);

			if(rns.fog.settings.active)
				m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);

			// Set ptr to lightstyles array
			CArray<Float>* pLightStyleValuesArray = gLightStyles.GetLightStyleValuesArray();
			for(Uint32 j = 0; j < m_pCurrentWater->stylebatches.size(); j++)
			{
				cl_water_style_batches_t& stylebatches = m_pCurrentWater->stylebatches[j];
				if(stylebatches.styleindex == NO_POSITION)
					continue;

				Float styleStrength = (*pLightStyleValuesArray)[stylebatches.styleindex];
				if(!styleStrength)
					continue;

				for(Uint32 k = 1; k < MAX_SURFACE_STYLES; k++)
				{
					if(stylebatches.batches[k].empty())
						continue;

					// Reset to base past the normal map unit
					R_ClearBinds(resetUnit);
					textureUnit = resetUnit;

					m_pShader->SetUniform1i(m_attribs.u_lightmap, textureUnit);
					R_Bind2DTexture(GL_TEXTURE0 + textureUnit, m_pCurrentWater->plightmap_textures[k]->gl_index);
					textureUnit++;

					m_pShader->SetUniform1f(m_attribs.u_stylestrength, styleStrength);

					if(g_pCvarSpecular->GetValue() >= 1 && psettings->specularstrength 
						&& m_pCurrentWater->plightmap_diffuse_textures[k] 
						&& m_pCurrentWater->plightmap_lightvecs_textures[k])
					{
						result = m_pShader->SetDeterminator(m_attribs.d_specular, 1);
						if(!result)
							break;

						m_pShader->SetUniform1i(m_attribs.u_diffusemap, textureUnit);
						R_Bind2DTexture(GL_TEXTURE0 + textureUnit, m_pCurrentWater->plightmap_diffuse_textures[k]->gl_index);
						textureUnit++;

						m_pShader->SetUniform1i(m_attribs.u_lightvecsmap, textureUnit);
						R_Bind2DTexture(GL_TEXTURE0 + textureUnit, m_pCurrentWater->plightmap_lightvecs_textures[k]->gl_index);
						textureUnit++;
					}
					else
					{
						result = m_pShader->SetDeterminator(m_attribs.d_specular, 0);
						if(!result)
							break;
					}

					for(Uint32 l = 0; l < stylebatches.batches[k].size(); l++)
					{
						cl_water_style_batch_t& batch = stylebatches.batches[k][l];
						glDrawElements(GL_TRIANGLES, batch.num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(batch.start_index));
					}
				}
			}

			if(rns.fog.settings.active)
				m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);

			result = m_pShader->SetDeterminator(m_attribs.d_lightonly, FALSE, false);
			if(!result)
				break;
		}
	}

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	if(pRTT)
		gRTTCache.Free(pRTT);

	if (pScreenFBO)
		gFBOCache.Free(pScreenFBO);

	if(!result)
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());

	glCullFace(GL_FRONT);

	// Clear any binds
	R_ClearBinds();

	return result;
}

//====================================
//
//====================================
const water_settings_t* CWaterShader::GetActiveSettings( void ) const
{ 
	return GetWaterSettings(m_pCurrentWater);
}

//====================================
//
//====================================
const water_settings_t* CWaterShader::GetWaterSettings( cl_water_t* pwater ) const
{
	if (pwater->settingsindex < 0 ||
		pwater->settingsindex >= m_waterSettingsArray.size())
		return &m_waterSettingsArray[0];
	else
		return &m_waterSettingsArray[pwater->settingsindex];
}