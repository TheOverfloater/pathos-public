/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef FONTSET_H
#define FONTSET_H

// Number of glyphs
static const Uint32 NUM_GLYPHS = 256;
// Default font size
static const Uint32 DEFAULT_FONT_SIZE = 20;
// Default font size
static const Uint32 MAX_FONT_SIZE = 72;
// Default font set name
static const Char DEFAULT_FONTSET_NAME[] = "calibri.ttf";

// No GL info index
static const Int32 NO_GL_INFO_INDEX = -1;

struct font_glyph_t
{
	font_glyph_t():
		width(0),
		height(0),
		bitmap_left(0),
		bitmap_top(0),
		advancex(0),
		advancey(0),
		ymin(0),
		pitch(0),
		start_vertex(0)
	{
		memset(texcoords, 0, sizeof(texcoords));
	}

	Uint16 width;
	Uint16 height;

	Int32 bitmap_left;
	Int32 bitmap_top;

	Int32 advancex;
	Int32 advancey;

	Int32 ymin;
	Int32 pitch;

	Uint32 start_vertex;

	// s and t offsets
	Float texcoords[4][2];
};

struct font_set_t
{
	font_set_t():
		fontsize(0),
		maxheight(0),
		outlineradius(0),
		outline(false),
		infoindex(NO_GL_INFO_INDEX)
	{}

	CString name;
	Int32 fontsize;
	Int32 maxheight;
	Uint32 outlineradius;

	font_glyph_t glyphs[NUM_GLYPHS];
	font_glyph_t glyphs_outline[NUM_GLYPHS];

	bool outline;
	color32_t outlinecolor;

	Int32 infoindex;
};

#endif //FONTSET_H