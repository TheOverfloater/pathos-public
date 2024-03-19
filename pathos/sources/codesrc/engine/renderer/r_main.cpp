/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <ctime>

#include "includes.h"
#include "system.h"
#include "window.h"
#include "vid.h"
#include "enginestate.h"
#include "texturemanager.h"
#include "r_glextf.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_main.h"
#include "r_basicdraw.h"
#include "r_menu.h"
#include "r_menuparticles.h"
#include "r_common.h"
#include "input.h"

#include "cl_entity.h"
#include "cl_main.h"
#include "cl_utils.h"
#include "commands.h"
#include "cl_pmove.h"
#include "cache_model.h"
#include "modelcache.h"
#include "com_math.h"
#include "cl_tempentities.h"
#include "cl_efx.h"
#include "file.h"
#include "ald.h"
#include "tga.h"

#include "uimanager.h"
#include "uielements.h"
#include "com_math.h"
#include "frustum.h"
#include "r_bsp.h"
#include "r_text.h"
#include "console.h"
#include "cl_snd.h"
#include "r_interface.h"
#include "r_dlights.h"
#include "r_rttcache.h"
#include "r_glowaura.h"
#include "r_decals.h"
#include "r_water.h"
#include "r_mirror.h"
#include "r_monitor.h"
#include "r_particles.h"
#include "r_sprites.h"
#include "r_postprocess.h"
#include "r_cubemaps.h"
#include "r_cables.h"
#include "r_vbm.h"
#include "r_legacyparticles.h"
#include "r_beams.h"
#include "r_portals.h"
#include "r_blackhole.h"
#include "r_lensflare.h"
#include "r_glqueries.h"
#include "r_wadtextures.h"
#include "r_sky.h"

#include "stepsound.h"

// Global cvars
CCVar* g_pCvarBumpMaps = nullptr;
CCVar* g_pCvarDrawEntities = nullptr;
CCVar* g_pCvarPhongExponent = nullptr;
CCVar* g_pCvarWireFrame = nullptr;
CCVar* g_pCvarSpecular = nullptr;
CCVar* g_pCvarCaustics = nullptr;
CCVar* g_pCvarFarZ = nullptr;
CCVar* g_pCvarShadows = nullptr;
CCVar* g_pCvarGaussianBlur = nullptr;
CCVar* g_pCvarDynamicLights = nullptr;
CCVar* g_pCvarNoFBO = nullptr;
CCVar* g_pCvarStats = nullptr;
CCVar* g_pCvarCubemaps = nullptr;
CCVar* g_pCvarDrawOrigins = nullptr;
CCVar* g_pCvarAnisotropy = nullptr;
CCVar* g_pCvarWadTextureChecks = nullptr;
CCVar* g_pCvarBspTextureChecks = nullptr;
CCVar* g_pCvarGLSLOnDemand = nullptr;
CCVar* g_pCvarGLSLActiveLoad = nullptr;
CCVar* g_pCvarGLSLActiveMaxShaders = nullptr;
CCVar* g_pCvarHighlightEntity = nullptr;
CCVar* g_pCvarModelLightMultiplier = nullptr;
CCVar* g_pCvarGLSLValidate = nullptr;
CCVar* g_pCvarSkipFrames = nullptr;
CCVar* g_pCvarGraphHeight = nullptr;
CCVar* g_pCvarTimeGraph = nullptr;
CCVar* g_pCvarOcclusionQueries = nullptr;
CCVar* g_pCvarTraceGlow = nullptr;

// Caustics texture list file path
static const Char CAUSTICS_TEXTURE_FILE_PATH[] = "textures/general/caustics_textures.txt";
// Projective texture list file path
static const Char PROJECTIVE_TEXTURE_FILE_PATH[] = "textures/general/projective_textures.txt";
// Path for the rotating lights sprite
static const Char ROTATING_LIGHT_SPRITE_PATH[] = "sprites/emerg_flare.spr";

// Reference width for the logo text
static const Uint32 LOAD_TEXT_BASE_WIDTH = 512;
// Reference width for the logo text
static const Uint32 LOAD_TEXT_BASE_HEIGHT = 128;

// Reference width for the logo text
static const Uint32 PAUSED_TEXT_BASE_WIDTH = 512;
// Reference width for the logo text
static const Uint32 PAUSED_TEXT_BASE_HEIGHT = 128;

// Max extensions per line
static const Uint32 MAX_EXTENSIONS_PER_LINE = 6;

// Max active-load shaders per frame
static const Uint32 MAX_ACTIVELOAD_SHADERS = 4;

// Max timings on time graph
static const Uint32 MAX_TIMINGS = 400;

// Holds rendering related information
renderer_state_t rns;

// External function loading class
CGLExtF gGLExtF;

// Rotating light sprite model
static cache_model_t* g_pRotLightSprite = nullptr;

// Used by beam testing
static Vector g_beamStartPosition;
// Used by beam testing
static cl_entity_t* g_pBeamStartEntity = nullptr;
// Used by beam testing
static Vector g_beamEndPosition;
// Used by beam testing
static cl_entity_t* g_pBeamEndEntity = nullptr;

// Default material list set by r_list_default_materials
CArray<CString> g_defaultMaterialPMFList;
// Material currently being shown on scren
en_material_t* g_pMaterialShown = nullptr;

// List of pending shaders to load
CLinkedList<active_load_shader_t> g_pendingShadersList;

//====================================
//
//====================================
bool R_Init( void )
{
	// Init cvars
	g_pCvarBumpMaps = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_bumpmaps", "1", "Toggles normal mapping");
	g_pCvarSpecular = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_specular", "1", "Toggles specular highlights");
	g_pCvarDrawEntities = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_drawentities", "1", "Toggles rendering of entities");
	g_pCvarPhongExponent = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_phong_exponent", "1", "Phong exponent setting");
	g_pCvarWireFrame = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_wireframe", "0", "Toggle wireframe rendering");
	g_pCvarCaustics = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_water_caustics", "1", "Toggle water caustics");
	g_pCvarFarZ = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_farz", "16384", "Far clipping plane distance");
	g_pCvarShadows = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_shadows", "1", "Controls rendering of shadows" );
	g_pCvarGaussianBlur = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_gaussianblur", "1", "Toggles the use of gaussian blurring effects" );
	g_pCvarDynamicLights = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_dynamiclights", "1", "Toggles dynamic light effects" );
	g_pCvarNoFBO = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_nofbo", "0", "Enable/Disable FBO usage" );
	g_pCvarStats = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_stats", "0", "Toggle render statistics info printing" );
	g_pCvarCubemaps = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_cubemaps", "1", "Toggles cubemap reflections" );
	g_pCvarDrawOrigins = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_draworigins", "0", "Toggle rendering of origin points");
	g_pCvarAnisotropy = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_GL_DEPENDENT|FL_CV_CLIENT|FL_CV_SAVE), ANISOTROPY_CVAR_NAME, "0", "Controls texture anisotropy", R_AnisotropyCvarCallBack);
	g_pCvarWadTextureChecks = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_wadtexturechecks", "0", "Perform checks for WAD textures without scripts." );
	g_pCvarBspTextureChecks = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT | FL_CV_SAVE), "r_bsptexturechecks", "0", "Perform checks for BSP textures without scripts.");
	g_pCvarGLSLOnDemand = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_glsl_ondemand", "1", "On-demand load GLSL shaders." );
	g_pCvarGLSLActiveLoad = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_glsl_activeload", "1", "Load shaders during active runtime." );
	g_pCvarGLSLActiveMaxShaders = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_glsl_activeload_max_shaders", "4", "Max number of shaders loaded each frame for active load.", R_ActiveLoadMaxShadersCvarCallBack);
	g_pCvarHighlightEntity = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_highlightentity", "0", "Set a specific entity to be highlighted");
	g_pCvarModelLightMultiplier = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_modellightfactor", "1.0", "Corrective scaling factor for model lights");
	g_pCvarGLSLValidate = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT), "r_glsl_validate", "0", "Toggle shader validation debug functionality." );
	g_pCvarSkipFrames = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_skipframes", "2", "Number of frames to skip after game initialization." );
	g_pCvarGraphHeight = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_graphheight", "15", "Height of the time graph." );
	g_pCvarTimeGraph = gConsole.CreateCVar( CVAR_FLOAT, FL_CV_CLIENT, "r_timegraph", "0", "Show render performance timegraph." );
	g_pCvarOcclusionQueries = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_glowocclusion", "1", "Toggles the use of occlusion queries for glows." );
	g_pCvarTraceGlow = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_traceglow", "0", "Enable/disable performance intensive trace tests." );

	gCommands.CreateCommand("r_exportald", ALD_ExportLightmaps, "Exports current lightmap info as nightstage light info");
	gCommands.CreateCommand("r_detail_auto", Cmd_DetailAuto, "Generates detail texture entries for world textures without");
	gCommands.CreateCommand("r_list_default_materials", Cmd_ListDefaultMaterials, "List world textures with default material types");
	gCommands.CreateCommand("r_set_texture_material", Cmd_SetTextureMaterialType, "Set material type in list of default material types");
	gCommands.CreateCommand("r_show_list_material", Cmd_ShowListMaterial, "Show material from default list on-screen");

	gCommands.CreateCommand("pastedecal", Cmd_PasteDecal, "Creates a decal in front of the view");
	gCommands.CreateCommand("createsprite", Cmd_CreateSprite, "Creates a sprite in front of the view");
	gCommands.CreateCommand("createdynlight", Cmd_CreateDynamicLight, "Creates a dynamic light");
	gCommands.CreateCommand("createspotlight", Cmd_CreateSpotLight, "Creates a dynamic spotlight");
	gCommands.CreateCommand("loadmodel", Cmd_LoadModel, "Loads a model.");

	gCommands.CreateCommand("efx_tempsprite", Cmd_EFX_TempSprite, "Creates a temporary sprite entity.");
	gCommands.CreateCommand("efx_tempmodel", Cmd_EFX_TempModel, "Creates a temporary model entity.");
	gCommands.CreateCommand("efx_funnelsprite", Cmd_EFX_FunnelSprite, "Creates a funnel sprite effect.");
	gCommands.CreateCommand("efx_spheremodel", Cmd_EFX_SphereModel, "Creates a spherical shower of models.");
	gCommands.CreateCommand("efx_breakmodel", Cmd_EFX_BreakModel, "Creates a volume of breakage gib models.");
	gCommands.CreateCommand("efx_bubbletrail", Cmd_EFX_BubbleTrail, "Creates a trail of bubble sprites.");
	gCommands.CreateCommand("efx_bubbles", Cmd_EFX_Bubbles, "Creates a volume of bubble sprites.");

	gCommands.CreateCommand("efx_particleexplosion1", Cmd_EFX_ParticleExplosion1, "Creates a legacy explosion particle effect");
	gCommands.CreateCommand("efx_particleexplosion2", Cmd_EFX_ParticleExplosion2, "Creates a legacy explosion particle effect");
	gCommands.CreateCommand("efx_blobexplosion", Cmd_EFX_BlobExplosion, "Creates a legacy blob explosion particle effect");
	gCommands.CreateCommand("efx_rocketexplosion", Cmd_EFX_CreateRocketExplosion, "Creates a legacy rocket explosion particle effect");
	gCommands.CreateCommand("efx_particleeffect", Cmd_EFX_CreateParticleEffect, "Creates a legacy particle effect");
	gCommands.CreateCommand("efx_lavasplash", Cmd_EFX_CreateLavaSplash, "Creates a legacy lava splash particle effect");
	gCommands.CreateCommand("efx_teleportsplash", Cmd_EFX_CreateTeleportSplash, "Creates a legacy teleport particle effect");
	gCommands.CreateCommand("efx_rockettrail", Cmd_EFX_CreateRocketTrail, "Creates a legacy rocket trail particle effect");

	gCommands.CreateCommand("efx_beamsetstart", Cmd_EFX_BeamSetStart, "Sets beam start entity and/or position");
	gCommands.CreateCommand("efx_beamsetend", Cmd_EFX_BeamSetEnd, "Sets beam start entity and/or position");
	gCommands.CreateCommand("efx_beamlightning", Cmd_EFX_BeamLightning, "Creates a lightning beam");
	gCommands.CreateCommand("efx_beamcirclepoints", Cmd_EFX_BeamCirclePoints, "Creates beam lightning in a circle");
	gCommands.CreateCommand("efx_beamentitypoint", Cmd_EFX_BeamEntityPoint, "Creates a beam between a point and an entity");
	gCommands.CreateCommand("efx_beamentities", Cmd_EFX_BeamEntities, "Creates a beam between two entities");
	gCommands.CreateCommand("efx_beamfollow", Cmd_EFX_BeamFollow, "Creates a beam that follows an entity");
	gCommands.CreateCommand("efx_beamvaportrail", Cmd_EFX_BeamVaporTrail, "Creates a vapor trail beam");
	gCommands.CreateCommand("efx_beampoints", Cmd_EFX_BeamPoints, "Creates a beam between two points");
	gCommands.CreateCommand("efx_beamring", Cmd_EFX_BeamRing, "Creates a beam ring between two entities");
	gCommands.CreateCommand("efx_createparticle", Cmd_EFX_CreateParticle, "Creates a particle system in front of the view");

	gCommands.CreateCommand("r_bsp2smd_lm", Cmd_BSPToSMD_Lightmap, "Exports BSP geometry to smd with lightmap texcoords");
	gCommands.CreateCommand("r_bsp2smd_tx", Cmd_BSPToSMD_Textures, "Exports BSP geometry to smd with regular texcoords");

	gCommands.CreateCommand("timerefresh", Cmd_TimeRefresh, "Command for measuring rendering performance");

	gCommands.CreateCommand("r_loadallparticlescripts", Cmd_LoadAllParticleScripts, "Load all particle scripts in particle script folder");

	//
	// Init classes
	//

	if(!gMenu.InitGL())
		return false;

	if(!gSkyRenderer.Init())
		return false;

	if(!gBSPRenderer.Init())
		return false;

	if(!gDynamicLights.Init())
		return false;

	if(!gGlowAura.Init())
		return false;

	if(!gWaterShader.Init())
		return false;

	if(!gMonitorManager.Init())
		return false;

	if(!gParticleEngine.Init())
		return false;

	if(!gSpriteRenderer.Init())
		return false;

	if(!gCubemaps.Init())
		return false;

	if(!gPostProcess.Init())
		return false;

	if(!gCableRenderer.Init())
		return false;

	if(!gVBMRenderer.Init())
		return false;

	if(!gLegacyParticles.Init())
		return false;

	if(!gBeamRenderer.Init())
		return false;

	if(!gPortalManager.Init())
		return false;

	if(!gBlackHoleRenderer.Init())
		return false;

	if(!gLensFlareRenderer.Init())
		return false;

	// Init decal class
	gDecals.Init();

	return true;
}

//====================================
//
//====================================
void R_Shutdown( void )
{
	// Shut down classes
	gSkyRenderer.Shutdown();
	gBSPRenderer.Shutdown();
	gDynamicLights.Shutdown();
	gGlowAura.Shutdown();
	gWaterShader.Shutdown();
	gMonitorManager.Shutdown();
	gParticleEngine.Shutdown();
	gSpriteRenderer.Shutdown();
	gCubemaps.Shutdown();
	gPostProcess.Shutdown();
	gCableRenderer.Shutdown();
	gVBMRenderer.Shutdown();
	gLegacyParticles.Shutdown();
	gBeamRenderer.Shutdown();
	gPortalManager.Shutdown();
	gLensFlareRenderer.Shutdown();

	CBasicDraw::DeleteInstance();
}

//=============================================
// @brief
//
//=============================================
bool R_LoadTextureListFile( const Char* pstrTextureListFile, CArray<en_texture_t*>& texArray, rs_level_t level, const GLint* pborder, bool clamp )
{
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(pstrTextureListFile, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, pstrTextureListFile);
		return false;
	}

	// Read opening bracket
	Char token[MAX_PARSE_LENGTH];
	const Char* pstr = Common::Parse(reinterpret_cast<const Char*>(pfile), token);
	if(qstrcmp(token, "{"))
	{
		Con_Printf("%s - Expected '{', got '%s' instead in '%s'.\n", __FUNCTION__, token, pstrTextureListFile);
		FL_FreeFile(pfile);
		return false;
	}

	Int32 flags = TX_FL_NONE;
	if(clamp)
		flags |= (TX_FL_CLAMP_S|TX_FL_CLAMP_T);
	if(pborder)
		flags |= (TX_FL_BORDER|TX_FL_NOMIPMAPS);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Read in the entries
	while(true)
	{
		// Read in next token
		pstr = Common::Parse(pstr, token);

		if(!qstrcmp(token, "}"))
			break;

		if(!pstr)
		{
			Con_Printf("%s - Unexpected EOF reached in '%s'.\n", __FUNCTION__, pstrTextureListFile);
			FL_FreeFile(pfile);
			texArray.clear();
			return false;
		}

		// Load the file in
		en_texture_t* ptexture = pTextureManager->LoadTexture(token, level, flags, pborder);
		if(ptexture)
			texArray.push_back(ptexture);
	}

	// Release the file
	FL_FreeFile(pfile);

	// Signal if load was successful
	if(texArray.empty())
		return false;
	else
		return true;
}

