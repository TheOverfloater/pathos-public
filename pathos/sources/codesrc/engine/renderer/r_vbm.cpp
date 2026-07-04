/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_main.h"
#include "r_main.h"
#include "cl_entity.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "studio.h"
#include "vbmformat.h"
#include "cvar.h"
#include "console.h"
#include "r_vbm.h"
#include "system.h"
#include "r_common.h"
#include "texturemanager.h"
#include "vbm_shared.h"
#include "cache_model.h"
#include "modelcache.h"
#include "r_rttcache.h"
#include "common.h"
#include "enginestate.h"
#include "trace.h"
#include "trace_shared.h"
#include "cl_pmove.h"
#include "r_dlights.h"
#include "r_water.h"
#include "r_decals.h"
#include "cl_utils.h"
#include "file_interface.h"
#include "file.h"
#include "flexmanager.h"
#include "vbmtrace.h"
#include "r_lightstyles.h"
#include "mcdformat.h"

// Notes:
// Part of this implementation is based on the implementation in the Half-Life SDK
// The studiomodel format is Valve's original work, and I take no ownership of it
// No copyright infringement intended
// Baked vertex lighting related code was done by valina354, extended and modified
// by Overfloater.
// Some of the code from which this originates from is the work of BUzer, so credit
// goes to him for his work on Paranoia

// Number of random colors
static constexpr Uint32 NUM_RANDOM_COLORS = 16;

// Number of light reductions
const Uint32 CVBMRenderer::NUM_LIGHT_REDUCTIONS = 3;
// Time it takes to interpolate lighting value changes
const Float CVBMRenderer::LIGHTING_LERP_TIME = 0.35;

// Max decals on a single model entity
const Uint32 CVBMRenderer::MAX_VBM_ENTITY_DECALS = 16;

// Minimum array size for vbm model vertexes
const Uint32 CVBMRenderer::MIN_VBMDECAL_VERTEXES = 32768;

// Eyeglint texture path
const Char CVBMRenderer::EYEGLINT_TEXTURE_PATH[] = "general/eyeglint.tga";

// Default lightmap sampling offset
const Float CVBMRenderer::DEFAULT_LIGHTMAP_SAMPLE_OFFSET = 16;

