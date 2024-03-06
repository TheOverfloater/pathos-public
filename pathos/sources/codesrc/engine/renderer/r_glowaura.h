/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GLOW_AURA_H
#define GLOW_AURA_H

#include "r_rttcache.h"

// Maximum entities rendered for glow
static const Uint32 MAX_AURA_ENTITIES	= 32;

enum glow_shaders_t
{
	glowaura_blur_h = 0,
	glowaura_blur_v,
	glowaura_combine,
	glowaura_texture,
};

struct aura_vertex_t
{
	aura_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(texcoord, 0, sizeof(texcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	Float texcoord[2];
	byte padding[8];
};

struct aura_attribs
{
	aura_attribs():
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_size(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_screensize(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture0(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scrntexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_type(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 a_origin;
	Int32 a_texcoord;

	Int32 u_projection;
	Int32 u_modelview;
	
	Int32 u_size;
	Int32 u_screensize;

	Int32 u_texture0;
	Int32 u_scrntexture;

	Int32 d_type;
};

/*
====================
CGlowAura

====================
*/
class CGlowAura
{
public:
	// Radius of aura effect
	static const Float AURA_RADIUS;
	// Maximum aura resolution
	static const Uint32 AURA_RESOLUTION;
public:
	CGlowAura( void );
	~CGlowAura( void );

public:
	bool Init( void );
	void Shutdown( void );
	bool DrawAuras( void );

	bool InitGL( void );
	void ClearGL( void );

private:
	void GetEntities( void );
	bool DrawSolid( void );
	bool DrawColors( void );
	bool DrawFinal( void );
	bool BlurTexture( void );

private:
	// Entities to render for glow aura
	cl_entity_t	*m_pEntities[MAX_AURA_ENTITIES];
	// Number of entities to render
	Uint32	m_iNumEntities;

private:
	// Shader object
	class CGLSLShader *m_pShader;
	// VBO object
	class CVBO *m_pVBO;

	// Shader attribs
	aura_attribs m_attribs;

	// RTT textures
	rtt_texture_t* m_pScreenRTT;
	rtt_texture_t* m_pWhiteRTT;
	rtt_texture_t* m_pColorsRTT;
	rtt_texture_t* m_pBlurRTT;

private:
	// Cvar controlling glow aura
	CCVar* m_pCvarGlowAura;
};

extern CGlowAura gGlowAura;
#endif