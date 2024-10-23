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

// Notes:
// Part of this implementation is based on the implementation in the Half-Life SDK
// The studiomodel format is Valve's original work, and I take no ownership of it
// No copyright infringement intended
// AO mapping related code was done by valina354.


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
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_drawBufferIndex(0),
	m_pFlexTexture(nullptr),
	m_pScreenTexture(nullptr),
	m_pScreenFBO(nullptr),
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
	m_numModelLights(0),
	m_numDynamicLights(0),
	m_isMultiPass(false),
	m_isAuraPass(false),
	m_numDrawSubmodels(0),
	m_numVBMDecals(0),
	m_numTempIndexes(0),
	m_numTempVertexes(0),
	m_vCache_Index(0),
	m_vCache_Base(0),
	m_iCache_Index(0),
	m_iCache_Base(0),
	m_pFlexManager(nullptr)
{
	memset(m_pInternalRotationMatrix, 0, sizeof(m_pInternalRotationMatrix));
	memset(m_pInternalBoneTransform, 0, sizeof(m_pInternalBoneTransform));
	memset(m_pInternalWeightBoneTransform, 0, sizeof(m_pInternalWeightBoneTransform));
	memset(m_pDynamicLights, 0, sizeof(m_pDynamicLights));
	memset(m_pSubmodelDrawList, 0, sizeof(m_pSubmodelDrawList));
	memset(m_flexTexels, 0, sizeof(m_flexTexels));
	memset(m_uboBoneMatrixData, 0, sizeof(m_uboBoneMatrixData));
	memset(m_uboMatricesData, 0, sizeof(m_uboMatricesData));

	for(Uint32 i = 0; i < MAX_TEMP_VBM_INDEXES; i++)
		m_tempIndexes[i] = 0;

	for(Uint32 i = 0; i < MAXSTUDIOBONES; i++)
	{
		m_bonePositions1[i].Clear();

		for(Uint32 j = 0; j < 4; j++)
			m_boneQuaternions1[i][j] = 0;
	}

	for(Uint32 i = 0; i < MAXSTUDIOBONES; i++)
	{
		m_bonePositions2[i].Clear();

		for(Uint32 j = 0; j < 4; j++)
			m_boneQuaternions2[i][j] = 0;
	}

	for(Uint32 i = 0; i < MAXSTUDIOBONES; i++)
	{
		m_bonePositions3[i].Clear();

		for(Uint32 j = 0; j < 4; j++)
			m_boneQuaternions3[i][j] = 0;
	}

	for(Uint32 i = 0; i < MAXSTUDIOBONES; i++)
	{
		m_bonePositions4[i].Clear();

		for(Uint32 j = 0; j < 4; j++)
			m_boneQuaternions4[i][j] = 0;
	}

	for(Uint32 i = 0; i < MAXSTUDIOBONES; i++)
	{
		m_bonePositions5[i].Clear();

		for(Uint32 j = 0; j < 4; j++)
			m_boneQuaternions5[i][j] = 0;
	}

	for(Uint32 i = 0; i < 3; i++)
	{
		for(Uint32 j = 0; j < 4; j++)
			m_boneMatrix[i][j] = 0;
	}
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
	ClearGame();

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
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;
		else if(g_pCvarGLSLOnDemand->GetValue() > 0)
			shaderFlags |= CGLSLShader::FL_GLSL_ONDEMAND_LOAD;

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

		m_attribs.u_flextexture = m_pShader->InitUniform("flextexture", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_flextexturesize = m_pShader->InitUniform("flextexture_size", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_phong_exponent = m_pShader->InitUniform("phong_exponent", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_specularfactor = m_pShader->InitUniform("specfactor", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_causticsm1 = m_pShader->InitUniform("causticsm1", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_causticsm2 = m_pShader->InitUniform("causticsm2", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_scroll = m_pShader->InitUniform("scroll", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_texture0 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_texture1 = m_pShader->InitUniform("texture1", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_rectangle = m_pShader->InitUniform("rectangle", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_spectexture = m_pShader->InitUniform("spectexture", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_lumtexture = m_pShader->InitUniform("lumtexture", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_aotexture = m_pShader->InitUniform("aotex", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_normalmap = m_pShader->InitUniform("normalmap", CGLSLShader::UNIFORM_INT1);
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
			|| !R_CheckShaderUniform(m_attribs.u_aotexture, "aotex", m_pShader, Sys_ErrorPopup)
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
			|| !R_CheckShaderUniform(m_attribs.u_caustics_interp, "caust_interp", m_pShader, Sys_ErrorPopup))
			return false;

		if(!m_areUBOsSupported)
		{
			m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.u_normalmatrix = m_pShader->InitUniform("normalmatrix", CGLSLShader::UNIFORM_MATRIX4);

			if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.u_normalmatrix, "normalmatrix", m_pShader, Sys_ErrorPopup))
				return false;
		}
		else
		{
			m_attribs.ub_vsmatrices = m_pShader->InitUniformBufferObject("vs_matrices", sizeof(m_uboMatricesData));
			if(!R_CheckShaderUniform(m_attribs.ub_vsmatrices, "vs_matrices", m_pShader, Sys_ErrorPopup))
				return false;
		}

		m_attribs.d_numlights = m_pShader->GetDeterminatorIndex("num_lights");
		m_attribs.d_chrome = m_pShader->GetDeterminatorIndex("chrome");
		m_attribs.d_shadertype = m_pShader->GetDeterminatorIndex("shadertype");
		m_attribs.d_alphatest = m_pShader->GetDeterminatorIndex("alphatest");
		m_attribs.d_flexes = m_pShader->GetDeterminatorIndex("flex");
		m_attribs.d_specular = m_pShader->GetDeterminatorIndex("specular");
		m_attribs.d_luminance = m_pShader->GetDeterminatorIndex("luminance");
		m_attribs.d_bumpmapping = m_pShader->GetDeterminatorIndex("bumpmapping");
		m_attribs.d_numdlights = m_pShader->GetDeterminatorIndex("numdlights");
		m_attribs.d_use_ubo = m_pShader->GetDeterminatorIndex("use_ubo");

		if(!R_CheckShaderDeterminator(m_attribs.d_numlights, "num_lights", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_chrome, "chrome", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_shadertype, "shadertype", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_alphatest, "alphatest", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_flexes, "flex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_specular, "specular", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_luminance, "luminance", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_bumpmapping, "bumpmapping", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_use_ubo, "use_ubo", m_pShader, Sys_ErrorPopup))
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

			CString lightdeterminatorshadowmap;
			lightdeterminatorshadowmap << "dlight" << i << "_shadow";

			m_attribs.dlights[i].u_light_color = m_pShader->InitUniform(lightcolor.c_str(), CGLSLShader::UNIFORM_FLOAT4);
			m_attribs.dlights[i].u_light_origin = m_pShader->InitUniform(lightorigin.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			m_attribs.dlights[i].u_light_radius = m_pShader->InitUniform(lightradius.c_str(), CGLSLShader::UNIFORM_FLOAT1);
			m_attribs.dlights[i].u_light_cubemap = m_pShader->InitUniform(lightcubemap.c_str(), CGLSLShader::UNIFORM_INT1);
			m_attribs.dlights[i].u_light_projtexture = m_pShader->InitUniform(lightprojtexture.c_str(), CGLSLShader::UNIFORM_INT1);
			m_attribs.dlights[i].u_light_shadowmap = m_pShader->InitUniform(lightshadowmap.c_str(), CGLSLShader::UNIFORM_INT1);
			m_attribs.dlights[i].u_light_matrix = m_pShader->InitUniform(lightmatrix.c_str(), CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.dlights[i].d_light_shadowmap = m_pShader->GetDeterminatorIndex(lightdeterminatorshadowmap.c_str());

			if(!R_CheckShaderUniform(m_attribs.dlights[i].u_light_color, lightcolor.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_origin, lightorigin.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_radius, lightradius.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_cubemap, lightcubemap.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_projtexture, lightprojtexture.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_shadowmap, lightshadowmap.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.dlights[i].u_light_matrix, lightmatrix.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderDeterminator(m_attribs.dlights[i].d_light_shadowmap, lightdeterminatorshadowmap.c_str(), m_pShader, Sys_ErrorPopup))
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
		BuildVBO();
		CreateVertexTexture();
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

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}

	// Delete any decals
	DeleteDecals();
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

	BuildVBO();

	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::ClearGame( void )
{
	DeleteDecals();

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
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::SetupBones( Int32 flags )
{
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
	Float frame = VBM_EstimateFrame(pseqdesc, m_pCurrentEntity->curstate, rns.time);

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

	// Calculate bone matrices
	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(static_cast<Uint32>(i));

		Math::QuaternionMatrix(m_boneQuaternions1[i], m_boneMatrix);

		for(Uint32 j = 0; j < 3; j++)
			m_boneMatrix[j][3] = m_bonePositions1[i][j];

		if(pbone->parent == -1)
		{
			Math::ConcatTransforms((*m_pRotationMatrix), m_boneMatrix, (*m_pBoneTransform)[i]);
			ApplyRenderFX((*m_pBoneTransform)[i]);
		}
		else
		{
			Math::ConcatTransforms((*m_pBoneTransform)[pbone->parent], m_boneMatrix, (*m_pBoneTransform)[i]);
		}
	}

	// Rotate by the inverse bind pose
	if(m_pVBMHeader)
	{
		for(Int32 i = 0; i < m_pVBMHeader->numboneinfo; i++)
		{
			const vbmboneinfo_t* pvbmbone = m_pVBMHeader->getBoneInfo(i);
			Math::ConcatTransforms((*m_pBoneTransform)[i], pvbmbone->bindtransform, (*m_pWeightBoneTransform)[i]);
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
		m_pBoneTransform = (Float (*)[MAXSTUDIOBONES][3][4])m_pInternalBoneTransform;
		m_pWeightBoneTransform = (Float (*)[MAXSTUDIOBONES][3][4])m_pInternalWeightBoneTransform;
		m_pExtraInfo = nullptr;
	}
	else
	{
		m_pExtraInfo = CL_GetEntityExtraData(m_pCurrentEntity);
		m_pRotationMatrix = (Float (*)[3][4])m_pExtraInfo->paniminfo->rotation;
		m_pBoneTransform = (Float (*)[MAXSTUDIOBONES][3][4])m_pExtraInfo->paniminfo->bones;
		m_pWeightBoneTransform = (Float (*)[MAXSTUDIOBONES][3][4])m_pExtraInfo->paniminfo->weightbones;
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

	if (m_pCurrentEntity->curstate.sequence >=  m_pStudioHeader->numseq) 
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
	if(m_pExtraInfo->paniminfo->lastframe != VBM_EstimateFrame( pseqdesc, m_pCurrentEntity->curstate, rns.time ))
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
bool CVBMRenderer::SetModel( void )
{
	cache_model_t* pmodel = gModelCache.GetModelByIndex(m_pCurrentEntity->curstate.modelindex);
	if(!pmodel)
	{
		m_pCacheModel = nullptr;
		return false;
	}

	// Value depends on the loaded state
	m_pCacheModel = pmodel;
	return m_pCacheModel->isloaded ? true : false;
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
		if(!m_pCacheModel || !m_pCacheModel->pcachedata)
		{
			Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
			return true;
		}

		if(flags & VBM_RENDER)
		{
			EndDraw();

			// Clear shader ptr
			m_pShader->SetVBO(nullptr);
			m_pShader->ResetShader();
		}

		if(m_pVBO)
		{
			delete m_pVBO;
			m_pVBO = nullptr;
		}
		
		BuildVBO();

		// Re-set shader
		if(flags & VBM_RENDER)
		{
			m_pShader->SetVBO(m_pVBO);
			if(!PrepareDraw())
				return false;
		}
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

		// See if we're using any scope textures
		if(!(flags & VBM_SETUPBONES) && m_pCvarDrawModels->GetValue() >= 1)
		{
			Int32 i = 0;
			for(; i < m_pVBMHeader->numtextures; i++)
			{
				const vbmtexture_t* ptexture = m_pVBMHeader->getTexture(i);
				en_material_t* pmaterial = CTextureManager::GetInstance()->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if(pmaterial->flags & TX_FL_SCOPE)
					break;
			}

			if(i != m_pVBMHeader->numtextures)
			{
				// Grab the screen texture
				m_pScreenTexture = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);
				R_BindRectangleTexture(GL_TEXTURE0_ARB, m_pScreenTexture->palloc->gl_index, true);
				glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);
				R_BindRectangleTexture(GL_TEXTURE0_ARB, 0);
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
		SetupLighting();

		if(m_pStudioHeader->flags & STUDIO_MF_HAS_FLEXES)
			m_pFlexManager->UpdateValues( rns.time, m_pCurrentEntity->curstate.health, m_pCurrentEntity->mouth.mouthopen, m_pExtraInfo->pflexstate, false );

		// Draw the model
		result = Render();
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
		Math::VectorTransform(pattachment->org, (*m_pBoneTransform)[pattachment->bone], m_pCurrentEntity->attachments[i]);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::AddVBM( studiohdr_t *phdr, vbmheader_t *pvbm )
{
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

	// Set the offsets
	pvbm->ibooffset = m_pVBO->GetIBOSize()/sizeof(Uint32);
	pvbm->vbooffset = m_pVBO->GetVBOSize()/sizeof(vbm_glvertex_t);

	Uint32 *pvboindexes = new Uint32[pvbm->numindexes];
	const Uint32 *pvbmindexes = pvbm->getIndexes();

	for(Int32 j = 0; j < pvbm->numindexes; j++)
		pvboindexes[j] = pvbmindexes[j]+pvbm->vbooffset;

	m_pVBO->Append(pvboverts, sizeof(vbm_glvertex_t)*pvbm->numverts, pvboindexes, sizeof(Uint32)*pvbm->numindexes);
	delete[] pvboindexes;
	delete[] pvboverts;

	// set up textures
	CString modelname;
	Common::Basename(pvbm->name, modelname);
	modelname.tolower();

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for(Int32 i = 0; i < pvbm->numtextures; i++)
	{
		vbmtexture_t *ptexture = pvbm->getTexture(static_cast<Uint32>(i));

		CString textureName;
		Common::Basename(ptexture->name, textureName); 
		textureName.tolower();

		// Create and assign the group
		CString materialscriptpath;
		materialscriptpath << MODEL_MATERIALS_BASE_PATH << modelname << PATH_SLASH_CHAR << textureName.c_str() << PMF_FORMAT_EXTENSION;

		// Retreive material name
		en_material_t* pmaterial = pTextureManager->LoadMaterialScript(materialscriptpath.c_str(), RS_GAME_LEVEL);
		if(!pmaterial)
			pmaterial = pTextureManager->GetDummyMaterial();

		if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND))
			ptexture->flags |= FL_VBM_TEXTURE_BLEND;

		if(!pmaterial->containername.empty())
		{
			Con_EPrintf("%s - Container name specified for non-world material script '%s'. Texture will be null.\n", __FUNCTION__, materialscriptpath.c_str());
			pmaterial->ptextures[MT_TX_DIFFUSE] = pTextureManager->GetDummyTexture();
		}

		ptexture->index = pmaterial->index;
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
void CVBMRenderer::BuildVBO( void )
{
	if(ens.isloading)
		VID_DrawLoadingScreen("Loading VBM geometry and textures");

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}

	m_pVBO = new CVBO(gGLExtF, true, true);
	m_pShader->SetVBO(m_pVBO);

	for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
	{
		cache_model_t *pmodel = gModelCache.GetModelByIndex((i+1));
			
		if(!pmodel)
			break;

		if(pmodel->type != MOD_VBM)
			continue;

		const vbmcache_t* pcache = pmodel->getVBMCache();
		if(!pcache->pstudiohdr || !pcache->pvbmhdr)
			continue;

		if(!pcache->pvbmhdr->numbodyparts)
			continue;

		AddVBM(pcache->pstudiohdr, pcache->pvbmhdr); 

		// Mark as loaded into GL
		pmodel->isloaded = true;
	}

	// Create base for draw buffer
	m_drawBufferIndex = m_pVBO->GetVBOSize()/sizeof(vbm_glvertex_t);
	vbm_glvertex_t* ptempverts = new vbm_glvertex_t[MAX_TEMP_VBM_VERTEXES];
	m_pVBO->Append(ptempverts, sizeof(vbm_glvertex_t)*MAX_TEMP_VBM_VERTEXES, nullptr, 0);
	delete[] ptempverts;

	// Add in reserve
	m_vCache_Base = m_pVBO->GetVBOSize()/sizeof(vbm_glvertex_t);
	m_vCache_Index = m_vCache_Base;

	m_iCache_Base = m_pVBO->GetIBOSize()/sizeof(Uint32);
	m_iCache_Index = m_iCache_Base;
	
	// Get the cache size
	m_decalVertexCacheSize = m_pCvarDecalCacheSize->GetValue();
	if(m_decalVertexCacheSize < MIN_VBMDECAL_VERTEXES)
	{
		Con_Printf("Warning: value '%d' too low on cvar '%s', resetting to %d.\n", m_decalVertexCacheSize, m_pCvarDecalCacheSize->GetName(), static_cast<Int32>(MIN_VBMDECAL_VERTEXES));
		gConsole.CVarSetFloatValue(m_pCvarDecalCacheSize->GetName(), MIN_VBMDECAL_VERTEXES);
		m_decalVertexCacheSize = MIN_VBMDECAL_VERTEXES;
	}

	// Count 3 indexes for each vertex
	m_decalIndexCacheSize = m_decalVertexCacheSize*3;

	// Allocate the blanks
	vbm_glvertex_t *pblankverts = new vbm_glvertex_t[m_decalVertexCacheSize];
	Uint32 *pblankindexes = new Uint32[m_decalIndexCacheSize];

	// Append it to the VBO
	m_pVBO->Append(pblankverts, sizeof(vbm_glvertex_t)*m_decalVertexCacheSize, pblankindexes, sizeof(Uint32)*m_decalIndexCacheSize);
	m_pVBO->DeleteCaches();

	delete[] pblankverts;
	delete[] pblankindexes;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::UpdateLightValues ( void )
{
	if(!m_pExtraInfo)
		return;

	if(!m_pExtraInfo->plightinfo->lighttime
		|| m_pExtraInfo->plightinfo->lighttime == -1)
	{
		// Add in any lightstyle crap to local every frame, as
		// the lightstyle values can change on the fly
		for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
		{
			if(m_lightingInfo.lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
				continue;

			Float lightstylevalue = gLightStyles.GetLightStyleValue(m_lightingInfo.lightstyles[i]);
			Math::VectorMA(m_lightingInfo.ambient_color, lightstylevalue, m_lightingInfo.lightstylecolors_ambient[i], m_lightingInfo.ambient_color);
			Math::VectorMA(m_lightingInfo.direct_color, lightstylevalue, m_lightingInfo.lightstylecolors_diffuse[i], m_lightingInfo.direct_color);
		}

		if(m_pExtraInfo->plightinfo->lighttime != -1)
			m_pExtraInfo->plightinfo->lighttime = -1;

		return;
	}

	// If blend time expired, then set the final values
	Float lightfulltime = m_lightingInfo.lighttime + LIGHTING_LERP_TIME;
	if(lightfulltime < rns.time)
	{
		// Set final values
		Math::VectorCopy(m_lightingInfo.target_ambient, m_lightingInfo.ambient_color);
		Math::VectorCopy(m_lightingInfo.target_diffuse, m_lightingInfo.direct_color);
		Math::VectorCopy(m_lightingInfo.target_lightdir, m_lightingInfo.lightdirection);

		// Set final lightstyle values as well
		for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
		{
			if(m_lightingInfo.target_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
			{
				m_lightingInfo.lightstyles[i] = NULL_LIGHTSTYLE_INDEX;
				continue;
			}

			Math::VectorCopy(m_lightingInfo.target_stylecolors_ambient[i], m_lightingInfo.lightstylecolors_ambient[i]);
			Math::VectorCopy(m_lightingInfo.target_stylecolors_diffuse[i], m_lightingInfo.lightstylecolors_diffuse[i]);
			m_lightingInfo.lightstyles[i] = m_lightingInfo.target_lightstyles[i];
		}

		// Set this to signal that we don't need to do this again
		m_lightingInfo.lighttime = -1;
		// Fix: Set in global array as well
		(*m_pExtraInfo->plightinfo) = m_lightingInfo;

		// Add in any lightstyle crap to local after we're done setting everything
		for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
		{
			if(m_lightingInfo.lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
				continue;

			Float lightstylevalue = gLightStyles.GetLightStyleValue(m_lightingInfo.lightstyles[i]);
			Math::VectorMA(m_lightingInfo.ambient_color, lightstylevalue, m_lightingInfo.lightstylecolors_ambient[i], m_lightingInfo.ambient_color);
			Math::VectorMA(m_lightingInfo.direct_color, lightstylevalue, m_lightingInfo.lightstylecolors_diffuse[i], m_lightingInfo.direct_color);
		}

		return;
	}

	Double lighttime = rns.time - m_lightingInfo.lighttime;
	Double lightfrac = lighttime / LIGHTING_LERP_TIME;

	Vector tmp;
	Math::VectorScale(m_lightingInfo.prev_ambient, (1.0 - lightfrac), tmp);
	Math::VectorMA(tmp, lightfrac, m_lightingInfo.target_ambient, m_lightingInfo.ambient_color);

	Math::VectorScale(m_lightingInfo.prev_diffuse, (1.0 - lightfrac), tmp);
	Math::VectorMA(tmp, lightfrac, m_lightingInfo.target_diffuse, m_lightingInfo.direct_color);

	Math::VectorScale(m_lightingInfo.prev_lightdir, (1.0 - lightfrac), tmp);
	Math::VectorMA(tmp, lightfrac, m_lightingInfo.target_lightdir, m_lightingInfo.lightdirection);

	// Blend in previous style colors
	Vector lightstylecolors_ambient[MAX_SURFACE_STYLES-1];
	Vector lightstylecolors_diffuse[MAX_SURFACE_STYLES-1];

	// Lightstyle values array
	CArray<Float>* pstylevalues = gLightStyles.GetLightStyleValuesArray();

	for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
	{
		if(m_lightingInfo.prev_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
			break;

		// Fetch lightstyle value
		Float stylevalue = (*pstylevalues)[m_lightingInfo.prev_lightstyles[i]];

		// Handle ambient
		Math::VectorScale(m_lightingInfo.prev_stylecolors_ambient[i], (1.0 - lightfrac), tmp);
		Math::VectorScale(tmp, stylevalue, lightstylecolors_ambient[i]);

		// Handle diffuse
		Math::VectorScale(m_lightingInfo.prev_stylecolors_diffuse[i], (1.0 - lightfrac), tmp);
		Math::VectorScale(tmp, stylevalue, lightstylecolors_diffuse[i]);
	}

	// Blend in current style colors
	for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
	{
		if(m_lightingInfo.target_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
			break;

		// Fetch lightstyle value
		Float stylevalue = (*pstylevalues)[m_lightingInfo.target_lightstyles[i]];

		// Handle ambient
		Math::VectorScale(m_lightingInfo.target_stylecolors_diffuse[i], lightfrac, tmp);
		Math::VectorMA(lightstylecolors_diffuse[i], stylevalue, tmp, m_lightingInfo.lightstylecolors_diffuse[i]);

		// Handle diffuse
		Math::VectorScale(m_lightingInfo.target_stylecolors_ambient[i], lightfrac, tmp);
		Math::VectorMA(lightstylecolors_ambient[i], stylevalue, tmp, m_lightingInfo.lightstylecolors_ambient[i]);
	}

	// Copy these to the global array as well
	Math::VectorCopy(m_lightingInfo.ambient_color, m_pExtraInfo->plightinfo->ambient_color);
	Math::VectorCopy(m_lightingInfo.direct_color, m_pExtraInfo->plightinfo->direct_color);
	Math::VectorCopy(m_lightingInfo.lightdirection, m_pExtraInfo->plightinfo->lightdirection);

	// Add lightstyle values into local variables
	for(Uint32 i = 0; i < (MAX_SURFACE_STYLES-1); i++)
	{
		if(m_lightingInfo.target_lightstyles[i] == NULL_LIGHTSTYLE_INDEX)
			break;

		Math::VectorAdd(m_lightingInfo.ambient_color, m_lightingInfo.lightstylecolors_ambient[i], m_lightingInfo.ambient_color);
		Math::VectorAdd(m_lightingInfo.direct_color, m_lightingInfo.lightstylecolors_diffuse[i], m_lightingInfo.direct_color);
	}
}

//=============================================
//
//
//=============================================
void CVBMRenderer::SetupLighting ( void )
{
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

	if( m_pExtraInfo )
	{
		// Copy previous values
		m_lightingInfo = *m_pExtraInfo->plightinfo;

		// If we did not change, just keep blending values
		if(!m_pExtraInfo->plightinfo->reset && Math::VectorCompare(m_pExtraInfo->plightinfo->lastlightorigin, lightorigin))
		{
			UpdateLightValues();
			return;
		}
	}

	// Get sky light info
	bool gotLighting = false;
	bool gotBumpLighting = false;
	bool gotLightmapLighting = false;

	Vector surfnormal;
	byte lightstyles[MAX_SURFACE_STYLES];
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
		if(ens.pworld->plightdata[SURF_LIGHTMAP_DEFAULT] && !(m_pStudioHeader->flags & STUDIO_MF_SKYLIGHT))
		{
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
							if(lightcolors[SURF_LIGHTMAP_DEFAULT].Length() < lmapdiffusecolors[SURF_LIGHTMAP_DEFAULT].Length())
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
						if(lightcolors[SURF_LIGHTMAP_DEFAULT].Length() < lmapdiffusecolors[SURF_LIGHTMAP_DEFAULT].Length())
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
					Float strengths[4];
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

		for(Uint32 j = 0; j < MAX_SURFACE_STYLES; j++)
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
	if(m_numModelLights > 0 || m_numDynamicLights > 0)
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
	if(m_pExtraInfo && !(m_lightingInfo.flags & MDL_LIGHT_NOBLEND) && !m_pExtraInfo->plightinfo->reset
		&& !CompareLightValues(ambientcolors, diffusecolors, lightdir, m_lightingInfo))
	{
		if(m_lightingInfo.lighttime != 0)
		{
			// Set current light values as the previous value to blend from
			Math::VectorCopy(m_lightingInfo.ambient_color, m_lightingInfo.prev_ambient);
			Math::VectorCopy(m_lightingInfo.direct_color, m_lightingInfo.prev_diffuse);
			Math::VectorCopy(m_lightingInfo.lightdirection, m_lightingInfo.prev_lightdir);

			// Set lightstyles too(this isn't 100% correct, but fuck it, 
			// the solution would be extremely overcomplicated)
			if(m_lightingInfo.lighttime != -1)
			{
				Double lighttime = rns.time - m_lightingInfo.lighttime;
				Double lightfrac = lighttime / LIGHTING_LERP_TIME;

				if(lightfrac > 0.5)
				{
					for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
					{
						if(m_lightingInfo.lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
						{
							Math::VectorCopy(m_lightingInfo.target_stylecolors_ambient[i], m_lightingInfo.prev_stylecolors_ambient[i]);
							Math::VectorCopy(m_lightingInfo.target_stylecolors_diffuse[i], m_lightingInfo.prev_stylecolors_diffuse[i]);
						}

						m_lightingInfo.prev_lightstyles[i] = m_lightingInfo.target_lightstyles[i];
					}
				}
			}
			else
			{
				for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
				{
					if(m_lightingInfo.lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
					{
						Math::VectorCopy(m_lightingInfo.lightstylecolors_ambient[i], m_lightingInfo.prev_stylecolors_ambient[i]);
						Math::VectorCopy(m_lightingInfo.lightstylecolors_diffuse[i], m_lightingInfo.prev_stylecolors_diffuse[i]);
					}

					m_lightingInfo.prev_lightstyles[i] = m_lightingInfo.lightstyles[i];
				}
			}
		}
		else
		{
			// Set the previous state to the same, because this is the first time we're doing this
			Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.prev_ambient);
			Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.prev_diffuse);
			Math::VectorCopy(lightdir, m_lightingInfo.prev_lightdir);

			for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
			{
				if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
				{
					Math::VectorCopy(ambientcolors[i+1], m_lightingInfo.prev_stylecolors_ambient[i]);
					Math::VectorCopy(diffusecolors[i+1], m_lightingInfo.prev_stylecolors_diffuse[i]);
				}

				m_lightingInfo.prev_lightstyles[i] = lightstyles[i+1];
			}
		}

		// Set target
		Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.target_ambient);
		Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.target_diffuse);
		Math::VectorCopy(lightdir, m_lightingInfo.target_lightdir);

		for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
		{
			if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
			{
				Math::VectorCopy(ambientcolors[i+1], m_lightingInfo.target_stylecolors_ambient[i]);
				Math::VectorCopy(diffusecolors[i+1], m_lightingInfo.target_stylecolors_diffuse[i]);
			}

			m_lightingInfo.target_lightstyles[i] = lightstyles[i+1];
		}

		if(!rns.time)
		{
			// In this case this was cleared, or we're on the first frame
			Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.ambient_color);
			Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.direct_color);
			Math::VectorCopy(lightdir, m_lightingInfo.lightdirection);

			for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
			{
				if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
				{
					Math::VectorCopy(ambientcolors[i+1], m_lightingInfo.lightstylecolors_ambient[i]);
					Math::VectorCopy(diffusecolors[i+1], m_lightingInfo.lightstylecolors_diffuse[i]);
				}

				m_lightingInfo.lightstyles[i] = lightstyles[i+1];
			}
		}
		else
		{
			// Set time because target light values changed
			m_lightingInfo.lighttime = rns.time;
		}
	}
	else if(!m_pExtraInfo && !CompareLightValues(ambientcolors, diffusecolors, lightdir, m_lightingInfo)
		|| m_pExtraInfo && m_pExtraInfo->plightinfo->reset)
	{
		Math::VectorCopy(ambientcolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.ambient_color);
		Math::VectorCopy(diffusecolors[BASE_LIGHTMAP_INDEX], m_lightingInfo.direct_color);
		Math::VectorCopy(lightdir, m_lightingInfo.lightdirection);

		for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
		{
			if(lightstyles[i+1] != NULL_LIGHTSTYLE_INDEX)
			{
				Math::VectorCopy(ambientcolors[i+1], m_lightingInfo.lightstylecolors_ambient[i]);
				Math::VectorCopy(diffusecolors[i+1], m_lightingInfo.lightstylecolors_diffuse[i]);
			}

			m_lightingInfo.lightstyles[i] = lightstyles[i+1];
		}

		m_lightingInfo.lighttime = 0;
	}

	// Save all the info for next time
	if(m_pExtraInfo)
	{
		// Save basics
		Math::VectorCopy(m_lightingInfo.ambient_color, m_pExtraInfo->plightinfo->ambient_color);
		Math::VectorCopy(m_lightingInfo.direct_color, m_pExtraInfo->plightinfo->direct_color);
		Math::VectorCopy(m_lightingInfo.lightdirection, m_pExtraInfo->plightinfo->lightdirection);

		Math::VectorCopy(m_lightingInfo.target_ambient, m_pExtraInfo->plightinfo->target_ambient);
		Math::VectorCopy(m_lightingInfo.prev_ambient, m_pExtraInfo->plightinfo->prev_ambient);

		Math::VectorCopy(m_lightingInfo.target_diffuse, m_pExtraInfo->plightinfo->target_diffuse);
		Math::VectorCopy(m_lightingInfo.prev_diffuse, m_pExtraInfo->plightinfo->prev_diffuse);

		Math::VectorCopy(m_lightingInfo.target_lightdir, m_pExtraInfo->plightinfo->target_lightdir);
		Math::VectorCopy(m_lightingInfo.prev_lightdir, m_pExtraInfo->plightinfo->prev_lightdir);

		Math::VectorCopy(saved_lightorigin, m_pExtraInfo->plightinfo->lastlightorigin);

		for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
		{
			// Current style
			if(m_lightingInfo.lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
			{
				Math::VectorCopy(m_lightingInfo.lightstylecolors_ambient[i], m_pExtraInfo->plightinfo->lightstylecolors_ambient[i]);
				Math::VectorCopy(m_lightingInfo.lightstylecolors_diffuse[i], m_pExtraInfo->plightinfo->lightstylecolors_diffuse[i]);
			}
			m_pExtraInfo->plightinfo->lightstyles[i] = m_lightingInfo.lightstyles[i];

			// Previous style
			if(m_lightingInfo.prev_lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
			{
				Math::VectorCopy(m_lightingInfo.prev_stylecolors_ambient[i], m_pExtraInfo->plightinfo->prev_stylecolors_ambient[i]);
				Math::VectorCopy(m_lightingInfo.target_stylecolors_ambient[i], m_pExtraInfo->plightinfo->target_stylecolors_ambient[i]);
			}
			m_pExtraInfo->plightinfo->prev_lightstyles[i] = m_lightingInfo.prev_lightstyles[i];

			// Target style
			if(m_lightingInfo.target_lightstyles[i] != NULL_LIGHTSTYLE_INDEX)
			{
				Math::VectorCopy(m_lightingInfo.prev_stylecolors_diffuse[i], m_pExtraInfo->plightinfo->prev_stylecolors_diffuse[i]);
				Math::VectorCopy(m_lightingInfo.target_stylecolors_diffuse[i], m_pExtraInfo->plightinfo->target_stylecolors_diffuse[i]);
			}
			m_pExtraInfo->plightinfo->target_lightstyles[i] = m_lightingInfo.target_lightstyles[i];
		}

		// Clear any reset state
		if(m_pExtraInfo->plightinfo->reset)
			m_pExtraInfo->plightinfo->reset = false;

		// Set lighting time also
		m_pExtraInfo->plightinfo->lighttime = m_lightingInfo.lighttime;

		// Call this so render values are set
		UpdateLightValues();
	}
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::CompareLightValues( Vector* pambientlightvalues, Vector* pdiffuselightvalues, const Vector& lightdir, entity_lightinfo_t& lightinfo )
{
	if(!Math::VectorCompare(lightdir, m_lightingInfo.target_lightdir))
		return false;

	if(!Math::VectorCompare(pambientlightvalues[BASE_LIGHTMAP_INDEX], m_lightingInfo.target_ambient))
		return false;

	if(!Math::VectorCompare(pdiffuselightvalues[BASE_LIGHTMAP_INDEX], m_lightingInfo.target_diffuse))
		return false;

	for(Uint32 i = 0; i < MAX_SURFACE_STYLES-1; i++)
	{
		if(!Math::VectorCompare(pambientlightvalues[i+1], m_lightingInfo.target_stylecolors_ambient[i]))
			return false;

		if(!Math::VectorCompare(pdiffuselightvalues[i+1], m_lightingInfo.target_stylecolors_diffuse[i]))
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
	mlight_t* plightslist[MAX_ENT_ACTIVE_DLIGHTS];
	Float lightstrengths[MAX_ENT_ACTIVE_DLIGHTS];

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

		// Build mins/maxs
		Vector mins, maxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = pdl->origin[i] - pdl->radius;
			maxs[i] = pdl->origin[i] + pdl->radius;
		}

		// This needs to be set every time(also, be sure to use the viewsize from view_params!)
		if(!DL_IsLightVisible(rns.view.frustum, mins, maxs, pdl))
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
			Vector vmins, vmaxs;
			for(Uint32 i = 0; i < 3; i++)
			{
				vmins[i] = pdl->origin[i]-pdl->radius;
				vmaxs[i] = pdl->origin[i]+pdl->radius;
			}

			if(Math::CheckMinsMaxs(vmins, vmaxs, m_mins, m_maxs))
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
	if (m_pCurrentEntity->curstate.sequence >=  m_pStudioHeader->numseq) 
		m_pCurrentEntity->curstate.sequence = 0;

	// Build full bounding box
	const mstudioseqdesc_t *pseqdesc = m_pStudioHeader->getSequence(m_pCurrentEntity->curstate.sequence);

	Vector vTemp;
	static Vector vBounds[8];
	for (Uint32 i = 0; i < 8; i++)
	{
		if ( i & 1 ) vTemp[0] = pseqdesc->bbmin[0];
		else vTemp[0] = pseqdesc->bbmax[0];
		if ( i & 2 ) vTemp[1] = pseqdesc->bbmin[1];
		else vTemp[1] = pseqdesc->bbmax[1];
		if ( i & 4 ) vTemp[2] = pseqdesc->bbmin[2];
		else vTemp[2] = pseqdesc->bbmax[2];
		Math::VectorCopy( vTemp, vBounds[i] );
	}

	Vector angles = m_pCurrentEntity->curstate.angles;
	angles[PITCH] = -angles[PITCH];

	Math::AngleMatrix(angles, (*m_pRotationMatrix));

	for (Uint32 i = 0; i < 8; i++ )
	{
		Math::VectorCopy(vBounds[i], vTemp);
		Math::VectorRotate(vTemp, (*m_pRotationMatrix), vBounds[i]);
	}

	// Set the bounding box
	Vector vMins = NULL_MINS;
	Vector vMaxs = NULL_MAXS;
	for(Uint32 i = 0; i < 8; i++)
	{
		// Mins
		if(vBounds[i][0] < vMins[0]) vMins[0] = vBounds[i][0];
		if(vBounds[i][1] < vMins[1]) vMins[1] = vBounds[i][1];
		if(vBounds[i][2] < vMins[2]) vMins[2] = vBounds[i][2];

		// Maxs
		if(vBounds[i][0] > vMaxs[0]) vMaxs[0] = vBounds[i][0];
		if(vBounds[i][1] > vMaxs[1]) vMaxs[1] = vBounds[i][1];
		if(vBounds[i][2] > vMaxs[2]) vMaxs[2] = vBounds[i][2];
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
bool CVBMRenderer::Render( void )
{
	m_renderAlpha = 1.0;
	m_useBlending = false;
	m_isMultiPass = false;

	// Set transparency
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);

	if ( m_pCurrentEntity->curstate.rendermode == RENDER_TRANSADDITIVE )
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		m_renderAlpha = R_RenderFxBlend(m_pCurrentEntity) / 255.0;
		m_useBlending = true;
	}

	if ( (m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK) == RENDER_TRANSALPHA 
		|| (m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK) == RENDER_TRANSTEXTURE )
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_renderAlpha = R_RenderFxBlend(m_pCurrentEntity) / 255.0;
		m_useBlending = true;
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

	if(!DrawDecals())
		return false;

	if(!DrawWireframe())
		return false;

	if(m_pCvarDrawModels->GetValue() == 2)
	{
		if(!DrawBones())
			return false;
	}

	if(m_pCvarDrawModels->GetValue() == 3)
	{
		if(!DrawHitBoxes())
			return false;
	}

	if(m_pCvarDrawModels->GetValue() == 4)
	{
		if(!DrawBoundingBox())
			return false;
	}

	if(m_pCvarDrawModels->GetValue() == 5)
	{
		if(!DrawHullBoundingBox())
			return false;
	}

	if(m_pCvarDrawModels->GetValue() == 6)
	{
		if(!DrawLightVectors())
			return false;
	}

	if(m_pCvarDrawModels->GetValue() == 7)
	{
		if(!DrawAttachments())
			return false;
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
	if(rns.inwater && g_pCvarCaustics->GetValue() >= 1 
		&& gWaterShader.GetWaterQualitySetting() > CWaterShader::WATER_QUALITY_NO_REFLECT_REFRACT)
	{
		const water_settings_t* pwatersettings = gWaterShader.GetActiveSettings();
		if(pwatersettings && pwatersettings->causticscale > 0 && pwatersettings->causticstrength > 0)
			m_isMultiPass = true;
	}
	
	// If water caustics did not add to multipass, then check for lights
	if(!m_isMultiPass && !R_IsEntityTransparent(*m_pCurrentEntity, true) 
		&& (m_numDynamicLights != 0 && g_pCvarDynamicLights->GetValue() >= 1))
			m_isMultiPass = true;

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
	Math::MatMult(pmatrix.Transpose(), m_lightingInfo.lightdirection, &vtransformed);

	m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_ambient, m_lightingInfo.ambient_color[0], m_lightingInfo.ambient_color[1], m_lightingInfo.ambient_color[2]);
	m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, m_lightingInfo.direct_color[0], m_lightingInfo.direct_color[1], m_lightingInfo.direct_color[2]);

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

	if(!m_pShader->SetDeterminator(m_attribs.d_chrome, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, m_numModelLights, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false))
		return false;

	m_pShader->EnableAttribute(m_attribs.a_normal);
	m_pShader->EnableAttribute(m_attribs.a_tangent);
	m_pShader->EnableAttribute(m_attribs.a_texcoord1);

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
	}

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->EnableSync(m_attribs.u_normalmatrix);
	}

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
	if(!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false))
		return false;

	m_pShader->DisableAttribute(m_attribs.a_flexcoord);
	m_pShader->DisableAttribute(m_attribs.a_texcoord2);
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
	m_pVBO->VBOSubBufferData((m_pVBMHeader->vbooffset+pflexinfo->first_vertex)*sizeof(vbm_glvertex_t), m_tempVertexes, pflexinfo->num_vertexes*sizeof(vbm_glvertex_t));
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
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha);
				}
				else if(pmaterial->flags & TX_FL_ALPHABLEND)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, m_renderAlpha*pmaterial->alpha);
				}

				if ( rns.fog.settings.active )
					m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.0, 0.0, 0.0);

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
bool CVBMRenderer::DrawMesh( en_material_t *pmaterial, const vbmmesh_t *pmesh, bool drawBlended )
{
	// Set the determinator states
	if(!m_pShader->SetDeterminator(m_attribs.d_chrome, (!m_isMultiPass && (pmaterial->flags & (TX_FL_CHROME) || pmaterial->flags & (TX_FL_EYEGLINT) && drawBlended)) ? TRUE : FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, (pmaterial->flags & (TX_FL_FULLBRIGHT|TX_FL_SCOPE)) ? 0 : m_numModelLights, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, (pmaterial->ptextures[MT_TX_SPECULAR]) && !(pmaterial->flags & TX_FL_FULLBRIGHT) && !m_isMultiPass && g_pCvarSpecular->GetValue() > 0 ? true : false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, (pmaterial->ptextures[MT_TX_LUMINANCE]) && !(pmaterial->flags & TX_FL_FULLBRIGHT), false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, (pmaterial->ptextures[MT_TX_AO]) && !(pmaterial->flags & TX_FL_FULLBRIGHT), false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, (pmaterial->ptextures[MT_TX_NORMALMAP]) && !(pmaterial->flags & TX_FL_FULLBRIGHT) && g_pCvarBumpMaps->GetValue() > 0, false))
	return false;

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
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_scope, false);
	}
	else if(pmaterial->flags & TX_FL_FULLBRIGHT)
	{
		// Fullbright with default or fullbright color
		if(!pmaterial->fullbrightcolor.IsZero())
			m_pShader->SetUniform4f(m_attribs.u_color, pmaterial->fullbrightcolor.x, pmaterial->fullbrightcolor.y, pmaterial->fullbrightcolor.z, m_renderAlpha);

		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, rns.fog.settings.active ? vbm_texonly_fog : vbm_texonly, false);
	}
	else if(m_isMultiPass) 
	{
		// Multipass without textures
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_notexture, false);
	}
	else
	{
		// Normal textured single-pass rendering
		result = m_pShader->SetDeterminator(m_attribs.d_shadertype, rns.fog.settings.active ? vbm_texture_fog : vbm_texture, false);
	}
		
	if(pmaterial->flags & TX_FL_SCOPE)
	{
		Float flscale = pmaterial->scale / g_pCvarReferenceFOV->GetValue();
		m_pShader->SetUniform1f(m_attribs.u_scope_scale, flscale);

		m_pShader->SetUniform2f(m_attribs.u_scope_scrsize, rns.screenwidth, rns.screenheight);
		R_BindRectangleTexture(GL_TEXTURE1_ARB, m_pScreenTexture->palloc->gl_index);
		m_pShader->SetUniform1i(m_attribs.u_rectangle, 1);
	}

	// Verify the settings
	if(!result || !m_pShader->VerifyDeterminators())
		return false;

	if(pmaterial->ptextures[MT_TX_SPECULAR] && g_pCvarSpecular->GetValue() > 0)
	{
		m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
		m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

		m_pShader->SetUniform1i(m_attribs.u_spectexture, 2);
		R_Bind2DTexture(GL_TEXTURE2, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_LUMINANCE])
	{
		m_pShader->SetUniform1i(m_attribs.u_lumtexture, 3);
		R_Bind2DTexture(GL_TEXTURE3, pmaterial->ptextures[MT_TX_LUMINANCE]->palloc->gl_index);
	}

	if(pmaterial->ptextures[MT_TX_NORMALMAP] && g_pCvarBumpMaps->GetValue() > 0)
	{
		m_pShader->SetUniform1i(m_attribs.u_normalmap, 4);
		R_Bind2DTexture(GL_TEXTURE4, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
	}
	
	if (pmaterial->ptextures[MT_TX_AO])
	{
		m_pShader->SetUniform1i(m_attribs.u_aotexture, 5);
		R_Bind2DTexture(GL_TEXTURE5, pmaterial->ptextures[MT_TX_AO]->palloc->gl_index);
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

	if(pmesh->numbones)
		SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);
	
	if(drawBlended && pmaterial->flags & TX_FL_EYEGLINT)
		R_Bind2DTexture(GL_TEXTURE0, m_pGlintTexture->palloc->gl_index);
	else if(!m_isMultiPass || pmaterial->flags & (TX_FL_ALPHATEST|TX_FL_FULLBRIGHT|TX_FL_SCOPE))
		R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

	if(pmaterial->flags & TX_FL_NO_CULLING)
		glDisable(GL_CULL_FACE);

	R_ValidateShader(m_pShader);

	glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));

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
bool CVBMRenderer::DrawLights( bool specularPass )
{
	if(!m_isMultiPass)
		return true;

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, specularPass, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false))
		return false;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	if(!specularPass)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_ONE, GL_ONE);

		glDepthFunc(GL_EQUAL);
	}

	if(!m_numDynamicLights)
		return true;

	Int32 firstexunit = 0;

	if(m_useFlexes)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, TRUE, false))
			return false;

		m_pShader->SetUniform1i(m_attribs.u_flextexture, firstexunit);
		R_Bind2DTexture(GL_TEXTURE0_ARB+firstexunit, m_pFlexTexture->gl_index);
		firstexunit++;
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false))
			return false;
	}

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
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);

	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_fogcolor);
	m_pShader->DisableSync(m_attribs.u_fogparams);
	m_pShader->DisableSync(m_attribs.u_light_radius);

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->EnableSync(m_attribs.u_normalmatrix);
	}

	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_color);

	m_pShader->EnableAttribute(m_attribs.a_texcoord1);

	Int32 skinnum = m_pCurrentEntity->curstate.skin; // for short..
	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();

	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	CMatrix pmatrix;
	const Float *fltranspose = rns.view.modelview.Transpose();

	// Highest normal map texture unit used
	Int32 highestnormalmapunit = NO_POSITION;
	// Highest specular map texture unit used
	Int32 highestspecularmapunit = NO_POSITION;
	// Highest AO unit used
	Int32 highestaounit = NO_POSITION;

	lightBatches.begin();
	while(!lightBatches.end())
	{
		lightbatch_t& batch = lightBatches.get();

		if(batch.type == LB_TYPE_POINTLIGHT || batch.type == LB_TYPE_POINTLIGHT_SHADOW)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_dynlight, false))
				return false;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_spotlight, false))
				return false;
		}

		// Latest light index
		Uint32 lightindex = 0;
		// Next available texture unit
		Int32 texunit = firstexunit;
		// Normal map unit
		Int32 normalmapunit = NO_POSITION;
		// Specular map unit
		Int32 specularmapunit = NO_POSITION;
		// AO mapping unit to use
		Int32 aomapunit = NO_POSITION;
		
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
					if(!m_pShader->SetDeterminator(m_attribs.dlights[lightindex].d_light_shadowmap, TRUE, false))
						return false;

					m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_shadowmap);
					m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_light_shadowmap, texunit);
					R_Bind2DTexture(GL_TEXTURE0+texunit, pdlight->getProjShadowMap()->pfbo->ptexture1->gl_index);
					texunit++;
				}
				else
				{
					m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_shadowmap);
					if(!m_pShader->SetDeterminator(m_attribs.dlights[lightindex].d_light_shadowmap, FALSE, false))
						return false;
				}

				m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_projtexture);
				m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_light_projtexture, texunit);
				R_Bind2DTexture(GL_TEXTURE0+texunit, rns.objects.projective_textures[pdlight->textureindex]->palloc->gl_index);
				texunit++;

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
				m_pShader->SetUniformMatrix4fv(m_attribs.dlights[lightindex].u_light_matrix, pmatrix.Transpose());
			}
			else
			{
				m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_projtexture);
				m_pShader->DisableSync(m_attribs.dlights[lightindex].u_light_shadowmap);

				if(DL_CanShadow(pdlight))
				{
					if(!m_pShader->SetDeterminator(m_attribs.dlights[lightindex].d_light_shadowmap, TRUE, false))
						return false;

					m_pShader->EnableSync(m_attribs.dlights[lightindex].u_light_cubemap);
					m_pShader->SetUniform1i(m_attribs.dlights[lightindex].u_light_cubemap, texunit);
					R_BindCubemapTexture(GL_TEXTURE0+texunit, pdlight->getCubeShadowMap()->pfbo->ptexture1->gl_index);
					texunit++;

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
					if(!m_pShader->SetDeterminator(m_attribs.dlights[lightindex].d_light_shadowmap, FALSE, false))
						return false;
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

		if(!m_pShader->SetDeterminator(m_attribs.d_numdlights, lightindex, false))
			return false;
		
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

				if(pmaterial->flags & (TX_FL_ADDITIVE|TX_FL_ALPHABLEND|TX_FL_FULLBRIGHT|TX_FL_SCOPE))
					continue;

				if(specularPass)
				{
					if(!pmaterial->ptextures[MT_TX_SPECULAR])
						continue;

					// Set specular map
					specularmapunit = texunit;
					texunit++;

					m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
					m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

					R_Bind2DTexture(GL_TEXTURE0+specularmapunit, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
				}

				if(pmaterial->ptextures[MT_TX_NORMALMAP] && g_pCvarBumpMaps->GetValue() > 0)
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, true))
						return false;

					// Set normal map
					normalmapunit = texunit;
					texunit++;

					R_Bind2DTexture(GL_TEXTURE0+normalmapunit, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				}
				else
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false))
						return false;
				}

				if (pmaterial->ptextures[MT_TX_AO])
				{
					if (!m_pShader->SetDeterminator(m_attribs.d_ao, true))
						return false;

					// Specify the AO map unit
					aomapunit = texunit;
					texunit++;

					R_Bind2DTexture(GL_TEXTURE0+aomapunit, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				}
				else
				{
					if (!m_pShader->SetDeterminator(m_attribs.d_ao, false))
						return false;

					aomapunit = NO_POSITION;
				}

				// u_specular always needs to be set, otherwise AMD will complain
				// about two samplers being on the same unit.
				m_pShader->SetUniform1i(m_attribs.u_normalmap, normalmapunit);
				if (specularmapunit != NO_POSITION)
					m_pShader->SetUniform1i(m_attribs.u_spectexture, specularmapunit);
				else if (aomapunit != NO_POSITION)
					m_pShader->SetUniform1i(m_attribs.u_spectexture, aomapunit + 1);
				else
					m_pShader->SetUniform1i(m_attribs.u_spectexture, normalmapunit + 1);

				// Update the highest units used
				if (highestnormalmapunit < normalmapunit)
					highestnormalmapunit = normalmapunit;

				if (highestspecularmapunit < specularmapunit)
					highestspecularmapunit = specularmapunit;

				if (highestaounit < aomapunit)
					highestaounit = aomapunit;

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				R_ValidateShader(m_pShader);

				glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
			}
		}

		// Reset texunits
		texunit = firstexunit;

		batch.lightslist.begin();
		while(!batch.lightslist.end())
		{
			cl_dlight_t* pdlight = batch.lightslist.get();

			if(pdlight->cone_size)
			{
				if(DL_CanShadow(pdlight))
				{
					R_Bind2DTexture(GL_TEXTURE1+texunit, 0);
					texunit++;
				}

				R_Bind2DTexture(GL_TEXTURE0+texunit, 0);
				texunit++;
			}
			else if(DL_CanShadow(pdlight))
			{
				R_BindCubemapTexture(GL_TEXTURE0+texunit, 0);
				texunit++;

				glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			}

			batch.lightslist.next();
		}

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
		
			// Reset all of these
			if(!m_pShader->SetDeterminator(m_attribs.dlights[i].d_light_shadowmap, FALSE, false))
				return false;		
		}

		lightBatches.next();
	}

	// Ensure these get reset
	if (highestnormalmapunit != NO_POSITION)
		R_Bind2DTexture(GL_TEXTURE0 + highestnormalmapunit, 0);

	if (highestspecularmapunit != NO_POSITION)
		R_Bind2DTexture(GL_TEXTURE0 + highestspecularmapunit, 0);

	if (highestaounit != -1)
		R_Bind2DTexture(GL_TEXTURE0 + highestaounit, 0);

	if(!m_pShader->SetDeterminator(m_attribs.d_numdlights, 0, false))
		return false;

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

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
	}

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->DisableSync(m_attribs.u_normalmatrix);
	}

	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_color);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);

	if(m_useFlexes)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, TRUE, false))
			return false;

		m_pShader->SetUniform1i(m_attribs.u_flextexture, 1);
		R_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, FALSE, false))
			return false;
	}
	
	bool hasSpecular = false;
	Int32 textureFlags = 0;

	const Int16 *pskinref = m_pVBMHeader->getSkinFamilies();
	Int32 skinnum = m_pCurrentEntity->curstate.skin;
	if (skinnum != 0 && skinnum < m_pVBMHeader->numskinfamilies)
		pskinref += (skinnum * m_pVBMHeader->numskinref);

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false))
		return false;

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

			m_pShader->SetUniform1i(m_attribs.u_texture1, 1);
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

			R_Bind2DTexture(GL_TEXTURE0, rns.objects.caustics_textures[causticsCurFrame]->palloc->gl_index);
			R_Bind2DTexture(GL_TEXTURE1, rns.objects.caustics_textures[causticsNextFrame]->palloc->gl_index);

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

					if(pmesh->numbones)
						SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

					R_ValidateShader(m_pShader);

					glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
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

			if(pmesh->numbones)
				SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

			R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
			R_ValidateShader(m_pShader);

			glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
		}

		if(textureFlags & TX_FL_CHROME)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_chrome, true))
				return false;

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

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
				R_ValidateShader(m_pShader);

				glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
			}

			if(!m_pShader->SetDeterminator(m_attribs.d_chrome, false))
				return false;

			m_pShader->DisableAttribute(m_attribs.a_normal);

			// Remove it from the bit flags
			textureFlags &= ~TX_FL_CHROME;
		}
	}

	//
	// Render meshes with specular highlights
	//
	if(hasSpecular && g_pCvarSpecular->GetValue() > 0)
	{
		// TODO: add AO here!
		glBlendFunc(GL_ONE, GL_ONE);

		if(!m_pShader->SetDeterminator(m_attribs.d_numlights, m_numModelLights, false) || 
			!m_pShader->SetDeterminator(m_attribs.d_specular, TRUE, false) || 
			!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_speconly, false))
			return false;

		m_pShader->EnableSync(m_attribs.u_sky_ambient);
		m_pShader->EnableSync(m_attribs.u_sky_diffuse);
		m_pShader->EnableSync(m_attribs.u_sky_dir);
		m_pShader->EnableSync(m_attribs.u_spectexture);
		m_pShader->EnableSync(m_attribs.u_normalmap);

		m_pShader->EnableAttribute(m_attribs.a_texcoord1);
		m_pShader->EnableAttribute(m_attribs.a_normal);

		if (!m_areUBOsSupported)
		{
			m_pShader->EnableSync(m_attribs.u_normalmatrix);
			m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());
		}

		m_pShader->SetUniform1i(m_attribs.u_spectexture, 2);
		m_pShader->SetUniform1i(m_attribs.u_normalmap, 3);
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

		// Set all the uniforms again
		Vector vtransformed;
		CMatrix pmatrix(rns.view.modelview.GetInverse());
		Math::MatMult(pmatrix.Transpose(), m_lightingInfo.lightdirection, &vtransformed);

		m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
		m_pShader->SetUniform3f(m_attribs.u_sky_ambient, m_lightingInfo.ambient_color[0], m_lightingInfo.ambient_color[1], m_lightingInfo.ambient_color[2]);
		m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, m_lightingInfo.direct_color[0], m_lightingInfo.direct_color[1], m_lightingInfo.direct_color[2]);

		for (Uint32 i = 0; i < m_numDrawSubmodels; i++)
		{
			m_pVBMSubModel = m_pSubmodelDrawList[i];

			for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
			{
				// First texture unit used
				Int32 textureunit = 2;
				// Normal mapping unit used
				Int32 normalmapunit = NO_POSITION;
				// Specular mapping unit used
				Int32 specularmapunit = NO_POSITION;
				// Ambient occlusion mapping unit used
				Int32 aomapunit = NO_POSITION;

				const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
				const vbmtexture_t *ptexture = m_pVBMHeader->getTexture(pskinref[pmesh->skinref]);

				en_material_t* pmaterial = pTextureManager->FindMaterialScriptByIndex(ptexture->index);
				if(!pmaterial)
					continue;

				if(!pmaterial->ptextures[MT_TX_SPECULAR])
					continue;

				m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
				m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

				// Set the specular mapping unit
				specularmapunit = textureunit;
				textureunit++;

				R_Bind2DTexture(GL_TEXTURE0 + specularmapunit, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
				m_pShader->SetUniform1i(m_attribs.u_spectexture, specularmapunit);

				// Set normal map if any
				if (pmaterial->ptextures[MT_TX_NORMALMAP])
				{
					if (!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, true))
						return false;

					// Specify the AO map unit
					normalmapunit = textureunit;
					textureunit++;

					R_Bind2DTexture(GL_TEXTURE0 + normalmapunit, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				}
				else
				{
					if (!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false))
						return false;

					normalmapunit = NO_POSITION;
				}

				if (pmaterial->ptextures[MT_TX_AO])
				{
					if (!m_pShader->SetDeterminator(m_attribs.d_ao, true))
						return false;

					// Specify the AO map unit
					aomapunit = textureunit;
					textureunit++;

					R_Bind2DTexture(GL_TEXTURE0 + aomapunit, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				}
				else
				{
					if (!m_pShader->SetDeterminator(m_attribs.d_ao, false))
						return false;

					aomapunit = NO_POSITION;
				}
					
				// u_specular always needs to be set, otherwise AMD will complain
				// about two samplers being on the same unit.
				m_pShader->SetUniform1i(m_attribs.u_normalmap, normalmapunit);
				if (specularmapunit != NO_POSITION)
					m_pShader->SetUniform1i(m_attribs.u_spectexture, specularmapunit);
				else if (aomapunit != NO_POSITION)
					m_pShader->SetUniform1i(m_attribs.u_spectexture, aomapunit + 1);
				else
					m_pShader->SetUniform1i(m_attribs.u_spectexture, normalmapunit + 1);

				if(pmesh->numbones)
					SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

				R_ValidateShader(m_pShader);

				glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
			}
		}

		m_pShader->DisableSync(m_attribs.u_sky_ambient);
		m_pShader->DisableSync(m_attribs.u_sky_diffuse);
		m_pShader->DisableSync(m_attribs.u_sky_dir);

		if (!m_areUBOsSupported)
			m_pShader->DisableSync(m_attribs.u_normalmatrix);

		// Draw dynamic light specular lighting
		if(m_numDynamicLights)
		{
			if(!DrawLights(true))
				return false;

			if(m_useFlexes)
			{
				m_pShader->SetUniform1i(m_attribs.u_flextexture, 1);
				R_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);
			}

			m_pShader->EnableSync(m_attribs.u_fogcolor);
			m_pShader->EnableSync(m_attribs.u_fogparams);
		}

		if(!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false) ||
			!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false) ||
			!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
			!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false))
			return false;

		m_pShader->DisableAttribute(m_attribs.a_texcoord1);
		m_pShader->DisableAttribute(m_attribs.a_normal);
	}

	//
	// Render meshes with fog
	//
	if(rns.fog.settings.active)
	{
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

				glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
			}
		}
	}

	// Reset this
	glDepthFunc(GL_LEQUAL);

	if((textureFlags & (TX_FL_ALPHABLEND|TX_FL_ADDITIVE|TX_FL_EYEGLINT)))
	{
		m_pShader->EnableSync(m_attribs.u_vorigin);
		m_pShader->EnableSync(m_attribs.u_vright);
		m_pShader->EnableSync(m_attribs.u_sky_ambient);
		m_pShader->EnableSync(m_attribs.u_sky_diffuse);
		m_pShader->EnableSync(m_attribs.u_sky_dir);

		if(!m_areUBOsSupported)
		{
			m_pShader->EnableSync(m_attribs.u_normalmatrix);
			m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());
		}

		// Render any additive parts
		bool result = false;
		if(!rns.fog.settings.active) 
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture, false);
		else
			result = m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_texture_fog, false);

		if(!result || !m_pShader->SetDeterminator(m_attribs.d_numlights, m_numModelLights))
			return false;

		m_pShader->EnableAttribute(m_attribs.a_texcoord1);
		m_pShader->EnableAttribute(m_attribs.a_normal);

		if (rns.fog.settings.active)
			m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.0, 0.0, 0.0);

		// Set all the uniforms again
		Vector vtransformed;
		CMatrix pmatrix(rns.view.modelview.GetInverse());
		Math::MatMult(pmatrix.Transpose(), m_lightingInfo.lightdirection, &vtransformed);

		m_pShader->SetUniform3f(m_attribs.u_sky_dir, vtransformed[0], vtransformed[1], vtransformed[2]);
		m_pShader->SetUniform3f(m_attribs.u_sky_ambient, m_lightingInfo.ambient_color[0], m_lightingInfo.ambient_color[1], m_lightingInfo.ambient_color[2]);
		m_pShader->SetUniform3f(m_attribs.u_sky_diffuse, m_lightingInfo.direct_color[0], m_lightingInfo.direct_color[1], m_lightingInfo.direct_color[2]);

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
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
				}
				else if(pmaterial->flags & TX_FL_ALPHABLEND)
				{
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, pmaterial->alpha);
				}

				if(!DrawMesh(pmaterial, pmesh, true))
					return false;
			}
		}

		m_pShader->DisableAttribute(m_attribs.a_texcoord1);
		m_pShader->DisableAttribute(m_attribs.a_normal);

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
	}

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->DisableSync(m_attribs.u_normalmatrix);
	}

	m_pShader->EnableSync(m_attribs.u_color);
	m_pShader->EnableSync(m_attribs.u_flextexture);
	m_pShader->EnableSync(m_attribs.u_flextexturesize);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);

	if(m_pVBMHeader->flags & VBM_HAS_FLEXES && m_isVertexFetchSupported)
	{
		m_pShader->EnableAttribute(m_attribs.a_flexcoord);

		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, 1, false))
			return false;

		m_pShader->SetUniform1i(m_attribs.u_flextexture, 1);
		R_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, 0, false))
			return false;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_chrome, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid))
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

			glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(m_pVBMHeader->flags & VBM_HAS_FLEXES && m_isVertexFetchSupported)
		m_pShader->DisableAttribute(m_attribs.a_flexcoord);

	if(g_pCvarWireFrame->GetValue() >= 2)
		glEnable(GL_DEPTH_TEST);

	if(!m_pShader->SetDeterminator(m_attribs.d_flexes, 0, false))
		return false;

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

	// Set pointer for current mesh
	vbm_decal_mesh_t* pdecalmesh = new vbm_decal_mesh_t;
	pdecal->meshes.push_back(pdecalmesh);

	Uint32 curstart;
	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_NONE);

		const byte *pboneids = nullptr;
		for (Int32 j = 0; j < m_pVBMSubModel->nummeshes; j++) 
		{
			curstart = m_numTempVertexes;
			const vbmmesh_t *pmesh = m_pVBMSubModel->getMesh(m_pVBMHeader, j);
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

			if(m_pVBMSubModel->flexinfoindex != -1 && !m_isVertexFetchSupported)
				continue;

			const vbmvertex_t *triverts[3];
			for(Int32 k = 0; k < pmesh->num_indexes; k += 3)
			{
				for(Uint32 l = 0; l < 3; l++)
					triverts[l] = &pvertexes[pindexes[pmesh->start_index+k+l]];

				if (!DecalTriangle(pdecal, pdecalmesh, triverts, pboneids, position, normal, pdecal, up, right, curstart, flags, pmaterial))
				{
					Con_Printf("%s - Error creating decal '%s' for VBM model '%s'.\n", __FUNCTION__, texptr->name.c_str(), m_pVBMHeader->name);
					DeleteDecal(pdecal);
					return;
				}
			}
		}
	}

	// No geometry was decalled
 	if(!m_numTempIndexes || !m_numTempVertexes)
	{
		DeleteDecal(pdecal);
		return;
	}

	// Add the last mesh
	FinalizeDecalMesh(pdecal, pdecalmesh, curstart);
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

	m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*vertexoffset, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	
	if(!pdecal->start_vertex)
		pdecal->start_vertex = vertexoffset;

	pdecal->num_vertexes += m_numTempVertexes;

	m_pVBO->IBOSubBufferData(sizeof(Uint32)*indexoffset, m_tempIndexes, sizeof(Uint32)*m_numTempIndexes);
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
bool CVBMRenderer::DecalTriangle( vbmdecal_t* pdecal, vbm_decal_mesh_t*& pmesh, const vbmvertex_t **pverts, const byte *pboneids, const Vector& position, const Vector& normal, vbmdecal_t *decal, const Vector& up, const Vector& right, Uint32& curstart, byte flags, en_material_t* pmaterial )
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
			Math::VectorTransform(pverts[i]->origin, (*m_pWeightBoneTransform)[boneindex], tmp);
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

	// Split the decal if we're going over the array size
	if((m_numTempVertexes+3) >= MAX_TEMP_VBM_VERTEXES 
		|| (m_numTempIndexes+3) >= MAX_TEMP_VBM_INDEXES
		|| pmesh->alphatest != alphatest
		|| pmesh->alphatest && ptexture != pmesh->ptexture)
	{
		// Finalize the mesh
		FinalizeDecalMesh(pdecal, pmesh, curstart);
			
		// Allocate a new mesh
		pmesh = new vbm_decal_mesh_t;
		pdecal->meshes.push_back(pmesh);
		pmesh->alphatest = alphatest;

		if(pmesh->alphatest)
			pmesh->ptexture = ptexture;
	}

	Uint32 numadd = 0;
	byte addbones[MAX_SHADER_BONES];
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

	// Determine how many new bones we'll have
	if(pmesh->numbones && (pmesh->numbones + numadd) > MAX_SHADER_BONES)
	{
		// Finalize the mesh
		FinalizeDecalMesh(pdecal, pmesh, curstart);
			
		// Allocate a new mesh
		pmesh = new vbm_decal_mesh_t;
		pdecal->meshes.push_back(pmesh);
	}

	// Add it if it's not
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
bool CVBMRenderer::DrawDecals( void )
{
	if(m_pCvarDrawModelDecals->GetValue() < 1)
		return true;

	if(!m_pExtraInfo)
		return true;

	if(!m_pExtraInfo->pvbmdecalheader)
		return true;

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
	}

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->DisableSync(m_attribs.u_normalmatrix);
	}

	m_pShader->EnableSync(m_attribs.u_color);
	m_pShader->EnableSync(m_attribs.u_flextexture);
	m_pShader->EnableSync(m_attribs.u_flextexturesize);
	m_pShader->EnableSync(m_attribs.u_fogcolor);
	m_pShader->EnableSync(m_attribs.u_fogparams);
	m_pShader->EnableSync(m_attribs.u_texture0);
	m_pShader->EnableSync(m_attribs.u_texture1);

	Int32 alphatestMode = (rns.msaa && rns.mainframe) ? ALPHATEST_COVERAGE : ALPHATEST_LESSTHAN;

	if(!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_alphatest, alphatestMode, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false))
		return false;

	m_pShader->EnableAttribute(m_attribs.a_texcoord1);

	if(alphatestMode == ALPHATEST_COVERAGE)
	{
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		gGLExtF.glSampleCoverage(0.5, GL_FALSE);
	}

	if((m_pVBMHeader->flags & VBM_HAS_FLEXES) && m_isVertexFetchSupported)
	{
		m_pShader->EnableAttribute(m_attribs.a_flexcoord);

		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, 1, false))
			return false;

		m_pShader->SetUniform1i(m_attribs.u_flextexture, 1);
		R_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, 0, false))
			return false;
	}

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glPolygonOffset(-1,-1);
	glEnable(GL_POLYGON_OFFSET_FILL);

	// Last used decal rendermode
	decal_rendermode_t renderMode = DECAL_RENDERMODE_NONE;

	while(pnext)
	{
		R_Bind2DTexture(GL_TEXTURE0, pnext->pentry->ptexture->palloc->gl_index);

		for(Uint32 i = 0; i < pnext->meshes.size(); i++)
		{
			vbm_decal_mesh_t* pmesh = pnext->meshes[i];

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
				if((m_pVBMHeader->flags & VBM_HAS_FLEXES) && m_isVertexFetchSupported)
				{
					m_pShader->SetUniform1i(m_attribs.u_texture1, 2);
					R_Bind2DTexture(GL_TEXTURE2, pmesh->ptexture->palloc->gl_index);
				}
				else
				{
					m_pShader->SetUniform1i(m_attribs.u_texture1, 1);
					R_Bind2DTexture(GL_TEXTURE1, pmesh->ptexture->palloc->gl_index);
				}
			}

			SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->pbones, pmesh->numbones);

			R_ValidateShader(m_pShader);

			glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(pmesh->start_index));
		}

		vbmdecal_t *next = pnext->next;
		pnext = next;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_flexes, 0, false))
		return false;

	if(m_pVBMHeader->flags & VBM_HAS_FLEXES && m_isVertexFetchSupported)
		m_pShader->DisableAttribute(m_attribs.a_flexcoord);

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

	for (Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		SetupModel(i, VBM_LOD_DISTANCE);

		if(m_pVBMSubModel->flexinfoindex != -1)
			continue;

		m_pSubmodelDrawList[m_numDrawSubmodels] = m_pVBMSubModel;
		m_numDrawSubmodels++;
	}

	// Run draw routines
	if(!SetupRenderer())
		return false;

	if(!DrawFirst())
		return false;

	if(!DrawLights(false))
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

		m_pShader->SetUniform1i(m_attribs.u_flextexture, 1);
		m_pShader->SetUniform1f(m_attribs.u_flextexturesize, VBM_FLEXTEXTURE_SIZE);

		R_Bind2DTexture(GL_TEXTURE1_ARB, m_pFlexTexture->gl_index);

		if(!m_pShader->SetDeterminator(m_attribs.d_flexes, TRUE, false))
			return false;

		m_useFlexes = true;
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

		if(!DrawLights(false))
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

		m_pShader->DisableSync(m_attribs.u_flextexture);
		m_pShader->DisableSync(m_attribs.u_flextexturesize);
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

	vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
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

	vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
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
				origin[j] = (*m_pBoneTransform)[i][j][3];

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

	vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
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
			transvector[i] -= (*m_pBoneTransform)[boneindex][i][3];

		Math::VectorInverseRotate(transvector, (*m_pBoneTransform)[boneindex], vector);
	}
	else
	{
		Math::VectorTransform(transvector, (*m_pBoneTransform)[boneindex], vector);
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

	vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
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
		Math::VectorInverseRotate(transvector, (*m_pBoneTransform)[boneindex], vector);
	else
		Math::VectorRotate(transvector, (*m_pBoneTransform)[boneindex], vector);
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

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
	{
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	if(m_areUBOsSupported)
	{
		R_SetMatrixData(rns.view.projection.GetMatrix(), m_uboMatricesData[VS_MATRIX_PROJECTION]);
		R_SetMatrixData(rns.view.modelview.GetMatrix(), m_uboMatricesData[VS_MATRIX_MODELVIEW]);
		R_SetMatrixData(rns.view.modelview.GetInverse(), m_uboMatricesData[VS_MATRIX_NORMALMATRIX]);

		m_pShader->SetUniformBufferObjectData(m_attribs.ub_vsmatrices, m_uboMatricesData, sizeof(m_uboMatricesData));
	}
	else
	{
		m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());
	}

	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);
	return true;
}

