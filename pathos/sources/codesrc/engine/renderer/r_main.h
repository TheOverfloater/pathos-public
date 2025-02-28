/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_MAIN_H
#define R_MAIN_H

#include "matrix.h"
#include "ref_params.h"
#include "brushmodel.h"
#include "com_math.h"
#include "frustum.h"
#include "mlight.h"
#include "vid.h"
#include "r_fbo.h"

class Vector;
class CFrustum;
class CCVar;
class CGLExtF;
class CBasicDraw;
class CGLSLShader;

struct cl_dlight_t;
struct cl_entity_t;
struct en_texture_t;
struct r_interface_t;
struct font_set_t;
struct rtt_texture_t;
struct en_texalloc_t;
struct cache_model_t;
struct glowquery_t;
struct fbobind_t;

// Maximum textures bound at once
static constexpr Uint32 MAX_BOUND_TEXTURES = 16;
// Near clipping distance
static constexpr Float NEAR_CLIP_DISTANCE = 4.0f;
// Max number of model lights
static constexpr Uint32 MAX_MODEL_LIGHTS = 2048;
// Render pass id for main frame's render pass
static constexpr Uint32 MAINFRAME_RENDERPASS_ID = 0;
// Max lights in a single batch
static constexpr Uint32 MAX_BATCH_LIGHTS = 4;

extern CGLExtF gGLExtF;

enum alphatest_mode_t
{
	ALPHATEST_DISABLED = 0,
	ALPHATEST_LESSTHAN,
	ALPHATEST_COVERAGE
};

enum decal_rendermode_t
{
	DECAL_RENDERMODE_NONE = 0,
	DECAL_RENDERMODE_NORMAL,
	DECAL_RENDERMODE_ALPHATEST
};

struct texbind_states_t
{
	texbind_states_t()
	{
		memset(texturebinds_2d, 0, sizeof(texturebinds_2d));
		memset(texturebinds_cube, 0, sizeof(texturebinds_cube));
		memset(texturebinds_rect, 0, sizeof(texturebinds_rect));
	}

	Uint32 texturebinds_2d[MAX_BOUND_TEXTURES];
	Uint32 texturebinds_cube[MAX_BOUND_TEXTURES];
	Uint32 texturebinds_rect[MAX_BOUND_TEXTURES];
};

struct fog_settings_t
{
	fog_settings_t():
		start(0),
		end(0),
		blend(0),
		affectsky(false),
		active(false),
		entindex(0)
	{
	}

	Vector color;
	Float start;
	Float end;
	Float blend;

	bool affectsky;
	bool active;

	entindex_t entindex;
};

struct fog_states_t
{
	fog_states_t():
		blendtime(0),
		specialfog(false),
		prevspecialfog(false)
		{}

	fog_settings_t blend1;
	fog_settings_t blend2;

	fog_settings_t settings;

	Double blendtime;
	bool specialfog;
	bool prevspecialfog;
};

struct view_info_t
{
	view_info_t():
		znear(0),
		zfar(0),
		pviewleaf(nullptr),
		viewsize_x(0),
		viewsize_y(0),
		fov(0),
		nearclip(0),
		leafvisframe(0)
	{};

	// Matrices used for rendering
	CMatrix modelview;
	CMatrix projection;

	// Render params
	ref_params_t params;

	Vector v_origin;
	Vector v_angles;

	Vector v_forward;
	Vector v_right;
	Vector v_up;

	Vector v_visorigin;

	// clipping plane distances
	Float znear;
	Float zfar;

	// leaf the view is on
	const mleaf_t* pviewleaf;
	// culling frustum object
	CFrustum frustum;

	// viewport width
	Float viewsize_x;
	// viewport height
	Float viewsize_y;
	// Field of view
	Float fov;
	// Near clip distance
	Float nearclip;

	// Visframe of leaf we're on
	Int32 leafvisframe;
};

struct render_objects_t
{
	render_objects_t():
		numvisents(0),
		ploadinglogo(nullptr),
		ppausedlogo(nullptr),
		nummodellights(0)
	{
		for(Uint32 i = 0; i < MAX_RENDER_ENTITIES; i++)
		{
			pvisents_unsorted[i] = nullptr;
			pvisents[i] = nullptr;
		}
	}

	// Array of unsorted visents
	cl_entity_t* pvisents_unsorted[MAX_RENDER_ENTITIES];
	// Array of visents
	cl_entity_t* pvisents[MAX_RENDER_ENTITIES];
	// Number of visible entities
	Uint32 numvisents;

	// Loading text logo
	en_texture_t* ploadinglogo;
	// Paused text logo
	en_texture_t* ppausedlogo;