// Array of random colors
const Float RANDOM_COLOR_ARRAY[NUM_RANDOM_COLORS][3] = 
{
	{ 1.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
	{ 1.0, 1.0, 0.0 },
	{ 0.0, 1.0, 1.0 },
	{ 0.5, 1.0, 0.5 },
	{ 0.0, 1.0, 0.5 },
	{ 0.5, 1.0, 0.0 },
	{ 0.1, 0.6, 0.9 },
	{ 0.5, 0.2, 0.5 },
	{ 0.3, 0.8, 0.1 },
	{ 0.5, 0.0, 0.4 },
	{ 0.8, 0.1, 0.2 },
	{ 0.8, 0.8, 0.3 },
	{ 0.9, 0.5, 0.1 },
	{ 0.2, 0.5, 0.5 }
};

// Class object definition
CVBMRenderer gVBMRenderer;

//=============================================
//
//
//=============================================
CVBMRenderer::CVBMRenderer( void ):
	m_pCvarDrawModels(nullptr),
	m_pCvarDrawModelDecals(nullptr),
	m_pCvarVertexTextures(nullptr),
	m_pCvarDecalCacheSize(nullptr),
	m_pCvarSkyLighting(nullptr),
	m_pCvarSampleOffset(nullptr),
	m_pCvarUseBumpData(nullptr),
	m_pCvarLightRatio(nullptr),
	m_pCvarDrawPlayer(nullptr),
	m_pShader(nullptr),
	m_pDecalVBO(nullptr),
	m_pTempDrawVBO(nullptr),
	m_pCurrentVBO(nullptr),
	m_drawBufferIndex(0),
	m_pFlexTexture(nullptr),
	m_pScreenTexture(nullptr),
	m_pScreenFBO(nullptr),
	m_firstTextureUnit(0),
	m_pVBMSubModel(nullptr),
	m_decalIndexCacheSize(0),
	m_decalVertexCacheSize(0),
	m_pRotationMatrix(nullptr),
	m_pBoneTransform(nullptr),
	m_pWeightBoneTransform(nullptr),
	m_pGlintTexture(nullptr),
	m_pCacheModel(nullptr),
	m_pCurrentEntity(nullptr),
	m_pStudioHeader(nullptr),
	m_pVBMHeader(nullptr),
	m_pExtraInfo(nullptr),
	m_renderAlpha(0),
	m_areUBOsSupported(false),
	m_isVertexFetchSupported(false),
	m_useFlexes(false),
	m_useBlending(false),
	m_pLightingInfo(nullptr),
	m_numModelLights(0),
	m_numDynamicLights(0),
	m_isMultiPass(false),
	m_isAuraPass(false),
	m_pSubmodelDrawList(RENDERED_SUBMODELS_ALLOC_SIZE),
	m_numDrawSubmodels(0),
	m_numVBMDecals(0),
	m_numTempIndexes(0),
	m_numTempVertexes(0),
	m_vCache_Index(0),
	m_vCache_Base(0),
	m_iCache_Index(0),
	m_iCache_Base(0),
	m_gaitEstimate(0),
	m_gaitMovement(0),
	m_pFlexManager(nullptr)
{
	memset(m_pInternalRotationMatrix, 0, sizeof(m_pInternalRotationMatrix));
	memset(m_pDynamicLights, 0, sizeof(m_pDynamicLights));
	memset(m_flexTexels, 0, sizeof(m_flexTexels));
	memset(m_uboBoneMatrixData, 0, sizeof(m_uboBoneMatrixData));

	for(Uint32 i = 0; i < MAX_TEMP_VBM_INDEXES; i++)
		m_tempIndexes[i] = 0;

	for(Uint32 i = 0; i < 3; i++)
	{
		for(Uint32 j = 0; j < 4; j++)
			m_boneMatrix[i][j] = 0;
	}

	// Resize arrays to the usual nb of bones
	m_internalBoneTransform.resize(MAXSTUDIOBONES);
	m_internalWeightBoneTransform.resize(MAXSTUDIOBONES);

	m_bonePositions1.resize(MAXSTUDIOBONES);
	m_boneQuaternions1.resize(MAXSTUDIOBONES);

	m_bonePositions2.resize(MAXSTUDIOBONES);
	m_boneQuaternions2.resize(MAXSTUDIOBONES);

	m_bonePositions3.resize(MAXSTUDIOBONES);
	m_boneQuaternions3.resize(MAXSTUDIOBONES);

	m_bonePositions4.resize(MAXSTUDIOBONES);
	m_boneQuaternions4.resize(MAXSTUDIOBONES);

	m_bonePositions5.resize(MAXSTUDIOBONES);
	m_boneQuaternions5.resize(MAXSTUDIOBONES);
}

//=============================================
//
//
//=============================================
CVBMRenderer::~CVBMRenderer( void )
{
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::Init( void )
{
	m_pCvarDrawModels = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_drawmodels", "1", "Controls the rendering of models.");
	m_pCvarDrawModelDecals = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_drawmodeldecals", "1", "Controls the rendering of model decals.");
	m_pCvarVertexTextures = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_vertex_textures", "1", "Controls the use of vertex textures for facial expressions.");
	m_pCvarSkyLighting = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_model_skylight", "1", "Controls whether models take sky lighting.");
	m_pCvarUseBumpData = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_model_bumpdata", "0", "Controls whether models should use BSP bump data for lighting.");
	m_pCvarLightRatio = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_model_light_ratio", "0.5", "Controls division ratio between ambient and direct lighting for non-bump mapped lighting fetches.");
	m_pCvarDrawPlayer = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_drawlocalplayer", "0", "Controls the rendering of the local player model. For debug purposes.");

	CString minvalue;
	minvalue << DEFAULT_LIGHTMAP_SAMPLE_OFFSET;
	m_pCvarSampleOffset = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_model_sampleoffset", minvalue.c_str(), "Controls the lightmap sampling offset.");

	minvalue.clear();
	minvalue << static_cast<Int32>(MIN_VBMDECAL_VERTEXES);
	m_pCvarDecalCacheSize = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_vbmdecal_vcache_size", minvalue.c_str(), "Size of the decal vertex cache.");

	// Create flex manager instance
	m_pFlexManager = new CFlexManager(FL_GetInterface());
	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::Shutdown( void )
{
	ClearGL();

	if(m_numVBMDecals)
	{
		for(Uint32 i = 0; i < MAX_VBM_TOTAL_DECALS; i++)
			ClearDecal(&m_vbmDecals[i]);

		m_numVBMDecals = 0;
	}

	if(m_pFlexManager)
	{
		delete m_pFlexManager;
		m_pFlexManager = nullptr;
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::InitGL( void )
{
	// Init shader here because we need to check for the compatibility and the cvar
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_CHECK_SAMPLER_OVERLAP;

#ifndef _DEBUG
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;
		else if(g_pCvarGLSLOnDemand->GetValue() > 0)
			shaderFlags |= CGLSLShader::FL_GLSL_ONDEMAND_LOAD;
#endif

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, shaderFlags, VID_ShaderCompileCallback);

		// Check if we want vertex textures
		if(m_pCvarVertexTextures->GetValue() <= 0 || !R_IsExtensionSupported("GL_ARB_texture_float"))
		{
			m_pShader->DisableDeterminatorState("flex", TRUE);
			m_isVertexFetchSupported = false;
		}
		else
			m_isVertexFetchSupported = true;

		// UBOs so far are SLOWER than using standard uniforms, so
		// do not use them until I fix them
#ifdef USE_UBOS
		GLint maxUBOBindings = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);

		if(maxUBOBindings < 3 || !R_IsExtensionSupported("GL_ARB_uniform_buffer_object"))
		{
			m_pShader->DisableDeterminatorState("use_ubo", TRUE);
			m_areUBOsSupported = false;
		}
		else
		{
			m_pShader->DisableDeterminatorState("use_ubo", FALSE);
			m_areUBOsSupported = true;
		}
#else
		m_pShader->DisableDeterminatorState("use_ubo", TRUE);
		m_areUBOsSupported = false;
#endif

		// Try and compile it
		if(!m_pShader->Compile("vbmrenderer.bss"))
		{
			// Try disabling the vertex textures and UBOs
			m_pShader->DisableDeterminatorState("flex", TRUE);
			m_pShader->DisableDeterminatorState("use_ubo", TRUE);

			m_isVertexFetchSupported = false;
			m_areUBOsSupported = false;

			if(!m_pShader->Compile("vbmrenderer.bss"))
			{
				Sys_ErrorPopup("%s - Could not compile shader: %s", __FUNCTION__, m_pShader->GetError());
				return false;
			}
		}

		// If active load is set, keep loading shaders during game runtime
		if((shaderFlags & CGLSLShader::FL_GLSL_ONDEMAND_LOAD) && g_pCvarGLSLActiveLoad->GetValue() > 0)
			R_AddShaderForLoading(m_pShader);

		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 3, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, origin));
		m_attribs.a_tangent = m_pShader->InitAttribute("in_tangent", 3, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, tangent));
		m_attribs.a_normal = m_pShader->InitAttribute("in_normal", 3, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, normal));
		m_attribs.a_texcoord1 = m_pShader->InitAttribute("in_texcoord1", 2, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, texcoord1));
		m_attribs.a_texcoord2 = m_pShader->InitAttribute("in_texcoord2", 2, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, texcoord2));
		m_attribs.a_boneindexes = m_pShader->InitAttribute("in_boneindexes", MAX_VBM_BONEWEIGHTS, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, boneindexes));
		m_attribs.a_boneweights = m_pShader->InitAttribute("in_boneweights", MAX_VBM_BONEWEIGHTS, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, boneweights));

		if(m_isVertexFetchSupported)
			m_attribs.a_flexcoord = m_pShader->InitAttribute("in_flexcoord", 2, GL_FLOAT, sizeof(vbm_glvertex_t), OFFSET(vbm_glvertex_t, flexcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_tangent, "in_tangent", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_normal, "in_normal", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord1, "in_texcoord1", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord2, "in_texcoord2", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_boneindexes, "in_boneindexes", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_boneweights, "in_boneweights", m_pShader, Sys_ErrorPopup))
			return false;

		if(m_isVertexFetchSupported && !R_CheckShaderVertexAttribute(m_attribs.a_flexcoord, "in_flexcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.a_vertexlight_vectors = m_pShader->InitAttribute("in_vlight_vectors", 3, GL_UNSIGNED_BYTE, sizeof(vbm_vlight_glvertex_t), OFFSET(vbm_vlight_glvertex_t, vertexlight0_vector));
		m_attribs.a_vertexlight_diffuse = m_pShader->InitAttribute("in_vlight_diffuse", 3, GL_UNSIGNED_BYTE, sizeof(vbm_vlight_glvertex_t), OFFSET(vbm_vlight_glvertex_t, vertexlight0_vector));
		m_attribs.a_vertexlight_ambient = m_pShader->InitAttribute("in_vlight_ambient", 3, GL_UNSIGNED_BYTE, sizeof(vbm_vlight_glvertex_t), OFFSET(vbm_vlight_glvertex_t, vertexlight0_vector));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_vertexlight_vectors, "in_vlight_vectors", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_vertexlight_diffuse, "in_vlight_diffuse", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_vertexlight_ambient, "in_vlight_ambient", m_pShader, Sys_ErrorPopup))
			return false;

		if(!m_areUBOsSupported)
		{
			for(Uint32 i = 0; i < MAX_SHADER_BONES; i++)
			{
				CString uniformname;
				uniformname << "bones[" << static_cast<Int32>(i*3) << "]";
				m_attribs.boneindexes[i] = m_pShader->InitUniform(uniformname.c_str(), CGLSLShader::UNIFORM_NOSYNC, 3);
				if(!R_CheckShaderUniform(m_attribs.boneindexes[i], uniformname.c_str(), m_pShader, Sys_ErrorPopup))
					return false;
			}

			for(Uint32 i = 0; i < MAX_ENT_MLIGHTS; i++)
			{
				CString uniformname;
				uniformname << "lights_" << static_cast<Int32>(i) << "_origin";
				m_attribs.lights[i].u_origin = m_pShader->InitUniform(uniformname.c_str(), CGLSLShader::UNIFORM_FLOAT3);
				if(!R_CheckShaderUniform(m_attribs.lights[i].u_origin, uniformname.c_str(), m_pShader, Sys_ErrorPopup))
					return false;

				uniformname.clear();
				uniformname << "lights_" << static_cast<Int32>(i) << "_color";
				m_attribs.lights[i].u_color = m_pShader->InitUniform(uniformname.c_str(), CGLSLShader::UNIFORM_FLOAT3);
				if(!R_CheckShaderUniform(m_attribs.lights[i].u_color, uniformname.c_str(), m_pShader, Sys_ErrorPopup))
					return false;

				uniformname.clear();
				uniformname << "lights_" << static_cast<Int32>(i) << "_radius";
				m_attribs.lights[i].u_radius = m_pShader->InitUniform(uniformname.c_str(), CGLSLShader::UNIFORM_FLOAT1);
				if(!R_CheckShaderUniform(m_attribs.lights[i].u_radius, uniformname.c_str(), m_pShader, Sys_ErrorPopup))
					return false;
			}
		}
		else
		{
			m_attribs.ub_bonematrices = m_pShader->InitUniformBufferObject("bonematrices", sizeof(Float)*MAX_SHADER_BONES*3*4);
			if(!R_CheckShaderUniform(m_attribs.ub_bonematrices, "bonematrices", m_pShader, Sys_ErrorPopup))
				return false;

			m_attribs.ub_modellights = m_pShader->InitUniformBufferObject("modellights", sizeof(m_uboModelLightData));
			if(!R_CheckShaderUniform(m_attribs.ub_bonematrices, "modellights", m_pShader, Sys_ErrorPopup))
				return false;
		}

		m_attribs.u_flextexture = m_pShader->InitUniform("flextexture", CGLSLShader::UNIFORM_SAMPLER2D);
		m_attribs.u_flextexturesize = m_pShader->InitUniform("flextexture_size", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_phong_exponent = m_pShader->InitUniform("phong_exponent", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_specularfactor = m_pShader->InitUniform("specfactor", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_causticsm1 = m_pShader->InitUniform("causticsm1", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_causticsm2 = m_pShader->InitUniform("causticsm2", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_scroll = m_pShader->InitUniform("scroll", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_texture0 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_SAMPLER2D);
		m_attribs.u_texture1 = m_pShader->InitUniform("texture1", CGLSLShader::UNIFORM_SAMPLER2D);
		m_attribs.u_rectangle = m_pShader->InitUniform("rectangle", CGLSLShader::UNIFORM_SAMPLERRECT);
		m_attribs.u_spectexture = m_pShader->InitUniform("spectexture", CGLSLShader::UNIFORM_SAMPLER2D);
		m_attribs.u_lumtexture = m_pShader->InitUniform("lumtexture", CGLSLShader::UNIFORM_SAMPLER2D);
		m_attribs.u_normalmap = m_pShader->InitUniform("normalmap", CGLSLShader::UNIFORM_SAMPLER2D);
		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_light_radius = m_pShader->InitUniform("light_radius", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_scope_scale = m_pShader->InitUniform("scope_scale", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_scope_scrsize = m_pShader->InitUniform("scope_scrsize", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_sky_ambient = m_pShader->InitUniform("skylight_ambient", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_sky_diffuse = m_pShader->InitUniform("skylight_diffuse", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_sky_dir = m_pShader->InitUniform("skylight_dir", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_vorigin = m_pShader->InitUniform("v_origin", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_vright = m_pShader->InitUniform("v_right", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_caustics_interp = m_pShader->InitUniform("caust_interp", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_vlight_stylestrength = m_pShader->InitUniform("vlight_stylestrength", CGLSLShader::UNIFORM_FLOAT3);

		if(!R_CheckShaderUniform(m_attribs.u_flextexture, "flextexture", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_flextexturesize, "flextexture_size", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_phong_exponent, "phong_exponent", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_specularfactor, "specfactor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_causticsm1, "causticsm1", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_causticsm2, "causticsm2", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scroll, "scroll", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_color, "color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture0, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture1, "texture1", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_rectangle, "rectangle", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_spectexture, "spectexture", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_lumtexture, "lumtexture", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmap, "normalmap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_light_radius, "light_radius", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scope_scale, "scope_scale", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scope_scrsize, "scope_scrsize", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_sky_ambient, "skylight_ambient", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_sky_diffuse, "skylight_diffuse", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_sky_dir, "skylight_dir", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vorigin, "v_origin", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vright, "v_right", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_caustics_interp, "caust_interp", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vlight_stylestrength, "vlight_stylestrength", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_normalmatrix = m_pShader->InitUniform("normalmatrix", CGLSLShader::UNIFORM_MATRIX4);

		if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmatrix, "normalmatrix", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_use_ubo = m_pShader->GetDeterminatorIndex("use_ubo");
		m_attribs.d_shadertype = m_pShader->GetDeterminatorIndex("shadertype");
		m_attribs.d_flexes = m_pShader->GetDeterminatorIndex("flex");
		m_attribs.d_alphatest = m_pShader->GetDeterminatorIndex("alphatest");
		m_attribs.d_vertexlight = m_pShader->GetDeterminatorIndex("vertexlight");

		if(!R_CheckShaderDeterminator(m_attribs.d_shadertype, "shadertype", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_use_ubo, "use_ubo", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_flexes, "flex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_alphatest, "alphatest", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_vertexlight, "vertexlight", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_d_numlights = m_pShader->InitUniform("d_num_lights", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_chrome = m_pShader->InitUniform("d_chrome", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_specular = m_pShader->InitUniform("d_specular", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_luminance = m_pShader->InitUniform("d_luminance", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_bumpmapping = m_pShader->InitUniform("d_bumpmapping", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_numdlights = m_pShader->InitUniform("d_numdlights", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_blendmultipass = m_pShader->InitUniform("d_blendmultipass", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_vlight_style1 = m_pShader->InitUniform("d_vlight_style1", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_vlight_style2 = m_pShader->InitUniform("d_vlight_style2", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_d_vlight_style3 = m_pShader->InitUniform("d_vlight_style3", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_d_numlights, "num_lights", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_chrome, "chrome", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_specular, "specular", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_luminance, "luminance", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_bumpmapping, "bumpmapping", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_blendmultipass, "blendmultipass", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_vlight_style1, "d_vlight_style1", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_vlight_style2, "d_vlight_style2", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_d_vlight_style3, "d_vlight_style3", m_pShader, Sys_ErrorPopup))
			return false;

		for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
		{
			CString lightcolor;
			lightcolor << "dlight_" << i << "_color";

			CString lightorigin;
			lightorigin << "dlight_" << i << "_origin";

			CString lightradius;
			lightradius << "dlight_" << i << "_radius";

			CString lightcubemap;
			lightcubemap << "dlight_" << i << "_cubemap";

			CString lightprojtexture;
			lightprojtexture << "dlight_" << i << "_projtexture";

			CString lightshadowmap;
			lightshadowmap << "dlight_" << i << "_shadowmap";

			CString lightmatrix;
			lightmatrix << "dlight_" << i << "_matrix";

			CString lightconesize;
			lightconesize << "dlight_" << i << "_cone_size";

			CString lightspotdirection;
			lightspotdirection << "dlight_" << i << "_spotdirection";

			CString lightdeterminatorshadowmap;
			lightdeterminatorshadowmap << "d_dlight" << i << "_shadow";

			m_attribs.dlights[i].u_light_color = m_pShader->InitUniform(lightcolor.c_str(), CGLSLShader::UNIFORM_FLOAT4);
			m_attribs.dlights[i].u_light_origin = m_pShader->InitUniform(lightorigin.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			m_attribs.dlights[i].u_light_radius = m_pShader->InitUniform(lightradius.c_str(), CGLSLShader::UNIFORM_FLOAT1);
			m_attribs.dlights[i].u_light_cubemap = m_pShader->InitUniform(lightcubemap.c_str(), CGLSLShader::UNIFORM_SAMPLERCUBE);
			m_attribs.dlights[i].u_light_projtexture = m_pShader->InitUniform(lightprojtexture.c_str(), CGLSLShader::UNIFORM_SAMPLER2D);
			m_attribs.dlights[i].u_light_shadowmap = m_pShader->InitUniform(lightshadowmap.c_str(), CGLSLShader::UNIFORM_SAMPLER2D);
			m_attribs.dlights[i].u_light_matrix = m_pShader->InitUniform(lightmatrix.c_str(), CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.dlights[i].u_light_cone_size = m_pShader->InitUniform(lightconesize.c_str(), CGLSLShader::UNIFORM_FLOAT1);
			m_attribs.dlights[i].u_light_spotdirection = m_pShader->InitUniform(lightspotdirection.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			m_attribs.dlights[i].u_d_light_shadowmap = m_pShader->InitUniform(lightdeterminatorshadowmap.c_str(), CGLSLShader::UNIFORM_INT1);

			if(!R_CheckShaderUniform(m_attribs.dlights[i].u_light_color, lightcolor.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_origin, lightorigin.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_radius, lightradius.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_cubemap, lightcubemap.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_projtexture, lightprojtexture.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_shadowmap, lightshadowmap.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_matrix, lightmatrix.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_cone_size, lightconesize.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_spotdirection, lightspotdirection.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_d_light_shadowmap, lightdeterminatorshadowmap.c_str(), m_pShader, Sys_ErrorPopup))
				return false;
		}

		m_pShader->SetDeterminator(m_attribs.d_use_ubo, m_areUBOsSupported ? TRUE : FALSE, false);
	}

	if(m_isVertexFetchSupported)
		Con_Printf("Vertex textures are enabled.\n");
	else
		Con_Printf("Vertex textures are disabled.\n");
	
	if(CL_IsGameActive())
	{
		DeleteDecals();
		BuildVBOs();
		CreateVertexTexture();

		if(!RebuildVertexLightingVBOs())
		{
			Con_EPrintf("%s - Error encountered while rebuilding VBOs for VBM renderer.", __FUNCTION__);
			return false;
		}
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::ClearGL( void )
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(!m_pVBMVBOArray.empty())
	{
		for(Uint32 i = 0; i < m_pVBMVBOArray.size(); i++)
		{
			if(m_pVBMVBOArray[i])
				delete m_pVBMVBOArray[i];
		}

		m_pVBMVBOArray.clear();
	}

	if(!m_pVertexLightingVBOArray.empty())
	{
		for(Uint32 i = 0; i < m_pVertexLightingVBOArray.size(); i++)
		{
			if(m_pVertexLightingVBOArray[i])
			{
				delete m_pVertexLightingVBOArray[i]->pvbo;
				m_pVertexLightingVBOArray[i]->pvbo = nullptr;
			}
		}

		m_pVertexLightingVBOArray.clear();
	}

	if(m_pDecalVBO)
	{
		delete m_pDecalVBO;
		m_pDecalVBO = nullptr;
	}

	if(m_pTempDrawVBO)
	{
		delete m_pTempDrawVBO;
		m_pTempDrawVBO = nullptr;
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::InitGame( void )
{
	// Create the vertex texture
	CreateVertexTexture();

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Load the glint texture
	m_pGlintTexture = pTextureManager->LoadTexture(EYEGLINT_TEXTURE_PATH, RS_GAME_LEVEL);
	if(!m_pGlintTexture)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, EYEGLINT_TEXTURE_PATH);
		m_pGlintTexture = pTextureManager->GetDummyTexture();
	}

	BuildVBOs();

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::ClearGame( void )
{
	if(m_numVBMDecals)
	{
		for(Uint32 i = 0; i < MAX_VBM_TOTAL_DECALS; i++)
			ClearDecal(&m_vbmDecals[i]);

		m_numVBMDecals = 0;
	}

	if(m_pShader)
	{
		m_pShader->SetVBO(nullptr);
		m_pShader->ResetShader();
	}

	if(!m_pVBMVBOArray.empty())
	{
		for(Uint32 i = 0; i < m_pVBMVBOArray.size(); i++)
		{
			if(m_pVBMVBOArray[i])
				delete m_pVBMVBOArray[i];
		}

		m_pVBMVBOArray.clear();
	}

	if(!m_pVertexLightingVBOArray.empty())
	{
		for(Uint32 i = 0; i < m_pVertexLightingVBOArray.size(); i++)
		{
			if(m_pVertexLightingVBOArray[i])
				delete m_pVertexLightingVBOArray[i];
		}

		m_pVertexLightingVBOArray.clear();
	}

	if(m_pDecalVBO)
	{
		delete m_pDecalVBO;
		m_pDecalVBO = nullptr;
	}

	if(m_pTempDrawVBO)
	{
		delete m_pTempDrawVBO;
		m_pTempDrawVBO = nullptr;
	}

	if(m_pFlexManager)
		m_pFlexManager->Clear();
}

//=============================================
//
//
//=============================================
void CVBMRenderer::CreateVertexTexture( void )
{
	// Create the vertex texture
	if(!m_isVertexFetchSupported)
		return;

	m_pFlexTexture = CTextureManager::GetInstance()->GenTextureIndex(RS_GAME_LEVEL);

	glBindTexture(GL_TEXTURE_2D, m_pFlexTexture->gl_index);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, VBM_FLEXTEXTURE_SIZE, VBM_FLEXTEXTURE_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);
}

//=============================================
//
//
//=============================================
void CVBMRenderer::DeleteDecals( void )
{
	if(m_numVBMDecals)
	{
		for(Uint32 i = 0; i < MAX_VBM_TOTAL_DECALS; i++)
			DeleteDecal(&m_vbmDecals[i]);

		m_numVBMDecals = 0;
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetOrientation( void )
{
	Math::VectorCopy(m_pCurrentEntity->curstate.origin, m_renderOrigin);
	Math::VectorCopy(m_pCurrentEntity->curstate.angles, m_renderAngles);

	if(m_pCurrentEntity->curstate.movetype == MOVETYPE_STEP)
	{
		const mstudioseqdesc_t* pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);

		Float interp = 0;
		if((rns.time < m_pCurrentEntity->curstate.animtime + 1.0f)
			&& (m_pCurrentEntity->curstate.animtime != m_pCurrentEntity->latched.animtime))
			interp = (rns.time-m_pCurrentEntity->curstate.animtime)/(m_pCurrentEntity->curstate.animtime-m_pCurrentEntity->latched.animtime);

		if(m_pCurrentEntity->curstate.groundent == NO_ENTITY_INDEX)
			interp = 0;

		// Do interpolation
		interp -= 1.0;

		if(pseqdesc->motiontype & STUDIO_LX || m_pCurrentEntity->curstate.effects & EF_VBM_SLERP)
		{
			for(Uint32 i = 0; i < 3; i++)
				m_renderOrigin[i] += (m_pCurrentEntity->curstate.origin[i] - m_pCurrentEntity->latched.origin[i]) * interp;
		}

		for(Uint32 i = 0; i < 3; i++)
		{
			Float angle1 = m_pCurrentEntity->curstate.angles[i];
			Float angle2 = m_pCurrentEntity->latched.angles[i];

			Float anglediff = angle1-angle2;
			if(anglediff > 180)
				anglediff -= 360;
			else if(anglediff < -180)
				anglediff += 360;

			m_renderAngles[i] += anglediff*interp;
		}
	}
	else if(m_pCurrentEntity->curstate.movetype != MOVETYPE_NONE)
	{
		// Just set the angles
		Math::VectorCopy(m_pCurrentEntity->curstate.angles, m_renderAngles);
	}

	// Reverse pitch on render angles
	m_renderAngles[PITCH] = -m_renderAngles[PITCH];
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetupTransformationMatrix( void )
{
	Math::AngleMatrix(m_renderAngles, (*m_pRotationMatrix));

	for(Uint32 i = 0; i < 3; i++)
		(*m_pRotationMatrix)[i][3] = m_renderOrigin[i];

	// Apply scale to models that require it
	if((m_pCurrentEntity->curstate.renderfx == RenderFx_ScaledModel 
		|| m_pCurrentEntity->curstate.renderfx == RenderFx_SkyEntScaled
		|| m_pCurrentEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
		&& m_pCurrentEntity->curstate.scale != 0)
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			for(Uint32 j = 0; j < 3; j++)
				(*m_pRotationMatrix)[i][j] *= m_pCurrentEntity->curstate.scale;
		}
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::ApplyRenderFX( Float (*pmatrix)[4] )
{
	// Not when paused
	if(cls.paused)
		return;

	// Do not apply diary effects in non-aura passes
	if(m_pCurrentEntity->curstate.renderfx == RenderFx_Diary && !m_isAuraPass)
		return;

	switch(m_pCurrentEntity->curstate.renderfx)
	{
	case RenderFx_Diary:
	case RenderFx_Distort:
	case RenderFx_Hologram:
		if(Common::RandomLong(0,49) == 0)
		{
			Uint32 axis = Common::RandomLong(0,1);
			if(axis==1)
				axis = 2;

			Float scale = Common::RandomFloat(1, 1.5);
			for(Uint32 i = 0; i < 3; i++)
				pmatrix[axis][i] = pmatrix[axis][i] * scale;
		}
		else if(Common::RandomLong(0, 49) == 0)
		{
			Uint32 axis = Common::RandomLong(0, 2);
			Float offset = Common::RandomFloat(-10, 10);
			pmatrix[axis][3] += offset;
		}
		break;
	case RenderFx_Explode:
		{
			Float scale = 1.0 + (rns.time - m_pCurrentEntity->curstate.animtime)*10;
			if(scale > 2)
				scale = 2;

			for(Uint32 i = 0; i < 3; i++)
				pmatrix[i][1] *= scale;
		}
		break;
	default:
		break;
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::SetupBones( Int32 flags )
{
	// Ensure bone arrays are of proper sizes
	if(m_bonePositions1.size() < m_pStudioHeader->numbones)
		m_bonePositions1.resize(m_pStudioHeader->numbones);

	if(m_boneQuaternions1.size() < m_pStudioHeader->numbones)
		m_boneQuaternions1.resize(m_pStudioHeader->numbones);

	if(m_bonePositions2.size() < m_pStudioHeader->numbones)
		m_bonePositions2.resize(m_pStudioHeader->numbones);

	if(m_boneQuaternions2.size() < m_pStudioHeader->numbones)
		m_boneQuaternions2.resize(m_pStudioHeader->numbones);

	if(m_bonePositions3.size() < m_pStudioHeader->numbones)
		m_bonePositions3.resize(m_pStudioHeader->numbones);

	if(m_boneQuaternions3.size() < m_pStudioHeader->numbones)
		m_boneQuaternions3.resize(m_pStudioHeader->numbones);
	
	if(m_bonePositions4.size() < m_pStudioHeader->numbones)
		m_bonePositions4.resize(m_pStudioHeader->numbones);

	if(m_boneQuaternions4.size() < m_pStudioHeader->numbones)
		m_boneQuaternions4.resize(m_pStudioHeader->numbones);

	if(m_bonePositions5.size() < m_pStudioHeader->numbones)
		m_bonePositions5.resize(m_pStudioHeader->numbones);

	if(m_boneQuaternions5.size() < m_pStudioHeader->numbones)
		m_boneQuaternions5.resize(m_pStudioHeader->numbones);

	// Also the bone transforms
	if(m_pBoneTransform == &m_internalBoneTransform)
	{
		if(m_pBoneTransform->size() < m_pStudioHeader->numbones)
			m_pBoneTransform->resize(m_pStudioHeader->numbones);
	}
	else if(m_pBoneTransform->size() != static_cast<Uint32>(m_pStudioHeader->numbones))
		m_pBoneTransform->resize(m_pStudioHeader->numbones);

	if(m_pWeightBoneTransform == &m_internalWeightBoneTransform)
	{
		if(m_pWeightBoneTransform->size() < m_pStudioHeader->numbones)
			m_pWeightBoneTransform->resize(m_pStudioHeader->numbones);
	}
	else if(m_pWeightBoneTransform->size() != static_cast<Uint32>(m_pStudioHeader->numbones))
		m_pWeightBoneTransform->resize(m_pStudioHeader->numbones);

	// Determine interpolation time
	Float interptime = VBM_SEQ_BLEND_TIME;
	if(m_pCurrentEntity->curstate.effects & EF_FASTINTERP)
		interptime *= (2.0f/3.0f);

	// Cap sequence
	if(m_pCurrentEntity->curstate.sequence < 0 || m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	// Check if we have a sync target
	if((m_pCurrentEntity->curstate.effects & EF_SYNCSEQUENCE) && m_pCurrentEntity->curstate.aiment != NO_ENTITY_INDEX)
	{
		cl_entity_t* psynctarget = CL_GetEntityByIndex(m_pCurrentEntity->curstate.aiment);
		if(psynctarget)
		{
			m_pCurrentEntity->curstate.animtime = psynctarget->curstate.animtime;
			m_pCurrentEntity->curstate.frame = psynctarget->curstate.frame;
		}
	}

	// Get sequence data
	const mstudioseqdesc_t* pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);
	Float frame = VBM_EstimateFrame(pseqdesc, rns.time, m_pCurrentEntity->curstate.frame, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->curstate.framerate, m_pCurrentEntity->curstate.effects);

	// Get animation and calc rotations
	const mstudioanim_t* panim = VBM_GetAnimation(m_pStudioHeader, pseqdesc);
	if(!panim)
	{
		Con_EPrintf("%s - Pathos does not support models with sequence groups. Model '%s' not rendered.\n", __FUNCTION__, m_pCacheModel->name.c_str());
		return false;
	}

	// Calculate rotations
	VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions1, m_boneQuaternions1, pseqdesc, panim, frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

	// Manage blending
	if(pseqdesc->numblends > 1)
	{
		panim += m_pStudioHeader->numbones;
		VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions2, m_boneQuaternions2, pseqdesc, panim, frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

		Float dadt = VBM_EstimateInterpolant(rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime);
		Float interp = (m_pCurrentEntity->curstate.blending[0] * dadt + m_pCurrentEntity->latched.blending[0] * (1.0 - dadt))/255.0f;

		VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions1, m_bonePositions1, m_boneQuaternions2, m_bonePositions2, interp, m_boneQuaternions1, m_bonePositions1);

		if(pseqdesc->numblends == 4)
		{
			panim += m_pStudioHeader->numbones;
			VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions3, m_boneQuaternions3, pseqdesc, panim, frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

			panim += m_pStudioHeader->numbones;
			VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions4, m_boneQuaternions4, pseqdesc, panim, frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

			interp = (m_pCurrentEntity->curstate.blending[0] * dadt + m_pCurrentEntity->latched.blending[0] * (1.0 - dadt))/255.0f;
			VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions3, m_bonePositions3, m_boneQuaternions4, m_bonePositions4, interp, m_boneQuaternions3, m_bonePositions3);

			interp = (m_pCurrentEntity->curstate.blending[1] * dadt + m_pCurrentEntity->latched.blending[1] * (1.0 - dadt))/255.0f;
			VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions1, m_bonePositions1, m_boneQuaternions3, m_bonePositions3, interp, m_boneQuaternions1, m_bonePositions1);
		}
	}

	if( m_pExtraInfo && m_pCurrentEntity->latched.sequencetime &&
		( m_pCurrentEntity->latched.sequencetime + interptime > rns.time ) &&
		( m_pCurrentEntity->latched.sequence < m_pStudioHeader->numseq ) &&
		!( m_pCurrentEntity->curstate.effects & EF_NOLERP ) &&
		m_pCurrentEntity->curstate.framerate > 0 )
	{
		// If sequence was changed, use last known frame for blending
		if( m_pExtraInfo->paniminfo->lastsequencetime != m_pCurrentEntity->latched.sequencetime
			&& m_pCurrentEntity->latched.sequence != m_pCurrentEntity->curstate.sequence )
			m_pCurrentEntity->latched.frame = m_pExtraInfo->paniminfo->lastframe;

		pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->latched.sequence);
		panim = VBM_GetAnimation(m_pStudioHeader, pseqdesc);
		if(!panim)
		{
			Con_EPrintf("%s - Pathos does not support models with sequence groups. Model '%s' not rendered.\n", __FUNCTION__, m_pCacheModel->name.c_str());
			return false;
		}

		// Calculate rotations for previous frame
		VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions5, m_boneQuaternions5, pseqdesc, panim, m_pCurrentEntity->latched.frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

		if(pseqdesc->numblends > 1)
		{
			panim += m_pStudioHeader->numbones;
			VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions3, m_boneQuaternions3, pseqdesc, panim, m_pCurrentEntity->latched.frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

			panim += m_pStudioHeader->numbones;
			VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime, m_bonePositions4, m_boneQuaternions4, pseqdesc, panim, m_pCurrentEntity->latched.frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

			Float interp = m_pCurrentEntity->latched.prevseqblending[0] / 255.0f;
			VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions3, m_bonePositions3, m_boneQuaternions4, m_bonePositions4, interp, m_boneQuaternions3, m_bonePositions3);

			interp = m_pCurrentEntity->latched.prevseqblending[1] / 255.0f;
			VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions5, m_bonePositions5, m_boneQuaternions3, m_bonePositions3, interp, m_boneQuaternions5, m_bonePositions5);
		}

		Float seqblend = (rns.time - m_pCurrentEntity->latched.sequencetime);
		seqblend = 1.0 - Common::SplineFraction( seqblend, (1.0/interptime) );

		VBM_InterpolateBones(m_pStudioHeader, m_boneQuaternions1, m_bonePositions1, m_boneQuaternions5, m_bonePositions5, seqblend, m_boneQuaternions1, m_bonePositions1);
	}
	else
	{
		// Make sure this gets set
		m_pCurrentEntity->latched.frame = frame;
	}

	// Calculate gait sequence if present
	if(m_pCurrentEntity->curstate.gaitsequence < 0 || m_pCurrentEntity->curstate.gaitsequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.gaitsequence = 0;

	if(m_pCurrentEntity->curstate.gaitsequence > 0)
	{
		pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.gaitsequence);
		panim = VBM_GetAnimation(m_pStudioHeader, pseqdesc);
		frame = VBM_EstimateFrame(pseqdesc, rns.time, m_pCurrentEntity->curstate.gaitframe, m_pCurrentEntity->curstate.gaitanimtime, m_pCurrentEntity->curstate.framerate, m_pCurrentEntity->curstate.effects);

		VBM_CalculateRotations(m_pStudioHeader, rns.time, m_pCurrentEntity->curstate.gaitanimtime, m_pCurrentEntity->latched.gaitanimtime, m_bonePositions2, m_boneQuaternions2, pseqdesc, panim, frame, m_pCurrentEntity->curstate.controllers, m_pCurrentEntity->latched.controllers, m_pCurrentEntity->mouth.mouthopen);

		// TODO: Find a solution to mark special bones
		for(Uint32 i = 0; i < m_pStudioHeader->numbones; i++)
		{
			const mstudiobone_t* pbone = m_pStudioHeader->getBone(static_cast<Uint32>(i));
			if(!qstrcmp(pbone->name, "Bip01 Spine"))
				break;

			m_bonePositions1[i] = m_bonePositions2[i];
			m_boneQuaternions1[i] = m_boneQuaternions2[i];
		}
	}

	// Calculate bone matrices
	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(static_cast<Uint32>(i));

		Math::QuaternionMatrix(m_boneQuaternions1[i], m_boneMatrix);

		for(Uint32 j = 0; j < 3; j++)
			m_boneMatrix[j][3] = m_bonePositions1[i][j];

		if(pbone->parent == -1)
		{
			Math::ConcatTransforms((*m_pRotationMatrix), m_boneMatrix, (*m_pBoneTransform)[i].matrix);
			ApplyRenderFX((*m_pBoneTransform)[i].matrix);
		}
		else
		{
			Math::ConcatTransforms((*m_pBoneTransform)[pbone->parent].matrix, m_boneMatrix, (*m_pBoneTransform)[i].matrix);
		}
	}

	// Rotate by the inverse bind pose
	if(m_pVBMHeader)
	{
		for(Int32 i = 0; i < m_pVBMHeader->numboneinfo; i++)
		{
			const vbmboneinfo_t* pvbmbone = m_pVBMHeader->getBoneInfo(i);
			Math::ConcatTransforms((*m_pBoneTransform)[i].matrix, pvbmbone->bindtransform, (*m_pWeightBoneTransform)[i].matrix);
		}
	}

	// Set the extrainfo data if available, but ONLY if flags is not VBM_SETUPBONES
	if(!(flags & VBM_SETUPBONES) && m_pExtraInfo)
	{
		// Clear syncbase
		if(m_pExtraInfo->paniminfo->lastsequence != m_pCurrentEntity->curstate.sequence)
			m_pCurrentEntity->eventframe = -0.01;

		m_pExtraInfo->paniminfo->prevframe_sequence = m_pExtraInfo->paniminfo->lastsequence;
		m_pExtraInfo->paniminfo->lastsequence = m_pCurrentEntity->curstate.sequence;
		m_pExtraInfo->paniminfo->lastsequencetime = m_pCurrentEntity->latched.sequencetime;
		m_pExtraInfo->paniminfo->prevframe_frame = m_pExtraInfo->paniminfo->lastframe;
		m_pExtraInfo->paniminfo->lastframe = frame;

		if(m_pCurrentEntity->curstate.renderfx == RenderFx_ScaledModel 
			|| m_pCurrentEntity->curstate.renderfx == RenderFx_SkyEntScaled
			|| m_pCurrentEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
			m_pExtraInfo->paniminfo->scale = m_pCurrentEntity->curstate.scale;

		Math::VectorCopy(m_renderAngles, m_pExtraInfo->paniminfo->lastangles);
		Math::VectorCopy(m_renderOrigin, m_pExtraInfo->paniminfo->lastorigin);

		m_pExtraInfo->paniminfo->lastmouth = m_pCurrentEntity->mouth.mouthopen;
		memcpy(m_pExtraInfo->paniminfo->lastcontroller, m_pCurrentEntity->curstate.controllers, sizeof(byte)*MAX_CONTROLLERS);
		memcpy(m_pExtraInfo->paniminfo->lastblending, m_pCurrentEntity->curstate.blending, sizeof(byte)*MAX_BLENDING);
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetExtraInfo( void )
{
	if(m_pCurrentEntity->entindex < 1)
	{
		m_pRotationMatrix = (Float (*)[3][4])m_pInternalRotationMatrix;
		m_pBoneTransform = &m_internalBoneTransform;
		m_pWeightBoneTransform = &m_internalWeightBoneTransform;
		m_pExtraInfo = nullptr;
	}
	else
	{
		m_pExtraInfo = CL_GetEntityExtraData(m_pCurrentEntity);
		m_pRotationMatrix = (Float (*)[3][4])m_pExtraInfo->paniminfo->rotation;
		m_pBoneTransform = &m_pExtraInfo->paniminfo->bones;
		m_pWeightBoneTransform = &m_pExtraInfo->paniminfo->weightbones;
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::ShouldAnimate( void )
{
	if(!m_pExtraInfo)
		return true;

	if(m_pCurrentEntity->curstate.sequence < 0 || m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	if(m_pCurrentEntity->curstate.renderfx == RenderFx_Distort || m_pCurrentEntity->curstate.renderfx == RenderFx_Diary)
		return true;

	if(m_pExtraInfo->paniminfo->lastsequence != m_pCurrentEntity->curstate.sequence)
		return true;

	if(m_pExtraInfo->paniminfo->lastmouth != m_pCurrentEntity->mouth.mouthopen)
		return true;

	if((m_pCurrentEntity->curstate.renderfx == RenderFx_ScaledModel 
		|| m_pCurrentEntity->curstate.renderfx == RenderFx_SkyEntScaled
		|| m_pCurrentEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
		&& (m_pExtraInfo->paniminfo->scale != m_pCurrentEntity->curstate.scale))
		return true;

	Uint32 i = 0;
	for(; i < MAX_CONTROLLERS; i++)
	{
		if(m_pExtraInfo->paniminfo->lastcontroller[i] != m_pCurrentEntity->curstate.controllers[i])
			break;
	}

	if(i != MAX_CONTROLLERS)
		return true;

	i = 0;
	for(; i < MAX_BLENDING; i++)
	{
		if(m_pExtraInfo->paniminfo->lastblending[i] != m_pCurrentEntity->curstate.blending[i])
			break;
	}

	if(!Math::VectorCompare(m_pExtraInfo->paniminfo->lastorigin, m_renderOrigin))
		return true;

	if(!Math::VectorCompare(m_pExtraInfo->paniminfo->lastangles, m_renderAngles))
		return true;

	// Leave this for last, as it's the most expensive
	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);
	if(m_pExtraInfo->paniminfo->lastframe != VBM_EstimateFrame( pseqdesc, rns.time, m_pCurrentEntity->curstate.frame, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->curstate.framerate, m_pCurrentEntity->curstate.effects ))
		return true;

	Float interptime = VBM_SEQ_BLEND_TIME;
	if(m_pCurrentEntity->curstate.effects & EF_FASTINTERP)
		interptime *= (2.0f/3.0f);

	// Animate if we're switching animations
	if(m_pCurrentEntity->latched.sequence != m_pCurrentEntity->curstate.sequence
		&& (m_pCurrentEntity->latched.sequencetime+interptime) > rns.time)
		return true;

	return false;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::CalcPlayerBlend( const mstudioseqdesc_t* pseqdesc, Float& blend, Float& pitch )
{
	blend = pitch * 3;
	if(blend < pseqdesc->blendstart[0])
	{
		pitch -= (pseqdesc->blendstart[0] / 3.0f);
		blend = 0;
	}
	else if(blend > pseqdesc->blendend[0])
	{
		pitch -= (pseqdesc->blendend[0] / 3.0f);
		blend = 255;
	}
	else
	{
		if((pseqdesc->blendend[0] - pseqdesc->blendstart[0]) < 0.1f)
			blend = 127;
		else
			blend = 255 * (blend - pseqdesc->blendstart[0]) / (pseqdesc->blendend[0] - pseqdesc->blendstart[0]);

		pitch = 0;
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::ProcessGait( void )
{
	if(m_pCurrentEntity->curstate.sequence < 0 || m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	// Calculate blending
	Float playerBlend = 0;
	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);
	CalcPlayerBlend(pseqdesc, playerBlend, m_pCurrentEntity->curstate.angles[PITCH]);

	m_pCurrentEntity->latched.angles[PITCH] = m_pCurrentEntity->curstate.angles[PITCH];
	m_pCurrentEntity->curstate.blending[0] = playerBlend;

	m_pCurrentEntity->latched.blending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	// Estimate gait
	Double dt = cls.cl_time - cls.cl_oldtime;
	dt = clamp(dt, 0, 1);
	EstimateGait(dt);

	// Calculate the yaw value
	Float yaw = m_pCurrentEntity->curstate.angles[YAW] - m_pCurrentEntity->curstate.gaityaw;
	yaw = yaw - SDL_floor(yaw / 360.0f) * 360.0f;

	if(yaw < -180.0f)
		yaw += 360.0f;
	else if(yaw > 180.0f)
		yaw -= 360.0f;

	if(yaw > 120.0f)
	{
		m_pCurrentEntity->curstate.gaityaw = m_pCurrentEntity->curstate.gaityaw - 180.0f;
		m_gaitMovement = -m_gaitMovement;
		yaw -= 180.0f;
	}
	else if(yaw < -120.0f)
	{
		m_pCurrentEntity->curstate.gaityaw = m_pCurrentEntity->curstate.gaityaw + 180.0f;
		m_gaitMovement = -m_gaitMovement;
		yaw += 180.0f;
	}

	// Adjust the torso controllers
	Float controllerValue = ((yaw / static_cast<Float>(MAX_CONTROLLERS)) + 30) / (60.0f / 255.0f);

	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
	{
		m_pCurrentEntity->curstate.controllers[i] = controllerValue;
		m_pCurrentEntity->latched.controllers[i] = m_pCurrentEntity->curstate.controllers[i];
	}

	m_pCurrentEntity->curstate.angles[YAW] = m_pCurrentEntity->curstate.gaityaw;
	if(m_pCurrentEntity->curstate.angles[YAW] < -0)
		m_pCurrentEntity->curstate.angles[YAW] += 360.0f;

	m_pCurrentEntity->latched.angles[YAW] = m_pCurrentEntity->curstate.angles[YAW];

	if(m_pCurrentEntity->curstate.gaitsequence < 0 || m_pCurrentEntity->curstate.gaitsequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.gaitsequence = 0;

	pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.gaitsequence);

	if(pseqdesc->linearmovement[0] > 0)
		m_pCurrentEntity->curstate.gaitframe += (m_gaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	else
		m_pCurrentEntity->curstate.gaitframe += pseqdesc->fps * dt;

	m_pCurrentEntity->curstate.gaitframe = m_pCurrentEntity->curstate.gaitframe - SDL_floor(m_pCurrentEntity->curstate.gaitframe / pseqdesc->numframes) * pseqdesc->numframes;
	if(m_pCurrentEntity->curstate.gaitframe < 0)
		m_pCurrentEntity->curstate.gaitframe += pseqdesc->numframes; 
}

//=============================================
//
//
//=============================================
void CVBMRenderer::EstimateGait( Double dt )
{
	if(!dt || m_pCurrentEntity->curstate.renderframe == rns.framecount_main)
	{
		// No gait movement
		m_gaitMovement = 0;
	}
	else
	{
		Vector estimatedVelocity;
		if(m_gaitEstimate)
		{
			estimatedVelocity = m_pCurrentEntity->curstate.origin - m_pCurrentEntity->latched.prevgaitorigin;
			m_pCurrentEntity->latched.prevgaitorigin = m_pCurrentEntity->curstate.origin;
			m_gaitMovement = estimatedVelocity.Length();
			if(dt <= 0 || (m_gaitMovement / dt) < 5.0f)
			{
				m_gaitMovement = 0;

				for(Uint32 i = 0; i < 1; i++)
					estimatedVelocity[i] = 0;
			}
		}
		else
		{
			estimatedVelocity = m_pCurrentEntity->curstate.velocity;
			m_gaitMovement = estimatedVelocity.Length() * dt;
		}

		if(!estimatedVelocity[1] && !estimatedVelocity[0])
		{
			Float yawDiff = m_pCurrentEntity->curstate.angles[YAW] - m_pCurrentEntity->curstate.gaityaw;
			yawDiff = yawDiff - SDL_floor(yawDiff / 360.0f) * 360.0f;
			if(yawDiff > 180.0f)
				yawDiff -= 360.0f;
			else if(yawDiff < -180.0f)
				yawDiff += 360.0f;

			if(dt < 0.25)
				yawDiff *= dt * 4;
			else
				yawDiff *= dt;

			m_pCurrentEntity->curstate.gaityaw += yawDiff;
			m_pCurrentEntity->curstate.gaityaw -= SDL_floor(m_pCurrentEntity->curstate.gaityaw / 360.0f) * 360.0f;
			m_gaitMovement = 0;
		}
		else
		{
			m_pCurrentEntity->curstate.gaityaw = (SDL_atan2(estimatedVelocity[1], estimatedVelocity[0]) * (180.0f / M_PI));
			m_pCurrentEntity->curstate.gaityaw = clamp(m_pCurrentEntity->curstate.gaityaw, -180, 180);
		}

		m_pCurrentEntity->curstate.renderframe = rns.framecount_main;
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::SetModel( void )
{
	// Ensure this model actually exists
	cache_model_t* pmodel = gModelCache.GetModelByIndex(m_pCurrentEntity->curstate.modelindex);
	if(!pmodel)
	{
		m_pCacheModel = nullptr;
		return false;
	}

	// Get cache ptr and check if it has GL data loaded
	vbmcache_t* pcache = pmodel->getVBMCache();
	if(!pmodel->isloaded)
	{
		// Build cache entry for this VBM
		BuildVBMVBO(pcache);
		// Mark as having GL data
		pmodel->isloaded = true;
	}

	CVBO* pVBO = m_pVBMVBOArray[pcache->vboindex];
	if(m_pCurrentVBO != pVBO)
	{
		// Set the current VBO and enable basic attribs we'll need
		m_pShader->SetVBO(pVBO);
		m_pCurrentVBO = pVBO;

		m_pShader->EnableAttribute(m_attribs.a_origin);
		m_pShader->EnableAttribute(m_attribs.a_boneindexes);
		m_pShader->EnableAttribute(m_attribs.a_boneweights);
	}

	m_pCacheModel = pmodel;

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawEntityDecals( cl_entity_t* pentity )
{
	// Make sure we can draw this entity right now
	if(R_IsEntityTransparent(*pentity, true) && pentity->curstate.renderamt <= 0 ||
		pentity->curstate.renderfx == RenderFx_MirrorOnly && !rns.mirroring ||
		pentity->curstate.renderfx == RenderFx_MonitorOnly && !rns.monitorpass)
		return true;

	if(!pentity->pmodel)
		return true;

	// Set data pointers
	m_pCurrentEntity = pentity;

	// Ensure this model actually exists
	cache_model_t* pmodel = gModelCache.GetModelByIndex(m_pCurrentEntity->curstate.modelindex);
	if(!pmodel)
	{
		m_pCacheModel = nullptr;
		return true;
	}

	m_pCacheModel = pmodel;

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return true;

	SetExtraInfo();

	// Don't bother if there's no extra info(temptentity)
	if(!m_pExtraInfo)
		return true;

	// Don't bother if there's no decals to draw
	if(!m_pExtraInfo->pvbmdecalheader)
		return true;

	// Also cull skybox entities now with frustum culling - the exception for 
	// sky ents was an ancient remnant from the Paranoia-type skybox rendering, 
	// and was never removed after that got replaced
	if (CheckBBox())
		return true;

	// Set up player-relevant stuff
	Vector savedAngles;
	if(m_pCurrentEntity->player)
	{
		if(m_pCurrentEntity->curstate.gaitsequence)
		{
			// Save angles and process gait animation
			savedAngles = m_pCurrentEntity->curstate.angles;
			ProcessGait();
		}
		else
		{
			for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
			{
				m_pCurrentEntity->curstate.controllers[i] = 127.0f;
				m_pCurrentEntity->latched.controllers[i] = m_pCurrentEntity->curstate.controllers[i];
			}
		}
	}

	// Set basic infos
	SetOrientation();

	// Only animate if needed
	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones((VBM_SETUPBONES|VBM_RENDER));

		if(m_pCurrentEntity->player && m_pCurrentEntity->curstate.gaitsequence)
			m_pCurrentEntity->curstate.angles = savedAngles;
	}

	// Apply any flexes
	if(m_pVBMHeader->flags & VBM_HAS_FLEXES)
		m_pFlexManager->UpdateValues( rns.time, m_pCurrentEntity->curstate.health, m_pCurrentEntity->mouth.mouthopen, m_pExtraInfo->pflexstate, false );

	// Draw the model decals
	if(!DrawModelDecals())
		return false;

	// Make sure these states are reset
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawModel( Int32 flags, cl_entity_t* pentity )
{
	// Make sure we can draw this entity right now
	if(R_IsEntityTransparent(*pentity, true) && pentity->curstate.renderamt <= 0 ||
		pentity->curstate.renderfx == RenderFx_MirrorOnly && !rns.mirroring ||
		pentity->curstate.renderfx == RenderFx_MonitorOnly && !rns.monitorpass)
		return true;

	if(!pentity->pmodel)
		return true;

	// Set data pointers
	m_pCurrentEntity = pentity;

	// Make sure model is handled
	if(!SetModel())
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
		return true;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return true;

	SetExtraInfo();

	if(flags & VBM_RENDER)
	{
		// Also cull skybox entities now with frustum culling - the exception for 
		// sky ents was an ancient remnant from the Paranoia-type skybox rendering, 
		// and was never removed after that got replaced
		if (CheckBBox())
			return true;

		// See if we're using any scope textures, and if so, grab the screen texture
		if(!(flags & VBM_SETUPBONES) && m_pCvarDrawModels->GetValue() >= 1 && (m_pVBMHeader->flags & VBM_HAS_SCOPE_TEXTURE))
		{
			m_pScreenTexture = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);
			R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenTexture->palloc->gl_index, true);
			glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
			R_BindRectangleTexture(GL_TEXTURE0_ARB, 0);
		}
	}

	// Set up player-relevant stuff
	Vector savedAngles;
	if(m_pCurrentEntity->player)
	{
		if(m_pCurrentEntity->curstate.gaitsequence)
		{
			// Save angles and process gait animation
			savedAngles = m_pCurrentEntity->curstate.angles;
			ProcessGait();
		}
		else
		{
			for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
			{
				m_pCurrentEntity->curstate.controllers[i] = 127.0f;
				m_pCurrentEntity->latched.controllers[i] = m_pCurrentEntity->curstate.controllers[i];
			}
		}
	}

	// Set basic infos
	SetOrientation();

	// Only animate if needed
	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(flags);

		if(m_pCurrentEntity->player && m_pCurrentEntity->curstate.gaitsequence)
			m_pCurrentEntity->curstate.angles = savedAngles;
	}

	// Leave after calculating bones
	if (flags & VBM_SETUPBONES)
	{
		// Free the RTT texture if it was used
		if (m_pScreenTexture)
		{
			gRTTCache.Free(m_pScreenTexture);
			m_pScreenTexture = nullptr;
		}

		if (m_pScreenFBO)
		{
			gFBOCache.Free(m_pScreenFBO);
			m_pScreenFBO = nullptr;
		}

		return true;
	}

	// Handle any events
	if(flags & VBM_ANIMEVENTS && m_pCurrentEntity->entindex > 0)
	{
		CalculateAttachments();	
		DispatchClientEvents();
	}

	// for error tracking
	bool result = true;

	// Render any meshes
	if(flags & VBM_RENDER && m_pCvarDrawModels->GetValue() >= 1)
	{
		GetModelLights();
		GetDynamicLights();
		SetupLighting(flags);

		if(m_pVBMHeader->flags & VBM_HAS_FLEXES)
			m_pFlexManager->UpdateValues( rns.time, m_pCurrentEntity->curstate.health, m_pCurrentEntity->mouth.mouthopen, m_pExtraInfo->pflexstate, false );

		// Draw the model
		result = Render(flags);
	}

	// For sound engine checks
	m_pCurrentEntity->visframe = cls.framecount;

	// Make sure these states are reset
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);

	// Free the RTT texture if it was used
	if (m_pScreenTexture)
	{
		gRTTCache.Free(m_pScreenTexture);
		m_pScreenTexture = nullptr;
	}

	if (m_pScreenFBO)
	{
		gFBOCache.Free(m_pScreenFBO);
		m_pScreenFBO = nullptr;
	}

	return result;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::CalculateAttachments( void )
{
	for(Int32 i = 0; i < m_pStudioHeader->numattachments; i++)
	{
		const mstudioattachment_t* pattachment = m_pStudioHeader->getAttachment(static_cast<Uint32>(i));
		Math::VectorTransform(pattachment->org, (*m_pBoneTransform)[pattachment->bone].matrix, m_pCurrentEntity->attachments[i]);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::BuildVBMVBO( vbmcache_t* pvbmcache )
{
	vbmheader_t* pvbm = pvbmcache->pvbmhdr;

	//
	// Compile in vertexes
	//
	vbm_glvertex_t *pvboverts = new vbm_glvertex_t[pvbm->numverts];
	const vbmvertex_t* pvbmverts = pvbm->getVertexes();
	for(Int32 i = 0; i < pvbm->numverts; i++)
	{
		Math::VectorCopy(pvbmverts[i].origin, pvboverts[i].origin);
		Math::VectorCopy(pvbmverts[i].normal, pvboverts[i].normal);
		Math::VectorCopy(pvbmverts[i].tangent, pvboverts[i].tangent);

		pvboverts[i].texcoord1[0] = pvbmverts[i].texcoord[0];
		pvboverts[i].texcoord1[1] = pvbmverts[i].texcoord[1];

		if( pvbmverts[i].flexvertindex != -1 )
		{
			Uint32 row_verts = VBM_FLEXTEXTURE_SIZE/3;
			Uint32 tcy = (pvbmverts[i].flexvertindex + 1) / row_verts;
			Uint32 tcx = (pvbmverts[i].flexvertindex + 1) % row_verts;

			pvboverts[i].flexcoord[0] = static_cast<Float>(tcx*3) / static_cast<Float>(VBM_FLEXTEXTURE_SIZE);
			pvboverts[i].flexcoord[1] = static_cast<Float>(tcy*3) / static_cast<Float>(VBM_FLEXTEXTURE_SIZE);
		}
		else
		{
			pvboverts[i].flexcoord[0] = 0;
			pvboverts[i].flexcoord[1] = 0;
		}

		for(Uint32 k = 0; k < MAX_VBM_BONEWEIGHTS; k++)
		{
			pvboverts[i].boneindexes[k] = pvbmverts[i].boneindexes[k];
			pvboverts[i].boneweights[k] = (static_cast<Float>(pvbmverts[i].boneweights[k])/255.0f);
		}

		VBM_NormalizeWeights(pvboverts[i].boneweights, MAX_VBM_BONEWEIGHTS);
	}

	// Set the offsets to zero
	pvbm->ibooffset = 0;
	pvbm->vbooffset = 0;

	// Set vertex hash
	Uint32 bufferSize = pvbm->numverts * sizeof(vbmvertex_t);
	CMD5 vertexHash(reinterpret_cast<const byte*>(pvbm->getVertexes()), bufferSize);
	pvbmcache->vertexhash = vertexHash.HexDigest();

	// Set VBO
	CVBO* pVBO = new CVBO(gGLExtF, pvboverts, pvbm->numverts*sizeof(vbm_glvertex_t), pvbm->getIndexes(), pvbm->numindexes*sizeof(Uint32));
	pvbmcache->vboindex = m_pVBMVBOArray.size();
	m_pVBMVBOArray.push_back(pVBO);

	//
	// Set up textures
	//
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for(Int32 i = 0; i < pvbm->numtextures; i++)
	{
		vbmtexture_t *ptexture = pvbm->getTexture(static_cast<Uint32>(i));
		
		// Retreive material name
		CString materialscriptpath = GetModelTexturePath(pvbm->name, ptexture->name);
		en_material_t* pmaterial = pTextureManager->LoadMaterialScript(materialscriptpath.c_str(), RS_GAME_LEVEL);
		if(!pmaterial)
			pmaterial = pTextureManager->GetDummyMaterial();

		if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND))
			pvbm->flags |= VBM_HAS_BLEND_TEXTURE;

		if(pmaterial->flags & TX_FL_SCOPE)
			pvbm->flags |= VBM_HAS_SCOPE_TEXTURE;

		if(!pmaterial->containername.empty())
		{
			Con_EPrintf("%s - Container name specified for non-world material script '%s'. Texture will be null.\n", __FUNCTION__, materialscriptpath.c_str());
			pmaterial->ptextures[MT_TX_DIFFUSE] = pTextureManager->GetDummyTexture();
		}

		ptexture->index = pmaterial->index;
	}

	mcdheader_t* pmcd = pvbmcache->pmcdheader;
	if(pmcd)
	{
		for(Int32 i = 0; i < pmcd->numtextures; i++)
		{
			mcdtexture_t *ptexture = pmcd->getTexture(static_cast<Uint32>(i));
		
			// Retreive material name
			CString materialscriptpath = GetModelTexturePath(pvbm->name, ptexture->name);
			en_material_t* pmaterial = pTextureManager->LoadMaterialScript(materialscriptpath.c_str(), RS_GAME_LEVEL);
			if(!pmaterial)
				pmaterial = pTextureManager->GetDummyMaterial();

			if(!pmaterial->containername.empty())
			{
				Con_EPrintf("%s - Container name specified for non-world material script '%s'. Texture will be null.\n", __FUNCTION__, materialscriptpath.c_str());
				pmaterial->ptextures[MT_TX_DIFFUSE] = pTextureManager->GetDummyTexture();
			}

			ptexture->index = pmaterial->index;
		}
	}

	// 
	// Update bone compatibility settings in VBM data
	//
	if(VBM_PostLoadVBMCheck(pvbm, VBM_FindMaterialScriptByIndex))
	{
		// Alert client that change is needed
		Con_VPrintf("Texture flags settings forced a data recorrection on %s\n", pvbm->name);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::BuildVBOs( void )
{
	if(ens.isloading)
		VID_DrawLoadingScreen("Loading VBM geometry and textures");

	if(!m_pVBMVBOArray.empty())
	{
		for(Uint32 i = 0; i < m_pVBMVBOArray.size(); i++)
			delete m_pVBMVBOArray[i];

		m_pVBMVBOArray.clear();
	}

	if(m_pDecalVBO)
	{
		delete m_pDecalVBO;
		m_pDecalVBO = nullptr;
	}

	if(m_pTempDrawVBO)
	{
		delete m_pTempDrawVBO;
		m_pTempDrawVBO = nullptr;
	}
	
	for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
	{
		cache_model_t *pmodel = gModelCache.GetModelByIndex((i+1));
			
		if(!pmodel)
			break;

		if(pmodel->type != MOD_VBM)
			continue;

		vbmcache_t* pcache = pmodel->getVBMCache();
		if(!pcache->pstudiohdr || !pcache->pvbmhdr)
			continue;

		if(!pcache->pvbmhdr->numbodyparts)
			continue;

		// Create entry
		BuildVBMVBO(pcache);

		// Mark as loded into OpenGL
		pmodel->isloaded = true;
	}
	
	// Add in the decal buffer
	m_decalVertexCacheSize = m_pCvarDecalCacheSize->GetValue();
	if(m_decalVertexCacheSize < MIN_VBMDECAL_VERTEXES)
	{
		Con_Printf("Warning: value '%d' too low on cvar '%s', resetting to %d.\n", m_decalVertexCacheSize, m_pCvarDecalCacheSize->GetName(), static_cast<Int32>(MIN_VBMDECAL_VERTEXES));
		gConsole.CVarSetFloatValue(m_pCvarDecalCacheSize->GetName(), MIN_VBMDECAL_VERTEXES);
		m_decalVertexCacheSize = MIN_VBMDECAL_VERTEXES;
	}

	// Count 3 indexes for each vertex
	m_decalIndexCacheSize = m_decalVertexCacheSize*3;

	// Add these in
	m_vCache_Base = 0;
	m_vCache_Index = m_vCache_Base;

	m_iCache_Base = 0;
	m_iCache_Index = m_iCache_Base;

	// Create buffers for decals
	vbm_glvertex_t* pvertexbuffer = new vbm_glvertex_t[m_decalVertexCacheSize];
	memset(pvertexbuffer, 0, sizeof(vbm_glvertex_t)*m_decalVertexCacheSize);

	Uint32* pindexbuffer = new Uint32[m_decalIndexCacheSize];
	memset(pindexbuffer, 0, sizeof(Uint32)*m_decalIndexCacheSize);

	m_pDecalVBO = new CVBO(gGLExtF, pvertexbuffer, sizeof(vbm_glvertex_t)*m_decalVertexCacheSize, pindexbuffer, sizeof(Uint32)*m_decalIndexCacheSize);

	delete[] pvertexbuffer;
	delete[] pindexbuffer;

	// Create buffer for drawing debug stuff
	pvertexbuffer = new vbm_glvertex_t[MAX_TEMP_VBM_VERTEXES];
	memset(pvertexbuffer, 0, sizeof(vbm_glvertex_t)*MAX_TEMP_VBM_VERTEXES);

	m_pTempDrawVBO = new CVBO(gGLExtF, pvertexbuffer, sizeof(vbm_glvertex_t)*MAX_TEMP_VBM_VERTEXES, nullptr, 0);
	delete[] pvertexbuffer;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::RebuildVertexLightingVBOs( void )
{
	if(m_pVertexLightingVBOArray.empty())
		return true;

	for(Uint32 i = 0; i < m_pVertexLightingVBOArray.size(); i++)
	{
		vlight_vbo_t* pvlightvbo = m_pVertexLightingVBOArray[i];
		if(pvlightvbo[i].pvbo)
		{
			delete pvlightvbo[i].pvbo;
			pvlightvbo[i].pvbo = nullptr;
		}

		if(!BuildVertexLightVBO(pvlightvbo))
		{
			Sys_ErrorPopup("%s - Could not compile vertex lighting VBO at index %d for model '%s'.\n", __FUNCTION__, i, pvlightvbo->pvbmcache->pvbmhdr->name);
			return false;
		}
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::UpdateLightValues ( void )
{
	if(m_pExtraInfo)
	{
		if(m_pLightingInfo->lighttime > 0 && m_pLightingInfo->lighttime != -1)
		{
			// If blend time expired, then set the final values
			Float lightfulltime = m_pLightingInfo->lighttime + LIGHTING_LERP_TIME;
			if(lightfulltime < rns.time)
			{
				// Set final values
				Math::VectorCopy(m_pLightingInfo->target_ambient, m_pLightingInfo->ambient_color);
				Math::VectorCopy(m_pLightingInfo->target_diffuse, m_pLightingInfo->direct_color);
				Math::VectorCopy(m_pLightingInfo->target_lightdir, m_pLightingInfo->lightdirection);

				// Set final lightstyle values as well
				for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
				{
					if(m_pLightingInfo->target_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
					{
						m_pLightingInfo->lightstyles[i] = NULL_LIGHTSTYLE_INDEX;
						continue;
					}

					Math::VectorCopy(m_pLightingInfo->target_stylecolors_ambient[i], m_pLightingInfo->lightstylecolors_ambient[i]);
					Math::VectorCopy(m_pLightingInfo->target_stylecolors_diffuse[i], m_pLightingInfo->lightstylecolors_diffuse[i]);
					m_pLightingInfo->lightstyles[i] = m_pLightingInfo->target_lightstyles[i];
				}

				// Set this to signal that we don't need to do this again
				m_pLightingInfo->lighttime = -1;
			}
			else
			{
				Double lighttime = rns.time - m_pLightingInfo->lighttime;
				Double lightfrac = lighttime / LIGHTING_LERP_TIME;

				Vector tmp;
				Math::VectorScale(m_pLightingInfo->prev_ambient, (1.0 - lightfrac), tmp);
				Math::VectorMA(tmp, lightfrac, m_pLightingInfo->target_ambient, m_pLightingInfo->ambient_color);

				Math::VectorScale(m_pLightingInfo->prev_diffuse, (1.0 - lightfrac), tmp);
				Math::VectorMA(tmp, lightfrac, m_pLightingInfo->target_diffuse, m_pLightingInfo->direct_color);

				Math::VectorScale(m_pLightingInfo->prev_lightdir, (1.0 - lightfrac), tmp);
				Math::VectorMA(tmp, lightfrac, m_pLightingInfo->target_lightdir, m_pLightingInfo->lightdirection);

				// Lightstyle values array
				CArray<Float>* pstylevalues = gLightStyles.GetLightStyleValuesArray();

				for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
				{
					if(m_pLightingInfo->prev_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
						break;

					// Fetch lightstyle value
					Float stylevalue = (*pstylevalues)[m_pLightingInfo->prev_lightstyles[i]];

					// Handle ambient
					Math::VectorScale(m_pLightingInfo->prev_stylecolors_ambient[i], (1.0 - lightfrac), tmp);
					Math::VectorScale(tmp, stylevalue, m_pLightingInfo->lightstylecolors_ambient[i]);

					// Handle diffuse
					Math::VectorScale(m_pLightingInfo->prev_stylecolors_diffuse[i], (1.0 - lightfrac), tmp);
					Math::VectorScale(tmp, stylevalue, m_pLightingInfo->lightstylecolors_diffuse[i]);
				}

				// Blend in current style colors
				for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
				{
					if(m_pLightingInfo->target_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
						break;

					// Fetch lightstyle value
					Float stylevalue = (*pstylevalues)[m_pLightingInfo->target_lightstyles[i]];

					// Handle ambient
					Math::VectorScale(m_pLightingInfo->target_stylecolors_diffuse[i], lightfrac, tmp);
					Math::VectorMA(m_pLightingInfo->lightstylecolors_diffuse[i], stylevalue, tmp, m_pLightingInfo->lightstylecolors_diffuse[i]);

					// Handle diffuse
					Math::VectorScale(m_pLightingInfo->target_stylecolors_ambient[i], lightfrac, tmp);
					Math::VectorMA(m_pLightingInfo->lightstylecolors_ambient[i], stylevalue, tmp, m_pLightingInfo->lightstylecolors_ambient[i]);
				}
			}
		}
		else if(m_pLightingInfo->lighttime != -1)
		{
			// Reset this
			m_pLightingInfo->lighttime = -1;
		}
	}

	// Copy into the render target vectors
	Math::VectorCopy(m_pLightingInfo->lightdirection, m_renderLightVector);
	Math::VectorCopy(m_pLightingInfo->ambient_color, m_renderAmbientColor);
	Math::VectorCopy(m_pLightingInfo->direct_color, m_renderDiffuseColor);

	// Add in any lightstyle crap to local every frame, as
	// the lightstyle values can change on the fly
	for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
	{
		if(m_pLightingInfo->lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
			continue;

		if(m_pLightingInfo->lighttime > 0 && m_pLightingInfo->lighttime != -1)
		{
			// If blending, these are calculated already with style value in mind
			Math::VectorAdd(m_renderAmbientColor, m_pLightingInfo->lightstylecolors_ambient[i], m_renderAmbientColor);
			Math::VectorAdd(m_renderDiffuseColor, m_pLightingInfo->lightstylecolors_diffuse[i], m_renderDiffuseColor);
		}
		else
		{
			// We're grabbing from the final colors
			Float lightstylevalue = gLightStyles.GetLightStyleValue(m_pLightingInfo->lightstyles[i]);
			Math::VectorMA(m_renderAmbientColor, lightstylevalue, m_pLightingInfo->lightstylecolors_ambient[i], m_renderAmbientColor);
			Math::VectorMA(m_renderDiffuseColor, lightstylevalue, m_pLightingInfo->lightstylecolors_diffuse[i], m_renderDiffuseColor);
		}
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetupLighting ( Int32 flags )
{
	// Rebuild the entity's light origin each frame
	Vector lightorigin;
	Vector saved_lightorigin;

	if(m_pCurrentEntity->curstate.effects & EF_ALTLIGHTORIGIN)
	{
		// Use light origin if it's set
		Math::VectorCopy(m_pCurrentEntity->curstate.lightorigin, lightorigin);
	}
	else if(m_pCurrentEntity->curstate.renderfx != RenderFx_SkyEnt
		&& m_pCurrentEntity->curstate.renderfx != RenderFx_SkyEntScaled
		&& m_pStudioHeader->flags & STUDIO_MF_CENTERLIGHT)
	{
		Math::VectorScale(m_mins, 0.5, lightorigin);
		Math::VectorMA(lightorigin, 0.5, m_maxs, lightorigin);
	}
	else if( m_pCurrentEntity->curstate.effects & EF_CLIENTENT )
	{
		// Raise Z to center
		lightorigin[0] = m_renderOrigin[0];
		lightorigin[1] = m_renderOrigin[1];
		lightorigin[2] = m_mins[2]*0.5 + m_maxs[2]*0.5;
	}
	else
		Math::VectorCopy(m_renderOrigin, lightorigin);

	// Raise off the ground a bit
	Vector zadjust(0, 0, 4);
	if(m_pCurrentEntity->curstate.effects & EF_INVLIGHT)
		Math::VectorScale(zadjust, -1, zadjust);

	Math::VectorAdd(lightorigin, zadjust, lightorigin);
	Math::VectorCopy(lightorigin, saved_lightorigin);

	if( m_pExtraInfo )
	{
		// Copy values from global array into local
		m_pLightingInfo = m_pExtraInfo->plightinfo;

		// If we did not change, just keep blending values
		if(!m_pLightingInfo->reset && Math::VectorCompare(m_pLightingInfo->lastlightorigin, lightorigin))
		{
			UpdateLightValues();
			return;
		}
	}
	else
	{
		// Use local structure for non-tracked entities
		m_pLightingInfo = &m_localLightingInfo;
	}

	// Get sky light info
	bool gotLighting = false;
	bool gotBumpLighting = false;
	bool gotLightmapLighting = false;

	Vector surfnormal;
	byte lightstyles[MAX_SURFACE_STYLES] = { 0 };
	Vector lightdirs[MAX_SURFACE_STYLES];
	Vector lightcolors[MAX_SURFACE_STYLES];
	Vector lmapdiffusecolors[MAX_SURFACE_STYLES];

	// Try to trace against the sky vector
	if(rns.sky.drawsky && !cls.skycolor.IsZero() && m_pCvarSkyLighting->GetValue() >= 1)
	{
		Vector skytracevector;
		Vector skyvector = cls.skyvec;

		skyvector[2] = -skyvector[2];
		Math::VectorMA(lightorigin, -16384, skyvector, skytracevector);

		trace_t pmtrace;
		if(!(m_pStudioHeader->flags & STUDIO_MF_SKYLIGHT))
			CL_PlayerTrace(lightorigin, skytracevector, FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, pmtrace);

		if((m_pStudioHeader->flags & STUDIO_MF_SKYLIGHT) || !pmtrace.allSolid() && !pmtrace.startSolid() && !pmtrace.noHit()
			&& CL_PointContents(CL_GetEntityByIndex(0), pmtrace.endpos) == CONTENTS_SKY)
		{
			Math::VectorScale(cls.skycolor, 1.0f/255.0f, lightcolors[BASE_LIGHTMAP_INDEX]);
			Math::VectorCopy(skyvector, lightdirs[BASE_LIGHTMAP_INDEX]);

			if(m_pExtraInfo)
			{
				lightstyles[BASE_LIGHTMAP_INDEX] = 0;
				for(Uint32 i = 1; i < MAX_SURFACE_STYLES; i++)
					lightstyles[i] = NULL_LIGHTSTYLE_INDEX;
			}

			gotLighting = true;
		}
	}

	if(!gotLighting)
	{
		// Trace against the world
		Vector lighttop;
		Vector lightbottom;
		const brushmodel_t* pbrushmodel = nullptr;

		if(m_pCurrentEntity->curstate.groundent != NO_ENTITY_INDEX && 
			m_pCurrentEntity->curstate.groundent != WORLDSPAWN_ENTITY_INDEX
			&& !(m_pCurrentEntity->curstate.effects & EF_ALTLIGHTORIGIN)
			&& (m_pCurrentEntity->curstate.rendermode == RENDER_NORMAL
			|| (m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK) == RENDER_TRANSALPHA
			|| m_pCurrentEntity->curstate.renderamt > 0))
		{
			cl_entity_t* pentity = CL_GetEntityByIndex(m_pCurrentEntity->curstate.groundent);
			if(pentity && pentity->pmodel && pentity->pmodel->type == MOD_BRUSH
				&& !(pentity->curstate.effects &EF_COLLISION))
			{
				Vector offsetorigin;
				Math::VectorSubtract(lightorigin, pentity->curstate.origin, offsetorigin);
				if(!pentity->curstate.angles.IsZero())
					Math::RotateToEntitySpace(pentity->curstate.angles, offsetorigin);

				Math::VectorCopy(offsetorigin, lighttop);
				Math::VectorCopy(offsetorigin, lightbottom);
				if (m_pCurrentEntity->curstate.effects & EF_INVLIGHT) 
					lightbottom[2] += 8196;
				else
					lightbottom[2] -= 8196;

				const brushmodel_t* pentbrushmodel = pentity->pmodel->getBrushmodel();

				// Try and get bump data if possible
				if(m_pCvarUseBumpData->GetValue() >= 1.0 
					&& ens.pworld->plightdata[SURF_LIGHTMAP_AMBIENT]
					&& ens.pworld->plightdata[SURF_LIGHTMAP_DIFFUSE]
					&& ens.pworld->plightdata[SURF_LIGHTMAP_VECTORS])
				{
					gotLightmapLighting = Mod_RecursiveLightPoint_BumpData(pentbrushmodel, &pentbrushmodel->pnodes[pentbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, lightcolors, lmapdiffusecolors, lightdirs, &surfnormal, lightstyles);
					if(gotLightmapLighting)
					{
						gotBumpLighting = true;
						gotLighting = true;
					}
				}

				// If we didn't get bump data, use normal light data
				if(!gotLightmapLighting)
					gotLightmapLighting = Mod_RecursiveLightPoint(pentbrushmodel, &pentbrushmodel->pnodes[pentbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, lightcolors, lightstyles);

				if(gotLightmapLighting)
				{
					// Use this brushmodel for further lighting
					pbrushmodel = pentbrushmodel;
				}
			}
		}

		if(!pbrushmodel)
		{
			// Take lighting from world
			pbrushmodel = ens.pworld;

			// Set the trace bottom
			Math::VectorCopy(lightorigin, lighttop);
			Math::VectorCopy(lightorigin, lightbottom);
		
			if (m_pCurrentEntity->curstate.effects & EF_INVLIGHT) 
				lightbottom[2] += 8196;
			else
				lightbottom[2] -= 8196;

			// Try and get bump data if possible
			if(m_pCvarUseBumpData->GetValue() >= 1.0)
			{
				gotLightmapLighting = Mod_RecursiveLightPoint_BumpData(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, lightcolors, lmapdiffusecolors, lightdirs, &surfnormal, lightstyles);
				if(gotLightmapLighting)
				{
					gotBumpLighting = true;
					gotLighting = true;
				}
			}

			// If we didn't get bump data, use normal light data
			if(!gotLightmapLighting)
				gotLightmapLighting = Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, lightcolors, lightstyles);
		}

		// Only do this thing if we don't have bump data
		if(gotLightmapLighting && !gotBumpLighting)
		{
			Float offset = m_pCvarSampleOffset->GetValue();
			if(offset != 0)
			{
				if(offset < 0)
					offset = DEFAULT_LIGHTMAP_SAMPLE_OFFSET;

				// Sample 1
				Float strengths[4] = { 0 };
				Vector offsetu = lighttop +  Vector(-offset, -offset, 0);
				Vector offsetd = lightbottom + Vector(-offset, -offset, 0);

				Vector samplecolor;
				if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
					&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
				{
					offsetu = lightorigin + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					offsetd = lightbottom + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);

					Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
				}

				strengths[0] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

				// Sample 2
				offsetu = lighttop + Vector(offset, -offset, 0);
				offsetd = lightbottom + Vector(offset, -offset, 0);

				if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
					&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
				{
					offsetu = lightorigin + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					offsetd = lightbottom + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);

					Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
				}

				strengths[1] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

				// Sample 3
				offsetu = lighttop + Vector(offset, offset, 0);
				offsetd = lightbottom + Vector(offset, offset, 0);

				if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
					&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
				{
					offsetu = lightorigin + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					offsetd = lightbottom + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);

					Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
				}

				strengths[2] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

				// Sample 4
				offsetu = lighttop + Vector(-offset, offset, 0);
				offsetd = lightbottom + Vector(-offset, offset, 0);

				if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
					&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
				{
					offsetu = lightorigin + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					offsetd = lightbottom + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);

					Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
				}

				strengths[3] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

				Float length = Math::DotProduct4(strengths, strengths);
				length = SDL_sqrt(length);

				if(length)
				{
					Float ilength = 1.0f/length;
					for(Uint32 i = 0; i < 4; i++)
						strengths[i] *= ilength;
				}

				// Calculate final result
				lightdirs[BASE_LIGHTMAP_INDEX][0] = strengths[0] - strengths[1] - strengths[2] + strengths[3];
				lightdirs[BASE_LIGHTMAP_INDEX][1] = strengths[1] + strengths[0] - strengths[2] - strengths[3];
				lightdirs[BASE_LIGHTMAP_INDEX][2] = -1.0;

				Math::VectorNormalize(lightdirs[BASE_LIGHTMAP_INDEX]);
			}
			else
			{
				// Default to basic lightdir
				lightdirs[BASE_LIGHTMAP_INDEX] = Vector(0, 0, -1);
			}

			// We got proper lighting
			gotLighting = true;
		}
	}

	// If we don't get a lighting info, just rely on skyvec
	if(!gotLighting)
	{
		Math::VectorScale(cls.skycolor, 1.0f/255.0f, lightcolors[BASE_LIGHTMAP_INDEX]);
		Math::VectorCopy(cls.skyvec, lightdirs[BASE_LIGHTMAP_INDEX]);
		lightdirs[BASE_LIGHTMAP_INDEX][2] = -lightdirs[BASE_LIGHTMAP_INDEX][2];
		lightstyles[BASE_LIGHTMAP_INDEX] = 0;

		for(Uint32 i = 1; i < MAX_SURFACE_STYLES; i++)
			lightstyles[i] = NULL_LIGHTSTYLE_INDEX;
	}

	// Do modulations
	Vector tmp;
	Vector diffusecolors[MAX_SURFACE_STYLES];
	Vector ambientcolors[MAX_SURFACE_STYLES];
	Vector lightdir;

	// Assign final colors and lightvec
	if(!gotBumpLighting)
	{
		Float ratio = m_pCvarLightRatio->GetValue();
		if(ratio < 0)
			ratio = 0;
		else if(ratio > 1.0)
			ratio = 1.0;

		for(Uint32 j = 0; j < MAX_SURFACE_STYLES; j++)
		{
			if(lightstyles[j] == NULL_LIGHTSTYLE_INDEX)
				break;

			Math::VectorScale(lightcolors[j], ratio, diffusecolors[j]);
			Math::VectorScale(lightcolors[j], (1.0 - ratio), ambientcolors[j]);
		}

		Math::VectorCopy(lightdirs[BASE_LIGHTMAP_INDEX], lightdir);
	}
	else
	{
		Math::VectorCopy(lmapdiffusecolors[BASE_LIGHTMAP_INDEX], diffusecolors[BASE_LIGHTMAP_INDEX]);
		Math::VectorCopy(lightcolors[BASE_LIGHTMAP_INDEX], ambientcolors[BASE_LIGHTMAP_INDEX]);
		Math::VectorCopy(lightdirs[BASE_LIGHTMAP_INDEX], lightdir);

		for(Uint32 j = 1; j < MAX_SURFACE_STYLES; j++)
		{
			if(lightstyles[j] == NULL_LIGHTSTYLE_INDEX)
				break;

			Float dp = -Math::DotProduct(lightdirs[j], lightdirs[j]);
			if(dp > 1)
				dp = 1;
			else if(dp < 0)
				dp = 0;

			Math::VectorScale(lmapdiffusecolors[j], dp, diffusecolors[j]);
			Math::VectorCopy(lightcolors[j], ambientcolors[j]);
		}
	}

	// Reduce direct light based on how many lights are affecting us
	if((m_numModelLights > 0 || m_numDynamicLights > 0) 
		&& !(m_pCurrentEntity->curstate.flags & EF_LADDER)
		&& !R_IsEntityTransparent(*m_pCurrentEntity, true))
	{
		Uint32 numlights = m_numModelLights + m_numDynamicLights;
		if(numlights > NUM_LIGHT_REDUCTIONS) 
			numlights = NUM_LIGHT_REDUCTIONS;

		Float fllightreduction = (static_cast<Float>(numlights)/NUM_LIGHT_REDUCTIONS);
		if(fllightreduction > 1.0)
			fllightreduction = 1.0;

		// Modify the light values
		Math::VectorScale(diffusecolors[BASE_LIGHTMAP_INDEX], fllightreduction*0.6, tmp);
		Math::VectorSubtract(diffusecolors[BASE_LIGHTMAP_INDEX], tmp, diffusecolors[BASE_LIGHTMAP_INDEX]);
	
		Math::VectorScale(ambientcolors[BASE_LIGHTMAP_INDEX], fllightreduction*0.4, tmp);
		Math::VectorSubtract(ambientcolors[BASE_LIGHTMAP_INDEX], tmp, ambientcolors[BASE_LIGHTMAP_INDEX]);
	}

	// Only do anything if the values actually changed
	if(m_pExtraInfo)
	{
		if(rns.time == 0 || m_pLightingInfo->lighttime == 0 || m_pLightingInfo->reset 
			|| !CompareLightValues(ambientcolors, diffusecolors, lightdir, lightstyles))
		{
			if(rns.time == 0 || m_pLightingInfo->lighttime == 0 
				|| m_pExtraInfo->plightinfo->reset || (m_pLightingInfo->flags & MDL_LIGHT_NOBLEND))
			{
				Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_pLightingInfo->ambient_color);
				Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_pLightingInfo->direct_color);
				Math::VectorCopy(lightdir, m_pLightingInfo->lightdirection);

				for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
				{
					if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
					{
						Math::VectorCopy(ambientcolors[i+1], m_pLightingInfo->lightstylecolors_ambient[i]);
						Math::VectorCopy(diffusecolors[i+1], m_pLightingInfo->lightstylecolors_diffuse[i]);
					}

					m_pLightingInfo->lightstyles[i] = lightstyles[i+1];
				}

				m_pLightingInfo->lighttime = -1;
				m_pLightingInfo->lastlightorigin = lightorigin;
			}
			else
			{
				// Set current light values as the previous value to blend from
				Math::VectorCopy(m_pLightingInfo->ambient_color, m_pLightingInfo->prev_ambient);
				Math::VectorCopy(m_pLightingInfo->direct_color, m_pLightingInfo->prev_diffuse);
				Math::VectorCopy(m_pLightingInfo->lightdirection, m_pLightingInfo->prev_lightdir);

				// Set lightstyles too(this isn't 100% correct, but fuck it, 
				// the solution would be extremely overcomplicated)
				if(m_pLightingInfo->lighttime != -1)
				{
					for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
					{
						if(m_pLightingInfo->target_lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
						{
							Math::VectorCopy(m_pLightingInfo->target_stylecolors_ambient[i], m_pLightingInfo->prev_stylecolors_ambient[i]);
							Math::VectorCopy(m_pLightingInfo->target_stylecolors_diffuse[i], m_pLightingInfo->prev_stylecolors_diffuse[i]);
						}

						m_pLightingInfo->prev_lightstyles[i] = m_pLightingInfo->target_lightstyles[i];
					}
				}
				else
				{
					for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
					{
						if(m_pLightingInfo->lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
						{
							Math::VectorCopy(m_pLightingInfo->lightstylecolors_ambient[i], m_pLightingInfo->prev_stylecolors_ambient[i]);
							Math::VectorCopy(m_pLightingInfo->lightstylecolors_diffuse[i], m_pLightingInfo->prev_stylecolors_diffuse[i]);
						}

						m_pLightingInfo->prev_lightstyles[i] = m_pLightingInfo->lightstyles[i];
					}
				}

				// Set target
				Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_pLightingInfo->target_ambient);
				Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_pLightingInfo->target_diffuse);
				Math::VectorCopy(lightdir, m_pLightingInfo->target_lightdir);

				for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
				{
					if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
					{
						Math::VectorCopy(ambientcolors[i+1], m_pLightingInfo->target_stylecolors_ambient[i]);
						Math::VectorCopy(diffusecolors[i+1], m_pLightingInfo->target_stylecolors_diffuse[i]);
					}

					m_pLightingInfo->target_lightstyles[i] = lightstyles[i+1];
				}

				// Set time because target light values changed
				m_pLightingInfo->lighttime = rns.time;
			}
		}

		// Make sure this flag is removed
		if(m_pLightingInfo->reset)
			m_pLightingInfo->reset = false;

		// Always set this
		m_pLightingInfo->lastlightorigin = lightorigin;
	}
	else
	{
		Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_pLightingInfo->ambient_color);
		Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_pLightingInfo->direct_color);
		Math::VectorCopy(lightdir, m_pLightingInfo->lightdirection);

		for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
		{
			if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
			{
				Math::VectorCopy(ambientcolors[i+1], m_pLightingInfo->lightstylecolors_ambient[i]);
				Math::VectorCopy(diffusecolors[i+1], m_pLightingInfo->lightstylecolors_diffuse[i]);
			}

			m_pLightingInfo->lightstyles[i] = lightstyles[i+1];
		}
	}

	// Call this so render values are set
	UpdateLightValues();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::CompareLightValues( const Vector* pambientlightvalues, const Vector* pdiffuselightvalues, const Vector& lightdir, const byte* plightstyles )
{
	if(!Math::VectorCompare(lightdir, m_pLightingInfo->target_lightdir))
		return false;

	if(!Math::VectorCompare(pambientlightvalues[BASE_LIGHTMAP_INDEX], m_pLightingInfo->target_ambient))
		return false;

	if(!Math::VectorCompare(pdiffuselightvalues[BASE_LIGHTMAP_INDEX], m_pLightingInfo->target_diffuse))
		return false;

	for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
	{
		if(m_pLightingInfo->target_lightstyles[i] != plightstyles[i+1])
			return false;

		if(m_pLightingInfo->target_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
			continue;

		if(!Math::VectorCompare(pambientlightvalues[i+1], m_pLightingInfo->target_stylecolors_ambient[i]))
			return false;

		if(!Math::VectorCompare(pdiffuselightvalues[i+1], m_pLightingInfo->target_stylecolors_diffuse[i]))
			return false;
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::GetModelLights( void )
{
	m_numModelLights = 0;

	trace_t pmtrace;
	Vector vcenter, vlightdir;
	Math::VectorAdd(m_mins, m_maxs, vcenter);
	Math::VectorScale(vcenter, 0.5, vcenter);

	// Get pointer to data if it's present
	entity_lightinfo_t* plightinfo = nullptr;
	if(m_pExtraInfo)
		plightinfo = m_pExtraInfo->plightinfo;

	// collect strongest modellights
	Uint32 nummodellights = 0;
	mlight_t* plightslist[MAX_ENT_ACTIVE_DLIGHTS] = { nullptr };
	Float lightstrengths[MAX_ENT_ACTIVE_DLIGHTS] = { 0 };

	//
	// Reduce max active lights to 4 model/entity lights,
	// and keep 2 slots for fading lights. This should help
	// reduce sudden changes from other lights being culled
	// out due to limits

	// Total lights affecting model
	CArray<mlight_t*> ptotallights;

	// Look in the modellights array first
	mlight_t *mlight = rns.objects.modellights;
	for (Uint32 i = 0; i < rns.objects.nummodellights; i++, mlight++)
	{
		if (Math::CheckMinsMaxs(mlight->mins, mlight->maxs, m_mins, m_maxs))
			continue;
		
		// perform trace
		if(!(m_pCurrentEntity->curstate.effects & EF_NOELIGHTTRACE))
		{
			CL_PlayerTrace(vcenter, mlight->origin, FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, pmtrace);
			if (pmtrace.fraction < 1.0 && !(pmtrace.flags & FL_TR_STARTSOLID))
				continue; // blocked
		}

		Math::VectorSubtract(mlight->origin, vcenter, vlightdir);
		Float radius = mlight->radius*mlight->radius;
		Float dist = Math::DotProduct(vlightdir, vlightdir);
		Float strength = (mlight->color.x + mlight->color.y + mlight->color.z)/3.0f;
		strength *= clamp((dist/radius-1.0) * -1.0, 0.0, 1.0);

		if(nummodellights == MAX_ENT_ACTIVE_DLIGHTS)
		{
			for(Uint32 j = 0; j < MAX_ENT_ACTIVE_DLIGHTS; j++)
			{
				if(strength > lightstrengths[j])
				{
					plightslist[j] = mlight;
					lightstrengths[j] = strength;
					break;
				}
			}
		}
		else
		{
			plightslist[nummodellights] = mlight;
			lightstrengths[nummodellights] = strength;
			nummodellights++;
		}

		// Remember for total list
		ptotallights.push_back(mlight);
	}	

	// Parse the lights we've gathered
	for(Uint32 i = 0; i < nummodellights; i++)
	{
		mlight = plightslist[i];

		// Add this to the list
		mlightinfo_t* pinfo = &m_modelLights[m_numModelLights];
		m_numModelLights++;

		// Set light data
		pinfo->light = (*mlight);

		if(plightinfo && mlight->entindex 
			&& !(plightinfo->flags & MDL_LIGHT_NOBLEND) 
			&& !mlight->noblend)
		{
			// try and find this in the previous list
			mlightinfo_t *pprevinfo = nullptr;
			for(Uint32 j = 0; j < plightinfo->numsavedmlights; j++)
			{
				if(plightinfo->savedmlights[j].light.entindex == mlight->entindex)
				{
					pprevinfo = &plightinfo->savedmlights[j];
					break;
				}
			}

			if(plightinfo->flags & MDL_LIGHT_FIRST)
			{
				// Don't blend at map spawn
				pinfo->lighttime = rns.time;
				pinfo->strength = 1.0;
				pinfo->prev_strength = 1.0;
			}
			else if(!pprevinfo)
			{
				// New light, so set lighttime and strength
				pinfo->lighttime = rns.time;
				pinfo->strength = 0.0;
				pinfo->prev_strength = 0.0;
			}
			else
			{
				// Previous light, copy values
				if(pinfo->occluded)
				{
					pinfo->lighttime = rns.time;
					pinfo->strength = pprevinfo->strength;
					pinfo->prev_strength = pprevinfo->strength;
				}
				else
				{
					pinfo->lighttime = pprevinfo->lighttime;
					pinfo->strength = pprevinfo->strength;
					pinfo->prev_strength = pprevinfo->prev_strength;
				}
			}
		}
		else
		{
			// no fade in/out
			pinfo->lighttime = 0;
			pinfo->strength = 1.0;
			pinfo->prev_strength = 1.0;
		}

		// Reset this
		pinfo->occluded = false;
	}

	// Check previous list for touching but occluded lights
	if(plightinfo && !(plightinfo->flags & MDL_LIGHT_NOBLEND))
	{
		for(Uint32 i = 0; i < plightinfo->numsavedmlights; i++)
		{
			if(m_numModelLights == MAX_ENT_MLIGHTS)
				break;

			mlightinfo_t *pprevinfo = &plightinfo->savedmlights[i];
			if(!pprevinfo->light.entindex)
				continue;

			// Check that it's not on the current list
			Uint32 j = 0;
			for(; j < m_numModelLights; j++)
			{
				if(m_modelLights[j].light.entindex == pprevinfo->light.entindex)
					break;
			}

			if(j != m_numModelLights)
				continue;

			if(Math::CheckMinsMaxs(pprevinfo->light.mins, pprevinfo->light.maxs, m_mins, m_maxs))
				continue;
			
			if(pprevinfo->strength <= 0)
				continue;

			if(pprevinfo->occluded)
			{
				Float lightendtime = pprevinfo->lighttime + LIGHTING_LERP_TIME;
				if(lightendtime <= rns.time)
					continue;
			}

			if(!(m_pCurrentEntity->curstate.effects & EF_NOELIGHTTRACE))
			{
				CL_PlayerTrace(vcenter, pprevinfo->light.origin, FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, pmtrace);
				if (pmtrace.fraction == 1.0)
				{
					j = 0;
					for(; j < ptotallights.size(); j++)
					{
						if(pprevinfo->light.entindex == ptotallights[j]->entindex)
							break;
					}

					if(j == ptotallights.size())
					{
						// not blocked, not on total list, probably turned off
						continue; 
					}
				}
			}

			mlightinfo_t* pinfo = &m_modelLights[m_numModelLights];
			m_numModelLights++;

			// Set light data
			memcpy(&pinfo->light, &pprevinfo->light, sizeof(mlight_t));

			if(!pprevinfo->occluded)
			{
				pinfo->lighttime = rns.time;
				pinfo->prev_strength = pprevinfo->strength;
			}
			else
			{
				pinfo->lighttime = pprevinfo->lighttime;
				pinfo->prev_strength = pprevinfo->prev_strength;
			}

			// Make sure it's set
			pinfo->strength = pprevinfo->strength;
			pinfo->occluded = true;
		}

		// apply fade in/out on lights
		for(Uint32 i = 0; i < m_numModelLights; i++)
		{
			if(!m_modelLights[i].light.entindex)
				continue;

			if(!m_modelLights[i].lighttime)
				continue;

			Float lightendtime = m_modelLights[i].lighttime + LIGHTING_LERP_TIME;
			if(lightendtime <= rns.time)
			{
				m_modelLights[i].strength = 1.0;
				m_modelLights[i].lighttime = 0;
				continue;
			}

			Double lightfrac = rns.time - m_modelLights[i].lighttime;
			lightfrac = lightfrac / LIGHTING_LERP_TIME;
			
			if(lightfrac > 1.0) 
				lightfrac = 1.0;

			if(m_modelLights[i].occluded)
				m_modelLights[i].strength = m_modelLights[i].prev_strength * (1.0 - lightfrac);
			else
				m_modelLights[i].strength = m_modelLights[i].prev_strength * (1.0 - lightfrac) + lightfrac * 1.0;
		}

		// Copy this to the lightinfo struct
		memcpy(plightinfo->savedmlights, m_modelLights, sizeof(mlightinfo_t)*m_numModelLights);
		plightinfo->numsavedmlights = m_numModelLights;

		// Clear first time flag
		if(plightinfo->flags & MDL_LIGHT_FIRST)
			plightinfo->flags &= ~MDL_LIGHT_FIRST;
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::GetDynamicLights( void )
{
	m_numDynamicLights = 0;

	CLinkedList<cl_dlight_t*>& dlList = gDynamicLights.GetLightList();
	if(dlList.empty())
		return;

	trace_t pmtrace;

	Vector vcenter;
	Math::VectorAdd(m_mins, m_maxs, vcenter);
	Math::VectorScale(vcenter, 0.5, vcenter);

	dlList.begin();
	while(!dlList.end())
	{
		cl_dlight_t* pdl = dlList.get();

		// This needs to be set every time(also, be sure to use the viewsize from view_params!)
		if(!DL_IsLightVisible(rns.view.frustum, pdl->mins, pdl->maxs, pdl))
		{
			dlList.next();
			continue;
		}		
		
		if(m_numDynamicLights == MAX_ENT_DLIGHTS)
			return;

		if(pdl->pfrustum)
		{
			if(pdl->pfrustum->CullBBox(m_mins, m_maxs))
			{
				dlList.next();
				continue;
			}
		}
		else
		{
			if(Math::CheckMinsMaxs(pdl->mins, pdl->maxs, m_mins, m_maxs))
			{
				dlList.next();
				continue;
			}
		}

		m_pDynamicLights[m_numDynamicLights] = pdl;
		m_numDynamicLights++;

		dlList.next();
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::CheckBBox( void )
{
	if(m_pCurrentEntity->curstate.sequence < 0 || m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	// Build full bounding box
	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);

	Vector vTemp;
	for (Uint32 i = 0; i < 8; i++)
	{
		if ( i & 1 ) 
			vTemp[0] = pseqdesc->bbmin[0];
		else 
			vTemp[0] = pseqdesc->bbmax[0];

		if ( i & 2 ) 
			vTemp[1] = pseqdesc->bbmin[1];
		else 
			vTemp[1] = pseqdesc->bbmax[1];

		if ( i & 4 ) 
			vTemp[2] = pseqdesc->bbmin[2];
		else 
			vTemp[2] = pseqdesc->bbmax[2];

		Math::VectorCopy( vTemp, m_bboxCorners[i] );
	}

	Vector angles = m_pCurrentEntity->curstate.angles;
	angles[PITCH] = -angles[PITCH];

	Math::AngleMatrix(angles, (*m_pRotationMatrix));

	for (Uint32 i = 0; i < 8; i++ )
	{
		Math::VectorCopy(m_bboxCorners[i], vTemp);
		Math::VectorRotate(vTemp, (*m_pRotationMatrix), m_bboxCorners[i]);
	}

	// Set the bounding box
	Vector vMins = NULL_MINS;
	Vector vMaxs = NULL_MAXS;
	for(Uint32 i = 0; i < 8; i++)
	{
		// Mins
		if(m_bboxCorners[i][0] < vMins[0]) 
			vMins[0] = m_bboxCorners[i][0];
		if(m_bboxCorners[i][1] < vMins[1]) 
			vMins[1] = m_bboxCorners[i][1];
		if(m_bboxCorners[i][2] < vMins[2]) 
			vMins[2] = m_bboxCorners[i][2];

		// Maxs
		if(m_bboxCorners[i][0] > vMaxs[0]) 
			vMaxs[0] = m_bboxCorners[i][0];
		if(m_bboxCorners[i][1] > vMaxs[1]) 
			vMaxs[1] = m_bboxCorners[i][1];
		if(m_bboxCorners[i][2] > vMaxs[2]) 
			vMaxs[2] = m_bboxCorners[i][2];
	}

	// Make sure stuff like barnacles work fine
	if(m_pStudioHeader->numbonecontrollers)
	{
		Float *pcontroller1 = m_pCurrentEntity->curstate.controllers;
		Float *pcontroller2 = m_pCurrentEntity->latched.controllers;
		Float flInterp = VBM_EstimateInterpolant(rns.time, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->latched.animtime);

		for (Int32 j = 0; j < m_pStudioHeader->numbonecontrollers; j++)
		{
			const mstudiobonecontroller_t *pbonecontroller = m_pStudioHeader->getBoneController(j);

			if(!(pbonecontroller->type & STUDIO_Y))
				continue;

			if(pbonecontroller->type & STUDIO_RLOOP)
				continue;

			Uint32 iIndex = pbonecontroller->index;
			Float flValue = (pcontroller1[iIndex]*flInterp+pcontroller2[iIndex]*(1.0-flInterp))/255.0;
		
			if (flValue < 0) flValue = 0;
			if (flValue > 1) flValue = 1;
			
			vMins[2] += (1.0-flValue)*pbonecontroller->start+flValue*pbonecontroller->end;
		}
	}

	// Scale mins/maxs as well
	if ((m_pCurrentEntity->curstate.renderfx == RenderFx_ScaledModel
		|| m_pCurrentEntity->curstate.renderfx == RenderFx_SkyEntScaled
		|| m_pCurrentEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
		&& (m_pExtraInfo->paniminfo->scale != m_pCurrentEntity->curstate.scale))
	{
		Math::VectorScale(vMins, m_pCurrentEntity->curstate.scale, vMins);
		Math::VectorScale(vMaxs, m_pCurrentEntity->curstate.scale, vMaxs);
	}

	// Add in origin
	Math::VectorAdd(m_pCurrentEntity->curstate.origin, vMins, m_mins);
	Math::VectorAdd(m_pCurrentEntity->curstate.origin, vMaxs, m_maxs);
	
	if(m_pExtraInfo)
	{
		m_pExtraInfo->absmin = m_mins;
		m_pExtraInfo->absmax = m_maxs;
	}

	// View entity is always present
	if(m_pCurrentEntity == cls.dllfuncs.pfnGetViewModel())
		return false;

	return rns.view.frustum.CullBBox(m_mins, m_maxs);
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::Render( Int32 flags )
{
	m_isMultiPass = false;

	if(!(flags & VBM_DEBUG_ONLY))
	{
		// Set transparency
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);

		Int32 trueRenderMode = (m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK);
		switch(trueRenderMode)
		{
		case RENDER_TRANSADDITIVE:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				m_renderAlpha = R_RenderFxBlend(m_pCurrentEntity) / 255.0;
				m_useBlending = true;
			}
			break;
		case RENDER_TRANSALPHA: 
		case RENDER_TRANSTEXTURE:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				m_renderAlpha = R_RenderFxBlend(m_pCurrentEntity) / 255.0;
				m_useBlending = true;
			}
			break;
		default:
			{
				m_renderAlpha = 1.0;
				m_useBlending = false;
			}
			break;
		}

		if(!DrawNormalSubmodels())
			return false;

		if(!DrawFlexedSubmodels())
			return false;

		if (m_useBlending)
		{
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}

		if(!DrawWireframe())
			return false;
	}

	if(!DrawDebug())
		return false;

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawDebug( void )
{
	// Check for drawing special stuff
	if(m_pCvarDrawModels->GetValue() < 2)
		return true;

	// Disable all attribs pointing to previous VBO
	Uint32 attribCount = m_pShader->GetNbAttributes();
	for(Uint32 i = 0; i < attribCount; i++) 
		m_pShader->DisableAttribute(i);

	// Set the current VBO and enable basic attribs we'll need
	m_pShader->SetVBO(m_pTempDrawVBO);
	m_pCurrentVBO = m_pTempDrawVBO;

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	switch(static_cast<Int32>(m_pCvarDrawModels->GetValue()))
	{
	case 2:
		{
			if(!DrawBones())
				return false;
		}
		break;
	case 3:
		{
			if(!DrawHitBoxes())
				return false;
		}
		break;
	case 4:
		{
			if(!DrawBoundingBox())
				return false;
		}
		break;
	case 5:
		{
			if(!DrawHullBoundingBox())
				return false;
		}
		break;
	case 6:
		{
			if(!DrawLightVectors())
				return false;
		}
		break;
	case 7:
		{
			if(!DrawAttachments())
				return false;
		}
		break;
	default:
		break;
	}

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::SetupRenderer( void )
{
	// If in water with caustics, add to multipass
	if(rns.inwater && g_pCvarCaustics->GetValue() >= 1 && m_pCurrentEntity->curstate.renderfx != RenderFx_NoDynamicLighting
		&& gWaterShader.GetWaterQualitySetting() > CWaterShader::WATER_QUALITY_NO_REFLECT_REFRACT)
	{
		const water_settings_t* pwatersettings = gWaterShader.GetActiveSettings();
		if(pwatersettings && pwatersettings->causticscale > 0 && pwatersettings->causticstrength > 0)
			m_isMultiPass = true;
	}
	
	// If water caustics did not add to multipass, then check for lights
	if(!m_isMultiPass && (m_numDynamicLights != 0 && g_pCvarDynamicLights->GetValue() >= 1))
		m_isMultiPass = true;

	// If doing baked lighting, check for styles
	if(!m_isMultiPass && m_pCurrentEntity->curstate.vlight_vbo_index != NO_POSITION)
	{
		CArray<Float>* pstylesarray = gLightStyles.GetLightStyleValuesArray();
		for(Uint32 i = 1; i < MAX_ENTITY_STYLES; i++)
		{
			if(m_pCurrentEntity->curstate.vlight_styles[i] != NULL_LIGHTSTYLE_INDEX 
				&& (*pstylesarray)[m_pCurrentEntity->curstate.vlight_styles[i]] > 0)
			{
				m_isMultiPass = true;
				break;
			}
		}
	}

	bool result = false;
	if(!rns.fog.settings.active && !m_isMultiPass) 
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture, false);
	else if(m_isMultiPass)
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_notexture, false);
	else
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture_fog, false);

	if(!result)
		return false;

	m_pShader->SetUniform3f(m_attribs.u_vorigin, rns.view.v_origin[0], rns.view.v_origin[1], rns.view.v_origin[2]);
	m_pShader->SetUniform3f(m_attribs.u_vright, rns.view.v_right[0], rns.view.v_right[1], rns.view.v_right[2]);

	m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
	m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));

	Vector vtransformed;
	CMatrix pmatrix(rns.view.modelview.GetInverse());
	Math::MatMult(pmatrix.Transpose(), m_renderLightVector, &vtransformed);

	m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_ambient, m_renderAmbientColor[0], m_renderAmbientColor[1], m_renderAmbientColor[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, m_renderDiffuseColor[0], m_renderDiffuseColor[1], m_renderDiffuseColor[2]);

	const Float *fltranspose = rns.view.modelview.Transpose();
	for(Uint32 i = 0; i < m_numModelLights; i++)
	{
		Math::MatMultPosition(fltranspose, m_modelLights[i].light.origin, &vtransformed);

		Vector colorScaled;
		Math::VectorScale(m_modelLights[i].light.color, m_modelLights[i].strength, colorScaled);

		if(!m_areUBOsSupported)
		{
			m_pShader->SetUniform3f(m_attribs.lights[i].u_origin, vtransformed[0], vtransformed[1], vtransformed[2]);
			m_pShader->SetUniform3f(m_attribs.lights[i].u_color, colorScaled[0], colorScaled[1], colorScaled[2]);
			m_pShader->SetUniform1f(m_attribs.lights[i].u_radius, m_modelLights[i].light.radius);
		}
		else
		{
			Math::VectorCopy(vtransformed, m_uboModelLightData[i].origin);
			Math::VectorCopy(colorScaled, m_uboModelLightData[i].color);
			m_uboModelLightData[i].radius[0] = m_modelLights[i].light.radius;
		}
	}

	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_modellights, (void*)m_uboModelLightData, sizeof(m_uboModelLightData));

	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, m_numModelLights);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);

	m_pShader->EnableAttribute(m_attribs.a_normal);
	m_pShader->EnableAttribute(m_attribs.a_tangent);
	m_pShader->EnableAttribute(m_attribs.a_texcoord1);

	// Bind VBO for baked vertex lighting if any
	if(m_pCurrentEntity->curstate.vlight_vbo_index != NO_POSITION
		&& m_pVertexLightingVBOArray.size() > m_pCurrentEntity->curstate.vlight_vbo_index)
	{
		vlight_vbo_t* pvblightvbo = m_pVertexLightingVBOArray[m_pCurrentEntity->curstate.vlight_vbo_index];
		m_pShader->SetVBO(pvblightvbo->pvbo, 1);

		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_vectors, OFFSET(vbm_vlight_glvertex_t, vertexlight0_vector), 1);
		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_ambient, OFFSET(vbm_vlight_glvertex_t, vertexlight0_ambient), 1);
		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_diffuse, OFFSET(vbm_vlight_glvertex_t, vertexlight0_diffuse), 1);

		m_pShader->EnableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_diffuse);

		if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, TRUE, false))
			return false;
	}
	else
	{
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_diffuse);

		if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, FALSE, false))
			return false;
	}

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha);

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_light_radius);

	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
	}

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->EnableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_vorigin);
	m_pShader->EnableSync(m_attribs.u_vright);
	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_sky_ambient);
	m_pShader->EnableSync(m_attribs.u_sky_diffuse);
	m_pShader->EnableSync(m_attribs.u_sky_dir);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);
	m_pShader->EnableSync(m_attribs.u_color);

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::RestoreRenderer( void )
{
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);

	if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE))
		return false;

	m_pShader->DisableAttribute(m_attribs.a_flexcoord);
	m_pShader->DisableAttribute(m_attribs.a_texcoord2);

	if(m_pCurrentEntity->curstate.vlight_vbo_index != NO_POSITION)
	{
		// Disable the attributes used by baked vertex lighting
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_diffuse);

		// Unbind secondary VBO from shader
		m_pShader->SetVBO(nullptr, 1);

		if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, FALSE, false))
			return false;
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::CalculateFlexesHW( const vbmsubmodel_t* psubmodel )
{
	const vbmflexinfo_t* pflexinfo = m_pVBMHeader->getFlexInfo(psubmodel->flexinfoindex);
	const vbmflexvertex_t* pflexverts = pflexinfo->getFlexVertexes(m_pVBMHeader);
	const vbmflexvertinfo_t* pflexvertinfos = pflexinfo->getFlexVertexInfos(m_pVBMHeader);
	
	const vbmflexcontroller_t* pflexcontrollers = m_pVBMHeader->getFlexControllers();
	const byte* pflexcontrollerindexes = pflexinfo->getFlexControllerIndexes(m_pVBMHeader);

	Int32 height = 0;
	for(Int32 i = 0; i < pflexinfo->numflexvertinfo; i++)
	{
		vec4_t* originoffset = m_flexTexels + pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset;
		vec4_t* normoffset = m_flexTexels + pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset + 1;

		if( pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset / VBM_FLEXTEXTURE_SIZE > height )
			height = (pflexverts[pflexvertinfos[i].vertinfoindexes[0]].offset / VBM_FLEXTEXTURE_SIZE);

		// Reset to base value
		for(Uint32 j = 0; j < 4; j++)
		{
			(*originoffset)[j] = 0;
			(*normoffset)[j] = 0;
		}

		for(Int32 j = 1; j < pflexinfo->numflexes; j++)
		{
			if(pflexvertinfos[i].vertinfoindexes[j] == -1)
				continue;

			Int32 controller_idx = pflexcontrollerindexes[j];
			Int32 script_idx = m_pExtraInfo->pflexstate->indexmap[controller_idx];
			if(script_idx == -1)
				continue;

			Float value = m_pExtraInfo->pflexstate->values[script_idx];
			value *= (pflexcontrollers[controller_idx].maxvalue - pflexcontrollers[controller_idx].minvalue);
			value += pflexcontrollers[controller_idx].minvalue;

			const vbmflexvertex_t* pflexvert = &pflexverts[pflexvertinfos[i].vertinfoindexes[j]];

			for(Uint32 k = 0; k < 3; k++)
			{
				(*originoffset)[k] = (*originoffset)[k] + pflexvert->originoffset[k] * value;
				(*normoffset)[k] = (*normoffset)[k] + pflexvert->normaloffset[k] * value;
			}
		}
	}

	// Now upload the texture
	R_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index, true);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VBM_FLEXTEXTURE_SIZE, height + 1, GL_RGBA, GL_FLOAT, m_flexTexels);
}

//=============================================
//
//
//=============================================
void CVBMRenderer::CalculateFlexesSW( const vbmsubmodel_t* psubmodel )
{
	const vbmflexinfo_t* pflexinfo = m_pVBMHeader->getFlexInfo(psubmodel->flexinfoindex);
	const vbmflexvertex_t* pflexverts = pflexinfo->getFlexVertexes(m_pVBMHeader);
	const vbmflexvertinfo_t* pflexvertinfos = pflexinfo->getFlexVertexInfos(m_pVBMHeader);
	
	const vbmflexcontroller_t* pflexcontrollers = m_pVBMHeader->getFlexControllers();
	const byte* pflexcontrollerindexes = pflexinfo->getFlexControllerIndexes(m_pVBMHeader);

	const vbmvertex_t* pvbmverts = m_pVBMHeader->getVertexes() + pflexinfo->first_vertex;

	for(Int32 i = 0; i < pflexinfo->num_vertexes; i++)
	{
		Math::VectorCopy(pvbmverts[i].origin, m_tempVertexes[i].origin);
		Math::VectorCopy(pvbmverts[i].normal, m_tempVertexes[i].normal);
		Math::VectorCopy(pvbmverts[i].tangent, m_tempVertexes[i].tangent);

		if(pvbmverts[i].flexvertindex != -1)
		{
			for(Int32 j = 1; j < pflexinfo->numflexes; j++)
			{
				if(pflexvertinfos[pvbmverts[i].flexvertindex].vertinfoindexes[j] == -1)
					continue;

				Int32 controller_idx = pflexcontrollerindexes[j];
				Int32 script_idx = m_pExtraInfo->pflexstate->indexmap[controller_idx];
				if(script_idx == -1)
					continue;

				Float value = m_pExtraInfo->pflexstate->values[script_idx];
				value *= (pflexcontrollers[controller_idx].maxvalue - pflexcontrollers[controller_idx].minvalue);
				value += pflexcontrollers[controller_idx].minvalue;

				const vbmflexvertex_t* pflexvert = &pflexverts[pflexvertinfos[pvbmverts[i].flexvertindex].vertinfoindexes[j]];
				Math::VectorMA(m_tempVertexes[i].origin, value, pflexvert->originoffset, m_tempVertexes[i].origin);
				Math::VectorMA(m_tempVertexes[i].normal, value, pflexvert->normaloffset, m_tempVertexes[i].normal);
			}

			Math::VectorNormalize(m_tempVertexes[i].normal);
		}

		m_tempVertexes[i].flexcoord[0] = 0;
		m_tempVertexes[i].flexcoord[1] = 0;

		m_tempVertexes[i].texcoord1[0] = pvbmverts[i].texcoord[0];
		m_tempVertexes[i].texcoord1[1] = pvbmverts[i].texcoord[1];

		for(Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
		{
			m_tempVertexes[i].boneindexes[j] = pvbmverts[i].boneindexes[j];
			m_tempVertexes[i].boneweights[j] = (static_cast<Float>(pvbmverts[i].boneweights[j])/255.0f);
		}

		VBM_NormalizeWeights(m_tempVertexes[i].boneweights, MAX_VBM_BONEWEIGHTS);
	}
 
	// Update the VBO
	m_pCurrentVBO->VBOSubBufferData((m_pVBMHeader->vbooffset+pflexinfo->first_vertex)*sizeof(vbm_glvertex_t), m_tempVertexes, pflexinfo->num_vertexes*sizeof(vbm_glvertex_t));
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawFirst( void )
{
	Int32 skinnum = m_pCurrentEntity->curstate.skin; // for short..
	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();

	if(skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for(Uint32 i = 0; i < m_numDrawSubmodels; i++)
	{
		m_pVBMSubModel = m_pSubmodelDrawList[i];

		for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
		{
			const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
			const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

			en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
			if(!pmaterial)
				continue;

			if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE))
				continue;

			if(!DrawMesh(pmaterial, pmesh, false))
				return false;

			rns.counters.modelpolies += pmesh->num_indexes/3;
		}
	}

	if ( !m_useBlending && !m_isMultiPass )
	{
		for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[i];

			for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if(!(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT)))
					continue;

				glEnable(GL_BLEND);
				glDepthMask(GL_FALSE);

				if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_EYEGLINT))
				{
					if ( rns.fog.settings.active )
						m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.0, 0.0, 0.0);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha);
				}
				else if(pmaterial->flags & TX_FL_ALPHABLEND)
				{
					if ( rns.fog.settings.active )
						m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
					
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha*pmaterial->alpha);
				}

				if(!DrawMesh(pmaterial, pmesh, true))
				{
					glDepthMask(GL_TRUE);
					glDisable(GL_BLEND);
					return false;
				}

				rns.counters.modelpolies += pmesh->num_indexes/3;

				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
			}
		}

		// Reset this
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha);

		if ( rns.fog.settings.active )
			m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
	}

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawStyles( bool specularPass, bool transparentPass )
{
	// Only draw styles if we have a valid VBO index AND are in multipass mode
	if(!m_isMultiPass || m_pCurrentEntity->curstate.vlight_vbo_index == NO_POSITION)
		return true;

	// Check if we have any valid styles at all
	CArray<Float>* pstylesarray = gLightStyles.GetLightStyleValuesArray();
	Uint32 i = 1;
	for(; i < MAX_ENTITY_STYLES; i++)
	{
		if(m_pCurrentEntity->curstate.vlight_styles[i] != NULL_LIGHTSTYLE_INDEX 
			&& (*pstylesarray)[m_pCurrentEntity->curstate.vlight_styles[i]] > 0)
			break;
	}

	// If there's no styles, then don't bother
	if(i == MAX_ENTITY_STYLES)
		return true;

	// Fetch texture related ptrs
	Int32 skinnum = m_pCurrentEntity->curstate.skin; // for short..
	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();

	if(skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, specularPass);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);

	// If doing specular pass, these are already done by DrawFinalSpecular
	if(!specularPass)
	{
		if(transparentPass || m_useBlending)
			glDepthFunc(GL_LEQUAL);
		else
			glDepthFunc(GL_EQUAL);

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_ONE, GL_ONE);

		m_pShader->EnableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_diffuse);
	}

	if(!transparentPass && !m_useBlending)
	{
		// If not drawing blended parts, don't sync fog and chrome stuff
		m_pShader->DisableSync(m_attribs.u_vorigin);
		m_pShader->DisableSync(m_attribs.u_vright);
		m_pShader->DisableSync(m_attribs.u_fogcolor);
		m_pShader->DisableSync(m_attribs.u_fogparams);
	}
	else
	{
		// If doing blending, set chrome related stuff
		m_pShader->SetUniform3f(m_attribs.u_vorigin, rns.view.v_origin[0], rns.view.v_origin[1], rns.view.v_origin[2]);
		m_pShader->SetUniform3f(m_attribs.u_vright, rns.view.v_right[0], rns.view.v_right[1], rns.view.v_right[2]);

		m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));
	}

	// Switch over to the appropriate vertex light only shader
	Int32 shaderIndex = specularPass ? vbm_vlight_only_specular : vbm_vlight_only;
	if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, TRUE, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, shaderIndex))
		return false;

	for(i = 1; i < MAX_ENTITY_STYLES; i++)
	{
		if(m_pCurrentEntity->curstate.vlight_styles[i] == NULL_LIGHTSTYLE_INDEX 
			|| (*pstylesarray)[m_pCurrentEntity->curstate.vlight_styles[i]] <= 0)
			continue;

		// Set the attribute pointers to the appropriate layer
		const void* pvectorsoffset;
		const void* pambientoffset;
		const void* pdiffuseoffset;
		switch(i)
		{
		case 2:
			pvectorsoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight2_vector);
			pambientoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight2_ambient);
			pdiffuseoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight2_diffuse);
			break;
		case 3:
			pvectorsoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight3_vector);
			pambientoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight3_ambient);
			pdiffuseoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight3_diffuse);
			break;
		default:
		case 1:
			pvectorsoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight1_vector);
			pambientoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight1_ambient);
			pdiffuseoffset = OFFSET(vbm_vlight_glvertex_t, vertexlight1_diffuse);
			break;
		}

		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_vectors, pvectorsoffset, 1);
		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_ambient, pambientoffset, 1);
		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_diffuse, pdiffuseoffset, 1);

		// Color strength comes from animated style
		Float colorstrength = (*pstylesarray)[m_pCurrentEntity->curstate.vlight_styles[i]];

		for (Uint32 j = 0; j < m_numDrawSubmodels; j++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[j];

			for (Int32 k = 0; k < m_pVBMSubModel->nummeshes; k++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, k);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);
				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if(!m_useBlending)
				{
					if(!transparentPass && (pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND|TX_FL_FULLBRIGHT))
						|| transparentPass && !(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND|TX_FL_FULLBRIGHT)))
						continue;
				}

				if(pmaterial->flags & TX_FL_SCOPE)
					continue;

				// The m_firstTextureUnit marks the first available unit
				Int32 texunit_inner = m_firstTextureUnit;
				// Clear every sampler higher than this unit
				m_pShader->ResetSamplerIndex(texunit_inner);

				if(specularPass)
				{
					if(!pmaterial->ptextures[MT_TX_SPECULAR])
						continue;

					m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
					m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

					texunit_inner = m_pShader->AutoSetSamplerUniform(m_attribs.u_spectexture);
					R_Bind2DTexture(GL_TEXTURE0 + texunit_inner, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
				}

				if(pmaterial->ptextures[MT_TX_NORMALMAP] && g_pCvarBumpMaps->GetValue() > 0)
				{
					m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, TRUE);

					texunit_inner = m_pShader->AutoSetSamplerUniform(m_attribs.u_normalmap);
					R_Bind2DTexture(GL_TEXTURE0 + texunit_inner, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				}
				else
				{
					m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
				}

				if(transparentPass || m_useBlending)
				{
					m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, rns.fog.settings.active ? BLENDMULTIPASS_BLACKFOG : BLENDMULTIPASS_NORMAL);
					m_pShader->SetUniform1i(m_attribs.u_d_chrome, (pmaterial->flags & (TX_FL_CHROME) || pmaterial->flags & (TX_FL_EYEGLINT)) ? TRUE : FALSE);

					texunit_inner = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);
					R_Bind2DTexture(GL_TEXTURE0 + texunit_inner, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

					// Set transparency
					Float alpha = m_renderAlpha;
					if(pmaterial->flags & (TX_FL_ALPHABLEND))
						alpha *= pmaterial->alpha;

					alpha *= colorstrength;
					m_pShader->SetUniform4f(m_attribs.u_color, alpha, alpha, alpha, 1.0);
				}
				else
				{
					m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, BLENDMULTIPASS_OFF);
					m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
					m_pShader->SetUniform4f(m_attribs.u_color, colorstrength, colorstrength, colorstrength, 1.0);
				}

				// Fix overlapping sampler issue
				if(!m_pShader->PerformPreRenderChecks())
					return false;

				R_ValidateShader(m_pShader);

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

				// Remove all current texture binds
				R_ClearBinds(m_firstTextureUnit);
			}
		}
	}

	// If not doing specular pass, disable the attribs pointing to the secondary VBO
	if(!specularPass)
	{
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_diffuse);

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_BLEND);
	}

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

	if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, FALSE, false))
		return false;
	else
		return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawMesh( en_material_t *pmaterial, const vbmmesh_t *pmesh, bool drawBlended )
{
	// Set the determinator states
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, ((!m_isMultiPass || m_useBlending) && (pmaterial->flags & (TX_FL_CHROME) || pmaterial->flags & (TX_FL_EYEGLINT) && drawBlended)) ? TRUE : FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, (pmaterial->flags & (TX_FL_FULLBRIGHT|TX_FL_SCOPE)) ? 0 : m_numModelLights);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, (pmaterial->ptextures[MT_TX_SPECULAR]) && !(pmaterial->flags & TX_FL_FULLBRIGHT) && (!m_isMultiPass || m_useBlending) && g_pCvarSpecular->GetValue() > 0 ? true : false);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, (pmaterial->ptextures[MT_TX_LUMINANCE]) && !(pmaterial->flags & TX_FL_FULLBRIGHT));
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, (pmaterial->ptextures[MT_TX_NORMALMAP]) && !(pmaterial->flags & TX_FL_FULLBRIGHT) && g_pCvarBumpMaps->GetValue() > 0);

	// Alpha testing needs to be handled specially
	Int32 alphatestMode = ALPHATEST_DISABLED;
	if((!m_useBlending && (pmaterial->flags & TX_FL_ALPHATEST) && !(pmaterial->flags & (TX_FL_SCOPE|TX_FL_CHROME|TX_FL_EYEGLINT))))
	{
		alphatestMode = (rns.msaa && rns.mainframe) ? ALPHATEST_COVERAGE : ALPHATEST_LESSTHAN;
		if(alphatestMode == ALPHATEST_COVERAGE)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			gGLExtF.glSampleCoverage(0.5, GL_FALSE);
		}
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, alphatestMode, false))
		return false;

	bool result = false;
	if(pmaterial->flags & TX_FL_SCOPE)
	{
		// Apply scope effect
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_scope);
	}
	else if(pmaterial->flags & TX_FL_FULLBRIGHT)
	{
		// Fullbright with default or fullbright color
		if(!pmaterial->fullbrightcolor.IsZero())
			m_pShader->SetUniform4f(m_attribs.u_color, pmaterial->fullbrightcolor.x, pmaterial->fullbrightcolor.y, pmaterial->fullbrightcolor.z, m_renderAlpha);

		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, rns.fog.settings.active ? vbm_texonly_fog : vbm_texonly);
	}
	else if(m_isMultiPass && !m_useBlending) 
	{
		// Multipass without textures
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_notexture);
	}
	else
	{
		// Normal textured single-pass rendering
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, rns.fog.settings.active ? vbm_texture_fog : vbm_texture);
	}
		
	// Verify the settings
	if(!result)
		return false;

	// Reset sampler to the first texture unit available
	m_pShader->ResetSamplerIndex(m_firstTextureUnit);
	Int32 textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);

	if(drawBlended && pmaterial->flags & TX_FL_EYEGLINT)
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_pGlintTexture->palloc->gl_index);
	else if(!m_isMultiPass || m_useBlending || pmaterial->flags & (TX_FL_ALPHATEST|TX_FL_FULLBRIGHT|TX_FL_SCOPE))
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

	if(pmaterial->flags & TX_FL_SCOPE)
	{
		Float flscale = pmaterial->scale / g_pCvarReferenceFOV->GetValue();
		m_pShader->SetUniform1f(m_attribs.u_scope_scale, flscale);
		m_pShader->SetUniform2f(m_attribs.u_scope_scrsize, rns.screenwidth, rns.screenheight);

		textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_rectangle);
		R_BindRectangleTexture(GL_TEXTURE0_ARB + textureIndex, m_pScreenTexture->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_SPECULAR] && g_pCvarSpecular->GetValue() > 0)
	{
		m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
		m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

		textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_spectexture);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_LUMINANCE])
	{
		textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_lumtexture);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_LUMINANCE]->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_NORMALMAP] && g_pCvarBumpMaps->GetValue() > 0)
	{
		textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_normalmap);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
	}
	
	if(pmaterial->scrollu || pmaterial->scrollv)
	{
		Float scrollu = pmaterial->scrollu ? (rns.time * pmaterial->scrollu) : 0;
		Float scrollv = pmaterial->scrollv ? (rns.time * pmaterial->scrollv) : 0;

		m_pShader->SetUniform2f(m_attribs.u_scroll, scrollu, scrollv);
	}
	else
	{
		// No scrolling
		m_pShader->SetUniform2f(m_attribs.u_scroll, 0, 0);
	}

	// Fix overlapping sampler issue
	if(!m_pShader->PerformPreRenderChecks())
		return false;

	if(pmesh->numbones)
		SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

	if(pmaterial->flags & TX_FL_NO_CULLING)
		glDisable(GL_CULL_FACE);

	R_ValidateShader(m_pShader);

	m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

	if(pmaterial->flags & TX_FL_NO_CULLING)
		glEnable(GL_CULL_FACE);

	// Restore default color if fullbright had a custom color
	if((pmaterial->flags & TX_FL_FULLBRIGHT) && !pmaterial->fullbrightcolor.IsZero())
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha);

	if(alphatestMode == ALPHATEST_COVERAGE)
	{
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		gGLExtF.glSampleCoverage(1.0, GL_FALSE);
	}

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawLights( bool specularPass, bool transparentPass )
{
	if(!transparentPass && !m_isMultiPass)
		return true;

	if(!m_numDynamicLights)
		return true;

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_vertexlight, FALSE, false))
		return false;

	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, specularPass);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	if(!specularPass)
	{
		if(transparentPass || m_useBlending)
			glDepthFunc(GL_LEQUAL);
		else
			glDepthFunc(GL_EQUAL);

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	// Set this determinator
	if(!m_pShader->SetDeterminator(m_attribs.d_flexes, m_useFlexes, false))
		return false;

	// Linked list of dynamic light batches
	CLinkedList<lightbatch_t> lightBatches;
	for(Uint32 i = 0; i < m_numDynamicLights; i++)
	{
		cl_dlight_t *pdlight = m_pDynamicLights[i];

		// Pointer to batch we'll use
		lightbatch_t* pbatch = nullptr;

		// Determine batch type
		lightbatchtype_t type;
		if(pdlight->cone_size)
		{
			if(pdlight->noShadow())
				type = LB_TYPE_SPOTLIGHT;
			else
				type = LB_TYPE_SPOTLIGHT_SHADOW;
		}
		else
		{
			if(pdlight->noShadow())
				type = LB_TYPE_POINTLIGHT;
			else
				type = LB_TYPE_POINTLIGHT_SHADOW;
		}

		if(g_pCvarBatchDynamicLights->GetValue() >= 1)
		{
			// See if we have a fitting batch
			lightBatches.begin();
			while(!lightBatches.end())
			{
				lightbatch_t& curbatch = lightBatches.get();
				if(curbatch.type == type && curbatch.lightslist.size() < MAX_BATCH_LIGHTS)
				{
					pbatch = &curbatch;
					break;
				}

				lightBatches.next();
			}
		}

		if(!pbatch)
		{
			lightbatch_t newbatch;
			newbatch.type = type;

			pbatch = &lightBatches.add(newbatch)->_val;
		}

		pbatch->lightslist.add(pdlight);
	}

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);

	if(!transparentPass && !m_useBlending)
	{
		m_pShader->DisableSync(m_attribs.u_vorigin);
		m_pShader->DisableSync(m_attribs.u_vright);
		m_pShader->DisableSync(m_attribs.u_fogcolor);
		m_pShader->DisableSync(m_attribs.u_fogparams);
	}
	else
	{
		m_pShader->SetUniform3f(m_attribs.u_vorigin, rns.view.v_origin[0], rns.view.v_origin[1], rns.view.v_origin[2]);
		m_pShader->SetUniform3f(m_attribs.u_vright, rns.view.v_right[0], rns.view.v_right[1], rns.view.v_right[2]);

		m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));
	}

	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_light_radius);

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->EnableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_color);

	m_pShader->EnableAttribute(m_attribs.a_texcoord1);

	Int32 skinnum = m_pCurrentEntity->curstate.skin; // for short..
	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();

	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	CMatrix pmatrix;
	const Float *fltranspose = rns.view.modelview.Transpose();

	lightBatches.begin();
	while(!lightBatches.end())
	{
		lightbatch_t& batch = lightBatches.get();

		if(batch.type == LB_TYPE_POINTLIGHT || batch.type == LB_TYPE_POINTLIGHT_SHADOW)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_dynlight))
				return false;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_spotlight))
				return false;
		}

		// Latest light index
		Uint32 lightindex = 0;

		// The m_firstTextureUnit marks the first available unit
		Int32 texunit = m_firstTextureUnit;
		// Reset everything after this sampler
		m_pShader->ResetSamplerIndex(texunit);

		batch.lightslist.begin();
		while(!batch.lightslist.end())
		{
			cl_dlight_t* pdlight = batch.lightslist.get();

			m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_color);
			m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_origin);
			m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_radius);

			if(pdlight->cone_size)
			{
				if(DL_CanShadow(pdlight))
				{
					m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_d_light_shadowmap, TRUE);
					m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_shadowmap);

					texunit = m_pShader->AutoSetSamplerUniform(m_attribs.dlights[lightindex].u_light_shadowmap);
					R_Bind2DTexture(GL_TEXTURE0 + texunit, pdlight->getProjShadowMap()->pfbo->ptexture1->gl_index);
				}
				else
				{
					m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_shadowmap);
					m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_d_light_shadowmap, FALSE);
				}

				Int32 textureIndex = pdlight->textureindex;
				if(textureIndex >= rns.objects.projective_textures.size())
					textureIndex = 0;

				m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_projtexture);
				texunit = m_pShader->AutoSetSamplerUniform(m_attribs.dlights[lightindex].u_light_projtexture);
				R_Bind2DTexture(GL_TEXTURE0 + texunit, rns.objects.projective_textures[textureIndex]->palloc->gl_index);

				Vector angles = pdlight->angles;
				Common::FixVector(angles);

				Vector vforward, vtarget;
				Math::AngleVectors(angles, &vforward, nullptr, nullptr);
				Math::VectorMA(pdlight->origin, pdlight->radius, vforward, vtarget);

				pmatrix.LoadIdentity();
				pmatrix.Translate(0.5, 0.5, 0.5);
				pmatrix.Scale(0.5, 0.5, 0.5);

				Float flSize = tan((M_PI/360) * pdlight->cone_size);
				pmatrix.SetFrustum(-flSize, flSize, -flSize, flSize, 1, pdlight->radius);
				pmatrix.LookAt(pdlight->origin[0], pdlight->origin[1], pdlight->origin[2], vtarget[0], vtarget[1], vtarget[2], 0, 0, Common::IsPitchReversed(angles[PITCH]) ? -1 : 1);

				pmatrix.MultMatrix(rns.view.modelview.GetInverse());

				m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_matrix);
				m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_cone_size);
				m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_spotdirection);

				m_pShader->SetUniformMatrix4fv(m_attribs.dlights[lightindex].u_light_matrix, pmatrix.Transpose());
				m_pShader->SetUniform1f(m_attribs.dlights[lightindex].u_light_cone_size, pdlight->cone_size);

				// Transform light direction vector to eye space
				Vector transdirection;
				Math::MatMult(fltranspose, vforward, &transdirection);
				m_pShader->SetUniform3f(m_attribs.dlights[lightindex].u_light_spotdirection, transdirection[0], transdirection[1], transdirection[2]);
			}
			else
			{
				m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_projtexture);
				m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_shadowmap);

				if(DL_CanShadow(pdlight))
				{
					m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_d_light_shadowmap, TRUE);
					m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_cubemap);

					texunit = m_pShader->AutoSetSamplerUniform(m_attribs.dlights[lightindex].u_light_cubemap);
					R_BindCubemapTexture(GL_TEXTURE0 + texunit, pdlight->getCubeShadowMap()->pfbo->ptexture1->gl_index);

					glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

					// Set up world-space matrix
					pmatrix.LoadIdentity();
					pmatrix.Rotate(-90,  1, 0, 0);// put X going down
					pmatrix.Rotate(90,  0, 0, 1); // put Z going up
					pmatrix.Translate(-pdlight->origin[0], -pdlight->origin[1], -pdlight->origin[2]);

					// set up light matrix
					m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_matrix);
					m_pShader->SetUniformMatrix4fv(m_attribs.dlights[lightindex].u_light_matrix, pmatrix.GetMatrix(), true);
				}
				else
				{
					m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_cubemap);
					m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_d_light_shadowmap, FALSE);
				}
			}

			Vector vtransorigin;
			Math::MatMultPosition(fltranspose, pdlight->origin, &vtransorigin);

			Vector color;
			Math::VectorCopy(pdlight->color, color);
			gLightStyles.ApplyLightStyle(pdlight, color);

			m_pShader->SetUniform3f(m_attribs.dlights[lightindex].u_light_origin, vtransorigin[0], vtransorigin[1], vtransorigin[2]);
			m_pShader->SetUniform4f(m_attribs.dlights[lightindex].u_light_color, color.x, color.y, color.z, 1.0);
			m_pShader->SetUniform1f(m_attribs.dlights[lightindex].u_light_radius, pdlight->radius);

			batch.lightslist.next();
			lightindex++;
		}

		m_pShader->SetUniform1i(m_attribs.u_d_numdlights, lightindex);

		for (Uint32 j = 0; j < m_numDrawSubmodels; j++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[j];

			for (Int32 k = 0; k < m_pVBMSubModel->nummeshes; k++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, k);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);
				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if(!m_useBlending)
				{
					if(!transparentPass && (pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND|TX_FL_FULLBRIGHT))
						|| transparentPass && !(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND|TX_FL_FULLBRIGHT)))
						continue;
				}

				if(pmaterial->flags & TX_FL_SCOPE)
					continue;

				// Local texture unit tracking for each mesh
				// As texunit marks the last used unit, we need to add +1
				Int32 texunit_inner = texunit + 1;
				// Clear every sampler higher than this unit
				m_pShader->ResetSamplerIndex(texunit_inner);

				if(specularPass)
				{
					if(!pmaterial->ptextures[MT_TX_SPECULAR])
						continue;

					m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
					m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

					texunit_inner = m_pShader->AutoSetSamplerUniform(m_attribs.u_spectexture);
					R_Bind2DTexture(GL_TEXTURE0 + texunit_inner, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
				}

				if(pmaterial->ptextures[MT_TX_NORMALMAP] && g_pCvarBumpMaps->GetValue() > 0)
				{
					m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, TRUE);

					texunit_inner = m_pShader->AutoSetSamplerUniform(m_attribs.u_normalmap);
					R_Bind2DTexture(GL_TEXTURE0 + texunit_inner, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				}
				else
				{
					m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
				}

				if(transparentPass || m_useBlending)
				{
					m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, rns.fog.settings.active ? BLENDMULTIPASS_BLACKFOG : BLENDMULTIPASS_NORMAL);
					m_pShader->SetUniform1i(m_attribs.u_d_chrome, (pmaterial->flags & (TX_FL_CHROME) || pmaterial->flags & (TX_FL_EYEGLINT)) ? TRUE : FALSE);

					texunit_inner = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);
					R_Bind2DTexture(GL_TEXTURE0 + texunit_inner, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

					// Set transparency
					Float alpha = m_renderAlpha;
					if(pmaterial->flags & (TX_FL_ALPHABLEND))
						alpha *= pmaterial->alpha;

					m_pShader->SetUniform4f(m_attribs.u_color, alpha, alpha, alpha, 1.0);
				}
				else
				{
					m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, BLENDMULTIPASS_OFF);
					m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
				}

				// Fix overlapping sampler issue
				if(!m_pShader->PerformPreRenderChecks())
					return false;

				R_ValidateShader(m_pShader);

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

				// Remove all current texture binds
				R_ClearBinds(texunit + 1);
			}
		}

		// Reset texunits
		R_ClearBinds(texunit);

		// Reset everything
		for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
		{
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
			m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
		
			m_pShader->SetUniform1i(m_attribs.dlights[i].u_d_light_shadowmap, FALSE);
		}

		lightBatches.next();
	}

	m_pShader->SetUniform1i(m_attribs.u_d_numdlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, BLENDMULTIPASS_OFF);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawFinal ( void )
{
	if(!m_isMultiPass)
		return true;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);
	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_light_radius);

	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
	}

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->DisableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_color);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);

	// m_firstTextureUnit marks the first free unit
	Int32 firstexunit = m_firstTextureUnit;
	// Clear everything after this unit
	m_pShader->ResetSamplerIndex(firstexunit);

	// Set this determinator
	if(!m_pShader->SetDeterminator(m_attribs.d_flexes, m_useFlexes, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_vertexlight, FALSE, false))
		return false;

	bool hasSpecular = false;
	Int32 textureFlags = 0;

	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();
	Int32 skinnum = m_pCurrentEntity->curstate.skin;
	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
		return false;

	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);

	if(!m_useBlending)
	{
		glEnable(GL_BLEND);
		glDepthFunc(GL_EQUAL);
		glDepthMask(GL_FALSE);

		m_pShader->DisableAttribute(m_attribs.a_normal);

		// Draw caustics
		if(rns.inwater && g_pCvarCaustics->GetValue() >= 1)
		{
			const water_settings_t *psettings = gWaterShader.GetActiveSettings();
			if(!psettings->cheaprefraction && psettings->causticscale > 0 && psettings->causticstrength > 0)
			{
				GLfloat splane[4] = {0.005f*psettings->causticscale, 0.0025f*psettings->causticscale, 0.0, 0.0};
				GLfloat tplane[4] = {0.0, 0.005f*psettings->causticscale, 0.0025f*psettings->causticscale, 0.0};

				if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_caustics))
					return false;

				m_pShader->SetUniform4fv(m_attribs.u_causticsm1, splane, 1);
				m_pShader->SetUniform4fv(m_attribs.u_causticsm2, tplane, 1);
				m_pShader->SetUniform4f(m_attribs.u_color, 
					psettings->fogparams.color[0]*psettings->causticstrength, 
					psettings->fogparams.color[1]*psettings->causticstrength, 
					psettings->fogparams.color[2]*psettings->causticstrength, 
					1.0);

				Float causticsTime = rns.time*10*psettings->causticstimescale;
				Int32 causticsCurFrame = static_cast<Int32>(causticsTime) % rns.objects.caustics_textures.size();
				Int32 causticsNextFrame = (causticsCurFrame+1) % rns.objects.caustics_textures.size();
				Float causticsInterp = (causticsTime)-static_cast<Int32>(causticsTime);

				// No need for complex tracking of units here, as these are only set once
				Int32 firstexunit_local = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);
				R_Bind2DTexture(GL_TEXTURE0 + firstexunit_local, rns.objects.caustics_textures[causticsCurFrame]->palloc->gl_index);

				// Bind other texture too
				firstexunit_local = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture1);
				R_Bind2DTexture(GL_TEXTURE0 + firstexunit_local, rns.objects.caustics_textures[causticsNextFrame]->palloc->gl_index);

				m_pShader->SetUniform1f(m_attribs.u_caustics_interp, causticsInterp);

				glBlendFunc(GL_DST_COLOR, GL_ONE);

				for (Uint32 j = 0; j < m_numDrawSubmodels; j++)
				{
					m_pVBMSubModel = m_pSubmodelDrawList[j];

					for (Int32 k = 0; k < m_pVBMSubModel->nummeshes; k++) 
					{
						const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, k);
						const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

						en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
						if(!pmaterial)
							continue;

						if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_FULLBRIGHT|TX_FL_SCOPE))
							continue;

						// Fix overlapping sampler issue
						if(!m_pShader->PerformPreRenderChecks())
							return false;

						if(pmesh->numbones)
							SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

						R_ValidateShader(m_pShader);

						m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
					}
				}

				m_pShader->SetUniform4f(m_attribs.u_color,  1.0, 1.0, 1.0, 1.0);
			}
		}

		// Render meshes with textures
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

		m_pShader->EnableAttribute(m_attribs.a_texcoord1);
		if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texonly))
			return false;

		// Ensure every sampler gets reset, except for flex
		m_pShader->ResetSamplerIndex(m_firstTextureUnit);
		// Set the texturing unit only once
		Int32 firstexunit_local = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);

		for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[i];

			for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);
			
				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				// Skip specific texflags
				if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_FULLBRIGHT|TX_FL_SCOPE|TX_FL_CHROME))
				{
					textureFlags |= pmaterial->flags;
					continue;
				}

				// Remember glint and specular for later
				if(pmaterial->flags & TX_FL_EYEGLINT)
					textureFlags |= pmaterial->flags;

				if(pmaterial->ptextures[MT_TX_SPECULAR])
					hasSpecular = true;

				if(pmaterial->scrollu || pmaterial->scrollv)
				{
					Float scrollu = pmaterial->scrollu ? (rns.time * pmaterial->scrollu) : 0;
					Float scrollv = pmaterial->scrollv ? (rns.time * pmaterial->scrollv) : 0;

					m_pShader->SetUniform2f(m_attribs.u_scroll, scrollu, scrollv);
				}
				else
				{
					// No scrolling
					m_pShader->SetUniform2f(m_attribs.u_scroll, 0, 0);
				}

				// Fix overlapping sampler issue
				if(!m_pShader->PerformPreRenderChecks())
					return false;

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				R_Bind2DTexture(GL_TEXTURE0 + firstexunit_local, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
				R_ValidateShader(m_pShader);

				m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
			}

			if(textureFlags & TX_FL_CHROME)
			{
				m_pShader->SetUniform1i(m_attribs.u_d_chrome, TRUE);
				m_pShader->EnableAttribute(m_attribs.a_normal);

				m_pShader->SetUniform3f(m_attribs.u_vorigin, rns.view.v_origin[0], rns.view.v_origin[1], rns.view.v_origin[2]);
				m_pShader->SetUniform3f(m_attribs.u_vright, rns.view.v_right[0], rns.view.v_right[1], rns.view.v_right[2]);

				for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
				{
					const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
					const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

					en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
					if(!pmaterial)
						continue;

					if(!(pmaterial->flags & TX_FL_CHROME))
						continue;

					// Fix overlapping sampler issue
					if(!m_pShader->PerformPreRenderChecks())
						return false;

					if(pmesh->numbones)
						SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

					R_Bind2DTexture(GL_TEXTURE0 + firstexunit_local, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
					R_ValidateShader(m_pShader);

					m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
				}

				m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
				m_pShader->DisableAttribute(m_attribs.a_normal);

				// Remove it from the bit flags
				textureFlags &= ~TX_FL_CHROME;
			}
		}

		// Ensure these get reset
		m_pShader->ResetSamplerIndex(m_firstTextureUnit);
	}
	else
	{
		// We still need some info about the submodels
		for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[i];

			for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);
			
				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				// Skip specific texflags
				if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_FULLBRIGHT|TX_FL_SCOPE|TX_FL_CHROME))
				{
					textureFlags |= pmaterial->flags;
					continue;
				}

				// Remember glint and specular for later
				if(pmaterial->flags & TX_FL_EYEGLINT)
					textureFlags |= pmaterial->flags;

				if(pmaterial->ptextures[MT_TX_SPECULAR])
					hasSpecular = true;
			}
		}
	}

	//
	// Render meshes with specular highlights
	//
	if(hasSpecular && g_pCvarSpecular->GetValue() > 0)
	{
		if(!DrawFinalSpecular(false))
			return false;
	}

	// No further stuff is needed in this case
	if(m_useBlending)
		return true;

	//
	// Render meshes with fog
	//
	if(rns.fog.settings.active)
	{
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));	
		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_fogpass))
			return false;

		for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[i];

			for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_FULLBRIGHT|TX_FL_SCOPE))
					continue;

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				R_ValidateShader(m_pShader);

				m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
			}
		}
	}

	// Reset this
	glDepthFunc(GL_LEQUAL);

	//
	// If we had any transparent parts, then render those last
	//
	if((textureFlags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT)))
	{
		m_pShader->EnableSync(m_attribs.u_vorigin);
		m_pShader->EnableSync(m_attribs.u_vright);
		m_pShader->EnableSync(m_attribs.u_sky_ambient);
		m_pShader->EnableSync(m_attribs.u_sky_diffuse);
		m_pShader->EnableSync(m_attribs.u_sky_dir);

		m_pShader->EnableSync(m_attribs.u_normalmatrix);
		m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());

		// Render any additive parts
		bool result = false;
		if(!rns.fog.settings.active) 
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture, false);
		else
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture_fog, false);

		if(!result)
			return false;

		m_pShader->SetUniform1i(m_attribs.u_d_numlights, m_numModelLights);

		m_pShader->EnableAttribute(m_attribs.a_texcoord1);
		m_pShader->EnableAttribute(m_attribs.a_normal);

		// Set all the uniforms again
		Vector vtransformed;
		CMatrix pmatrix(rns.view.modelview.GetInverse());
		Math::MatMult(pmatrix.Transpose(), m_renderLightVector, &vtransformed);

		m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
		m_pShader->SetUniform3f(m_attribs.u_sky_ambient, m_renderAmbientColor[0], m_renderAmbientColor[1], m_renderAmbientColor[2]);
		m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, m_renderDiffuseColor[0], m_renderDiffuseColor[1], m_renderDiffuseColor[2]);

		if(textureFlags & (TX_FL_EYEGLINT|TX_FL_CHROME))
		{
			m_pShader->SetUniform3f(m_attribs.u_vorigin, rns.view.v_origin[0], rns.view.v_origin[1], rns.view.v_origin[2]);
			m_pShader->SetUniform3f(m_attribs.u_vright, rns.view.v_right[0], rns.view.v_right[1], rns.view.v_right[2]);
		}

		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

		// Disable so chrome will be applied
		m_isMultiPass = false;

		for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[i];

			for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
			{
				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if (!(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT)))
					continue;

				if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_EYEGLINT))
				{
					if (rns.fog.settings.active)
						m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.0, 0.0, 0.0);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
				}
				else if(pmaterial->flags & TX_FL_ALPHABLEND)
				{
					if (rns.fog.settings.active)
						m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);

					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, pmaterial->alpha);
				}

				if(!DrawMesh(pmaterial, pmesh, true))
					return false;
			}
		}

		m_pShader->DisableAttribute(m_attribs.a_texcoord1);
		m_pShader->DisableAttribute(m_attribs.a_normal);

		// Draw any styles for transparents
		if(!DrawStyles(false, true))
			return false;

		// Draw any dynamic lighting for transparents
		if(!DrawLights(false, true))
			return false;
		
		// Draw specular highlights at the very end
		if(hasSpecular && g_pCvarSpecular->GetValue() > 0)
		{
			if(!DrawFinalSpecular(true))
				return false;
		}

		// For wireframe
		m_isMultiPass = true;

		if(rns.fog.settings.active)
			m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawFinalSpecular( bool transparentPass )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();
	Int32 skinnum = m_pCurrentEntity->curstate.skin;
	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	if(transparentPass || m_useBlending)
	{
		glDepthFunc(GL_LEQUAL);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);
	}
	else
		glDepthFunc(GL_EQUAL);

	glBlendFunc(GL_ONE, GL_ONE);

	m_pShader->SetUniform1i(m_attribs.u_d_numlights, m_numModelLights);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, TRUE);

	if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_speconly, false))
		return false;

	m_pShader->EnableSync(m_attribs.u_sky_ambient);
	m_pShader->EnableSync(m_attribs.u_sky_diffuse);
	m_pShader->EnableSync(m_attribs.u_sky_dir);
	m_pShader->EnableSync(m_attribs.u_spectexture);
	m_pShader->EnableSync(m_attribs.u_normalmap);

	m_pShader->EnableAttribute(m_attribs.a_texcoord1);
	m_pShader->EnableAttribute(m_attribs.a_normal);

	m_pShader->EnableSync(m_attribs.u_normalmatrix);
	m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());

	// Set all the uniforms again
	Vector vtransformed;
	CMatrix pmatrix(rns.view.modelview.GetInverse());
	Math::MatMult(pmatrix.Transpose(), m_renderLightVector, &vtransformed);

	m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_ambient, m_renderAmbientColor[0], m_renderAmbientColor[1], m_renderAmbientColor[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, m_renderDiffuseColor[0], m_renderDiffuseColor[1], m_renderDiffuseColor[2]);

	// Bind VBO for baked vertex lighting if any
	if(m_pCurrentEntity->curstate.vlight_vbo_index != NO_POSITION
		&& m_pVertexLightingVBOArray.size() > m_pCurrentEntity->curstate.vlight_vbo_index)
	{
		vlight_vbo_t* pvblightvbo = m_pVertexLightingVBOArray[m_pCurrentEntity->curstate.vlight_vbo_index];
		m_pShader->SetVBO(pvblightvbo->pvbo, 1);

		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_vectors, OFFSET(vbm_vlight_glvertex_t, vertexlight0_vector), 1);
		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_ambient, OFFSET(vbm_vlight_glvertex_t, vertexlight0_ambient), 1);
		m_pShader->SetAttributePointer(m_attribs.a_vertexlight_diffuse, OFFSET(vbm_vlight_glvertex_t, vertexlight0_diffuse), 1);

		m_pShader->EnableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->EnableAttribute(m_attribs.a_vertexlight_diffuse);

		if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, TRUE))
			return false;
	}
	else
	{
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_diffuse);

		if(!m_pShader->SetDeterminator(m_attribs.d_vertexlight, FALSE))
			return false;
	}

	for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
	{
		m_pVBMSubModel = m_pSubmodelDrawList[i];

		for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
		{
			const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
			const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

			en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
			if(!pmaterial)
				continue;

			if(!m_useBlending)
			{
				if (transparentPass && !(pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT))
					|| !transparentPass && (pmaterial->flags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT)))
					continue;
			}

			if(pmaterial->flags & (TX_FL_SCOPE|TX_FL_FULLBRIGHT))
				continue;

			if(!pmaterial->ptextures[MT_TX_SPECULAR])
				continue;

			m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
			m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

			// m_firstTextureUnit marks the first available unit
			Int32 texunit_local = m_firstTextureUnit;
			// Reset every sampler after firstexunit
			m_pShader->ResetSamplerIndex(texunit_local);

			texunit_local = m_pShader->AutoSetSamplerUniform(m_attribs.u_spectexture);
			R_Bind2DTexture(GL_TEXTURE0 + texunit_local, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
			
			// Set normal map if any
			if (pmaterial->ptextures[MT_TX_NORMALMAP])
			{
				m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, TRUE);

				texunit_local = m_pShader->AutoSetSamplerUniform(m_attribs.u_normalmap);
				R_Bind2DTexture(GL_TEXTURE0 + texunit_local, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
			}
			else
			{
				m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
			}

			if(transparentPass || m_useBlending)
			{
				m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, rns.fog.settings.active ? BLENDMULTIPASS_BLACKFOG : BLENDMULTIPASS_NORMAL);
				m_pShader->SetUniform1i(m_attribs.u_d_chrome, (pmaterial->flags & (TX_FL_CHROME) || pmaterial->flags & (TX_FL_EYEGLINT)) ? TRUE : FALSE);

				texunit_local = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);
				R_Bind2DTexture(GL_TEXTURE0 + texunit_local, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

				// Set transparency
				Float alpha = m_renderAlpha;
				if(pmaterial->flags & (TX_FL_ALPHABLEND))
					alpha *= pmaterial->alpha;

				m_pShader->SetUniform4f(m_attribs.u_color, alpha, alpha, alpha, 1.0);
			}
			else
			{
				m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, BLENDMULTIPASS_OFF);
				m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
				m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
			}

			if(pmaterial->scrollu || pmaterial->scrollv)
			{
				Float scrollu = pmaterial->scrollu ? (rns.time * pmaterial->scrollu) : 0;
				Float scrollv = pmaterial->scrollv ? (rns.time * pmaterial->scrollv) : 0;

				m_pShader->SetUniform2f(m_attribs.u_scroll, scrollu, scrollv);
			}
			else
			{
				// No scrolling
				m_pShader->SetUniform2f(m_attribs.u_scroll, 0, 0);
			}

			// Fix overlapping sampler issue
			if(!m_pShader->PerformPreRenderChecks())
				return false;

			if(pmesh->numbones)
				SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

			R_ValidateShader(m_pShader);

			m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

			// Remove binds
			R_ClearBinds(m_firstTextureUnit);
		}
	}

	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);

	// Draw any styles using specular
	if(!DrawStyles(true, transparentPass))
		return false;

	// Disable normalmatrix sync only after doing styles
	m_pShader->DisableSync(m_attribs.u_normalmatrix);

	// Disable these attribs
	if(m_pCurrentEntity->curstate.vlight_vbo_index != NO_POSITION
		&& m_pVertexLightingVBOArray.size() > m_pCurrentEntity->curstate.vlight_vbo_index)
	{
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_vectors);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_ambient);
		m_pShader->DisableAttribute(m_attribs.a_vertexlight_diffuse);
	}

	// Draw dynamic light specular lighting
	if(m_numDynamicLights)
	{
		if(!DrawLights(true, transparentPass))
			return false;

		m_pShader->EnableSync(m_attribs.u_fogcolor);
		m_pShader->EnableSync(m_attribs.u_fogparams);
	}

	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_blendmultipass, BLENDMULTIPASS_OFF);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);

	m_pShader->DisableAttribute(m_attribs.a_texcoord1);
	m_pShader->DisableAttribute(m_attribs.a_normal);

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawWireframe( void )
{
	if(g_pCvarWireFrame->GetValue() < 1)
		return true;

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);
	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_light_radius);
	m_pShader->DisableSync(m_attribs.u_texture0);

	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
	}

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->DisableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_color);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);

	// Reset any samplers altogether
	m_pShader->ResetSamplerIndex();

	if(m_pVBMHeader->flags & VBM_HAS_FLEXES && m_isVertexFetchSupported)
	{
		m_pShader->EnableAttribute(m_attribs.a_flexcoord);

		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, TRUE, false))
			return false;

		// Use AutoSetSamplerUniform always, even if we only use one unit
		Int32 textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_flextexture);
		R_Bind2DTexture(GL_TEXTURE0_ARB + textureIndex, m_pFlexTexture->gl_index);
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false))
			return false;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
		return false;

	m_pShader->SetUniform1i(m_attribs.u_d_chrome, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, 0);

	if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid))
		return false;

	if(m_isMultiPass)
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 0.0, 0.0, 1.0);
	else
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1);

	if(g_pCvarWireFrame->GetValue() >= 2)
		glDisable(GL_DEPTH_TEST);

	m_pShader->DisableAttribute(m_attribs.a_normal);
	m_pShader->DisableAttribute(m_attribs.a_tangent);
	m_pShader->DisableAttribute(m_attribs.a_texcoord1);

	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_DISTANCE);

		for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
		{
			const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
		
			if(pmesh->numbones)
				SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

			R_ValidateShader(m_pShader);

			m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(m_pVBMHeader->flags & VBM_HAS_FLEXES && m_isVertexFetchSupported)
		m_pShader->DisableAttribute(m_attribs.a_flexcoord);

	if(g_pCvarWireFrame->GetValue() >= 2)
		glEnable(GL_DEPTH_TEST);

	if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false))
		return false;
	else
		return true;
}

