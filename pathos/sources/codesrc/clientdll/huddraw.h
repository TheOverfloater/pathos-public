/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef HUDDRAW_H
#define HUDDRAW_H

struct font_set_t;

// All sizes relative to a 1920x1080 resolution
#define BASE_RESOLUTION_X			1920.0
#define BASE_RESOLUTION_Y			1080.0

struct hud_attribs_t
{
	hud_attribs_t():
	u_modelview(0),
	u_projection(0),
	u_origin(0),
	u_size(0),
	u_indent(0),
	u_alphamod(0),
	u_color(0),
	u_texture(0),
	a_origin(0),
	a_texcoord(0),
	a_sizemod(0),
	a_alpha(0),
	a_indent(0),
	a_alphamod(0),
	d_solid(0)
	{}

	Int32 u_modelview;
	Int32 u_projection;
	Int32 u_origin;
	Int32 u_size;
	Int32 u_indent;
	Int32 u_alphamod;

	Int32 u_color;
	Int32 u_texture;

	Int32 a_origin;
	Int32 a_texcoord;
	Int32 a_sizemod;
	Int32 a_alpha;
	Int32 a_indent;
	Int32 a_alphamod;

	Int32 d_solid;
};

struct hud_vertex_t
{
	hud_vertex_t():
		indent(0),
		alpha(0)
	{
		memset(origin, 0, sizeof(origin));
		memset(sizemod, 0, sizeof(sizemod));
		memset(alphamod, 0, sizeof(alphamod));
		memset(texcoord, 0, sizeof(texcoord));
		memset(pad, 0, sizeof(pad));
	}

	vec4_t origin;
	Float sizemod[2];
	Float alphamod[4];
	Float texcoord[2];
	Float indent;
	Float alpha;
	byte pad[40];
};

/*
=================================
CHudDraw

=================================
*/
class CHUDDraw
{
public:
	// Edge size 
	static const Float HUD_EDGE_SIZE;
	// Number of verts in VBO for body
	static const Uint32 NUM_BODY_VERTEXES;
	// Number of verts in VBO for a quad
	static const Uint32 NUM_QUAD_VERTEXES;
	// Number of verts in VBO total
	static const Uint32 NUM_TOTAL_VERTEXES;

public:
	CHUDDraw( void );
	~CHUDDraw( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes the class
	bool InitGL( void );
	// Shuts down the class
	void ClearGL( void );
	
public:
	// Sets up rendering
	bool SetupDraw( void );
	// Finishes rendering
	void FinishDraw( void );

	// Draws a body
	bool DrawBody( bool indent = false );
	// Draws a quad
	bool DrawQuad( struct en_texture_t *ptexture );
	// Draws HUD text
	bool DrawText( const Vector& color, Float alpha, Float x, Float y, const Char *sztext, const font_set_t *pfont );

	// Sets the color
	void SetColor( const Vector& rgb, byte a );
	// Sets the color
	void SetColor( byte r, byte g, byte b, byte a );
	// Sets the alpha mod
	void SetAlphaMod( byte a1, byte a2, byte a3, byte a4 );

	// Sets the origin
	void SetOrigin( Float x, Float y );
	// Sets the size
	void SetSize( Float x, Float y );

	// Scales X coordinate by the reference size
	Float ScaleX( Float x ) const;
	// Scales Y coordinate by the reference size
	Float ScaleY( Float y ) const;
	// Returns the relative X coordinate
	Float ScaleXRelative( Float x ) const;

	// Tells if the HUD renderer has any errors
	bool HasError( void ) const;
	// Returns the error message
	const Char* GetError( void ) const;
	// Manages error popup
	void ManageErrorMessage( void ) const;

private:
	// Constructs the quad
	static void ConstructQuad( hud_vertex_t *pvertexes );
	// Constructs the body
	void ConstructBody( hud_vertex_t *pvertexes ) const;

private:
	// GLSL shader
	class CGLSLShader	*m_pShader;
	// VBO object
	class CVBO			*m_pVBO;
	// Shader attribs
	hud_attribs_t		m_attribs;

	// Primary color with alpha
	color32_t			m_color;
	// Alpha mod for corners
	byte				m_alphaMod[4];

	// Screen width
	Uint32				m_screenWidth;
	// Screen height
	Uint32				m_screenHeight;
};
extern CHUDDraw gHUDDraw;
#endif //HUDDRAW_H