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

	// Prepares font rendering
	bool Prepare( void );
	// Ends font rendering
	static void Reset( void );

	// Sets the default font set
	void SetDefaultFont( const font_set_t* pset ) { m_pDefaultSet = pset; };
	// Returns the default font set
	const font_set_t* GetDefaultFont( void ) const { return m_pDefaultSet; }

	// Loads a font set
	const font_set_t *LoadFont( const Char *pstrFilename, Int32 fontSize, bool outline = false, const color32_t* poutlinecolor = nullptr, Uint32 outlineradius = 0 );
	// Draws a single string on the screen
	bool DrawString( const font_set_t *pFontSet, const Char *pstrString, Int32 x, Int32 y, bool reverse = false, Uint32 lineoffset = 0, Uint32 minlineheight = 0, Uint32 xoffset = 0 );
	// Draws a string using faster routines
	bool DrawSimpleString( const font_set_t *pFontSet, const Char *pstrString, Int32 x, Int32 y, Int32 maxlenght = 0 );
	// Draws a single character on the screen
	bool DrawChar( const font_set_t *pFontSet, Char character, Int32 x, Int32 y, Uint32 r, Uint32 g, Uint32 b, Uint32 a );
	// Sets the font color
	void SetColor( Uint32 r, Uint32 g, Uint32 b, Uint32 a );

	// Sets the drawing rectangle
	void SetRectangle( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety );
	// Returns the size of a string in pixels
	static void GetStringSize( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height = nullptr, Int32 *ymin = nullptr );

	// Binds a specific font set
	bool BindSet( const font_set_t *pset );
	// Unbinds a font set
	void UnBind( const font_set_t *pset );

	// Estimates the height of a string drawn onscreen
	Int32 EstimateHeight( const font_set_t *pFontSet, const Char *pstrString, Uint32 minlineheight = 0 );

	// Returns the shader's error message
	const Char* GetShaderError( void );
	// Tells if the shader has an error
	bool HasError( void );

private:
	// Returns the ideal string sizes
	static void GetIdealSizes( Uint32 fontsize, Uint32 *resx, Uint32 *resy, Uint32 *glyphsize, Uint32 *padding );
	// Tells if the string should be newlined
	bool ShouldNewline( Int32 offsx, Int32 x, const font_set_t *pFont, const Char *pString );
	// Renders glyphs for a font set
	bool RenderGlyphs( font_set_t* pset, fontsetglinfo_t* psetinfo, font_glyph_t* pglyphs, FT_Face pFace, Uint32 iResX, Uint32 yOffset, Uint32 baseResY, Uint32 iResY, Uint32 iGlyphSize, Uint32 iPadding, Uint32 bufferoffset, bool outline, byte* poutbuffer, Uint32 outlineradius );
	// Draws simple string characters
	void DrawSimpleStringChars( const Char* pstrString, const font_glyph_t* pglyphs, Int32 iX, Int32 iY, Int32 r, Int32 g, Int32 b, Int32 a, Int32 maxlenght, bool outline, Int32* padvancex = nullptr, Int32* padvancey = nullptr );

private:
	// Freetype library pointer
	FT_Library m_pLibrary;

	// Array of loaded fontsets
	CArray<font_set_t*> m_fontSetsArray;

	// Drawing rectangle
	Int32 m_iRectangle[4];
	// Inset values
	Int32 m_iInset[2];

	// A list of lines to render
	CLinkedList<line_t> m_linesList;

	// Colors for rendering
	Uint32 m_iR;
	Uint32 m_iG;
	Uint32 m_iB;
	Uint32 m_iA;
		
private:
	// Shader used for rendering
	class CGLSLShader *m_pShader;
	// Array of VBOs
	CArray<fontsetglinfo_t*> m_fontInfoArray;

	// Uniform indexes
	Int32	m_uiUniformProjection;
	Int32	m_uiUniformModelView;
	Int32	m_uiUniformColor;
	Int32	m_uiUniformTexture;
	Int32	m_uiUniformOffset;

	// Attribute indexes
	Int32	m_uiAttribPosition;
	Int32	m_uiAttribTexCoord;

	// Default font set
	const font_set_t* m_pDefaultSet;
};
extern CText gText;
#endif