//=============================================
//
//
//=============================================
vbmdecal_t* CVBMRenderer::AllocDecalSlot( void )
{
	if(m_numVBMDecals == MAX_VBM_TOTAL_DECALS)
		m_numVBMDecals = 0;

	vbmdecal_t *pDecal = &m_vbmDecals[m_numVBMDecals];
	m_numVBMDecals++;

	// Unlink this
	if(pDecal->next)
	{
		pDecal->next->prev = pDecal->prev;
	}

	if(pDecal->prev)
	{
		// relink on this end
		pDecal->prev->next = pDecal->next;
		vbmdecal_t *proot = pDecal->prev;
		while(1)
		{
			if(!proot->prev)
				break;

			proot = proot->prev;
		}

		// take down one
		proot->totaldecals--;
	}
	else if(pDecal->pentity && pDecal->pentity->pextradata && pDecal->pentity->pextradata->pvbmdecalheader == pDecal)
	{
		if(pDecal->next)
		{
			pDecal->pentity->pextradata->pvbmdecalheader = pDecal->next;
			pDecal->next->totaldecals = pDecal->totaldecals-1;
		}
		else
		{
			pDecal->pentity->pextradata->pvbmdecalheader = nullptr;
		}
	}

	// Clear decal
	(*pDecal) = vbmdecal_t();

	return pDecal;
};

