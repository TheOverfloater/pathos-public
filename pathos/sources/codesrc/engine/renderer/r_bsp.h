/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_BSP_H
#define R_BSP_H

class CCVar;
class CGLSLShader;
class CVBO;

struct en_texture_t;
struct cl_dlight_t;
struct decalgroupentry_t;
struct cl_entity_t;
struct aldheader_t;
struct cubemapinfo_t;
struct miptex_t;

#include "com_math.h"
#include "ald.h"
#include "r_nsmoothing.h"
#include "r_glsl.h"
#include "r_main.h"

enum bsp_shaders_t
{
	shader_detailtex = 0,
	shader_nodetail,
	shader_chrome,
	shader_texunit0,
	shader_texunit1,
	shader_dynlight,
	shader_unused1,
	shader_spotlight,
	shader_unused2,
	shader_caustics,
	shader_lightalpha,
	shader_solidcolor,
	shader_vsm_store,
	shader_vsm_alpha,
	shader_fogpass,
	shader_fogpass_fc,
	shader_lightonly,
	shader_main_detail,
	shader_speconly,
	shader_cubeonly,
	shader_decal_holes,
	shader_decal,
	shader_texunit0_x4
};

enum bsp_fog_settings_t
{
	fog_none = 0,
	fog_radial,
	fog_fogcoord
};