//====================================
//
//====================================
void R_LoadTextures( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Load the loading text logo
	if(!rns.objects.ploadinglogo)
	{
		rns.objects.ploadinglogo = pTextureManager->LoadTexture("menu/loading_text.tga", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
		if(!rns.objects.ploadinglogo)
			rns.objects.ploadinglogo = pTextureManager->GetDummyTexture();
	}

	// Load the loading text logo
	if(!rns.objects.ppausedlogo)
	{
		rns.objects.ppausedlogo = pTextureManager->LoadTexture("menu/paused_text.tga", RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
		if(!rns.objects.ppausedlogo)
			rns.objects.ppausedlogo = pTextureManager->GetDummyTexture();
	}

	// Load caustics textures
	if(!R_LoadTextureListFile(CAUSTICS_TEXTURE_FILE_PATH, rns.objects.caustics_textures, RS_WINDOW_LEVEL, nullptr, false))
	{
		Con_Printf("%s - Couldn't load caustics textures file '%s'.\n", __FUNCTION__, CAUSTICS_TEXTURE_FILE_PATH);
		rns.objects.caustics_textures.push_back(pTextureManager->GetDummyTexture());
	}

	// Load projective textures
	GLint border[4] = {0, 0, 0, 0};
	if(!R_LoadTextureListFile(PROJECTIVE_TEXTURE_FILE_PATH, rns.objects.projective_textures, RS_WINDOW_LEVEL, border, true))
	{
		Con_Printf("%s - Couldn't load projective textures file '%s'.\n", __FUNCTION__, PROJECTIVE_TEXTURE_FILE_PATH);
		rns.objects.projective_textures.push_back(pTextureManager->GetDummyTexture());
	}
}

//====================================
//
//====================================
bool R_InitGL( void )
{
	// Set screen size, as the classes depend on it
	rns.screenwidth = gWindow.GetWidth();
	rns.screenheight = gWindow.GetHeight();

	// Populate extensions list
	R_PopulateExtensionsArray();

	// Load any textures
	R_LoadTextures();

	// Determine FBO use
	if(R_IsExtensionSupported("GL_EXT_framebuffer_object") || R_IsExtensionSupported("GL_ARB_framebuffer_object"))
	{
		rns.fboused = (g_pCvarNoFBO->GetValue() > 0) ? false : true;
		rns.nofbo = rns.fboused ? false : true;

		Con_Printf("Framebuffer objects are enabled.\n");
	}
	else
	{
		rns.fboused = false;
		rns.nofbo = true;

		Con_Printf("Framebuffer objects are disabled.\n");
	}

	if(rns.nofbo)
		Con_Printf("Warning: FBOs are not enabled. Shadow maps will not be available.\n");

	// Create basic draw instance
	CBasicDraw* pDraw = CBasicDraw::CreateInstance();

	if(!gConsole.InitGL())
		return false;

	// Reload save textures
	if(!gMenu.InitGL())
		return false;

	// Init render classes
	if(!pDraw->InitGL(gGLExtF, FL_GetInterface(), Sys_ErrorPopup))
		return false;

	if(!gText.InitGL())
		return false;

	// Initialize menu
	if(!gMenu.Init())
		return false;

	// Mark this
	rns.basicsinitialized = true;

	// Draw the menu loading screen
	VID_BeginLoading(false);

	// Initialize menu particles
	if(!gMenuParticles.Init())
		return false;

	// Initialize classes
	if(!gSkyRenderer.InitGL())
		return false;

	// Initialize classes
	if(!gBSPRenderer.InitGL())
		return false;

	if(!gDynamicLights.InitGL())
		return false;

	// Initialize any RTT textures
	gRTTCache.InitGL();

	if(!gGlowAura.InitGL())
		return false;

	if(!gWaterShader.InitGL())
		return false;

	if(!gMonitorManager.InitGL())
		return false;

	if(!gMirrorManager.InitGL())
		return false;

	if(!gParticleEngine.InitGL())
		return false;

	if(!gSpriteRenderer.InitGL())
		return false;

	if(!gCubemaps.InitGL())
		return false;

	if(!gPostProcess.InitGL())
		return false;

	if(!gCableRenderer.InitGL())
		return false;

	if(!gVBMRenderer.InitGL())
		return false;

	if(!gLegacyParticles.InitGL())
		return false;

	if(!gBeamRenderer.InitGL())
		return false;

	if(!gPortalManager.InitGL())
		return false;

	if(!gBlackHoleRenderer.InitGL())
		return false;

	// Draw the menu loading screen
	VID_DrawLoadingScreen();

	if(!cls.dllfuncs.pfnGLInit())
		return false;

	// Create query objects
	R_InitQueryObjects();

	// Set whether MSAA is enabled
	rns.msaa = gWindow.IsMSAAEnabled();

	return true;
}

//====================================
//
//====================================
void R_ShutdownGL( void )
{
	// Release GL data from render classes
	CBasicDraw::GetInstance()->ClearGL();
	gText.ClearGL();

	// Clear classes
	gMenu.ClearGL();
	gMenuParticles.ClearGL();
	gSkyRenderer.ClearGL();
	gBSPRenderer.ClearGL();
	gDynamicLights.ClearGL();
	gGlowAura.ClearGL();
	gWaterShader.ClearGL();
	gMonitorManager.ClearGL();
	gMirrorManager.ClearGL();
	gParticleEngine.ClearGL();
	gSpriteRenderer.ClearGL();
	gCubemaps.ClearGL();
	gPostProcess.ClearGL();
	gCableRenderer.ClearGL();
	gVBMRenderer.ClearGL();
	gLegacyParticles.ClearGL();
	gBeamRenderer.ClearGL();
	gPortalManager.ClearGL();
	gBlackHoleRenderer.ClearGL();

	// Reset cache states
	gModelCache.ClearGL();

	// Reset this
	rns.basicsinitialized = false;

	// Call client objects to clear as well
	cls.dllfuncs.pfnGLClear();

	// Clear query objects
	R_ClearQueryObjects();
}

//====================================
//
//====================================
bool R_LoadResources( void )
{
	if(cls.cl_state != CLIENT_ACTIVE)
		return true;

	VID_DrawLoadingScreen("Loading decals");

	// Init all renderer classes
	if(!gDecals.InitGame())
		return false;

	if(!gSkyRenderer.InitGame())
		return false;

	if(!gBSPRenderer.InitGame())
		return false;

	if(!gDynamicLights.InitGame())
		return false;

	if(!gWaterShader.InitGame())
		return false;

	if(!gMonitorManager.InitGame())
		return false;

	if(!gMirrorManager.InitGame())
		return false;

	VID_DrawLoadingScreen("Initializing particles");

	if(!gParticleEngine.InitGame())
		return false;

	if(!gSpriteRenderer.InitGame())
		return false;

	VID_DrawLoadingScreen("Loading cubemaps");

	if(!gCubemaps.InitGame())
		return false;

	if(!gPostProcess.InitGame())
		return false;

	if(!gCableRenderer.InitGame())
		return false;

	if(!gVBMRenderer.InitGame())
		return false;

	if(!gLegacyParticles.InitGame())
		return false;

	if(!gBeamRenderer.InitGame())
		return false;

	if(!gPortalManager.InitGame())
		return false;

	if(!gLensFlareRenderer.InitGame())
		return false;

	VID_DrawLoadingScreen("Renderer ready");

	return true;
}

//====================================
//
//====================================
bool R_InitGame( void )
{
	// Mark begin time
	Double initbegintime = Sys_FloatTime();

	// Load renderer resources
	if(!R_LoadResources())
	{
		Con_EPrintf("%s - Error at renderer setup.\n", __FUNCTION__);
		return false;
	}

	// Allocate vis buffer
	if(!rns.pvisbuffer)
	{
		rns.pvisbuffer = new byte[ens.visbuffersize];
		memset(rns.pvisbuffer, 0, sizeof(byte)*ens.visbuffersize);
	}

	// Allocate secondary vis buffer
	if(!rns.psecondaryvisbuffer)
	{
		rns.psecondaryvisbuffer = new byte[ens.visbuffersize];
		memset(rns.psecondaryvisbuffer, 0, sizeof(byte)*ens.visbuffersize);
	}

	// Set this as true
	rns.isgameready = true;

	// Print load time
	Double loadtime = Sys_FloatTime() - initbegintime;
	Con_DPrintf("Renderer setup time: %.2f seconds.\n", loadtime);

	// Reset this
	rns.nextfreerenderpassidx = MAINFRAME_RENDERPASS_ID + 1;

	return true;
}

//====================================
//
//====================================
void R_ResetGame( void )
{
	// Reset these
	rns.time = 0;
	rns.framecount = 0;
	rns.visframe = 0;

	rns.framecount = 0;
	rns.framecount_main = 0;
	rns.visframe = 0;
	rns.view.pviewleaf = nullptr;

	// Reset these
	R_ClearFog();

	rns.sky.drawsky = false;
	rns.sky.skybox = false;
	rns.isgameready = false;
	rns.daystage = DAYSTAGE_NORMAL;
	rns.numskipframes = 0;

	g_beamStartPosition.Clear();
	g_pBeamStartEntity = nullptr;
	g_beamEndPosition.Clear();
	g_pBeamEndEntity = nullptr;
	g_pMaterialShown = nullptr;

	if(!g_defaultMaterialPMFList.empty())
		g_defaultMaterialPMFList.clear();

	rns.sky.skysize = 0;
	rns.sky.local_origin = ZERO_VECTOR;
	rns.sky.world_origin = ZERO_VECTOR;



	// Clear this in particular
	g_pRotLightSprite = nullptr;

	// Clear all renderer classes
	gSkyRenderer.ClearGame();
	gBSPRenderer.ClearGame();
	gDynamicLights.ClearGame();
	gRTTCache.Clear(RS_GAME_LEVEL);
	gDecals.ClearGame();
	gWaterShader.ClearGame();
	gMonitorManager.ClearGame();
	gMirrorManager.ClearGame();
	gParticleEngine.ClearGame();
	gSpriteRenderer.ClearGame();
	gCubemaps.ClearGame();
	gPostProcess.ClearGame();
	gCableRenderer.ClearGame();
	gVBMRenderer.ClearGame();
	gLegacyParticles.ClearGame();
	gBeamRenderer.ClearGame();
	gPortalManager.ClearGame();
	gBlackHoleRenderer.ClearGame();
	gLensFlareRenderer.ClearGame();

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Tell texloader to release all game-level resources
	pTextureManager->DeleteTextures(RS_GAME_LEVEL, false);
	pTextureManager->DeleteMaterials(RS_GAME_LEVEL);
}

//=============================================
// @brief
//
//=============================================
void R_PopulateExtensionsArray( void )
{
	if(!rns.glextensions.empty())
		return;

	// Keep a max of 6 extensions per line
	CString line;
	Uint32 numonline = 0;

	if(ens.pgllogfile)
		ens.pgllogfile->Printf("OpenGL Extensions:\n");

	if(gGLExtF.glGetStringi)
	{
		GLint numExtensions;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

		// Fill extensions array
		rns.glextensions.reserve(numExtensions);
		for(Int32 i = 0; i < numExtensions; i++)
		{
			const Char* pstrExtension = reinterpret_cast<const Char*>(gGLExtF.glGetStringi(GL_EXTENSIONS, i));
			rns.glextensions.push_back(pstrExtension);

			// If we have a gl log file, print all extensions
			if(ens.pgllogfile)
			{
				line << pstrExtension;
				numonline++;

				if(i < (numExtensions-1))
					line << ", ";

				// Print the line if we are over the limit
				if(numonline >= MAX_EXTENSIONS_PER_LINE)
				{
					ens.pgllogfile->Printf("%s%s", line.c_str(), NEWLINE);
					numonline = 0;
					line.clear();
				}
			}
		}
	}
	else
	{
		const Char* pstrExtensions = reinterpret_cast<const Char*>(glGetString(GL_EXTENSIONS));
		const Char* pstr = pstrExtensions;

		// Reserve enough so we don't waste time resizing each time
		rns.glextensions.reserve(8192);

		char szToken[64];
		while(pstr)
		{
			pstr = Common::Parse(pstr, szToken);
			rns.glextensions.push_back(szToken);

			// If we have a gl log file, print all extensions
			if(ens.pgllogfile)
			{
				line << szToken;
				numonline++;

				line << ", ";

				// Print the line if we are over the limit
				if(numonline >= MAX_EXTENSIONS_PER_LINE)
				{
					ens.pgllogfile->Printf("%s%s", line.c_str(), NEWLINE);
					numonline = 0;
					line.clear();
				}
			}
		}

		rns.glextensions.resize(rns.glextensions.size());
	}

	// Make sure last ones are printed too
	if(ens.pgllogfile && !line.empty())
		ens.pgllogfile->Printf("%s%s", line.c_str(), NEWLINE);
}

//=============================================
// @brief
//
//=============================================
bool R_IsExtensionSupported( const Char *pstrextension )
{
	for(Uint32 i = 0; i < rns.glextensions.size(); i++)
	{
		if(!qstrcmp(rns.glextensions[i], pstrextension))
			return true;
	}

	return false;
}

//====================================
//
//====================================
void R_Bind2DTexture( Int32 texture, Uint32 id, bool force )
{
	Int32 idx = texture - GL_TEXTURE0;
	assert(idx < MAX_BOUND_TEXTURES);

	if (rns.textures.texturebinds_2d[idx] != id || force)
	{
		gGLExtF.glActiveTexture( texture );
		glBindTexture( GL_TEXTURE_2D, id );

		rns.textures.texturebinds_2d[idx] = id;
	}
}

//====================================
//
//====================================
void R_BindCubemapTexture( Int32 texture, Uint32 id, bool force )
{
	Int32 idx = texture - GL_TEXTURE0;
	assert(idx < MAX_BOUND_TEXTURES);

	if (rns.textures.texturebinds_cube[idx] != id || force)
	{
		gGLExtF.glActiveTexture( texture );
		glBindTexture( GL_TEXTURE_CUBE_MAP, id );

		rns.textures.texturebinds_cube[idx] = id;
	}
}

//====================================
//
//====================================
void R_BindRectangleTexture( Int32 texture, Uint32 id, bool force )
{
	Int32 idx = texture - GL_TEXTURE0;
	assert(idx < MAX_BOUND_TEXTURES);

	if (rns.textures.texturebinds_rect[idx] != id || force)
	{
		gGLExtF.glActiveTexture( texture );
		glBindTexture( GL_TEXTURE_RECTANGLE, id );

		rns.textures.texturebinds_rect[idx] = id;
	}
}

//====================================
//
//====================================
void R_ClearBinds( void )
{
	for(Uint32 i = 0; i < MAX_BOUND_TEXTURES; i++)
	{
		if(rns.textures.texturebinds_2d[i] != 0)
		{
			gGLExtF.glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, 0 );
		}

		rns.textures.texturebinds_2d[i] = 0;
	}

	for(Uint32 i = 0; i < MAX_BOUND_TEXTURES; i++)
	{
		if(rns.textures.texturebinds_cube[i] != 0)
		{
			gGLExtF.glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
		}

		rns.textures.texturebinds_cube[i] = 0;
	}

	for(Uint32 i = 0; i < MAX_BOUND_TEXTURES; i++)
	{
		if(rns.textures.texturebinds_rect[i] != 0)
		{
			gGLExtF.glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_RECTANGLE, 0 );
		}

		rns.textures.texturebinds_rect[i] = 0;
	}
}

//====================================
//
//====================================
void R_SetupRefDef( cl_entity_t* pclient, ref_params_t& params )
{
	params.time = rns.time;
	params.frametime = rns.frametime;

	// Always reset this
	params.nodraw = false;
	// Always enabled for now
	params.smoothsteps = true;

	// Copy over values from the client
	params.pl_origin = pclient->curstate.origin;
	params.pl_viewangles = pclient->curstate.viewangles;
	params.pl_punchangle = pclient->curstate.punchangles;
	params.pl_viewoffset = pclient->curstate.view_offset;
	params.pl_velocity = pclient->curstate.velocity;
	params.waterlevel = pclient->curstate.waterlevel;
	params.idealpitch = pclient->curstate.idealpitch;
	params.onground = pclient->curstate.groundent;
	params.v_angles = params.pl_viewangles;
	params.paused = cls.paused;
	params.pmovevars = &cls.pminfo.movevars;
	params.screenwidth = gWindow.GetWidth();
	params.screenheight = gWindow.GetHeight();

	// Get last usercmd
	params.pcmd = &cls.cmd;

	// Call client to set the view
	cls.dllfuncs.pfnCalcRefDef(params);

	// Set view info
	rns.view.v_origin = params.v_origin;
	rns.view.v_angles = params.v_angles;
}

//====================================
//
//====================================
void R_SetupView( const ref_params_t& params )
{
	// Reset counters
	rns.counters.brushpolies = 0;
	rns.counters.modelpolies = 0;
	rns.counters.particles = 0;
	rns.counters.batches = 0;
	rns.renderflags = params.renderflags;

	rns.framecount++;

	// Copy over to view data
	rns.view.v_angles = params.v_angles;
	rns.view.v_origin = params.v_origin;

	Math::AngleVectors(rns.view.v_angles, &rns.view.v_forward, &rns.view.v_right, &rns.view.v_up);

	// Set frustum
	if(!rns.monitorpass)
		rns.view.fov = R_GetRenderFOV(params.viewsize);
	else
		rns.view.fov = params.viewsize;

	rns.view.nearclip = NEAR_CLIP_DISTANCE;
	rns.view.viewsize_x = params.screenwidth;
	rns.view.viewsize_y = params.screenheight;

	// Set the modelview matrix
	R_SetModelViewMatrix(rns.view.v_origin, rns.view.v_angles);

	// Set the projection matrix
	if(!rns.mirroring && !rns.cubemapdraw)
		R_SetProjectionMatrix(rns.view.nearclip, rns.view.fov);

	if(rns.fog.settings.active)
		glClearColor(rns.fog.settings.color.x, rns.fog.settings.color.y, rns.fog.settings.color.z, GL_ONE);
	else
		glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Tell client our view information
	cls.dllfuncs.pfnSetupView(params);

	// Mark visible leaves
	if(!rns.sky.skybox || rns.fog.settings.affectsky || rns.water_skydraw)
	{
		Vector vieworigin;
		if(rns.usevisorigin)
			vieworigin = rns.view.v_visorigin;
		else
			vieworigin = rns.view.v_origin;

		R_MarkLeaves(vieworigin);
	}

	// Sort entities by distance
	qsort(&rns.objects.pvisents[0], rns.objects.numvisents, sizeof(cl_entity_t*), R_SortEntities);
}

//====================================
//
//====================================
void R_SetProjectionMatrix( Float znear, Float fovY )
{
	// Set in renderer info structure
	rns.view.znear = znear;

	if(rns.fog.settings.active && rns.fog.settings.affectsky && g_pCvarWireFrame->GetValue() <= 0)
		rns.view.zfar = rns.fog.settings.end;
	else
		rns.view.zfar = g_pCvarFarZ->GetValue();

	Float ratio = (Float)rns.screenwidth/(Float)rns.screenheight;
	Float fovX = GetXFOVFromY(fovY, ratio * 0.75f);

	Float width = 2*rns.view.znear*SDL_tan(fovX*M_PI/360.0f);
	Float height = width/ratio;

	static Float matrix[16];
	matrix[0] = 2*rns.view.znear/width;
	matrix[1] = matrix[2] = matrix[3] = matrix[4] = 0.0;
	matrix[5] = 2*rns.view.znear/height;
	matrix[6] = matrix[7] = matrix[8] = matrix[9] = 0.0;
	matrix[10] = rns.view.zfar/(rns.view.znear-rns.view.zfar);
	matrix[11] = -1;
	matrix[12] = matrix[13] = 0.0;
	matrix[14] = rns.view.znear*rns.view.zfar/(rns.view.znear-rns.view.zfar);
	matrix[15] = 0.0;

	rns.view.projection.SetMatrix(matrix);
}