//=============================================
//
//
//=============================================
vbmdecal_t* CVBMRenderer::AllocDecal( void )
{
	if(!m_pExtraInfo->pvbmdecalheader)
	{
		vbmdecal_t *pDecal = AllocDecalSlot();
		pDecal->totaldecals = 1;

		m_pExtraInfo->pvbmdecalheader = pDecal;
		return pDecal;
	}

	// What this code does is basically set up a linked list as long
	// as it can, and once the max amount of decals have been reached
	// it starts going from the back again, replacing each original decal. 
	vbmdecal_t *pfirst = m_pExtraInfo->pvbmdecalheader;
	vbmdecal_t *pnext = pfirst;

	if(pfirst->totaldecals == MAX_VBM_ENTITY_DECALS)
		pfirst->totaldecals = 0;

	for(Uint32 i = 0; i < MAX_VBM_ENTITY_DECALS; i++)
	{
		if(i == pfirst->totaldecals)
		{
			if(!pnext->meshes.empty())
			{
				for(Uint32 j = 0; j < pnext->meshes.size(); j++)
					delete pnext->meshes[j];

				pnext->meshes.clear();
			}

			pnext->entindex = NO_ENTITY_INDEX;
			pnext->pentity = nullptr; 
			pnext->num_vertexes = 0;
			pnext->pentry = nullptr; 
			pnext->start_vertex = 0;
			pnext->totaldecals = 0;
			
			pfirst->totaldecals++;
			return pnext;
		}

		if(!pnext->next)
		{
			vbmdecal_t *pDecal = AllocDecalSlot();
			pnext->next = pDecal;
			pDecal->prev = pnext;
			pfirst->totaldecals++;

			return pDecal;
		}

		vbmdecal_t *next = pnext->next;
		pnext = next;
	}
	return nullptr;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::CreateDecal( const Vector& position, const Vector& normal, decalgroupentry_t *texptr, cl_entity_t *pEntity, byte flags )
{
	if(!texptr)
		return;

	if(!pEntity->pmodel)
		return;

	if(pEntity->pmodel->type != MOD_VBM)
		return;

	// Make sure the texture is loaded
	if(!texptr->ptexture)
	{
		gDecals.PrecacheTexture(texptr->name.c_str());
		if(!texptr->ptexture)
		{
			Con_Printf("%s - Could not load texture for entry '%s'.\n", __FUNCTION__, texptr->name.c_str());
			return;
		}
	}

	m_pCurrentEntity = pEntity;

	m_pCacheModel = gModelCache.GetModelByIndex(pEntity->curstate.modelindex);
	if(!m_pCacheModel || !m_pCacheModel->pcachedata)
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, pEntity->curstate.modelindex);
		return;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return;
	
	SetExtraInfo();
	if(!m_pExtraInfo)
		return;

	SetOrientation();

	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_SETUPBONES);
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Allocate a decal
	vbmdecal_t *pdecal = AllocDecal();
	if(!pdecal)
		return;

	pdecal->entindex = m_pCurrentEntity->entindex;
	pdecal->pentity = m_pCurrentEntity;
	pdecal->identifier = m_pCurrentEntity->identifier;
	pdecal->pentry = texptr;

	Int32 skinnum = m_pCurrentEntity->curstate.skin;
	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();

	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	const vbmvertex_t *pvertexes = m_pVBMHeader->getVertexes();
	const Uint32 *pindexes = m_pVBMHeader->getIndexes();

	Vector right, up;
	Math::GetUpRight(normal, up, right);

	// Make sure these are reset
	m_numTempIndexes = 0;
	m_numTempVertexes = 0;

	// Current mesh being built
	vbm_decal_mesh_t* pdecalmesh = nullptr;

	Uint32 curstart;
	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		const vbmbodypart_t *pbodypart = m_pVBMHeader->getBodyPart(i);

		Uint32 submodelindex = m_pCurrentEntity->curstate.body / pbodypart->base;
		submodelindex = submodelindex % pbodypart->numsubmodels;

		const vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(m_pVBMHeader, submodelindex);

		// Allocate a new mesh for this bodygroup
		if(!pdecalmesh)
			pdecalmesh = new vbm_decal_mesh_t;

		// Set current data
		pdecalmesh->bodypartindex = i;
		pdecalmesh->submodelindex = submodelindex;

		const byte *pboneids = nullptr;
		for (Int32 j = 0; j < psubmodel->nummeshes; j++) 
		{
			curstart = m_numTempVertexes;
			const vbmmesh_t *pmesh = psubmodel->getMesh(m_pVBMHeader, j);
			const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

			en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
			if(!pmaterial)
				continue;

			if(pmesh->numbones)
				pboneids = pmesh->getBones(m_pVBMHeader);

			if(pmaterial->flags & TX_FL_NODECAL)
				continue;

			if(!pboneids)
				continue;

			if(psubmodel->flexinfoindex != -1 && !m_isVertexFetchSupported)
				continue;

			const vbmvertex_t *triverts[3] = { nullptr };
			for(Int32 k = 0; k < pmesh->num_indexes; k += 3)
			{
				for(Uint32 l = 0; l < 3; l++)
					triverts[l] = &pvertexes[pindexes[pmesh->start_index+k+l]];

				if (!DecalTriangle(i, submodelindex, pdecal, pdecalmesh, triverts, pboneids, position, normal, pdecal, up, right, curstart, flags, pmaterial))
				{
					Con_Printf("%s - Error creating decal '%s' for VBM model '%s'.\n", __FUNCTION__, texptr->name.c_str(), m_pVBMHeader->name);
					DeleteDecal(pdecal);
					return;
				}
			}
		}

		// Finalize this mesh if it has anything in it
		if(m_numTempVertexes > 0)
		{
			FinalizeDecalMesh(pdecal, pdecalmesh, curstart);
			pdecal->meshes.push_back(pdecalmesh);

			// Reset this so we allocate a new one
			pdecalmesh = nullptr;
		}
	}
	
 	if(pdecal->meshes.empty())
	{
		// No geometry was decalled
		DeleteDecal(pdecal);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::FinalizeDecalMesh( vbmdecal_t* pdecal, vbm_decal_mesh_t* pmesh, Uint32& curstart )
{
	Uint32 indexoffset, vertexoffset;
	GetDecalOffsets(m_numTempVertexes, m_numTempIndexes, vertexoffset, indexoffset);

	for(Uint32 i = 0; i < m_numTempIndexes; i++)
		m_tempIndexes[i] += vertexoffset;

	for(Uint32 i = 0; i < m_numTempVertexes; i++)
	{
		for(Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[i].boneweights[j] /= 255.0f;

		VBM_NormalizeWeights(m_tempVertexes[i].boneweights, MAX_VBM_BONEWEIGHTS);
	}

	m_pDecalVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*vertexoffset, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	
	if(!pdecal->start_vertex)
		pdecal->start_vertex = vertexoffset;

	pdecal->num_vertexes += m_numTempVertexes;

	m_pDecalVBO->IBOSubBufferData(sizeof(Uint32)*indexoffset, m_tempIndexes, sizeof(Uint32)*m_numTempIndexes);
	pmesh->start_index = indexoffset;
	pmesh->num_indexes = m_numTempIndexes;

	m_numTempVertexes = 0;
	m_numTempIndexes = 0;
	curstart = 0;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DecalTriangle( Int32 pbodypartindex, Int32 submodelindex, vbmdecal_t* pdecal, vbm_decal_mesh_t*& pmesh, const vbmvertex_t **pverts, const byte *pboneids, const Vector& position, const Vector& normal, vbmdecal_t *decal, const Vector& up, const Vector& right, Uint32& curstart, byte flags, en_material_t* pmaterial )
{
	static Vector tmp;
	static Vector baseverts[3];
	static Vector dverts1[64];
	static Vector dverts2[64];
	static Float boneweights[MAX_VBM_BONEWEIGHTS];

	for(Uint32 i = 0; i < 3; i++)
	{
		Math::VectorClear(baseverts[i]);
		for(Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
			boneweights[j] = static_cast<Float>(pverts[i]->boneweights[j])/255.0f;

		VBM_NormalizeWeights(boneweights, MAX_VBM_BONEWEIGHTS);

		for(Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
		{
			if(!pverts[i]->boneweights[j])
				continue;

			byte boneindex = pboneids[pverts[i]->boneindexes[j]/3];
			Math::VectorTransform(pverts[i]->origin, (*m_pWeightBoneTransform)[boneindex].matrix, tmp);
			Math::VectorMA(baseverts[i], boneweights[j], tmp, baseverts[i]);
		}

		Math::VectorCopy(baseverts[i], dverts1[i]);
	}
	
	if(!(flags & FL_DECAL_NORMAL_PERMISSIVE))
	{
		Vector vecA = (baseverts[0] - baseverts[1]);
		Vector vecB = (baseverts[1] - baseverts[2]);

		Vector trinormal;
		Math::CrossProduct(vecA, vecB, trinormal);
		Math::VectorNormalize(trinormal);
		Math::VectorScale(trinormal, -1, trinormal);

		if(Math::DotProduct(normal, trinormal) < 0.01)
			return true;
	}

	Float texc_orig_x = Math::DotProduct(position, right);
	Float texc_orig_y = Math::DotProduct(position, up);

	Int32 xsize = decal->pentry->xsize;
	Int32 ysize = decal->pentry->ysize;

	Uint32 nv;
	Vector planepoint;
	Math::VectorMA(position, -xsize, right, planepoint);	
	nv = Decal_ClipPolygon(dverts1, 3, right, planepoint, dverts2);
	if (nv < 3) 
		return true;

	Math::VectorMA(position, xsize, right, planepoint);
	nv = Decal_ClipPolygon(dverts2, nv, right*-1, planepoint, dverts1);
	if (nv < 3) 
		return true;

	Math::VectorMA(position, -ysize, up, planepoint);
	nv = Decal_ClipPolygon(dverts1, nv, up, planepoint, dverts2);
	if (nv < 3) 
		return true;

	Math::VectorMA(position, ysize, up, planepoint);
	nv = Decal_ClipPolygon(dverts2, nv, up*-1, planepoint, dverts1);
	if (nv < 3)
		return true;

	bool alphatest = pmaterial->flags & TX_FL_ALPHATEST ? true : false;
	en_texture_t* ptexture = pmaterial->ptextures[MT_TX_DIFFUSE];

	// Check if we need to account for alphatest texture switches, etc
	if(pmesh->alphatest != alphatest || pmesh->alphatest && ptexture != pmesh->ptexture)
	{
		// Finalize the mesh
		if(m_numTempVertexes > 0)
		{
			// Close this mesh
			FinalizeDecalMesh(pdecal, pmesh, curstart);
			pdecal->meshes.push_back(pmesh);
			
			// Allocate a new mesh
			pmesh = new vbm_decal_mesh_t;
		}

		pmesh->alphatest = alphatest;
		pmesh->bodypartindex = pbodypartindex;
		pmesh->submodelindex = submodelindex;

		if(pmesh->alphatest)
			pmesh->ptexture = ptexture;
	}

	// Split the decal if we're going over the array size
	if((m_numTempVertexes+3) >= MAX_TEMP_VBM_VERTEXES || (m_numTempIndexes+3) >= MAX_TEMP_VBM_INDEXES)
	{
		// Close this mesh
		FinalizeDecalMesh(pdecal, pmesh, curstart);
		pdecal->meshes.push_back(pmesh);
			
		// Allocate a new mesh
		pmesh = new vbm_decal_mesh_t;

		// Set basic infos
		pmesh->alphatest = alphatest;
		pmesh->bodypartindex = pbodypartindex;
		pmesh->submodelindex = submodelindex;

		if(pmesh->alphatest)
			pmesh->ptexture = ptexture;
	}

	Uint32 numadd = 0;
	byte addbones[MAX_SHADER_BONES] = { 0 };
	for (Uint32 i = 0; i < 3; i++)
	{
		for (Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
		{
			if (!pverts[i]->boneweights[j])
				continue;

			byte boneindex = pboneids[static_cast<byte>(pverts[i]->boneindexes[j] / 3)];

			Int32 k = 0;
			for (; k < pmesh->numbones; k++)
			{
				if (pmesh->pbones[k] == boneindex)
					break;
			}

			Int32 l = 0;
			for (; l < numadd; l++)
			{
				if (boneindex == addbones[l])
					break;
			}

			if (k == pmesh->numbones && static_cast<Uint32>(l) == numadd)
			{
				addbones[numadd] = boneindex;
				numadd++;
			}
		}
	}

	// Check if we go over the max number of bones in a single decal mesh,
	// and if we do, close off this one and create a new entry altogether
	if(pmesh->numbones && (pmesh->numbones + numadd) > MAX_SHADER_BONES)
	{
		// Finalize the current mesh
		FinalizeDecalMesh(pdecal, pmesh, curstart);
		pdecal->meshes.push_back(pmesh);

		// Allocate a new mesh
		pmesh = new vbm_decal_mesh_t;

		// Set basic infos
		pmesh->alphatest = alphatest;
		pmesh->bodypartindex = pbodypartindex;
		pmesh->submodelindex = submodelindex;

		if(pmesh->alphatest)
			pmesh->ptexture = ptexture;

		// Re-calculate the number of bones to be added, as we need to discard
		// the bones from the previous mesh altogether and build a new list of
		// bones to add
		numadd = 0;

		// We are safe here, as we have 3x4 bones max which means
		// we cannot overflow the 32 bone limit
		for (Uint32 i = 0; i < 3; i++)
		{
			for (Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
			{
				if (!pverts[i]->boneweights[j])
					continue;

				byte boneindex = pboneids[static_cast<byte>(pverts[i]->boneindexes[j] / 3)];
				Uint32 l = 0;
				for (; l < numadd; l++)
				{
					if (boneindex == addbones[l])
						break;
				}

				if(l == numadd)
				{
					addbones[numadd] = boneindex;
					numadd++;
				}
			}
		}
	}

	// Add any bones we need to
	assert(pmesh->numbones + numadd <= MAX_SHADER_BONES);
	if (numadd > 0)
	{
		void* pnewbuffer = Common::ResizeArray(pmesh->pbones, sizeof(byte), pmesh->numbones, numadd);
		pmesh->pbones = static_cast<byte*>(pnewbuffer);

		for (Uint32 j = 0; j < numadd; j++)
			pmesh->pbones[pmesh->numbones+j] = addbones[j];

		pmesh->numbones += numadd;
	}

	// Cutting is just for exclusion testing, we use the original triangle otherwise
	for(Uint32 i = 0; i < 3; i++)
	{
		// See if this vertex is already present
		Uint32 j = curstart;
		for(; j < m_numTempVertexes; j++)
		{
			if(Math::VectorCompare(m_tempVertexes[j].origin, pverts[i]->origin))
			{
				Uint32 k = 0;
				for(; k < MAX_VBM_BONEWEIGHTS; k++)
				{
					if(!pverts[i]->boneweights[k])
						continue;

					byte boneidx1 = pboneids[static_cast<byte>(pverts[i]->boneindexes[k]/3)];
					byte boneidx2 = pmesh->pbones[static_cast<byte>(m_tempVertexes[j].boneindexes[k]/3)];

					if(boneidx1 != boneidx2)
						break;

					if(pverts[i]->boneweights[k] != static_cast<byte>(m_tempVertexes[j].boneweights[k]))
						break;
				}

				if(k == MAX_VBM_BONEWEIGHTS)
					break;
			}
		}

		// Add if it it wasn't found
		if(j == m_numTempVertexes)
		{
			// Add the bones to the mesh
			for (Uint32 k = 0; k < MAX_VBM_BONEWEIGHTS; k++)
			{
				if (!pverts[i]->boneweights[k])
				{
					m_tempVertexes[m_numTempVertexes].boneindexes[k] = 0;
					m_tempVertexes[m_numTempVertexes].boneweights[k] = 0;
					continue;
				}

				// See if it's present already
				byte boneindex = pboneids[static_cast<byte>(pverts[i]->boneindexes[k] / 3)];

				Int32 l = 0;
				for (; l < pmesh->numbones; l++)
				{
					if (pmesh->pbones[l] == boneindex)
						break;
				}

				if (l == pmesh->numbones)
				{
					Con_EPrintf("%s - Could not find bone in mesh bone list generated.\n", __FUNCTION__);
					return false;
				}

				m_tempVertexes[m_numTempVertexes].boneweights[k] = pverts[i]->boneweights[k];
				m_tempVertexes[m_numTempVertexes].boneindexes[k] = (l * 3);
			}

			// Calculate texcoords
			Float texc_x = (Math::DotProduct(baseverts[i], right) - texc_orig_x)/xsize;
			Float texc_y = (Math::DotProduct(baseverts[i], up) - texc_orig_y)/ysize;

			m_tempVertexes[m_numTempVertexes].texcoord1[0] = ((texc_x + 1)/2);
			m_tempVertexes[m_numTempVertexes].texcoord1[1] = ((texc_y + 1)/2);

			m_tempVertexes[m_numTempVertexes].texcoord2[0] = pverts[i]->texcoord[0];
			m_tempVertexes[m_numTempVertexes].texcoord2[1] = pverts[i]->texcoord[1];

			if( pverts[i]->flexvertindex != -1 )
			{
				Int32 row_verts = VBM_FLEXTEXTURE_SIZE/3;
				Int32 tcy = (pverts[i]->flexvertindex + 1) / row_verts;
				Int32 tcx = (pverts[i]->flexvertindex + 1) % row_verts;

				m_tempVertexes[m_numTempVertexes].flexcoord[0] = static_cast<Float>(tcx*3) / static_cast<Float>(VBM_FLEXTEXTURE_SIZE);
				m_tempVertexes[m_numTempVertexes].flexcoord[1] = static_cast<Float>(tcy*3) / static_cast<Float>(VBM_FLEXTEXTURE_SIZE);
			}
			else
			{
				m_tempVertexes[m_numTempVertexes].flexcoord[0] = 0;
				m_tempVertexes[m_numTempVertexes].flexcoord[1] = 0;
			}

			Math::VectorCopy(pverts[i]->origin, m_tempVertexes[m_numTempVertexes].origin); 
			m_numTempVertexes++;
		}

		m_tempIndexes[m_numTempIndexes] = j;
		m_numTempIndexes++;
	}

	if (pmesh->numbones <= MAX_SHADER_BONES)
	{
		return true;
	}
	else
	{
		Con_EPrintf("%s - Exceeded MAX_SHADER_BONES on mesh.\n", __FUNCTION__);
		return false;
	}

}

//=============================================
//
//
//=============================================
void CVBMRenderer::DeleteDecal( vbmdecal_t *pdecal )
{
	entity_extrainfo_t* pextrainfo = CL_GetEntityExtraData(pdecal->pentity);

	// Unlink this
	if(pdecal->next)
	{
		pdecal->next->prev = pdecal->prev;
	}

	if(pdecal->prev)
	{
		// relink on this end
		pdecal->prev->next = pdecal->next;

		vbmdecal_t *proot = pdecal->prev;
		while(true)
		{
			if(!proot->prev)
				break;

			proot = proot->prev;
		}

		// take down one
		proot->totaldecals--;
	}
	else if(pextrainfo && pextrainfo->pvbmdecalheader && pextrainfo->pvbmdecalheader == pdecal)
	{
		if(pdecal->next)
		{
			pextrainfo->pvbmdecalheader = pdecal->next;
			pdecal->next->totaldecals = pdecal->totaldecals-1;
		}
		else
		{
			pextrainfo->pvbmdecalheader = nullptr;
		}
	}

	ClearDecal(pdecal);
}

//=============================================
//
//
//=============================================
void CVBMRenderer::ClearDecal( vbmdecal_t* pdecal )
{
	if(!pdecal->meshes.empty())
	{
		for(Uint32 i = 0; i < pdecal->meshes.size(); i++)
			delete pdecal->meshes[i];

		pdecal->meshes.clear();
	}

	// Clear the decal
	(*pdecal) = vbmdecal_t();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawModelDecals( void )
{
	vbmdecal_t *pnext = m_pExtraInfo->pvbmdecalheader;

	// Check if playermodel or v_ model was changed
	if(pnext->identifier != m_pCurrentEntity->identifier)
	{
		FreeEntityData(m_pCurrentEntity);
		return true;
	}

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);
	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_light_radius);

	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
	}

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->DisableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_color);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);
	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_texture1);

	Int32 alphatestMode = (rns.msaa && rns.mainframe) ? ALPHATEST_COVERAGE : ALPHATEST_LESSTHAN;
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, alphatestMode, false))
		return false;

	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);

	m_pShader->EnableAttribute(m_attribs.a_texcoord1);

	if(alphatestMode == ALPHATEST_COVERAGE)
	{
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		gGLExtF.glSampleCoverage(0.5, GL_FALSE);
	}

	// Reset all units used
	m_pShader->ResetSamplerIndex();

	Int32 textureIndex = 0;
	if((m_pVBMHeader->flags & VBM_HAS_FLEXES) && m_isVertexFetchSupported)
	{
		m_pShader->EnableAttribute(m_attribs.a_flexcoord);

		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, TRUE, false))
			return false;

		textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_flextexture);
		R_Bind2DTexture(GL_TEXTURE0_ARB + textureIndex, m_pFlexTexture->gl_index);
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false))
			return false;
	}

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glPolygonOffset(-1,-1);
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Set neutral color as fog color
	m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.5, 0.5, 0.5);

	// Last used decal rendermode
	decal_rendermode_t renderMode = DECAL_RENDERMODE_NONE;

	while(pnext)
	{
		// textureIndex holds the last used unit, so add +1
		Int32 local_index = textureIndex + 1;
		m_pShader->ResetSamplerIndex(local_index);

		local_index = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture0);
		R_Bind2DTexture(GL_TEXTURE0 + local_index, pnext->pentry->ptexture->palloc->gl_index);

		for(Uint32 i = 0; i < pnext->meshes.size(); i++)
		{
			vbm_decal_mesh_t* pmesh = pnext->meshes[i];

			// Check if the bodypart this decal mesh belongs to is actually visible
			const vbmbodypart_t *pbodypart = m_pVBMHeader->getBodyPart(pmesh->bodypartindex);
			Uint32 submodelindex = m_pCurrentEntity->curstate.body / pbodypart->base;
			submodelindex = submodelindex % pbodypart->numsubmodels;

			// Submodel index matches if this is the actively displayed submodel of this
			// body part
			if(submodelindex != static_cast<Uint32>(pmesh->submodelindex))
				continue;

			// Determine rendering method required
			decal_rendermode_t requestedRenderMode;
			if(pmesh->alphatest && pmesh->ptexture && pmesh->ptexture->palloc)
				requestedRenderMode = DECAL_RENDERMODE_ALPHATEST;
			else
				requestedRenderMode = DECAL_RENDERMODE_NORMAL;

			// Switch only if needed
			if(requestedRenderMode != renderMode)
			{
				switch(requestedRenderMode)
				{
				case DECAL_RENDERMODE_ALPHATEST:
					{
						if(rns.fog.settings.active)
						{
							if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texonly_holes_fog))
								return false;
						}
						else
						{
							if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texonly_holes))
								return false;
						}

						// Enable texcoord for main tex
						m_pShader->EnableAttribute(m_attribs.a_texcoord2);
					}
					break;
				case DECAL_RENDERMODE_NORMAL:
					{
						if(rns.fog.settings.active)
						{
							if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texonly_fog))
								return false;

							m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.5, 0.5, 0.5);
						}
						else
						{
							if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texonly))
								return false;
						}

						m_pShader->DisableAttribute(m_attribs.a_texcoord2);
					}
					break;
				}

				// Optimize state switches
				renderMode = requestedRenderMode;
			}

			// Bind main texture if using alphatest
			if(renderMode == DECAL_RENDERMODE_ALPHATEST)
			{
				local_index = m_pShader->AutoSetSamplerUniform(m_attribs.u_texture1);
				R_Bind2DTexture(GL_TEXTURE0 + local_index, pmesh->ptexture->palloc->gl_index);
			}

			// Fix overlapping sampler issue
			if(!m_pShader->PerformPreRenderChecks())
				return false;

			SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->pbones, pmesh->numbones);

			R_ValidateShader(m_pShader);

			m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(pmesh->start_index));
		}

		vbmdecal_t *next = pnext->next;
		pnext = next;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
		return false;

	if(m_pVBMHeader->flags & VBM_HAS_FLEXES && m_isVertexFetchSupported)
		m_pShader->DisableAttribute(m_attribs.a_flexcoord);

	m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);

	// Ensure this is disabled
	m_pShader->DisableAttribute(m_attribs.a_texcoord2);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	if(alphatestMode == ALPHATEST_COVERAGE)
	{
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		gGLExtF.glSampleCoverage(1.0, GL_FALSE);
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::GetDecalOffsets( Uint32 numverts, Uint32 numindexes, Uint32& vertexoffset, Uint32& indexoffset )
{
	if((m_vCache_Index-m_vCache_Base)+numverts > m_decalVertexCacheSize)
		m_vCache_Index = m_vCache_Base;

	if((m_iCache_Index-m_iCache_Base)+numindexes > m_decalIndexCacheSize)
		m_iCache_Index = m_iCache_Base;

	for(Uint32 i = 0; i < MAX_VBM_TOTAL_DECALS; i++)
	{
		vbmdecal_t *pdecal = &m_vbmDecals[i];

		if(!pdecal->num_vertexes)
			continue;
		
		// Check if vertex start is inside
		if(m_vCache_Index >= pdecal->start_vertex && m_vCache_Index < (pdecal->start_vertex+pdecal->num_vertexes))
		{
			DeleteDecal(pdecal);
			continue;
		}

		// Check if vertex end is inside
		if((m_vCache_Index+numverts) >= pdecal->start_vertex && (m_vCache_Index+numverts) < (pdecal->start_vertex+pdecal->num_vertexes))
		{
			DeleteDecal(pdecal);
			continue;
		}

		// Check if current decal vertexes are inside of this
		if(m_vCache_Index <= pdecal->start_vertex && (m_vCache_Index+numverts) > (pdecal->start_vertex+pdecal->num_vertexes))
		{
			DeleteDecal(pdecal);
			continue;
		}

		for(Uint32 j = 0; j < pdecal->meshes.size(); j++)
		{
			vbm_decal_mesh_t* pmesh = pdecal->meshes[j];

			// Check if index start is inside
			if(m_iCache_Index >= pmesh->start_index && m_iCache_Index < (pmesh->start_index+pmesh->num_indexes))
			{
				DeleteDecal(pdecal);
				break;
			}
			
			// Check if index end is inside
			if((m_iCache_Index+numindexes) >= pmesh->start_index && (m_iCache_Index+numindexes) < (pmesh->start_index+pmesh->num_indexes))
			{
				DeleteDecal(pdecal);
				break;
			}

			// Check if current decal vertexes are inside of this
			if(m_iCache_Index <= pmesh->start_index && (m_iCache_Index+numindexes) > (pmesh->start_index+pmesh->num_indexes))
			{
				DeleteDecal(pdecal);
				break;
			}
		}
	}

	vertexoffset = m_vCache_Index;
	m_vCache_Index += numverts;

	indexoffset = m_iCache_Index;
	m_iCache_Index += numindexes;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawNormalSubmodels( void )
{
	// Reset
	m_numDrawSubmodels = 0;
	m_firstTextureUnit = 0;

	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_DISTANCE);

		if(m_pVBMSubModel->flexinfoindex != -1)
			continue;

		if(m_numDrawSubmodels == RENDERED_SUBMODELS_ALLOC_SIZE)
			m_pSubmodelDrawList.resize(m_pSubmodelDrawList.size()+RENDERED_SUBMODELS_ALLOC_SIZE);

		m_pSubmodelDrawList[m_numDrawSubmodels] = m_pVBMSubModel;
		m_numDrawSubmodels++;
	}

	// Run draw routines
	if(!SetupRenderer())
		return false;

	if(!DrawFirst())
		return false;

	if(!DrawStyles(false, false))
		return false;

	if(!DrawLights(false, false))
		return false;

	if(!DrawFinal())
		return false;

	if(!RestoreRenderer())
		return false;

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawFlexedSubmodels( void )
{
	if(!(m_pVBMHeader->flags & VBM_HAS_FLEXES))
		return true;

	if(m_isVertexFetchSupported)
	{
		m_pShader->EnableSync(m_attribs.u_flextexture);
		m_pShader->EnableSync(m_attribs.u_flextexturesize);

		m_pShader->EnableAttribute(m_attribs.a_flexcoord);

		// Always use AutoSetSamplerUniform if overlap checks are present
		m_pShader->ResetSamplerIndex();
		Int32 textureIndex = m_pShader->AutoSetSamplerUniform(m_attribs.u_flextexture);
		R_Bind2DTexture(GL_TEXTURE0_ARB + textureIndex, m_pFlexTexture->gl_index);

		m_pShader->SetUniform1f(m_attribs.u_flextexturesize, VBM_FLEXTEXTURE_SIZE);
		m_firstTextureUnit = textureIndex + 1;

		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, TRUE, false))
			return false;

		m_useFlexes = true;
	}
	else
	{
		m_firstTextureUnit = 0;
	}

	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_NONE);

		if(m_pVBMSubModel->flexinfoindex == -1)
			continue;

		// Calculate flexes if we have any
		if( m_pExtraInfo )
		{
			if( m_isVertexFetchSupported )
				CalculateFlexesHW(m_pVBMSubModel);
			else
				CalculateFlexesSW(m_pVBMSubModel);
		}

		// Draw submodel by submodel if using flexes
		m_pSubmodelDrawList[0] = m_pVBMSubModel;
		m_numDrawSubmodels = 1;

		// Draw normally
		if(!SetupRenderer())
			return false;

		if(!DrawFirst())
			return false;

		if(!DrawStyles(false, false))
			return false;

		if(!DrawLights(false, false))
			return false;

		if(!DrawFinal())
			return false;

		if(!RestoreRenderer())
			return false;
	}

	if(m_isVertexFetchSupported)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false))
			return false;

		m_pShader->DisableAttribute(m_attribs.a_flexcoord);
	}

	m_useFlexes = false;
	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::UpdateAttachments( cl_entity_t *pEntity )
{
	m_pCurrentEntity = pEntity;
	m_pCacheModel = gModelCache.GetModelByIndex(pEntity->curstate.modelindex);
	if(!m_pCacheModel || !m_pCacheModel->pcachedata)
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, pEntity->curstate.modelindex);
		return;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return;

	SetExtraInfo();
	SetOrientation();

	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_SETUPBONES);
	}

	if ( m_pStudioHeader->numattachments > MAX_ATTACHMENTS )
		return;

	CalculateAttachments();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::GetBonePosition( cl_entity_t *pEntity, const Char *szname, Vector& origin )
{
	m_pCurrentEntity = pEntity;
	m_pCacheModel = gModelCache.GetModelByIndex(pEntity->curstate.modelindex);
	if(!m_pCacheModel || !m_pCacheModel->pcachedata)
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, pEntity->curstate.modelindex);
		return false;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return false;

	SetExtraInfo();
	SetOrientation();

	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_SETUPBONES);
	}

	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t *pbone = m_pStudioHeader->getBone(i);

		if(!strcmp(pbone->name, szname))
		{
			for(Uint32 j = 0; j < 3; j++)
				origin[j] = (*m_pBoneTransform)[i].matrix[j][3];

			return true;
		}
	}

	return false;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::TransformVectorByBoneMatrix( cl_entity_t *pEntity, Int32 boneindex, Vector& vector, bool inverse )
{
	m_pCurrentEntity = pEntity;
	m_pCacheModel = gModelCache.GetModelByIndex(pEntity->curstate.modelindex);
	if(!m_pCacheModel || !m_pCacheModel->pcachedata)
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, pEntity->curstate.modelindex);
		return;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	if(!pstudiocache)
		return;

	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return;

	if(boneindex >= m_pStudioHeader->numbones || boneindex < 0)
	{
		Con_Printf("%s - Bone index %d out of bounds.\n", __FUNCTION__, boneindex);
		return;
	}

	SetExtraInfo();
	SetOrientation();

	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_SETUPBONES);
	}

	Vector transvector = vector;
	if(inverse)
	{
		for(Uint32 i = 0; i < 3; i++)
			transvector[i] -= (*m_pBoneTransform)[boneindex].matrix[i][3];

		Math::VectorInverseRotate(transvector, (*m_pBoneTransform)[boneindex].matrix, vector);
	}
	else
	{
		Math::VectorTransform(transvector, (*m_pBoneTransform)[boneindex].matrix, vector);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::RotateVectorByBoneMatrix( cl_entity_t *pEntity, Int32 boneindex, Vector& vector, bool inverse )
{
	m_pCurrentEntity = pEntity;
	m_pCacheModel = gModelCache.GetModelByIndex(pEntity->curstate.modelindex);
	if(!m_pCacheModel || !m_pCacheModel->pcachedata)
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, pEntity->curstate.modelindex);
		return;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	if(!pstudiocache)
		return;

	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return;

	if(boneindex >= m_pStudioHeader->numbones || boneindex < 0)
	{
		Con_Printf("%s - Bone index %d out of bounds.\n", __FUNCTION__, boneindex);
		return;
	}

	SetExtraInfo();
	SetOrientation();

	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_SETUPBONES);
	}

	Vector transvector = vector;
	if(inverse)
		Math::VectorInverseRotate(transvector, (*m_pBoneTransform)[boneindex].matrix, vector);
	else
		Math::VectorRotate(transvector, (*m_pBoneTransform)[boneindex].matrix, vector);
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::PrepareDraw( void )
{
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	if(!m_pShader->EnableShader())
		return false;

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());
	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::EndDraw( void )
{
	m_pShader->DisableShader();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawDecals( bool transparentPass )
{
	if(g_pCvarDrawEntities->GetValue() != 1)
		return true;

	if(m_pCvarDrawModelDecals->GetValue() < 1)
		return true;

	if(!PrepareDraw())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	// Set 
	m_pShader->SetVBO(m_pDecalVBO);
	m_pCurrentVBO = m_pDecalVBO;

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pEntity = rns.objects.pvisents[i];

		if(pEntity->curstate.effects & EF_LADDER)
			continue;

		if(pEntity->curstate.effects & EF_VIEWONLY)
			continue;

		// Handle skydraw specially
		if (rns.water_skydraw)
		{
			if (pEntity->curstate.renderfx != RenderFx_SkyEnt
				&& pEntity->curstate.renderfx != RenderFx_SkyEntScaled)
				continue;
		}
		else
		{
			if (pEntity->curstate.renderfx == RenderFx_SkyEnt
				|| pEntity->curstate.renderfx == RenderFx_SkyEntScaled)
				continue;
		}

		// Handle portals specially
		if (rns.portalpass)
		{
			if (pEntity->curstate.renderfx != RenderFx_InPortalEntity
				&& pEntity->curstate.renderfx != RenderFx_InPortalScaledModel)
				continue;
		}
		else
		{
			if (pEntity->curstate.renderfx == RenderFx_InPortalEntity
				|| pEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
				continue;
		}

		// Never allow no-depth cull entities to be rendered here
		if (pEntity->curstate.renderfx == RenderFx_SkyEntNC)
			continue;

		if(pEntity->pmodel->type != MOD_VBM)
			continue;

		if(R_IsSpecialRenderEntity(*pEntity))
			continue;

		if(transparentPass)
		{
			if(!R_IsEntityTransparent(*pEntity))
				continue;
		}
		else
		{
			if(R_IsEntityTransparent(*pEntity))
				continue;
		}

		if(pEntity->player)
			continue;
		
		if(!DrawEntityDecals(pEntity))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndDraw();
			return false;
		}
	}

	if(m_pCvarDrawPlayer->GetValue() >= 1)
	{
		cl_entity_t* plocalplayer = CL_GetLocalPlayer();

		if(!DrawEntityDecals(plocalplayer))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndDraw();
			return false;
		}
	}

	EndDraw();

	// Clear any binds
	R_ClearBinds();
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawNormal( void )
{
	if(g_pCvarDrawEntities->GetValue() < 1)
		return true;

	if(!PrepareDraw())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pEntity = rns.objects.pvisents[i];

		if(pEntity->curstate.effects & EF_LADDER)
			continue;

		if(pEntity->curstate.effects & EF_VIEWONLY)
			continue;

		// Handle skydraw specially
		if (rns.water_skydraw)
		{
			if (pEntity->curstate.renderfx != RenderFx_SkyEnt
				&& pEntity->curstate.renderfx != RenderFx_SkyEntScaled)
				continue;
		}
		else
		{
			if (pEntity->curstate.renderfx == RenderFx_SkyEnt
				|| pEntity->curstate.renderfx == RenderFx_SkyEntScaled)
				continue;
		}

		// Handle portals specially
		if (rns.portalpass)
		{
			if (pEntity->curstate.renderfx != RenderFx_InPortalEntity
				&& pEntity->curstate.renderfx != RenderFx_InPortalScaledModel)
				continue;
		}
		else
		{
			if (pEntity->curstate.renderfx == RenderFx_InPortalEntity
				|| pEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
				continue;
		}

		// Never allow no-depth cull entities to be rendered here
		if (pEntity->curstate.renderfx == RenderFx_SkyEntNC)
			continue;

		if(pEntity->pmodel->type != MOD_VBM)
			continue;

		if(R_IsEntityTransparent(*pEntity))
			continue;

		if(R_IsSpecialRenderEntity(*pEntity))
			continue;

		if(pEntity->player)
			continue;
		
		if(!DrawModel(VBM_RENDER, pEntity))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndDraw();
			return false;
		}
	}

	if(m_pCvarDrawPlayer->GetValue() >= 1)
	{
		cl_entity_t* plocalplayer = CL_GetLocalPlayer();

		if(!DrawModel(VBM_RENDER, plocalplayer))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndDraw();
			return false;
		}
	}

	EndDraw();

	// Clear any binds
	R_ClearBinds();
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawTransparent( void )
{
	if(g_pCvarDrawEntities->GetValue() < 1)
		return true;

	if(!PrepareDraw())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pEntity = rns.objects.pvisents[i];

		if(pEntity->curstate.effects & EF_LADDER)
			continue;

		if(pEntity->curstate.effects & EF_VIEWONLY)
			continue;

		// Handle skydraw specially
		if (rns.water_skydraw)
		{
			if (pEntity->curstate.renderfx != RenderFx_SkyEnt
				&& pEntity->curstate.renderfx != RenderFx_SkyEntScaled)
				continue;
		}
		else
		{
			if (pEntity->curstate.renderfx == RenderFx_SkyEnt
				|| pEntity->curstate.renderfx == RenderFx_SkyEntScaled)
				continue;
		}

		// Handle portals specially
		if (rns.portalpass)
		{
			if (pEntity->curstate.renderfx != RenderFx_InPortalEntity
				&& pEntity->curstate.renderfx != RenderFx_InPortalScaledModel)
				continue;
		}
		else
		{
			if (pEntity->curstate.renderfx == RenderFx_InPortalEntity
				|| pEntity->curstate.renderfx == RenderFx_InPortalScaledModel)
				continue;
		}

		// Never allow no-depth cull entities to be rendered here
		if (pEntity->curstate.renderfx == RenderFx_SkyEntNC)
			continue;

		if(pEntity->pmodel->type != MOD_VBM)
			continue;

		if(!R_IsEntityTransparent(*pEntity))
			continue;

		if(R_IsSpecialRenderEntity(*pEntity))
			continue;

		if(pEntity->player)
			continue;
		
		if(!DrawModel(VBM_RENDER, pEntity))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndDraw();
			return false;
		}
	}

	EndDraw();

	// Clear any binds
	R_ClearBinds();
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawSky( void )
{
	if(g_pCvarDrawEntities->GetValue() < 1)
		return true;

	if(!PrepareDraw())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pEntity = rns.objects.pvisents[i];

		if(pEntity->curstate.effects & EF_LADDER)
			continue;

		if(pEntity->curstate.effects & EF_VIEWONLY)
			continue;

		if(pEntity->curstate.renderfx != RenderFx_SkyEnt
			&& pEntity->curstate.renderfx != RenderFx_SkyEntScaled)
			continue;

		if(pEntity->pmodel->type != MOD_VBM)
			continue;

		if(R_IsEntityTransparent(*pEntity, true))
			continue;

		if(pEntity->player)
			continue;
		
		if(!DrawModel(VBM_RENDER, pEntity))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndDraw();
			return false;
		}
	}

	EndDraw();

	// Clear any binds
	R_ClearBinds();
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::PrepareVSM( cl_dlight_t *dl )
{
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	if(!m_pShader->EnableShader())
		return false;

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_vsm))
		return false;

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	m_pShader->SetUniform1f(m_attribs.u_light_radius, dl->radius);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);
	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_fogcolor);
	m_pShader->DisableSync(m_attribs.u_fogparams);
	m_pShader->DisableSync(m_attribs.u_color);

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->DisableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_light_radius);

	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
	}
	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::EndVSM( void )
{
	m_pShader->DisableAttribute(m_attribs.a_texcoord1);
	m_pShader->DisableShader();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawVSM( cl_dlight_t *dl, cl_entity_t** pvisents, Uint32 numentities )
{
	if(g_pCvarDrawEntities->GetValue() < 1)
		return true;

	if(!PrepareVSM(dl))
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	for(Uint32 i = 0; i < numentities; i++)
	{
		if(pvisents[i]->curstate.effects & EF_LADDER)
			continue;

		if(pvisents[i]->curstate.effects & EF_VIEWONLY)
			continue;

		if(pvisents[i]->curstate.renderfx == RenderFx_SkyEnt)
			continue;

		if(pvisents[i]->curstate.renderfx == RenderFx_NoShadow)
			continue;

		if (pvisents[i]->curstate.renderfx == RenderFx_SkyEntScaled)
			continue;

		if(pvisents[i]->pmodel->type != MOD_VBM)
			continue;

		if(R_IsEntityTransparent(*pvisents[i], true))
			continue;

		if(pvisents[i]->player)
			continue;
		
		if(dl->isStatic())
		{
			if(pvisents[i]->curstate.movetype != MOVETYPE_NONE)
				continue;
		}

		if(dl->key)
		{
			if(dl->key == pvisents[i]->entindex)
				continue;

			if(dl->key == -static_cast<Int32>(pvisents[i]->entindex))
				continue;
		}

		if(!DrawModelVSM(pvisents[i], dl))
		{
			Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
			EndVSM();
			return false;
		}
	}

	EndVSM();
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawModelVSM( cl_entity_t *pEntity, cl_dlight_t *dl )
{
	if(R_IsEntityTransparent(*pEntity, true) && pEntity->curstate.renderamt == 0)
		return true;

	if(!pEntity->pmodel)
		return true;

	m_pCurrentEntity = pEntity;

	// Make sure model is handled
	if(!SetModel())
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
		return true;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return true;

	// Set extra info
	SetExtraInfo();
	if (CheckBBox())
		return true;

	SetOrientation();

	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_RENDER);
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();
	Int32 skinnum = m_pCurrentEntity->curstate.skin; // for short..
	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	Int32 lastBoundShader = -1;
	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_SHADOW);

		for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
		{
			const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
			const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

			en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
			if(!pmaterial)
				continue;

			if(pmaterial->flags & TX_FL_ADDITIVE)
				continue;

			if(pmaterial->flags & TX_FL_ALPHATEST)
			{
				m_pShader->EnableAttribute(m_attribs.a_texcoord1);
				if(lastBoundShader != vbm_vsmalpha)
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_vsmalpha))
						return false;

					lastBoundShader = vbm_vsmalpha;
				}

				R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
			}
			else
			{
				if(lastBoundShader != vbm_vsm)
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_vsm))
						return false;

					lastBoundShader = vbm_vsm;
				}

				m_pShader->DisableAttribute(m_attribs.a_texcoord1);
			}

			// Fix overlapping sampler issue
			if(!m_pShader->PerformPreRenderChecks())
				return false;

			if(pmesh->numbones)
				SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

			R_ValidateShader(m_pShader);

			m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
		}
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::PlayEvents( void )
{
	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		if(rns.objects.pvisents[i]->curstate.effects & EF_LADDER)
			continue;

		if(rns.objects.pvisents[i]->curstate.effects & EF_VIEWONLY)
			continue;

		if(rns.objects.pvisents[i]->curstate.renderfx == RenderFx_SkyEnt)
			continue;

		if (rns.objects.pvisents[i]->curstate.renderfx == RenderFx_SkyEntScaled)
			continue;

		if(rns.objects.pvisents[i]->pmodel->type != MOD_VBM)
			continue;

		if(rns.objects.pvisents[i]->entindex <= 0)
			continue;

		if(rns.objects.pvisents[i]->curstate.effects & EF_CLIENTENT)
			continue;

		if(rns.objects.pvisents[i]->player)
			continue;
		
		DrawModel(VBM_ANIMEVENTS, rns.objects.pvisents[i]);
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::PrepAuraPass( void )
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	if(!m_pShader->EnableShader())
		return false;

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid))
		return false;

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_light_radius);
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);
	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_fogcolor);
	m_pShader->DisableSync(m_attribs.u_fogparams);

	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_matrix);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_cone_size);
		m_pShader->DisableSync(m_attribs.dlights[i].u_light_spotdirection);
	}

	m_pShader->EnableSync(m_attribs.u_projection);
	m_pShader->EnableSync(m_attribs.u_modelview);
	m_pShader->DisableSync(m_attribs.u_normalmatrix);

	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_color);

	m_isAuraPass = true;
	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::FinishAuraPass( void )
{
	m_isAuraPass = false;
	m_pShader->DisableShader();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawAura( cl_entity_t *pEntity, const Vector& color, Float alpha )
{
	if(R_IsEntityTransparent(*pEntity, true) && pEntity->curstate.renderamt == 0)
		return true;

	if(!pEntity->pmodel)
		return true;

	// Set entity ptr
	m_pCurrentEntity = pEntity;

	// Make sure model is handled
	if(!SetModel())
	{
		Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
		return true;
	}

	const vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
	m_pStudioHeader = pstudiocache->pstudiohdr;
	m_pVBMHeader = pstudiocache->pvbmhdr;

	if(!m_pStudioHeader || !m_pVBMHeader)
		return true;

	SetExtraInfo();
	if (CheckBBox())
		return true;

	m_pShader->SetUniform4f(m_attribs.u_color, color[0], color[1], color[2], alpha);

	SetOrientation();
	if(ShouldAnimate())
	{
		SetupTransformationMatrix();
		SetupBones(VBM_RENDER);
	}

	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_DISTANCE);

		for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
		{
			const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);

			if(pmesh->numbones)
				SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

			R_ValidateShader(m_pShader);

			m_pShader->DrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
		}
	}

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::DispatchClientEvents( void )
{
	if(!m_pExtraInfo)
		return;

	// Get current sequence info
	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);

	// Nothing to do here
	if(!pseqdesc->numevents)
		return;

	Float flframe = VBM_EstimateFrame(pseqdesc, rns.time, m_pCurrentEntity->curstate.frame, m_pCurrentEntity->curstate.animtime, m_pCurrentEntity->curstate.framerate, m_pCurrentEntity->curstate.effects);

	// Fixes first-frame event bug
	if(!flframe) 
		m_pCurrentEntity->eventframe = -0.01f;

	if(flframe == m_pCurrentEntity->eventframe) 
		return;

	if (flframe < m_pCurrentEntity->eventframe)
	{
		if(m_pExtraInfo->paniminfo->prevframe_sequence == m_pCurrentEntity->curstate.sequence 
			&& m_pExtraInfo->paniminfo->prevframe_frame > flframe)
		{
			for (Int32 i = 0; i < pseqdesc->numevents; i++)
			{
				const mstudioevent_t *pevent = pseqdesc->getEvent(m_pStudioHeader, i);
				if(pevent->frame <= m_pCurrentEntity->eventframe)
					continue;
				
				cls.dllfuncs.pfnVBMEvent(pevent, m_pCurrentEntity);
			}

			// Necessary to get the next loop working
			m_pCurrentEntity->eventframe = -0.01;
		}
		else
			m_pCurrentEntity->eventframe = -0.01;
	}

	for (Int32 i = 0; i < pseqdesc->numevents; i++)
	{
		const mstudioevent_t *pevent = pseqdesc->getEvent(m_pStudioHeader, i);
		if((pevent->frame > m_pCurrentEntity->eventframe && pevent->frame <= flframe))
			cls.dllfuncs.pfnVBMEvent(pevent, m_pCurrentEntity);
	}

	m_pCurrentEntity->eventframe = flframe;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::FreeEntityData( const cl_entity_t *pEntity )
{
	for(Uint32 i = 0; i < MAX_VBM_TOTAL_DECALS; i++)
	{
		if(m_vbmDecals[i].pentity == pEntity)
			DeleteDecal(&m_vbmDecals[i]);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetupModel( Uint32 bodypart, vbmlod_type_t type )
{
	const vbmbodypart_t *pbodypart = m_pVBMHeader->getBodyPart(bodypart);

	Uint32 index = m_pCurrentEntity->curstate.body / pbodypart->base;
	index = index % pbodypart->numsubmodels;

	m_pVBMSubModel = pbodypart->getSubmodel(m_pVBMHeader, index);
	
	// Get LOD if it's present
	if(m_pVBMSubModel->numlods && type != VBM_LOD_NONE)
		m_pVBMSubModel = GetIdealLOD(m_pVBMSubModel, type);
}

//=============================================
//
//
//=============================================
const vbmsubmodel_t* CVBMRenderer::GetIdealLOD( const vbmsubmodel_t* psubmodel, vbmlod_type_t type )
{
	if(!psubmodel->numlods)
		return psubmodel;

	Vector vorigin, vtmp;
	Math::VectorScale(m_mins, 0.5, vorigin);
	Math::VectorMA(vorigin, 0.5, m_maxs, vorigin);
	Math::VectorSubtract(vorigin, rns.view.v_origin, vtmp);
	
	// this algorythm sucks
	Float lastdistance = 0;
	Float distance = vtmp.Length();

	const vbmsubmodel_t* preturn = psubmodel;
	for(Int32 i = 0; i < psubmodel->numlods; i++)
	{
		const vbmlod_t* plod = psubmodel->getLOD(m_pVBMHeader, i);
		if(plod->type != type)
			continue;

		// Shadow LODs are simple
		if(type == VBM_LOD_SHADOW)
		{
			preturn = plod->getSubmodel(m_pVBMHeader);
			break;
		}
		else if(type == VBM_LOD_DISTANCE)
		{
			// Get the most ideal lod for this distance
			if(distance >= plod->distance && plod->distance > lastdistance)
			{
				preturn = plod->getSubmodel(m_pVBMHeader);
				lastdistance = plod->distance;
			}
		}
	}

	return preturn;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawBones( void )
{
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);

	if(m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, true))
		return false;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glLineWidth(2.0);

	// Draw the points
	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(i);
		if(pbone->parent == -1)
			continue;

		// Set bone
		byte boneindex = pbone->parent;
		SetShaderBoneTransform(m_pBoneTransform, &boneindex, 1);

		// Set color
		const Float* pcolor = RANDOM_COLOR_ARRAY[i%NUM_RANDOM_COLORS];
		m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 1.0);

		// Begin compiling the vertex data
		m_numTempVertexes = 0;

		Vector worldOrigin;
		for(Uint32 j = 0; j < 3; j++)
			worldOrigin[j] = (*m_pBoneTransform)[i].matrix[j][3];

		Vector parentOrigin;
		for(Uint32 j = 0; j < 3; j++)
			parentOrigin[j] = (*m_pBoneTransform)[pbone->parent].matrix[j][3];

		Vector temp;
		Math::VectorSubtract(worldOrigin, parentOrigin, temp);
		Math::VectorInverseRotate(temp, (*m_pBoneTransform)[pbone->parent].matrix, worldOrigin);

		BatchVertex(Vector(0, 0, 0));
		BatchVertex(worldOrigin);

		m_pTempDrawVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

		R_ValidateShader(m_pShader);

		m_pShader->DrawArrays(GL_LINES, m_drawBufferIndex, m_numTempVertexes);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawHitBoxes( void )
{
	Vector bboxpoints[8];

	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);

	if(m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, true))
		return false;

	glDepthMask(GL_FALSE);
	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	TR_VBMSetHullInfo(m_pCurrentEntity->pvbmhulldata, m_pCurrentEntity->pmodel, ZERO_VECTOR, ZERO_VECTOR, m_pCurrentEntity->curstate, cls.cl_time, HULL_POINT);
	entity_vbmhulldata_t* pdata = m_pCurrentEntity->pvbmhulldata;

	for(Int32 i = 0; i < m_pStudioHeader->numhitboxes; i++)
	{
		const mstudiobbox_t *pbbox = m_pStudioHeader->getHitBox(i);

		// Set bone transform
		byte boneindex = pbbox->bone;
		SetShaderBoneTransform(&pdata->bonetransform, &boneindex, 1);

		// Set color
		const Float* pcolor = RANDOM_COLOR_ARRAY[pbbox->group%NUM_RANDOM_COLORS];
		m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.25);

		DrawBox(pbbox->bbmin, pbbox->bbmax);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawBoundingBox( void )
{
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);

	if(m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, true))
		return false;

	glDepthMask(GL_FALSE);
	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Float matrix[3][4] = { 0 };
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;

	// Set bone transform to identity
	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
	else
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

	// Set color
	const Float* pcolor = RANDOM_COLOR_ARRAY[m_pCurrentEntity->entindex%NUM_RANDOM_COLORS];
	m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.25);

	DrawBox(m_mins, m_maxs);

	// Draw wireframe outline(do it like Q2 does, cause I like how it looks)
	glDisable(GL_BLEND);
	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	Vector triverts[3];

	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_numTempVertexes = 0;

	for(Uint32 i = 0; i < 3; i++)
		matrix[i][3] = m_renderOrigin[i];

	// Set bone transform to move by origin
	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
	else
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

	Uint32 i = 0;
	for(; i < 3; i++)
	{
		triverts[i] = m_bboxCorners[i];
		BatchVertex(triverts[i]);
	}

	for(; i < 8; i++)
	{
		triverts[0] = triverts[1];
		triverts[1] = triverts[2];
		triverts[2] = m_bboxCorners[i];

		for(Uint32 j = 0; j < 3; j++)
			BatchVertex(triverts[j]);
	}

	// Draw the planes
	m_pTempDrawVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	m_pShader->DrawArrays(GL_TRIANGLES, m_drawBufferIndex, m_numTempVertexes);

	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);

	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawLightVectors( void )
{
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);

	if(m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, true))
		return false;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(4.0);

	Float matrix[3][4] = { 0 };
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
	matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0.0;
	matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

	// Set bone transform to identity
	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
	else
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

	// Rebuild the entity's light origin each frame
	Vector lightorigin;
	Vector saved_lightorigin;

	if(m_pCurrentEntity->curstate.effects & EF_ALTLIGHTORIGIN)
	{
		Math::VectorCopy(m_pCurrentEntity->curstate.lightorigin, lightorigin);
	}
	else if(m_pCurrentEntity->curstate.renderfx != RenderFx_SkyEnt
		&& m_pCurrentEntity->curstate.renderfx != RenderFx_SkyEntScaled
		&& m_pStudioHeader->flags & STUDIO_MF_CENTERLIGHT)
	{
		Math::VectorScale(m_mins, 0.5, lightorigin);
		Math::VectorMA(lightorigin, 0.5, m_maxs, lightorigin);
	}
	else if( m_pCurrentEntity->curstate.effects & EF_CLIENTENT )
	{
		// Raise Z to center
		lightorigin[0] = m_renderOrigin[0];
		lightorigin[1] = m_renderOrigin[1];
		lightorigin[2] = m_mins[2]*0.5 + m_maxs[2]*0.5;
	}
	else
		Math::VectorCopy(m_renderOrigin, lightorigin);

	// Raise off the ground a bit
	Vector zadjust(0, 0, 4);
	if(m_pCurrentEntity->curstate.effects & EF_INVLIGHT)
		Math::VectorScale(zadjust, -1, zadjust);

	Math::VectorAdd(lightorigin, zadjust, lightorigin);
	Math::VectorCopy(lightorigin, saved_lightorigin);

	// Get sky light info
	bool gotLighting = false;
	bool gotBumpLighting = false;
	bool gotLightmapLighting = false;

	Vector lightdir;
	Vector lightcolor;
	Vector lmapdiffusecolor;

	// Try to trace against the sky vector
	if(rns.sky.drawsky && !cls.skycolor.IsZero() 
		&& m_pCvarSkyLighting->GetValue() >= 1)
	{
		Vector skytracevector;
		Vector skyvector = cls.skyvec;

		skyvector[2] = -skyvector[2];
		Math::VectorMA(lightorigin, -16384, skyvector, skytracevector);

		trace_t pmtrace;
		if(!(m_pStudioHeader->flags & STUDIO_MF_SKYLIGHT))
			CL_PlayerTrace(lightorigin, skytracevector, FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, pmtrace);

		if((m_pStudioHeader->flags & STUDIO_MF_SKYLIGHT) || !pmtrace.allSolid() && !pmtrace.startSolid() && !pmtrace.noHit()
			&& CL_PointContents(CL_GetEntityByIndex(0), pmtrace.endpos) == CONTENTS_SKY)
		{
			Math::VectorScale(cls.skycolor, 1.0f/255.0f, lightcolor);
			Math::VectorCopy(skyvector, lightdir);
			gotLighting = true;
			
			// Draw the light
			DrawLine(lightorigin, skytracevector, lightcolor);
		}
	}

	if(!gotLighting)
	{
		// Trace against the world
		if(ens.pworld->plightdata[SURF_LIGHTMAP_DEFAULT] && !(m_pStudioHeader->flags & STUDIO_MF_SKYLIGHT))
		{
			Vector lighttop;
			Vector lightbottom;
			bool gotlighting = false;
			const brushmodel_t* pbrushmodel = nullptr;

			if(m_pCurrentEntity->curstate.groundent != NO_ENTITY_INDEX && 
				m_pCurrentEntity->curstate.groundent != WORLDSPAWN_ENTITY_INDEX
				&& !(m_pCurrentEntity->curstate.effects & EF_ALTLIGHTORIGIN)
				&& ((m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK) == RENDER_NORMAL
				|| (m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK) == RENDER_TRANSALPHA
				|| m_pCurrentEntity->curstate.renderamt > 0))
			{
				cl_entity_t* pentity = CL_GetEntityByIndex(m_pCurrentEntity->curstate.groundent);
				if(pentity && pentity->pmodel && pentity->pmodel->type == MOD_BRUSH)
				{
					Vector offsetorigin;
					Math::VectorSubtract(lightorigin, pentity->curstate.origin, offsetorigin);
					if(!pentity->curstate.angles.IsZero())
						Math::RotateToEntitySpace(pentity->curstate.angles, offsetorigin);

					Math::VectorCopy(offsetorigin, lighttop);
					Math::VectorCopy(offsetorigin, lightbottom);
					if (m_pCurrentEntity->curstate.effects & EF_INVLIGHT) 
						lightbottom[2] += 8196;
					else
						lightbottom[2] -= 8196;

					const brushmodel_t* pentbrushmodel = pentity->pmodel->getBrushmodel();

					// Try and get bump data if possible
					if(m_pCvarUseBumpData->GetValue() >= 1.0 
						&& ens.pworld->plightdata[SURF_LIGHTMAP_AMBIENT]
						&& ens.pworld->plightdata[SURF_LIGHTMAP_DIFFUSE]
						&& ens.pworld->plightdata[SURF_LIGHTMAP_VECTORS])
					{
						gotLightmapLighting = Mod_RecursiveLightPoint_BumpData(pentbrushmodel, &pentbrushmodel->pnodes[pentbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, &lightcolor, &lmapdiffusecolor, &lightdir, nullptr);
						if(gotLightmapLighting)
						{
							if(lightcolor.Length() < lmapdiffusecolor.Length())
							{
								gotBumpLighting = true;
								gotLighting = true;
							}
							else
							{
								// We sometimes have an odd case where we have no diffuse light
								// In this case, switch color values so it doesn't look unnatural
								gotLightmapLighting = false;
							}
						}
					}

					// If we didn't get bump data, use normal light data
					if(!gotLightmapLighting)
						gotLightmapLighting = Mod_RecursiveLightPoint(pentbrushmodel, &pentbrushmodel->pnodes[pentbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, &lightcolor);

					if(gotLightmapLighting)
					{
						// Use this brushmodel for further lighting
						pbrushmodel = pentbrushmodel;

						Vector drawbottom;
						Math::VectorSubtract(lightorigin, Vector(0, 0, 8192), drawbottom);
						DrawLine(lightorigin, drawbottom, lightcolor);
					}
				}
			}

			if(!pbrushmodel)
			{
				// Take lighting from world
				pbrushmodel = ens.pworld;

				// Set the trace bottom
				Math::VectorCopy(lightorigin, lighttop);
				Math::VectorCopy(lightorigin, lightbottom);
		
				if (m_pCurrentEntity->curstate.effects & EF_INVLIGHT) 
					lightbottom[2] += 8196;
				else
					lightbottom[2] -= 8196;

				// Try and get bump data if possible
				if(m_pCvarUseBumpData->GetValue() >= 1.0)
				{
					gotLightmapLighting = Mod_RecursiveLightPoint_BumpData(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, &lightcolor, &lmapdiffusecolor, &lightdir, nullptr);
					if(gotLightmapLighting)
					{
						if(lightcolor.Length() < lmapdiffusecolor.Length())
						{
							gotBumpLighting = true;
							gotLighting = true;
						}
						else
						{
							// We sometimes have an odd case where we have no diffuse light
							// In this case, switch color values so it doesn't look unnatural
							gotLightmapLighting = false;
						}
					}
				}

				// If we didn't get bump data, use normal light data
				if(!gotLightmapLighting)
					gotLightmapLighting = Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], lighttop, lightbottom, &lightcolor);
			}

			if(gotlighting)
			{
				// Draw the line
				Vector drawtop, drawbottom;
				Math::VectorSubtract(lightorigin, Vector(0, 0, 8192), drawbottom);
				DrawLine(lightorigin, drawbottom, lightcolor);			
			
				Float offset = m_pCvarSampleOffset->GetValue();
				if(offset != 0)
				{
					if(offset <= 0)
						offset = DEFAULT_LIGHTMAP_SAMPLE_OFFSET;

					// Sample 1
					Float strengths[4] = { 0 };
					Vector offsetu = lighttop + Vector(-offset, -offset, 0);
					Vector offsetd = lightbottom + Vector(-offset, -offset, 0);
					drawtop = lightorigin + Vector(-offset, -offset, 0);
					drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(-offset, -offset, 0);
				
					Vector samplecolor;
					if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
						&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
					{
						offsetu = lightorigin + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						offsetd = lightbottom + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawtop = lightorigin + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					
						Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
					}

					strengths[0] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

					// Draw the line
					DrawLine(drawtop, drawbottom, samplecolor);

					// Sample 2
					offsetu = lighttop + Vector(offset, -offset, 0);
					offsetd = lightbottom + Vector(offset, -offset, 0);
					drawtop = lightorigin + Vector(offset, -offset, 0);
					drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(offset, -offset, 0);
				
					if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
						&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
					{
						offsetu = lightorigin + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						offsetd = lightbottom + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawtop = lightorigin + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, -DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					
						Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
					}

					strengths[1] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

					// Draw the line
					DrawLine(drawtop, drawbottom, samplecolor);

					// Sample 3
					offsetu = lighttop + Vector(offset, offset, 0);
					offsetd = lightbottom + Vector(offset, offset, 0);
					drawtop = lightorigin + Vector(offset, offset, 0);
					drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(offset, offset, 0);
				
					if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
						&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
					{
						offsetu = lightorigin + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						offsetd = lightbottom + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawtop = lightorigin + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
					
						Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
					}

					strengths[2] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

					// Draw the line
					DrawLine(drawtop, drawbottom, samplecolor);

					// Sample 4
					offsetu = lighttop + Vector(-offset, offset, 0);
					offsetd = lightbottom + Vector(-offset, offset, 0);
					drawtop = lightorigin + Vector(-offset, offset, 0);
					drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(-offset, offset, 0);
				
					if(!Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor)
						&& offset != DEFAULT_LIGHTMAP_SAMPLE_OFFSET)
					{
						offsetu = lightorigin + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						offsetd = lightbottom + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawtop = lightorigin + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);
						drawbottom = lightorigin - Vector(0, 0, 8192) + Vector(-DEFAULT_LIGHTMAP_SAMPLE_OFFSET, DEFAULT_LIGHTMAP_SAMPLE_OFFSET, 0);

						Mod_RecursiveLightPoint(pbrushmodel, &pbrushmodel->pnodes[pbrushmodel->hulls[0].firstclipnode], offsetu, offsetd, &samplecolor);
					}

					strengths[3] = (samplecolor.x + samplecolor.y + samplecolor.z) / 3.0f;

					// Draw the line
					DrawLine(drawtop, drawbottom, samplecolor);

					Float length = Math::DotProduct4(strengths, strengths);
					length = SDL_sqrt(length);

					if(length)
					{
						Float ilength = 1.0f/length;
						for(Uint32 i = 0; i < 4; i++)
							strengths[i] *= ilength;
					}

					// Calculate final result
					lightdir[0] = strengths[0] - strengths[1] - strengths[2] + strengths[3];
					lightdir[1] = strengths[1] + strengths[0] - strengths[2] - strengths[3];
					lightdir[2] = -1.0;

					Math::VectorNormalize(lightdir);
				}
			}
			else
			{
				// Default to basic lightdir
				lightdir = Vector(0, 0, -1);
			}

			// We got proper lighting
			gotLighting = true;
		}
	}

	// If we don't get a lighting info, just rely on skyvec
	if(!gotLighting)
	{
		Math::VectorScale(cls.skycolor, 1.0f/255.0f, lightcolor);
		Math::VectorCopy(cls.skyvec, lightdir);
		lightdir[2] = -lightdir[2];
	}

	// Draw the line
	Vector lightstart = m_pCurrentEntity->curstate.origin + lightdir * 16;
	Vector lightend = m_pCurrentEntity->curstate.origin - lightdir * 16;

	DrawLine(lightstart, lightend, lightcolor);

	glDisable(GL_BLEND);
	glLineWidth(1.0);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawAttachments( void )
{
	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);

	if(m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, true))
		return false;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glLineWidth(2.0);

	// Draw the points
	for(Int32 i = 0; i < m_pStudioHeader->numattachments; i++)
	{
		const mstudioattachment_t* pattachment = m_pStudioHeader->getAttachment(i);
		if(!pattachment)
			break;

		// Set bone
		Float matrix[3][4] = { 0 };
		matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;

		// Set bone transform to identity
		if(m_areUBOsSupported)
			m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
		else
			m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

		Vector color;
		color.x = RANDOM_COLOR_ARRAY[i%NUM_RANDOM_COLORS][0];
		color.y = RANDOM_COLOR_ARRAY[i%NUM_RANDOM_COLORS][1];
		color.z = RANDOM_COLOR_ARRAY[i%NUM_RANDOM_COLORS][2];

		// Set color
		m_pShader->SetUniform4f(m_attribs.u_color, color[0], color[1], color[2], 1.0);

		// Begin compiling the vertex data
		m_numTempVertexes = 0;

		Vector worldOrigin;
		for(Uint32 j = 0; j < 3; j++)
			worldOrigin[j] = (*m_pBoneTransform)[pattachment->bone].matrix[j][3];

		Vector attachmentOrigin;
		Math::VectorTransform(pattachment->org, (*m_pBoneTransform)[pattachment->bone].matrix, attachmentOrigin);

		Math::VectorCopy(worldOrigin, m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		Math::VectorCopy(attachmentOrigin, m_tempVertexes[m_numTempVertexes].origin);
		m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
		m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;
		m_tempVertexes[m_numTempVertexes].normal.Clear();
		for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
			m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;
		m_numTempVertexes++;

		m_pTempDrawVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

		// Draw the line
		m_pShader->DrawArrays(GL_LINES, m_drawBufferIndex, m_numTempVertexes);

		color.x = RANDOM_COLOR_ARRAY[(i+2)%NUM_RANDOM_COLORS][0];
		color.y = RANDOM_COLOR_ARRAY[(i+2)%NUM_RANDOM_COLORS][1];
		color.z = RANDOM_COLOR_ARRAY[(i+2)%NUM_RANDOM_COLORS][2];

		// Draw the point
		glPointSize(5);
		m_pShader->SetUniform4f(m_attribs.u_color, color[0], color[1], color[2], 1.0);

		m_pShader->DrawArrays(GL_POINTS, m_drawBufferIndex+1, 1);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	glPointSize(1);
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawHullBoundingBox( void )
{
	if( m_pCurrentEntity->curstate.effects & EF_CLIENTENT )
		return true;

	m_pShader->SetUniform1i(m_attribs.u_d_numlights, 0);
	m_pShader->SetUniform1i(m_attribs.u_d_bumpmapping, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_chrome, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_luminance, FALSE);
	m_pShader->SetUniform1i(m_attribs.u_d_specular, FALSE);

	if(m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) 
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, true))
		return false;

	glDepthMask(GL_FALSE);
	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Float matrix[3][4] = { 0 };
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;

	// Set bone transform to identity
	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
	else
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

	// Set color
	const Float* pcolor = RANDOM_COLOR_ARRAY[m_pCurrentEntity->entindex%NUM_RANDOM_COLORS];
	m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.25);

	Vector mins, maxs;
	Math::VectorAdd(m_pCurrentEntity->curstate.mins, m_pCurrentEntity->curstate.origin, mins);
	Math::VectorAdd(m_pCurrentEntity->curstate.maxs, m_pCurrentEntity->curstate.origin, maxs);

	DrawBox(mins, maxs);

	// Draw wireframe outline(do it like Q2 does, cause I like how it looks)
	glDisable(GL_BLEND);

	Vector vTemp;
	for (Uint32 i = 0; i < 8; i++)
	{
		if ( i & 1 ) 
			vTemp[0] = mins[0];
		else 
			vTemp[0] = maxs[0];

		if ( i & 2 ) 
			vTemp[1] = mins[1];
		else 
			vTemp[1] = maxs[1];

		if ( i & 4 ) 
			vTemp[2] = mins[2];
		else 
			vTemp[2] = maxs[2];

		Math::VectorCopy( vTemp, m_bboxCorners[i] );
	}

	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

	m_numTempVertexes = 0;

	Vector triverts[3];

	Uint32 i = 0;
	for(; i < 3; i++)
	{
		triverts[i] = m_bboxCorners[i];
		BatchVertex(triverts[i]);
	}

	for(; i < 8; i++)
	{
		triverts[0] = triverts[1];
		triverts[1] = triverts[2];
		triverts[2] = m_bboxCorners[i];

		for(Uint32 j = 0; j < 3; j++)
			BatchVertex(triverts[j]);
	}

	// Draw the planes
	m_pTempDrawVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	m_pShader->DrawArrays(GL_TRIANGLES, m_drawBufferIndex, m_numTempVertexes);

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::DrawBox( const Vector& bbmin, const Vector& bbmax )
{
	Vector bboxpoints[8];

	bboxpoints[0][0] = bbmin[0];
	bboxpoints[0][1] = bbmax[1];
	bboxpoints[0][2] = bbmin[2];

	bboxpoints[1][0] = bbmin[0];
	bboxpoints[1][1] = bbmin[1];
	bboxpoints[1][2] = bbmin[2];

	bboxpoints[2][0] = bbmax[0];
	bboxpoints[2][1] = bbmax[1];
	bboxpoints[2][2] = bbmin[2];

	bboxpoints[3][0] = bbmax[0];
	bboxpoints[3][1] = bbmin[1];
	bboxpoints[3][2] = bbmin[2];

	bboxpoints[4][0] = bbmax[0];
	bboxpoints[4][1] = bbmax[1];
	bboxpoints[4][2] = bbmax[2];

	bboxpoints[5][0] = bbmax[0];
	bboxpoints[5][1] = bbmin[1];
	bboxpoints[5][2] = bbmax[2];

	bboxpoints[6][0] = bbmin[0];
	bboxpoints[6][1] = bbmax[1];
	bboxpoints[6][2] = bbmax[2];

	bboxpoints[7][0] = bbmin[0];
	bboxpoints[7][1] = bbmin[1];
	bboxpoints[7][2] = bbmax[2];

	Vector triverts[3];

	m_numTempVertexes = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		// Remember triverts
		triverts[i] = bboxpoints[i&7];

		// Add to the draw list
		BatchVertex(bboxpoints[i]);
	}

	for(Uint32 i = 3; i < 10; i++)
	{
		triverts[0] = triverts[1];
		triverts[1] = triverts[2];
		triverts[2] = bboxpoints[i&7];

		for(Uint32 j = 0; j < 3; j++)
			BatchVertex(triverts[j]);
	}

	BatchVertex(bboxpoints[6]);
	BatchVertex(bboxpoints[0]);
	BatchVertex(bboxpoints[4]);
	BatchVertex(bboxpoints[0]);
	BatchVertex(bboxpoints[4]);
	BatchVertex(bboxpoints[2]);
	BatchVertex(bboxpoints[1]);
	BatchVertex(bboxpoints[7]);
	BatchVertex(bboxpoints[3]);
	BatchVertex(bboxpoints[7]);
	BatchVertex(bboxpoints[3]);
	BatchVertex(bboxpoints[5]);

	// Draw the planes
	m_pTempDrawVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	m_pShader->DrawArrays(GL_TRIANGLES, m_drawBufferIndex, m_numTempVertexes);
}