//=============================================
//
//
//=============================================
void CVBMRenderer::EndDraw( void )
{
	m_pShader->DisableShader();
	m_pVBO->UnBind();
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

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
	{
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_vsm))
	{
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);
	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	m_pShader->SetUniform1f(m_attribs.u_light_radius, dl->radius);

	if(m_areUBOsSupported)
	{
		R_SetMatrixData(rns.view.projection.GetMatrix(), m_uboMatricesData[VS_MATRIX_PROJECTION]);
		R_SetMatrixData(rns.view.modelview.GetMatrix(), m_uboMatricesData[VS_MATRIX_MODELVIEW]);

		m_pShader->SetUniformBufferObjectData(m_attribs.ub_vsmatrices, m_uboMatricesData, sizeof(m_uboMatricesData));
	}
	else
	{
		m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	}

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_flextexture);
	m_pShader->DisableSync(m_attribs.u_flextexturesize);
	m_pShader->DisableSync(m_attribs.u_vorigin);
	m_pShader->DisableSync(m_attribs.u_vright);
	m_pShader->DisableSync(m_attribs.u_sky_ambient);
	m_pShader->DisableSync(m_attribs.u_sky_diffuse);
	m_pShader->DisableSync(m_attribs.u_sky_dir);
	m_pShader->DisableSync(m_attribs.u_fogcolor);
	m_pShader->DisableSync(m_attribs.u_fogparams);
	m_pShader->DisableSync(m_attribs.u_color);

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->DisableSync(m_attribs.u_normalmatrix);
	}

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
	m_pVBO->UnBind();
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
		if(!m_pCacheModel || !m_pCacheModel->pcachedata)
		{
			Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
			return true;
		}

		EndVSM();

		// Clear shader ptr
		m_pShader->SetVBO(nullptr);
		m_pShader->ResetShader();

		if(m_pVBO)
		{
			delete m_pVBO;
			m_pVBO = nullptr;
		}
		
		// Rebuild the VBO
		BuildVBO();

		// Re-set shader
		m_pShader->SetVBO(m_pVBO);
		if(!PrepareVSM(dl))
			return false;
	}

	vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
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
				if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_vsmalpha, false))
					return false;

				R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
			}
			else
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_vsm, false))
					return false;

				m_pShader->DisableAttribute(m_attribs.a_texcoord1);
			}

			if(!m_pShader->VerifyDeterminators())
				return false;

			if(pmesh->numbones)
				SetShaderBoneTransform(m_pWeightBoneTransform, pmesh->getBones(m_pVBMHeader), pmesh->numbones);

			R_ValidateShader(m_pShader);

			glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
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

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
	{
		m_pVBO->UnBind();
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_boneindexes);
	m_pShader->EnableAttribute(m_attribs.a_boneweights);

	if(!m_pShader->SetDeterminator(m_attribs.d_chrome, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid))
	{
		m_pVBO->UnBind();
		return false;
	}

	if(m_areUBOsSupported)
	{
		R_SetMatrixData(rns.view.projection.GetMatrix(), m_uboMatricesData[VS_MATRIX_PROJECTION]);
		R_SetMatrixData(rns.view.modelview.GetMatrix(), m_uboMatricesData[VS_MATRIX_MODELVIEW]);

		m_pShader->SetUniformBufferObjectData(m_attribs.ub_vsmatrices, m_uboMatricesData, sizeof(m_uboMatricesData));
	}
	else
	{
		m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	}

	m_pShader->DisableSync(m_attribs.u_causticsm1);
	m_pShader->DisableSync(m_attribs.u_causticsm2);
	m_pShader->DisableSync(m_attribs.u_light_radius);
	m_pShader->DisableSync(m_attribs.u_flextexture);
	m_pShader->DisableSync(m_attribs.u_flextexturesize);
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
	}

	if(!m_areUBOsSupported)
	{
		m_pShader->EnableSync(m_attribs.u_projection);
		m_pShader->EnableSync(m_attribs.u_modelview);
		m_pShader->DisableSync(m_attribs.u_normalmatrix);
	}

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
	m_pVBO->UnBind();
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
		if(!m_pCacheModel || !m_pCacheModel->pcachedata)
		{
			Con_Printf("%s - Failed to get model by index %d.\n", __FUNCTION__, m_pCurrentEntity->curstate.modelindex);
			return true;
		}

		FinishAuraPass();

		// Clear shader ptr
		m_pShader->SetVBO(nullptr);
		m_pShader->ResetShader();

		if(m_pVBO)
		{
			delete m_pVBO;
			m_pVBO = nullptr;
		}
		
		// Rebuild the VBO
		BuildVBO();

		// Re-set shader
		m_pShader->SetVBO(m_pVBO);
		if(!PrepAuraPass())
			return false;
	}

	vbmcache_t* pstudiocache = m_pCacheModel->getVBMCache();
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

			glDrawElements(GL_TRIANGLES, pmesh->num_indexes, GL_UNSIGNED_INT, BUFFER_OFFSET(m_pVBMHeader->ibooffset+pmesh->start_index));
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

	Float flframe = VBM_EstimateFrame(pseqdesc, m_pCurrentEntity->curstate, rns.time);

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
	vbmbodypart_t *pbodypart = m_pVBMHeader->getBodyPart(bodypart);

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
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
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

		Vector worldOrigin((*m_pBoneTransform)[i][0][3], (*m_pBoneTransform)[i][1][3], (*m_pBoneTransform)[i][2][3]);
		Vector parentOrigin((*m_pBoneTransform)[pbone->parent][0][3], (*m_pBoneTransform)[pbone->parent][1][3], (*m_pBoneTransform)[pbone->parent][2][3]);

		Vector temp;
		Math::VectorSubtract(worldOrigin, parentOrigin, temp);
		Math::VectorInverseRotate(temp, (*m_pBoneTransform)[pbone->parent], worldOrigin);

		BatchVertex(Vector(0, 0, 0));
		BatchVertex(worldOrigin);

		m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

		R_ValidateShader(m_pShader);

		glDrawArrays(GL_LINES, m_drawBufferIndex, m_numTempVertexes);
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

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

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
		m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.5);

		DrawBox(pbbox->bbmin, pbbox->bbmax);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawBoundingBox( void )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Float matrix[3][4];
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
	matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0.0;
	matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

	// Set bone transform to identity
	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
	else
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

	// Set color
	const Float* pcolor = RANDOM_COLOR_ARRAY[m_pCurrentEntity->entindex%NUM_RANDOM_COLORS];
	m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.5);

	DrawBox(m_mins, m_maxs);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	return true;
}

