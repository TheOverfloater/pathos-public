/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_CABLES_H
#define R_CABLES_H

#define MAX_CABLE_ENTITIES	512

struct cable_vertex_t
{
	cable_vertex_t():
		width(0)
	{
		memset(origin, 0, sizeof(origin));
		memset(vpoint, 0, sizeof(vpoint));
		memset(color, 0, sizeof(color));
		memset(texcoord, 0, sizeof(texcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	Vector vpoint;
	vec4_t color;

	Float texcoord[2];
	Float width;
	byte padding[8];
};

struct cable_attribs
{
	cable_attribs():
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_vorigin(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_start(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_width(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_vpoint(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fog(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}


	Int32 u_projection;
	Int32 u_modelview;

	Int32 u_fogparams;
	Int32 u_fogcolor;

	Int32 u_vorigin;
	Int32 u_start;

	Int32 a_texcoord;
	Int32 a_origin;
	Int32 a_color;
	Int32 a_width;
	Int32 a_vpoint;

	Int32 d_fog;
};

struct cable_lsample_t
{
	cable_lsample_t()
	{
		memset(styles, 0, sizeof(styles));
	}

	Vector diffuselight[MAX_SURFACE_STYLES];
	Vector ambientlight[MAX_SURFACE_STYLES];
	Vector lightdirs[MAX_SURFACE_STYLES];
	byte styles[MAX_SURFACE_STYLES];
};

struct cable_object_t
{
	cable_object_t():
		numsegments(0),
		pmodel(nullptr),
		start_vertex(0),
		num_vertexes(0),
		falldepth(0),
		width(0),
		windx(0),
		windy(0),
		numleafs(0)
		{}

	Vector start;
	Vector end;
	Vector vmins;
	Vector vmaxs;

	Int32 numsegments;

	const cache_model_t* pmodel;
	CArray<cable_lsample_t> pointsamples;

	Int32 start_vertex;
	Int32 num_vertexes;
	Float falldepth;
	Float width;
	Float windx;
	Float windy;

	CArray<Uint32> leafnums;
	Uint32 numleafs;
};

/*
====================
CCableRenderer

====================
*/
class CCableRenderer
{
public:
	CCableRenderer( void );
	~CCableRenderer( void );

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
	// Draws cables
	bool DrawCables( void );
	// Adds a new cable object
	void AddCable( Int32 spritemodelindex, const Vector& start, const Vector& end, Uint32 depth, Uint32 width, Uint32 numsegments, Float windx, Float windy );
	// Recalculates lighting for cables
	void RefreshLighting( void );

private:
	// Calculate position of a segment
	Vector CalculatePoint( Uint32 segment, Uint32 numsegments, const Vector& start, const Vector& end, const Vector& midpoint );
	// Calculate light color for a point
	Vector CalculateLighting( const cable_object_t& cable, const cable_lsample_t& sample, const Vector& position, const Vector& normal );

private:
	// Array of cable objects
	CArray<cable_object_t> m_cablesArray;
};
extern CCableRenderer gCableRenderer;
#endif