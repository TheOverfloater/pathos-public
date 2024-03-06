/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_SPRITES_H
#define R_SPRITES_H

struct cl_entity_t;

#include "r_glsl.h"

struct sprite_vertex_t
{
	sprite_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(color, 0, sizeof(color));
		memset(texcoord, 0, sizeof(texcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	vec4_t color;

	Float texcoord[2];
	byte padding[24];
};

struct sprite_attribs
{
	sprite_attribs():
	u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
	u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
	u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
	u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
	u_texture(CGLSLShader::PROPERTY_UNAVAILABLE),
	a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
	a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
	a_color(CGLSLShader::PROPERTY_UNAVAILABLE),
	d_fog(CGLSLShader::PROPERTY_UNAVAILABLE),
	d_solid(CGLSLShader::PROPERTY_UNAVAILABLE)
	{}

	Int32 u_projection;
	Int32 u_modelview;

	Int32 u_fogparams;
	Int32 u_fogcolor;

	Int32 u_texture;

	Int32 a_texcoord;
	Int32 a_origin;
	Int32 a_color;

	Int32 d_fog;
	Int32 d_solid;
};

struct temp_sprite_t
{
	temp_sprite_t():
		key(0),
		life(0)
		{}

	Int32 key;
	Double life;

	cl_entity_t entity;
};

/*
====================
CSpriteRenderer

====================
*/
class CSpriteRenderer
{
public:
	// Max rendered sprites
	static const Uint32 MAX_RENDERED_SPRITES;

	// Glow interpolation speed
	static const Float GLOW_INTERP_SPEED;

	// Glow minimum distance
	static const Float GLOW_MINDIST;
	// Glow maximum distance
	static const Float GLOW_MAXDIST;
	// Glow halo distance
	static const Float GLOW_HALODIST;

public:
	// Number of glow tracelines
	static const Uint32 GLOW_NUM_TRACES = 5;
	// Maximum temporary sprites
	static const Uint32 MAX_TEMP_SPRITES = 64;

public:
	CSpriteRenderer( void );
	~CSpriteRenderer( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Inits game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

	// Sets up opengl objects
	bool InitGL( void );
	// Clears opengl objects
	void ClearGL( void );

public:
	// Animates sprites
	void Animate( void );
	// Draws the sprites
	bool DrawSprites( void );

	// Allocates a static sprite
	cl_entity_t* AllocStaticSprite( void );
	// Allocates a temporary sprite object
	cl_entity_t* AllocTempSprite( Int32 key, Float life );	

public:
	// Draw function for rendering glow dots
	void DrawFunction( const Vector& origin );

private:
	// Updates tempentities
	void UpdateTempEnts( void );

	// Batches a sprite
	bool BatchSprite( cl_entity_t *pEntity );
	// Batches a sprite vertex
	void BatchVertex( const Vector& vertex, const vec4_t& color, Float texcoords, Float texcoordt );
	// Batches a sprite vertex
	void BatchVertex( Uint32 position, const Vector& vertex, Float r, Float g, Float b, Float a, Float texcoords, Float texcoordt );

	// Batches sprites for rendering
	void BatchSprites( cl_entity_t* entitiesArray, Uint32 numEntities );
	// Draws sprite arrays
	bool DrawSpriteArrays( cl_entity_t* entitiesArray, Uint32 numEntities );

private:
	// Array of temporary sprite entities
	temp_sprite_t m_tempSpritesArray[MAX_TEMP_SPRITES];

	// Static sprite entities
	CArray<cl_entity_t> m_staticSpritesArray;

	// Toggles rendering of sprites
	CCVar*	m_pCvarDrawSprites;

private:
	// Pointer to shader object
	class CGLSLShader *m_pShader;
	// Pointer to VBO object
	class CVBO *m_pVBO;

	// Shader attrib indexes
	sprite_attribs m_attribs;

	// Offset for occlusion query buffer
	Uint32 m_occlusionQueryVBOBufferOffset;

private:
	// View matrix for rendering
	Float m_viewMatrix[3][4];

	// Number of indexes to render
	Uint32 m_numIndexes;

	// Array for vertex uploads to GPU
	sprite_vertex_t* m_pVertexes;
	// Number of vertexes batched
	Uint32	m_numVertexes;
	// TRUE if we prompted about limits this frame
	bool m_promptedLimitsThisFrame;

	// Left vector for glow occlusion
	Vector m_aLeft;
	// Step vector for glow occlusion
	Vector m_step;
	// Glow offset fraction
	Int32 m_glowStep;
};

extern CSpriteRenderer gSpriteRenderer;
extern void SPR_DrawFunction( void* pContext, const Vector& origin );
#endif //R_SPRITES_H