/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_VBM_H
#define R_VBM_H

#include "entity_extrainfo.h"
#include "mlight.h"
#include "vbm_shared.h"
#include "r_glsl.h"
#include "r_main.h"
#include "r_fbocache.h"

// Notes:
// Part of this implementation is based on the implementation in the Half-Life SDK
// The studiomodel format is Valve's original work, and I take no ownership of it
// No copyright infringement intended

struct decalgroupentry_t;
struct cache_model_t;
struct entity_extrainfo_t;

// Max dynamic lights affecting a model entity
static constexpr Uint32 MAX_ENT_ACTIVE_DLIGHTS		= 4;
// Max dynamic lights affecting a model entity
static constexpr Uint32 MAX_ENT_DLIGHTS				= 12;
// Max studio decals total
static constexpr Uint32 MAX_VBM_TOTAL_DECALS		= 512;
// Max temporary indexes for decal creation
static constexpr Uint32 MAX_TEMP_VBM_INDEXES		= 16384;
// Max temporary vertexes for special render stuff
static constexpr Uint32 MAX_TEMP_VBM_VERTEXES		= MAXSTUDIOBONES*2;
// Maximum submodels rendered at once
static constexpr Uint32 MAX_VBM_RENDERED_SUBMODELS	= 64;

enum vbm_shtype
{
	vbm_texture = 0,
	vbm_texture_fog,
	vbm_notexture,
	vbm_dynlight,
	vbm_unused1,
	vbm_spotlight,
	vbm_unused2,
	vbm_caustics,
	vbm_solid,
	vbm_vsm,
	vbm_vsmalpha,
	vbm_fogpass,
	vbm_texonly,
	vbm_scope,
	vbm_texonly_fog,
	vbm_speconly,
	vbm_texonly_holes,
	vbm_texonly_holes_fog
};

struct vbm_decal_mesh_t
{
	vbm_decal_mesh_t():
		start_index(0),
		num_indexes(0),
		pbones(nullptr),
		numbones(0),
		alphatest(false),
		ptexture(nullptr)
	{
	}
	~vbm_decal_mesh_t()
	{
		if(pbones)
			delete[] pbones;
	}

	Uint32 start_index;
	Uint32 num_indexes;

	byte *pbones;
	Int32 numbones;

	bool alphatest;
	en_texture_t* ptexture;
};

struct vbmdecal_t
{
	vbmdecal_t():
		entindex(NO_ENTITY_INDEX),
		pentity(nullptr),
		identifier(0),
		start_vertex(0),
		num_vertexes(0),
		pentry(nullptr),
		totaldecals(0),
		next(nullptr),
		prev(nullptr)
	{
	}
	~vbmdecal_t()
	{
		for(Uint32 i = 0; i < meshes.size(); i++)
			delete meshes[i];
	}

	Int32 entindex;
	cl_entity_t *pentity;
	Uint32 identifier;

	Uint32 start_vertex;
	Uint32 num_vertexes;

	CArray<vbm_decal_mesh_t*> meshes;

	const decalgroupentry_t *pentry;

	Uint32 totaldecals;
	vbmdecal_t *next;
	vbmdecal_t *prev;
};

struct vbm_glvertex_t
{
	vbm_glvertex_t()
	{
		memset(texcoord1, 0, sizeof(texcoord1));
		memset(texcoord2, 0, sizeof(texcoord2));
		memset(boneindexes, 0, sizeof(boneindexes));
		memset(boneweights, 0, sizeof(boneweights));
		memset(flexcoord, 0, sizeof(flexcoord));
		memset(pad, 0, sizeof(pad));
	}
		
	Vector origin;
	Vector tangent;
	Vector normal;
	Float texcoord1[2];
	Float texcoord2[2];
	Float boneindexes[MAX_VBM_BONEWEIGHTS];
	Float boneweights[MAX_VBM_BONEWEIGHTS];
	Float flexcoord[2];
	byte pad[4];
};

struct ubo_modellight_t
{
	ubo_modellight_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(color, 0, sizeof(color));
		memset(radius, 0, sizeof(radius));
	}

	Float origin[4];
	Float color[4];
	Float radius[4];
};

struct attrib_light
{
	attrib_light():
		u_color(0),
		u_origin(0),
		u_radius(0)
		{}

	Int32 u_color;
	Int32 u_origin;
	Int32 u_radius;
};