	// Array of projective textures
	CArray<en_texture_t*> projective_textures;
	// Array of caustics textures
	CArray<en_texture_t*> caustics_textures;

	// model lights array
	mlight_t modellights[MAX_MODEL_LIGHTS];
	// number of model lights
	Uint32 nummodellights;

	// Glow query array
	CArray<glowquery_t> glowqueryarray;
};

struct counters_t
{
	counters_t():
		brushpolies(0),
		modelpolies(0),
		particles(0),
		batches(0)
		{}

	// brush polycount
	Uint64 brushpolies;
	// model polies
	Uint64 modelpolies;
	// particles
	Uint64 particles;
	// draw batches
	Uint64 batches;
};

struct sky_settings_t
{
	sky_settings_t():
		drawsky(false),
		skybox(false),
		skysize(0)
		{
		}

	bool drawsky;
	bool skybox;
	Float skysize;

	Vector world_origin;
	Vector local_origin;

	fog_settings_t fog;
};

struct renderer_state_t
{
	renderer_state_t():
		renderflags(RENDERER_FL_NONE),
		framecount(0),
		framecount_main(0),
		visframe(0),
		frametime(0),
		time(0),
		screenwidth(0),
		screenheight(0),
		fboused(false),
		inwater(false),
		drawuiwhileloading(false),
		isbgrectangletexture(false),
		water_skydraw(false),
		mirroring(false),
		cubemapdraw(false),
		daystage(DAYSTAGE_NORMAL),
		monitorpass(false),
		portalpass(false),
		mainframe(false),
		isgameready(false),
		isdrawingloadingscreen(false),
		fatalerror(false),
		numskipframes(0),
		basicsinitialized(false),
		usevisorigin(false),
		validateshaders(false),
		msaa(false),
		fboblitsupported(false),
		hasdaystagedata(false),
		pvisbuffer(nullptr),
		psecondaryvisbuffer(nullptr),
		ploadbackground(nullptr),
		pbgrtttexture(nullptr),
		nextfreerenderpassidx(0),
		renderpassidx(0),
		pboundfbo(nullptr)
	{}
	~renderer_state_t()
	{
		if(pvisbuffer)
			delete[] pvisbuffer;

		if(psecondaryvisbuffer)
			delete[] psecondaryvisbuffer;
	}

	// Holds texture bind related info
	texbind_states_t textures;
	// Holds view related data like matrices, vectors
	view_info_t view;
	// Render objects
	render_objects_t objects;
	// Counters
	counters_t counters;
	// Fog settings
	fog_states_t fog;
	// Sky settings
	sky_settings_t sky;
	// Renderer flags
	Int32 renderflags;

	// Current framecount
	Uint64 framecount;
	// Framecount without renderpasses
	Uint64 framecount_main;
	// Visframe
	Int32 visframe;
	// Time since last draw
	Double frametime;
	// Current time
	Double time;

	// screen width
	Uint32 screenwidth;
	// screen height
	Uint32 screenheight;

	// Array of GL extensions
	CArray<CString> glextensions;

	// true if fbos are supported
	bool fboused;
	// true if we're underrendering water areas
	bool inwater;
	// true if we need to draw UI while loading
	bool drawuiwhileloading;
	// true if loading texture is a rectangle texture
	bool isbgrectangletexture;
	// true if we're drawing sky for water
	bool water_skydraw;
	// true if we're mirroring
	bool mirroring;
	// true if we're rendering cubemaps
	bool cubemapdraw;
	// true if rendering a monitor renderpass
	bool monitorpass;
	// true if we're rendering a portal renderpass
	bool portalpass;
	// true if we're rendering the main screen contents
	bool mainframe;
	// true if the renderer is ready to draw
	bool isgameready;
	// true if rendering loading screen
	bool isdrawingloadingscreen;
	// true if we've encountered a renderer error
	bool fatalerror;
	// number of frames to be skipped before we render
	int numskipframes;
	// true if basic GL stuff was initialized
	bool basicsinitialized;
	// true if v_visorigin should be used for setting VIS
	bool usevisorigin;
	// true if we need to run validation checks for shaders
	bool validateshaders;
	// true if using MSAA
	bool msaa;
	// true if fbo blit is supported
	bool fboblitsupported;
	// true if we have relevant daystage data
	bool hasdaystagedata;

	// day stage
	daystage_t daystage;

	// VIS buffer
	byte* pvisbuffer;
	// Secondary VIS buffer
	byte* psecondaryvisbuffer;

	// loading background texture
	en_texalloc_t* ploadbackground;
	// rtt texture for bg if any
	rtt_texture_t* pbgrtttexture;

	// Next free render pass id
	Uint32 nextfreerenderpassidx;
	// Current render pass id
	Uint32 renderpassidx;

