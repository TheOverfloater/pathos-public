/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef R_TEXT_H
#define R_TEXT_H

// FreeType Headers
#include <ft2build.h>
#include FT_FREETYPE_H
#include <ftglyph.h> // needed for bounding box bit
#include "fontset.h"
#include <ftstroke.h>

#include "r_glsl.h"

struct fontsetglinfo_t
{
	fontsetglinfo_t():
		palloc(nullptr),
		index_offset_outline(0),
		pvbo(nullptr)
		{}
	~fontsetglinfo_t()
	{
		if(pvbo)
			delete pvbo;
	}

	struct en_texalloc_t* palloc;
	Uint32 index_offset_outline;

	class CVBO *pvbo;
};

struct font_vertex_t
{
	font_vertex_t()
	{
		memset(position, 0, sizeof(position));
		memset(texcoord, 0, sizeof(texcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t position;
	Float texcoord[2];
	byte padding[8];
};

struct line_chunk_t
{
	line_chunk_t():
		pstringbegin(nullptr),
		length(0)
		{}

	color24_t color;
	const Char* pstringbegin;
	Uint32 length;
};

struct line_t
{
	line_t():
		lineheight(0)
		{}

	CLinkedList<line_chunk_t> chunks;
	Int32 lineheight;
};

struct text_attribs_t
{
	text_attribs_t():
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_offset(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_position(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE)
	{}

	// Uniform indexes
	Int32 u_projection;
	Int32 u_modelview;
	Int32 u_color;
	Int32 u_texture;
	Int32 u_offset;

	// Attribute indexes
	Int32 a_position;
	Int32 a_texcoord;
};

//=====================
//	CText
//
//=====================
class CText
{
private:
	// Font set base directory
	static const Char FONT_DIRECTORY[];

public:
	CText( void );
	~CText( void );

public:
	// Initializes the class
	bool Init ( void );
	// Performs shutdown ops
	void Shutdown( void );
	// Initializes GL objects
	bool InitGL( void );
	// Releases GL objects
	void ClearGL( void );

	// Prepares font rendering by binding shaders, setting matrices, etc
	bool Prepare( void );
	// Ends font rendering, unbinds shaders, VBOs, etc
	void Reset( void );

	// Sets the default font set
	void SetDefaultFont( const font_set_t* pset ) { m_pDefaultSet = pset; };
	// Returns the default font set
	const font_set_t* GetDefaultFont( void ) const { return m_pDefaultSet; }

	// Loads a font set
	const font_set_t *LoadFont( const Char *pstrFilename, Int32 fontSize, bool outline = false, const color32_t* poutlinecolor = nullptr, Uint32 outlineradius = 0 );
	// Draws a single string on the screen
	bool DrawString( const Char *pstrString, Int32 x, Int32 y, bool reverse = false, Uint32 lineoffset = 0, Uint32 minlineheight = 0, Uint32 xoffset = 0 );
	// Draws a string using faster routines without newlining or bounds checks
	bool DrawSimpleString( const Char *pstrString, Int32 x, Int32 y, Int32 maxlenght = 0 );
	// Draws a single character on the screen
	bool DrawChar( Char character, Int32 x, Int32 y, Uint32 r, Uint32 g, Uint32 b, Uint32 a );
	// Sets the font color
	void SetColor( Uint32 r, Uint32 g, Uint32 b, Uint32 a );

	// Sets the drawing rectangle bounds
	void SetRectangle( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety );
	// Returns the size of a string in pixels
	static void GetStringSize( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height = nullptr, Int32 *ymin = nullptr );

	// Binds a specific font set
	bool BindSet( const font_set_t *pset );
	// Unbinds the currently bound font set
	void UnBindCurrentSet( void );

	// Estimates the height of a string drawn onscreen
	Int32 EstimateHeight( const font_set_t *pFontSet, const Char *pstrString, Uint32 minlineheight = 0 );

	// Returns the shader's error message
	const Char* GetShaderError( void );
	// Tells if the shader has an error
	bool HasError( void );

private:
	// Returns the ideal resolution for a font set texture
	static void GetIdealSizes( Uint32 fontsize, Uint32 *resx, Uint32 *resy, Uint32 *glyphsize, Uint32 *padding );
	// Tells if the string should be newlined
	bool ShouldNewline( Int32 offsx, Int32 x, const font_set_t *pfont, const Char *pstring );
	// Renders glyphs for a font set
	bool RenderGlyphs( font_set_t* pset, fontsetglinfo_t* psetinfo, font_glyph_t* pglyphs, FT_Face pface, Uint32 sizex, Uint32 yoffset, Uint32 basesizey, Uint32 sizey, Uint32 glyphsize, Uint32 iPadding, Uint32 bufferoffset, bool outline, byte* poutbuffer, Uint32 outlineradius );
	// Draws simple string characters
	void DrawSimpleStringChars( const Char* pstrString, const font_glyph_t* pglyphs, Int32 x, Int32 y, Int32 r, Int32 g, Int32 b, Int32 a, Int32 maxlenght, bool outline, Int32* padvancex = nullptr, Int32* padvancey = nullptr );

private:
	// Freetype library pointer
	FT_Library m_pLibrary;

	// Array of loaded fontsets
	CArray<font_set_t*> m_fontSetsArray;
	// Array of VBOs
	CArray<fontsetglinfo_t*> m_fontInfoArray;

	// Drawing rectangle
	Int32 m_rectangleBounds[4];
	// Inset values
	Int32 m_textInset[2];

	// A list of lines to render
	CLinkedList<line_t> m_linesList;

	// Currently bound font set
	const font_set_t* m_pBoundFontSet;
	// Currently bound font set's info struct
	fontsetglinfo_t* m_pCurrentSetInfo;

	// Colors for rendering
	color32_t m_textColor;
	// TRUE if shader was enabled
	bool m_isActive;

private:
	// Shader used for rendering
	class CGLSLShader *m_pShader;

	// Shader attribs
	text_attribs_t m_attribs;

	// Default font set
	const font_set_t* m_pDefaultSet;
};
extern CText gText;
#endif