struct vbm_dlight_attribs_t
{
	vbm_dlight_attribs_t():
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

struct vbm_attribs
{
	vbm_attribs():
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_tangent(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_normal(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord1(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord2(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_boneindexes(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_boneweights(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_flexcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmatrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_flextexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_flextexturesize(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_caustics_interp(CGLSLShader::PROPERTY_UNAVAILABLE),
		ub_bonematrices(CGLSLShader::PROPERTY_UNAVAILABLE),
		ub_modellights(CGLSLShader::PROPERTY_UNAVAILABLE),
		ub_vsmatrices(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_vorigin(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_vright(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_causticsm1(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_causticsm2(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scroll(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture0(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture1(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_rectangle(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_spectexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_lumtexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_aotexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_sky_ambient(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_sky_diffuse(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_sky_dir(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_light_radius(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scope_scale(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scope_scrsize(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_phong_exponent(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_specularfactor(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_numlights(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_shadertype(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_chrome(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_alphatest(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_flexes(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_specular(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_luminance(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_ao(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_bumpmapping(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_numdlights(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_use_ubo(CGLSLShader::PROPERTY_UNAVAILABLE)
		{
			for(Uint32 i = 0; i < MAX_SHADER_BONES; i++)
				boneindexes[i] = 0;
		}

	Int32 a_origin;
	Int32 a_tangent;
	Int32 a_normal;
	Int32 a_texcoord1;
	Int32 a_texcoord2;
	Int32 a_boneindexes;
	Int32 a_boneweights;
	Int32 a_flexcoord;

	Int32 u_projection;
	Int32 u_modelview;
	
	Int32 u_normalmatrix;

	Int32 u_flextexture;
	Int32 u_flextexturesize;

	Int32 u_caustics_interp;

	Int32 ub_bonematrices;
	Int32 ub_modellights;
	Int32 ub_vsmatrices;

	Int32 boneindexes[MAX_SHADER_BONES];

	Int32 u_vorigin;
	Int32 u_vright;

	Int32 u_causticsm1;
	Int32 u_causticsm2;

	Int32 u_scroll;

	Int32 u_texture0;
	Int32 u_texture1;
	Int32 u_rectangle;
	Int32 u_spectexture;
	Int32 u_lumtexture;
	Int32 u_aotexture;
	Int32 u_normalmap;

	Int32 u_sky_ambient;
	Int32 u_sky_diffuse;
	Int32 u_sky_dir;

	Int32 u_light_radius;

	Int32 u_fogcolor;
	Int32 u_fogparams;

	Int32 u_color;

	Int32 u_scope_scale;
	Int32 u_scope_scrsize;

	Int32 u_phong_exponent;
	Int32 u_specularfactor;

	attrib_light lights[MAX_ENT_MLIGHTS];
	Int32 d_numlights;

	Int32 d_shadertype;
	Int32 d_chrome;
	Int32 d_alphatest;
	Int32 d_flexes;
	Int32 d_specular;
	Int32 d_luminance;
	Int32 d_ao;
	Int32 d_bumpmapping;
	Int32 d_numdlights;
	Int32 d_use_ubo;
	
	vbm_dlight_attribs_t dlights[MAX_BATCH_LIGHTS];
};

/*
=================================
-Class: CVBMRenderer
-Description:

=================================
*/
class CVBMRenderer
{
public:
	// Number of light reductions
	static const Uint32 NUM_LIGHT_REDUCTIONS;
	// Time it takes to interpolate lighting value changes
	static const Float LIGHTING_LERP_TIME;

	// Max decals on a single model entity
	static const Uint32 MAX_VBM_ENTITY_DECALS;

	// Minimum array size for vbm model vertexes
	static const Uint32 MIN_VBMDECAL_VERTEXES;

	// Eyeglint texture path
	static const Char EYEGLINT_TEXTURE_PATH[];

	// Default lightmap sampling offset
	static const Float DEFAULT_LIGHTMAP_SAMPLE_OFFSET;

	enum vs_matrices_t
	{
		VS_MATRIX_PROJECTION = 0,
		VS_MATRIX_MODELVIEW,
		VS_MATRIX_NORMALMATRIX,

		NB_VS_MATRICES
	};

public:
	CVBMRenderer( void );
	~CVBMRenderer( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

public:
	// Initializes OpenGL objects
	bool InitGL( void );
	// Clears OpenGL objects
	void ClearGL( void );

public:
	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

public:
	// Draws a model
	bool DrawModel( Int32 flags, cl_entity_t* pentity );
	// Updates attachments on a model
	void UpdateAttachments( cl_entity_t *pEntity );
	// Updates bone positions and gives back a particular bone's position
	bool GetBonePosition( cl_entity_t *pEntity, const Char *szname, Vector& origin );
	// Transforms a vector by a bone matrix
	void TransformVectorByBoneMatrix( cl_entity_t *pEntity, Int32 boneindex, Vector& vector, bool inverse );
	// Rotates a vector by a bone matrix
	void RotateVectorByBoneMatrix( cl_entity_t *pEntity, Int32 boneindex, Vector& vector, bool inverse );

	// Prepares for rendering a model
	bool PrepareDraw( void );
	// Resets renderer objects after drawing
	void EndDraw( void );
	// Draws non-blended objects
	bool DrawNormal( void );
	// Draws blended objects
	bool DrawTransparent( void );
	// Draws skybox objects
	bool DrawSky( void );

	// Draws VSM objects
	bool DrawVSM( struct cl_dlight_t *dl, cl_entity_t** pvisents, Uint32 numentities );
	// Triggers clientside events
	void PlayEvents( void );

	// Prepares for aura rendering
	bool PrepAuraPass( void );
	// Ends aura rendering
	void FinishAuraPass( void );
	// Draws a model for aura rendering
	bool DrawAura( cl_entity_t *pEntity, const Vector& color, Float alpha );

	// Prepares for VSM rendering
	bool PrepareVSM( cl_dlight_t *dl );
	// Ends VSM rendering
	void EndVSM( void );
	// Draws model for VSM rendering
	bool DrawModelVSM( cl_entity_t *pEntity, cl_dlight_t *dl );

public:
	// Applies a decal to a model
	void CreateDecal( const Vector& position, const Vector& normal, decalgroupentry_t *texptr, cl_entity_t *pEntity, byte flags );
	// Releases entity VBM data
	void FreeEntityData( const cl_entity_t* pEntity );

	// Returns the flex manager pointer
	class CFlexManager* GetFlexManager( void ) { return m_pFlexManager; }

	// Get the shader error string
	const Char* GetShaderErrorString( void ) const;

private:
	// Deletes all decals
	void DeleteDecals( void );
	// Sets orientation-related data
	void SetOrientation( void );
	// Sets up the transformation matrix
	void SetupTransformationMatrix( void );

	// Applies any renderfx effects
	void ApplyRenderFX( Float (*pmatrix)[4] );

	// Sets up bones
	bool SetupBones( Int32 flags );
	// Set extrainfo data
	void SetExtraInfo( void );
	// Tells if the model should re-calculate bones
	bool ShouldAnimate( void );

	// Calculates attachment positions
	void CalculateAttachments( void );
	// Updates currently fetched light values
	void UpdateLightValues( void );
	// Sets up model lighting
	void SetupLighting( void );
	// Compare light values with light info
	bool CompareLightValues( Vector* pambientlightvalues, Vector* pdiffuselightvalues, const Vector& lightdir, entity_lightinfo_t& lightinfo );

	// Gets model lights
	void GetModelLights( void );
	// Gets dynamic lights
	void GetDynamicLights( void );

	// Tells if the entity is visible
	bool CheckBBox( void );

	// Triggers client-side events
	void DispatchClientEvents( void );

private:
	// Calls main render routines
	bool Render( void );
	// Sets up rendering routines
	bool SetupRenderer( void );
	// Restores rendering states
	bool RestoreRenderer( void );

	// Retreives the model for rendering
	bool SetModel( void );

	// Draws first pass 
	bool DrawFirst( void );
	// Draws a mesh
	bool DrawMesh( en_material_t *pmaterial, const vbmmesh_t *pmesh, bool drawBlended );
	// Draws lights
	bool DrawLights( bool specularPass );
	// Draws final renderpasses
	bool DrawFinal( void );
	// Draws in wireframe mode
	bool DrawWireframe( void );
	// Draws bones
	bool DrawBones( void );
	// Draws hitboxes
	bool DrawHitBoxes( void );
	// Draws the bbox
	bool DrawBoundingBox( void );
	// Draws the collision hull bbox
	bool DrawHullBoundingBox( void );
	// Draws decals
	bool DrawDecals( void );
	// Draws light vectors
	bool DrawLightVectors( void );
	// Draws attachments
	bool DrawAttachments( void );

	// Draws a box
	void DrawBox( const Vector& bbmin, const Vector& bbmax );
	// Draws a line
	void DrawLine( const Vector& start, const Vector& end, const Vector& color );
	// Batches a vertex for rendering
	void BatchVertex( const Vector& origin );

	// Sets up flexes for flex texture
	void CalculateFlexesHW( const vbmsubmodel_t* psubmodel );
	// Sets up flexes for VBO upload
	void CalculateFlexesSW( const vbmsubmodel_t* psubmodel );

	// Draws collected submodels
	bool DrawNormalSubmodels( void );
	// Draws submodels with flexes on them
	bool DrawFlexedSubmodels( void );

	// Sets up submodels
	void SetupModel( Uint32 bodypart, vbmlod_type_t type );
	// Retreives the ideal LOD for the type specified
	const vbmsubmodel_t* GetIdealLOD( const vbmsubmodel_t* psubmodel, vbmlod_type_t type );

	// Initializes the vertex texture
	void CreateVertexTexture( void );

	// Set bone UBO contents
	void SetShaderBoneTransform( Float (*pbonetransform)[MAXSTUDIOBONES][3][4], const byte* pboneindexes, Uint32 numbones );

private:
	// Allocates a decal slot
	vbmdecal_t* AllocDecalSlot( void );
	// Allocates a decal
	vbmdecal_t* AllocDecal( void );
	// Finalizes a decal mesh
	void FinalizeDecalMesh( vbmdecal_t* pdecal, vbm_decal_mesh_t* pmesh, Uint32& curstart );
	// Applies a decal on a triangle
	bool DecalTriangle( vbmdecal_t* pdecal, vbm_decal_mesh_t*& pmesh, const vbmvertex_t **pverts, const byte *pboneids, const Vector& position, const Vector& normal, vbmdecal_t *decal, const Vector& up, const Vector& right, Uint32& curstart, byte flags, en_material_t* pmaterial );
	// Deletes a decal
	static void DeleteDecal( vbmdecal_t *pdecal );
	// Retreives the offset for the decal mesh
	void GetDecalOffsets( Uint32 numverts, Uint32 numindexes, Uint32& vertexoffset, Uint32& indexoffset );

private:
	// Builds the VBO
	void BuildVBO( void );
	// Adds a VBM file to the VBO object
	void AddVBM( studiohdr_t *phdr, vbmheader_t *pvbm );

private:
	// Toggles rendering of models
	CCVar* m_pCvarDrawModels;
	// Toggles rendering of model decals
	CCVar* m_pCvarDrawModelDecals;
	// Toggles use of vertex textures
	CCVar* m_pCvarVertexTextures;
	// Controls the size of the vbm decal vertex cache
	CCVar* m_pCvarDecalCacheSize;
	// Controls whether we use skylight
	CCVar* m_pCvarSkyLighting;
	// Lightmap sampling offset
	CCVar* m_pCvarSampleOffset;
	// Controls whether we can use bump data for model lighting
	CCVar* m_pCvarUseBumpData;
	// Lighting ratio used for non-bump lightdata fetches
	CCVar* m_pCvarLightRatio;

private:
	// GLSL shader object
	class CGLSLShader* m_pShader;
	// VBO object
	class CVBO* m_pVBO;

	// Shader attribs
	vbm_attribs m_attribs;

	// Draw buffer offset
	Uint32 m_drawBufferIndex;

	// Flex texture allocation
	struct en_texalloc_t* m_pFlexTexture;
	// Screen texture pointer
	struct rtt_texture_t* m_pScreenTexture;
	// Screen FBO pointer
	CFBOCache::cache_fbo_t* m_pScreenFBO;

private:
	// Currently rendered VBM submodel
	const vbmsubmodel_t *m_pVBMSubModel;

	// Decal index cache size
	Uint32 m_decalIndexCacheSize;
	// Decal vertex cache size
	Uint32 m_decalVertexCacheSize;

	// Pointer to rotation matrix used
	Float (*m_pRotationMatrix)[3][4];
	// Internal rotation matrix
	Float m_pInternalRotationMatrix[3][4];

	// Pointer to bone matrix array
	Float (*m_pBoneTransform)[MAXSTUDIOBONES][3][4];
	// Pointer to bone weight transformation bone matrix array
	Float (*m_pWeightBoneTransform)[MAXSTUDIOBONES][3][4];
	// Internal bone matrix array
	Float m_pInternalBoneTransform[MAXSTUDIOBONES][3][4];
	// Internal bone array matrix used for weighted transformation
	Float m_pInternalWeightBoneTransform[MAXSTUDIOBONES][3][4];

	// Model's render angles
	Vector m_renderAngles;
	// Model's render origin
	Vector m_renderOrigin;

	// Eye glint texture
	struct en_texture_t *m_pGlintTexture;

private:
	// Model cache pointer for entity
	cache_model_t *m_pCacheModel;
	// Current rendered entity
	cl_entity_t *m_pCurrentEntity;

	// Studio data pointer
	studiohdr_t *m_pStudioHeader;
	// VBM data pointer
	vbmheader_t *m_pVBMHeader;
	// Pointer to entity extradata
	entity_extrainfo_t *m_pExtraInfo;
	// Entity alpha used for rendering
	Float m_renderAlpha;

private:
	// Entity absolute mins
	Vector m_mins;
	// Entity absolute maxs
	Vector m_maxs;

	// Tells if UBOs are supported
	bool m_areUBOsSupported;
	// Tells if vertex texture fetch is supported
	bool m_isVertexFetchSupported;
	// Tells if flexes are used
	bool m_useFlexes;

	// TRUE if we are blendig
	bool m_useBlending;

private:
	// Lighting information for entity
	entity_lightinfo_t m_lightingInfo;

	// Model lights array
	mlightinfo_t m_modelLights[MAX_ENT_MLIGHTS];
	// Number of model lights
	Uint32 m_numModelLights;

	// Pointer array of dynamic lights
	cl_dlight_t *m_pDynamicLights[MAX_ENT_DLIGHTS];
	// Number of dynamic lights
	Uint32 m_numDynamicLights;

	// TRUE if we're rendering in multipass
	bool m_isMultiPass;
	// TRUE if we're rendering an aura pass
	bool m_isAuraPass;

private:
	// List of submodels to render
	const vbmsubmodel_t *m_pSubmodelDrawList[MAX_VBM_RENDERED_SUBMODELS];
	// Amount of batched submodels
	Uint32 m_numDrawSubmodels;

	// Array of vbm entity decals
	vbmdecal_t m_vbmDecals[MAX_VBM_TOTAL_DECALS];
	// Number of decals used by VBM entities
	Uint32 m_numVBMDecals;

	// Temporary indexes array for decal creation
	Uint32 m_tempIndexes[MAX_TEMP_VBM_INDEXES];
	// Number of temporary indexes
	Uint32 m_numTempIndexes;

	// Temporary vertexes for decal creation
	vbm_glvertex_t m_tempVertexes[MAX_TEMP_VBM_VERTEXES];
	// Number of temporary vertexes in array
	Uint32 m_numTempVertexes;

	// Current index for decal vertex allocation
	Uint32 m_vCache_Index;
	// Base index for vertex cache
	Uint32 m_vCache_Base;

	// Current index for index cache
	Uint32 m_iCache_Index;
	// Base index for index cache
	Uint32 m_iCache_Base;

private:
	// Flex manager instance
	class CFlexManager* m_pFlexManager;

	// Flex texture texels
	vec4_t m_flexTexels[VBM_FLEXTEXTURE_SIZE*VBM_FLEXTEXTURE_SIZE];

private:
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions1[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions1[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions2[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions2[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions3[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions3[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions4[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions4[MAXSTUDIOBONES];
	// Quaternion and vector arrays used for bone transforms
	Vector	m_bonePositions5[MAXSTUDIOBONES];
	vec4_t	m_boneQuaternions5[MAXSTUDIOBONES];
	// Used for bone transform calculations
	Float	m_boneMatrix[3][4];

private:
	// Used for uploading modellight data to the modellight UBO
	ubo_modellight_t m_uboModelLightData[MAX_ENT_MLIGHTS];
	// Used for uploading to the bonematrices UBO
	vec4_t	m_uboBoneMatrixData[MAX_SHADER_BONES][3];
	// Used for uploading matrices to the vs_matrices ubo
	Float m_uboMatricesData[NB_VS_MATRICES][16];
};
extern CVBMRenderer gVBMRenderer;
extern en_material_t* VBM_FindMaterialScriptByIndex( Int32 index );
#endif//R_VBM_H