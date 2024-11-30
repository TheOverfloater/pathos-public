/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

// Notes:
// AO mapping related code was done by valina354.

#include "includes.h"
#include "brushmodel.h"
#include "cl_entity.h"
#include "cvar.h"
#include "com_math.h"
#include "frustum.h"
#include "console.h"
#include "system.h"
#include "enginestate.h"
#include "modelcache.h"
#include "file.h"
#include "aldformat.h"

#include "r_dlights.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "texturemanager.h"
#include "r_bsp.h"
#include "r_main.h"
#include "r_common.h"
#include "cl_main.h"
#include "cl_utils.h"
#include "r_dlights.h"
#include "r_decals.h"
#include "r_water.h"
#include "r_particles.h"
#include "r_cubemaps.h"
#include "r_vbm.h"
#include "r_common.h"
#include "r_wadtextures.h"
#include "vid.h"
#include "tga.h"
#include "r_lightstyles.h"

// Default lightmap width
const Uint32 CBSPRenderer::LIGHTMAP_DEFAULT_WIDTH = 128;
// Default lightmap height
const Uint32 CBSPRenderer::LIGHTMAP_DEFAULT_HEIGHT = 128;
// BSP decal cache size
const Uint32 CBSPRenderer::NB_BSP_DECAL_VERTS = 16384;
// Backface epsilon value
const Float CBSPRenderer::BACKFACE_EPSILON = 0.01;
// Max overlapping decals in a place
const Uint32 CBSPRenderer::MAX_DECAL_OVERLAP = 4;
// Decal vertex allocation size
const Uint32 CBSPRenderer::BSP_DECALVERT_ALLOC_SIZE = 8196;
// Temporary decal vertex array allocation size
const Uint32 CBSPRenderer::TEMP_DECAL_VERTEX_ALLOC_SIZE = 64;
// Specialfog distance
const Float CBSPRenderer::SPECIALFOG_DISTANCE = 300;

// Object definition
CBSPRenderer gBSPRenderer;

//=============================================
// @brief
//
//=============================================
CBSPRenderer::CBSPRenderer( void ):
	m_pCurrentEntity(nullptr),
	m_multiPass(false),
	m_addMulti(false),
	m_bumpMaps(false),
	m_useLightStyles(false),
	m_disableMultiPass(false),
	m_pChromeTexture(nullptr),
	m_vertexCacheBase(0),
	m_vertexCacheIndex(0),
	m_vertexCacheSize(0),
	m_pLightStyleValuesArray(nullptr),
	m_pCvarDetailTextures(nullptr),
	m_pCvarDetailScale(nullptr),
	m_pCvarDrawWorld(nullptr),
	m_pCvarNormalBlendAngle(nullptr),
	m_pCvarLegacyTransparents(nullptr),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_pDecalVBO(nullptr),
	m_isCubemappingSupported(false)
{
	m_tempDecalVertsArray.resize(TEMP_DECAL_VERTEX_ALLOC_SIZE);
	memset(m_lightmapWidths, 0, sizeof(m_lightmapWidths));
	memset(m_lightmapHeights, 0, sizeof(m_lightmapHeights));

	memset(m_lightmapIndexes, 0, sizeof(m_lightmapIndexes));
	memset(m_ambientLightmapIndexes, 0, sizeof(m_ambientLightmapIndexes));
	memset(m_diffuseLightmapIndexes, 0, sizeof(m_diffuseLightmapIndexes));
	memset(m_lightVectorsIndexes, 0, sizeof(m_lightVectorsIndexes));
}