//=============================================
//
//
//=============================================
void CVBMRenderer::DrawLine( const Vector& start, const Vector& end, const Vector& color )
{
	m_pShader->SetUniform4f(m_attribs.u_color, color[0], color[1], color[2], 1.0);

	m_numTempVertexes = 0;
	BatchVertex(start);
	BatchVertex(end);

	m_pTempDrawVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

	R_ValidateShader(m_pShader);

	m_pShader->DrawArrays(GL_LINES, m_drawBufferIndex, m_numTempVertexes);
}

//=============================================
//
//
//=============================================
void CVBMRenderer::BatchVertex( const Vector& origin )
{
	if(m_numTempVertexes >= MAX_TEMP_VBM_VERTEXES)
		return;

	// Add to the draw list
	m_tempVertexes[m_numTempVertexes].origin = origin;
	m_tempVertexes[m_numTempVertexes].boneindexes[0] = 0;
	m_tempVertexes[m_numTempVertexes].boneweights[0] = 1.0;

	for(Uint32 j = 1; j < MAX_VBM_BONEWEIGHTS; j++)
		m_tempVertexes[m_numTempVertexes].boneweights[j] = 0;

	m_numTempVertexes++;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetShaderBoneTransform( BoneTransformArray_t* pbonetransform, const byte* pboneindexes, Uint32 numbones )
{
	if(m_areUBOsSupported)
	{
		for(Uint32 i = 0; i < numbones; i++)
			memcpy((void *)m_uboBoneMatrixData[i], (*pbonetransform)[pboneindexes[i]].matrix, sizeof(Float)*3*4);

		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, m_uboBoneMatrixData, numbones*3*4*sizeof(Float));
	}
	else
	{
		for(Int32 i = 0; i < numbones; i++)
			m_pShader->SetUniform4fv(m_attribs.boneindexes[i], (Float *)(*pbonetransform)[pboneindexes[i]].matrix, 3);
	}
}

