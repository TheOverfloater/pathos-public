/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_BASICDRAW_H
#define R_BASICDRAW_H

#include "r_basic_vertex.h"
#include "constants.h"

/*
=================================
CBasicDraw

=================================
*/
class CBasicDraw
{
public:
	// Increment by 1024 vertexes
	static const Uint32 BASICDRAW_CACHE_SIZE;

private:
	struct shader_attribs_t
	{
		shader_attribs_t():
			u_projection(0),
			u_modelview(0),
			u_texture(0),
			u_rectangle(0),
			u_multiplier(0),
			u_fogparams(0),
			u_fogcolor(0),
			a_position(0),
			a_texcoord(0),
			a_color(0),
			d_texture(0),
			d_rectangle(0),
			d_fog(0)
		{}

		Int32 u_projection;
		Int32 u_modelview;
		Int32 u_texture;
		Int32 u_rectangle;
		Int32 u_multiplier;

		Int32 u_fogparams;
		Int32 u_fogcolor;

		Int32 a_position;
		Int32 a_texcoord;
		Int32 a_color;

		Int32 d_texture;
		Int32 d_rectangle;
		Int32 d_fog;
	};

private:
	CBasicDraw( void );
	~CBasicDraw( void );

public:
	// Sets up OpenGL objects
	bool InitGL( const CGLExtF& gGlExtF, const file_interface_t& fileFuncs, pfnErrorPopup_t pfnErrorPopup );
	// Clears OpenGL objects
	void ClearGL( void );

	// Resets states for primitive rendering
	void Begin( Int32 primitiveType );
	// Renders the primitives
	void End( bool clearData = false );
	// Enables texture use
	bool EnableTexture( void );
	// Disables texture use
	bool DisableTexture( void );
	// Enables simple draw
	bool Enable( void );
	// Disables simple draw
	void Disable( void );
	// Enables texture use
	bool EnableRectangleTexture( void );
	// Disables texture use
	bool DisableRectangleTexture( void );

	// Enables fogging
	bool EnableFog( void );
	// Sets fog parameters
	void SetFogParams( const Vector& fogcolor, Float startdist, Float enddist );
	// Disables fogging
	bool DisableFog( void );

	// Tells if we're bound right now
	bool IsActive( void ) const { return m_isActive; }
	
	// Sets the projection matrix
	void SetProjection( const Float* pMatrix );
	// Sets the modelview matrix
	void SetModelview( const Float* pMatrix );

	// Sets a vertex's color
	void Color4f( Float r, Float g, Float b, Float a );
	void Color4fv( const Float* pfc );
	// Sets a vertex's texcoords
	void TexCoord2f( Float u, Float v );
	void TexCoord2fv( const Float* ptc );
	// Sets a vertex's origin
	void Vertex3f( Float x, Float y, Float z );
	void Vertex3fv( const Float* pfv );
	// Sets the brightness value
	void Brightness1f( Float brightness );
	// Sets the color multiplier value
	void SetColorMultiplier( Float multiplier );

	// Returns the shader's error message
	const Char* GetShaderError( void ) const;
	// Tells if the shader has an error
	bool HasError( void ) const;

	// Validates current shader setup
	void ValidateShaderSetup( void (*pfnConPrintfFnPtr)( const Char *fmt, ... ) );

public:
	// Creates an instance of this class
	static CBasicDraw* CreateInstance( void );
	// Returns the current instance of this class
	static CBasicDraw* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

public:
	// TRUE if we're bound right now
	bool m_isActive;
	// Array of vertexes
	CArray<basic_vertex_t> m_vertexesArray;
	// Number of currently cached vertexes
	Uint32 m_numVertexes;
	// Primitive type
	Int32 m_primitiveType;
	// Last color specified
	vec4_t m_vertexColor;
	// Brightness
	Float m_brightness;

	// Pointer to VBO object
	class CVBO* m_pVBO;
	// Pointer to GLSL shader object
	class CGLSLShader* m_pShader;
	// Shader attribs
	shader_attribs_t m_shaderAttribs;

private:
	// Current instance of the class
	static CBasicDraw* g_pInstance;
};
#endif //R_BASICDRAW_H