//=============================================
// @brief
//
//=============================================
CBSPRenderer::~CBSPRenderer()
{
	Shutdown();
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::Init( void ) 
{
	// set cvars
	m_pCvarDrawWorld = gConsole.CreateCVar( CVAR_FLOAT, FL_CV_CLIENT, "r_drawworld", "1", "Toggle world rendering." );
	m_pCvarDetailTextures = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_detail", "1", "Toggle detail textures." );
	m_pCvarDetailScale = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_detail_scale", "1", "Adjusts detail texture scaling." );
	m_pCvarNormalBlendAngle = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_smooth_angle", "60", "Controls normal blending for brushes." );
	m_pCvarLegacyTransparents = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_bsp_legacytransparents", "0", "Controls whether BSP rendering uses legacy(HL1 style unlit) rendering for transparent entities." );

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::Shutdown( void ) 
{
	ClearGL();
	ClearGame();
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::InitGL( void ) 
{
	// Initialize our shader
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;

		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;
		else if(g_pCvarGLSLOnDemand->GetValue() > 0)
			shaderFlags |= CGLSLShader::FL_GLSL_ONDEMAND_LOAD;

		// Set to enabled by default
		m_isCubemappingSupported = true;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, shaderFlags, VID_ShaderCompileCallback);
		if(!m_pShader->Compile("bsprenderer.bss"))
		{
			// The shader might be too complex with cubemapping added, so
			// disable it and try again
			m_isCubemappingSupported = false;

			m_pShader->DisableDeterminatorState("cubemaps", 1);
			m_pShader->DisableDeterminatorState("cubemaps", 2);
			m_pShader->DisableDeterminatorState("shadertype", static_cast<Int32>(shader_cubeonly));

			if(!m_pShader->Compile("bsprenderer.bss"))
			{
				Sys_ErrorPopup("%s - Could not compile shader: %s", __FUNCTION__, m_pShader->GetError());
				return false;
			}
		}		

		// If active load is set, keep loading shaders during game runtime
		if((shaderFlags & CGLSLShader::FL_GLSL_ONDEMAND_LOAD) && g_pCvarGLSLActiveLoad->GetValue() > 0)
			R_AddShaderForLoading(m_pShader);

		// Initialize determinators
		m_attribs.d_shadertype = m_pShader->GetDeterminatorIndex("shadertype");
		m_attribs.d_fogtype = m_pShader->GetDeterminatorIndex("fogtype");
		m_attribs.d_alphatest = m_pShader->GetDeterminatorIndex("alphatest");
		m_attribs.d_bumpmapping = m_pShader->GetDeterminatorIndex("bumpmapping");
		m_attribs.d_specular = m_pShader->GetDeterminatorIndex("specular");
		if(m_isCubemappingSupported)
			m_attribs.d_cubemaps = m_pShader->GetDeterminatorIndex("cubemaps");
		m_attribs.d_luminance = m_pShader->GetDeterminatorIndex("luminance");
		m_attribs.d_ao = m_pShader->GetDeterminatorIndex("ao");
		m_attribs.d_numlights = m_pShader->GetDeterminatorIndex("numlights");

		if(!R_CheckShaderDeterminator(m_attribs.d_shadertype, "shadertype", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_fogtype, "fogtype", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_alphatest, "alphatest", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_bumpmapping, "bumpmapping", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_specular, "specular", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_luminance, "luminance", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_ao, "ao", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_numlights, "numlights", m_pShader, Sys_ErrorPopup))
			return false;

		if(m_isCubemappingSupported)
		{
			if(!R_CheckShaderDeterminator(m_attribs.d_cubemaps, "cubemaps", m_pShader, Sys_ErrorPopup))
				return false;
		}

		// Initialize attribs
		m_attribs.a_position = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, origin));
		m_attribs.a_tangent = m_pShader->InitAttribute("in_tangent", 3, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, tangent));
		m_attribs.a_binormal = m_pShader->InitAttribute("in_binormal", 3, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, binormal));
		m_attribs.a_normal = m_pShader->InitAttribute("in_normal", 3, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, normal));
		m_attribs.a_lmapcoord = m_pShader->InitAttribute("in_lmapcoord", 2, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, lmapcoord[0]));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, texcoord));
		m_attribs.a_dtexcoord = m_pShader->InitAttribute("in_dtexcoord", 2, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, dtexcoord));
		m_attribs.a_fogcoord = m_pShader->InitAttribute("in_fogcoord", 1, GL_FLOAT, sizeof(bsp_vertex_t), OFFSET(bsp_vertex_t, fogcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_position, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_tangent, "in_tangent", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_binormal, "in_binormal", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_normal, "in_normal", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_lmapcoord, "in_lmapcoord", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_dtexcoord, "in_dtexcoord", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_fogcoord, "in_fogcoord", m_pShader, Sys_ErrorPopup))
			return false;

		// vertex shader uniforms
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);

		m_attribs.u_normalmatrix = m_pShader->InitUniform("normalmatrix", CGLSLShader::UNIFORM_MATRIX4);

		m_attribs.u_causticsm1 = m_pShader->InitUniform("caustics_m1", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_causticsm2 = m_pShader->InitUniform("caustics_m2", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_interpolant = m_pShader->InitUniform("interpolant", CGLSLShader::UNIFORM_FLOAT1);

		m_attribs.u_vorigin = m_pShader->InitUniform("v_origin", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_vright = m_pShader->InitUniform("v_right", CGLSLShader::UNIFORM_FLOAT3);

		m_attribs.u_uvoffset = m_pShader->InitUniform("uvoffset", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_phong_exponent = m_pShader->InitUniform("phong_exponent", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_specularfactor = m_pShader->InitUniform("specfactor", CGLSLShader::UNIFORM_FLOAT1);
		
		if(m_isCubemappingSupported)
		{
			m_attribs.u_modelmatrix = m_pShader->InitUniform("modelmatrix", CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.u_inv_modelmatrix = m_pShader->InitUniform("inv_modelmatrix", CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.u_cubemapstrength = m_pShader->InitUniform("cubemapstrength", CGLSLShader::UNIFORM_FLOAT1);
		}

		m_attribs.u_decalalpha = m_pShader->InitUniform("decalalpha", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_decalscale = m_pShader->InitUniform("decalscale", CGLSLShader::UNIFORM_NOSYNC);

		m_attribs.u_baselightmap = m_pShader->InitUniform("baselightmap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_maintexture = m_pShader->InitUniform("maintexture", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_detailtex = m_pShader->InitUniform("detailtex", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_chrometex = m_pShader->InitUniform("chrometex", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_normalmap = m_pShader->InitUniform("normalmap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_luminance = m_pShader->InitUniform("luminance", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_aomap = m_pShader->InitUniform("aomaptex", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_difflightmap = m_pShader->InitUniform("difflightmap", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_lightvecstex = m_pShader->InitUniform("lightvecstex", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_specular = m_pShader->InitUniform("speculartex", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
		m_attribs.u_light_radius = m_pShader->InitUniform("light_radius", CGLSLShader::UNIFORM_FLOAT1);

		if(m_isCubemappingSupported)
		{
			m_attribs.u_cubemap = m_pShader->InitUniform("cubemap", CGLSLShader::UNIFORM_INT1);
			m_attribs.u_cubemap_prev = m_pShader->InitUniform("cubemap_prev", CGLSLShader::UNIFORM_INT1);
		}

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmatrix, "normalmatrix", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_causticsm1, "caustics_m1", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_causticsm2, "caustics_m2", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_interpolant, "interpolant", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vorigin, "v_origin", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vright, "v_right", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_uvoffset, "uvoffset", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_phong_exponent, "phong_exponent", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_specularfactor, "specfactor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_decalalpha, "decalalpha", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_decalscale, "decalscale", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_baselightmap, "baselightmap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_maintexture, "maintexture", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_detailtex, "detailtex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_chrometex, "chrometex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_normalmap, "normalmap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_aomap, "aomaptex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_difflightmap, "difflightmap", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_lightvecstex, "lightvecstex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_specular, "speculartex", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_color, "color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_light_radius, "light_radius", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup))
			return false;

		for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
		{
			CString lightcolor;
			lightcolor << "light_" << i << "_color";

			CString lightorigin;
			lightorigin << "light_" << i << "_origin";

			CString lightradius;
			lightradius << "light_" << i << "_radius";

			CString lightcubemap;
			lightcubemap << "light_" << i << "_cubemap";

			CString lightprojtexture;
			lightprojtexture << "light_" << i << "_projtexture";

			CString lightshadowmap;
			lightshadowmap << "light_" << i << "_shadowmap";

			CString lightmatrix;
			lightmatrix << "light_" << i << "_matrix";

			CString lightdeterminatorshadowmap;
			lightdeterminatorshadowmap << "light" << i << "_shadowmap";

			m_attribs.lights[i].u_light_color = m_pShader->InitUniform(lightcolor.c_str(), CGLSLShader::UNIFORM_FLOAT4);
			m_attribs.lights[i].u_light_origin = m_pShader->InitUniform(lightorigin.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			m_attribs.lights[i].u_light_radius = m_pShader->InitUniform(lightradius.c_str(), CGLSLShader::UNIFORM_FLOAT1);
			m_attribs.lights[i].u_light_cubemap = m_pShader->InitUniform(lightcubemap.c_str(), CGLSLShader::UNIFORM_INT1);
			m_attribs.lights[i].u_light_projtexture = m_pShader->InitUniform(lightprojtexture.c_str(), CGLSLShader::UNIFORM_INT1);
			m_attribs.lights[i].u_light_shadowmap = m_pShader->InitUniform(lightshadowmap.c_str(), CGLSLShader::UNIFORM_INT1);
			m_attribs.lights[i].u_light_matrix = m_pShader->InitUniform(lightmatrix.c_str(), CGLSLShader::UNIFORM_MATRIX4);
			m_attribs.lights[i].d_light_shadowmap = m_pShader->GetDeterminatorIndex(lightdeterminatorshadowmap.c_str());

			if(!R_CheckShaderUniform(m_attribs.lights[i].u_light_color, lightcolor.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.lights[i].u_light_origin, lightorigin.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.lights[i].u_light_radius, lightradius.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.lights[i].u_light_cubemap, lightcubemap.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.lights[i].u_light_projtexture, lightprojtexture.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.lights[i].u_light_shadowmap, lightshadowmap.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.lights[i].u_light_matrix, lightmatrix.c_str(), m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderDeterminator(m_attribs.lights[i].d_light_shadowmap, lightdeterminatorshadowmap.c_str(), m_pShader, Sys_ErrorPopup))
				return false;
		}

		if(m_isCubemappingSupported)
		{
			if(!R_CheckShaderUniform(m_attribs.u_cubemapstrength, "cubemapstrength", m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.u_cubemap, "cubemap", m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.u_cubemap_prev, "cubemap_prev", m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.u_inv_modelmatrix, "inv_modelmatrix", m_pShader, Sys_ErrorPopup)
				|| !R_CheckShaderUniform(m_attribs.u_modelmatrix, "modelmatrix", m_pShader, Sys_ErrorPopup))
			return false;
		}

		// Disable these by default
		if(m_isCubemappingSupported)
		{
			m_pShader->DisableSync(m_attribs.u_modelmatrix);
			m_pShader->DisableSync(m_attribs.u_inv_modelmatrix);
		}
	}

	if(CL_IsGameActive())
	{
		VID_DrawLoadingScreen("Reloading world geometry");

		// Reload textures
		LoadTextures();

		// Init lightmap
		InitLightmaps();

		// Create VBO
		InitVBO();

		// Delete WAD resource
		if(ens.pwadresource)
		{
			delete ens.pwadresource;
			ens.pwadresource = nullptr;
		}

		// Rebind decal VBO
		m_pDecalVBO->RebindGL();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::ClearGL( void ) 
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

	if(m_pDecalVBO)
		m_pDecalVBO->ClearGL();

	// Clear these
	memset(m_lightmapIndexes, 0, sizeof(m_lightmapIndexes));
	memset(m_ambientLightmapIndexes, 0, sizeof(m_ambientLightmapIndexes));
	memset(m_diffuseLightmapIndexes, 0, sizeof(m_diffuseLightmapIndexes));
	memset(m_lightVectorsIndexes, 0, sizeof(m_lightVectorsIndexes));
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::LoadTextures( void ) 
{
	VID_DrawLoadingScreen("Loading WAD files");

	// Get the list of WAD files
	CArray<CString> wadFilesList;
	if(!Common::GetWADList(ens.pworld->pentdata, wadFilesList))
	{
		Con_EPrintf("%s - Failed to get WAD list for '%s'.\n", __FUNCTION__, ens.pworld->name.c_str());
		wadFilesList.clear();
	}

	if(!ens.pwadresource)
	{
		// WAD texture managing object
		ens.pwadresource = new CWADTextureResource();
		if(!ens.pwadresource->Init(
			ens.pworld->name.c_str(), 
			wadFilesList, 
			(g_pCvarWadTextureChecks->GetValue() >= 1) ? true : false,
			(g_pCvarBspTextureChecks->GetValue() >= 1) ? true : false))
		{
			Con_Printf("%s - Failed to set up wad textures.\n", __FUNCTION__);
		}
	}

	VID_DrawLoadingScreen("Loading world textures");

	// Load textures
	InitTextures(*ens.pwadresource, wadFilesList);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	m_pChromeTexture = pTextureManager->LoadTexture("general/chrome.DDS", RS_GAME_LEVEL);
	if(!m_pChromeTexture)
		m_pChromeTexture = pTextureManager->GetDummyTexture();
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::InitGame( void ) 
{
	// allocate surface array
	m_surfacesArray.resize(ens.pworld->numsurfaces);

	// Load textures
	LoadTextures();

	// Set lightmap texcoords
	SetLightmapCoords();

	// Init the VBO
	InitVBO();

	// Create decal VBO
	InitDecalVBO();

	// Set up lightmap
	InitLightmaps();

	// Set ptr to lightstyles array
	m_pLightStyleValuesArray = gLightStyles.GetLightStyleValuesArray();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::ClearGame( void ) 
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

	if(m_pDecalVBO)
	{
		delete m_pDecalVBO;
		m_pDecalVBO = nullptr;
	}

	m_vertexCacheBase = 0;
	m_vertexCacheIndex = 0;
	m_vertexCacheSize = 0;

	memset(m_lightmapIndexes, 0, sizeof(m_lightmapIndexes));
	memset(m_ambientLightmapIndexes, 0, sizeof(m_ambientLightmapIndexes));
	memset(m_diffuseLightmapIndexes, 0, sizeof(m_diffuseLightmapIndexes));
	memset(m_lightVectorsIndexes, 0, sizeof(m_lightVectorsIndexes));

	for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
	{
		m_lightmapWidths[i] = 0;
		m_lightmapHeights[i] = 0;
	}

	m_bumpMaps = false;
	m_addMulti = false;
	m_multiPass = false;
	m_useLightStyles = false;

	m_pCurrentEntity = nullptr;

	if(!m_surfacesArray.empty())
		m_surfacesArray.clear();

	if(!m_texturesArray.empty())
		m_texturesArray.clear();

	if(!m_staticDecalsArray.empty())
	{
		for(Uint32 i = 0; i < m_staticDecalsArray.size(); i++)
			delete m_staticDecalsArray[i];

		m_staticDecalsArray.clear();
	}

	if(!m_decalsList.empty())
	{
		m_decalsList.begin();
		while(!m_decalsList.end())
		{
			bsp_decal_t* pdecal = m_decalsList.get();
			m_decalsList.remove(m_decalsList.get_link());
			delete pdecal;

			m_decalsList.next();
		}

		m_decalsList.clear();
	}
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::SetLightmapCoords( void ) 
{
	// Set default height
	for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
	{
		// Set to the default
		m_lightmapWidths[i] = LIGHTMAP_DEFAULT_WIDTH;
		m_lightmapHeights[i] = LIGHTMAP_DEFAULT_HEIGHT;

		Uint32* pallocations = new Uint32[LIGHTMAP_DEFAULT_WIDTH];
		memset(pallocations, 0, sizeof(Uint32)*LIGHTMAP_DEFAULT_WIDTH);

		// Allocate lightmap positions first
		for(Uint32 j = 0; j < ens.pworld->numsurfaces; j++)
		{
			msurface_t* psurface = &ens.pworld->psurfaces[j];
			if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			// Determine sizes
			Uint32 xsize = (psurface->extents[0] / psurface->lightmapdivider)+1;
			Uint32 ysize = (psurface->extents[1] / psurface->lightmapdivider)+1;

			// Skip empty styles
			if(i > BASE_LIGHTMAP_INDEX && psurface->styles[i] == NULL_LIGHTSTYLE_INDEX)
				continue;

			// Allocate lightmap slot
			Uint32 light_s, light_t;
			R_AllocBlock(xsize, ysize, light_s, light_t, m_lightmapWidths[i], m_lightmapHeights[i], pallocations);

			bsp_surface_t* pbspsurface = &m_surfacesArray[j];
			psurface->light_s[i] = pbspsurface->light_s[i] = light_s;
			psurface->light_t[i] = pbspsurface->light_t[i] = light_t;
		}

		delete[] pallocations;
	}
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::InitLightmaps( void ) 
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	
	// Get overdarken treshold
	Float overdarken = g_pCvarOverdarkenTreshold->GetValue();
	if(overdarken < 0)
		overdarken = 0;

	// Reset this
	m_bumpMaps = false;
	m_useLightStyles = false;

	Uint32 lightmapDataTotal = 0;

	//
	// Initialize the basic lightmap first
	//
	for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
	{
		// alloc default lightmap's data
		Uint32 lightmapdatasize = 0;
		color32_t* plightmap = new color32_t[m_lightmapWidths[i]*m_lightmapHeights[i]];
		memset(plightmap, 0, sizeof(color32_t)*m_lightmapWidths[i]*m_lightmapHeights[i]);

		// Process the surfaces
		for(Uint32 j = 0; j < ens.pworld->numsurfaces; j++)
		{
			const msurface_t* psurface = &ens.pworld->psurfaces[j];
			if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			bsp_surface_t* pbspsurface = &m_surfacesArray[j];
		
			// Skip empty styles
			if(i > BASE_LIGHTMAP_INDEX && psurface->styles[i] == NULL_LIGHTSTYLE_INDEX)
				continue;

			bool isfullbright = false;
			if(psurface->infoindex != NO_INFO_INDEX)
			{
				bsp_texture_t* ptexture = pbspsurface->ptexture;
				if(ptexture && ptexture->pmaterial && (ptexture->pmaterial->flags & TX_FL_FULLBRIGHT))
					isfullbright = true;
			}

			// Determine sizes
			Uint32 xsize = (psurface->extents[0] / psurface->lightmapdivider)+1;
			Uint32 ysize = (psurface->extents[1] / psurface->lightmapdivider)+1;
			Uint32 size = xsize*ysize;

			Float overdarkValue = (i == BASE_LIGHTMAP_INDEX) ? overdarken : 0;

			// Build the base lightmap
			color24_t* psrclightdata;
			if(ens.pworld->plightdata[SURF_LIGHTMAP_DEFAULT])
				psrclightdata = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(ens.pworld->plightdata[SURF_LIGHTMAP_DEFAULT]) + psurface->lightoffset);
			else
				psrclightdata = nullptr;

			R_BuildLightmap(pbspsurface->light_s[i], pbspsurface->light_t[i], psrclightdata, psurface, plightmap, i, m_lightmapWidths[i], overdarkValue, false, isfullbright);
			lightmapdatasize += size*sizeof(color32_t);
		}

		if(i > BASE_LIGHTMAP_INDEX && lightmapdatasize <= 0)
		{
			delete[] plightmap;
			continue;
		}

		if(g_pCvarDumpLightmaps->GetValue() >= 1)
		{
			CString basename;
			Common::Basename(ens.pworld->name.c_str(), basename);

			CString directoryPath;
			directoryPath << "dumps" << PATH_SLASH_CHAR << "lightmaps" << PATH_SLASH_CHAR << basename << PATH_SLASH_CHAR;
			if(FL_CreateDirectory(directoryPath.c_str()))
			{
				CString filepath;
				filepath << directoryPath << PATH_SLASH_CHAR << "dump_lightmap_default_layer_" << i << ".tga";

				const byte* pwritedata = reinterpret_cast<const byte*>(plightmap);
				if(TGA_Write(pwritedata, 4, m_lightmapWidths[i], m_lightmapHeights[i], filepath.c_str(), FL_GetInterface(), Con_Printf))
					Con_Printf("Exported %s.\n", filepath.c_str());
			}
			else
			{
				Con_Printf("%s - Failed to create directory '%s'.\n", __FUNCTION__, directoryPath.c_str());
			}
		}

		// Set default lightmap
		if(!m_lightmapIndexes[i])
			m_lightmapIndexes[i] = pTextureManager->GenTextureIndex(RS_GAME_LEVEL)->gl_index;

		Uint32 resultsize;
		R_SetLightmapTexture(m_lightmapIndexes[i], m_lightmapWidths[i], m_lightmapHeights[i], false, plightmap, resultsize);
		Con_Printf("Loaded 1 lightmaps for default layer %d: %.2f mbytes.\n", i, static_cast<Float>(resultsize)/(1024.0f*1024.0f));
		lightmapDataTotal += resultsize;

		if(i > 0)
			m_useLightStyles = true;

		delete[] plightmap;
	}

	//
	// Now process any bump mapped ones
	//
	
	if(g_pCvarBumpMaps->GetValue() >= 1
		&& ens.pworld->plightdata[SURF_LIGHTMAP_AMBIENT]
		&& ens.pworld->plightdata[SURF_LIGHTMAP_DIFFUSE]
		&& ens.pworld->plightdata[SURF_LIGHTMAP_VECTORS])
	{
		for(Uint32 k = 1; k < NB_SURF_LIGHTMAP_LAYERS; k++)
		{
			// Determine specifics
			Float _overdarken = (k == SURF_LIGHTMAP_AMBIENT) ? overdarken : 0;
			bool isvectormap = (k == SURF_LIGHTMAP_VECTORS) ? true : false;

			CString lmapname;
			switch(k)
			{
			case SURF_LIGHTMAP_VECTORS:
				lmapname = "vectors";
				break;
			case SURF_LIGHTMAP_AMBIENT:
				lmapname = "ambient";
				break;
			case SURF_LIGHTMAP_DIFFUSE:
				lmapname = "diffuse";
				break;
			}

			for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
			{
				// alloc ambient lightmap's data
				Uint32 lightdatasize = 0;
				color32_t* plightmapdata = new color32_t[m_lightmapWidths[i]*m_lightmapHeights[i]];
				memset(plightmapdata, 0, sizeof(color32_t)*m_lightmapWidths[i]*m_lightmapHeights[i]);

				// Process the surfaces
				for(Uint32 j = 0; j < ens.pworld->numsurfaces; j++)
				{
					const msurface_t* psurface = &ens.pworld->psurfaces[j];
					if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
						continue;

					// Skip empty styles
					if(i > BASE_LIGHTMAP_INDEX && psurface->styles[i] == NULL_LIGHTSTYLE_INDEX)
						continue;

					bsp_surface_t* pbspsurface = &m_surfacesArray[j];
		
					bool isfullbright = false;
					if(psurface->infoindex != NO_INFO_INDEX)
					{
						bsp_texture_t* ptexture = pbspsurface->ptexture;
						if(ptexture && ptexture->pmaterial && (ptexture->pmaterial->flags & TX_FL_FULLBRIGHT))
							isfullbright = true;
					}

					// Determine sizes
					Uint32 xsize = (psurface->extents[0] / psurface->lightmapdivider)+1;
					Uint32 ysize = (psurface->extents[1] / psurface->lightmapdivider)+1;
					Uint32 size = xsize*ysize;

					color24_t* psrc = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(ens.pworld->plightdata[k]) + psurface->lightoffset);
					R_BuildLightmap(pbspsurface->light_s[i], pbspsurface->light_t[i], psrc, psurface, plightmapdata, i, m_lightmapWidths[i], _overdarken, isvectormap);
					lightdatasize += size*sizeof(color32_t);
				}

				if(i > BASE_LIGHTMAP_INDEX && lightdatasize <= 0)
				{
					delete[] plightmapdata;
					continue;
				}

				if(g_pCvarDumpLightmaps->GetValue() >= 1)
				{
					CString basename;
					Common::Basename(ens.pworld->name.c_str(), basename);

					CString directoryPath;
					directoryPath << "dumps" << PATH_SLASH_CHAR << "lightmaps" << PATH_SLASH_CHAR << basename << PATH_SLASH_CHAR;

					if(FL_CreateDirectory(directoryPath.c_str()))
					{
						// Write file
						CString filepath;
						filepath << directoryPath << "dump_" << basename << "_lightmap_" << lmapname << "_layer_" << i << ".tga";

						const byte* pwritedata = reinterpret_cast<const byte*>(plightmapdata);
						if(TGA_Write(pwritedata, 4, m_lightmapWidths[i], m_lightmapHeights[i], filepath.c_str(), FL_GetInterface(), Con_Printf))
							Con_Printf("Exported %s.\n", filepath.c_str());
					}
					else
					{
						Con_Printf("%s - Failed to create directory '%s'.\n", __FUNCTION__, directoryPath.c_str());
					}
				}

				Uint32* pdestindex = nullptr;
				switch(k)
				{
				case SURF_LIGHTMAP_VECTORS:
					pdestindex = &m_lightVectorsIndexes[i];
					break;
				case SURF_LIGHTMAP_AMBIENT:
					pdestindex = &m_ambientLightmapIndexes[i];
					break;
				case SURF_LIGHTMAP_DIFFUSE:
					pdestindex = &m_diffuseLightmapIndexes[i];
					break;
				}

				// Load the ambient lightmap
				if(!(*pdestindex))
					(*pdestindex) = pTextureManager->GenTextureIndex(RS_GAME_LEVEL)->gl_index;

				Uint32 resultsize;
				R_SetLightmapTexture((*pdestindex), m_lightmapWidths[i], m_lightmapHeights[i], isvectormap, plightmapdata, resultsize);
				Con_Printf("Loaded 1 lightmaps for %s layer %d: %.2f mbytes.\n", lmapname.c_str(), i, static_cast<Float>(resultsize)/(1024.0f*1024.0f));
				lightmapDataTotal += resultsize;

				if(!m_bumpMaps)
					m_bumpMaps = true;
			}
		}
	}

	Con_Printf("Done loading lightmaps: %.2f mbytes loaded total.\n", static_cast<Float>(lightmapDataTotal)/(1024.0f*1024.0f));

	glBindTexture(GL_TEXTURE_2D, 0);
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::InitVBO( void ) 
{
	if(ens.isloading)
		VID_DrawLoadingScreen("Loading world geometry");

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}

	Uint32 numWorldVertexes = 0;
	Uint32 numVertexes = 0;
	Uint32 curVertexIndex = 0;

	Uint32 numIndexes = 0;
	Uint32 curIndex = 0;

	// Calculate needed sizes
	for(Uint32 i = 0; i < ens.pworld->numsurfaces; i++)
	{
		const msurface_t* psurface = &ens.pworld->psurfaces[i];
		if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;

		numWorldVertexes += psurface->numedges;
		numIndexes += 3+(psurface->numedges-3)*3;
	}

	// Set this for full vbo
	numVertexes = numWorldVertexes;

	// create smoothing object
	CNormalSmoothing *psmoothing = nullptr;
	if(m_pCvarNormalBlendAngle->GetValue() > 0)
	{
		Vector totalMins(NULL_MINS);
		Vector totalMaxs(NULL_MAXS);

		// Determine total world mins/maxs including brushmodels
		for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
		{
			cache_model_t* pmodel = gModelCache.GetModelByIndex(i+1);
			if(!pmodel || pmodel->type != MOD_BRUSH)
				continue;

			for(int j = 0; j < 3; j++)
			{
				if(pmodel->mins[j] < totalMins[j])
					totalMins[j] = pmodel->mins[j];

				if(pmodel->maxs[j] > totalMaxs[j])
					totalMaxs[j] = pmodel->maxs[j];
			}

			// Pad by 32 for vertexes that are edge cases
			Math::VectorAdd(totalMaxs, Vector(32, 32, 32), totalMaxs);
			Math::VectorSubtract(totalMins, Vector(32, 32, 32), totalMins);
		}

		// Create smoothing object
		psmoothing = new CNormalSmoothing(totalMins, totalMaxs, numWorldVertexes, m_pCvarNormalBlendAngle->GetValue());
	}

	// Create the arrays
	bsp_vertex_t* pvertexes = new bsp_vertex_t[numVertexes];
	Uint32* pindexes = new Uint32[numIndexes];

	// Organize triangles by textures for performance
	for(Uint32 i = 0; i < ens.pworld->numtextures; i++)
	{
		mtexture_t* ptexture = &ens.pworld->ptextures[i];

		// Link with the texture entry
		if(ptexture->infoindex == NO_INFO_INDEX)
		{
			Con_EPrintf("Texture '%s' not linked to info in BSP.\n", ptexture->name.c_str());
			continue;
		}

		// Set up the surfaces
		for(Uint32 j = 0; j < ens.pworld->numsurfaces; j++)
		{
			msurface_t* psurface = &ens.pworld->psurfaces[j];
			if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			// Do not add yet if it isn't tied to the current texture
			if(psurface->ptexinfo->ptexture != ptexture)
				continue;

			bsp_surface_t* pbspsurface = &m_surfacesArray[j];

			// link with model_t surface
			pbspsurface->pmsurface = psurface;
			psurface->infoindex = j;

			Uint32 vertexBase = curVertexIndex;
			pbspsurface->start_index = curIndex;

			// Link it to the texture
			pbspsurface->ptexture = &m_texturesArray[ptexture->infoindex];

			for(Uint32 k = 0; k < psurface->numedges; k++)
			{
				Vector vertexOrigin;
				Int32 edgeIndex = ens.pworld->psurfedges[psurface->firstedge+k];
				if(edgeIndex > 0)
					vertexOrigin = ens.pworld->pvertexes[ens.pworld->pedges[edgeIndex].vertexes[0]].origin;
				else
					vertexOrigin = ens.pworld->pvertexes[ens.pworld->pedges[-edgeIndex].vertexes[1]].origin;

				// allocate the new vertex
				bsp_vertex_t& vertex = pvertexes[curVertexIndex];
				curVertexIndex++;

				// Set origin
				for(Uint32 l = 0; l < 3; l++)
					vertex.origin[l] = vertexOrigin[l];

				vertex.origin[3] = 1.0;

				if(rns.fog.specialfog)
					vertex.fogcoord = CalcFogCoord(vertex.origin[2]);

				// Store tangents
				mtexinfo_t* ptexinfo = psurface->ptexinfo;
				Math::VectorCopy(ptexinfo->vecs[0], vertex.tangent);
				Math::VectorNormalize(vertex.tangent);
				Math::VectorCopy(ptexinfo->vecs[1], vertex.binormal);
				Math::VectorNormalize(vertex.binormal);

				// Store normal
				Math::VectorCopy(psurface->pplane->normal, vertex.normal);
				if(psurface->flags & SURF_PLANEBACK)
				{
					for(Uint32 l = 0; l < 3; l++)
						vertex.normal[l] *= -1;
				}

				if(psmoothing)
					psmoothing->ManageVertex(vertexOrigin, vertex.normal, curVertexIndex-1);

				// Set texcoords
				vertex.texcoord[0] = Math::DotProduct(vertexOrigin, ptexinfo->vecs[0])+ptexinfo->vecs[0][3];
				vertex.texcoord[0] /= static_cast<Float>(psurface->ptexinfo->ptexture->width);

				vertex.texcoord[1] = Math::DotProduct(vertexOrigin, ptexinfo->vecs[1])+ptexinfo->vecs[1][3];
				vertex.texcoord[1] /= static_cast<Float>(psurface->ptexinfo->ptexture->height);

				// Set detail texcoords if needed
				if(pbspsurface->ptexture->pmaterial->ptextures[MT_TX_DETAIL])
				{
					vertex.dtexcoord[0] = vertex.texcoord[0]*pbspsurface->ptexture->pmaterial->dt_scalex*m_pCvarDetailScale->GetValue();
					vertex.dtexcoord[1] = vertex.texcoord[1]*pbspsurface->ptexture->pmaterial->dt_scaley*m_pCvarDetailScale->GetValue();
				}

				for(Uint32 l = 0; l < MAX_SURFACE_STYLES; l++)
				{
					// Set lightmap coords
					vertex.lmapcoord[l][0] = Math::DotProduct(vertexOrigin, ptexinfo->vecs[0]) + ptexinfo->vecs[0][3];
					vertex.lmapcoord[l][0] -= psurface->texturemins[0];
					vertex.lmapcoord[l][0] += pbspsurface->light_s[l]*psurface->lightmapdivider + (psurface->lightmapdivider / 2.0f);
					vertex.lmapcoord[l][0] /= m_lightmapWidths[l]*psurface->lightmapdivider;

					vertex.lmapcoord[l][1] = Math::DotProduct(vertexOrigin, ptexinfo->vecs[1]) + ptexinfo->vecs[1][3];
					vertex.lmapcoord[l][1] -= psurface->texturemins[1];
					vertex.lmapcoord[l][1] += pbspsurface->light_t[l]*psurface->lightmapdivider + (psurface->lightmapdivider / 2.0f);
					vertex.lmapcoord[l][1] /= m_lightmapHeights[l]*psurface->lightmapdivider;
				}
			}

			// Set indexes
			Uint32 indexes[3] = { 0 };
			for(Uint32 k = 0; k < 3; k++)
			{
				indexes[k] = vertexBase + k;
				pindexes[curIndex] = indexes[k];
				curIndex++;
			}

			// Break the triangle fan into raw triangles
			for(Uint32 k = 0, l = 3; k < (psurface->numedges-3); k++, l++)
			{
				indexes[1] = indexes[2];
				indexes[2] = vertexBase+l;

				pindexes[curIndex++] = indexes[0];
				pindexes[curIndex++] = indexes[1];
				pindexes[curIndex++] = indexes[2];
			}

			// Set end index
			pbspsurface->end_index = curIndex;
			pbspsurface->num_indexes = curIndex - pbspsurface->start_index;

			// Calculate mins/maxs
			Vector mins = NULL_MINS;
			Vector maxs = NULL_MAXS;
			for(Uint32 k = 0; k < pbspsurface->end_index-pbspsurface->start_index; k++)
			{
				bsp_vertex_t *pvertex = &pvertexes[pindexes[pbspsurface->start_index+k]];
				for(Int32 l = 0; l < 3; l++)
				{
					if(mins[l] > pvertex->origin[l])
						mins[l] = pvertex->origin[l]-1;

					if(maxs[l] < pvertex->origin[l])
						maxs[l] = pvertex->origin[l]+1;
				}
			}

			Math::VectorCopy(mins, pbspsurface->mins);
			Math::VectorCopy(maxs, pbspsurface->maxs);
		}
	}

	// Set smoothed normals
	if(psmoothing)
	{
		for(Uint32 i = 0; i < numWorldVertexes; i++)
		{
			bsp_vertex_t* pvertex = &pvertexes[i];

			// If we have a smoothed normal, then apply it over the original
			const Vector* pnormal = psmoothing->GetVertexNormal(i);
			if(pnormal)
				pvertex->normal = *pnormal;

			Vector tangent, binormal;
			for(int j = 0; j < 3; j++)
			{
				tangent[j] = (pvertex->tangent[j] - pvertex->normal[j] * Math::DotProduct(pvertex->normal, pvertex->tangent));
				binormal[j] = (pvertex->binormal[j] - pvertex->normal[j] * Math::DotProduct(pvertex->normal, pvertex->binormal));
			}

			Math::VectorNormalize(tangent);
			Math::VectorCopy(tangent, pvertex->tangent);

			Math::VectorNormalize(binormal);
			Math::VectorCopy(binormal, pvertex->binormal);
		}

		delete psmoothing;
	}

	// Set the VBO
	m_pVBO = new CVBO(gGLExtF, pvertexes, sizeof(bsp_vertex_t)*numVertexes, pindexes, sizeof(Uint32)*numIndexes);

	delete[] pvertexes;
	delete[] pindexes;

	rns.fog.prevspecialfog = rns.fog.specialfog;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::InitDecalVBO( void )
{
	if(m_pDecalVBO)
		delete m_pDecalVBO;

	// Set the decal cache
	m_vertexCacheBase = 0; // We use separate buffers now
	m_vertexCacheIndex = m_vertexCacheBase;
	m_vertexCacheSize = NB_BSP_DECAL_VERTS;

	// Set the VBO
	bsp_vertex_t* pvertexes = new bsp_vertex_t[m_vertexCacheSize];
	m_pDecalVBO = new CVBO(gGLExtF, pvertexes, sizeof(bsp_vertex_t)*m_vertexCacheSize, nullptr, 0, true);
	delete[] pvertexes;
}

//=============================================
// @brief
//
//=============================================
en_material_t* CBSPRenderer::LoadMapTexture( CWADTextureResource& wadTextures, const CArray<CString>& wadFilesList, const Char* pstrtexturename )
{
	// First try loading it under "world" as a normal material
	CString materialPath;
	materialPath << WORLD_TEXTURES_PATH_BASE << pstrtexturename << PMF_FORMAT_EXTENSION;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	en_material_t* pmaterial = pTextureManager->LoadMaterialScript(materialPath.c_str(), RS_GAME_LEVEL, false);
	if(!pmaterial)
	{
		CString folderPath = WAD_GetWADFolderPath(ens.pworld->name.c_str(), WORLD_TEXTURES_PATH_BASE);
		materialPath = WAD_GetWADTexturePath(folderPath.c_str(), pstrtexturename);

		pmaterial = pTextureManager->LoadMaterialScript(materialPath.c_str(), RS_GAME_LEVEL, false);
		if(!pmaterial)
		{
			// Look under WAD paths
			for(Uint32 i = 0; i < wadFilesList.size(); i++)
			{
				folderPath = WAD_GetWADFolderPath(wadFilesList[i].c_str(), WORLD_TEXTURES_PATH_BASE);
				materialPath = WAD_GetWADTexturePath(folderPath.c_str(), pstrtexturename);

				pmaterial = pTextureManager->LoadMaterialScript(materialPath.c_str(), RS_GAME_LEVEL, false);
				if(pmaterial)
					break;
			}
		}
	}

	if(!pmaterial)
	{
		// Just get the dummy material
		pmaterial = pTextureManager->GetDummyMaterial();
	}
	else if(!pmaterial->ptextures[MT_TX_DIFFUSE] && !pmaterial->containername.empty())
	{
		// Load the texture from the WAD
		pmaterial->ptextures[MT_TX_DIFFUSE] = wadTextures.GetWADTexture(pmaterial, pmaterial->containername.c_str(), pmaterial->containertexturename.c_str());
	}

	// Make sure this is set
	if(!pmaterial->ptextures[MT_TX_DIFFUSE])
		pmaterial->ptextures[MT_TX_DIFFUSE] = pTextureManager->GetDummyTexture();

	return pmaterial;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::InitTextures( CWADTextureResource& wadTextures, const CArray<CString>& wadFilesList ) 
{
	if(m_texturesArray.empty())
	{
		// allocate array size
		m_texturesArray.resize(ens.pworld->numtextures);
	}

	for(Uint32 i = 0; i < ens.pworld->numtextures; i++)
	{
		mtexture_t* ptexture = &ens.pworld->ptextures[i];
		bsp_texture_t* pbsptexture = &m_texturesArray[i];

		// link it up
		ptexture->infoindex = i;
		pbsptexture->pmodeltexture = ptexture;

		pbsptexture->numlightbatches = 0;
		pbsptexture->nummultibatches = 0;
		pbsptexture->numsinglebatches = 0;
		
		pbsptexture->index = i;
		pbsptexture->psurfchain = nullptr;

		// Load the texture
		pbsptexture->pmaterial = LoadMapTexture(wadTextures, wadFilesList, ptexture->name.c_str());

		// See how many surfaces are tied to this texture
		Uint32 numsurfaces = 0;
		for(Uint32 j = 0; j < ens.pworld->numsurfaces; j++)
		{
			const msurface_t* psurface = &ens.pworld->psurfaces[j];
			if(psurface->ptexinfo->ptexture->infoindex != static_cast<Int32>(i))
				continue;

			for(Uint32 k = 1; k < MAX_SURFACE_STYLES; k++)
			{
				if(psurface->styles[k] != NULL_LIGHTSTYLE_INDEX)
				{
					// Allocate if needed
					if(pbsptexture->lightstyleinfos.empty())
						pbsptexture->lightstyleinfos.resize(CLightStyleManager::CUSTOM_LIGHTSTYLE_START_INDEX);

					// Allocate batches also if needed
					lightstyleinfo_t& styleInfo = pbsptexture->lightstyleinfos[psurface->styles[k]];
					if(styleInfo.stylebatches.empty())
						styleInfo.stylebatches.resize(MAX_SURFACE_STYLES);
				}
			}

			numsurfaces++;
		}

		// Set drawbatch array sizes
		pbsptexture->light_batches.resize(numsurfaces);
		pbsptexture->single_batches.resize(numsurfaces);
		pbsptexture->multi_batches.resize(numsurfaces);

		// Allocate for each style layer of each style
		for(Uint32 j = 0; j < pbsptexture->lightstyleinfos.size(); j++)
		{
			lightstyleinfo_t& styleInfo = pbsptexture->lightstyleinfos[j];
			if(styleInfo.stylebatches.empty())
				continue;

			for(Uint32 k = 1; k < MAX_SURFACE_STYLES; k++)
				styleInfo.stylebatches[k].batches.resize(numsurfaces);
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawNormal( void ) 
{
	// Set shader's VBO and bind it
	m_pShader->SetVBO(m_pVBO);
	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_position);

	// Set projection
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Draw the world next if we didn't fail
	bool result = DrawWorld();

	// Make sure this is reset
	if(result && !m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
		result = false;

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	// Error might be thrown by vbm renderer from skybox draw, so check
	// the BSP shader if it has any errors
	if(!result)
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());

	return result;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawTransparent( void ) 
{
	if(m_pCvarDrawWorld->GetValue() < 1)
		return true;

	// Set shader's VBO and bind it
	m_pShader->SetVBO(m_pVBO);
	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_position);

	// Set projection
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Check for shader errors
	bool result = true;
	// Do not allow the use of multipass rendering for transparents
	m_disableMultiPass = true;

	// Draw static entities first
	if(g_pCvarDrawEntities->GetValue() > 0)
	{
		for(Uint32 i = 0; i < rns.objects.numvisents; i++)
		{
			cl_entity_t& entity = *rns.objects.pvisents[i];

			if(!entity.pmodel || entity.pmodel->type != MOD_BRUSH)
				continue;

			if(!R_IsEntityTransparent(entity)
				|| R_IsSpecialRenderEntity(entity))
				continue;

			// Handle skydraw specially
			if (rns.water_skydraw)
			{
				if (entity.curstate.renderfx != RenderFx_SkyEnt
					&& entity.curstate.renderfx != RenderFx_SkyEntScaled)
					continue;
			}
			else
			{
				if (entity.curstate.renderfx == RenderFx_SkyEnt
					|| entity.curstate.renderfx == RenderFx_SkyEntScaled)
					continue;
			}

			// Handle portals specially
			if (rns.portalpass)
			{
				if (entity.curstate.renderfx != RenderFx_InPortalEntity
					&& entity.curstate.renderfx != RenderFx_InPortalScaledModel)
					continue;
			}
			else
			{
				if (entity.curstate.renderfx == RenderFx_InPortalEntity
					|| entity.curstate.renderfx == RenderFx_InPortalScaledModel)
					continue;
			}

			// Never allow no-depth cull entities to be rendered here
			if (entity.curstate.renderfx == RenderFx_SkyEntNC)
				continue;

			result = DrawBrushModel(entity, false);
			if(!result)
				break;
		}
	}

	// Reset everything after rendering transparents
	m_disableMultiPass = false;
	m_pVBO->UnBind();

	// Draw decals last
	if(result)
	{
		// Set shader's VBO and bind it
		m_pDecalVBO->Bind();
		m_pShader->SetVBO(m_pDecalVBO);

		result = DrawDecals(true);

		// Disable VBO
		m_pDecalVBO->UnBind();
	}

	m_pShader->DisableShader();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	// Clear any binds
	R_ClearBinds();

	if(!result)
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());

	return result;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawSkyBox( bool inZElements ) 
{
	// Set shader's VBO and bind it
	m_pShader->SetVBO(m_pVBO);
	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_position);

	// Set projection
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	m_pCurrentEntity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);

	if(!inZElements)
	{
		if(!Prepare())
			return false;

		RecursiveWorldNode(ens.pworld->pnodes);

		if(!DrawFirst()
			|| !DrawLights(false)
			|| !DrawFinal())
			return false;
	}

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		if(!rns.objects.pvisents[i]->pmodel)
			continue;

		if(rns.objects.pvisents[i]->pmodel->type != MOD_BRUSH)
			continue;

		if(!inZElements)
		{
			if(rns.objects.pvisents[i]->curstate.renderfx != RenderFx_SkyEnt
				&& rns.objects.pvisents[i]->curstate.renderfx != RenderFx_SkyEntScaled)
				continue;
		}
		else
		{
			if(rns.objects.pvisents[i]->curstate.renderfx != RenderFx_SkyEntNC)
				continue;
		}

		if(!DrawBrushModel((*rns.objects.pvisents[i]), false))
			return false;
	}

	// Disable so the others can render
	m_pShader->DisableAttribute(m_attribs.a_position);
	m_pShader->DisableAttribute(m_attribs.a_normal);
	m_pShader->DisableAttribute(m_attribs.a_lmapcoord);
	m_pShader->DisableAttribute(m_attribs.a_texcoord);
	m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
	m_pShader->DisableAttribute(m_attribs.a_fogcoord);
	m_pShader->DisableShader();
	m_pVBO->UnBind();

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawWorld( void ) 
{
	if(m_pCvarDrawWorld->GetValue() < 1)
		return true;

	// Draw world first
	m_pCurrentEntity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);

	if(!Prepare())
		return false;

	RecursiveWorldNode(ens.pworld->pnodes);

	// Draw static entities first
	if(g_pCvarDrawEntities->GetValue() > 0)
	{
		for(Uint32 i = 0; i < rns.objects.numvisents; i++)
		{
			cl_entity_t& entity = *rns.objects.pvisents[i];

			if(!entity.pmodel || entity.pmodel->type != MOD_BRUSH)
				continue;

			// Handle skydraw specially
			if (rns.water_skydraw)
			{
				if (entity.curstate.renderfx != RenderFx_SkyEnt
					&& entity.curstate.renderfx != RenderFx_SkyEntScaled)
					continue;
			}
			else
			{
				if (entity.curstate.renderfx == RenderFx_SkyEnt
					|| entity.curstate.renderfx == RenderFx_SkyEntScaled)
					continue;
			}

			// Handle portals specially
			if (rns.portalpass)
			{
				if (entity.curstate.renderfx != RenderFx_InPortalEntity
					&& entity.curstate.renderfx != RenderFx_InPortalScaledModel)
					continue;
			}
			else
			{
				if (entity.curstate.renderfx == RenderFx_InPortalEntity
					|| entity.curstate.renderfx == RenderFx_InPortalScaledModel)
					continue;
			}

			// Never allow no-depth cull entities to be rendered here
			if (entity.curstate.renderfx == RenderFx_SkyEntNC)
				continue;

			if(R_IsEntityMoved(entity) 
				|| R_IsEntityTransparent(entity)
				|| R_IsSpecialRenderEntity(entity))
				continue;

			if(!DrawBrushModel(entity, true))
				return false;
		}
	}

	// Reset this for texture anims
	m_pCurrentEntity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);

	if(!DrawFirst()
		|| !DrawLights(false)
		|| !DrawFinal())
		return false;

	// Draw moved entities
	if(g_pCvarDrawEntities->GetValue() > 0)
	{
		for(Uint32 i = 0; i < rns.objects.numvisents; i++)
		{
			cl_entity_t& entity = *rns.objects.pvisents[i];

			if(!entity.pmodel || entity.pmodel->type != MOD_BRUSH)
				continue;

			// Handle skydraw specially
			if (rns.water_skydraw)
			{
				if (entity.curstate.renderfx != RenderFx_SkyEnt
					&& entity.curstate.renderfx != RenderFx_SkyEntScaled)
					continue;
			}
			else
			{
				if (entity.curstate.renderfx == RenderFx_SkyEnt
					|| entity.curstate.renderfx == RenderFx_SkyEntScaled)
					continue;
			}

			// Handle portals specially
			if (rns.portalpass)
			{
				if (entity.curstate.renderfx != RenderFx_InPortalEntity
					&& entity.curstate.renderfx != RenderFx_InPortalScaledModel)
					continue;
			}
			else
			{
				if (entity.curstate.renderfx == RenderFx_InPortalEntity
					|| entity.curstate.renderfx == RenderFx_InPortalScaledModel)
					continue;
			}

			// Never allow no-depth cull entities to be rendered here
			if (entity.curstate.renderfx == RenderFx_SkyEntNC)
				continue;

			if(!R_IsEntityMoved(entity) 
				|| R_IsEntityTransparent(entity)
				|| R_IsSpecialRenderEntity(entity))
				continue;

			if(!DrawBrushModel(entity, false))
				return false;
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::Prepare( void ) 
{
	m_multiPass = (!m_disableMultiPass && ((!gDynamicLights.GetLightList().empty() && !R_IsEntityTransparent(*m_pCurrentEntity) 
		|| rns.inwater && !R_IsEntityTransparent(*m_pCurrentEntity)) && g_pCvarDynamicLights->GetValue() >= 1)) ? true : false;

	if(rns.fog.settings.active)
	{
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));

		if(rns.fog.specialfog)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_fogcoord, false))
				return false;

			m_pShader->EnableAttribute(m_attribs.a_fogcoord);
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_radial, false))
				return false;
		}
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false))
			return false;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_specular, false, false))
		return false;

	if(m_isCubemappingSupported)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_OFF, false))
			return false;
	}

	// Load in current modelview
	m_pShader->SetUniformMatrix4fv(m_attribs.u_normalmatrix, rns.view.modelview.GetInverse());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// Clear all chains and batches
	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		bsp_texture_t& texture = m_texturesArray[i];
		texture.numlightbatches = 0;
		texture.numsinglebatches = 0;
		texture.nummultibatches = 0;

		texture.psurfchain = nullptr;

		for(Uint32 j = 0; j < texture.lightstyleinfos.size(); j++)
		{
			lightstyleinfo_t& styleinfo = texture.lightstyleinfos[j];
			if(styleinfo.stylebatches.empty())
				continue;

			for(Uint32 k = 0; k < MAX_SURFACE_STYLES; k++)
				styleinfo.stylebatches[k].numbatches = 0;
		}
	}

	// Reset everything
	for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
	{
		m_pShader->DisableSync(m_attribs.lights[i].u_light_color);
		m_pShader->DisableSync(m_attribs.lights[i].u_light_origin);
		m_pShader->DisableSync(m_attribs.lights[i].u_light_radius);
		m_pShader->DisableSync(m_attribs.lights[i].u_light_cubemap);
		m_pShader->DisableSync(m_attribs.lights[i].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.lights[i].u_light_shadowmap);
		m_pShader->DisableSync(m_attribs.lights[i].u_light_matrix);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::RecursiveWorldNode( mnode_t* pnode ) 
{
	Uint32 i;
	Float dot;

	if(pnode->contents == CONTENTS_SOLID)
		return;

	if(rns.view.pviewleaf->contents != CONTENTS_SOLID)
	{
		if(pnode->visframe != rns.visframe)
			return;
	}

	if(rns.view.frustum.CullBBox(pnode->mins, pnode->maxs))
		return;

	if(pnode->contents < 0)
	{
		mleaf_t* pleaf = reinterpret_cast<mleaf_t*>(pnode);
		msurface_t** pmark = pleaf->pfirstmarksurface;

		for(i = 0; i < pleaf->nummarksurfaces; i++, pmark++)
			(*pmark)->visframe = rns.framecount;

		return;
	}

	plane_t* pplane = pnode->pplane;
	switch(pplane->type)
	{
	case PLANE_X:
		dot = rns.view.v_origin[0] - pplane->dist; break;
	case PLANE_Y:
		dot = rns.view.v_origin[1] - pplane->dist; break;
	case PLANE_Z:
		dot = rns.view.v_origin[2] - pplane->dist; break;
	default:
		dot = Math::DotProduct(rns.view.v_origin, pplane->normal) - pplane->dist;
		break;
	}

	Int32 side;
	Int32 sidebit;
	if(dot >= 0)
	{
		side = 0;
		sidebit = 0;
	}
	else
	{
		side = 1;
		sidebit = SURF_PLANEBACK;
	}

	// Go down the children, front side first
	RecursiveWorldNode(pnode->pchildren[side]);

	// Batch the surfaces
	if(pnode->numsurfaces)
	{
		msurface_t* psurface = ens.pworld->psurfaces + pnode->firstsurface;
		for(i = 0; i < pnode->numsurfaces; i++, psurface++)
		{
			FlagIfDynamicLighted(psurface->mins, psurface->maxs);

			if(psurface->visframe != rns.framecount)
				continue;

			if((psurface->flags & SURF_PLANEBACK) != sidebit)
				continue;

			if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			BatchSurface(psurface);
		}
	}

	// Recurse down the back side
	RecursiveWorldNode(pnode->pchildren[!side]);
}

//=============================================
// @brief
//
//=============================================
__forceinline void CBSPRenderer::BatchSurface( msurface_t* psurface )
{
	bsp_surface_t* pbspsurface = nullptr;
	pbspsurface = &m_surfacesArray[psurface->infoindex];
	bsp_texture_t* ptexture = pbspsurface->ptexture;
	if(!ptexture)
		return;

	bool addMultiPass = m_addMulti;
	if(!m_disableMultiPass)
	{
		// check for animated lights
		if(m_useLightStyles)
		{
			for(Uint32 i = 1; i < MAX_SURFACE_STYLES; i++)
			{
				Uint32 styleIndex = psurface->styles[i];
				if(styleIndex == NULL_LIGHTSTYLE_INDEX)
					break;

				if((*m_pLightStyleValuesArray)[styleIndex] <= 0)
					continue;

				lightstyleinfo_t& info = ptexture->lightstyleinfos[styleIndex];

				// Add to batch
				stylebatches_t& batch = info.stylebatches[i];
				AddBatch(batch.batches, batch.numbatches, pbspsurface);

				// See if we need to flag multipass
				if(!addMultiPass)
				{
					addMultiPass = true;

					if(!m_multiPass)
						m_multiPass = true;
				}
			}
		}
	}

	// Add to the appropriate batch list
	if(addMultiPass)
	{
		AddBatch(ptexture->multi_batches, ptexture->nummultibatches, pbspsurface);
		
		pbspsurface->ptexturechain = ptexture->psurfchain;
		ptexture->psurfchain = psurface;
	}
	else
	{
		AddBatch(ptexture->single_batches, ptexture->numsinglebatches, pbspsurface);
	}

	rns.counters.brushpolies++;
}

//=============================================
// @brief
//
//=============================================
__forceinline void CBSPRenderer::AddBatch( CArray<drawbatch_t>& batches, Uint32& numbatches, bsp_surface_t *psurface )
{
	drawbatch_t *pbatch;
	if(numbatches > 0)
	{
		// So based on some testing I did, it doesn't make much
		// of a difference to go through all batches, or to just
		// check the last one, so always only check the last batch
		// for matching indexes
		pbatch = &batches[numbatches-1];

		if(psurface->end_index == pbatch->start_index)
		{
			pbatch->start_index = psurface->start_index;
			return;
		}

		if(psurface->start_index == pbatch->end_index)
		{
			pbatch->end_index = psurface->end_index;
			return;
		}
	}

	// Add a new one
	pbatch = &batches[numbatches];
	numbatches++;

	pbatch->start_index = psurface->start_index;
	pbatch->end_index = psurface->end_index;
	rns.counters.batches++;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::FlagIfDynamicLighted( const Vector& mins, const Vector& maxs ) 
{
	Vector lmins, lmaxs;
	cl_dlight_t *dl;

	if(m_disableMultiPass)
	{
		m_addMulti = false;
		return;
	}

	if(g_pCvarDynamicLights->GetValue() < 1)
	{
		m_addMulti = false;
		return;
	}

	if(!m_multiPass)
	{
		m_addMulti = false;
		return;
	}

	if(rns.inwater && g_pCvarCaustics->GetValue() >= 1 
		&& gWaterShader.GetWaterQualitySetting() > CWaterShader::WATER_QUALITY_NO_REFLECT_REFRACT)
	{
		const water_settings_t* pwatersettings = gWaterShader.GetActiveSettings();
		if(pwatersettings 
			&& !pwatersettings->cheaprefraction 
			&& pwatersettings->causticscale > 0 
			&& pwatersettings->causticstrength > 0)
		{
			m_addMulti = true;
			return;
		}
	}

	CLinkedList<cl_dlight_t*>& dlightlist = gDynamicLights.GetLightList();

	dlightlist.begin();
	while(!dlightlist.end())
	{
		dl = dlightlist.get();
		if(dl->pfrustum)
		{
			if(dl->pfrustum->CullBBox(mins, maxs))
			{
				dlightlist.next();
				continue;
			}
		}
		else
		{
			for(Uint32 i = 0; i < 3; i++)
			{
				lmins[i] = dl->origin[i] - dl->radius;
				lmaxs[i] = dl->origin[i] + dl->radius;
			}

			if(Math::CheckMinsMaxs(mins, maxs, lmins, lmaxs))
			{
				dlightlist.next();
				continue;
			}
		}
		
		m_addMulti = true;
		return;
	}

	m_addMulti = false;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawFirst( void ) 
{
	Int32 rendermodeext;
	if(m_pCvarLegacyTransparents->GetValue() >= 1)
		rendermodeext = m_pCurrentEntity->curstate.rendermode;
	else
		rendermodeext = m_pCurrentEntity->curstate.rendermode & RENDERMODE_BITMASK;

	// Flag for whether the view matrix was set
	bool cubematrixSet = false;

	m_pShader->EnableAttribute(m_attribs.a_lmapcoord);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	m_pShader->SetUniform2f(m_attribs.u_uvoffset, 0, 0);

	// Render normal ones first
	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		if(!m_texturesArray[i].pmodeltexture)
			continue;

		// Nothing to draw
		if(!m_texturesArray[i].numsinglebatches)
			continue;

		// Get the animated texture
		mtexture_t *pworldtexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
		bsp_texture_t *ptexturehandle = &m_texturesArray[pworldtexture->infoindex];
		en_material_t* pmaterial = ptexturehandle->pmaterial;

		if(m_pCurrentEntity->curstate.effects & EF_CONVEYOR)
			m_pShader->SetUniform2f(m_attribs.u_uvoffset, -rns.time*m_pCurrentEntity->curstate.scale*0.02, 0);

		GLuint cubemapUnit = 0;
		cubemapinfo_t* pcubemapinfo = nullptr;
		cubemapinfo_t* pprevcubemapinfo = nullptr;

		bool alphaToCoverageEnabled = false;

		// rendermode overrides
		if(m_pCvarLegacyTransparents->GetValue() >= 1 && !m_multiPass 
			&& (rendermodeext == RENDER_TRANSADDITIVE || rendermodeext == RENDER_TRANSTEXTURE 
			|| rendermodeext == RENDER_TRANSALPHA_UNLIT || rendermodeext == RENDER_TRANSCOLOR 
			|| rendermodeext == RENDER_TRANSCOLOR_LIT))
		{
			m_pShader->DisableAttribute(m_attribs.a_normal);
			m_pShader->DisableAttribute(m_attribs.a_tangent);
			m_pShader->DisableAttribute(m_attribs.a_binormal);

			// Make sure these are disabled
			if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false) ||
				!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false) ||
				!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_chrome, false) ||
				!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_OFF, false) ||
				!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false))
				return false;

			bool result = true;
			switch(rendermodeext)
			{
			case RENDER_TRANSADDITIVE:
			case RENDER_TRANSTEXTURE:
			case RENDER_TRANSALPHA_UNLIT:
				{
					// Only texture
					m_pShader->SetUniform1i(m_attribs.u_maintexture, 0);
					R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

					m_pShader->EnableAttribute(m_attribs.a_lmapcoord);
					m_pShader->EnableAttribute(m_attribs.a_texcoord);

					if(pmaterial->ptextures[MT_TX_DETAIL] && m_pCvarDetailTextures->GetValue() > 0)
					{
						// Base texture AND detail texture
						result = m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_main_detail, false);

						m_pShader->SetUniform1i(m_attribs.u_detailtex, 1);
						R_Bind2DTexture(GL_TEXTURE1, pmaterial->ptextures[MT_TX_DETAIL]->palloc->gl_index);

						// Enable detail texcoord
						m_pShader->EnableAttribute(m_attribs.a_dtexcoord);
					}
					else
					{
						// Only main texture
						result = m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_texunit1, false);
						m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
					}
				}
				break;
			case RENDER_TRANSCOLOR:
				{
					// Only color
					result = m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_solidcolor, false);

					// Disable both of these
					m_pShader->DisableAttribute(m_attribs.a_texcoord);
					m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
					m_pShader->DisableAttribute(m_attribs.a_lmapcoord);
				}
				break;
			case RENDER_TRANSCOLOR_LIT:
				{
					// Only lightmap
					m_pShader->SetUniform1i(m_attribs.u_baselightmap, 0);
					R_Bind2DTexture(GL_TEXTURE0, m_ambientLightmapIndexes[BASE_LIGHTMAP_INDEX]);

					// Enable lightmap coord sends
					m_pShader->EnableAttribute(m_attribs.a_lmapcoord);

					result = m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_texunit0_x4, false);
				}
				break;
			}

			if(!result)
				return false;

			if((pmaterial->flags & TX_FL_ALPHATEST)
				&& (rendermodeext == RENDER_TRANSALPHA_UNLIT || rendermodeext == RENDER_TRANSALPHA))
			{
				if(!rns.msaa || !rns.mainframe)
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_LESSTHAN))
						return false;
				}
				else
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_COVERAGE))
						return false;

					glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
					gGLExtF.glSampleCoverage(0.5, GL_FALSE);
				}

				alphaToCoverageEnabled = true;
			}
			else
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED))
					return false;
			}
		}
		else
		{
			// Find any cubemaps
			if(m_isCubemappingSupported && g_pCvarCubemaps->GetValue() > 0 && pmaterial->flags & TX_FL_CUBEMAPS)
			{
				pcubemapinfo = gCubemaps.GetIdealCubemap();
				if(gCubemaps.GetInterpolant() != 1.0)
					pprevcubemapinfo = gCubemaps.GetPrevCubemap();
			}

			// Set up binds
			if(!BindTextures(ptexturehandle, pcubemapinfo, pprevcubemapinfo, cubemapUnit, alphaToCoverageEnabled))
				return false;
		
			// Set specular and phong if it's needed
			if(pmaterial->ptextures[MT_TX_SPECULAR])
			{
				m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
				m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);
			}

			// Reset cubemap bind
			if(m_isCubemappingSupported && pcubemapinfo && g_pCvarCubemaps->GetValue() > 0 && !cubematrixSet)
			{
				// So it gets synced between drawcalls
				m_pShader->EnableSync(m_attribs.u_modelmatrix);
				m_pShader->EnableSync(m_attribs.u_inv_modelmatrix);
				cubematrixSet = true;

				CMatrix modelMatrix;
				modelMatrix.LoadIdentity();
				modelMatrix.Rotate(90,  1, 0, 0);// put X going down
				modelMatrix.Rotate(-90,  0, 0, 1); // put Z going up
				modelMatrix.Scale(-1.0, 1.0, 1.0);
				modelMatrix.Translate(-rns.view.v_origin[0], -rns.view.v_origin[1], -rns.view.v_origin[2]);

				// We need to multiply normals with the inverse
				m_pShader->SetUniformMatrix4fv(m_attribs.u_modelmatrix, modelMatrix.GetMatrix());
				m_pShader->SetUniformMatrix4fv(m_attribs.u_inv_modelmatrix, modelMatrix.GetInverse());
				m_pShader->SetUniform1f(m_attribs.u_cubemapstrength, pmaterial->cubemapstrength);

				glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			}
			else if(cubematrixSet)
			{
				m_pShader->DisableSync(m_attribs.u_modelmatrix);
				m_pShader->DisableSync(m_attribs.u_inv_modelmatrix);
				cubematrixSet = false;
			}
		}

		R_ValidateShader(m_pShader);

		ptexturehandle = &m_texturesArray[i];
		drawbatch_t *pbatch = &ptexturehandle->single_batches[0];
		for(Uint32 j = 0; j < ptexturehandle->numsinglebatches; j++, pbatch++)
			glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));

		if(m_pCurrentEntity->curstate.effects & EF_CONVEYOR)
			m_pShader->SetUniform2f(m_attribs.u_uvoffset, 0, 0);

		if(alphaToCoverageEnabled)
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			gGLExtF.glSampleCoverage(1.0, GL_FALSE);
		}

		// Reset cubemap bind
		if(m_isCubemappingSupported && pcubemapinfo && g_pCvarCubemaps->GetValue() > 0)
		{
			R_BindCubemapTexture(GL_TEXTURE0_ARB + cubemapUnit, 0);

			if(pprevcubemapinfo)
				R_BindCubemapTexture(GL_TEXTURE0_ARB + cubemapUnit + 1, 0);

			glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}

		if(g_pCvarWireFrame->GetValue() >= 1)
		{
			m_pShader->DisableAttribute(m_attribs.a_lmapcoord);
			m_pShader->DisableAttribute(m_attribs.a_texcoord);
			m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
			m_pShader->DisableAttribute(m_attribs.a_normal);
			m_pShader->DisableAttribute(m_attribs.a_tangent);
			m_pShader->DisableAttribute(m_attribs.a_binormal);

			if(g_pCvarWireFrame->GetValue() >= 2)
				glDisable(GL_DEPTH_TEST);

			glLineWidth(1);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
			if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_ao, FALSE, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_solidcolor))
				return false;

			if(m_isCubemappingSupported)
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_OFF, false))
					return false;
			}

			R_ValidateShader(m_pShader);

			pbatch = &ptexturehandle->single_batches[0];
			for(Uint32 j = 0; j < ptexturehandle->numsinglebatches; j++, pbatch++)
				glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if(g_pCvarWireFrame->GetValue() >= 2)
				glEnable(GL_DEPTH_TEST);

			m_pShader->EnableAttribute(m_attribs.a_lmapcoord);
			m_pShader->EnableAttribute(m_attribs.a_texcoord);

			if(rns.fog.settings.active)
			{
				if(rns.fog.specialfog)
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_fogcoord, false))
						return false;
				}
				else
				{
					if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_radial, false))
						return false;
				}
			}
		}
	}

	// disable sends on detail texcoord
	m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
	m_pShader->DisableAttribute(m_attribs.a_normal);
	m_pShader->DisableAttribute(m_attribs.a_tangent);
	m_pShader->DisableAttribute(m_attribs.a_binormal);

	if(!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false)
		|| m_isCubemappingSupported && !m_pShader->SetDeterminator(m_attribs.d_cubemaps, FALSE, false))
		return false;

	// Make sure this gets disabled
	if(m_isCubemappingSupported)
	{
		m_pShader->DisableSync(m_attribs.u_modelmatrix);
		m_pShader->DisableSync(m_attribs.u_inv_modelmatrix);
	}

	//Render lightmaps only now
	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		if(!m_texturesArray[i].pmodeltexture)
			continue;

		if(!m_texturesArray[i].nummultibatches)
			continue;

		mtexture_t *pworldtexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
		bsp_texture_t* ptexturehandle = &m_texturesArray[pworldtexture->infoindex];
		en_material_t* pmaterial = ptexturehandle->pmaterial;

		// True if we need texcoord attrib
		bool useTexcoord = false;
		// Next free texturing unit
		Int32 textureIndex = 0;
		// Alpha testing method used
		Int32 alphatestMode = ALPHATEST_DISABLED;

		if(pmaterial->flags & TX_FL_ALPHATEST)
		{
			en_texture_t* pnormalmap = pmaterial->ptextures[MT_TX_NORMALMAP];
			if(m_bumpMaps && pnormalmap && g_pCvarBumpMaps->GetValue() > 0)
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, TRUE, false))
					return false;

				m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_ambientLightmapIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_maintexture, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_difflightmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_diffuseLightmapIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_lightvecstex, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightVectorsIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_normalmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pnormalmap->palloc->gl_index);
				textureIndex++;
			}
			else
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false))
					return false;

				m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightmapIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_maintexture, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
				textureIndex++;
			}

			alphatestMode = (rns.msaa && rns.mainframe) ? ALPHATEST_COVERAGE : ALPHATEST_LESSTHAN;
			if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, alphatestMode, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_lightalpha, false))
				return false;

			// We'll need texcoords
			useTexcoord = true;
		}
		else
		{
			en_texture_t* pnormalmap = pmaterial->ptextures[MT_TX_NORMALMAP];
			if(m_bumpMaps && pnormalmap && g_pCvarBumpMaps->GetValue() > 0)
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, TRUE, false))
					return false;

				m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_ambientLightmapIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_difflightmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_diffuseLightmapIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_lightvecstex, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightVectorsIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				m_pShader->SetUniform1i(m_attribs.u_normalmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pnormalmap->palloc->gl_index);
				textureIndex++;

				// We'll need texcoords
				useTexcoord = true;
			}
			else
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false))
					return false;

				m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightmapIndexes[BASE_LIGHTMAP_INDEX]);
				textureIndex++;

				// Texcoords won't be needed
				useTexcoord = false;
			}

			if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_lightonly, false))
				return false;
		}

		if(pmaterial->ptextures[MT_TX_LUMINANCE])
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_luminance, TRUE))
				return false;

			en_texture_t* pluminancetexture = pmaterial->ptextures[MT_TX_LUMINANCE];

			m_pShader->SetUniform1i(m_attribs.u_luminance, textureIndex);
			R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pluminancetexture->palloc->gl_index);
			textureIndex++;

			// We'll need texcoords
			useTexcoord = true;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE))
				return false;
		}
		
		if (pmaterial->ptextures[MT_TX_AO])
		{
			if (!m_pShader->SetDeterminator(m_attribs.d_ao, TRUE))
				return false;

			en_texture_t* paotexture = pmaterial->ptextures[MT_TX_AO];
			m_pShader->SetUniform1i(m_attribs.u_aomap, textureIndex);
			R_Bind2DTexture(GL_TEXTURE0 + textureIndex, paotexture->palloc->gl_index);
			textureIndex++;

			// We'll need texcoords
			useTexcoord = true;
		}
		else
		{
			if (!m_pShader->SetDeterminator(m_attribs.d_ao, FALSE))
				return false;
		}

		R_ValidateShader(m_pShader);

		if(useTexcoord)
			m_pShader->EnableAttribute(m_attribs.a_texcoord);

		// Manage MSAA if needed
		if(alphatestMode == ALPHATEST_COVERAGE)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			gGLExtF.glSampleCoverage(0.5, GL_FALSE);
		}

		bsp_texture_t* pbatchtexturehandle = &m_texturesArray[i];
		drawbatch_t *pbatch = &pbatchtexturehandle->multi_batches[0];
		for(Uint32 j = 0; j < pbatchtexturehandle->nummultibatches; j++, pbatch++)
			glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));

		if(alphatestMode == ALPHATEST_COVERAGE)
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			gGLExtF.glSampleCoverage(1.0, GL_FALSE);
		}

		if(useTexcoord)
			m_pShader->DisableAttribute(m_attribs.a_texcoord);
	}

	if(!m_disableMultiPass && m_useLightStyles)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthFunc(GL_EQUAL);

		// Render lightstyle batches now
		for(Uint32 i = 0; i < m_texturesArray.size(); i++)
		{
			if(!m_texturesArray[i].pmodeltexture)
				continue;

			if(!m_texturesArray[i].nummultibatches)
				continue;

			mtexture_t *pworldtexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
			bsp_texture_t* ptexturehandle = &m_texturesArray[pworldtexture->infoindex];
			en_material_t* pmaterial = ptexturehandle->pmaterial;

			// Don't bother if we have no lightstyles
			bsp_texture_t* pbatchtexturehandle = &m_texturesArray[i];
			if(pbatchtexturehandle->lightstyleinfos.empty())
				continue;

			// Render per layer
			for(Uint32 j = 0; j < pbatchtexturehandle->lightstyleinfos.size(); j++)
			{
				lightstyleinfo_t& styleinfo = pbatchtexturehandle->lightstyleinfos[j];
				if(styleinfo.stylebatches.empty())
					continue;

				// Don't bother if the lightstyle value is off
				Float stylestrength = (*m_pLightStyleValuesArray)[j];
				if(stylestrength <= 0)
					continue;

				// Set the color value
				m_pShader->SetUniform4f(m_attribs.u_color, stylestrength, stylestrength, stylestrength, 1.0);

				for(Uint32 k = 1; k < MAX_SURFACE_STYLES; k++)
				{
					// Check if we actually have any batches
					stylebatches_t& batches = styleinfo.stylebatches[k];
					if(batches.numbatches <= 0)
						continue;

					// Modify the lightmap coord pointer to use the appropriate texcoord
					m_pShader->SetAttributePointer(m_attribs.a_lmapcoord, OFFSET(bsp_vertex_t, lmapcoord[k]));

					// True if we need texcoord attrib
					bool useTexcoord = false;
					// Next free texturing unit
					Int32 textureIndex = 0;

					en_texture_t* pnormalmap = pmaterial->ptextures[MT_TX_NORMALMAP];
					if(m_bumpMaps && pnormalmap && g_pCvarBumpMaps->GetValue() > 0)
					{
						if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, TRUE, false))
							return false;

						m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_ambientLightmapIndexes[k]);
						textureIndex++;

						m_pShader->SetUniform1i(m_attribs.u_difflightmap, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_diffuseLightmapIndexes[k]);
						textureIndex++;

						m_pShader->SetUniform1i(m_attribs.u_lightvecstex, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightVectorsIndexes[k]);
						textureIndex++;

						m_pShader->SetUniform1i(m_attribs.u_normalmap, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pnormalmap->palloc->gl_index);
						textureIndex++;

						// We'll need texcoords
						useTexcoord = true;
					}
					else
					{
						if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false))
							return false;

						m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightmapIndexes[k]);
						textureIndex++;

						// Texcoords won't be needed
						useTexcoord = false;
					}

					if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false)
						|| !m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false)
						|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_lightonly, false))
						return false;

					if(pmaterial->ptextures[MT_TX_LUMINANCE])
					{
						if(!m_pShader->SetDeterminator(m_attribs.d_luminance, TRUE))
							return false;

						en_texture_t* pluminancetexture = pmaterial->ptextures[MT_TX_LUMINANCE];

						m_pShader->SetUniform1i(m_attribs.u_luminance, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pluminancetexture->palloc->gl_index);
						textureIndex++;

						// We'll need texcoords
						useTexcoord = true;
					}
					else
					{
						if(!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE))
							return false;
					}

					if (pmaterial->ptextures[MT_TX_AO])
					{
						if (!m_pShader->SetDeterminator(m_attribs.d_ao, TRUE))
							return false;

						en_texture_t* paotexture = pmaterial->ptextures[MT_TX_AO];
						m_pShader->SetUniform1i(m_attribs.u_aomap, textureIndex);
						R_Bind2DTexture(GL_TEXTURE0 + textureIndex, paotexture->palloc->gl_index);
						textureIndex++;

						// We'll need texcoords
						useTexcoord = true;
					}
					else
					{
						if (!m_pShader->SetDeterminator(m_attribs.d_ao, FALSE))
							return false;
					}

					R_ValidateShader(m_pShader);

					if(useTexcoord)
						m_pShader->EnableAttribute(m_attribs.a_texcoord);

					drawbatch_t *pbatch = &batches.batches[0];
					for(Uint32 l = 0; l < batches.numbatches; l++, pbatch++)
						glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));

					if(useTexcoord)
						m_pShader->DisableAttribute(m_attribs.a_texcoord);
				}
			}
		}

		// Re-set the lightmap coord to the first unit
		m_pShader->SetAttributePointer(m_attribs.a_lmapcoord, OFFSET(bsp_vertex_t, lmapcoord[0]));
		// Reset this to normal
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);

		glDisable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
	}

	m_pShader->DisableAttribute(m_attribs.a_lmapcoord);
	m_pShader->DisableAttribute(m_attribs.a_texcoord);
	m_pShader->DisableAttribute(m_attribs.a_dtexcoord);

	// Make sure to restore these
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_ao, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
mtexture_t *CBSPRenderer::TextureAnimation( mtexture_t *pbase, Uint32 frame )
{
	mtexture_t* ptexture = pbase;
	if(frame)
	{
		if(ptexture->palt_anims)
			ptexture = ptexture->palt_anims;
	}
	
	if((ptexture->name[0] != '+') || (!ptexture->anim_total))
		return ptexture;

	Int32 count = 0;
	Int32 relative = static_cast<Uint32>(rns.time*10) % ptexture->anim_total;
	while (ptexture->anim_min > relative || ptexture->anim_max <= relative)
	{
		ptexture = ptexture->panim_next;
		if (!ptexture)
			Con_Printf("TextureAnimation: broken cycle");
		
		count++;
		if (count > 100)
			Con_Printf("TextureAnimation: infinite cycle");
	}

	return ptexture;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::BindTextures( bsp_texture_t* phandle, cubemapinfo_t* pcubemapinfo, cubemapinfo_t* pprevcubemap, GLuint& cubemapUnit, bool& alphaToCoverageEnabled )
{
	Uint32 textureIndex = 0;
	en_material_t* pmaterial = phandle->pmaterial;
	bool bChrome = (R_IsEntityTransparent(*m_pCurrentEntity) && pmaterial->flags & TX_FL_CHROME);

	bool enableNormal = false;
	bool enableTangent = false;
	bool enableBinormal = false;
	bool specularTexBound = false;
	bool normalTexBound = false;

	en_texture_t* pnormalmap = pmaterial->ptextures[MT_TX_NORMALMAP];
	if(m_bumpMaps && pnormalmap && g_pCvarBumpMaps->GetValue() > 0)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, TRUE, false))
			return false;

		m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_ambientLightmapIndexes[BASE_LIGHTMAP_INDEX]);
		textureIndex++;

		m_pShader->SetUniform1i(m_attribs.u_difflightmap, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_diffuseLightmapIndexes[BASE_LIGHTMAP_INDEX]);
		textureIndex++;

		m_pShader->SetUniform1i(m_attribs.u_lightvecstex, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightVectorsIndexes[BASE_LIGHTMAP_INDEX]);
		textureIndex++;

		m_pShader->SetUniform1i(m_attribs.u_normalmap, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pnormalmap->palloc->gl_index);
		textureIndex++;

		normalTexBound = true;
		
		en_texture_t* pspecular = pmaterial->ptextures[MT_TX_SPECULAR];
		if(pspecular && g_pCvarSpecular->GetValue() > 0)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_specular, TRUE, false))
				return false;

			enableNormal = enableBinormal = enableTangent = true;

			m_pShader->SetUniform1i(m_attribs.u_specular, textureIndex);
			R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pspecular->palloc->gl_index);
			textureIndex++;

			specularTexBound = true;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false))
				return false;
		}
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false)
			|| !m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false))
			return false;
		
		m_pShader->SetUniform1i(m_attribs.u_baselightmap, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_lightmapIndexes[BASE_LIGHTMAP_INDEX]);
		textureIndex++;
	}

	// Bind the main texture
	m_pShader->SetUniform1i(m_attribs.u_maintexture, textureIndex);
	R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);
	textureIndex++;

	// Bind chrome if present
	if(bChrome)
	{
		Vector vorigin, view_right;
		Math::VectorSubtract(rns.view.v_origin, m_pCurrentEntity->curstate.origin, vorigin);
		Math::VectorCopy(rns.view.v_right, view_right);

		if(m_pCurrentEntity->curstate.angles[0] || m_pCurrentEntity->curstate.angles[1] || m_pCurrentEntity->curstate.angles[2])
		{
			Math::RotateToEntitySpace(m_pCurrentEntity->curstate.angles, vorigin);
			Math::RotateToEntitySpace(m_pCurrentEntity->curstate.angles, view_right);
		}

		m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
		if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_chrome, false))
			return false;

		m_pShader->SetUniform3f(m_attribs.u_vorigin, vorigin[0], vorigin[1], vorigin[2]);
		m_pShader->SetUniform3f(m_attribs.u_vright, view_right[0], view_right[1], view_right[2]);

		m_pShader->SetUniform1i(m_attribs.u_chrometex, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, m_pChromeTexture->palloc->gl_index);
		textureIndex++;

		enableNormal = true;
	}
	else
	{
		if(pmaterial->flags & TX_FL_ALPHATEST)
		{
			if(!rns.msaa || !rns.mainframe)
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_LESSTHAN, false))
					return false;
			}
			else
			{
				if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_COVERAGE, false))
					return false;

				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				gGLExtF.glSampleCoverage(0.5, GL_FALSE);
				alphaToCoverageEnabled = true;
			}
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
				return false;
		}

		en_texture_t* pdetailtexture = pmaterial->ptextures[MT_TX_DETAIL];
		if( pdetailtexture && m_pCvarDetailTextures->GetValue() > 0)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_detailtex, false))
				return false;

			m_pShader->EnableAttribute(m_attribs.a_dtexcoord);

			m_pShader->SetUniform1i(m_attribs.u_detailtex, textureIndex);
			R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pdetailtexture->palloc->gl_index);
			textureIndex++;
		}
		else
		{
			m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_nodetail, false))
				return false;
		}
	}

	// Manage cubemaps
	if(pmaterial->ptextures[MT_TX_SPECULAR] && m_isCubemappingSupported 
		&& pcubemapinfo && g_pCvarCubemaps->GetValue() > 0)
	{
		if(pmaterial->ptextures[MT_TX_NORMALMAP])
		{
			if(!normalTexBound)
			{
				m_pShader->SetUniform1i(m_attribs.u_normalmap, textureIndex);
				R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);
				textureIndex++;
			}

			enableBinormal = enableTangent = true;
		}

		if(!specularTexBound)
		{
			m_pShader->SetUniform1i(m_attribs.u_specular, textureIndex);
			R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);
			textureIndex++;
		}

		// Remember the texture unit
		cubemapUnit = textureIndex;
		enableNormal = true;
		textureIndex++;

		m_pShader->SetUniform1i(m_attribs.u_cubemap, cubemapUnit);
		R_BindCubemapTexture(GL_TEXTURE0_ARB + cubemapUnit, pcubemapinfo->palloc->gl_index);

		if(pprevcubemap)
		{
			m_pShader->SetUniform1f(m_attribs.u_interpolant, gCubemaps.GetInterpolant());
			if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_INTERP, false))
				return false;

			m_pShader->SetUniform1i(m_attribs.u_cubemap_prev, cubemapUnit + 1);
			R_BindCubemapTexture(GL_TEXTURE0_ARB + cubemapUnit + 1, pprevcubemap->palloc->gl_index);
		}
		else
		{
			m_pShader->SetUniform1f(m_attribs.u_interpolant, 0.0);
			if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_ON, false))
				return false;
		}
	}
	else if(m_isCubemappingSupported)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_OFF, false))
			return false;
	}

	if(pmaterial->ptextures[MT_TX_LUMINANCE])
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_luminance, TRUE, false))
			return false;

		en_texture_t* pluminancetexture = pmaterial->ptextures[MT_TX_LUMINANCE];

		m_pShader->SetUniform1i(m_attribs.u_luminance, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, pluminancetexture->palloc->gl_index);
		textureIndex++;
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_luminance, FALSE, false))
			return false;
	}
	
	if (pmaterial->ptextures[MT_TX_AO])
	{
		if (!m_pShader->SetDeterminator(m_attribs.d_ao, TRUE, false))
			return false;

		en_texture_t* paotexture = pmaterial->ptextures[MT_TX_AO];
		m_pShader->SetUniform1i(m_attribs.u_aomap, textureIndex);
		R_Bind2DTexture(GL_TEXTURE0 + textureIndex, paotexture->palloc->gl_index);
		textureIndex++;
	}
	else
	{
		if (!m_pShader->SetDeterminator(m_attribs.d_ao, FALSE, false))
			return false;
	}

	if(enableNormal)
		m_pShader->EnableAttribute(m_attribs.a_normal);
	else
		m_pShader->DisableAttribute(m_attribs.a_normal);

	if(enableTangent)
		m_pShader->EnableAttribute(m_attribs.a_tangent);
	else
		m_pShader->DisableAttribute(m_attribs.a_tangent);

	if(enableBinormal)
		m_pShader->EnableAttribute(m_attribs.a_binormal);
	else
		m_pShader->DisableAttribute(m_attribs.a_binormal);

	if(!m_pShader->VerifyDeterminators())
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawBrushModel( cl_entity_t& entity, bool isstatic )
{
	Int32 rendermode = entity.curstate.rendermode & RENDERMODE_BITMASK;
	Int32 rendermodeext = entity.curstate.rendermode;

	if(rendermodeext != RENDER_NORMAL
		&& entity.curstate.renderamt == 0)
		return true;

	if(entity.curstate.rendertype == RT_WATERSHADER 
		|| entity.curstate.rendertype == RT_MIRROR 
		|| entity.curstate.rendertype == RT_MONITORENTITY
		|| entity.curstate.rendertype == RT_PORTALSURFACE)
		return true;

	m_pCurrentEntity = &entity;
	const brushmodel_t* pmodel = entity.pmodel->getBrushmodel();

	Vector vorigin_local;
	Vector mins, maxs;

	Math::VectorCopy(rns.view.v_origin, vorigin_local);

	// Determine the mins/maxs
	if(R_IsEntityRotated(*m_pCurrentEntity))
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = m_pCurrentEntity->curstate.origin[i] - pmodel->radius;
			maxs[i] = m_pCurrentEntity->curstate.origin[i] + pmodel->radius;
		}
	}
	else
	{
		Math::VectorAdd(m_pCurrentEntity->curstate.origin, pmodel->mins, mins);
		Math::VectorAdd(m_pCurrentEntity->curstate.origin, pmodel->maxs, maxs);
	}

	// Also cull skybox entities now with frustum culling - the exception for 
	// sky ents was an ancient remnant from the Paranoia-type skybox rendering, 
	// and was never removed after that got replaced
	if(rns.view.frustum.CullBBox(mins, maxs))
		return true;

	// Transform to local space the origin
	Math::VectorSubtract(vorigin_local, m_pCurrentEntity->curstate.origin, vorigin_local);
	if(R_IsEntityRotated(*m_pCurrentEntity))
		Math::RotateToEntitySpace(m_pCurrentEntity->curstate.angles, vorigin_local);

	if(!isstatic)
	{
		// Apply the transformation to the
		bool ismoved = R_IsEntityMoved(*m_pCurrentEntity);
		if(ismoved)
		{
			rns.view.modelview.PushMatrix();
			R_RotateForEntity(rns.view.modelview, *m_pCurrentEntity);
		}

		// Prepare and load the matrix
		if(!Prepare())
			return false;

		if(ismoved)
			rns.view.modelview.PopMatrix();
	}

	Int32 highlightEntity = g_pCvarHighlightEntity->GetValue();
	if(highlightEntity != 0 && highlightEntity != m_pCurrentEntity->entindex)
		return true;

	// Apply transparency if any
	if(rendermode == RENDER_TRANSADDITIVE)
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		Float flalpha = R_RenderFxBlend(m_pCurrentEntity)/255.0f;
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, flalpha);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);
	}
	else if(rendermode == RENDER_TRANSTEXTURE)
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Float flalpha = R_RenderFxBlend(m_pCurrentEntity)/255.0f;
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, flalpha);
	}
	else if(rendermode == RENDER_TRANSCOLOR)
	{
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Float flalpha = R_RenderFxBlend(m_pCurrentEntity)/255.0f;
		Float flr = m_pCurrentEntity->curstate.rendercolor.x/255.0f;
		Float flg = m_pCurrentEntity->curstate.rendercolor.y/255.0f;
		Float flb = m_pCurrentEntity->curstate.rendercolor.z/255.0f;
		m_pShader->SetUniform4f(m_attribs.u_color, flr, flg, flb, flalpha);
	}

	// Check dynamic lighting
	FlagIfDynamicLighted(mins, maxs);

	// Batch the surfaces
	msurface_t* psurface = ens.pworld->psurfaces + pmodel->firstmodelsurface;
	for(Uint32 i = 0; i < pmodel->nummodelsurfaces; i++, psurface++)
	{
		plane_t* pplane = psurface->pplane;
		Float dp = Math::DotProduct(vorigin_local, pplane->normal) - pplane->dist;

		if(((psurface->flags & SURF_PLANEBACK) && (dp < -BACKFACE_EPSILON))
			|| (!(psurface->flags & SURF_PLANEBACK) && (dp > BACKFACE_EPSILON)))
		{
			if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			BatchSurface(psurface);
		}
	}

	// Draw everything now if it's not a static pass
	if(!isstatic)
	{
		if(!DrawFirst()
			|| !DrawLights(FALSE)
			|| !DrawFinal())
			return false;
	}

	// Disable blending
	if(rendermode == RENDER_TRANSADDITIVE
		|| rendermode == RENDER_TRANSTEXTURE
		|| rendermode == RENDER_TRANSCOLOR)
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	// Set this for decals
	m_pCurrentEntity->visframe = rns.framecount;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::SetupLight( cl_dlight_t* pdlight, Uint32 lightindex, Int32& texunit, lightbatchtype_t type )
{
	CMatrix matrix;
	Vector vtransorigin;

	m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_color);
	m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_origin);
	m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_radius);

	// Transform light origin to eye space
	Math::MatMultPosition(rns.view.modelview.Transpose(), pdlight->origin, &vtransorigin);
	
	if(type == LB_TYPE_SPOTLIGHT || type == LB_TYPE_SPOTLIGHT_SHADOW)
	{
		Vector vforward, vtarget;
		Vector angles = pdlight->angles;
		Common::FixVector(angles);

		Math::AngleVectors(angles, &vforward, nullptr, nullptr);
		Math::VectorMA(pdlight->origin, pdlight->radius, vforward, vtarget);

		m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_projtexture);
		m_pShader->SetUniform1i(m_attribs.lights[lightindex].u_light_projtexture, texunit);
		R_Bind2DTexture(GL_TEXTURE0+texunit, rns.objects.projective_textures[pdlight->textureindex]->palloc->gl_index);
		texunit++;

		if(DL_CanShadow(pdlight))
		{
			if(!m_pShader->SetDeterminator(m_attribs.lights[lightindex].d_light_shadowmap, TRUE, false))
				return false;

			m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_shadowmap);
			m_pShader->SetUniform1i(m_attribs.lights[lightindex].u_light_shadowmap, texunit);
			R_Bind2DTexture(GL_TEXTURE0+texunit, pdlight->getProjShadowMap()->pfbo->ptexture1->gl_index);
			texunit++;
		}
		else
		{
			m_pShader->DisableSync(m_attribs.lights[lightindex].u_light_shadowmap);
			if(!m_pShader->SetDeterminator(m_attribs.lights[lightindex].d_light_shadowmap, FALSE, false))
				return false;
		}

		m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_matrix);

		matrix.LoadIdentity();
		matrix.Translate(0.5, 0.5, 0.5);
		matrix.Scale(0.5, 0.5, 1.0);

		Float flsize = tan((M_PI/360) * pdlight->cone_size);
		matrix.SetFrustum(-flsize, flsize, -flsize, flsize, 1, pdlight->radius);

		matrix.LookAt(pdlight->origin[0], pdlight->origin[1], pdlight->origin[2], vtarget[0], vtarget[1], vtarget[2], 0, 0, Common::IsPitchReversed(angles[PITCH]) ? -1 : 1);
		matrix.MultMatrix(rns.view.modelview.GetInverse());

		m_pShader->SetUniformMatrix4fv(m_attribs.lights[lightindex].u_light_matrix, matrix.Transpose());
	}
	else
	{
		m_pShader->DisableSync(m_attribs.lights[lightindex].u_light_projtexture);
		m_pShader->DisableSync(m_attribs.lights[lightindex].u_light_shadowmap);

		if(DL_CanShadow(pdlight))
		{
			if(!m_pShader->SetDeterminator(m_attribs.lights[lightindex].d_light_shadowmap, TRUE, false))
				return false;

			m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_cubemap);
			m_pShader->SetUniform1i(m_attribs.lights[lightindex].u_light_cubemap, texunit);
			R_BindCubemapTexture(GL_TEXTURE0+texunit, pdlight->getCubeShadowMap()->pfbo->ptexture1->gl_index);
			texunit++;

			// Set up world-space matrix
			matrix.LoadIdentity();
			matrix.Rotate(-90,  1, 0, 0);// put X going down
			matrix.Rotate(90,  0, 0, 1); // put Z going up
			matrix.Translate(-pdlight->origin[0], -pdlight->origin[1], -pdlight->origin[2]);

			if(R_IsEntityMoved(*m_pCurrentEntity))
			{
				m_pCurrentEntity->curstate.angles[0] = -m_pCurrentEntity->curstate.angles[0];
				matrix.Translate(m_pCurrentEntity->curstate.origin[0],  m_pCurrentEntity->curstate.origin[1],  m_pCurrentEntity->curstate.origin[2]);
				matrix.Rotate(m_pCurrentEntity->curstate.angles[1],  0, 0, 1);
				matrix.Rotate(-m_pCurrentEntity->curstate.angles[0],  0, 1, 0);
				matrix.Rotate(m_pCurrentEntity->curstate.angles[2],  1, 0, 0);
				m_pCurrentEntity->curstate.angles[0] = -m_pCurrentEntity->curstate.angles[0];
			}

			// set up light matrix
			m_pShader->EnableSync(m_attribs.lights[lightindex].u_light_matrix);
			m_pShader->SetUniformMatrix4fv(m_attribs.lights[lightindex].u_light_matrix, matrix.GetMatrix(), true);

			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}
		else
		{
			m_pShader->DisableSync(m_attribs.lights[lightindex].u_light_cubemap);
			m_pShader->DisableSync(m_attribs.lights[lightindex].u_light_matrix);

			if(!m_pShader->SetDeterminator(m_attribs.lights[lightindex].d_light_shadowmap, FALSE, false))
				return false;
		}
	}

	Vector color;
	Math::VectorCopy(pdlight->color, color);
	gLightStyles.ApplyLightStyle(pdlight, color);

	m_pShader->SetUniform4f(m_attribs.lights[lightindex].u_light_color, color[0], color[1], color[2], 1.0);
	m_pShader->SetUniform3f(m_attribs.lights[lightindex].u_light_origin, vtransorigin[0], vtransorigin[1], vtransorigin[2]);
	m_pShader->SetUniform1f(m_attribs.lights[lightindex].u_light_radius, pdlight->radius);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::FinishLight( cl_dlight_t* pdlight, Int32& texunit )
{
	if(pdlight->cone_size)
	{
		R_Bind2DTexture(GL_TEXTURE0+texunit, 0);
		texunit++;

		if(DL_CanShadow(pdlight))
		{
			R_Bind2DTexture(GL_TEXTURE0+texunit, 0);
			texunit++;
		}
	}
	else if(DL_CanShadow(pdlight))
	{
		// binds cubemap texture
		R_BindCubemapTexture(GL_TEXTURE0+texunit, 0);
		texunit++;

		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawLights( bool specular ) 
{
	if(!m_multiPass)
		return true;

	if(g_pCvarDynamicLights->GetValue() < 1)
		return true;

	// Retreive dynlight list
	CLinkedList<cl_dlight_t*>& dlightlist = gDynamicLights.GetLightList();
	if(dlightlist.empty())
		return true;

	if(!specular)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);

		if(!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false))
			return false;
	}
	else
	{
		// Specular needs bump mapping calculations
		if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, TRUE, false)
			|| !m_pShader->SetDeterminator(m_attribs.d_specular, TRUE, false))
			return false;
	}

	glBlendFunc(GL_ONE, GL_ONE);

	m_pShader->EnableAttribute(m_attribs.a_normal);

	if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
		return false;

	// Linked list of dynamic light batches
	CLinkedList<lightbatch_t> lightBatches;

	// Build batches
	dlightlist.begin();
	while(!dlightlist.end())
	{
		cl_dlight_t* dl = dlightlist.get();

		Vector mins, maxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = dl->origin[i] - dl->radius;
			maxs[i] = dl->origin[i] + dl->radius;
		}

		if(!DL_IsLightVisible(rns.view.frustum, mins, maxs, dl))
		{
			dlightlist.next();
			continue;
		}

		// If not world or non-moving entity, check to make sure
		// the dynamic light is actually touching us
		if(m_pCurrentEntity->entindex != WORLDSPAWN_ENTITY_INDEX)
		{
			const brushmodel_t* pmodel = m_pCurrentEntity->pmodel->getBrushmodel();

			Vector entitymins, entitymaxs;
			if(R_IsEntityRotated(*m_pCurrentEntity))
			{
				for(Uint32 i = 0; i < 3; i++)
				{
					entitymins[i] = m_pCurrentEntity->curstate.origin[i] - pmodel->radius;
					entitymaxs[i] = m_pCurrentEntity->curstate.origin[i] + pmodel->radius;
				}
			}
			else
			{
				Math::VectorAdd(m_pCurrentEntity->curstate.origin, pmodel->mins, entitymins);
				Math::VectorAdd(m_pCurrentEntity->curstate.origin, pmodel->maxs, entitymaxs);
			}

			if(Math::CheckMinsMaxs(mins, maxs, entitymins, entitymaxs))
			{
				dlightlist.next();
				continue;
			}
		}

		// Pointer to batch we'll use
		lightbatch_t* pbatch = nullptr;

		// Determine batch type
		lightbatchtype_t type;
		if(dl->cone_size)
		{
			if(dl->noShadow())
				type = LB_TYPE_SPOTLIGHT;
			else
				type = LB_TYPE_SPOTLIGHT_SHADOW;
		}
		else
		{
			if(dl->noShadow())
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
					if(!Math::CheckMinsMaxs(curbatch.mins, curbatch.maxs, mins, maxs))
					{
						pbatch = &curbatch;
						break;
					}
				}

				lightBatches.next();
			}
		}

		// set bounding box
		Vector vorigin;
		Math::VectorCopy(dl->origin, vorigin);
		if(R_IsEntityMoved(*m_pCurrentEntity))
		{
			Math::VectorSubtract (vorigin, m_pCurrentEntity->curstate.origin, vorigin);
			if(m_pCurrentEntity->curstate.angles[0] || m_pCurrentEntity->curstate.angles[1] || m_pCurrentEntity->curstate.angles[2])
				Math::RotateToEntitySpace(m_pCurrentEntity->curstate.angles, vorigin);
		}

		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = vorigin[i] - dl->radius;
			maxs[i] = vorigin[i] + dl->radius;
		}

		if(!pbatch)
		{
			lightbatch_t newbatch;
			newbatch.type = type;
			newbatch.mins = mins;
			newbatch.maxs = maxs;

			pbatch = &lightBatches.add(newbatch)->_val;
		}

		for(Uint32 i = 0; i < 3; i++)
		{
			if(pbatch->mins[i] > mins[i])
				pbatch->mins[i] = mins[i];

			if(pbatch->maxs[i] < maxs[i])
				pbatch->maxs[i] = maxs[i];
		}

		pbatch->lightslist.add(dl);
		dlightlist.next();
	};

	// Now draw the actual batches
	lightBatches.begin();
	while(!lightBatches.end())
	{
		lightbatch_t& batch = lightBatches.get();

		// Latest light index
		Uint32 lightindex = 0;
		// Next available texture unit
		Int32 texunit = 0;

		if(batch.type == LB_TYPE_POINTLIGHT || batch.type == LB_TYPE_POINTLIGHT_SHADOW)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_dynlight, false))
				return false;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_spotlight, false))
				return false;
		}

		batch.lightslist.begin();
		while(!batch.lightslist.end())
		{
			cl_dlight_t* pdlight = batch.lightslist.get();

			// Build mins/maxs
			Vector mins, maxs;
			for(Uint32 i = 0; i < 3; i++)
			{
				mins[i] = pdlight->origin[i] - pdlight->radius;
				maxs[i] = pdlight->origin[i] + pdlight->radius;
			}

			if(!DL_IsLightVisible(rns.view.frustum, mins, maxs, pdlight))
			{
				dlightlist.next();
				continue;
			}

			// Set up the light for rendering
			if(!SetupLight(pdlight, lightindex, texunit, batch.type))
				return false;

			batch.lightslist.next();
			lightindex++;
		}

		if(!m_pShader->SetDeterminator(m_attribs.d_numlights, lightindex, false))
			return false;

		// Render normal ones first
		for(Uint32 i = 0; i < m_texturesArray.size(); i++)
		{
			if(!m_texturesArray[i].pmodeltexture)
				continue;

			// Nothing to draw
			if(!m_texturesArray[i].nummultibatches)
				continue;

			// Get the animated texture
			mtexture_t *pworldtexture = m_texturesArray[i].pmodeltexture;
			bsp_texture_t *ptexturehandle = &m_texturesArray[pworldtexture->infoindex];
			en_material_t* pmaterial = ptexturehandle->pmaterial;

			if(specular && (!pmaterial->ptextures[MT_TX_SPECULAR] || !pmaterial->ptextures[MT_TX_NORMALMAP]))
				continue;

			msurface_t *pnext = ptexturehandle->psurfchain;
			ptexturehandle->numlightbatches = 0;
			while(pnext)
			{
				// Do cheap bbox tests here
				bsp_surface_t *pclsurf = &m_surfacesArray[pnext->infoindex];
				if(Math::CheckMinsMaxs(pclsurf->mins, pclsurf->maxs, batch.mins, batch.maxs))
				{
					pnext = pclsurf->ptexturechain;
					continue;
				}

				AddBatch(ptexturehandle->light_batches, ptexturehandle->numlightbatches, pclsurf);
				pnext = pclsurf->ptexturechain;
				rns.counters.brushpolies++;
			}
			
			// TRUE if we should send texcoords
			bool useTexCoord = false;
			// Normal map unit
			Int32 normalmapunit = NO_POSITION;
			// Specular map unit
			Int32 specularmapunit = NO_POSITION;
			// AO mapping unit to use
			Int32 aomapunit = NO_POSITION;
			// First unit used
			Int32 firstunit = texunit;
			// Tex units for current texture
			Int32 texunit_local = firstunit;

			if(specular)
			{
				m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
				m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

				// Set normal map
				normalmapunit = texunit_local;
				texunit_local++;

				R_Bind2DTexture(GL_TEXTURE0 + normalmapunit, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);

				// Set specular map
				specularmapunit = texunit_local;
				texunit_local++;

				R_Bind2DTexture(GL_TEXTURE0 + specularmapunit, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);

				m_pShader->EnableAttribute(m_attribs.a_tangent);
				m_pShader->EnableAttribute(m_attribs.a_binormal);
				useTexCoord = true;
			}
			else if(pmaterial->ptextures[MT_TX_NORMALMAP] && g_pCvarBumpMaps->GetValue() > 0)
			{
				// Set normal map
				normalmapunit = texunit_local;
				texunit_local++;

				R_Bind2DTexture(GL_TEXTURE0 + normalmapunit, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);

				m_pShader->EnableAttribute(m_attribs.a_tangent);
				m_pShader->EnableAttribute(m_attribs.a_binormal);
				useTexCoord = true;

				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, true, false))
					return false;
			}
			else
			{
				m_pShader->DisableAttribute(m_attribs.a_tangent);
				m_pShader->DisableAttribute(m_attribs.a_binormal);
				useTexCoord = false;
				normalmapunit = texunit_local;

				if(!m_pShader->SetDeterminator(m_attribs.d_specular, false, false)
					|| !m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false, false))
					return false;
			}

			if (pmaterial->ptextures[MT_TX_AO])
			{
				// Specify the AO map unit
				aomapunit = texunit_local;
				texunit_local++;

				R_Bind2DTexture(GL_TEXTURE0 + aomapunit, pmaterial->ptextures[MT_TX_AO]->palloc->gl_index);
				useTexCoord = true;

				if (!m_pShader->SetDeterminator(m_attribs.d_ao, true, false))
					return false;
			}
			else
			{
				if (!m_pShader->SetDeterminator(m_attribs.d_ao, false, false))
					return false;
			}

			// u_specular always needs to be set, otherwise AMD will complain
			// about two samplers being on the same unit.
			m_pShader->SetUniform1i(m_attribs.u_normalmap, normalmapunit);
			if(specularmapunit != NO_POSITION)
				m_pShader->SetUniform1i(m_attribs.u_specular, specularmapunit);
			else if(aomapunit != NO_POSITION)
				m_pShader->SetUniform1i(m_attribs.u_specular, aomapunit + 1);
			else
				m_pShader->SetUniform1i(m_attribs.u_specular, normalmapunit + 1);

			if(aomapunit != NO_POSITION)
				m_pShader->SetUniform1i(m_attribs.u_aomap, aomapunit);

			// Verify that everything is ok with the states
			if(!m_pShader->VerifyDeterminators())
				return false;

			// Make sure shaders are valid
			R_ValidateShader(m_pShader);

			if(useTexCoord)
				m_pShader->EnableAttribute(m_attribs.a_texcoord);
			else
				m_pShader->DisableAttribute(m_attribs.a_texcoord);

			drawbatch_t *pdrawbatch = &ptexturehandle->light_batches[0];
			for(Uint32 j = 0; j < ptexturehandle->numlightbatches; j++, pdrawbatch++)
				glDrawElements(GL_TRIANGLES, pdrawbatch->end_index-pdrawbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pdrawbatch->start_index));

			// Remove all current binds from textures used
			R_ClearBinds(firstunit);
		}

		// Clean up bound textures
		texunit = 0;
		batch.lightslist.begin();
		while(!batch.lightslist.end())
		{
			cl_dlight_t* pdlight = batch.lightslist.get();

			FinishLight(pdlight, texunit);

			batch.lightslist.next();
			lightindex++;
		}

		// Remove all current binds
		R_ClearBinds();

		// Reset everything
		for(Uint32 i = 0; i < MAX_BATCH_LIGHTS; i++)
		{
			m_pShader->DisableSync(m_attribs.lights[i].u_light_color);
			m_pShader->DisableSync(m_attribs.lights[i].u_light_origin);
			m_pShader->DisableSync(m_attribs.lights[i].u_light_radius);
			m_pShader->DisableSync(m_attribs.lights[i].u_light_cubemap);
			m_pShader->DisableSync(m_attribs.lights[i].u_light_projtexture);
			m_pShader->DisableSync(m_attribs.lights[i].u_light_shadowmap);
			m_pShader->DisableSync(m_attribs.lights[i].u_light_matrix);
		
			// Reset all of these
			if(!m_pShader->SetDeterminator(m_attribs.lights[i].d_light_shadowmap, FALSE, false))
				return false;		
		}

		lightBatches.next();
	}

	if(!specular)
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
	}

	m_pShader->DisableAttribute(m_attribs.a_normal);
	m_pShader->DisableAttribute(m_attribs.a_tangent);
	m_pShader->DisableAttribute(m_attribs.a_binormal);
	m_pShader->DisableAttribute(m_attribs.a_texcoord);

	// Reset determinators
	if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_ao, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_numlights, 0, false))
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawFinal( void ) 
{
	if(!m_multiPass)
		return true;

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);

	if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false))
		return false;

	if(rns.inwater && g_pCvarCaustics->GetValue() >= 1)
	{
		const water_settings_t *psettings = gWaterShader.GetActiveSettings();
		if(psettings 
			&& !psettings->cheaprefraction 
			&& psettings->causticscale > 0 
			&& psettings->causticstrength > 0)
		{
			GLfloat splane[4] = {static_cast<Float>(0.005)*psettings->causticscale, static_cast<Float>(0.0025)*psettings->causticscale, 0.0, 0.0};
			GLfloat tplane[4] = {0.0, static_cast<Float>(0.005)*psettings->causticscale, static_cast<Float>(0.0025)*psettings->causticscale, 0.0};

			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_caustics))
				return false;

			m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
			m_pShader->DisableAttribute(m_attribs.a_texcoord);
			m_pShader->DisableAttribute(m_attribs.a_lmapcoord);

			m_pShader->SetUniform1i(m_attribs.u_maintexture, 0);
			m_pShader->SetUniform1i(m_attribs.u_detailtex, 1);
			m_pShader->SetUniform4f(m_attribs.u_causticsm1, splane[0], splane[1], splane[2], splane[3]);
			m_pShader->SetUniform4f(m_attribs.u_causticsm2, tplane[0], tplane[1], tplane[2], tplane[3]);

			Float causticsTime = rns.time*10*psettings->causticstimescale;
			Int32 causticsCurFrame = static_cast<Int32>(causticsTime) % rns.objects.caustics_textures.size();
			Int32 causticsNextFrame = (causticsCurFrame+1) % rns.objects.caustics_textures.size();
			Float causticsInterp = (causticsTime)-static_cast<Int32>(causticsTime);

			R_Bind2DTexture(GL_TEXTURE0, rns.objects.caustics_textures[causticsCurFrame]->palloc->gl_index);
			R_Bind2DTexture(GL_TEXTURE1, rns.objects.caustics_textures[causticsNextFrame]->palloc->gl_index);

			m_pShader->SetUniform1f(m_attribs.u_interpolant, causticsInterp);
			m_pShader->SetUniform4f(m_attribs.u_color, 
				psettings->fogparams.color[0]*psettings->causticstrength,
				psettings->fogparams.color[1]*psettings->causticstrength, 
				psettings->fogparams.color[2]*psettings->causticstrength,
				1.0);
		
			glBlendFunc(GL_DST_COLOR, GL_ONE);

			R_ValidateShader(m_pShader);

			for (Uint32 i = 0; i < m_texturesArray.size(); i++)
			{
				if(!m_texturesArray[i].pmodeltexture)
					continue;

				if(!m_texturesArray[i].nummultibatches)
					continue;

				drawbatch_t *pbatch = &m_texturesArray[i].multi_batches[0];
				for(Uint32 j = 0; j < m_texturesArray[i].nummultibatches; j++, pbatch++)
					glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
			}
		}
	}

	// blend lighting with textures
	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		if(!m_texturesArray[i].pmodeltexture)
			continue;

		if(!m_texturesArray[i].nummultibatches)
			continue;

		mtexture_t *ptexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
		bsp_texture_t* ptexturehandle = &m_texturesArray[ptexture->infoindex];
		en_material_t* pmaterial = ptexturehandle->pmaterial;

		if(m_pCurrentEntity->curstate.effects & EF_CONVEYOR)
			m_pShader->SetUniform2f(m_attribs.u_uvoffset, -rns.time*m_pCurrentEntity->curstate.scale*0.02, 0);

		en_texture_t* pdetail = pmaterial->ptextures[MT_TX_DETAIL];
		en_texture_t* pdiffuse = pmaterial->ptextures[MT_TX_DIFFUSE];
		if(pdetail && m_pCvarDetailTextures->GetValue())
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_main_detail))
				return false;

			m_pShader->SetUniform1i(m_attribs.u_detailtex, 1);
			m_pShader->EnableAttribute(m_attribs.a_dtexcoord);

			R_Bind2DTexture(GL_TEXTURE1, pdetail->palloc->gl_index);		
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_texunit1))
				return false;

			m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
		}

		m_pShader->SetUniform1i(m_attribs.u_maintexture, 0);
		R_Bind2DTexture(GL_TEXTURE0, pdiffuse->palloc->gl_index);

		R_ValidateShader(m_pShader);

		drawbatch_t *pbatch = &m_texturesArray[i].multi_batches[0];
		for(Uint32 j = 0; j < m_texturesArray[i].nummultibatches; j++, pbatch++)
			glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));

		if(m_pCurrentEntity->curstate.effects & EF_CONVEYOR)
			m_pShader->SetUniform2f(m_attribs.u_uvoffset, 0, 0);
	}

	// Render meshes with specular highlights
	if(!DrawFinalSpecular())
		return false;

	// Disable attribs
	m_pShader->DisableAttribute(m_attribs.a_dtexcoord);

	cubemapinfo_t* pcubemapinfo = nullptr;
	if(m_isCubemappingSupported && g_pCvarCubemaps->GetValue() > 0)
		pcubemapinfo = gCubemaps.GetIdealCubemap();

	// Draw any cubemaps
	if(m_isCubemappingSupported && pcubemapinfo && g_pCvarCubemaps->GetValue() > 0)
	{
		glBlendFunc(GL_ONE, GL_ONE);

		m_pShader->EnableAttribute(m_attribs.a_texcoord);
		m_pShader->EnableAttribute(m_attribs.a_normal);
		if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_cubeonly, false))
			return false;

		Float interp = gCubemaps.GetInterpolant();
		cubemapinfo_t* pprevcubemap = gCubemaps.GetPrevCubemap();
		if(interp != 1.0 && pprevcubemap)
		{
			m_pShader->SetUniform1f(m_attribs.u_interpolant, interp);
			if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_INTERP, false))
				return false;
		}
		else
		{
			m_pShader->SetUniform1f(m_attribs.u_interpolant, 0.0);
			if(!m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_ON, false))
				return false;
		}

		CMatrix modelMatrix;
		modelMatrix.LoadIdentity();
		modelMatrix.Rotate(90,  1, 0, 0);// put X going down
		modelMatrix.Rotate(-90,  0, 0, 1); // put Z going up
		modelMatrix.Scale(-1.0, 1.0, 1.0);
		modelMatrix.Translate(-rns.view.v_origin[0], -rns.view.v_origin[1], -rns.view.v_origin[2]);

		// We need to multiply normals with the inverse
		m_pShader->EnableSync(m_attribs.u_modelmatrix);
		m_pShader->EnableSync(m_attribs.u_inv_modelmatrix);

		m_pShader->SetUniformMatrix4fv(m_attribs.u_modelmatrix, modelMatrix.GetMatrix());
		m_pShader->SetUniformMatrix4fv(m_attribs.u_inv_modelmatrix, modelMatrix.GetInverse());

		// texindex base
		Uint32 texbase = 0;
		R_BindCubemapTexture(GL_TEXTURE0_ARB + texbase, pcubemapinfo->palloc->gl_index);
		m_pShader->SetUniform1i(m_attribs.u_cubemap, texbase);
		texbase++;

		// Bind previous cubemap if needed
		if(interp != 1.0 && pprevcubemap)
		{
			R_BindCubemapTexture(GL_TEXTURE0_ARB + texbase, pprevcubemap->palloc->gl_index);
			m_pShader->SetUniform1i(m_attribs.u_cubemap_prev, texbase);
			texbase++;
		}

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		for(Uint32 i = 0; i < m_texturesArray.size(); i++)
		{
			if(!m_texturesArray[i].pmodeltexture)
				continue;

			if(!m_texturesArray[i].nummultibatches)
				continue;

			mtexture_t *ptexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
			bsp_texture_t* ptexturehandle = &m_texturesArray[ptexture->infoindex];
			en_material_t* pmaterial = ptexturehandle->pmaterial;

			if(!pmaterial->ptextures[MT_TX_SPECULAR])
				continue;

			if(!(pmaterial->flags & TX_FL_CUBEMAPS))
				continue;			
			
			m_pShader->SetUniform1f(m_attribs.u_cubemapstrength, pmaterial->cubemapstrength);

			// Bind specular texture
			m_pShader->SetUniform1i(m_attribs.u_specular, texbase);
			R_Bind2DTexture(GL_TEXTURE0 + texbase, pmaterial->ptextures[MT_TX_SPECULAR]->palloc->gl_index);

			if(pmaterial->ptextures[MT_TX_NORMALMAP])
			{
				m_pShader->SetUniform1i(m_attribs.u_normalmap, texbase + 1);
				R_Bind2DTexture(GL_TEXTURE0 + texbase + 1, pmaterial->ptextures[MT_TX_NORMALMAP]->palloc->gl_index);

				m_pShader->EnableAttribute(m_attribs.a_binormal);
				m_pShader->EnableAttribute(m_attribs.a_tangent);

				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, true))
					return false;
			}
			else
			{
				m_pShader->DisableAttribute(m_attribs.a_binormal);
				m_pShader->DisableAttribute(m_attribs.a_tangent);

				if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, false))
					return false;
			}

			R_ValidateShader(m_pShader);

			drawbatch_t *pbatch = &ptexturehandle->multi_batches[0];
			for(Uint32 j = 0; j < ptexturehandle->nummultibatches; j++, pbatch++)
				glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
		}

		// texindex base
		texbase = 0;
		R_BindCubemapTexture(GL_TEXTURE0_ARB + texbase, 0);
		texbase++;

		// Bind previous cubemap if needed
		if(interp != 1.0 && pprevcubemap)
		{
			R_BindCubemapTexture(GL_TEXTURE0_ARB + texbase, 0);
			texbase++;
		}

		glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		m_pShader->DisableAttribute(m_attribs.a_normal);
		m_pShader->DisableAttribute(m_attribs.a_binormal);
		m_pShader->DisableAttribute(m_attribs.a_tangent);
		m_pShader->DisableSync(m_attribs.u_modelmatrix);
		m_pShader->DisableSync(m_attribs.u_inv_modelmatrix);

		if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false)
			|| !m_pShader->SetDeterminator(m_attribs.d_cubemaps, CUBEMAPS_OFF, false))
			return false;
	}

	// Disable texcoord sends
	m_pShader->DisableAttribute(m_attribs.a_texcoord);

	if(rns.fog.settings.active)
	{
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));

		if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false))
			return false;

		if(!rns.fog.specialfog)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_fogpass))
				return false;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_fogpass_fc))
				return false;

			m_pShader->EnableAttribute(m_attribs.a_fogcoord);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		R_ValidateShader(m_pShader);

		for (Uint32 i = 0; i < m_texturesArray.size(); i++)
		{
			if(!m_texturesArray[i].pmodeltexture)
				continue;

			if(!m_texturesArray[i].nummultibatches)
				continue;

			drawbatch_t *pbatch = &m_texturesArray[i].multi_batches[0];
			for(Uint32 j = 0; j < m_texturesArray[i].nummultibatches; j++, pbatch++)
				glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
		}

		if(rns.fog.specialfog)
		{
			m_pShader->DisableAttribute(m_attribs.a_fogcoord);
		}
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	if(g_pCvarWireFrame->GetValue() >= 1)
	{
		m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
		m_pShader->DisableAttribute(m_attribs.a_texcoord);
		m_pShader->DisableAttribute(m_attribs.a_lmapcoord);

		if(g_pCvarWireFrame->GetValue() >= 2)
			glDisable(GL_DEPTH_TEST);

		glLineWidth(1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 0.0, 0.0, 1.0);
		if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_solidcolor)
			|| !m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none))
			return false;

		R_ValidateShader(m_pShader);

		for (Uint32 i = 0; i < m_texturesArray.size(); i++)
		{
			if(!m_texturesArray[i].pmodeltexture)
				continue;

			if(!m_texturesArray[i].nummultibatches)
				continue;

			drawbatch_t *pbatch = &m_texturesArray[i].multi_batches[0];
			for(Uint32 j = 0; j < m_texturesArray[i].nummultibatches; j++, pbatch++)
				glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if(g_pCvarWireFrame->GetValue() >= 2)
			glEnable(GL_DEPTH_TEST);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawFinalSpecular( void ) 
{
	if(!m_bumpMaps || g_pCvarBumpMaps->GetValue() < 1 || g_pCvarSpecular->GetValue() < 1)
		return true;

	glBlendFunc(GL_ONE, GL_ONE);

	//
	// Draw specular for the first layer
	//

	m_pShader->EnableAttribute(m_attribs.a_texcoord);
	m_pShader->EnableAttribute(m_attribs.a_lmapcoord);
	m_pShader->EnableAttribute(m_attribs.a_tangent);
	m_pShader->EnableAttribute(m_attribs.a_normal);
	m_pShader->EnableAttribute(m_attribs.a_binormal);

	if(!m_pShader->SetDeterminator(m_attribs.d_bumpmapping, TRUE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_specular, TRUE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_speconly, false))
		return false;

	m_pShader->SetUniform1i(m_attribs.u_difflightmap, 0);
	m_pShader->SetUniform1i(m_attribs.u_lightvecstex, 1);

	R_Bind2DTexture(GL_TEXTURE0, m_diffuseLightmapIndexes[BASE_LIGHTMAP_INDEX]);
	R_Bind2DTexture(GL_TEXTURE1, m_lightVectorsIndexes[BASE_LIGHTMAP_INDEX]);

	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		if(!m_texturesArray[i].pmodeltexture)
			continue;

		if(!m_texturesArray[i].nummultibatches)
			continue;

		mtexture_t *ptexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
		bsp_texture_t* ptexturehandle = &m_texturesArray[ptexture->infoindex];
		en_material_t* pmaterial = ptexturehandle->pmaterial;

		en_texture_t* pspecular = pmaterial->ptextures[MT_TX_SPECULAR];
		en_texture_t* pnormalmap = pmaterial->ptextures[MT_TX_NORMALMAP];
		if(!pspecular || !pnormalmap)
			continue;

		m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
		m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

		m_pShader->SetUniform1i(m_attribs.u_normalmap, 2);
		R_Bind2DTexture(GL_TEXTURE2, pnormalmap->palloc->gl_index);

		m_pShader->SetUniform1i(m_attribs.u_specular, 3);
		R_Bind2DTexture(GL_TEXTURE3, pspecular->palloc->gl_index);

		if (pmaterial->ptextures[MT_TX_AO])
		{
			if (!m_pShader->SetDeterminator(m_attribs.d_ao, TRUE))
				return false;

			en_texture_t* paotexture = pmaterial->ptextures[MT_TX_AO];
			m_pShader->SetUniform1i(m_attribs.u_aomap, 4);
			R_Bind2DTexture(GL_TEXTURE0 + 4, paotexture->palloc->gl_index);
		}
		else
		{
			if (!m_pShader->SetDeterminator(m_attribs.d_ao, FALSE))
				return false;
		}

		R_ValidateShader(m_pShader);

		drawbatch_t *pbatch = &m_texturesArray[i].multi_batches[0];
		for(Uint32 j = 0; j < m_texturesArray[i].nummultibatches; j++, pbatch++)
			glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
	}

	//
	// Draw specular for all other layers
	//
	if(m_useLightStyles)
	{
		for(Uint32 i = 0; i < m_texturesArray.size(); i++)
		{
			if(!m_texturesArray[i].pmodeltexture)
				continue;

			mtexture_t *ptexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
			bsp_texture_t* ptexturehandle = &m_texturesArray[ptexture->infoindex];
			en_material_t* pmaterial = ptexturehandle->pmaterial;

			if(ptexturehandle->lightstyleinfos.empty())
				continue;

			en_texture_t* pspecular = pmaterial->ptextures[MT_TX_SPECULAR];
			en_texture_t* pnormalmap = pmaterial->ptextures[MT_TX_NORMALMAP];
			if(!pspecular || !pnormalmap)
				continue;

			// Render per layer
			for(Uint32 j = 0; j < ptexturehandle->lightstyleinfos.size(); j++)
			{
				lightstyleinfo_t& styleinfo = ptexturehandle->lightstyleinfos[j];
				if(styleinfo.stylebatches.empty())
					continue;

				// Don't bother if the lightstyle value is off
				Float strength = (*m_pLightStyleValuesArray)[j];
				if(strength <= 0)
					continue;

				// Set the color value
				m_pShader->SetUniform4f(m_attribs.u_color, strength, strength, strength, 1.0);

				for(Uint32 k = 1; k < MAX_SURFACE_STYLES; k++)
				{
					// Check if we actually have any batches
					stylebatches_t& batches = styleinfo.stylebatches[k];
					if(batches.numbatches <= 0)
						continue;

					// Modify the lightmap coord pointer to use the appropriate texcoord
					m_pShader->SetAttributePointer(m_attribs.a_lmapcoord, OFFSET(bsp_vertex_t, lmapcoord[k]));

					m_pShader->SetUniform1f(m_attribs.u_phong_exponent, pmaterial->phong_exp*g_pCvarPhongExponent->GetValue());
					m_pShader->SetUniform1f(m_attribs.u_specularfactor, pmaterial->spec_factor);

					// Bind the appropriate lightmap layers
					R_Bind2DTexture(GL_TEXTURE0, m_diffuseLightmapIndexes[k]);
					R_Bind2DTexture(GL_TEXTURE1, m_lightVectorsIndexes[k]);

					// Now bind the other textures
					m_pShader->SetUniform1i(m_attribs.u_normalmap, 2);
					R_Bind2DTexture(GL_TEXTURE2, pnormalmap->palloc->gl_index);

					m_pShader->SetUniform1i(m_attribs.u_specular, 3);
					R_Bind2DTexture(GL_TEXTURE3, pspecular->palloc->gl_index);

					if (pmaterial->ptextures[MT_TX_AO])
					{
						if (!m_pShader->SetDeterminator(m_attribs.d_ao, TRUE))
							return false;

						en_texture_t* paotexture = pmaterial->ptextures[MT_TX_AO];
						m_pShader->SetUniform1i(m_attribs.u_aomap, 4);
						R_Bind2DTexture(GL_TEXTURE0 + 4, paotexture->palloc->gl_index);
					}
					else
					{
						if (!m_pShader->SetDeterminator(m_attribs.d_ao, FALSE))
							return false;
					}

					R_ValidateShader(m_pShader);

					drawbatch_t *pbatch = &batches.batches[0];
					for(Uint32 l = 0; l < batches.numbatches; l++, pbatch++)
						glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
				}
			}
		}

		// Re-set the lightmap coord to the first unit
		m_pShader->SetAttributePointer(m_attribs.a_lmapcoord, OFFSET(bsp_vertex_t, lmapcoord[0]));
		// Reset this to normal
		m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_specular, FALSE, false)
		|| !m_pShader->SetDeterminator(m_attribs.d_bumpmapping, FALSE, false))
		return false;

	m_pShader->DisableAttribute(m_attribs.a_tangent);
	m_pShader->DisableAttribute(m_attribs.a_binormal);
	m_pShader->DisableAttribute(m_attribs.a_normal);

	// Draw specular for dynamic lights
	if(!DrawLights(true))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::PrepareVSM( void )
{
	// Load current modelview
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// clear all chains
	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		bsp_texture_t& texture = m_texturesArray[i];

		texture.nummultibatches = 0;
		texture.numsinglebatches = 0;
		texture.psurfchain = nullptr;

		for(Uint32 j = 0; j < texture.lightstyleinfos.size(); j++)
		{
			lightstyleinfo_t& styleinfo = texture.lightstyleinfos[j];
			if(styleinfo.stylebatches.empty())
				continue;

			for(Uint32 k = 0; k < MAX_SURFACE_STYLES; k++)
				styleinfo.stylebatches[k].numbatches = 0;
		}
	}

	// Make sure this is reset
	m_addMulti = false;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawVSMFaces( void )
{
	// Render normal ones first
	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		if(!m_texturesArray[i].pmodeltexture)
			continue;

		// Nothing to draw
		if(!m_texturesArray[i].numsinglebatches)
			continue;

		// Get the animated texture
		mtexture_t *pworldtexture = TextureAnimation(m_texturesArray[i].pmodeltexture, m_pCurrentEntity->curstate.frame);
		bsp_texture_t *ptexturehandle = &m_texturesArray[pworldtexture->infoindex];
		en_material_t* pmaterial = ptexturehandle->pmaterial;

		if(pmaterial->flags & TX_FL_ALPHATEST)
		{
			m_pShader->SetUniform1i(m_attribs.u_maintexture, 0);
			R_Bind2DTexture(GL_TEXTURE0, pmaterial->ptextures[MT_TX_DIFFUSE]->palloc->gl_index);

			m_pShader->EnableAttribute(m_attribs.a_texcoord);
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_vsm_alpha))
				return false;

			R_ValidateShader(m_pShader);

			ptexturehandle = &m_texturesArray[i];
			drawbatch_t *pbatch = &ptexturehandle->single_batches[0];
			for(Uint32 j = 0; j < ptexturehandle->numsinglebatches; j++, pbatch++)
				glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));

			// Restore and disable
			m_pShader->DisableAttribute(m_attribs.a_texcoord);
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_vsm_store))
				return false;

			R_ValidateShader(m_pShader);

			ptexturehandle = &m_texturesArray[i];
			drawbatch_t *pbatch = &ptexturehandle->single_batches[0];
			for(Uint32 j = 0; j < ptexturehandle->numsinglebatches; j++, pbatch++)
				glDrawElements(GL_TRIANGLES, pbatch->end_index-pbatch->start_index, GL_UNSIGNED_INT, BUFFER_OFFSET(pbatch->start_index));
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawVSM( cl_dlight_t *dl, cl_entity_t** pvisents, Uint32 numentities, bool drawworld )
{
	// Set shader's VBO and bind it
	m_pShader->SetVBO(m_pVBO);
	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_position);
	if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false))
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	// Set static uniforms
	m_pShader->SetUniform1i(m_attribs.u_maintexture, 0);
	m_pShader->SetUniform4f(m_attribs.u_color, 1.0, 1.0, 1.0, 1.0);
	m_pShader->SetUniform1f(m_attribs.u_light_radius, dl->radius);
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	m_multiPass = false;

	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);

	// Set initial entity to world
	m_pCurrentEntity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);

	// Disable multipass functionalities
	m_disableMultiPass = true;

	PrepareVSM();

	if(drawworld)
		RecursiveWorldNode(ens.pworld->pnodes);

	// Check for errors
	bool result = true;

	// render non-moved ents here
	if(g_pCvarDrawEntities->GetValue() >= 1)
	{
		// Draw all static entities
		for (Uint32 i = 0; i < numentities; i++)
		{
			cl_entity_t *pEntity = pvisents[i];

			if(pEntity->pmodel->type != MOD_BRUSH)
				continue;

			if(R_IsEntityMoved(*pEntity))
				continue;

			if(R_IsEntityTransparent(*pEntity))
				continue;

			if(pEntity->curstate.renderfx == RenderFx_SkyEnt ||
				pEntity->curstate.renderfx == RenderFx_SkyEntScaled ||
				pEntity->curstate.renderfx == RenderFx_SkyEntNC ||
				pEntity->curstate.rendertype == RT_WATERSHADER ||
				pEntity->curstate.rendertype == RT_MIRROR ||
				pEntity->curstate.rendertype == RT_MONITORENTITY ||
				pEntity->curstate.rendertype == RT_PORTALSURFACE)
				continue;

			result = DrawBrushModel(*pEntity, true);
			if(!result)
				break;
		}
	}

	// Reset entity to world
	m_pCurrentEntity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);

	// Render all statics to vsm
	if(result)
		result = DrawVSMFaces();

	if(g_pCvarDrawEntities->GetValue() >= 1 && result)
	{
		// Now render moved entities seperately each
		for (Uint32 i = 0; i < numentities; i++)
		{
			cl_entity_t *pEntity = pvisents[i];

			if(pEntity->pmodel->type != MOD_BRUSH)
				continue;

			if(dl->isStatic() && !(pEntity->curstate.effects & EF_STATICENTITY))
				continue;

			if(!R_IsEntityMoved(*pEntity))
				continue;

			if(R_IsEntityTransparent(*pEntity))
				continue;

			if(pEntity->curstate.renderfx == RenderFx_SkyEnt)
				continue;

			if (pEntity->curstate.renderfx == RenderFx_SkyEntScaled)
				continue;

			if(pEntity->curstate.renderfx == RenderFx_SkyEntNC)
				continue;

			if(pEntity->curstate.rendertype == RT_WATERSHADER)
				continue;

			if(pEntity->curstate.rendertype == RT_MIRROR)
				continue;

			if(pEntity->curstate.rendertype == RT_MONITORENTITY)
				continue;

			if (pEntity->curstate.rendertype == RT_PORTALSURFACE)
				continue;

			result = BatchBrushModelForVSM(*pEntity, false);
			if(!result)
				break;
		}
	}

	// Re-enable multipass functionalities
	m_disableMultiPass = false;

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	if(!result)
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::BatchBrushModelForVSM( cl_entity_t& entity, bool isstatic )
{
	if(entity.curstate.rendermode != RENDER_NORMAL 
		&& entity.curstate.renderamt == 0)
		return true;

	if(entity.curstate.rendertype == RT_WATERSHADER 
		|| entity.curstate.rendertype == RT_MIRROR 
		|| entity.curstate.rendertype == RT_MONITORENTITY
		|| entity.curstate.rendertype == RT_PORTALSURFACE)
		return true;

	m_pCurrentEntity = &entity;
	const brushmodel_t* pmodel = entity.pmodel->getBrushmodel();

	Vector vorigin_local;
	Vector mins, maxs;

	Math::VectorCopy(rns.view.v_origin, vorigin_local);

	// Determine the mins/maxs
	if(R_IsEntityRotated(*m_pCurrentEntity))
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = m_pCurrentEntity->curstate.origin[i] - pmodel->radius;
			maxs[i] = m_pCurrentEntity->curstate.origin[i] + pmodel->radius;
		}
	}
	else
	{
		Math::VectorAdd(m_pCurrentEntity->curstate.origin, pmodel->mins, mins);
		Math::VectorAdd(m_pCurrentEntity->curstate.origin, pmodel->maxs, maxs);
	}

	// Do not culling on skybox entities
	if(m_pCurrentEntity->curstate.renderfx != RenderFx_SkyEnt &&
		m_pCurrentEntity->curstate.renderfx != RenderFx_SkyEntScaled && 
		rns.view.frustum.CullBBox(mins, maxs))
		return true;

	// Transform to local space the origin
	Math::VectorSubtract(vorigin_local, m_pCurrentEntity->curstate.origin, vorigin_local);
	if(R_IsEntityRotated(*m_pCurrentEntity))
		Math::RotateToEntitySpace(m_pCurrentEntity->curstate.angles, vorigin_local);

	if(!isstatic)
	{
		// Apply the transformation to the
		bool ismoved = R_IsEntityMoved(*m_pCurrentEntity);
		if(ismoved)
		{
			rns.view.modelview.PushMatrix();
			R_RotateForEntity(rns.view.modelview, *m_pCurrentEntity);
		}

		// Prepare and load the matrix
		PrepareVSM();
		
		if(ismoved)
			rns.view.modelview.PopMatrix();
	}

	// Batch the surfaces
	msurface_t* psurface = ens.pworld->psurfaces + pmodel->firstmodelsurface;
	for(Uint32 i = 0; i < pmodel->nummodelsurfaces; i++, psurface++)
	{
		plane_t* pplane = psurface->pplane;
		Float dp = Math::DotProduct(vorigin_local, pplane->normal) - pplane->dist;

		if(((psurface->flags & SURF_PLANEBACK) && (dp < -BACKFACE_EPSILON))
			|| (!(psurface->flags & SURF_PLANEBACK) && (dp > BACKFACE_EPSILON)))
		{
			if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			BatchSurface(psurface);
		}
	}

	if(!isstatic)
	{
		if(!DrawVSMFaces())
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::CreateDecal( const Vector& origin, const Vector& normal, decalgroupentry_t* pentry, byte flags, Float life, Float fadetime, Float growthtime )
{
	// Make sure the texture is loaded
	if(!pentry->ptexture)
	{
		gDecals.PrecacheTexture(pentry->name.c_str());
		if(!pentry->ptexture)
		{
			Con_Printf("%s - Could not load texture for entry '%s'.\n", __FUNCTION__, pentry->name.c_str());
			return;
		}
	}

	// Set mins/maxs
	Vector mins, maxs;
	Vector decalmins, decalmaxs;
	Float radius = (pentry->xsize > pentry->ysize) ? pentry->xsize : pentry->ysize;
	for(Uint32 i = 0; i < 3; i++)
	{
		decalmins[i] = origin[i]-radius;
		decalmaxs[i] = origin[i]+radius;
	}

	// Allocate decal
	bsp_decal_t *pdecal = nullptr;
	if (flags & FL_DECAL_PERSISTENT)
	{
		pdecal = new bsp_decal_t;
		m_staticDecalsArray.push_back(pdecal);
	}
	else
	{
		if(!(flags & FL_DECAL_ALLOWOVERLAP))
		{
			Uint32 counter = 0;
			m_decalsList.begin();
			while(!m_decalsList.end())
			{
				bsp_decal_t* pdecalcheck = m_decalsList.get();
				if(pdecalcheck->ptexinfo->pgroup != pentry->pgroup)
				{
					m_decalsList.next();
					continue;
				}

				const decalgroupentry_t *pdecalentry = pdecalcheck->ptexinfo;
				Float fldecalrad = (pdecalentry->xsize > pdecalentry->ysize) ? pdecalentry->xsize : pdecalentry->ysize;
				for(Uint32 i = 0; i < 3; i++)
				{
					mins[i] = pdecalcheck->origin[i]-fldecalrad;
					maxs[i] = pdecalcheck->origin[i]+fldecalrad;
				}

				if(!Math::CheckMinsMaxs(mins, maxs, decalmins, decalmaxs))
				{
					counter++;

					if(counter == MAX_DECAL_OVERLAP)
					{
						DeleteDecal(pdecalcheck);
						counter--;
					}
				}

				m_decalsList.next();
			}
		}

		pdecal = new bsp_decal_t();
		m_decalsList.add(pdecal);
	}

	if(!pdecal)
		return;

	pdecal->ptexinfo = pentry;
	pdecal->spawntime = CL_GetClientTime();
	pdecal->growthtime = growthtime;

	if(!(flags & FL_DECAL_PERSISTENT) && life > 0)
	{
		pdecal->life = life;
		pdecal->fadetime = fadetime;
	}

	Math::VectorCopy(origin, pdecal->origin);
	Math::VectorCopy(normal, pdecal->normal);

	m_pCurrentEntity = nullptr;
	RecursivePasteDecal(ens.pworld->pnodes, pdecal, flags, decalmins, decalmaxs);

	Vector localorigin, localnormal;
	for(Uint32 i = 1; i < MAX_RENDER_ENTITIES; i++)
	{
		cl_entity_t *pentity = CL_GetEntityByIndex(i);

		if ( !pentity )
			break;

		if ( !pentity->pmodel )
			continue;
		
		if ( pentity->pmodel->type != MOD_BRUSH )
			continue;

		if( pentity->curstate.rendermode != RENDER_NORMAL && !pentity->curstate.renderamt )
			continue;

		if(R_IsEntityMoved((*pentity)))
		{
			Math::VectorSubtract(origin, pentity->curstate.origin, localorigin);
			if(pentity->curstate.angles[0] || pentity->curstate.angles[1] || pentity->curstate.angles[2])
			{
				Math::RotateToEntitySpace(pentity->curstate.angles, localorigin);
				Math::RotateToEntitySpace(pentity->curstate.angles, localnormal);
			}
			else
			{
				// Just copy the normal
				Math::VectorCopy(normal, localnormal);
			}
		}
		else
		{
			Math::VectorCopy(normal, localnormal);
			Math::VectorCopy(origin, localorigin);
		}

		for(Uint32 j = 0; j < 3; j++)
		{
			mins[j] = localorigin[j]-radius;
			maxs[j] = localorigin[j]+radius;
		}

		if (Math::CheckMinsMaxs(pentity->pmodel->mins, pentity->pmodel->maxs, mins, maxs))
			continue;

		m_pCurrentEntity = pentity;
		const brushmodel_t* pbrushmodel = pentity->pmodel->getBrushmodel();
		msurface_t *psurfaces = ens.pworld->psurfaces + pbrushmodel->firstmodelsurface;

		// Process non-alphatested first
		for(Uint32 k = 0; k < pbrushmodel->nummodelsurfaces; k++)
		{
			msurface_t* psurf = &psurfaces[k];

			bsp_texture_t* ptexture = &m_texturesArray[psurf->ptexinfo->ptexture->infoindex];
			if(ptexture->pmaterial->flags & TX_FL_NODECAL)
				continue;

			if(ptexture->pmaterial->flags & TX_FL_ALPHATEST)
				continue;

			Float dot;
			plane_t *pplane = psurf->pplane;
			dot = Math::DotProduct(localorigin, pplane->normal) - pplane->dist;

			if(dot < 0)
				dot *= -1;

			if(dot < radius)
			{
				if(!(flags & FL_DECAL_NORMAL_PERMISSIVE))
				{
					Vector planenormal = pplane->normal;
					if(psurf->flags & SURF_PLANEBACK)
						Math::VectorScale(planenormal, -1, planenormal);

					if( Math::DotProduct(planenormal, localnormal) < 0.01 )
						continue;
				}

				DecalSurface(psurf, pdecal, localnormal, localorigin, false);
			}
		}

		// Process alpha tested per texture
		for(Uint32 k = 0; k < m_texturesArray.size(); k++)
		{
			bsp_texture_t* ptexture = &m_texturesArray[k];
			if(ptexture->pmaterial->flags & TX_FL_NODECAL)
				continue;

			if(!(ptexture->pmaterial->flags & TX_FL_ALPHATEST))
				continue;

			for(Uint32 l = 0; l < pbrushmodel->nummodelsurfaces; l++)
			{
				msurface_t* psurf = &psurfaces[l];
				if(psurf->ptexinfo->ptexture->infoindex != static_cast<Int32>(k))
					continue;

				Float dot;
				plane_t *pplane = psurf->pplane;
				dot = Math::DotProduct(localorigin, pplane->normal) - pplane->dist;

				if(dot < 0)
					dot *= -1;

				if(dot < radius)
				{
					if(!(flags & FL_DECAL_NORMAL_PERMISSIVE))
					{
						Vector planenormal = pplane->normal;
						if(psurf->flags & SURF_PLANEBACK)
							Math::VectorScale(planenormal, -1, planenormal);

						if( Math::DotProduct(planenormal, localnormal) < 0.01 )
							continue;
					}

					DecalSurface(psurf, pdecal, localnormal, localorigin, true);
				}
			}
		}
	}

	if(pdecal->polygroups.empty())
	{
		if(!(flags & FL_DECAL_PERSISTENT))
		{
			DeleteDecal(pdecal);
		}
		else
		{
			// Remove the static decal
			m_staticDecalsArray.resize(m_staticDecalsArray.size()-1);
		}

		// oh god I missed this
		return;
	}

	// Restore mins/maxs before finding touched leaves
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = origin[i]-radius;
		maxs[i] = origin[i]+radius;
	}

	Mod_FindTouchedLeafs(ens.pworld, pdecal->leafnums, mins, maxs, ens.pworld->pnodes);
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::DeleteDecal( bsp_decal_t *pdecal )
{
	// Remove it from the list
	m_decalsList.remove(pdecal);
	RemoveDecalFromVBO(pdecal);
	delete pdecal;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::RemoveDecalFromVBO( bsp_decal_t *pdelete )
{
	if(pdelete->polygroups.empty())
		return;

	decalpolygroup_t* pfirstgroup = pdelete->polygroups[0];
	decalpolygroup_t* plastgroup = pdelete->polygroups[pdelete->polygroups.size()-1];

	Uint32 vertexstart = pfirstgroup->start_vertex;
	Uint32 vertexend = plastgroup->start_vertex + plastgroup->num_vertexes;
	Uint32 nbremoved = vertexend - vertexstart;

	Uint32 nbshift = m_vertexCacheIndex - vertexend;
	m_vertexCacheIndex -= nbremoved;

	// See if there's anything to shift
	if(!nbshift)
		return;

	// Get VBO vertex data
	const bsp_vertex_t* pvertexdata = static_cast<const bsp_vertex_t*>(m_pDecalVBO->GetVBOData());
	const bsp_vertex_t* psrcdata = pvertexdata + vertexend;

	m_pDecalVBO->VBOSubBufferData(sizeof(bsp_vertex_t)*vertexstart, psrcdata, sizeof(bsp_vertex_t)*nbshift);

	m_decalsList.push_iterator();
	m_decalsList.begin();
	while(!m_decalsList.end())
	{
		bsp_decal_t* pdecal = m_decalsList.get();
		for(Uint32 i = 0; i < pdecal->polygroups.size(); i++)
		{
			decalpolygroup_t* pgroup = pdecal->polygroups[i];
			if(pgroup->start_vertex > vertexstart)
				pgroup->start_vertex -= nbremoved;
		}

		m_decalsList.next();
	}
	m_decalsList.pop_iterator();

	for(Uint32 i = 0; i < m_staticDecalsArray.size(); i++)
	{
		bsp_decal_t* pdecal = m_staticDecalsArray[i];
		for(Uint32 j = 0; j < pdecal->polygroups.size(); j++)
		{
			decalpolygroup_t* pgroup = pdecal->polygroups[j];
			if(pgroup->start_vertex > vertexstart)
				pgroup->start_vertex -= nbremoved;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::RecursivePasteDecal( mnode_t *node, bsp_decal_t *pdecal, byte flags, const Vector& mins, const Vector& maxs )
{
	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->contents < 0)
		return;

	if(Math::CheckMinsMaxs(node->mins, node->maxs, mins, maxs))
		return;

	Int32 side;
	Float dot;
	plane_t *plane = node->pplane;

	switch (plane->type)
	{
		case PLANE_X:
			dot = pdecal->origin[0] - plane->dist;	break;
		case PLANE_Y:
			dot = pdecal->origin[1] - plane->dist;	break;
		case PLANE_Z:
			dot = pdecal->origin[2] - plane->dist;	break;
		default:
			dot = Math::DotProduct(pdecal->origin, plane->normal) - plane->dist; 
			break;
	}

	if (dot >= 0) 
		side = 0;
	else 
		side = 1;

	// recurse down the children, front side first
	RecursivePasteDecal(node->pchildren[side], pdecal, flags, mins, maxs);

	// draw stuff
	if (node->numsurfaces)
	{
		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;

		Int32 xsize = pdecal->ptexinfo->xsize;
		Int32 ysize = pdecal->ptexinfo->ysize;

		Float radius = (xsize > ysize) ? xsize : ysize;

		msurface_t *surf = ens.pworld->psurfaces + node->firstsurface;
		for (Uint32 i = 0; i < node->numsurfaces; i++, surf++)
		{	
			plane_t *pplane = surf->pplane;
			Float dp = Math::DotProduct(pdecal->origin, pplane->normal) - pplane->dist;

			if(dp < 0)
				dp *= -1;

			if(dp < radius)
			{
				if(!(flags & FL_DECAL_NORMAL_PERMISSIVE))
				{
					Vector planenormal = pplane->normal;
					if(surf->flags & SURF_PLANEBACK)
						Math::VectorScale(planenormal, -1, planenormal);

					if(Math::DotProduct(planenormal, pdecal->normal) < 0.01)
						continue;
				}

				DecalSurface(surf, pdecal, pdecal->normal, pdecal->origin, false);
			}
		}
	}

	RecursivePasteDecal (node->pchildren[!side], pdecal, flags, mins, maxs);
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::DecalSurface( const msurface_t *surf, bsp_decal_t *pdecal, const Vector& normal, const Vector& origin, bool transparent )
{
	Vector right, up, tmp;

	Int32 numverts = 0;
	static Vector dverts1[64];
	static Vector dverts2[64];

	// Disregard water and sky
	if(surf->flags & SURF_DRAWTURB || surf->flags & SURF_DRAWSKY)
		return;

	// Extract vertexes
	for(Uint32 i = 0; i < surf->numedges; i++)
	{
		Int32 e_index = ens.pworld->psurfedges[surf->firstedge+i];
		if(e_index > 0)
			Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[e_index].vertexes[0]].origin, dverts1[i]);
		else
			Math::VectorCopy(ens.pworld->pvertexes[ens.pworld->pedges[-e_index].vertexes[1]].origin, dverts1[i]);
	}

	Math::GetUpRight(normal, up, right);

	Int32 xsize = pdecal->ptexinfo->xsize;
	Int32 ysize = pdecal->ptexinfo->ysize;

	Float texc_orig_x = Math::DotProduct(origin, right);
	Float texc_orig_y = Math::DotProduct(origin, up);

	Int32 nv;
	Vector planepoint;
	Math::VectorMA(origin, -xsize, right, planepoint);
	nv = Decal_ClipPolygon(dverts1, surf->numedges, right, planepoint, dverts2);
	if (nv < 3)
		return;

	Math::VectorMA(origin, xsize, right, planepoint);
	Math::VectorScale(right, -1, tmp);
	nv = Decal_ClipPolygon(dverts2, nv, tmp, planepoint, dverts1);
	if (nv < 3)
		return;

	Math::VectorMA(origin, -ysize, up, planepoint);
	nv = Decal_ClipPolygon(dverts1, nv, up, planepoint, dverts2);
	if (nv < 3)
		return;

	Math::VectorMA(origin, ysize, up, planepoint);
	Math::VectorScale(up, -1, tmp);
	nv = Decal_ClipPolygon(dverts2, nv, tmp, planepoint, dverts1);
	if (nv < 3)
		return;

	// see if we have enough space
	numverts = 3+(nv-3)*3;
	Int32 ioffset = GetDecalOffset(numverts);

	// See if we need to allocate
	decalpolygroup_t *pgroup = nullptr;
	if(!pdecal->polygroups.empty())
		pgroup = pdecal->polygroups[pdecal->polygroups.size()-1];

	if(!pgroup || ioffset < pgroup->start_vertex 
		|| pgroup->pentity != m_pCurrentEntity
		|| pgroup->alphatest != transparent
		|| pgroup->alphatest && pgroup->ptexture != surf->ptexinfo->ptexture)
	{
		pgroup = new decalpolygroup_t;
		pdecal->polygroups.push_back(pgroup);
		pgroup->pentity = m_pCurrentEntity;
		pgroup->start_vertex = ioffset;
		pgroup->alphatest = transparent;
		pgroup->localmins = NULL_MINS;
		pgroup->localmaxs = NULL_MAXS;
		pgroup->localorigin = pdecal->origin;

		if(m_pCurrentEntity && R_IsEntityMoved(*m_pCurrentEntity))
		{
			Math::VectorSubtract(pgroup->localorigin, m_pCurrentEntity->curstate.origin, pgroup->localorigin);
			if(m_pCurrentEntity)
				Math::RotateToEntitySpace(m_pCurrentEntity->curstate.angles, pgroup->localorigin);
		}

		if(pgroup->alphatest)
			pgroup->ptexture = surf->ptexinfo->ptexture;
	}

	if(static_cast<Int32>(m_tempDecalVertsArray.size()) < numverts)
	{
		const Uint32 currentsize = m_tempDecalVertsArray.size();
		m_tempDecalVertsArray.resize(currentsize+numverts);
	}
	
	// triangulate
	Int32 curvert = 0;
	Vector *pvert = dverts1;
	bsp_vertex_t pconvverts[3];
	for(Int32 j = 0; j < 3; j++, pvert++)
	{
		Float texc_x = (Math::DotProduct(*pvert, right) - texc_orig_x)/xsize;
		Float texc_y = (Math::DotProduct(*pvert, up) - texc_orig_y)/ysize;

		pconvverts[j].texcoord[0] = ((texc_x + 1)/2);
		pconvverts[j].texcoord[1] = ((texc_y + 1)/2);

		pconvverts[j].origin[0] = (*pvert)[0];
		pconvverts[j].origin[1] = (*pvert)[1];
		pconvverts[j].origin[2] = (*pvert)[2];
		pconvverts[j].origin[3] = 1.0;

		for(Uint32 k = 0; k < 3; k++)
		{
			if(pconvverts[j].origin[k] < pgroup->localmins[k])
				pgroup->localmins[k] = pconvverts[j].origin[k];

			if(pconvverts[j].origin[k] > pgroup->localmaxs[k])
				pgroup->localmaxs[k] = pconvverts[j].origin[k];
		}

		if(transparent)
		{
			pconvverts[j].dtexcoord[0] = Math::DotProduct(pconvverts[j].origin, surf->ptexinfo->vecs[0])+surf->ptexinfo->vecs[0][3];
			pconvverts[j].dtexcoord[0] /= static_cast<Float>(surf->ptexinfo->ptexture->width);

			pconvverts[j].dtexcoord[1] = Math::DotProduct(pconvverts[j].origin, surf->ptexinfo->vecs[1])+surf->ptexinfo->vecs[1][3];
			pconvverts[j].dtexcoord[1] /= static_cast<Float>(surf->ptexinfo->ptexture->height);
		}
	}

	memcpy(&m_tempDecalVertsArray[curvert], &pconvverts[0], sizeof(bsp_vertex_t)); curvert++;
	memcpy(&m_tempDecalVertsArray[curvert], &pconvverts[1], sizeof(bsp_vertex_t)); curvert++;
	memcpy(&m_tempDecalVertsArray[curvert], &pconvverts[2], sizeof(bsp_vertex_t)); curvert++;

	for(Int32 j = 0; j < (nv-3); j++, pvert++)
	{
		memcpy(&pconvverts[1], &pconvverts[2], sizeof(bsp_vertex_t));

		Float texc_x = (Math::DotProduct(*pvert, right) - texc_orig_x)/xsize;
		Float texc_y = (Math::DotProduct(*pvert, up) - texc_orig_y)/ysize;
		pconvverts[2].texcoord[0] = ((texc_x + 1)/2);
		pconvverts[2].texcoord[1] = ((texc_y + 1)/2);

		pconvverts[2].origin[0] = (*pvert)[0];
		pconvverts[2].origin[1] = (*pvert)[1];
		pconvverts[2].origin[2] = (*pvert)[2];
		pconvverts[2].origin[3] = 1.0;

		for(Uint32 k = 0; k < 3; k++)
		{
			if(pconvverts[2].origin[k] < pgroup->localmins[k])
				pgroup->localmins[k] = pconvverts[2].origin[k];

			if(pconvverts[2].origin[k] > pgroup->localmaxs[k])
				pgroup->localmaxs[k] = pconvverts[2].origin[k];
		}

		if(transparent)
		{
			pconvverts[2].dtexcoord[0] = Math::DotProduct(pconvverts[2].origin, surf->ptexinfo->vecs[0])+surf->ptexinfo->vecs[0][3];
			pconvverts[2].dtexcoord[0] /= static_cast<Float>(surf->ptexinfo->ptexture->width);

			pconvverts[2].dtexcoord[1] = Math::DotProduct(pconvverts[2].origin, surf->ptexinfo->vecs[1])+surf->ptexinfo->vecs[1][3];
			pconvverts[2].dtexcoord[1] /= static_cast<Float>(surf->ptexinfo->ptexture->height);
		}

		memcpy(&m_tempDecalVertsArray[curvert], &pconvverts[0], sizeof(bsp_vertex_t)); curvert++;
		memcpy(&m_tempDecalVertsArray[curvert], &pconvverts[1], sizeof(bsp_vertex_t)); curvert++;
		memcpy(&m_tempDecalVertsArray[curvert], &pconvverts[2], sizeof(bsp_vertex_t)); curvert++;
	}
 
	for(Uint32 j = 0; j < 3; j++)
	{
		Float size = pgroup->localmaxs[j] - pgroup->localmins[j];
		if(size > pgroup->radius)
			pgroup->radius = size;
	}

	m_pDecalVBO->VBOSubBufferData(ioffset*sizeof(bsp_vertex_t), &m_tempDecalVertsArray[0], numverts*sizeof(bsp_vertex_t));
	pgroup->num_vertexes += numverts;
}

//=============================================
// @brief
//
//=============================================
Uint32 CBSPRenderer::GetDecalOffset( Uint32 numverts )
{
	Uint32 cacheMaxOffset = (m_vertexCacheIndex-m_vertexCacheBase)+numverts;
	if(cacheMaxOffset > m_vertexCacheSize)
	{
		Uint32 missingSize = cacheMaxOffset - m_vertexCacheSize;
		Uint32 allocSize = missingSize / BSP_DECALVERT_ALLOC_SIZE;
		if(missingSize % BSP_DECALVERT_ALLOC_SIZE != 0)
			allocSize += BSP_DECALVERT_ALLOC_SIZE;

		// Expand VBO
		bsp_vertex_t *pvertexes = new bsp_vertex_t[allocSize];
		m_pDecalVBO->Append(pvertexes, sizeof(bsp_vertex_t)*allocSize, nullptr, 0);
		m_vertexCacheSize += allocSize;
		delete[] pvertexes;
	}

	Int32 offset = m_vertexCacheIndex;
	m_vertexCacheIndex += numverts;

	return offset;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawDecal( bsp_decal_t *pdecal, bool transparents, decal_rendermode_t& rendermode )
{
	for(Uint32 i = 0; i < pdecal->polygroups.size(); i++)
	{
		decalpolygroup_t *pgroup = pdecal->polygroups[i];

		if(transparents && !pgroup->pentity)
			continue;

		Vector mins, maxs;
		if(pgroup->pentity)
		{
			if(R_IsEntityRotated((*pgroup->pentity)))
			{
				Vector origin = pgroup->localorigin;
				Math::RotateFromEntitySpace(pgroup->pentity->curstate.angles, origin);

				for(Uint32 j = 0; j < 3; j++)
				{
					mins[j] = pgroup->pentity->curstate.origin[j] + (origin[j] - pgroup->radius);
					maxs[j] = pgroup->pentity->curstate.origin[j] + (origin[j] + pgroup->radius);
				}
			}
			else
			{
				Math::VectorAdd(pgroup->localmins, pgroup->pentity->curstate.origin, mins);
				Math::VectorAdd(pgroup->localmaxs, pgroup->pentity->curstate.origin, maxs);
			}
		}
		else
		{
			mins = pgroup->localmins;
			maxs = pgroup->localmaxs;
		}

		if(rns.view.frustum.CullBBox(mins, maxs))
			continue;

		// Determine rendering method required
		decal_rendermode_t requestedRendermode;
		if(pgroup->alphatest && pgroup->ptexture && pgroup->ptexture)
			requestedRendermode = DECAL_RENDERMODE_ALPHATEST;
		else
			requestedRendermode = DECAL_RENDERMODE_NORMAL;

		// Switch only if needed
		if(requestedRendermode != rendermode)
		{
			switch(requestedRendermode)
			{
			case DECAL_RENDERMODE_ALPHATEST:
				{
					m_pShader->EnableAttribute(m_attribs.a_dtexcoord);
					if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_decal_holes))
						return false;
				}
				break;
			case DECAL_RENDERMODE_NORMAL:
				{
					m_pShader->DisableAttribute(m_attribs.a_dtexcoord);
					if(!m_pShader->SetDeterminator(m_attribs.d_shadertype, shader_decal))
						return false;
				}
				break;
			}

			// Optimize state switches
			rendermode = requestedRendermode;
		}

		R_ValidateShader(m_pShader);

		if(pgroup->pentity)
		{
			if(!transparents && R_IsEntityTransparent(*pgroup->pentity)
				|| transparents && !R_IsEntityTransparent(*pgroup->pentity)
				|| pgroup->pentity->visframe != rns.framecount)
				continue;

			if(R_IsEntityMoved(*pgroup->pentity))
			{
				rns.view.modelview.PushMatrix();
				R_RotateForEntity(rns.view.modelview, *pgroup->pentity);
			
				// load modelview and pop
				m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
				rns.view.modelview.PopMatrix();
			}
		}

		Float decalalpha;
		if(pdecal->life > 0 && pdecal->fadetime)
		{
			// Apply fade
			Double fadebegin = pdecal->spawntime + (pdecal->life - pdecal->fadetime);
			if(fadebegin > rns.time)
			{
				// No fade yet
				decalalpha = 1.0;
			}
			else
			{
				decalalpha = 1.0 - ((rns.time - fadebegin) / pdecal->fadetime);
				decalalpha = clamp(decalalpha, 0.0, 1.0);
			}
		}
		else
		{
			// No fade
			decalalpha = 1.0;
		}

		Float decalscale;
		if(pdecal->growthtime > 0 && rns.time < (pdecal->spawntime + pdecal->growthtime))
		{
			decalscale = (rns.time - pdecal->spawntime) / pdecal->growthtime;
			decalscale = clamp(decalscale, 0.0, 1.0);
			decalscale = (1.0 - decalscale) * 10 + decalscale;
		}
		else
		{
			// No growth
			decalscale = 1.0;
		}

		m_pShader->SetUniform1f(m_attribs.u_decalalpha, decalalpha);
		m_pShader->SetUniform1f(m_attribs.u_decalscale, decalscale);

		if (pgroup->alphatest && pgroup->ptexture 
			&& pgroup->ptexture && pgroup->ptexture->infoindex != NO_INFO_INDEX)
		{
			bsp_texture_t* ptexturehandle = &m_texturesArray[pgroup->ptexture->infoindex];
			en_texture_t* ptexture = ptexturehandle->pmaterial->ptextures[MT_TX_DIFFUSE];
			R_Bind2DTexture(GL_TEXTURE1, ptexture->palloc->gl_index);
		}

		R_Bind2DTexture(GL_TEXTURE0, pdecal->ptexinfo->ptexture->palloc->gl_index);
		glDrawArrays(GL_TRIANGLES, pgroup->start_vertex, pgroup->num_vertexes);

		// Reload the original if needed
		if(pgroup->pentity && R_IsEntityMoved(*pgroup->pentity))	
			m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawDecals( bool transparents )
{
	if(m_staticDecalsArray.empty() && m_decalsList.empty())
		return true;

	if(rns.fog.settings.active)
	{
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0.5, 0.5, 0.5);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end*0.8f, 1.0f/(static_cast<Float>(rns.fog.settings.end)*0.8f- static_cast<Float>(rns.fog.settings.start)));

		if(rns.fog.specialfog)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_fogcoord, false))
				return false;
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_radial, false))
				return false;
		}
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_fogtype, fog_none, false))
			return false;
	}

	Int32 alphatestMode = (rns.msaa && rns.mainframe) ? ALPHATEST_COVERAGE : ALPHATEST_LESSTHAN;
	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, alphatestMode, false))
		return false;

	if(alphatestMode == ALPHATEST_COVERAGE)
	{
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		gGLExtF.glSampleCoverage(0.5, GL_FALSE);
	}

	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	// reload modelview
	m_pShader->SetUniform1i(m_attribs.u_maintexture, 0);
	m_pShader->SetUniform1i(m_attribs.u_detailtex, 1);
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	if(transparents)
		glDisable(GL_CULL_FACE);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	glPolygonOffset(-1, -1);
	glEnable(GL_POLYGON_OFFSET_FILL);

	decal_rendermode_t renderMode = DECAL_RENDERMODE_NONE;

	for(Uint32 i = 0; i < m_staticDecalsArray.size(); i++)
	{
		if(!Common::CheckVisibility(m_staticDecalsArray[i]->leafnums, rns.pvisbuffer))
			continue;

		if(!DrawDecal(m_staticDecalsArray[i], transparents, renderMode))
			return false;
	}

	m_decalsList.begin();
	while(!m_decalsList.end())
	{
		bsp_decal_t* pdecal = m_decalsList.get();
		if(Common::CheckVisibility(pdecal->leafnums, rns.pvisbuffer))
		{
			if(!DrawDecal(pdecal, transparents, renderMode))
				return false;
		}
		m_decalsList.next();
	}

	glDepthMask(GL_TRUE);

	glDisable(GL_BLEND);
	glDisable(GL_POLYGON_OFFSET_FILL);

	if(transparents)
		glEnable(GL_CULL_FACE);

	if(!m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false))
		return false;

	if(alphatestMode == ALPHATEST_COVERAGE)
	{
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		gGLExtF.glSampleCoverage(1.0, GL_FALSE);
	}

	m_pShader->DisableAttribute(m_attribs.a_texcoord);
	m_pShader->DisableAttribute(m_attribs.a_dtexcoord);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBSPRenderer::DrawNormalDecals( void )
{
	// Set shader's VBO
	m_pShader->SetVBO(m_pDecalVBO);
	m_pDecalVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Rendering error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_position);

	// Set the projection matrix
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Draw decals last
	bool result = DrawDecals(false);

	// Make sure to restore this
	if(result)
		result = m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED, false);

	m_pShader->DisableShader();
	m_pDecalVBO->UnBind();

	return result;
}

