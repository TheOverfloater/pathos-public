/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_WATER_H
#define R_WATER_H

#include "r_fbo.h"
#include "ref_params.h"

struct cl_entity_t;
struct ref_params_t;
struct en_texture_t;

struct cl_water_t
{
	cl_water_t():
		index(0),
		pentity(nullptr),
		prefractfbo(nullptr),
		prefract_texture(nullptr),
		preflectfbo(nullptr),
		preflect_texture(nullptr),
		framecount(0),
		start_index(0),
		num_indexes(0),
		renderpassidx(0),
		plightmap_texture(nullptr),
		plightmap_diffuse_texture(nullptr),
		plightmap_lightvecs_texture(nullptr),
		lightmaptexturewidth(0),
		lightmaptextureheight(0),
		settingsindex(0)
		{}

	Uint32 index;
	cl_entity_t *pentity;

	Vector mins;
	Vector maxs;
	Vector origin;

	fbobind_t* prefractfbo;
	en_texalloc_t* prefract_texture;

	fbobind_t* preflectfbo;
	en_texalloc_t* preflect_texture;

	Uint32 framecount;

	Uint32 start_index;
	Uint32 num_indexes;

	Uint32 renderpassidx;

	en_texalloc_t* plightmap_texture;
	en_texalloc_t* plightmap_diffuse_texture;
	en_texalloc_t* plightmap_lightvecs_texture;

	Uint32 lightmaptexturewidth;
	Uint32 lightmaptextureheight;

	Int32 settingsindex;
};

struct water_settings_t
{
	water_settings_t():
		causticscale(0),
		causticstrength(0),
		causticstimescale(0),
		fresnel(0),
		scrollu(0),
		scrollv(0),
		strength(0),
		timescale(0),
		texscale(0),
		lightstrength(0),
		specularstrength(0),
		wavefresnelstrength(0),
		phongexponent(0),
		refractonly(false),
		cheaprefraction(false),
		pnormalmap(nullptr)
		{}

	fog_settings_t fogparams;
	Float causticscale;
	Float causticstrength;
	Float causticstimescale;
	Float fresnel;
	Float scrollu;
	Float scrollv;
	Float strength;
	Float timescale;
	Float texscale;
	Float lightstrength;
	Float specularstrength;
	Float wavefresnelstrength;
	Float flowmapspeed;
	Float phongexponent;
	bool refractonly;
	bool cheaprefraction;
	
	CString flowmappath;
	en_texture_t* pnormalmap;
	en_texture_t* pflowmap;
};

struct water_vertex_t
{
	water_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(texcoords, 0, sizeof(texcoords));
		memset(lightcoords, 0, sizeof(lightcoords));
		memset(pad, 0, sizeof(pad));
	}

	vec4_t origin;
	Vector normal;
	Vector tangent;
	Vector binormal;

	Float texcoords[2];
	Float lightcoords[2];

	byte pad[28];
};