struct light_attribs_t
{
	light_attribs_t():
		u_light_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_radius(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_cubemap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_projtexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_shadowmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_matrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_light_shadowmap(CGLSLShader::PROPERTY_UNAVAILABLE)
	{}

	Int32 u_light_color;
	Int32 u_light_origin;
	Int32 u_light_radius;
	Int32 u_light_cubemap;
	Int32 u_light_projtexture;
	Int32 u_light_shadowmap;
	Int32 u_light_matrix;

	Int32 d_light_shadowmap;
};

struct bsp_shader_attribs
{
	bsp_shader_attribs():
		d_shadertype(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fogtype(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_alphatest(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_bumpmapping(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_specular(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_cubemaps(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_luminance(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_numlights(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_position(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_tangent(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_binormal(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_normal(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_lmapcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_dtexcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_fogcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelmatrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_inv_modelmatrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmatrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_causticsm2(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_causticsm1(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_vorigin(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_vright(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_uvoffset(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_phong_exponent(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_specularfactor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_interpolant(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_cubemapstrength(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_decalalpha(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_decalscale(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_cubemap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_cubemap_prev(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_baselightmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_maintexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_detailtex(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_chrometex(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_luminance(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_difflightmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_lightvecstex(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_specular(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_radius(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE)
	{}

	Int32 d_shadertype;
	Int32 d_fogtype;
	Int32 d_alphatest;
	Int32 d_bumpmapping;
	Int32 d_specular;
	Int32 d_cubemaps;
	Int32 d_luminance;
	Int32 d_numlights;

	// vertex attribs
	Int32 a_position;
	Int32 a_tangent;
	Int32 a_binormal;
	Int32 a_normal;
	Int32 a_lmapcoord;
	Int32 a_texcoord;
	Int32 a_dtexcoord;
	Int32 a_fogcoord;

	// vertex shader uniforms
	Int32 u_projection;
	Int32 u_modelview;

	Int32 u_modelmatrix;
	Int32 u_inv_modelmatrix;

	Int32 u_normalmatrix;

	Int32 u_causticsm2;
	Int32 u_causticsm1;

	Int32 u_vorigin;
	Int32 u_vright;

	Int32 u_uvoffset;

	Int32 u_phong_exponent;
	Int32 u_specularfactor;
	Int32 u_interpolant;
	Int32 u_cubemapstrength;
	Int32 u_decalalpha;
	Int32 u_decalscale;

	// fragment shader uniforms
	Int32 u_cubemap;
	Int32 u_cubemap_prev;
	Int32 u_baselightmap;
	Int32 u_maintexture;
	Int32 u_detailtex;
	Int32 u_chrometex;
	Int32 u_normalmap;
	Int32 u_luminance;
	Int32 u_difflightmap;
	Int32 u_lightvecstex;
	Int32 u_specular;
	Int32 u_color;
	Int32 u_light_radius;

	Int32 u_fogcolor;
	Int32 u_fogparams;

	light_attribs_t lights[MAX_BATCH_LIGHTS];
};

struct bsp_vertex_t
{
	bsp_vertex_t():
		fogcoord(0)
	{
		memset(origin, 0, sizeof(origin));
		memset(lmapcoord, 0, sizeof(lmapcoord));
		memset(texcoord, 0, sizeof(texcoord));
		memset(dtexcoord, 0, sizeof(dtexcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin; // 16
	Vector tangent; // 28
	Vector binormal; // 40
	Vector normal; // 52

	Float lmapcoord[2]; // 60
	Float texcoord[2]; // 68
	Float dtexcoord[2]; // 76

	Float fogcoord; // 80

	byte padding[14]; // 96
};

struct drawbatch_t
{
	drawbatch_t():
		start_index(0),
		end_index(0)
	{
		memset(pad, 0, sizeof(pad));
	};
	drawbatch_t(Uint32 start, Uint32 end):
		start_index(start),
		end_index(end)
	{
		memset(pad, 0, sizeof(pad));
	};
	Uint32 start_index;
	Uint32 end_index;

	byte pad[24];
};

struct bsp_texture_t
{
	bsp_texture_t():
		flags(0),
		index(0),
		pmaterial(nullptr),
		nummultibatches(0),
		numsinglebatches(0),
		numlightbatches(0),
		psurfchain(nullptr),
		pmodeltexture(nullptr)
	{}

	Int32 flags;
	Int32 index;

	struct en_material_t* pmaterial;

	CArray<drawbatch_t> multi_batches;
	Uint32 nummultibatches;

	CArray<drawbatch_t> single_batches;
	Uint32 numsinglebatches;

	CArray<drawbatch_t> light_batches;
	Uint32 numlightbatches;

	struct msurface_t* psurfchain;

	struct mtexture_t* pmodeltexture;
};

struct bsp_surface_t
{
	bsp_surface_t():
		light_s(0),
		light_t(0),
		start_index(0),
		end_index(0),
		num_indexes(0),
		pmsurface(nullptr),
		ptexture(nullptr)
	{}

	// Mins
	Vector mins;
	// Maxs
	Vector maxs;

	// GL lightmap st coordinates
	Uint32 light_s;
	Uint32 light_t;

	// Start vertex index
	Uint32 start_index;
	// End vertex index
	Uint32 end_index;
	// Number of verts to draw
	Uint32 num_indexes;

	// brusmodel_t surface ptr
	msurface_t* pmsurface;
	// texture info
	bsp_texture_t* ptexture;
};

struct decalpolygroup_t
{
	decalpolygroup_t():
		start_vertex(0),
		num_vertexes(0),
		pentity(nullptr),
		ptexture(nullptr),
		radius(0),
		alphatest(false)
		{}

	Uint32 start_vertex;
	Uint32 num_vertexes;

	cl_entity_t *pentity;

	Vector localmins;
	Vector localmaxs;
	Vector localorigin;
	Float radius;

	en_texture_t* ptexture;
	bool alphatest;
};

struct bsp_decal_t 
{
	bsp_decal_t():
		ptexinfo(nullptr),
		life(0),
		fadetime(0),
		size(0),
		spawntime(0),
		growthtime(0)
		{}
	~bsp_decal_t()
	{
		if(!polygroups.empty())
		{
			for(Uint32 i = 0; i < polygroups.size(); i++)
				delete polygroups[i];
		}
	}

	CArray<decalpolygroup_t*> polygroups;

	const decalgroupentry_t *ptexinfo;

	Vector normal;
	Vector origin;

	CArray<Uint32> leafnums;

	Float life;
	Float fadetime;
	Float size;

	Double spawntime;
	Float growthtime;
};

/*
====================
CBSPRenderer

====================
*/
class CBSPRenderer
{
public:
	// Default lightmap width
	static const Uint32 LIGHTMAP_WIDTH;
	// Default lightmap height
	static const Uint32 LIGHTMAP_DEFAULT_HEIGHT;
	// Default decal vertex cache size
	static const Uint32 NB_BSP_DECAL_VERTS;
	// Backface epsilon value
	static const Float BACKFACE_EPSILON;
	// Max overlapping decals in a place
	static const Uint32 MAX_DECAL_OVERLAP;
	// Decal vertex allocation size
	static const Uint32 BSP_DECALVERT_ALLOC_SIZE;
	// Temporary decal vertex array allocation size
	static const Uint32 TEMP_DECAL_VERTEX_ALLOC_SIZE;
	// Specialfog distance
	static const Float SPECIALFOG_DISTANCE;

public:
	CBSPRenderer( void );
	~CBSPRenderer();

public:
	// Initializes the class
	bool Init( void );
	// Perform shutdown operations
	void Shutdown( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Clears OpenGL objects
	void ClearGL( void );

	// Prepares for the game
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );
	
public:
	// Draws non-transparent objects
	bool DrawNormal( void );
	// Draws transparent entities
	bool DrawTransparent( void );
	// Draws non-transparent decals
	bool DrawNormalDecals( void );
	// Draws objects for VSM renderpass
	bool DrawVSM( cl_dlight_t *dl, cl_entity_t** pvisents, Uint32 numentities, bool drawworld );
	// Draws skybox elements
	bool DrawSkyBox( bool inZElements );

	// Creates a decal
	void CreateDecal( const Vector& origin, const Vector& normal, decalgroupentry_t* pentry, byte flags, Float life, Float fadetime, Float growthtime );

	// Creates lightmaps
	void InitLightmaps( bool loadald );
	// Sets lightmap coords
	void SetLightmapCoords( void );
	// Initializes the main VBO
	void InitVBO( void );

	// Returns the lightmap height
	Uint32 GetLightmapHeight( void ) { return m_lightmapHeight; }

	// Performs think functions
	void Think( void );

private:
	// Loads textures
	void InitTextures( class CWADTextureResource& wadTextures, const CArray<CString>& wadFilesList );
	// Loads the chrome texture
	void LoadChromeTexture( void );
	// Loads in-memory textures
	void LoadTextures( void );
	// Loads a map texture
	en_material_t* LoadMapTexture( CWADTextureResource& wadTextures, const CArray<CString>& wadFilesList, const Char* pstrtexturename );

	// Draws the world and brush entities
	bool DrawWorld( void );
	// Renders any decals
	bool DrawDecals( bool transparents );
	
	// Traverses the world tree to collect renderable surfaces
	void RecursiveWorldNode( struct mnode_t* pnode );
	// Draws a brushmodel
	bool DrawBrushModel( cl_entity_t& entity, bool isstatic );

	// Tells if an object is affected by a dynamic light
	void FlagIfDynamicLighted( const Vector& mins, const Vector& maxs );
	// Retreives the texture animation for a texture
	mtexture_t *TextureAnimation( mtexture_t *pbase, Uint32 frame );
	// Binds textures for rendering in single-pass mode
	bool BindTextures( bsp_texture_t* phandle, cubemapinfo_t* pcubemapinfo, cubemapinfo_t* pprevcubemap, GLuint& cubemapUnit );
	// Calculates fog cordinate for special fog
	Float CalcFogCoord( Float z );

private:
	// Batches a surface
	__forceinline void BatchSurface( msurface_t* psurface );
	// Adds a render batch to the array
	__forceinline void AddBatch( CArray<drawbatch_t>& batches, Uint32& numbatches, bsp_surface_t *psurface );

private:
	// Prepares for rendering an object
	bool Prepare( void );

	// Renders first render pass
	bool DrawFirst( void );
	// Renders any lights
	bool DrawLights( bool specular );
	// Draws final renderpasses
	bool DrawFinal( void );

	// Prepares for VSM rendering
	void PrepareVSM( void );
	// Renders VSM faces
	bool DrawVSMFaces( void );
	// Batches a brushmodel for VSM
	bool BatchBrushModelForVSM( cl_entity_t& entity, bool isstatic );

	// Prepares a light for rendering
	bool SetupLight( cl_dlight_t* pdlight, Uint32 lightindex, Uint32& texunit, lightbatchtype_t type );
	// Finishes rendering of a light
	void FinishLight( cl_dlight_t* pdlight, Uint32& texunit );

private:
	// Draws a single decal
	bool DrawDecal( bsp_decal_t *pdecal, bool transparents, enum decal_rendermode_t& rendermode );

	// Retreives the vertex index offset for a decal
	Uint32 GetDecalOffset( Uint32 numverts );
	// Applies a decal to a surface
	void DecalSurface( const msurface_t *surf, bsp_decal_t *pdecal, const Vector& normal, const Vector& origin, bool transparent, en_texture_t* ptexture );
	// Recursively applies decals on the world surfaces
	void RecursivePasteDecal( struct mnode_t *node, bsp_decal_t *pdecal, byte flags, const Vector& mins, const Vector& maxs );
	// Deletes a decal from the chain
	void DeleteDecal( bsp_decal_t *pdecal );
	// Removes decal from VBO
	void RemoveDecalFromVBO( bsp_decal_t *pdelete );

private:
	// Current entity being rendered
	struct cl_entity_t* m_pCurrentEntity;

	// Array of allocated surfaces
	CArray<bsp_surface_t> m_surfacesArray;
	// Array of texture handles
	CArray<bsp_texture_t> m_texturesArray;

private:
	bool m_multiPass;
	bool m_addMulti;
	bool m_bumpMaps;

private:
	// Lightmap images
	Uint32 m_lightmapIndex;
	Uint32 m_ambientLightmapIndex;
	Uint32 m_diffuseLightmapIndex;
	Uint32 m_lightVectorsIndex;

	// Lightmap height
	Uint32 m_lightmapHeight;

	// Chrome texture
	en_texture_t* m_pChromeTexture;

private:
	// Vertex cache base index
	Uint32 m_vertexCacheBase;
	// Current index in the cache
	Uint32 m_vertexCacheIndex;
	// Cache size
	Uint32 m_vertexCacheSize;

	// Static decals array
	CArray<bsp_decal_t*> m_staticDecalsArray;
	// Linked list of non-static decals
	CLinkedList<bsp_decal_t*> m_decalsList;

	// Temporary decal vertexes array
	CArray<bsp_vertex_t> m_tempDecalVertsArray;

private:
	CCVar* m_pCvarDetailTextures;
	CCVar* m_pCvarDetailScale;
	CCVar* m_pCvarDrawWorld;
	CCVar* m_pCvarNormalBlendAngle;
	CCVar* m_pCvarLegacyTransparents;

private:
	// Shader object
	class CGLSLShader *m_pShader;
	// VBO object
	class CVBO *m_pVBO;

	// Shader attrib info
	bsp_shader_attribs m_attribs;
	// TRUE if cubemapping is supported
	bool m_isCubemappingSupported;
};
extern CBSPRenderer gBSPRenderer;
#endif