//=============================================
// @brief
//
//=============================================
Float CBSPRenderer::CalcFogCoord( Float z )
{
	Float fogcoord;
	Float start = 800; // set for detour07f
	Float end;

	if(z < 0) end = abs((ens.pworld->mins.z+SPECIALFOG_DISTANCE));
	else end = abs((ens.pworld->maxs.z-SPECIALFOG_DISTANCE));

	fogcoord = (end-abs(z))/(end-start);
	if(fogcoord > 1) fogcoord = 1;
	if(fogcoord < 0) fogcoord = 0;

	return (1.0-fogcoord)*rns.fog.settings.end;
}

//=============================================
// @brief
//
//=============================================
void CBSPRenderer::Think( void )
{
	if(!m_decalsList.empty())
	{
		CLinkedList<bsp_decal_t*> removedDecalsList;

		m_decalsList.begin();
		while(!m_decalsList.end())
		{
			bsp_decal_t* pdecal = m_decalsList.get();
			if(pdecal->life > 0)
			{
				Double deathtime = pdecal->spawntime + pdecal->life;
				if(deathtime <= cls.cl_time)
				{
					// Delete this decal
					removedDecalsList.add(pdecal);
					m_decalsList.remove(m_decalsList.get_link());
					m_decalsList.next();
				}
			}

			m_decalsList.next();
		}

		removedDecalsList.begin();
		while(!removedDecalsList.end())
		{
			bsp_decal_t* pdecal = removedDecalsList.get();
			RemoveDecalFromVBO(pdecal);
			delete pdecal;

			removedDecalsList.next();
		}
	}
}