//====================================
//
//====================================
void R_SetModelViewMatrix( const Vector& origin, const Vector& angles )
{
	// Reset the matrix
	rns.view.modelview.LoadIdentity();

	// Put Z going up
	rns.view.modelview.Rotate(-90, 1, 0, 0);
	rns.view.modelview.Rotate(90, 0, 0, 1);

	rns.view.modelview.Rotate(-angles[2], 1, 0, 0);
	rns.view.modelview.Rotate(-angles[0], 0, 1, 0);
	rns.view.modelview.Rotate(-angles[1], 0, 0, 1);

	rns.view.modelview.Translate(-origin[0], -origin[1], -origin[2]);
}

//====================================
//
//====================================
void R_MarkLeaves( const Vector& origin )
{
	// Get the current view leaf
	brushmodel_t* pworld = ens.pworld;

	rns.view.pviewleaf = Mod_PointInLeaf(origin, *pworld);
	rns.visframe++;

	byte* pvsdata = rns.pvisbuffer;
	Mod_LeafPVS(pvsdata, ens.visbuffersize, (*rns.view.pviewleaf), (*ens.pworld));

	// Mark any visible leaves
	for(Uint32 i = 0; i < ens.pworld->numleafs; i++)
	{
		if(pvsdata[i>>3] & (1<<(i&7)))
		{
			mnode_t* pnode = (mnode_t*)&ens.pworld->pleafs[i+1];
			do
			{
				if(pnode->visframe == rns.visframe)
					break;

				pnode->visframe = rns.visframe;
				pnode = pnode->pparent;
			} 
			while(pnode);
		}
	}
}

//====================================
//
//====================================
void R_ForceMarkLeaves( const mleaf_t* pleaf, Int32 visframe, byte *pvsdata )
{
	byte* _pvsdata = pvsdata;
	if(!_pvsdata)
	{
		_pvsdata = rns.psecondaryvisbuffer;
		Mod_LeafPVS(_pvsdata, ens.visbuffersize, (*pleaf), (*ens.pworld));
	}

	// Mark any visible leaves
	for(Uint32 i = 0; i < ens.pworld->numleafs-1; i++)
	{
		if(_pvsdata[i>>3] & (1<<(i&7)))
		{
			mnode_t* pnode = (mnode_t*)&ens.pworld->pleafs[i+1];
			do
			{
				pnode->visframe = visframe;
				pnode = pnode->pparent;
			} 
			while(pnode);
		}
	}
}

//====================================
//
//====================================
void R_Ent_ModelLight( cl_entity_t *pentity )
{
	if(rns.objects.nummodellights == MAX_MODEL_LIGHTS)
	{
		Con_Printf("Exceeded MAX_MODEL_LIGHTS.\n");
		return;
	}

	mlight_t *mlight = &rns.objects.modellights[rns.objects.nummodellights];
	rns.objects.nummodellights++;

	mlight->entindex = pentity->entindex;
	mlight->color.x	= (Float)pentity->curstate.rendercolor.x/255;
	mlight->color.y	= (Float)pentity->curstate.rendercolor.y/255;
	mlight->color.z	= (Float)pentity->curstate.rendercolor.z/255;
	mlight->radius = pentity->curstate.renderamt*ENV_ELIGHT_RADIUS_MULTIPLIER*g_pCvarModelLightMultiplier->GetValue();
	mlight->noblend = pentity->curstate.velocity.IsZero() ? false : true;

	Math::VectorCopy(pentity->curstate.origin, mlight->origin);
	for(Uint32 i = 0; i < 3; i++)
	{
		mlight->mins[i] = mlight->origin[i] - mlight->radius;
		mlight->maxs[i] = mlight->origin[i] + mlight->radius;
	}
}

//====================================
//
//====================================
void R_Ent_DynamicLight( cl_entity_t *pentity )
{
	bool isstatic = pentity->curstate.skin ? true : false;
	bool noshadow = pentity->curstate.sequence > 0 ? true : false;
	cl_dlight_t *pdlight = gDynamicLights.AllocDynamicPointLight(pentity->entindex, 0, isstatic, noshadow, pentity);

	pdlight->origin = pentity->curstate.origin;
	pdlight->color.x = pentity->curstate.rendercolor.x/255.0f;
	pdlight->color.y = pentity->curstate.rendercolor.y/255.0f;
	pdlight->color.z = pentity->curstate.rendercolor.z/255.0f;
	pdlight->radius = pentity->curstate.renderamt;
	pdlight->lightstyle = (Uint32)pentity->curstate.frame;
	pdlight->lastframe = rns.framecount_main;
	pdlight->die = -1;

	// Do not cull against main view if in portal/sky
	if(pentity->curstate.renderfx == RenderFx_InPortalEntity
		|| pentity->curstate.renderfx == RenderFx_SkyEnt)
		pdlight->setDontCull();
}

//====================================
//
//====================================
void R_Ent_Spotlight( cl_entity_t *pentity )
{
	if(pentity->curstate.body >= rns.objects.projective_textures.size())
	{
		Con_Printf("Texture index greater than the amount of flashlight textures loaded!\n");
		return;
	}

	bool isstatic = pentity->curstate.skin ? true : false;
	bool noshadow = pentity->curstate.sequence > 0 ? true : false;
	cl_dlight_t* pdlight = gDynamicLights.AllocDynamicSpotlight(pentity->entindex, 0, isstatic, noshadow, pentity);

	pdlight->angles = pentity->curstate.angles;
	pdlight->origin = pentity->curstate.origin;
	pdlight->color.x = pentity->curstate.rendercolor.x/255.0f;
	pdlight->color.y = pentity->curstate.rendercolor.y/255.0f;
	pdlight->color.z = pentity->curstate.rendercolor.z/255.0f;
	pdlight->radius = pentity->curstate.renderamt;
	pdlight->cone_size = pentity->curstate.scale;
	pdlight->textureindex = pentity->curstate.body;
	pdlight->lightstyle = (Uint32)pentity->curstate.frame;
	pdlight->lastframe = rns.framecount_main;
	pdlight->die = -1;

	// Do not cull against main view if in portal/sky
	if(pentity->curstate.renderfx == RenderFx_InPortalEntity
		|| pentity->curstate.renderfx == RenderFx_SkyEnt)
		pdlight->setDontCull();
}

//====================================
//
//====================================
void R_Ent_RotLight( cl_entity_t *pentity )
{
	gVBMRenderer.UpdateAttachments(pentity);

	Float radius = 600;
	Float cone_size = 120;

	bool noshadow = false;
	if(pentity->curstate.renderfx == RenderFx_RotlightNS)
		noshadow = true;

	Vector vforward, vleft, vback, vright;
	Math::VectorSubtract(pentity->getAttachment(1), pentity->getAttachment(0), vforward);
	vforward.Normalize();

	Math::VectorSubtract(pentity->getAttachment(2), pentity->getAttachment(0), vleft);
	vleft.Normalize();

	Math::VectorCopy(vforward, vback);
	Math::VectorScale(vback, -1, vback);

	Math::VectorCopy(vleft, vright);
	Math::VectorScale(vright, -1, vright);

	Vector angles = Math::VectorToAngles(vforward, vleft);

	// Allocate the lights
	cl_dlight_t* dlight1 = gDynamicLights.AllocDynamicSpotlight(pentity->entindex, 0, false, noshadow, pentity);
	dlight1->angles = angles;
	dlight1->radius = radius;
	dlight1->cone_size = cone_size;
	dlight1->origin = pentity->getAttachment(0);
	dlight1->textureindex = 0;
	dlight1->die = -1;
	dlight1->lastframe = rns.framecount_main;

	if(!pentity->curstate.rendercolor.x && !pentity->curstate.rendercolor.y && !pentity->curstate.rendercolor.z)
	{
		dlight1->color.x = 1.0;
		dlight1->color.y = 0.0;
		dlight1->color.z = 0.0;
	}
	else
	{
		dlight1->color.x = (Float)pentity->curstate.rendercolor.x/255.0f;
		dlight1->color.y = (Float)pentity->curstate.rendercolor.y/255.0f;
		dlight1->color.z = (Float)pentity->curstate.rendercolor.z/255.0f;
	}

	angles = Math::VectorToAngles(vback, vright);

	cl_dlight_t* dlight2 = gDynamicLights.AllocDynamicSpotlight(-((Int32)pentity->entindex), 0, false, noshadow, pentity);
	dlight2->angles = angles;
	dlight2->radius = radius;
	dlight2->cone_size = cone_size;
	dlight2->origin = pentity->getAttachment(0);
	dlight2->textureindex = 0;
	dlight2->die = -1;
	dlight2->lastframe = rns.framecount_main;

	if(!pentity->curstate.rendercolor.x && !pentity->curstate.rendercolor.y && !pentity->curstate.rendercolor.z)
	{
		dlight2->color.x = 1.0;
		dlight2->color.y = 0.0;
		dlight2->color.z = 0.0;
	}
	else
	{
		dlight2->color.x = (Float)pentity->curstate.rendercolor.x/255.0f;
		dlight2->color.y = (Float)pentity->curstate.rendercolor.y/255.0f;
		dlight2->color.z = (Float)pentity->curstate.rendercolor.z/255.0f;
	}

	// Allocate the sprites
	if( !g_pRotLightSprite )
		g_pRotLightSprite = gModelCache.LoadModel( ROTATING_LIGHT_SPRITE_PATH );

	if( g_pRotLightSprite )
	{
		Math::VectorSubtract(pentity->getAttachment(1), pentity->getAttachment(0), vforward);

		cl_entity_t* psprite1 = gSpriteRenderer.AllocTempSprite( pentity->entindex, 0.01 );

		Math::VectorCopy( dlight1->angles, psprite1->curstate.angles );
		Math::VectorAdd( pentity->getAttachment(0), vforward, psprite1->curstate.origin );
	
		psprite1->pmodel = g_pRotLightSprite;
		psprite1->curstate.renderfx = RenderFx_AngularSprite;
		psprite1->curstate.rendercolor.x = dlight2->color.x * 255;
		psprite1->curstate.rendercolor.y = dlight2->color.y * 255;
		psprite1->curstate.rendercolor.z = dlight2->color.z * 255;
		psprite1->curstate.rendermode = RENDER_TRANSGLOW;
		psprite1->curstate.renderamt = 155;
		psprite1->curstate.scale = 0.05;
		psprite1->curstate.fuser1 = dlight2->cone_size;

		cl_entity_t* psprite2 = gSpriteRenderer.AllocTempSprite( -((Int32)pentity->entindex), 0.01 );

		Math::VectorCopy( dlight2->angles, psprite2->curstate.angles );
		Math::VectorScale(vforward, -1, vforward);
		Math::VectorAdd( pentity->getAttachment(0), vforward, psprite2->curstate.origin );

		psprite2->pmodel = g_pRotLightSprite;
		psprite2->curstate.renderfx = RenderFx_AngularSprite;
		psprite2->curstate.rendercolor.x = dlight2->color.x * 255;
		psprite2->curstate.rendercolor.y = dlight2->color.y * 255;
		psprite2->curstate.rendercolor.z = dlight2->color.z * 255;
		psprite2->curstate.rendermode = RENDER_TRANSGLOW;
		psprite2->curstate.renderamt = 155;
		psprite2->curstate.scale = 0.05;
		psprite2->curstate.fuser1 = dlight2->cone_size;

		// Allocate the elights
		mlight_t* pmlight1 = &rns.objects.modellights[rns.objects.nummodellights];
		rns.objects.nummodellights++;

		pmlight1->entindex = pentity->entindex;
		Math::VectorCopy( psprite1->curstate.origin, pmlight1->origin );
		Math::VectorCopy( dlight1->color, pmlight1->color );
		pmlight1->radius = 128;

		for(Uint32 i = 0; i < 3; i++)
		{
			pmlight1->mins[i] = pmlight1->origin[i] - pmlight1->radius;
			pmlight1->maxs[i] = pmlight1->origin[i] + pmlight1->radius;
		}

		mlight_t* pmlight2 = &rns.objects.modellights[rns.objects.nummodellights];
		rns.objects.nummodellights++;

		pmlight2->entindex = -((Int32)pentity->entindex);
		Math::VectorCopy( psprite2->curstate.origin, pmlight2->origin );
		Math::VectorCopy( dlight2->color, pmlight2->color );
		pmlight2->radius = pmlight1->radius;

		for(Uint32 i = 0; i < 3; i++)
		{
			pmlight2->mins[i] = pmlight2->origin[i] - pmlight2->radius;
			pmlight2->maxs[i] = pmlight2->origin[i] + pmlight2->radius;
		}
	}
}

//====================================
//
//====================================
void R_Ent_SkyMarker( cl_entity_t *pentity )
{
	rns.sky.local_origin.x = pentity->curstate.origin.x;
	rns.sky.local_origin.y = pentity->curstate.origin.y;
	rns.sky.local_origin.z = pentity->curstate.origin.z;
}

//====================================
//
//====================================
void R_Ent_Mirror( cl_entity_t *pentity )
{
	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	if(pinfo->pmirrordata)
		return;

	gMirrorManager.AllocNewMirror(pentity);
}

//====================================
//
//====================================
void R_Ent_Monitor( cl_entity_t *pentity )
{
	if(pentity->curstate.aiment != NO_ENTITY_INDEX)
	{
		cl_entity_t* pcamerantity = CL_GetEntityByIndex(pentity->curstate.aiment);
		if(pcamerantity)
		{
			cl_entity_t* pplayer = CL_GetLocalPlayer();
			if(pcamerantity->curstate.msg_num == pplayer->curstate.msg_num)
			{
				entity_extrainfo_t* pcamerainfo = CL_GetEntityExtraData(pcamerantity);
				if(!pcamerainfo->ppvsdata)
				{
					Uint32 bufsize = ens.visbuffersize;
					pcamerainfo->ppvsdata = new byte[bufsize];

					const mleaf_t* pleaf = Mod_PointInLeaf(pcamerantity->curstate.origin, (*ens.pworld));
					Mod_LeafPVS(pcamerainfo->ppvsdata, bufsize, (*pleaf), (*ens.pworld));
				}
			}
		}
	}

	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	if(pinfo->pmonitordata)
		return;

	gMonitorManager.AllocNewMonitor(pentity);
}

//====================================
//
//====================================
void R_Ent_Portal( cl_entity_t *pentity )
{
	if(pentity->curstate.aiment != NO_ENTITY_INDEX)
	{
		cl_entity_t* pportalentity = CL_GetEntityByIndex(pentity->curstate.aiment);
		if(pportalentity)
		{
			cl_entity_t* pplayer = CL_GetLocalPlayer();
			if(pportalentity->curstate.msg_num == pplayer->curstate.msg_num)
			{
				entity_extrainfo_t* pportalinfo = CL_GetEntityExtraData(pportalentity);
				if(!pportalinfo->ppvsdata)
				{
					Uint32 bufsize = ens.visbuffersize;
					pportalinfo->ppvsdata = new byte[bufsize];

					const mleaf_t* pleaf = Mod_PointInLeaf(pportalentity->curstate.origin, (*ens.pworld));
					Mod_LeafPVS(pportalinfo->ppvsdata, bufsize, (*pleaf), (*ens.pworld));
				}
			}
		}
	}

	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	if(pinfo->pportaldata)
		return;

	gPortalManager.AllocNewPortal(pentity);
}

//====================================
//
//====================================
void R_Ent_Water( cl_entity_t *pentity )
{
	entity_extrainfo_t* pinfo = CL_GetEntityExtraData(pentity);
	if(pinfo->pwaterdata)
		return;

	gWaterShader.AddEntity(pentity);
}

//====================================
//
//====================================
void R_Ent_Beam( cl_entity_t *pentity )
{
	gBeamRenderer.AddBeamEntity(pentity);
}

//====================================
//
//====================================
bool R_IsNonRenderedRenderEntity( const cl_entity_t& entity )
{
	switch(entity.curstate.rendertype)
	{
	case RT_ENVELIGHT:
	case RT_ENVDLIGHT:
	case RT_ENVSPOTLIGHT:
	case RT_ENVSKYENT:
	case RT_MONITORCAMERA:
	case RT_ENVPOSPORTAL:
	case RT_ENVPOSPORTALWORLD:
	case RT_BEAM:
		return true;
		break;
	default:
		return false;
		break;
	}
}

//====================================
//
//====================================
bool R_IsSpecialRenderEntity( const cl_entity_t& entity )
{
	switch(entity.curstate.rendertype)
	{
	case RT_WATERSHADER:
	case RT_SKYWATERENT:
	case RT_MIRROR:
	case RT_MONITORENTITY:
	case RT_PORTALSURFACE:
	case RT_BEAM:
	case RT_LENSFLARE:
		return true;
		break;
	default:
		return false;
		break;
	}
}

//====================================
//
//====================================
bool R_AddSpecialEntity( cl_entity_t *pentity )
{
	// Manage rotating lights
	if(pentity->curstate.renderfx == RenderFx_Rotlight 
		|| pentity->curstate.renderfx == RenderFx_RotlightNS)
		R_Ent_RotLight(pentity);

	switch(pentity->curstate.rendertype)
	{
	case RT_ENVELIGHT:
		{
			R_Ent_ModelLight(pentity);
			return false;
		}
		break;
	case RT_ENVDLIGHT:
		{
			R_Ent_DynamicLight(pentity);
			return false;
		}
		break;
	case RT_ENVSPOTLIGHT:
		{
			R_Ent_Spotlight(pentity);
			return false;
		}
		break;
	case RT_ENVSKYENT:
		{
			R_Ent_SkyMarker(pentity);
			return false;
		}
		break;
	case RT_MIRROR:
		{
			R_Ent_Mirror(pentity);
			return true;
		}
		break;
	case RT_MONITORENTITY:
		{
			R_Ent_Monitor(pentity);
			return true;
		}
		break;
	case RT_PORTALSURFACE:
		{
			R_Ent_Portal(pentity);
			return true;
		}
		break;
	case RT_WATERSHADER:
	case RT_SKYWATERENT:
		{
			R_Ent_Water(pentity);
			return true;
		}
		break;
	case RT_BEAM:
		{
			R_Ent_Beam(pentity);
			return true;
		}
		break;
	default:
		{
			return true;
		}
		break;
	}
}

//====================================
//
//====================================
void R_AddEntity( cl_entity_t* pentity )
{
	if(rns.objects.numvisents >= MAX_RENDER_ENTITIES)
	{
		Con_Printf("%s - Exceeded MAX_RENDER_ENTITIES.\n", __FUNCTION__);
		return;
	}

	if(!R_AddSpecialEntity(pentity))
		return;

	if(R_IsNonRenderedRenderEntity(*pentity))
		return;

	rns.objects.pvisents[rns.objects.numvisents] = pentity;
	rns.objects.numvisents++;
}