//=============================================
//
//
//=============================================
bool CVBMRenderer::DrawLightVectors( void )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(4.0);

	Float matrix[3][4];
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
					Float strengths[4];
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
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
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
		Float matrix[3][4];
		matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
		matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0.0;
		matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

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

		Vector worldOrigin((*m_pBoneTransform)[pattachment->bone][0][3], (*m_pBoneTransform)[pattachment->bone][1][3], (*m_pBoneTransform)[pattachment->bone][2][3]);
		Vector attachmentOrigin;
		Math::VectorTransform(pattachment->org, (*m_pBoneTransform)[pattachment->bone], attachmentOrigin);

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

		m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

		// Draw the line
		glDrawArrays(GL_LINES, m_drawBufferIndex, m_numTempVertexes);

		color.x = RANDOM_COLOR_ARRAY[(i+2)%NUM_RANDOM_COLORS][0];
		color.y = RANDOM_COLOR_ARRAY[(i+2)%NUM_RANDOM_COLORS][1];
		color.z = RANDOM_COLOR_ARRAY[(i+2)%NUM_RANDOM_COLORS][2];

		// Draw the point
		glPointSize(5);
		m_pShader->SetUniform4f(m_attribs.u_color, color[0], color[1], color[2], 1.0);

		glDrawArrays(GL_POINTS, m_drawBufferIndex+1, 1);
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
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_chrome, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_flexes, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_luminance, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_specular, false, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_ao, 0, false) ||
		!m_pShader->SetDeterminator(m_attribs.d_shadertype, vbm_solid, true))
		return false;

	glDisable (GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Float matrix[3][4];
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
	matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0.0;
	matrix[0][3] = matrix[1][3] = matrix[2][3] = 0.0;

	// Set bone transform to identity
	if(m_areUBOsSupported)
		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, matrix, 3*sizeof(vec4_t));
	else
		m_pShader->SetUniform4fv(m_attribs.boneindexes[0], reinterpret_cast<Float *>(matrix), 3);

	// Set color
	const Float* pcolor = RANDOM_COLOR_ARRAY[m_pCurrentEntity->entindex%NUM_RANDOM_COLORS];
	m_pShader->SetUniform4f(m_attribs.u_color, pcolor[0], pcolor[1], pcolor[2], 0.5);

	Vector mins, maxs;
	Math::VectorAdd(m_pCurrentEntity->curstate.mins, m_pCurrentEntity->curstate.origin, mins);
	Math::VectorAdd(m_pCurrentEntity->curstate.maxs, m_pCurrentEntity->curstate.origin, maxs);

	DrawBox(mins, maxs);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

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
	m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);
	glDrawArrays(GL_TRIANGLES, m_drawBufferIndex, m_numTempVertexes);
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

	m_pVBO->VBOSubBufferData(sizeof(vbm_glvertex_t)*m_drawBufferIndex, m_tempVertexes, sizeof(vbm_glvertex_t)*m_numTempVertexes);

	R_ValidateShader(m_pShader);

	glDrawArrays(GL_LINES, m_drawBufferIndex, m_numTempVertexes);
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
void CVBMRenderer::SetShaderBoneTransform( Float (*pbonetransform)[MAXSTUDIOBONES][3][4], const byte* pboneindexes, Uint32 numbones )
{
	if(m_areUBOsSupported)
	{
		for(Uint32 i = 0; i < numbones; i++)
			memcpy((void *)m_uboBoneMatrixData[i], (void *)(*pbonetransform)[pboneindexes[i]], sizeof(vec4_t)*3);

		m_pShader->SetUniformBufferObjectData(m_attribs.ub_bonematrices, m_uboBoneMatrixData, numbones*3*sizeof(vec4_t));
	}
	else
	{
		for(Int32 i = 0; i < numbones; i++)
			m_pShader->SetUniform4fv(m_attribs.boneindexes[i], (Float *)(*pbonetransform)[pboneindexes[i]], 3);
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
// @brief Finds the material by it's index - ugly hack
//
//=============================================
en_material_t* VBM_FindMaterialScriptByIndex( Int32 index )
{
	return CTextureManager::GetInstance()->FindMaterialScriptByIndex(index);
}