struct water_attribs
{
	water_attribs():
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_normal(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_tangent(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_binormal(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoords(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_lightcoords(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_flowmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_lightmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_refract(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_reflect(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_rectrefract(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_diffusemap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_lightvecsmap(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmatrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_normalmatrix_v(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fresnel(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_time(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scroll(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_strength(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texscale(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_rectscale(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_lightstrength(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_specularstrength(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_phongexponent(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_flowspeed(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_side(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fog(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_rectrefract(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_specular(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_flowmap(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 a_origin;
	Int32 a_normal;
	Int32 a_tangent;
	Int32 a_binormal;
	Int32 a_texcoords;
	Int32 a_lightcoords;

	Int32 u_normalmap;
	Int32 u_flowmap;
	Int32 u_lightmap;
	Int32 u_refract;
	Int32 u_reflect;
	Int32 u_rectrefract;
	Int32 u_diffusemap;
	Int32 u_lightvecsmap;

	Int32 u_normalmatrix;
	Int32 u_normalmatrix_v;

	Int32 u_fogcolor;
	Int32 u_fogparams;

	Int32 u_fresnel;
	Int32 u_time;
	Int32 u_scroll;
	Int32 u_strength;
	Int32 u_texscale;
	Int32 u_rectscale;
	Int32 u_lightstrength;
	Int32 u_specularstrength;
	Int32 u_phongexponent;
	Int32 u_wavefresnelstrength;
	Int32 u_flowspeed;

	Int32 u_modelview;
	Int32 u_projection;

	Int32 d_side;
	Int32 d_fog;
	Int32 d_rectrefract;
	Int32 d_specular;
	Int32 d_flowmap;
};

/*
====================
CWaterShader

====================
*/
class CWaterShader
{
public:
	// Water quality level
	enum water_quality_t
	{
		WATER_QUALITY_NO_REFLECT_REFRACT = 0,
		WATER_QUALITY_NO_REFLECT,
		WATER_QUALITY_FULL
	};

public:
	// Default phong exponent value
	static const Float DEFAULT_PHONG_EXPONENT;
	// Default phong exponent value
	static const Float DEFAULT_SPECULAR_FACTOR;
	// Water shader default normalmap texture path
	static const Char WATER_DEFAULT_NORMALMAP_PATH[];
	// Script base path
	static const Char WATER_SCRIPT_BASEPATH[];
	// Default water script name
	static const Char DEFAULT_WATER_SCRIPT_FILENAME[];

	// Lightmap X resolution
	static const Uint32 WATER_LIGHTMAP_DEFAULT_WIDTH;
	// Lightmap Y resolution
	static const Uint32 WATER_LIGHTMAP_DEFAULT_HEIGHT;

	// FBO resolution for water
	static const Uint32 WATER_FBO_SIZE;
	// RTT resolution for water
	static const Uint32 WATER_RTT_SIZE;

public:
	CWaterShader( void );
	~CWaterShader( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );
	
	// Initializes OpenGL objects
	bool InitGL( void );
	// Clears GL objects
	void ClearGL( void );

	// Performs setup functions
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

public:
	// Adds a water entity to the list
	void AddEntity( cl_entity_t *pentity );
	// Parses a script file
	void ParseScript( const Char* pstrFilename, water_settings_t *psettings, const Char* pfile );

public:
	// Renders water entities
	bool DrawWater( bool skybox = false );
	// Render water renderpasses
	bool DrawWaterPasses( void );
	// Restores fog states
	void Restore( void ) const;

	// Loads water scripts
	void LoadScripts( void );
	// Reloads lightmap data for water entities
	void ReloadLightmapData( void );

public:
	// Retreives the current water's settings
	const water_settings_t *GetActiveSettings( void ) const;
	// Returns the current water quality setting
	const water_quality_t GetWaterQualitySetting( void ) const { return m_waterQuality; };
	// Returns the value of the reflection setting cvar
	Int32 GetWaterReflectionCvarSetting( void ) { return m_pCvarWaterReflectionSetting->GetValue(); }

private:
	// Renders the scene
	bool DrawScene( const ref_params_t& pparams, bool isrefracting );

	// Sets up for a refraction renderpass
	void SetupRefract( const water_settings_t* psettings );
	// Finalizes a refraction renderpass
	void FinishRefract( void );

	// Sets up for a reflection renderpass
	void SetupReflect( void );
	// Finishes a reflection renderpass
	void FinishReflect( void );

	// Sets up clipping
	void SetupClipping( const ref_params_t *pparams, bool negative ) const;
	// Creates a depth texture
	void CreateDepthTexture ( void );

	// Returns settings for a water entity
	const water_settings_t* GetWaterSettings( cl_water_t* pwater ) const;

private:
	// Tells if the view is within a water object
	bool ViewInWater( void );
	// Tells if a water entity should do a reflection pass
	bool ShouldReflect( Uint32 index, const water_settings_t* psettings ) const;

	// Gets the water entity's surface origin
	Vector GetWaterOrigin( cl_water_t *pwater = nullptr ) const;
	// Retreives the view origin
	Vector GetViewOrigin( void ) const;

	// Creates the lightmap texture
	void CreateLightmapTexture( cl_water_t* pwater );

	// Creates render-to-texture objects
	bool CreateRenderToTexture( cl_water_t* pwater );

private:
	// Array of water entities
	CArray<cl_water_t*> m_waterEntitiesArray;
	// Current water being rendered
	cl_water_t *m_pCurrentWater;

private:
	// Debug cvar for water
	CCVar *m_pCvarWaterDebug;
	// Cvar for water quality
	CCVar *m_pCvarWaterQuality;
	// Cvar for water reflection performance level
	CCVar *m_pCvarWaterReflectionSetting;
	// Number of renderpasses drawn
	Uint32	m_numPasses;

	// Renderpass counter
	Uint32 m_drawCounter;

	// Water quality level
	water_quality_t m_waterQuality;

private:
	// Normalmap texture for water surface
	en_texture_t *m_pDefaultNormalTexture;

	// View params for water
	ref_params_t m_waterParams;
	// Saved fog state
	fog_settings_t m_savedFog;

	// Water settings array
	CArray<water_settings_t> m_waterSettingsArray;

private:
	// GLSL shader for water
	class CGLSLShader *m_pShader;
	// VBO for water
	class CVBO *m_pVBO;

	// Shader attribs
	water_attribs m_attribs;

private:
	// Allocated texture for depth texture
	en_texalloc_t* m_pDepthTexture;
};
extern CWaterShader gWaterShader;
#endif