//=============================================
//
//
//=============================================
const Char* CVBMRenderer::GetShaderErrorString( void ) const
{
	if(!m_pShader)
		return "";

	return m_pShader->GetError();
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::SetupEntityVertexLightVBO( cl_entity_t* pentity, Int32 vlightoffset, Uint32 vertexcount, byte* plightstyles )
{
	// Ensure this model actually exists
	cache_model_t* pmodel = gModelCache.GetModelByIndex(pentity->curstate.modelindex);
	if(!pmodel)
	{
		Con_Printf("%s - Failed to get model with index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
		return false;
	}

	Uint32 stylecount = 1;
	for(Uint32 i = 1; i < MAX_ENTITY_STYLES; i++)
	{
		if(plightstyles[i] != NULL_LIGHTSTYLE_INDEX)
			stylecount++;
	}

	vbmcache_t* pvbmcache = pmodel->getVBMCache();
	vbmheader_t* pvbmheader = pvbmcache->pvbmhdr;
	if(vertexcount != pvbmheader->numverts)
	{
		Con_Printf("%s - Vertex count between BSP vertex lighting data and VBM file '%s' doesn't match(BSP: %d, model: %d).\n", __FUNCTION__, pmodel->name.c_str(), vertexcount, pvbmheader->numverts);
		return false;
	}


	vlight_vbo_t* pnew = new vlight_vbo_t();
	pnew->pvbmcache = pvbmcache;
	pnew->stylecount = stylecount;
	pnew->vertexcount = pvbmheader->numverts;
	pnew->vlightoffset = vlightoffset;

	for(Uint32 i = 0; i < MAX_ENTITY_STYLES; i++)
		pnew->styles[i] = plightstyles[i];

	if(!BuildVertexLightVBO(pnew))
	{
		delete pnew;
		return false;
	}

	pentity->curstate.vlight_vbo_index = m_pVertexLightingVBOArray.size();
	m_pVertexLightingVBOArray.push_back(pnew);

	return true;
}

//=============================================
// @brief Creates the VBO for a vlibht_vbo_t entry
//
//=============================================
bool CVBMRenderer::BuildVertexLightVBO( vlight_vbo_t* pvlightvbo )
{
	// Get world model
	cache_model_t* pworld = gModelCache.GetModelByIndex(1);
	if(!pworld)
	{
		Con_Printf("%s - Failed to get world model.\n", __FUNCTION__);
		return false;
	}

	// Check if BSP has data
	brushmodel_t* pworldbrushmodel = pworld->getBrushmodel();
	if(!pworldbrushmodel->pvertexlightdata[VERTEX_LIGHTING_VECTORS]
		|| !pworldbrushmodel->pvertexlightdata[VERTEX_LIGHTING_AMBIENT]
		|| !pworldbrushmodel->pvertexlightdata[VERTEX_LIGHTING_DIFFUSE])
	{
		Con_Printf("%s - BSP has none, or incomplete vertex lighting data.\n", __FUNCTION__);
		return false;
	}

	// Do bounds check
	vbmheader_t* pvbmheader = pvlightvbo->pvbmcache->pvbmhdr;
	Int32 offsetdatastart = pvlightvbo->vlightoffset;
	Int32 offsetdataend = offsetdatastart + (pvbmheader->numverts * 3) * pvlightvbo->stylecount;
	if(offsetdatastart > pworldbrushmodel->vertexlightdatasize || offsetdataend > pworldbrushmodel->vertexlightdatasize)
	{
		Con_Printf("%s - Vertex data pointer out of bounds.\n", __FUNCTION__);
		return false;
	}

	// Create vertex data
	const byte* pvlight_vector = reinterpret_cast<const byte*>(pworldbrushmodel->pvertexlightdata[VERTEX_LIGHTING_VECTORS]) + offsetdatastart;
	const byte* pvlight_ambient = reinterpret_cast<const byte*>(pworldbrushmodel->pvertexlightdata[VERTEX_LIGHTING_AMBIENT]) + offsetdatastart;
	const byte* pvlight_diffuse = reinterpret_cast<const byte*>(pworldbrushmodel->pvertexlightdata[VERTEX_LIGHTING_DIFFUSE]) + offsetdatastart;

	Uint32 offsetindex = 0;
	vbm_vlight_glvertex_t* pvertexbuffer = new vbm_vlight_glvertex_t[pvbmheader->numverts];

	for(Uint32 i = 0; i < MAX_ENTITY_STYLES; i++)
	{
		if(i > BASE_LIGHTMAP_INDEX && pvlightvbo->styles[i] == NULL_LIGHTSTYLE_INDEX)
			continue;

		for(Uint32 j = 0; j < pvbmheader->numverts; j++)
		{
			byte* pdest_vector = nullptr;
			byte* pdest_ambient = nullptr;
			byte* pdest_diffuse = nullptr;
			switch(i)
			{
			case 1:
				pdest_vector = pvertexbuffer[j].vertexlight1_vector;
				pdest_ambient = pvertexbuffer[j].vertexlight1_ambient;
				pdest_diffuse = pvertexbuffer[j].vertexlight1_diffuse;
				break;
			case 2:
				pdest_vector = pvertexbuffer[j].vertexlight2_vector;
				pdest_ambient = pvertexbuffer[j].vertexlight2_ambient;
				pdest_diffuse = pvertexbuffer[j].vertexlight2_diffuse;
				break;
			case 3:
				pdest_vector = pvertexbuffer[j].vertexlight3_vector;
				pdest_ambient = pvertexbuffer[j].vertexlight3_ambient;
				pdest_diffuse = pvertexbuffer[j].vertexlight3_diffuse;
				break;
			default:
			case 0:
				pdest_vector = pvertexbuffer[j].vertexlight0_vector;
				pdest_ambient = pvertexbuffer[j].vertexlight0_ambient;
				pdest_diffuse = pvertexbuffer[j].vertexlight0_diffuse;
				break;
			}

			for(Uint32 k = 0; k < 3; k++)
				pdest_vector[k] = pvlight_vector[offsetindex * pvbmheader->numverts * 3 + j * 3 + k];

			for(Uint32 k = 0; k < 3; k++)
				pdest_ambient[k] = pvlight_ambient[offsetindex * pvbmheader->numverts * 3 + j * 3 + k];

			for(Uint32 k = 0; k < 3; k++)
				pdest_diffuse[k] = pvlight_diffuse[offsetindex * pvbmheader->numverts * 3 + j * 3 + k];
		}

		offsetindex++;
	}

	CVBO* pVBO = new CVBO(gGLExtF, pvertexbuffer, pvbmheader->numverts * sizeof(vbm_vlight_glvertex_t), nullptr, 0, false, false);
	delete[] pvertexbuffer;

	pvlightvbo->pvbo = pVBO;
	return true;
}

//=============================================
// @brief Finds the material by it's index - ugly hack
//
//=============================================
en_material_t* VBM_FindMaterialScriptByIndex( Int32 index )
{
	return CTextureManager::GetInstance()->FindMaterialScriptByIndex(index);
}
