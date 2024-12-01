/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef TEXTURES_SHARED_H
#define TEXTURES_SHARED_H

// Texture flags
enum tx_flags
{
	TX_FL_NONE				= 0,
	TX_FL_FULLBRIGHT		= (1<<0),
	TX_FL_NOMIPMAPS			= (1<<1),
	TX_FL_NODECAL			= (1<<2),
	TX_FL_CUBEMAPS			= (1<<3),
	TX_FL_CLAMP_S			= (1<<4),
	TX_FL_CLAMP_T			= (1<<5),
	TX_FL_EYEGLINT			= (1<<6),
	TX_FL_CHROME			= (1<<7),
	TX_FL_SCOPE				= (1<<8),
	TX_FL_BORDER			= (1<<9),
	TX_FL_ALPHATEST			= (1<<10),
	TX_FL_NO_STEPSOUND		= (1<<11),
	TX_FL_ALPHABLEND		= (1<<12),
	TX_FL_ADDITIVE			= (1<<13),
	TX_FL_NO_CULLING		= (1<<14),
	TX_FL_RECTANGLE			= (1<<15),
	TX_FL_DXT1				= (1<<16),
	TX_FL_DXT5				= (1<<17),
	TX_FL_NO_IMPACT_EFFECTS = (1<<18),
	TX_FL_NO_PENETRATION	= (1<<19),
	TX_FL_BULLETPROOF		= (1<<20)
};

enum mt_texture_t
{
	MT_TX_UNKNOWN = -1,
	MT_TX_DIFFUSE = 0,
	MT_TX_NORMALMAP,
	MT_TX_DETAIL,
	MT_TX_SPECULAR,
	MT_TX_LUMINANCE,
	MT_TX_AO,

	// Must be last
	NB_MT_TX,
};

enum texture_format_t
{
	TX_FORMAT_UNDEFINED = 0,
	TX_FORMAT_TGA,
	TX_FORMAT_DDS,
	TX_FORMAT_BMP,
	TX_FORMAT_MEMORY
};

enum texture_compression_t
{
	TX_COMPRESSION_NONE = 0,
	TX_COMPRESSION_RLE,
	TX_COMPRESSION_DXT1,
	TX_COMPRESSION_DXT5
};

struct en_texalloc_t
{
	en_texalloc_t():
		gl_index(0),
		level(RS_LEVEL_UNDEFINED)
	{}

	GLuint gl_index;
	rs_level_t level;
};

struct en_texture_t
{
	en_texture_t():
		flags(TX_FL_NONE),
		bpp(0),
		width(0),
		height(0),
		needsload(true),
		level(RS_LEVEL_UNDEFINED),
		format(TX_FORMAT_UNDEFINED),
		compression(TX_COMPRESSION_NONE),
		palloc(nullptr)
	{
		for(Uint32 i = 0; i < 4; i++)
			bordercolor[i] = 0;
	}

	CString filepath;

	Int32 flags;

	Uint32 bpp;
	Uint32 width;
	Uint32 height;

	bool needsload;

	rs_level_t level;
	texture_format_t format;
	texture_compression_t compression;

	GLint bordercolor[4];

	en_texalloc_t* palloc;
};

struct en_material_t
{
	en_material_t():
		level(RS_LEVEL_UNDEFINED),
		dt_scalex(0),
		dt_scaley(0),
		int_width(0),
		int_height(0),
		alpha(0),
		phong_exp(0),
		spec_factor(0),
		scale(0),
		cubemapstrength(0),
		scrollu(0),
		scrollv(0),
		flags(TX_FL_NONE),
		index(0)
	{
		for(Uint32 i = 0; i < NB_MT_TX; i++)
			ptextures[i] = nullptr;
	}
	inline en_texture_t* getdiffuse( void ) { return ptextures[MT_TX_DIFFUSE]; }
	inline en_texture_t* getnormalmap( void ) { return ptextures[MT_TX_NORMALMAP]; }
	inline en_texture_t* getdetail( void ) { return ptextures[MT_TX_DETAIL]; }
	inline en_texture_t* getspecular( void ) { return ptextures[MT_TX_SPECULAR]; }
	inline en_texture_t* getluminance( void ) { return ptextures[MT_TX_LUMINANCE]; }
	inline en_texture_t* getao( void ) { return ptextures[MT_TX_AO]; }

	CString filepath;
	rs_level_t level;

	Float dt_scalex;
	Float dt_scaley;

	Uint32 int_width;
	Uint32 int_height;

	Float alpha;
	Float phong_exp;
	Float spec_factor;
	Float scale;
	Float cubemapstrength;
	Float scrollu;
	Float scrollv;

	Int32 flags;
	Int32 index;

	CString materialname;
	CString containername;
	CString containertexturename;

	Vector fullbrightcolor;

	en_texture_t* ptextures[NB_MT_TX];
};

// Path to world textures
static const Char WORLD_TEXTURES_PATH_BASE[] = "world/";
// Path to world textures
static const Char PMF_FORMAT_EXTENSION[] = ".pmf";
// Base texture path
static const Char TEXTURE_BASE_DIRECTORY_PATH[] = "textures/";

#endif // TEXTURES_SHARED_H