/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_PORTALS_H
#define R_PORTALS_H

#include "portal_shared.h"
#include "ref_params.h"

struct msurface_t;
struct fbobind_t;
struct en_texalloc_t;
struct cl_entity_t;
struct mleaf_t;

struct cl_portal_t
{
	cl_portal_t():
		pentity(nullptr),
		start_vertex(0),
		num_vertexes(0),
		renderpassidx(0),
		ptexture(nullptr),
		pvisset(nullptr)
		{}

	cl_entity_t *pentity;

	Vector mins;
	Vector maxs;

	Vector origin;
	CArray<msurface_t*> surfaces;

	Uint32 start_vertex;
	Uint32 num_vertexes;

	Uint32 renderpassidx;

	en_texalloc_t* ptexture;

	byte* pvisset;
};

struct portal_vertex_t
{
	portal_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	byte padding[16];
};

struct portal_attribs
{
	portal_attribs():
		a_vertex(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_screenwidth(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_screenheight(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fog(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 a_vertex;

	Int32 u_modelview;
	Int32 u_projection;
	
	Int32 u_texture;
	Int32 u_screenwidth;
	Int32 u_screenheight;

	Int32 u_fogcolor;
	Int32 u_fogparams;

	Int32 d_fog;
};

/*
====================
CPortalManager

====================
*/
class CPortalManager
{
public:
	CPortalManager( void );
	~CPortalManager( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes game objects
	bool InitGame( void );
	// Clears the game objects
	void ClearGame( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Clears OpenGL objects
	void ClearGL( void );

	// Render the portals
	bool DrawPortals( void );
	// Draws renderpasses for portals
	bool DrawPortalPasses( void );
	// Seeks out any matching portals
	cl_portal_t* GetMatchingPortal( Uint32 currentindex, class CFrustum& mainFrustum );
	
	// Allocates a new portal
	void AllocNewPortal( cl_entity_t* pentity );

	// Gets the number of portals
	Uint32 GetNbPortals( void ) const { return m_portalsArray.size(); }
	// Retreives the portal's PVS data
	const byte* GetPortalPVS( Uint32 portalindex );

private:
	// Sets up a portal renderpass
	bool SetupPortalPass( void );
	// Finishes a portal renderpass
	void FinishPortalPass( void );

	// Creates texture for a portal
	static bool CreatePortalTexture( cl_portal_t* pportal );

private:
	// Pointer to current portal
	cl_portal_t *m_pCurrentPortal;
	// Array of portal entities
	CArray<cl_portal_t*> m_portalsArray;

private:
	// Pointer to GLSL shader
	class CGLSLShader *m_pShader;
	// Pointer to VBO
	class CVBO *m_pVBO;

	// Shader attribs
	portal_attribs m_attribs;

	// CVar controlling portal debug info
	CCVar *m_pCvarPortalDebug;

	// Number of portals drawn
	Uint32 m_numPortalsDrawn;

private:
	// Portal view params
	ref_params_t m_portalParams;
};

extern CPortalManager gPortalManager;
#endif