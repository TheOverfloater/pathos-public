/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_MONITOR_H
#define R_MONITOR_H

#include "monitor_shared.h"
#include "ref_params.h"

struct msurface_t;
struct fbobind_t;
struct en_texalloc_t;
struct cl_entity_t;

struct cl_monitor_t
{
	cl_monitor_t():
		pentity(nullptr),
		start_vertex(0),
		num_vertexes(0),
		xresolution(0),
		yresolution(0),
		renderpassidx(0),
		pfbo(nullptr),
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

	Uint32 xresolution;
	Uint32 yresolution;

	Uint32 renderpassidx;

	fbobind_t *pfbo;
	en_texalloc_t* ptexture;

	byte* pvisset;
};

struct monitor_vertex_t
{
	monitor_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(texcoord, 0, sizeof(texcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	Float texcoord[2];
	byte padding[8];
};

struct monitor_attribs
{
	monitor_attribs():
		a_vertex(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scantexture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fog(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_grayscale(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 a_vertex;
	Int32 a_texcoord;

	Int32 u_modelview;
	Int32 u_projection;
	
	Int32 u_texture;
	Int32 u_scantexture;

	Int32 u_fogcolor;
	Int32 u_fogparams;

	Int32 d_fog;
	Int32 d_grayscale;
};

/*
====================
CMonitorManager

====================
*/
class CMonitorManager
{
public:
	// Scanline texture width
	static const Uint32 SCANLINE_TEXTURE_WIDTH;
	// Scanline texture height
	static const Uint32 SCANLINE_TEXTURE_HEIGHT;

public:
	CMonitorManager( void );
	~CMonitorManager( void );

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

	// Render the monitors
	bool DrawMonitors( void );
	// Draws renderpasses for monitors
	bool DrawMonitorPasses( void );
	// Seeks out any matching monitors
	cl_monitor_t* GetMatchingMonitor( Uint32 currentindex );
	
	// Allocates a new monitor
	void AllocNewMonitor( cl_entity_t* pentity );

	// Creates the scanline texture
	void CreateScanlineTexture( void );

	// Gets the number of monitors
	Uint32 GetNbMonitors( void ) const { return m_monitorsArray.size(); }
	// Retreives the monitor's PVS data
	const byte* GetMonitorPVS( Uint32 monitorindex );

private:
	// Sets up a monitor renderpass
	bool SetupMonitorPass( void );
	// Finishes a monitor renderpass
	void FinishMonitorPass( void );

	// Creates textures for a monitor
	static bool CreateMonitorTextures( cl_monitor_t* pmonitor );

private:
	// Pointer to current monitor
	cl_monitor_t *m_pCurrentMonitor;
	// Array of monitor entities
	CArray<cl_monitor_t*> m_monitorsArray;

private:
	// Pointer to GLSL shader
	class CGLSLShader *m_pShader;
	// Pointer to VBO
	class CVBO *m_pVBO;

	// Shader attribs
	monitor_attribs m_attribs;

	// Scanline texture allocation
	en_texalloc_t* m_pScanlineTexture;
	// CVar controlling monitor debug info
	CCVar *m_pCvarMonitorsDebug;

	// Numbe of monitors drawn
	Uint32 m_numMonitorsDrawn;

private:
	// Monitor view params
	ref_params_t m_monitorParams;
};

extern CMonitorManager gMonitorManager;
#endif