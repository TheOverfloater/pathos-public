/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_MIRROR_H
#define R_MIRROR_H

#include "ref_params.h"

struct fbobind_t;
struct cl_entity_t;
struct msurface_t;
struct en_texalloc_t;

struct cl_mirror_t
{
	cl_mirror_t():
		pentity(nullptr),
		psurface(nullptr),
		start_vertex(0),
		num_vertexes(0),
		renderpassidx(0),
		pfbo(nullptr),
		ptexture(nullptr)
		{}

	cl_entity_t *pentity;

	Vector mins;
	Vector maxs;

	Vector origin;
	msurface_t *psurface;

	Uint32 start_vertex;
	Uint32 num_vertexes;

	Uint32 renderpassidx;

	fbobind_t *pfbo;
	en_texalloc_t *ptexture;
};

struct mirror_vertex_t
{
	mirror_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	byte padding[16];
};

struct mirror_attribs
{
	mirror_attribs():
		a_vertex(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_mirrormatrix(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_dt_x(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_dt_y(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fog(CGLSLShader::PROPERTY_UNAVAILABLE)
		{
		}

	Int32 a_vertex;

	Int32 u_modelview;
	Int32 u_projection;

	Int32 u_mirrormatrix;
	
	Int32 u_texture;

	Int32 u_dt_x;
	Int32 u_dt_y;

	Int32 u_fogcolor;
	Int32 u_fogparams;

	Int32 d_fog;
};

/*
====================
CMirrorManager

====================
*/
class CMirrorManager
{
public:
	// Mirror FBO resolution
	static const Uint32 MIRROR_FBO_SIZE;
	// Mirror render-to-texture resolution
	static const Uint32 MIRROR_RTT_SIZE;

public:
	CMirrorManager( void );
	~CMirrorManager( void );
public:
	// Initializes the class
	bool InitGL( void );
	// Shuts down the class
	void ClearGL( void );

	// Clears the game
	bool InitGame( void );
	// Clears the game
	void ClearGame( void );

	// Draw mirrors
	bool DrawMirrors( void );
	// Draw mirror passes
	bool DrawMirrorPasses( void );

	// Allocates a new mirror
	void AllocNewMirror( cl_entity_t* pentity );

private:
	// Sets up a mirror renderpass
	void SetupMirrorPass( void );
	// Finishes a mirror renderpass
	void FinishMirrorPass( void );

	// Sets up clipping
	void SetupClipping( void );
	// Creates a depth texture
	void CreateDepthTexture( void );

	// Allocates textures for a mirror
	bool AllocTextures( cl_mirror_t* pmirror );

private:
	// Currently managed mirror
	cl_mirror_t *m_pCurrentMirror;
	// Array of mirror entities
	CArray<cl_mirror_t*> m_mirrorsArray;

private:
	// GLSL shader
	class CGLSLShader *m_pShader;
	// VBO
	class CVBO *m_pVBO;

	// Mirror shader attributes
	mirror_attribs m_attribs;

private:
	// Mirror view params
	ref_params_t m_mirrorParams;
	// Depth texture allocation
	en_texalloc_t* m_pDepthTexture;
};

extern CMirrorManager gMirrorManager;
#endif