//====================================
//
//====================================
void R_AddEntityLights( void )
{
	// Add entity lights
	for(Uint32 i = 0; i < MAX_ENTITY_LIGHTS; i++)
	{
		const entitylight_t& el = cls.entitylights[i];
		if(el.die < cls.cl_time)
			continue;

		mlight_t& ml = rns.objects.modellights[rns.objects.nummodellights];
		rns.objects.nummodellights++;

		// Assign values
		ml = el.mlight;
		ml.radius *= g_pCvarModelLightMultiplier->GetValue();

		// Set mins/maxs
		for(Uint32 j = 0; j < 3; j++)
		{
			ml.mins[j] = ml.origin[j] - ml.radius;
			ml.maxs[j] = ml.origin[j] + ml.radius;
		}
	}
}

//====================================
//
//====================================
void R_AddEntities( void )
{
	// Get the local player
	cl_entity_t *pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return;

	for(Uint32 i = 1; i < MAX_RENDER_ENTITIES; i++)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(i);
		if(!pentity)
			break;

		if(!pentity->pmodel)
			continue;

		if(pentity->curstate.msg_num != pplayer->curstate.msg_num)
			continue;

		if(pentity->curstate.effects & EF_NODRAW)
			continue;

		if(pentity->curstate.effects & EF_COLLISION)
			continue;

		if((pentity->curstate.flags & FL_PARENTED) && pentity->curstate.parent == NO_ENTITY_INDEX)
		{
			cl_entity_t* pparent = CL_GetEntityByIndex(pentity->curstate.parent);
			if(!pparent || pparent->curstate.msg_num != pplayer->curstate.msg_num)
				continue;

			continue;
		}

		R_AddEntity(pentity);
	}

	// Allow the client to add any rendered entities
	cls.dllfuncs.pfnAddEntities();

	// Add any entity lights
	R_AddEntityLights();
}

//====================================
//
//====================================
bool R_DrawLogo( en_texture_t* ptexture, Int32 basewidth, Int32 baseheight )
{
	assert(rns.objects.ploadinglogo != nullptr);

	// Set matrices
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0/(Float)gWindow.GetWidth(), 1.0/(Float)gWindow.GetHeight(), 1.0);

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	// Determine position/size
	Int32 logoWidth = R_GetRelativeX(basewidth, CMenu::MENU_BASE_WIDTH, gWindow.GetWidth());
	Int32 logoHeight = R_GetRelativeX(baseheight, CMenu::MENU_BASE_WIDTH, gWindow.GetWidth());

	Int32 logoOriginX = gWindow.GetCenterX() - logoWidth/2;
	Int32 logoOriginY = gWindow.GetCenterY() - logoHeight/2;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!pDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	
	// Draw the background
	R_Bind2DTexture(GL_TEXTURE0_ARB, ptexture->palloc->gl_index);

	pDraw->Begin(GL_TRIANGLES);
	pDraw->TexCoord2f(0.0, 0.0);
	pDraw->Vertex3f(logoOriginX, logoOriginY, -1.0);

	pDraw->TexCoord2f(1.0, 0.0);
	pDraw->Vertex3f(logoOriginX+logoWidth, logoOriginY, -1.0);

	pDraw->TexCoord2f(0.0, 1.0);
	pDraw->Vertex3f(logoOriginX, logoOriginY+logoHeight, -1.0);

	pDraw->TexCoord2f(0.0, 1.0);
	pDraw->Vertex3f(logoOriginX, logoOriginY+logoHeight, -1.0);

	pDraw->TexCoord2f(1.0, 0.0);
	pDraw->Vertex3f(logoOriginX+logoWidth, logoOriginY, -1.0);

	pDraw->TexCoord2f(1.0, 1.0);
	pDraw->Vertex3f(logoOriginX+logoWidth, logoOriginY+logoHeight, -1.0);
	pDraw->End();

	glDisable(GL_BLEND);
	return true;
}

//====================================
//
//====================================
bool R_DrawLoadingBackground( void )
{
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	assert(pDraw->IsActive());

	// Watch out for errors
	if(!rns.ploadbackground)
	{
		Con_Printf("%s - No background texture set.\n", __FUNCTION__);
		return true;
	}

	// Set matrices
	rns.view.modelview.LoadIdentity();

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());
	if(!pDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}
	
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	pDraw->Color4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
	
	// Draw the background
	Float tcymod, tcscalex, tcscaley;
	if(!rns.isbgrectangletexture)
	{
		tcymod = 0.0;
		tcscalex = 1.0;
		tcscaley = 1.0;

		// Just draw through regular rendering
		R_Bind2DTexture(GL_TEXTURE0_ARB, rns.ploadbackground->gl_index);
	}
	else
	{
		tcymod = 1.0;
		tcscalex = rns.screenwidth;
		tcscaley = rns.screenheight;

		// Render as rectangle texture
		if(!pDraw->EnableRectangleTexture())
		{
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			return false;
		}

		gGLExtF.glActiveTexture( GL_TEXTURE0_ARB );
		glBindTexture( GL_TEXTURE_RECTANGLE, rns.ploadbackground->gl_index );
	}

	pDraw->Begin(GL_TRIANGLES);
	pDraw->TexCoord2f(0.0, tcymod*tcscaley);
	pDraw->Vertex3f(0.0, 0.0, -1.0);

	pDraw->TexCoord2f(1.0*tcscalex, tcymod*tcscaley);
	pDraw->Vertex3f(1.0, 0.0, -1.0);

	pDraw->TexCoord2f(0.0, (1.0-tcymod)*tcscaley);
	pDraw->Vertex3f(0.0, 1.0, -1.0);

	pDraw->TexCoord2f(1.0*tcscalex, tcymod*tcscaley);
	pDraw->Vertex3f(1.0, 0.0, -1.0);

	pDraw->TexCoord2f(0.0, (1.0-tcymod)*tcscaley);
	pDraw->Vertex3f(0.0, 1.0, -1.0);

	pDraw->TexCoord2f(1.0*tcscalex, (1.0-tcymod)*tcscaley);
	pDraw->Vertex3f(1.0, 1.0, -1.0);
	pDraw->End();

	if(!pDraw->DisableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	if(rns.isbgrectangletexture)
	{
		if(!pDraw->DisableRectangleTexture())
		{
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			return false;
		}
	}

	return true;
}

//====================================
//
//====================================
bool R_DrawNormal( void )
{
	if(!rns.view.params.nodraw)
	{
		// Draw the sky first
		if(!gSkyRenderer.DrawSky())
			return false;		
	}

	// Create cached decals
	gDecals.CreateCached();

	if(!rns.view.params.nodraw)
	{
		// Draw the world first
		if(!gBSPRenderer.DrawNormal())
			return false;

		if(!(rns.renderflags & RENDERER_FL_DONT_DRAW_MODELS))
		{
			// Draw VBMs
			if(!gVBMRenderer.DrawNormal())
				return false;

			// Draw any ladders on client
			if(!cls.dllfuncs.pfnDrawLadders())
				return false;
		}

		// Draw mirrors
		if(!gMirrorManager.DrawMirrors())
			return false;

		// Draw monitors
		if(!gMonitorManager.DrawMonitors())
			return false;

		// Draw monitors
		if(!gPortalManager.DrawPortals())
			return false;

		// Draw water
		if(!gWaterShader.DrawWater())
			return false;

		// Draw cables
		if(!gCableRenderer.DrawCables())
			return false;

		// Draw debug info
		if(!gSoundEngine.DrawNormal())
			return false;

		// Draw decals after everything else
		if(!gBSPRenderer.DrawNormalDecals())
			return false;

		// Draw bogus decal positions
		if(!gDecals.DrawBogusDecals())
			return false;
	}

	// Allow client to draw
	if(!cls.dllfuncs.pfnDrawNormal())
		return false;
		
	return true;
}

//====================================
//
//====================================
bool R_DrawViewObjects( void )
{
	// Don't render these if dead
	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(pplayer->curstate.health <= 0)
		return true;

	// Call to draw view objects
	if(!cls.dllfuncs.pfnDrawViewObjects())
		return false;

	return true;
}

//====================================
//
//====================================
bool R_DrawViewModelParticles( void )
{
	// Draw particles for view model
	if(!gParticleEngine.DrawParticles(PARTICLES_VIEWMODEL))
		return false;

	return true;
}

//====================================
//
//====================================
bool R_DrawTransparent( void )
{
	if(!rns.view.params.nodraw)
	{
		if(!(rns.renderflags & RENDERER_FL_DONT_DRAW_MODELS))
		{
			// Draw transparent VBM entities
			if(!gVBMRenderer.DrawTransparent())
				return false;
		}

		// Draw transparent world entities
		if(!gBSPRenderer.DrawTransparent())
			return false;

		if(!rns.cubemapdraw)
		{
			if(!(rns.renderflags & RENDERER_FL_DONT_DRAW_PARTICLES))
			{
				// Render particles
				if(!gParticleEngine.DrawParticles(PARTICLES_NORMAL))
					return false;
			}

			// Render sprites
			if(!gSpriteRenderer.DrawSprites())
				return false;

			// Draw beams
			if(!gBlackHoleRenderer.DrawBlackHoles())
				return false;

			if(rns.mainframe)
			{
				// Draw glow auras
				if(!gGlowAura.DrawAuras())
					return false;

				// Draw view objects
				if(!R_DrawViewObjects())
					return false;
			}
		}

		if(!(rns.renderflags & RENDERER_FL_DONT_DRAW_PARTICLES))
		{
			// Draw legacy particles
			if(!gLegacyParticles.DrawParticles())
				return false;
		}

		// Draw beams
		if(!gBeamRenderer.DrawBeams())
			return false;

		if(rns.mainframe)
		{
			// Draw lens flares
			if(!gLensFlareRenderer.DrawLensFlares())
				return false;
		}
	}

	// Allow client to draw
	if(!cls.dllfuncs.pfnDrawTransparent())
		return false;
		
	return true;
}

//====================================
//
//====================================
bool R_Draw( const ref_params_t& params )
{
	if(!rns.view.params.nodraw)
	{
		// Set view params
		R_SetupView(params);
	}

	if(!rns.view.params.nodraw)
	{
		// Update ideal cubemap
		gCubemaps.Update(params.v_origin);
	}

	// Draw non-transparents
	if(!R_DrawNormal())
		return false;

	// Draw transparents
	if(!R_DrawTransparent())
		return false;
	
	if(!rns.view.params.nodraw)
	{
		// Draw origins if cvar is set
		if(!R_DrawOrigins())
			return false;
	}

	return true;
}

//====================================
//
//====================================
bool R_DrawRenderPasses( void )
{
	// Lets the renderer know we're not
	// drawing the main view
	rns.mainframe = false;
	
	// Draw shadowmaps for dynamic lights
	if(!gDynamicLights.DrawPasses())
		return false;

	// Draw renderpasses for special ents
	if(!gMirrorManager.DrawMirrorPasses())
		return false;

	// Draw render passes for water
	if(!gWaterShader.DrawWaterPasses())
		return false;

	// Draw monitors
	if(!gMonitorManager.DrawMonitorPasses())
		return false;
	
	// Draw portals
	if(!gPortalManager.DrawPortalPasses())
		return false;

	return true;
}

//====================================
//
//====================================
bool R_DrawScene( void )
{
	// Rely on local time for renderer
	if(!rns.time)
		rns.time = cls.cl_time;

	rns.frametime = cls.cl_time - rns.time;
	rns.time = cls.cl_time;

	if(rns.frametime > 1)
		rns.frametime = 1;

	if(rns.frametime < 0)
		rns.frametime = 0;

	// Get the local client
	cl_entity_t* pclient = CL_GetLocalPlayer();
	if(!pclient)
		return true;

	// Set up view params first
	R_SetupRefDef(pclient, rns.view.params);

	// Update renderer after getting setting up main view
	if(!R_Update())
		return false;

	// Draw any renderpasses
	if(!rns.view.params.nodraw)
	{
		if(!R_DrawRenderPasses())
			return false;
	}

	// Set this so engine knows it's
	// the main render frame
	rns.mainframe = true;
	// Set renderpass id for main frame
	rns.renderpassidx = MAINFRAME_RENDERPASS_ID;

	// Draw as usual
	if(!R_Draw(rns.view.params))
		return false;

	// Restore water
	gWaterShader.Restore();

	return true;
}

//====================================
//
//====================================
void R_DrawLineGraph( CBasicDraw* pDraw, Int32 x, Int32 y, Int32 h, Int32 s )
{
	Vector color;
	if(h == 10000)
		color = Vector(255, 192, 64);
	else if(h == 9999)
		color = Vector(255, 0, 0);
	else if(h == 9998)
		color = Vector(0, 0, 255);
	else
		color = Vector(255, 0, 128);

	Math::VectorScale(color, 1.0f/255.0f, color);

	Int32 _h = h;
	if(_h > s)
		_h = s;

	// Set color
	pDraw->Color4fv(color);

	pDraw->Begin(GL_LINES);
	pDraw->Vertex3f(x, y-_h, -1);
	pDraw->Vertex3f(x, y, -1);
	pDraw->End();
}

//====================================
//
//====================================
bool R_DrawTimeGraph( Double& time1, Double& time2 )
{
	static Int32 timex = 0;
	static byte timings[MAX_TIMINGS];

	// All UI elements use the simple draw interface
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	if(!pDraw->DisableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	rns.view.modelview.PushMatrix();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0/(Float)gWindow.GetWidth(), 1.0/(Float)gWindow.GetHeight(), 1.0);

	rns.view.projection.PushMatrix();
	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	// Remember past timing
	Int32 a = (time2 - time1) / 0.001f;
	timings[timex] = a;
	a = timex;

	// Set x coordinate
	Int32 x;
	if(rns.screenwidth < MAX_TIMINGS)
		x = rns.screenwidth - 1;
	else
		x = rns.screenwidth - (rns.screenwidth - MAX_TIMINGS)/2;

	Int32 s = g_pCvarGraphHeight->GetValue();
	Int32 y = rns.screenheight-16;

	// Draw the graph bars
	pDraw->Color4f(1.0, 1.0, 1.0, 1.0);
	pDraw->Begin(GL_LINES);
	pDraw->Vertex3f(x+1, y-s, -1);
	pDraw->Vertex3f(x+1, y, -1);
	pDraw->Vertex3f(x-MAX_TIMINGS-1, y, -1);
	pDraw->Vertex3f(x+1, y, -1);
	pDraw->Vertex3f(x-MAX_TIMINGS, y-s, -1);
	pDraw->Vertex3f(x-MAX_TIMINGS, y, -1);
	pDraw->End();

	do
	{
		R_DrawLineGraph(pDraw, x, rns.screenheight-16, timings[a], s);
		if(x == 0)
			break;

		x--;
		a--;

		if(a == -1)
			a = (MAX_TIMINGS-1);
	}
	while(a != timex);

	// Set timex
	timex = (timex+1) % MAX_TIMINGS;

	rns.view.modelview.PopMatrix();
	rns.view.projection.PopMatrix();

	pDraw->Disable();
	return true;
}

//====================================
//
//====================================
bool R_DrawInterface( void )
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// All UI elements use the simple draw interface
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable())
		return false;

	// Draw the menu
	CMenu::rendercode_t menuResult = gMenu.Draw();
	if(menuResult != CMenu::RC_OK)
	{
		const Char* pstrError = nullptr;
		switch(menuResult)
		{
		case CMenu::RC_BASICDRAW_FAIL:
			pstrError = pDraw->GetShaderError();
			break;
		case CMenu::RC_MENUPARTICLES_FAIL:
			pstrError = gMenuParticles.GetShaderError();
			break;
		case CMenu::RC_TEXT_FAIL:
			pstrError = gText.GetShaderError();
			break;
		}

		if(pstrError)
			Sys_ErrorPopup("Shader error: %s.\n", pstrError);
		else
			Sys_ErrorPopup("Unknown shader error.\n");

		return false;
	}

	// Draw shown texture if set
	if(!R_DrawShownMaterial())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	// Draw the UI
	if(!gUIManager.Draw())
	{
		if(pDraw->HasError())
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		else if(gText.HasError())
			Sys_ErrorPopup("Shader error: %s.\n", gText.GetShaderError());

		return false;
	}

	// Draw the menu fade
	if(!gMenu.DrawMenuFade())
	{
		if(pDraw->HasError())
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		else if(gText.HasError())
			Sys_ErrorPopup("Shader error: %s.\n", gText.GetShaderError());

		return false;
	}

	pDraw->Disable();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	return true;
}

//====================================
//
//====================================
bool R_DrawHUD( bool hudOnly )
{
	// Draw postprocess
	if(!gPostProcess.Draw())
		return false;

	// Call client to draw it's overlay stuff
	if(!cls.dllfuncs.pfnDrawHUD(hudOnly))
		return false;

	return true;
}

//====================================
//
//====================================
bool R_DrawLoadingScreen( const Char* pstrText )
{
	R_ClearBinds();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// All UI elements use the simple draw interface
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw)
		return false;

	if(!pDraw->Enable())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	// Draw the loading screen
	if(!R_DrawLoadingBackground())
		return false;

	// Draw the loading text logo
	if(!R_DrawLogo(rns.objects.ploadinglogo, LOAD_TEXT_BASE_WIDTH, LOAD_TEXT_BASE_HEIGHT))
		return false;

	pDraw->Disable();

	if(pstrText)
	{
		Uint32 textWidth = 0;
		Uint32 textHeight = 0;

		const font_set_t* pfontset = gText.GetDefaultFont();
		gText.GetStringSize(pfontset, pstrText, &textWidth, &textHeight);

		// Set the position off from the logo a bit
		Int32 xpos = gWindow.GetWidth() / 2 - textWidth / 2;
		Uint32 logoHeight = R_GetRelativeX(LOAD_TEXT_BASE_HEIGHT, CMenu::MENU_BASE_WIDTH, gWindow.GetWidth());
		Int32 ypos = gWindow.GetHeight() / 2 + R_GetRelativeX(logoHeight, CMenu::MENU_BASE_WIDTH, gWindow.GetWidth()) / 2.0f + pfontset->fontsize;

		// Draw the string
		if(!R_DrawString(color32_t(255, 255, 255, 255), xpos, ypos, pstrText, pfontset))
		{
			Sys_ErrorPopup("Shader error: %s.", gText.GetShaderError());
			return false;
		}
	}

	if(rns.drawuiwhileloading)
	{
		// Draw the console if active
		if(!pDraw->Enable())
		{
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			return false;
		}

		if(!gUIManager.Draw())
		{
			if(pDraw->HasError())
				Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			else if(gText.HasError())
				Sys_ErrorPopup("Shader error: %s.\n", gText.GetShaderError());

			pDraw->Disable();
			return false;
		}

		pDraw->Disable();
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	return true;
}