	// Currently bound FBO
	fbobind_t* pboundfbo;
	// FBO used by main screen
	fbobind_t mainfbo;
};

enum querytype_t
{
	GL_QUERY_UNDEFINED = 0,
	GL_QUERY_SPRITES,
	GL_QUERY_LENSFLARES,
};

struct glowquery_t
{
	glowquery_t():
		key(NO_POSITION),
		type(GL_QUERY_UNDEFINED),
		queried(false),
		renderpassidx(0)
	{
	}

	CArray<GLuint> queries;
	Int32 key;
	querytype_t type;
	bool queried;

	Uint32 renderpassidx;
};

struct active_load_shader_t
{
	active_load_shader_t():
		pshader(nullptr),
		lastshaderindex(0)
		{}

	CGLSLShader* pshader;
	Int32 lastshaderindex;
};

enum lightbatchtype_t
{
	LB_TYPE_NONE = 0,
	LB_TYPE_POINTLIGHT,
	LB_TYPE_POINTLIGHT_SHADOW,
	LB_TYPE_SPOTLIGHT,
	LB_TYPE_SPOTLIGHT_SHADOW,
};

struct lightbatch_t
{
	lightbatch_t():
		type(LB_TYPE_NONE)
		{}

	lightbatchtype_t type;
	Vector mins;
	Vector maxs;

	CLinkedList<cl_dlight_t*> lightslist;
};

// For access across the engine
extern renderer_state_t rns;

extern CCVar* g_pCvarBumpMaps;
extern CCVar* g_pCvarDrawEntities;
extern CCVar* g_pCvarPhongExponent;
extern CCVar* g_pCvarWireFrame;
extern CCVar* g_pCvarSpecular;
extern CCVar* g_pCvarCaustics;
extern CCVar* g_pCvarFarZ;
extern CCVar* g_pCvarNoFBO;
extern CCVar* g_pCvarStats;
extern CCVar* g_pCvarShadows;
extern CCVar* g_pCvarDynamicLights;
extern CCVar* g_pCvarGaussianBlur;
extern CCVar* g_pCvarCubemaps;
extern CCVar* g_pCvarAnisotropy;
extern CCVar* g_pCvarWadTextureChecks;
extern CCVar* g_pCvarBspTextureChecks;
extern CCVar* g_pCvarGLSLOnDemand;
extern CCVar* g_pCvarGLSLActiveLoad;
extern CCVar* g_pCvarHighlightEntity;
extern CCVar* g_pCvarGLSLValidate;
extern CCVar* g_pCvarSkipFrames;
extern CCVar* g_pCvarOcclusionQueries;
extern CCVar* g_pCvarTraceGlow;
extern CCVar* g_pCvarBatchDynamicLights;
extern CCVar* g_pCvarOverdarkenTreshold;
extern CCVar* g_pCvarDumpLightmaps;

extern void R_InitRenderInterface( r_interface_t &renderFuncs );

extern void R_Bind2DTexture( Int32 texture, Uint32 id, bool force = false );
extern void R_BindRectangleTexture( Int32 texture, Uint32 id, bool force = false );
extern void R_BindCubemapTexture( Int32 texture, Uint32 id, bool force = false );
extern void R_ClearBinds( Uint32 firstUnit = 0 );

extern bool R_DrawScene( void );
extern bool R_DrawInterface( void );
extern bool R_Draw( const ref_params_t& params );
extern bool R_DrawLoadingScreen( const Char* pstrText = nullptr );
extern bool R_DrawPausedLogo( void );
extern bool R_DrawHUD( bool hudOnly = false, bool noFilmGrain = false );
extern bool R_DrawShownMaterial( void );

extern bool R_Init( void );
extern void R_Shutdown( void );

extern bool R_InitGL( void );
extern void R_ShutdownGL( void );
extern bool R_Update( void );
extern void R_ClearFog( void );

extern bool R_InitGame( void );
extern bool R_LoadResources( void );
extern void R_ResetGame( void );
extern void R_MarkLeaves( const Vector& origin );
extern void R_ForceMarkLeaves( const mleaf_t* pleaf, Int32 visframe, byte *pvsdata = nullptr );
extern void R_SetupView( const ref_params_t& params );

extern void R_AddEntity( cl_entity_t* pentity );
extern bool R_AddTempEntity( cl_entity_t *entity );
extern void R_SetFrustum( CFrustum& frustum, const Vector& origin, const Vector& angles, Float fov, Float viewsize_x, Float viewsize_y, bool fogCull );
extern void R_SetModelViewMatrix( const Vector& origin, const Vector& angles );
extern void R_SetProjectionMatrix( Float znear, Float fovY );
extern void R_BindFBO( struct fbobind_t *pfbo );

