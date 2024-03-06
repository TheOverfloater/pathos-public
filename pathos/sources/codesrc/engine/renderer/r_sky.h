/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_SKY_H
#define R_SKY_H

struct sky_attribs_t
{
	sky_attribs_t():
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_mode(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}


	Int32 u_projection;
	Int32 u_modelview;
	Int32 u_color;
	Int32 u_texture;

	Int32 a_texcoord;
	Int32 a_origin;

	Int32 d_mode;
};

/*
====================
CSkyRenderer

====================
*/
class CSkyRenderer
{
public:
	// Skybox surface distance
	static const Float SKYBOX_SURFACE_DISTANCE;
	// Number of skybox textures
	static const Uint32 NB_SKYBOX_TEXTURES = 6;
	// Skybox texture base directory
	static const Char SKYBOX_TEXTURE_DIR[];
	// Skybox texture postfixes
	static const Char* SKY_TEXTURE_POSTFIXES[NB_SKYBOX_TEXTURES];

public:
	enum shader_modes_t
	{
		SHADER_TEXTURE = 0,
		SHADER_COLOR
	};

	struct skytextureset_t
	{
		skytextureset_t():
			serverindex(NO_POSITION)
		{
			for(Uint32 i = 0; i < NB_SKYBOX_TEXTURES; i++)
				ptextures[i] = nullptr;
		}

		// Sky texture name
		CString texturename;
		// Index on server
		Int32 serverindex;
		// Individual sky textures
		en_texture_t* ptextures[NB_SKYBOX_TEXTURES];
	};

public:
	CSkyRenderer( void );
	~CSkyRenderer( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Shuts down OpenGL objects
	void ClearGL( void );

	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

public:
	// Performs pre-frame functions
	void PreFrame( void );
	// Draws cables
	bool DrawSky( void );

	// Add a sky set
	void AddSkyTextureSet( const Char* pstrSkyTextureName, Int32 skysetindex );

	// Sets current sky set
	void SetCurrentSkySet( Int32 skysetindex );
	// Sets current sky set
	void SetSkyBoxSkySet( Int32 skysetindex );
	// Sets the sky texture to be used
	void SetSkyTexture( Int32 skysetindex );

private:
	// Load sky textures
	void LoadSkyTextures( const Char* pstrName, en_texture_t** pArray );
	// Creates skybox VBO
	void CreateVBO( void );
	// Finds a sky set
	Int32 FindSkySet( Int32 skysetindex );

private:
	// Skybox related
	en_texture_t* m_pSkyboxTextures[NB_SKYBOX_TEXTURES];
	Uint32 m_skyIndexBase;

	// Screen quad
	Uint32 m_screenQuadBase;

	// Cvar for toggling sky rendering
	CCVar* m_pCvarDrawSky;

	// Sky set used for rendering
	Int32 m_currentSkySet;
	// Sky texture set used by skybox
	Int32 m_skyBoxSkySet;
	// Sky set used for actual rendering
	Int32 m_skySetUsed;

	// Array of sky sets
	CArray<skytextureset_t> m_skySetsArray;

private:
	// Shader object
	class CGLSLShader* m_pShader;
	// VBO object
	class CVBO* m_pVBO;
	// Sky shader attribs
	sky_attribs_t m_attribs;
};
extern CSkyRenderer gSkyRenderer;
#endif