//====================================
//
//====================================
bool R_DrawPausedLogo( void )
{
	if(!cls.dllfuncs.pfnShouldDrawPausedLogo())
		return true;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// All UI elements use the simple draw interface
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	// Draw the loading text logo
	if(!R_DrawLogo(rns.objects.ppausedlogo, LOAD_TEXT_BASE_WIDTH, LOAD_TEXT_BASE_HEIGHT))
		return false;

	pDraw->Disable();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	return true;
}

//====================================
//
//====================================
bool R_DrawShownMaterial( void )
{
	if(!g_pMaterialShown)
		return true;

	// Draw the loading text logo
	if(!R_DrawLogo(g_pMaterialShown->ptextures[MT_TX_DIFFUSE], g_pMaterialShown->int_width, g_pMaterialShown->int_height))
		return false;

	return true;
}

//====================================
//
//====================================
void R_SetFrustum( CFrustum& frustum, const Vector& origin, const Vector& angles, Float fov, Float viewsize_x, Float viewsize_y, bool fogCull )
{
	Float flAspect = viewsize_x/viewsize_y;
	Float flFovX;
	if(!rns.cubemapdraw)
		flFovX = GetXFOVFromY( fov, flAspect * 0.75f );
	else 
		flFovX = fov;

	Float flCullEndDist = 0;
	if(fogCull)
		flCullEndDist = rns.fog.settings.affectsky ? rns.fog.settings.end : 0;

	frustum.SetFrustum(angles, origin, flFovX, fov, flCullEndDist);
}

//====================================
//
//====================================
void R_BindFBO( fbobind_t *pfbo )
{
	if(pfbo)
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, pfbo->fboid);
	else
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

//====================================
//
//====================================
inline void R_ValidateShader( CGLSLShader* pShader )
{
	if(!rns.validateshaders)
		return;

	pShader->ValidateProgram(Con_EPrintf);
}

//====================================
//
//====================================
inline void R_ValidateShader( CBasicDraw* pDraw )
{
	if(!rns.validateshaders)
		return;

	pDraw->ValidateShaderSetup(Con_EPrintf);
}

//====================================
//
//====================================
void R_ClearFog( void )
{
	rns.fog.blendtime = 0;
	rns.fog.specialfog = false;
	rns.fog.prevspecialfog = false;

	rns.fog.blend1 = fog_settings_t();
	rns.fog.blend2 = fog_settings_t();
	rns.fog.settings = fog_settings_t();
	rns.sky.fog = fog_settings_t();
}

//====================================
//
//====================================
void R_UpdateFog( void )
{
	// Blend fog
	if(!rns.fog.blendtime)
		return;

	Float fltime = rns.time;

	if(rns.fog.blendtime <= fltime)
	{
		rns.fog.settings = rns.fog.blend2;
		rns.fog.blend1 = fog_settings_t();
		rns.fog.blend2 = fog_settings_t();
		rns.fog.blendtime = 0;
	}
	else
	{
		Float flFrac = rns.fog.blendtime-fltime;
		flFrac = Common::SplineFraction( flFrac, 1.0f/rns.fog.blend2.blend );

		if(!rns.fog.blend1.affectsky || !rns.fog.blend2.affectsky)
			rns.fog.settings.affectsky = false;

		rns.fog.settings.color.x = (rns.fog.blend1.color.x*flFrac)+(rns.fog.blend2.color.x*(1.0-flFrac));
		rns.fog.settings.color.y = (rns.fog.blend1.color.y*flFrac)+(rns.fog.blend2.color.y*(1.0-flFrac));
		rns.fog.settings.color.z = (rns.fog.blend1.color.z*flFrac)+(rns.fog.blend2.color.z*(1.0-flFrac));
		rns.fog.settings.start = (rns.fog.blend1.start*flFrac)+(rns.fog.blend2.start*(1.0-flFrac));
		rns.fog.settings.end = (rns.fog.blend1.end*flFrac)+(rns.fog.blend2.end*(1.0-flFrac));
	}
}

//====================================
//
//====================================
void R_ResetFrameStates( void )
{
	rns.inwater = false;
	rns.mirroring = false;
	rns.mainframe = false;
	rns.water_skydraw = false;
	rns.cubemapdraw = false;
	rns.monitorpass = false;
	rns.portalpass = false;

	rns.objects.numvisents = 0;
	rns.objects.nummodellights = 0;
	rns.objects.numvisents = 0;

	gSkyRenderer.PreFrame();
}

//====================================
//
//====================================
bool R_Update( void )
{
	// Check the fbo cvar
	Float nofbnovalue = g_pCvarNoFBO->GetValue();
	if(nofbnovalue > 0 && !rns.nofbo || nofbnovalue == 0 && rns.nofbo)
	{
		if(!R_IsExtensionSupported("GL_EXT_framebuffer_object")  && !R_IsExtensionSupported("GL_ARB_framebuffer_object") && nofbnovalue < 0)
			Con_Printf("Framebuffer objects are not supported by your GPU.\n");
		else
			Con_Printf("This change will only take effect after reloading the game.\n");

		rns.nofbo = (nofbnovalue > 0) ? true : false;
	}

	// Reset outside of render loop
	R_ResetFrameStates();

	// Raise for dlights
	rns.framecount_main++;

	// Add entities to be rendered
	R_AddEntities();

	// Play any sound events for models
	gVBMRenderer.PlayEvents();

	// Update black holes before updating tempents and particles
	gBlackHoleRenderer.Think();

	// Update BSP Renderer
	gBSPRenderer.Think();

	// Update tempents
	gTempEntities.UpdateTempEntities();
	
	// Keep original list of unsorted visents
	memcpy(rns.objects.pvisents_unsorted, rns.objects.pvisents, sizeof(cl_entity_t*)*rns.objects.numvisents);

	if(cls.paused)
	{
		// Don't update while paused
		return true;
	}

	// Update classes
	R_UpdateFog();

	// Update dynamic lights
	if(!gDynamicLights.Update())
		return false;

	// Update particles
	gParticleEngine.Update();
	
	// Update sprites
	gSpriteRenderer.Animate();

	// Update legacy particles
	gLegacyParticles.UpdateParticles();

	// Update RTT cache
	gRTTCache.Think();

	// Update beams
	gBeamRenderer.Update();

	return true;
}

//====================================
//
//====================================
bool R_DrawString( color32_t color, Int32 x, Int32 y, const Char* pstrString, const font_set_t* pfont )
{
	const font_set_t* pfontset = nullptr;
	if(!pfont)
		pfontset = gText.GetDefaultFont();
	else
		pfontset = pfont;

	if(!gText.Prepare())
		return false;

	if(!gText.BindSet(pfontset))
		return false;

	gText.SetColor(color.r, color.g, color.b, color.a);
	if(!gText.DrawSimpleString(pfontset, pstrString, x, y))
		return false;

	gText.UnBind(pfontset);
	gText.Reset();

	return true;
}

//====================================
//
//====================================
bool R_DrawStringBox( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int16 insetx, Int16 insety, bool reverse, color32_t color, Int32 x, Int32 y, const Char* pstrString, const font_set_t* pfont, Uint32 lineoffset, Uint32 minlineheight, Uint32 xoffset )
{
	const font_set_t* pfontset = nullptr;
	if(!pfont)
		pfontset = gText.GetDefaultFont();
	else
		pfontset = pfont;

	if(!gText.Prepare())
		return false;

	if(!gText.BindSet(pfontset))
		return false;

	gText.SetColor(color.r, color.g, color.b, color.a);
	gText.SetRectangle(minx, miny, maxx, maxy, insetx, insety);

	if(!gText.DrawString(pfontset, pstrString, x, y, reverse, lineoffset, minlineheight, xoffset))
		return false;

	gText.SetRectangle(0, 0, 0, 0, 0, 0);

	gText.UnBind(pfontset);
	gText.Reset();

	return true;
}

//====================================
//
//====================================
bool R_BeginTextRendering( const font_set_t* pfontset )
{
	const font_set_t* pfont = nullptr;
	if(!pfontset)
		pfont = gText.GetDefaultFont();
	else
		pfont = pfontset;

	if(!gText.Prepare())
		return false;

	if(!gText.BindSet(pfont))
		return false;

	return true;
}

//====================================
//
//====================================
void R_FinishTextRendering( const font_set_t* pfontset )
{
	if(!pfontset)
	{
		Con_Printf("%s - No font set specified.\n", __FUNCTION__);
		return;
	}

	gText.UnBind(pfontset);
	gText.Reset();
}

//====================================
//
//====================================
bool R_DrawCharacter( const font_set_t* pfontset, Int32 x, Int32 y, Char character, Uint32 r, Uint32 g, Uint32 b, Uint32 a )
{
	if(!pfontset)
		return true;

	return gText.DrawChar(pfontset, character, x, y, r, g, b, a);
}

//====================================
//
//====================================
bool R_PrintCounters( void )
{
	if( g_pCvarStats->GetValue() <= 0 )
		return true;

	static Double lasttime = 0;
	Double curtime = Sys_FloatTime();
	if(!lasttime)
	{
		lasttime = curtime;
		return true;
	}

	Double frametime = curtime-lasttime;
	lasttime = curtime;

	// prevent divide by zero
	if(frametime <= 0) frametime = 1;
	if(frametime > 1) frametime = 1;

	Uint32 fps = 1/frametime;
	CString strPrint;
	strPrint << (Int32)rns.counters.brushpolies << " wpolys(" << (Int32)rns.counters.batches << " batches), " 
		<< (Int32)rns.counters.modelpolies << " vbm polys, " 
		<< (Int32)rns.counters.particles << " particles, " 
		<< (Int32)fps << " fps\n";

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	if(!R_DrawString(color32_t(255, 255, 255, 255), 15, 25, strPrint.c_str(), nullptr))
	{
		Sys_ErrorPopup("Shader error: %s.", gText.GetShaderError());
		return false;
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	return true;
}

//====================================
//
//====================================
bool R_AddTempEntity( cl_entity_t *entity )
{
	if(!entity->pmodel)
		return false;

	CArray<Uint32> leafnums;
	Vector mins = entity->curstate.origin + entity->pmodel->mins;
	Vector maxs = entity->curstate.origin + entity->pmodel->maxs;

	entity->latched.origin = entity->curstate.origin;
	entity->latched.angles = entity->curstate.angles;

	Mod_FindTouchedLeafs(ens.pworld, leafnums, mins, maxs, ens.pworld->pnodes);
	if(!Common::CheckVisibility(leafnums, rns.pvisbuffer))
		return false;

	R_AddEntity(entity);
	return true;
}

//====================================
//
//====================================
bool R_DrawOrigins( void )
{
	if(g_pCvarDrawOrigins->GetValue() < 1)
		return true;

	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable() || !pDraw->DisableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0);

	const Float linelength = 16;

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pentity = rns.objects.pvisents[i];
		if(!pentity->pmodel)
			continue;

		Vector forward, right, up;
		Math::AngleVectors(pentity->curstate.angles, &forward, &right, &up);

		pDraw->Begin(GL_LINES);
		pDraw->Color4f(1, 0, 0, 1);
		pDraw->Vertex3fv(pentity->curstate.origin);
		pDraw->Vertex3fv(pentity->curstate.origin + forward*linelength);
		pDraw->Color4f(0, 1, 0, 1);
		pDraw->Vertex3fv(pentity->curstate.origin);
		pDraw->Vertex3fv(pentity->curstate.origin + right*linelength);
		pDraw->Color4f(0, 0, 1, 1);
		pDraw->Vertex3fv(pentity->curstate.origin);
		pDraw->Vertex3fv(pentity->curstate.origin + up*linelength);
		pDraw->End();
	}

	pDraw->Disable();

	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0);
	return true;
}

//====================================
//
//====================================
Float R_GetRenderFOV( Float viewsize )
{
	return g_pCvarDefaultFOV->GetValue()*(viewsize/g_pCvarReferenceFOV->GetValue());
}

//=============================================
//
//=============================================
void R_AnisotropyCvarCallBack( CCVar* pCVar )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	if(!pTextureManager)
		return;

	Int32 cvarValue = pCVar->GetValue();
	Int32 maxAnisotropy = pTextureManager->GetNbAnisotropySettings();

	if(cvarValue > maxAnisotropy)
	{
		Con_Printf("Invalid setting %d for cvar '%s', max value is %d.\n", cvarValue, pCVar->GetName(), maxAnisotropy);
		gConsole.CVarSetFloatValue(pCVar->GetName(), maxAnisotropy);
	}
	else if(cvarValue < 0)
	{
		Con_Printf("Invalid setting %d for cvar '%s', minimum value is 0.\n", cvarValue, pCVar->GetName(), 0);
		gConsole.CVarSetFloatValue(pCVar->GetName(), 0);
	}

	pTextureManager->UpdateAnisotropySettings(cvarValue);
}

//=============================================
//
//=============================================
void R_ActiveLoadMaxShadersCvarCallBack( CCVar* pCVar )
{
	Int32 maxShaders = pCVar->GetValue();
	if(maxShaders <= 0)
	{
		Con_Printf("Invalid setting %d for cvar '%s', min value is 1.\n", maxShaders, pCVar->GetName());
		gConsole.CVarSetFloatValue(pCVar->GetName(), 1);
	}
	else if(maxShaders > MAX_ACTIVELOAD_SHADERS)
	{
		Con_Printf("Invalid setting %d for cvar '%s', max value is %d.\n", maxShaders, pCVar->GetName(), MAX_ACTIVELOAD_SHADERS);
		gConsole.CVarSetFloatValue(pCVar->GetName(), MAX_ACTIVELOAD_SHADERS);
	}
}

//====================================
//
//====================================
void R_LoadSprite( cache_model_t* pmodel )
{
	if(pmodel->type != MOD_SPRITE)
		return;

	if(pmodel->isloaded)
		return;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	Uint32 frameindex = 0;
	const msprite_t* psprite = pmodel->getSprite();
	for(Uint32 i = 0; i < psprite->frames.size(); i++)
	{
		mspriteframedesc_t* pframedesc = &psprite->frames[i];
		if(pframedesc->type == SPR_SINGLE)
		{
			mspriteframe_t* pframe = pframedesc->pframeptr;

			CString name, basename;
			Common::Basename(pmodel->name.c_str(), basename);
			name << basename << (Int32)frameindex << ".spr";
			frameindex++;

			const color24_t* ppalette = reinterpret_cast<const color24_t*>(psprite->palette);
			pframe->ptexture = pTextureManager->LoadPallettedTexture(name.c_str(), RS_GAME_LEVEL, pframe->pdata, ppalette, pframe->width, pframe->height, TX_FL_NONE);
			if(!pframe->ptexture)
			{
				Con_Printf("%s - Failed to load frame %d for sprite %s.\n", __FUNCTION__, frameindex, pmodel->name.c_str());
				pframe->ptexture = pTextureManager->GetDummyTexture();
			}
		}
		else if(pframedesc->type == SPR_GROUP)
		{
			mspritegroup_t* pgroup = pframedesc->pgroupptr;
			for(Uint32 j = 0; j < pgroup->frames.size(); j++)
			{
				mspriteframe_t* pframe = pgroup->frames[j];

				CString name, basename;
				Common::Basename(pmodel->name.c_str(), basename);
				name << basename << (Int32)frameindex << ".spr";
				frameindex++;

				const color24_t* ppalette = reinterpret_cast<const color24_t*>(psprite->palette);
				pframe->ptexture = pTextureManager->LoadPallettedTexture(name.c_str(), RS_GAME_LEVEL, pframe->pdata, ppalette, pframe->width, pframe->height, TX_FL_NONE);
				if(!pframe->ptexture)
				{
					Con_Printf("%s - Failed to load frame %d for sprite %s.\n", __FUNCTION__, frameindex, pmodel->name.c_str());
					pframe->ptexture = pTextureManager->GetDummyTexture();
				}
			}
		}
	}

	// Mark as loaded
	pmodel->isloaded = true;
}

//====================================
//
//====================================
Float R_RenderFxBlend( cl_entity_t* pentity )
{
	Float alpha = 0;
	Float offset = pentity->entindex * 256;

	switch(pentity->curstate.renderfx)
	{
	case RenderFx_PulseSlow:
		{
			alpha = pentity->curstate.renderamt + SDL_sin(offset + rns.time * 2.0f) * 16.0f;
		}
		break;
	case RenderFx_PulseFast:
		{
			alpha = pentity->curstate.renderamt + SDL_sin(offset + rns.time * 8.0f) * 16.0f;
		}
		break;
	case RenderFx_PulseSlowWide:
		{
			alpha = pentity->curstate.renderamt + SDL_sin(offset + rns.time * 2.0f) * 64.0f;
		}
		break;
	case RenderFx_PulseFastWide:
		{
			alpha = pentity->curstate.renderamt + SDL_sin(offset + rns.time * 8.0f) * 64.0f;
		}
		break;
	case RenderFx_FadeSlow:
		{
			if(pentity->curstate.renderamt > 0)
				pentity->curstate.renderamt -= 1.0f;
			else
				pentity->curstate.renderamt = 0;

			alpha = pentity->curstate.renderamt;
		}
		break;
	case RenderFx_FadeFast:
		{
			if(pentity->curstate.renderamt > 3)
				pentity->curstate.renderamt -= 4.0f;
			else
				pentity->curstate.renderamt = 0;

			alpha = pentity->curstate.renderamt;
		}
		break;
	case RenderFx_SolidSlow:
		{
			if(pentity->curstate.renderamt < 255)
				pentity->curstate.renderamt += 1.0f;
			else
				pentity->curstate.renderamt = 255.0f;

			alpha = pentity->curstate.renderamt;
		}
		break;
	case RenderFx_SolidFast:
		{
			if(pentity->curstate.renderamt < 252)
				pentity->curstate.renderamt += 4.0f;
			else
				pentity->curstate.renderamt = 255.0f;

			alpha = pentity->curstate.renderamt;
		}
		break;
	case RenderFx_StrobeSlow:
		{
			alpha = SDL_sin(offset + rns.time * 4.0f);
		}
		break;
	case RenderFx_StrobeFast:
		{
			alpha = SDL_sin(offset + rns.time * 16.0f);
			if((alpha * 20.0f) < 0)
				alpha = 0;
		}
		break;
	case RenderFx_StrobeFaster:
		{
			alpha = SDL_sin(offset + rns.time * 36.0f);
			if((alpha * 20.0f) < 0)
				alpha = 0;
		}
		break;
	case RenderFx_FlickerSlow:
		{
			alpha = SDL_sin(rns.time * 18.0f + offset) + SDL_sin(rns.time * 2.0f);
			if((alpha * 20.0f) < 0)
				alpha = 0;
		}
		break;
	case RenderFx_FlickerFast:
		{
			alpha = SDL_sin(rns.time * 23.0f + offset) + SDL_sin(rns.time * 16.0f);
			if((alpha * 20.0f) < 0)
				alpha = 0;
		}
		break;
	case RenderFx_Distort:
		{
			Float originDelta = (pentity->curstate.origin[0] - rns.view.v_origin[0]) * rns.view.v_forward[0]
				+ (pentity->curstate.origin[1] - rns.view.v_origin[1]) * rns.view.v_forward[1]
				+ (pentity->curstate.origin[2] - rns.view.v_origin[2]) * rns.view.v_forward[2];

			if(originDelta <= 0)
			{
				alpha = 0;
			}
			else
			{
				pentity->curstate.renderamt = 180;
				if(originDelta <= 100)
					alpha = 180;
				else
					alpha = (1.0f - (originDelta - 100.0f) * 0.0025f) * 180.0f;
			}

			alpha += Common::RandomLong(-32, 32);
		}
		break;
	case RenderFx_Hologram:
		{
			pentity->curstate.renderamt = alpha = 180.0f;
			alpha = Common::RandomLong(-32, 32);
		}
		break;
	default:
		alpha = pentity->curstate.renderamt;
		break;
	}

	return clamp(alpha, 0, 255);
}