extern inline void R_ValidateShader( CGLSLShader* pShader );
extern inline void R_ValidateShader( CBasicDraw* pDraw );

extern bool R_LoadTextureListFile( const Char* pstrTextureListFile, CArray<en_texture_t*>& texArray, rs_level_t level, const GLint* pborder, bool clamp );

extern bool R_PrintCounters( void );
extern void R_ResetFrameStates( void );

extern bool R_DrawString( color32_t color, Int32 x, Int32 y, const Char* pstrString, const font_set_t* pfont );
extern bool R_DrawStringBox( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int16 insetx, Int16 insety, bool reverse, color32_t color, Int32 x, Int32 y, const Char* pstrString, const font_set_t* pfont, Uint32 lineoffset, Uint32 minlineheight, Uint32 xoffset );
extern bool R_DrawTimeGraph( Double& time1, Double& time2 );
extern bool R_DrawFPSGraph( void );

extern bool R_BeginTextRendering( const font_set_t* pfontset );
extern void R_FinishTextRendering( const font_set_t* pfontset );
extern bool R_DrawCharacter( const font_set_t* pfontset, Int32 x, Int32 y, Char character, Uint32 r, Uint32 g, Uint32 b, Uint32 a );

extern void R_PopulateExtensionsArray( void );
extern bool R_IsExtensionSupported( const Char *pstrextension );

extern bool R_DrawOrigins( void );
extern bool R_DrawViewModelParticles( void );
extern Float R_GetRenderFOV( Float viewsize );
extern void R_AnisotropyCvarCallBack( CCVar* pCVar );
extern void R_ActiveLoadMaxShadersCvarCallBack( CCVar* pCVar );

extern bool R_IsSpecialRenderEntity( const cl_entity_t& entity );
extern void R_LoadSprite( cache_model_t* pmodel );

extern Float R_RenderFxBlend( cl_entity_t* pentity );

extern Int32 R_SortEntities( const void* p1, const void* p2 );

extern void R_AddShaderForLoading( CGLSLShader* pShader );
extern bool R_PerformPendingShaderLoads( void );
extern Vector R_GetLightingForPosition( const Vector& position, const Vector& defaultcolor );

extern void R_SetLightmapTexture( Uint32 glindex, Uint32 width, Uint32 height, bool isvectormap, color32_t* pdata, Uint32& resultsize );

extern void Cmd_PasteDecal( void );
extern void Cmd_CreateSprite( void );
extern void Cmd_CreateDynamicLight( void );
extern void Cmd_CreateSpotLight( void );
extern void Cmd_LoadModel( void );

extern void Cmd_EFX_TempSprite( void );
extern void Cmd_EFX_TempModel( void );
extern void Cmd_EFX_FunnelSprite( void );
extern void Cmd_EFX_SphereModel( void );
extern void Cmd_EFX_BreakModel( void );
extern void Cmd_EFX_BubbleTrail( void );
extern void Cmd_EFX_Bubbles( void );

extern void Cmd_EFX_ParticleExplosion1( void );
extern void Cmd_EFX_ParticleExplosion2( void );
extern void Cmd_EFX_BlobExplosion( void );
extern void Cmd_EFX_CreateRocketExplosion( void );
extern void Cmd_EFX_CreateParticleEffect( void );
extern void Cmd_EFX_CreateLavaSplash( void );
extern void Cmd_EFX_CreateTeleportSplash( void );
extern void Cmd_EFX_CreateRocketTrail( void );

extern void Cmd_EFX_BeamSetStart( void );
extern void Cmd_EFX_BeamSetEnd( void );
extern void Cmd_EFX_BeamLightning( void );
extern void Cmd_EFX_BeamCirclePoints( void );
extern void Cmd_EFX_BeamEntityPoint( void );
extern void Cmd_EFX_BeamEntities( void );
extern void Cmd_EFX_BeamFollow( void );
extern void Cmd_EFX_BeamVaporTrail( void );
extern void Cmd_EFX_BeamPoints( void );
extern void Cmd_EFX_BeamRing( void );
extern void Cmd_EFX_CreateParticle( void );
extern void Cmd_EFX_CreateTracer( void );

extern void Cmd_BSPToSMD_Lightmap( void );
extern void Cmd_BSPToSMD_Textures( void );

extern void Cmd_TimeRefresh( void );
extern void Cmd_DetailAuto( void );
extern void Cmd_ListDefaultMaterials( void );
extern void Cmd_SetTextureMaterialType( void );
extern void Cmd_ShowListMaterial( void );

extern void Cmd_LoadAllParticleScripts( void );
extern void Cmd_ExportALD( void );

#endif //R_MAIN_H