//====================================
//
//====================================
Int32 R_SortEntities( const void* p1, const void* p2 )
{
	cl_entity_t **ppentity1 = (cl_entity_t **)p1;
	cl_entity_t **ppentity2 = (cl_entity_t **)p2;

	Vector center1 = (*ppentity1)->curstate.origin + ((*ppentity1)->curstate.mins + (*ppentity1)->curstate.maxs) * 0.5;
	Vector center2 = (*ppentity2)->curstate.origin + ((*ppentity2)->curstate.mins + (*ppentity2)->curstate.maxs) * 0.5;

	Float length1 = (center1 - rns.view.v_origin).Length();
	Float length2 = (center2 - rns.view.v_origin).Length();

	if(length1 < length2)
		return 1;
	else if(length1 == length2)
		return 0;
	else
		return -1;
}

//====================================
//
//====================================
void R_AddShaderForLoading( CGLSLShader* pShader )
{
	if(!g_pendingShadersList.empty())
	{
		g_pendingShadersList.begin();
		while(!g_pendingShadersList.end())
		{
			active_load_shader_t& shaderinfo = g_pendingShadersList.get();
			if(pShader == shaderinfo.pshader)
			{
				Con_Printf("%s - Shader '%s' already in list.\n", __FUNCTION__, pShader->GetShaderScriptName());
				return;
			}
			g_pendingShadersList.next();
		}
	}

	active_load_shader_t shaderinfo;
	shaderinfo.pshader = pShader;

	g_pendingShadersList.add(shaderinfo);

	Con_Printf("Added shader '%s' for active loading.\n", pShader->GetShaderScriptName());
}

//====================================
//
//====================================
bool R_PerformPendingShaderLoads( void )
{
	if(g_pendingShadersList.empty())
		return true;

	Int32 maxShaders = g_pCvarGLSLActiveMaxShaders->GetValue();
	if(maxShaders <= 0)
		maxShaders = 1;
	else if(maxShaders > MAX_ACTIVELOAD_SHADERS)
		maxShaders = MAX_ACTIVELOAD_SHADERS;

	Uint32 numCompiled = 0;
	while(true)
	{
		// Pick first
		g_pendingShadersList.begin();
		active_load_shader_t& shaderinfo = g_pendingShadersList.get();

		Uint32 i = shaderinfo.lastshaderindex;
		Uint32 totalVariations = shaderinfo.pshader->GetNbTotalShaderVariations();
		for(; i < totalVariations; i++)
		{
			if(shaderinfo.pshader->IsShaderVariationCompiled(i))
				continue;

			if(!shaderinfo.pshader->CompileShaderVariation(i))
			{
				Sys_ErrorPopup("Shader error: %s.\n", shaderinfo.pshader->GetError());
				return false;
			}

			numCompiled++;
			if(numCompiled >= maxShaders)
				break;
		}

		if(i == totalVariations)
		{
			Con_Printf("Compiled all variations of shader script '%s'.\n", shaderinfo.pshader->GetShaderScriptName());
			g_pendingShadersList.remove(g_pendingShadersList.get_link());
		}
		else
		{
			// Remember for next
			shaderinfo.lastshaderindex = i + 1;
		}

		if(numCompiled >= maxShaders)
			break;
		else if(!g_pendingShadersList.empty())
			continue;
		else
			break;
	}
	
	return true;
}

//====================================
//
//====================================
void Cmd_PasteDecal( void )
{
	if(gCommands.Cmd_Argc() < 5)
	{
		Con_Printf("pastedecal usage: pastedecal <decal group name> <life> <fade time> <growth time>\n");
		return;
	}

	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Float life = SDL_atof(gCommands.Cmd_Argv(2));
	Float fadetime = SDL_atof(gCommands.Cmd_Argv(3));
	Float growthtime = SDL_atof(gCommands.Cmd_Argv(4));

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_HITBOXES, HULL_POINT, NO_ENTITY_INDEX, tr);

	if(!(tr.flags & FL_TR_ALLSOLID) && tr.fraction != 1.0 && tr.hitentity != NO_ENTITY_INDEX)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(tr.hitentity);
		if(pentity && pentity->pmodel)
		{
			const Char* pdecalname = gCommands.Cmd_Argv(1);
			if(pdecalname)
			{
				decalgroupentry_t *pentry = gDecals.GetDecalList().GetRandom(pdecalname);
				if(pentry)
				{
					Con_Printf("pastedecal called for decal '%s' on model '%s'\n", pdecalname, pentity->pmodel->name.c_str());

					if(pentity->pmodel->type == MOD_VBM)
						gVBMRenderer.CreateDecal(tr.endpos, tr.plane.normal, pentry, pentity, FL_DECAL_NORMAL_PERMISSIVE);
					else
						gBSPRenderer.CreateDecal(tr.endpos, tr.plane.normal, pentry, false, life, fadetime, growthtime);
				}
			}
		}
	}
}

//====================================
//
//====================================
void Cmd_CreateSprite( void )
{
	if(gCommands.Cmd_Argc() < 2)
	{
		Con_Printf("createsprite usage: createsprite <sprite name>\n");
		return;
	}

	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_HITBOXES, HULL_SMALL, NO_ENTITY_INDEX, tr);

	if(!(tr.flags & FL_TR_ALLSOLID) && tr.fraction != 1.0 && tr.hitentity != NO_ENTITY_INDEX)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(tr.hitentity);
		if(pentity && pentity->pmodel)
		{
			const Char* pspritename = gCommands.Cmd_Argv(1);
			CString spritename;
			spritename << "sprites/" << pspritename << ".spr";

			cache_model_t* pcache = gModelCache.LoadModel(spritename.c_str());
			if(pcache)
			{
				Con_Printf("createsprite called for sprite '%s' on model '%s'\n", spritename.c_str(), pentity->pmodel->name.c_str());
				gTempEntities.CreateTempSprite(tr.endpos, ZERO_VECTOR, 0.1, pcache->cacheindex, RENDER_TRANSADDITIVE, 0, 255, 15, 0, 0);
			}
		}
	}
}

//====================================
//
//====================================
void Cmd_CreateDynamicLight( void )
{
	if(gCommands.Cmd_Argc() < 7)
	{
		Con_Printf("createdynlight usage: createdynlight <r> <g> <b> <radius> <life> <decay>\n");
		return;
	}

	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector color;
	color.x = SDL_atof(gCommands.Cmd_Argv(1))/255.0f;
	color.y = SDL_atof(gCommands.Cmd_Argv(2))/255.0f;
	color.z = SDL_atof(gCommands.Cmd_Argv(3))/255.0f;
	Float radius = SDL_atof(gCommands.Cmd_Argv(4));
	Float life = SDL_atof(gCommands.Cmd_Argv(5));
	Float decay = SDL_atof(gCommands.Cmd_Argv(6));

	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
	{
		Con_Printf("%s - Could not get local player.\n", __FUNCTION__);
		return;
	}

	Vector origin = pplayer->curstate.origin + pplayer->curstate.view_offset;
	cl_dlight_t* pdl = gDynamicLights.AllocDynamicPointLight(0, 0, false, false, nullptr);
	pdl->origin = origin;
	pdl->color = color;
	pdl->decay = decay;
	pdl->radius = radius;
	pdl->die = cls.cl_time + life;
}

//====================================
//
//====================================
void Cmd_CreateSpotLight( void )
{
	if(gCommands.Cmd_Argc() < 8)
	{
		Con_Printf("createspotlight usage: createspotlight <r> <g> <b> <radius> <cone size> <life> <decay>\n");
		return;
	}

	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector color;
	color.x = SDL_atof(gCommands.Cmd_Argv(1))/255.0f;
	color.y = SDL_atof(gCommands.Cmd_Argv(2))/255.0f;
	color.z = SDL_atof(gCommands.Cmd_Argv(3))/255.0f;
	Float radius = SDL_atof(gCommands.Cmd_Argv(4));
	Float conesize = SDL_atof(gCommands.Cmd_Argv(5));
	Float life = SDL_atof(gCommands.Cmd_Argv(6));
	Float decay = SDL_atof(gCommands.Cmd_Argv(7));

	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
	{
		Con_Printf("%s - Could not get local player.\n", __FUNCTION__);
		return;
	}

	Vector origin = pplayer->curstate.origin + pplayer->curstate.view_offset;
	cl_dlight_t* pdl = gDynamicLights.AllocDynamicSpotlight(0, 0, false, false, nullptr);
	pdl->cone_size = conesize;
	pdl->origin = origin;
	pdl->angles = pplayer->curstate.viewangles;
	pdl->color = color;
	pdl->decay = decay;
	pdl->radius = radius;
	pdl->die = cls.cl_time + life;
}

//====================================
//
//====================================
void Cmd_LoadModel( void )
{
	if(gCommands.Cmd_Argc() < 2)
	{
		Con_Printf("loadmodel usage: loadmodel <model file path>\n");
		return;
	}

	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	CString strModel = gCommands.Cmd_Argv(1);
	if(strModel.find(0, "bsp") != -1)
	{
		Con_Printf("%s - BSP files are not allowed to be loaded through this function.\n", __FUNCTION__);
		return;
	}

	cache_model_t* pmodel = gModelCache.LoadModel(strModel.c_str());
	if(!pmodel)
		Con_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, strModel.c_str());
	else
		Con_Printf("%s - Loaded '%s'.\n", __FUNCTION__, strModel.c_str());
}

//====================================
//
//====================================
void Cmd_EFX_Bubbles( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	Vector mins = tr.endpos - Vector(64, 64, 64);
	Vector maxs = tr.endpos + Vector(64, 64, 64);

	cache_model_t* pcache = gModelCache.LoadModel("sprites/bubble.spr");
	if(!pcache)
		return;

	CL_Bubbles(mins, maxs, 512, pcache->cacheindex, 64, 64);
}

//====================================
//
//====================================
void Cmd_EFX_BubbleTrail( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	cache_model_t* pcache = gModelCache.LoadModel("sprites/bubble.spr");
	if(!pcache)
		return;

	Vector end = tr.endpos - Vector(64, 64, 64);
	CL_BubbleTrail(rns.view.v_origin, end, 512, pcache->cacheindex, 64, 64);
}

//====================================
//
//====================================
void Cmd_EFX_BreakModel( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	cache_model_t* pcache = gModelCache.LoadModel("models/metalplategibs.mdl");
	if(!pcache)
		return;

	Vector dir(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
	dir.Normalize();

	CL_BreakModel(tr.endpos+tr.plane.normal*4, Vector(64, 64, 64), dir, 40, 60, 4, pcache->cacheindex, TE_BOUNCE_SHELL, 0, 0, 0);
}

//====================================
//
//====================================
void Cmd_EFX_SphereModel( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	cache_model_t* pcache = gModelCache.LoadModel("models/metalplategibs.mdl");
	if(!pcache)
		return;

	Vector dir(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
	dir.Normalize();

	CL_SphereModel(tr.endpos+tr.plane.normal*4, Common::RandomFloat(32, 64), 60, 8, pcache->cacheindex, 0, 0, TE_BOUNCE_SHELL);
}

//====================================
//
//====================================
void Cmd_EFX_FunnelSprite( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	cache_model_t* pcache = gModelCache.LoadModel("sprites/glow01.spr");
	if(!pcache)
		return;

	CL_FunnelSprite(tr.endpos+tr.plane.normal*4, ZERO_VECTOR, 255, pcache->cacheindex, Common::RandomLong(0, 1) ? true : false);
}

//====================================
//
//====================================
void Cmd_EFX_TempModel( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	cache_model_t* pcache = gModelCache.LoadModel("models/metalplategibs.mdl");
	if(!pcache)
		return;

	Vector dir(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
	dir.Normalize();

	CL_TempModel(tr.endpos+tr.plane.normal*4, dir, Vector(0, Common::RandomLong(-360, 360), 0), 60, pcache->cacheindex, 0, TE_BOUNCE_SHELL, 0, 0);
}

//====================================
//
//====================================
void Cmd_EFX_TempSprite( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	cache_model_t* pcache = gModelCache.LoadModel("sprites/glow01.spr");
	if(!pcache)
		return;

	Vector dir(Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1), Common::RandomFloat(-1, 1));
	dir.Normalize();

	CL_TempSprite(tr.endpos+tr.plane.normal*4, dir*Common::RandomFloat(2.5, 6.5), Common::RandomFloat(0.5, 2.5), pcache->cacheindex, RENDER_TRANSADDITIVE, RenderFx_None, 255, 60, 0, TE_FL_COLLIDEWORLD|TE_FL_GRAVITY);
}

//====================================
//
//====================================
void Cmd_EFX_ParticleExplosion1( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateParticleExplosion1(tr.endpos+tr.plane.normal*4);
}

//====================================
//
//====================================
void Cmd_EFX_ParticleExplosion2( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateParticleExplosion2(tr.endpos+tr.plane.normal*4, 5, 64);
}

//====================================
//
//====================================
void Cmd_EFX_BlobExplosion( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateBlobExplosion(tr.endpos+tr.plane.normal*4);
}

//====================================
//
//====================================
void Cmd_EFX_CreateRocketExplosion( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateRocketExplosion(tr.endpos+tr.plane.normal*4, 128);
}

//====================================
//
//====================================
void Cmd_EFX_CreateParticleEffect( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	Vector velocity(Common::RandomFloat(-32, 32), Common::RandomFloat(-32, 32), Common::RandomFloat(-32, 32));

	gLegacyParticles.CreateParticleEffect(tr.endpos+tr.plane.normal*4, velocity, 128, 7);
}

//====================================
//
//====================================
void Cmd_EFX_CreateLavaSplash( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateLavaSplash(tr.endpos+tr.plane.normal*4);
}

//====================================
//
//====================================
void Cmd_EFX_CreateTeleportSplash( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateTeleportSplash(tr.endpos + tr.plane.normal*4);
}

//====================================
//
//====================================
void Cmd_EFX_CreateRocketTrail( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("%s - No active game.\n", __FUNCTION__);
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gLegacyParticles.CreateRocketTrail(tr.endpos + tr.plane.normal*4, tr.endpos + tr.plane.normal* 64, Common::RandomLong(0, 6));
}

//====================================
//
//====================================
void Cmd_EFX_BeamSetStart( void )
{
	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	if(tr.hitentity != NO_ENTITY_INDEX && tr.hitentity != WORLDSPAWN_ENTITY_INDEX)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(tr.hitentity);
		if(pentity->pmodel && pentity->pmodel->type == MOD_VBM)
			g_pBeamStartEntity = pentity;
	}

	g_beamStartPosition = tr.endpos;

	Con_Printf("Beam start set.\n");
}

//====================================
//
//====================================
void Cmd_EFX_BeamSetEnd( void )
{
	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	if(tr.hitentity != NO_ENTITY_INDEX && tr.hitentity != WORLDSPAWN_ENTITY_INDEX)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(tr.hitentity);
		if(pentity->pmodel && pentity->pmodel->type == MOD_VBM)
			g_pBeamEndEntity = pentity;
	}

	g_beamEndPosition = tr.endpos;

	Con_Printf("Beam end set.\n");
}

//====================================
//
//====================================
void Cmd_EFX_BeamLightning( void )
{
	if(gCommands.Cmd_Argc() < 8)
	{
		Con_Printf("efx_beamlightning usage: efx_beamlightning <sprite name> <life> <width> <amplitude> <brightness> <speed> <noise speed>\n");
		return;
	}

	CString spritename = gCommands.Cmd_Argv(1);
	Float life = SDL_atof(gCommands.Cmd_Argv(2));
	Float width = SDL_atof(gCommands.Cmd_Argv(3));
	Float amplitude = SDL_atof(gCommands.Cmd_Argv(4));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(5));
	Float speed = SDL_atof(gCommands.Cmd_Argv(6));
	Float noisespeed = SDL_atof(gCommands.Cmd_Argv(7));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	Vector endPosition = g_beamEndPosition;
	if(endPosition.IsZero())
	{
		Vector vend, vforward;
		Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
		Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

		trace_t tr;
		CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
		if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
			return;

		endPosition = tr.endpos;
	}

	Vector startPosition = g_beamStartPosition;
	if(startPosition.IsZero())
	{
		cl_entity_t* pPlayer = CL_GetLocalPlayer();
		if(!pPlayer)
			return;

		startPosition = pPlayer->curstate.origin;
	}

	gBeamRenderer.BeamLightning(endPosition, startPosition, pmodel->cacheindex, life, width, amplitude, brightness, speed, noisespeed, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_BeamCirclePoints( void )
{
	if(gCommands.Cmd_Argc() < 14)
	{
		Con_Printf("efx_beamcirclepoints usage: efx_beamcirclepoints <type> <sprite name> <life> <width> <amplitude> <brightness> <speed> <noise speed> <start frame> <framerate> <r> <g> <b>\n");
		return;
	}

	Int32 type = SDL_atoi(gCommands.Cmd_Argv(1));
	if(type < 0 || type >= NB_BEAM_TYPES)
	{
		Con_Printf("Invalid beam type specified.\n");
		return;
	}

	CString spritename = gCommands.Cmd_Argv(2);
	Float life = SDL_atof(gCommands.Cmd_Argv(3));
	Float width = SDL_atof(gCommands.Cmd_Argv(4));
	Float amplitude = SDL_atof(gCommands.Cmd_Argv(5));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(6));
	Float speed = SDL_atof(gCommands.Cmd_Argv(7));
	Float noisespeed = SDL_atof(gCommands.Cmd_Argv(8));
	Uint32 startframe = SDL_atoi(gCommands.Cmd_Argv(9));
	Float framerate = SDL_atof(gCommands.Cmd_Argv(10));
	Float r = SDL_atof(gCommands.Cmd_Argv(11));
	Float g = SDL_atof(gCommands.Cmd_Argv(12));
	Float b = SDL_atof(gCommands.Cmd_Argv(13));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	Vector endPosition = g_beamEndPosition;
	if(endPosition.IsZero())
	{
		Vector vend, vforward;
		Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
		Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

		trace_t tr;
		CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
		if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
			return;

		endPosition = tr.endpos;
	}

	Vector startPosition = g_beamStartPosition;
	if(startPosition.IsZero())
	{
		cl_entity_t* pPlayer = CL_GetLocalPlayer();
		if(!pPlayer)
			return;

		startPosition = pPlayer->curstate.origin;
	}

	gBeamRenderer.BeamCirclePoints((beam_types_t)type, startPosition, endPosition, pmodel->cacheindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_BeamEntityPoint( void )
{
	if(gCommands.Cmd_Argc() < 14)
	{
		Con_Printf("efx_beamentitypoint usage: efx_beamentitypoint <attachment index on entity> <sprite name> <life> <width> <amplitude> <brightness> <speed> <noise speed> <start frame> <framerate> <r> <g> <b>\n");
		return;
	}

	if(!g_pBeamStartEntity)
	{
		Con_Printf("No start entity set.\n");
		return;
	}

	Int32 attachment = SDL_atoi(gCommands.Cmd_Argv(1));
	CString spritename = gCommands.Cmd_Argv(2);
	Float life = SDL_atof(gCommands.Cmd_Argv(3));
	Float width = SDL_atof(gCommands.Cmd_Argv(4));
	Float amplitude = SDL_atof(gCommands.Cmd_Argv(5));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(6));
	Float speed = SDL_atof(gCommands.Cmd_Argv(7));
	Float noisespeed = SDL_atof(gCommands.Cmd_Argv(8));
	Uint32 startframe = SDL_atoi(gCommands.Cmd_Argv(9));
	Float framerate = SDL_atof(gCommands.Cmd_Argv(10));
	Float r = SDL_atof(gCommands.Cmd_Argv(11));
	Float g = SDL_atof(gCommands.Cmd_Argv(12));
	Float b = SDL_atof(gCommands.Cmd_Argv(13));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	Vector endPosition = g_beamEndPosition;
	if(endPosition.IsZero())
	{
		Vector vend, vforward;
		Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
		Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

		trace_t tr;
		CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
		if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
			return;

		endPosition = tr.endpos;
	}

	gBeamRenderer.BeamEntityPoint(g_pBeamStartEntity->entindex, attachment, endPosition, pmodel->cacheindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_BeamEntities( void )
{
	if(gCommands.Cmd_Argc() < 15)
	{
		Con_Printf("efx_beamentities usage: efx_beamentities <attachment index on entity 1> <attachment index on entity 2> <sprite name> <life> <width> <amplitude> <brightness> <speed> <noise speed> <start frame> <framerate> <r> <g> <b>\n");
		return;
	}

	if(!g_pBeamStartEntity)
	{
		Con_Printf("No start entity set.\n");
		return;
	}

	if(!g_pBeamEndEntity)
	{
		Con_Printf("No end entity set.\n");
		return;
	}

	Int32 attachment1 = SDL_atoi(gCommands.Cmd_Argv(1));
	Int32 attachment2 = SDL_atoi(gCommands.Cmd_Argv(2));
	CString spritename = gCommands.Cmd_Argv(3);
	Float life = SDL_atof(gCommands.Cmd_Argv(4));
	Float width = SDL_atof(gCommands.Cmd_Argv(5));
	Float amplitude = SDL_atof(gCommands.Cmd_Argv(6));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(7));
	Float speed = SDL_atof(gCommands.Cmd_Argv(8));
	Float noisespeed = SDL_atof(gCommands.Cmd_Argv(9));
	Uint32 startframe = SDL_atoi(gCommands.Cmd_Argv(10));
	Float framerate = SDL_atof(gCommands.Cmd_Argv(11));
	Float r = SDL_atof(gCommands.Cmd_Argv(12));
	Float g = SDL_atof(gCommands.Cmd_Argv(13));
	Float b = SDL_atof(gCommands.Cmd_Argv(14));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	gBeamRenderer.BeamEntities(g_pBeamStartEntity->entindex, g_pBeamEndEntity->entindex, attachment1, attachment2, pmodel->cacheindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_BeamFollow( void )
{
	if(gCommands.Cmd_Argc() < 9)
	{
		Con_Printf("efx_beamfollow usage: efx_beamfollow <attachment index on entity> <sprite name> <life> <width> <brightness> <r> <g> <b>\n");
		return;
	}

	if(!g_pBeamStartEntity)
	{
		Con_Printf("No start entity set.\n");
		return;
	}

	Int32 attachment = SDL_atoi(gCommands.Cmd_Argv(1));
	CString spritename = gCommands.Cmd_Argv(2);
	Float life = SDL_atof(gCommands.Cmd_Argv(3));
	Float width = SDL_atof(gCommands.Cmd_Argv(4));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(5));
	Float r = SDL_atof(gCommands.Cmd_Argv(6));
	Float g = SDL_atof(gCommands.Cmd_Argv(7));
	Float b = SDL_atof(gCommands.Cmd_Argv(8));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	gBeamRenderer.BeamFollow(g_pBeamStartEntity->entindex, attachment, pmodel->cacheindex, life, width, brightness, r, g, b);
}

//====================================
//
//====================================
void Cmd_EFX_BeamVaporTrail( void )
{
	if(gCommands.Cmd_Argc() < 14)
	{
		Con_Printf("efx_beamfollow usage: efx_beamfollow <sprite 1 name> <sprite 2 name> <life> <width> <brightness> <r1> <g1> <b1> <r2> <g2> <b2> <color fade delay> <color fade duration>\n");
		return;
	}

	CString sprite1name = gCommands.Cmd_Argv(1);
	CString sprite2name = gCommands.Cmd_Argv(2);

	Float life = SDL_atof(gCommands.Cmd_Argv(3));
	Float width = SDL_atof(gCommands.Cmd_Argv(4));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(5));

	Float r1 = SDL_atof(gCommands.Cmd_Argv(6));
	Float g1 = SDL_atof(gCommands.Cmd_Argv(7));
	Float b1 = SDL_atof(gCommands.Cmd_Argv(8));

	Float r2 = SDL_atof(gCommands.Cmd_Argv(9));
	Float g2 = SDL_atof(gCommands.Cmd_Argv(10));
	Float b2 = SDL_atof(gCommands.Cmd_Argv(11));

	Float fadedelay = SDL_atof(gCommands.Cmd_Argv(12));
	Float fadeduration = SDL_atof(gCommands.Cmd_Argv(13));

	cache_model_t* pmodel1 = gModelCache.LoadModel(sprite1name.c_str());
	if(!pmodel1)
	{
		Con_Printf("Could not load sprite '%s'.\n", sprite1name.c_str());
		return;
	}

	cache_model_t* pmodel2 = gModelCache.LoadModel(sprite2name.c_str());
	if(!pmodel1)
	{
		Con_Printf("Could not load sprite '%s'.\n", sprite2name.c_str());
		return;
	}

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	gBeamRenderer.BeamVaporTrail(rns.view.v_origin, tr.endpos, pmodel1->cacheindex, pmodel2->cacheindex, fadedelay, fadeduration, life, width, brightness, r1, g1, b1, r2, g2, b2, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_BeamPoints( void )
{
	if(gCommands.Cmd_Argc() < 13)
	{
		Con_Printf("efx_beampoints usage: efx_beampoints <sprite name> <life> <width> <amplitude> <brightness> <speed> <noise speed> <start frame> <framerate> <r> <g> <b>\n");
		return;
	}

	CString spritename = gCommands.Cmd_Argv(1);
	Float life = SDL_atof(gCommands.Cmd_Argv(2));
	Float width = SDL_atof(gCommands.Cmd_Argv(3));
	Float amplitude = SDL_atof(gCommands.Cmd_Argv(4));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(5));
	Float speed = SDL_atof(gCommands.Cmd_Argv(6));
	Float noisespeed = SDL_atof(gCommands.Cmd_Argv(7));
	Uint32 startframe = SDL_atoi(gCommands.Cmd_Argv(8));
	Float framerate = SDL_atof(gCommands.Cmd_Argv(9));
	Float r = SDL_atof(gCommands.Cmd_Argv(10));
	Float g = SDL_atof(gCommands.Cmd_Argv(11));
	Float b = SDL_atof(gCommands.Cmd_Argv(12));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	Vector endPosition = g_beamEndPosition;
	if(endPosition.IsZero())
	{
		Vector vend, vforward;
		Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
		Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

		trace_t tr;
		CL_PlayerTrace(rns.view.v_origin, vend, FL_TRACE_NORMAL, HULL_POINT, NO_ENTITY_INDEX, tr);
		if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
			return;

		endPosition = tr.endpos;
	}

	Vector startPosition = g_beamStartPosition;
	if(startPosition.IsZero())
	{
		cl_entity_t* pPlayer = CL_GetLocalPlayer();
		if(!pPlayer)
			return;

		startPosition = pPlayer->curstate.origin;
	}

	gBeamRenderer.BeamPoints(endPosition, startPosition, pmodel->cacheindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_BeamRing( void )
{
	if(gCommands.Cmd_Argc() < 15)
	{
		Con_Printf("efx_beamentities usage: efx_beamentities <attachment index on entity 1> <attachment index on entity 2> <sprite name> <life> <width> <amplitude> <brightness> <speed> <noise speed> <start frame> <framerate> <r> <g> <b>\n");
		return;
	}

	if(!g_pBeamStartEntity)
	{
		Con_Printf("No start entity set.\n");
		return;
	}

	if(!g_pBeamEndEntity)
	{
		Con_Printf("No end entity set.\n");
		return;
	}

	Int32 attachment1 = SDL_atoi(gCommands.Cmd_Argv(1));
	Int32 attachment2 = SDL_atoi(gCommands.Cmd_Argv(2));
	CString spritename = gCommands.Cmd_Argv(3);
	Float life = SDL_atof(gCommands.Cmd_Argv(4));
	Float width = SDL_atof(gCommands.Cmd_Argv(5));
	Float amplitude = SDL_atof(gCommands.Cmd_Argv(6));
	Float brightness = SDL_atof(gCommands.Cmd_Argv(7));
	Float speed = SDL_atof(gCommands.Cmd_Argv(8));
	Float noisespeed = SDL_atof(gCommands.Cmd_Argv(9));
	Uint32 startframe = SDL_atoi(gCommands.Cmd_Argv(10));
	Float framerate = SDL_atof(gCommands.Cmd_Argv(11));
	Float r = SDL_atof(gCommands.Cmd_Argv(12));
	Float g = SDL_atof(gCommands.Cmd_Argv(13));
	Float b = SDL_atof(gCommands.Cmd_Argv(14));

	cache_model_t* pmodel = gModelCache.LoadModel(spritename.c_str());
	if(!pmodel)
	{
		Con_Printf("Could not load sprite '%s'.\n", spritename.c_str());
		return;
	}

	gBeamRenderer.BeamRing(g_pBeamStartEntity->entindex, g_pBeamEndEntity->entindex, attachment1, attachment2, pmodel->cacheindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, FL_BEAM_NONE);
}

//====================================
//
//====================================
void Cmd_EFX_CreateParticle( void )
{
	if(gCommands.Cmd_Argc() < 3)
	{
		Con_Printf("efx_createparticle usage: efx_createparticle <system script> <type(0 - script, 1 - cluster)>\n");
		return;
	}

	CString scriptname = gCommands.Cmd_Argv(1);
	Int32 type = SDL_atoi(gCommands.Cmd_Argv(2)) == 0 ? PART_SCRIPT_SYSTEM : PART_SCRIPT_CLUSTER;

	Vector vend, vforward;
	Math::AngleVectors(rns.view.v_angles, &vforward, nullptr, nullptr);
	Math::VectorMA(rns.view.v_origin, 1024, vforward, vend);

	trace_t tr;
	CL_PlayerTrace(rns.view.v_origin, vend, (FL_TRACE_NORMAL|FL_TRACE_HITBOXES), HULL_POINT, NO_ENTITY_INDEX, tr);
	if(tr.flags & (FL_TR_STARTSOLID|FL_TR_ALLSOLID) || tr.fraction == 1.0)
		return;

	Int32 attachflags = 0;
	Int32 boneindex = NO_POSITION;
	if(tr.hitentity != NO_ENTITY_INDEX && tr.hitentity != WORLDSPAWN_ENTITY_INDEX)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(tr.hitentity);
		if(pentity && pentity->pmodel && pentity->pmodel->type == MOD_VBM)
		{
			studiohdr_t* phdr = pentity->pmodel->getVBMCache()->pstudiohdr;
			const mstudiobbox_t* phitbox = phdr->getHitBox(tr.hitgroup);
			boneindex = phitbox->bone;
			attachflags |= (PARTICLE_ATTACH_TO_PARENT|PARTICLE_ATTACH_TO_BONE);
		}
		else
		{
			tr.hitentity = NO_ENTITY_INDEX;
		}
	}
	else
	{
		tr.hitentity = NO_ENTITY_INDEX;
	}

	gParticleEngine.CacheCreateSystem(tr.endpos, tr.plane.normal, (part_script_type_t)type, scriptname.c_str(), 0, tr.hitentity, 0, boneindex, attachflags);
}

//====================================
//
//====================================
void Cmd_BSPToSMD_Textures( void )
{
	Uint32 fileidx = 0;
	CString filepath;
	FILE* pf = NULL;

	while(true)
	{
		filepath.clear();
		filepath << "dump_tris_" << fileidx << ".smd";

		if(!FL_FileExists(filepath.c_str()))
			break;

		fileidx++;
	}

	filepath.clear();
	filepath << ens.gamedir << PATH_SLASH_CHAR << "dump_tris_" << fileidx << ".smd";

	pf = fopen(filepath.c_str(), "w");
	if(!pf)
		return;

	fprintf(pf, "version 1\n");
	fprintf(pf, "nodes\n");
	fprintf(pf, "  0 \"bone01\"  -1\n");
	fprintf(pf, "end\n");
	fprintf(pf, "skeleton\n");
	fprintf(pf, "time 0\n");
	fprintf(pf, "  0 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000\n");
	fprintf(pf, "end\n");

	fprintf(pf, "triangles\n");
	for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
	{
		cache_model_t* pcache = gModelCache.GetModelByIndex(i+1);
		if(!pcache)
			break;

		if(pcache->type != MOD_BRUSH)
			continue;

		brushmodel_t *pmodel = pcache->getBrushmodel();
		if(!pmodel)
			break;

		for(Uint32 j = 0; j < pmodel->nummodelsurfaces; j++)
		{
			msurface_t *psurf = &pmodel->psurfaces[pmodel->firstmodelsurface + j];
			if(!psurf->ptexinfo || !psurf->ptexinfo->ptexture)
				continue;

			if(!psurf->numedges)
				continue;

			if(psurf->flags & SURF_DRAWSKY)
				continue;

			if(psurf->flags & SURF_DRAWTURB)
				continue;

			bsp_vertex_t* pverts = new bsp_vertex_t[psurf->numedges];
			for(Uint32 k = 0; k < psurf->numedges; k++)
			{
				int e_index = pmodel->psurfedges[psurf->firstedge+k];
				if(e_index > 0)
				{
					Math::VectorCopy(pmodel->pvertexes[pmodel->pedges[e_index].vertexes[0]].origin, pverts[k].origin);
				}
				else
				{
					Math::VectorCopy(pmodel->pvertexes[pmodel->pedges[-e_index].vertexes[1]].origin, pverts[k].origin);
				}

				mtexinfo_t *ptexinfo = psurf->ptexinfo;
				pverts[k].texcoord[0] = Math::DotProduct(pverts[k].origin, ptexinfo->vecs[0]) + ptexinfo->vecs[0][3];
				pverts[k].texcoord[0] /= (float)ptexinfo->ptexture->width;

				pverts[k].texcoord[1] = Math::DotProduct(pverts[k].origin, ptexinfo->vecs[1]) + ptexinfo->vecs[1][3];
				pverts[k].texcoord[1] /= (float)ptexinfo->ptexture->height;

				Math::VectorCopy(psurf->pplane->normal, pverts[k].normal);
			}

			int indexes[3];
			indexes[0] = 0;
			indexes[1] = 1;
			indexes[2] = 2;

			// Export first triangle
			fprintf(pf, "%s.bmp\n", psurf->ptexinfo->ptexture->name.c_str());
			for(Uint32 k = 0; k < 3; k++)
			{
				fprintf(pf, "  0   %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %.4f\n",
				pverts[indexes[k]].origin[0], pverts[indexes[k]].origin[1], pverts[indexes[k]].origin[2],
				pverts[indexes[k]].normal[0], pverts[indexes[k]].normal[1], pverts[indexes[k]].normal[2],
				pverts[indexes[k]].texcoord[0], pverts[indexes[k]].texcoord[1]);
			}

			// Export the rest
			for(Uint32 k = 0, l = 3; k < (psurf->numedges-3); k++, l++)
			{
				indexes[1] = indexes[2];
				indexes[2] = l;

				fprintf(pf, "%s.bmp\n", psurf->ptexinfo->ptexture->name.c_str());
				for(int m = 0; m < 3; m++)
				{
					fprintf(pf, "  0   %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %.4f\n",
					pverts[indexes[m]].origin[0], pverts[indexes[m]].origin[1], pverts[indexes[m]].origin[2],
					pverts[indexes[m]].normal[0], pverts[indexes[m]].normal[1], pverts[indexes[m]].normal[2],
					pverts[indexes[m]].texcoord[0], pverts[indexes[m]].texcoord[1]);
				}
			}

			delete[] pverts;
		}
	}

	fprintf(pf, "end\n");
	fclose(pf);

	Con_Printf("Exported %s.\n", filepath.c_str());
}

//====================================
//
//====================================
void Cmd_BSPToSMD_Lightmap( void )
{
	Uint32 fileidx = 0;
	CString filepath;
	FILE* pf = NULL;

	while(true)
	{
		filepath.clear();
		filepath << "dump_tris_" << fileidx << ".smd";

		if(!::FL_FileExists(filepath.c_str()))
			break;

		fileidx++;
	}

	filepath.clear();
	filepath << ens.gamedir << PATH_SLASH_CHAR << "dump_tris_" << fileidx << ".smd";

	pf = fopen(filepath.c_str(), "w");
	if(!pf)
		return;

	fprintf(pf, "version 1\n");
	fprintf(pf, "nodes\n");
	fprintf(pf, "  0 \"bone01\"  -1\n");
	fprintf(pf, "end\n");
	fprintf(pf, "skeleton\n");
	fprintf(pf, "time 0\n");
	fprintf(pf, "  0 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000\n");
	fprintf(pf, "end\n");

	fprintf(pf, "triangles\n");
	for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
	{
		cache_model_t* pcache = gModelCache.GetModelByIndex(i+1);
		if(!pcache)
			break;

		if(pcache->type != MOD_BRUSH)
			continue;

		brushmodel_t *pmodel = pcache->getBrushmodel();
		if(!pmodel)
			break;

		for(Uint32 j = 0; j < pmodel->nummodelsurfaces; j++)
		{
			msurface_t *psurf = &pmodel->psurfaces[pmodel->firstmodelsurface + j];
			if(!psurf->ptexinfo || !psurf->ptexinfo->ptexture)
				continue;

			if(!psurf->numedges)
				continue;

			if(psurf->flags & SURF_DRAWSKY)
				continue;

			if(psurf->flags & SURF_DRAWTURB)
				continue;

			bsp_vertex_t* pverts = new bsp_vertex_t[psurf->numedges];
			for(Uint32 k = 0; k < psurf->numedges; k++)
			{
				int e_index = pmodel->psurfedges[psurf->firstedge+k];
				if(e_index > 0)
				{
					Math::VectorCopy(pmodel->pvertexes[pmodel->pedges[e_index].vertexes[0]].origin, pverts[k].origin);
				}
				else
				{
					Math::VectorCopy(pmodel->pvertexes[pmodel->pedges[-e_index].vertexes[1]].origin, pverts[k].origin);
				}

				// Set lightmap coords
				mtexinfo_t *ptexinfo = psurf->ptexinfo;
				pverts[k].lmapcoord[0] = Math::DotProduct(pverts[k].origin, ptexinfo->vecs[0]) + ptexinfo->vecs[0][3];
				pverts[k].lmapcoord[0] -= psurf->texturemins[0];
				pverts[k].lmapcoord[0] += psurf->light_s*16+8;
				pverts[k].lmapcoord[0] /= (Float)CBSPRenderer::LIGHTMAP_WIDTH*16;

				pverts[k].lmapcoord[1] = Math::DotProduct(pverts[k].origin, ptexinfo->vecs[1]) + ptexinfo->vecs[1][3];
				pverts[k].lmapcoord[1] -= psurf->texturemins[1];
				pverts[k].lmapcoord[1] += psurf->light_t*16+8;
				pverts[k].lmapcoord[1] /= (Float)gBSPRenderer.GetLightmapHeight()*16;

				Math::VectorCopy(psurf->pplane->normal, pverts[k].normal);
			}

			Uint32 indexes[3];
			indexes[0] = 0;
			indexes[1] = 1;
			indexes[2] = 2;

			// Export first triangle
			fprintf(pf, "lightmap.bmp\n");
			for(Uint32 k = 0; k < 3; k++)
			{
				fprintf(pf, "  0   %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %f  %f\n",
				pverts[indexes[k]].origin[0], pverts[indexes[k]].origin[1], pverts[indexes[k]].origin[2],
				pverts[indexes[k]].normal[0], pverts[indexes[k]].normal[1], pverts[indexes[k]].normal[2],
				pverts[indexes[k]].lmapcoord[0], pverts[indexes[k]].lmapcoord[1]);
			}

			// Export the rest
			for(Uint32 k = 0, l = 3; k < (psurf->numedges-3); k++, l++)
			{
				indexes[1] = indexes[2];
				indexes[2] = l;

				fprintf(pf, "lightmap.bmp\n");
				for(Uint32 m = 0; m < 3; m++)
				{
					fprintf(pf, "  0   %.4f  %.4f  %.4f  %.4f  %.4f  %.4f  %f  %f\n",
					pverts[indexes[m]].origin[0], pverts[indexes[m]].origin[1], pverts[indexes[m]].origin[2],
					pverts[indexes[m]].normal[0], pverts[indexes[m]].normal[1], pverts[indexes[m]].normal[2],
					pverts[indexes[m]].lmapcoord[0], pverts[indexes[m]].lmapcoord[1]);
				}
			}

			delete[] pverts;
		}
	}

	fprintf(pf, "end\n");
	fclose(pf);

	Con_Printf("Exported %s.\n", filepath.c_str());

	// Export lightmap too
	fileidx = 0;
	while(true)
	{
		filepath.clear();
		filepath << "dump_lightmap_" << fileidx << ".tga";
		fileidx++;

		if(!::FL_FileExists(filepath.c_str()))
			break;
	}

	Uint32 lightmapWidth = CBSPRenderer::LIGHTMAP_WIDTH;
	Uint32 lightmapHeight = gBSPRenderer.GetLightmapHeight();

	// alloc default lightmap's data
	Uint32 lightmapdatasize = 0;
	color32_t* plightmap = new color32_t[lightmapWidth*lightmapHeight];
	memset(plightmap, 0, sizeof(color32_t)*lightmapWidth*lightmapHeight);

	// Process the surfaces
	for(Uint32 i = 0; i < ens.pworld->numsurfaces; i++)
	{
		const msurface_t* psurface = &ens.pworld->psurfaces[i];
		if(psurface->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;

		// Determine sizes
		Uint32 xsize = (psurface->extents[0]>>4)+1;
		Uint32 ysize = (psurface->extents[1]>>4)+1;
		Uint32 size = xsize*ysize;

		// Build the base lightmap
		color24_t* psrc = psurface->psamples;
		R_BuildLightmap(psurface->light_s, psurface->light_t, psrc, psurface, plightmap, 0, lightmapWidth, false, false);
		lightmapdatasize += size*sizeof(color32_t);
	}

	const byte* pwritedata = reinterpret_cast<const byte*>(plightmap);
	TGA_Write(pwritedata, 4, lightmapWidth, lightmapHeight, filepath.c_str(), FL_GetInterface(), Con_Printf);
	delete[] plightmap;
}

//====================================
//
//====================================
void Cmd_TimeRefresh( void )
{
	glDrawBuffer(GL_FRONT);
	glFinish();

	clock_t beginTime = clock();

	for(Uint32 i = 0; i < 128; i++)
	{
		glViewport(0, 0, rns.screenwidth, rns.screenheight);
		glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		rns.view.v_angles[YAW] = ((Float)i/128.0f) * 360.0f;
		R_DrawScene();

		// Increment frame counter
		rns.framecount++;
	}

	glFinish();

	clock_t endTime = clock();
	clock_t duration = endTime - beginTime;

	Float seconds = duration * MILLISECONDS_TO_SECONDS;
	Float fps = 128.0f / seconds;

	Con_Printf("%f seconds(%f fps)\n", seconds, fps);

	glDrawBuffer(GL_BACK);

	// Swap the OGL buffer
	gWindow.SwapWindow();
}

//====================================
//
//====================================
void Cmd_DetailAuto( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	assert(pTextureManager != nullptr);

	CArray<detailtexture_t*> detailTexturesArray;
	CArray<detail_association_t*> detailTextureAssociationArray;

	if(!WAD_LoadDetailTextureAssociations(detailTexturesArray, detailTextureAssociationArray))
	{
		Con_EPrintf("Failed to load detail texture associations file.\n");
		return;
	}

	// Array of textures missing detail association
	CArray<CString> missingList;
	missingList.reserve(ens.pworld->numtextures);

	// Get WAD list
	CArray<CString> wadList;
	Common::GetWADList(ens.pworld->pentdata, wadList);

	// Go through all world textures
	for(Uint32 i = 0; i < ens.pworld->numtextures; i++)
	{
		mtexture_t* ptexture = &ens.pworld->ptextures[i];
		CString texname = ptexture->name;
		texname.tolower();
		
		// Seek it out in the textures list
		detail_association_t* passoc = nullptr;
		for(Uint32 j = 0; j < detailTextureAssociationArray.size(); j++)
		{
			if(!qstrcmp(texname, detailTextureAssociationArray[j]->maptexturename))
			{
				passoc = detailTextureAssociationArray[j];
				break;
			}
		}

		if(!passoc)
		{
			missingList.push_back(texname);
			continue;
		}

		// Find original PMF file
		en_material_t* pmaterial = nullptr;
		for(Uint32 j = 0; j < wadList.size(); j++)
		{
			CString folderPath = WAD_GetWADFolderPath(wadList[j].c_str(), WORLD_TEXTURES_PATH_BASE);
			CString materialPath = WAD_GetWADTexturePath(folderPath.c_str(), texname.c_str());

			pmaterial = pTextureManager->FindMaterialScript(materialPath.c_str(), RS_GAME_LEVEL);
			if(pmaterial)
				break;
		}

		if(!pmaterial)
		{
			Con_Printf("%s - Unable to find material script for '%s'.\n", __FUNCTION__, texname.c_str());
			continue;
		}

		// Do not override existing entries
		if(pmaterial->ptextures[MT_TX_DETAIL] && pmaterial->dt_scalex && pmaterial->dt_scaley)
			continue;

		// Find associated detail texture
		detailtexture_t* pdetail = detailTexturesArray[passoc->detailtextureidx];

		pmaterial->dt_scalex = (((Float)ptexture->width)/256.0)*(128.0/((float)pdetail->width))*12.0;
		pmaterial->dt_scaley = (((Float)ptexture->height)/256.0)*(128.0/((float)pdetail->height))*12.0;

		CString dtfilepath;
		dtfilepath << pdetail->filename;

		pmaterial->ptextures[MT_TX_DETAIL] = pTextureManager->LoadTexture(dtfilepath.c_str(), RS_GAME_LEVEL);

		if(!pmaterial->ptextures[MT_TX_DETAIL])
			continue;

		pTextureManager->WritePMFFile(pmaterial);
	}

	for(Uint32 i = 0; i < detailTexturesArray.size(); i++)
		delete detailTexturesArray[i];

	for(Uint32 i = 0; i < detailTextureAssociationArray.size(); i++)
		delete detailTextureAssociationArray[i];

	// Write list of textures missing detail textures
	CString str;
	str << "World textures missign detail textures: " << NEWLINE;

	for(Uint32 i = 0; i < missingList.size(); i++)
		str << missingList[i] << NEWLINE;

	// Write to file
	CString mapname;
	Common::Basename(ens.pworld->name.c_str(), mapname);

	CString filepath;
	filepath << "logs/" << mapname << "_detail_missing.log";

	const byte* pwritedata = reinterpret_cast<const byte*>(str.c_str());
	FL_WriteFile(pwritedata, str.length(), filepath.c_str());

	Con_Printf("Wrote list of textures without detail texture associations to '%s'.\n", filepath.c_str());
}

//====================================
//
//====================================
void Cmd_ListDefaultMaterials( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	assert(pTextureManager != nullptr);

	// Get WAD list
	CArray<CString> wadList;
	Common::GetWADList(ens.pworld->pentdata, wadList);

	// List of textures with default names
	if(!g_defaultMaterialPMFList.empty())
		g_defaultMaterialPMFList.clear();

	// Go through all world textures
	for(Uint32 i = 0; i < ens.pworld->numtextures; i++)
	{
		mtexture_t* ptexture = &ens.pworld->ptextures[i];
		CString texname = ptexture->name;
		texname.tolower();

		// Find original PMF file
		en_material_t* pmaterial = nullptr;
		for(Uint32 j = 0; j < wadList.size(); j++)
		{
			CString folderPath = WAD_GetWADFolderPath(wadList[j].c_str(), WORLD_TEXTURES_PATH_BASE);
			CString materialPath = WAD_GetWADTexturePath(folderPath.c_str(), texname.c_str());

			pmaterial = pTextureManager->FindMaterialScript(materialPath.c_str(), RS_GAME_LEVEL);
			if(pmaterial)
				break;
		}

		if(!pmaterial)
		{
			Con_Printf("r_show_list_material - Unable to find material script for '%s'.\n", texname.c_str());
			continue;
		}

		// Do not override entries with non-default material types
		if(!pmaterial->materialname.empty() && qstrcicmp(pmaterial->materialname, "concrete"))
			continue;

		g_defaultMaterialPMFList.push_back(pmaterial->filepath);
	}

	// Write list of textures missing detail textures
	CString str;
	str << "World textures with default material types: " << NEWLINE;

	// Print to console also
	Con_Printf("World textures with default material types:\n");

	for(Uint32 i = 0; i < g_defaultMaterialPMFList.size(); i++)
	{
		// Print to file
		str << g_defaultMaterialPMFList[i] << NEWLINE;

		// Print to console also
		Con_Printf("%d - %s.\n", i, g_defaultMaterialPMFList[i].c_str());
	}

	// Write to file
	CString mapname;
	Common::Basename(ens.pworld->name.c_str(), mapname);

	CString filepath;
	filepath << "logs/" << mapname << "_default_material.log";

	const byte* pwritedata = reinterpret_cast<const byte*>(str.c_str());
	FL_WriteFile(pwritedata, str.length(), filepath.c_str());

	Con_Printf("Wrote list of textures with default material type to '%s'.\n", filepath.c_str());
}

//====================================
//
//====================================
void Cmd_SetTextureMaterialType( void )
{
	if(gCommands.Cmd_Argc() < 3)
	{
		Con_Printf("r_set_texture_material_type usage: r_set_texture_material_type <texture index in list generated by r_list_default_materials> <material type name>\n");
		return;
	}

	// Get index parameter
	Int32 index = SDL_atoi(gCommands.Cmd_Argv(1));
	if(index < 0 || index >= g_defaultMaterialPMFList.size())
	{
		Con_EPrintf("r_set_texture_material_type - Invalid index %d.\n", index);
		return;
	}

	// Get material type name
	const Char* pstrMaterialType = gCommands.Cmd_Argv(2);
	if(!pstrMaterialType)
	{
		Con_EPrintf("r_set_texture_material_type - No material type specified.\n");
		return;
	}

	// Material types list
	CArray<CString> materialTypesList;

	// Load material script
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(MATERIAL_TYPES_FILE_PATH, &filesize);
	if(!pfile)
	{
		Con_Printf("r_set_texture_material_type - Could not load '%s'.\n", MATERIAL_TYPES_FILE_PATH);
		return;
	}

	CString line;
	CString token;

	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		// Read the line
		pstr = Common::ReadLine(pstr, line);

		// Read first token
		const Char* plstr = Common::Parse(line.c_str(), token);
		if(!plstr)
		{
			Con_Printf("r_set_texture_material_type - Missing second token in '%s'.\n", MATERIAL_TYPES_FILE_PATH);
			continue;
		}

		// Discard first token and parse next
		Common::Parse(plstr, token);
		// Add to list of materials
		materialTypesList.push_back(token);
	}

	FL_FreeFile(pfile);

	Uint32 j = 0;
	for(; j < materialTypesList.size(); j++)
	{
		if(!qstrcicmp(materialTypesList[j], pstrMaterialType))
			break;
	}

	if(j == materialTypesList.size())
		Con_Printf("r_set_texture_material_type - Material type '%s' specified not found in '%s'.\n", pstrMaterialType, MATERIAL_TYPES_FILE_PATH);

	// Look up PMF file
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	assert(pTextureManager != nullptr);

	CString& str = g_defaultMaterialPMFList[index];

	// Find original PMF file
	en_material_t* pmaterial = pTextureManager->FindMaterialScript(str.c_str(), RS_GAME_LEVEL);

	if(!pmaterial)
	{
		Con_Printf("r_show_list_material - Unable to find material script for '%s'.\n", str.c_str());
		return;
	}

	// Set material type and write it out
	pmaterial->materialname = pstrMaterialType;
	pTextureManager->WritePMFFile(pmaterial);
	
	// Clear shown texture
	g_pMaterialShown = nullptr;
}

//====================================
//
//====================================
void Cmd_ShowListMaterial( void )
{
	if(gCommands.Cmd_Argc() < 2)
	{
		Con_Printf("r_show_list_material usage: r_show_list_material <texture index in list generated by r_list_default_materials or -1 for disable>\n");
		return;
	}

	// Get index parameter
	Int32 index = SDL_atoi(gCommands.Cmd_Argv(1));
	if(index != -1 && (index < 0 || index >= g_defaultMaterialPMFList.size()))
	{
		Con_EPrintf("r_show_list_material - Invalid index %d.\n", index);
		return;
	}

	if(index == -1)
	{
		g_pMaterialShown = nullptr;
		return;
	}

	// Look up PMF file
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	assert(pTextureManager != nullptr);

	CString& str = g_defaultMaterialPMFList[index];

	// Find original PMF file
	en_material_t* pmaterial = pTextureManager->FindMaterialScript(str.c_str(), RS_GAME_LEVEL);

	if(!pmaterial)
	{
		Con_Printf("r_show_list_material - Unable to find material script for '%s'.\n", str.c_str());
		return;
	}

	g_pMaterialShown = pmaterial;
}

//====================================
//
//====================================
void Cmd_LoadAllParticleScripts( void )
{
	CString searchpath;
	searchpath << ens.gamedir << PARTICLE_SCRIPT_PATH << "*.txt";

	// Parse directory for files
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if ((dir = FindFirstFile(searchpath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
	{
		printf("Directory %s not found.\n", searchpath.c_str());
		return;
	}

	while (true) 
	{
		if (qstrcmp(file_data.cFileName, ".") != 0 && qstrcmp(file_data.cFileName, "..") != 0 && qstrstr(file_data.cFileName, ".txt"))
		{
			CString path;
			path << PARTICLE_SCRIPT_PATH << file_data.cFileName;

			const byte* pf = FL_LoadFile(path.c_str());
			if(pf)
			{
				CString token;
				Common::Parse(reinterpret_cast<const Char*>(pf), token);

				if(!qstrcmp(token, "$particlescript"))
					gParticleEngine.PrecacheScript(PART_SCRIPT_SYSTEM, file_data.cFileName, false);
				else
					gParticleEngine.PrecacheScript(PART_SCRIPT_CLUSTER, file_data.cFileName, false);
			}

		}

		if(!FindNextFile(dir, &file_data))
			break;
	}
}

