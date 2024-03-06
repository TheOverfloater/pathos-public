/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_RENDERTEXCACHE_H
#define R_RENDERTEXCACHE_H

enum rs_level_t;

struct rtt_texture_t
{
	rtt_texture_t():
		index(0),
		palloc(nullptr),
		width(0),
		height(0),
		rectangle(false),
		internalformat(0),
		freetime(0),
		level(RS_LEVEL_UNDEFINED)
		{}

	Int32 index;
	struct en_texalloc_t* palloc;

	Uint32 width;
	Uint32 height;
	bool rectangle;
	GLenum internalformat;

	Double freetime;

	rs_level_t level;
};

//====================
// CRenderToTextureCache
// 
//====================
class CRenderToTextureCache
{
public:
	// Time until an unused texture is freed
	static const Float TEXTURE_RELEASE_DELAY;

public:
	CRenderToTextureCache( void );
	~CRenderToTextureCache( void );

public:
	// Clears the class
	void Clear( rs_level_t level );
	// Shuts down the class
	void Shutdown( void );

	// Initializes OpneGL objects
	void InitGL( void );

	// Allocates a render-to-texture object
	rtt_texture_t* Alloc( Uint32 width, Uint32 height, bool rectangle = false, GLenum internalformat = GL_RGBA, rs_level_t level = RS_GAME_LEVEL );
	// Releases a render-to-texture object
	void Free( rtt_texture_t* ptexture );
	// Deletes a render-to-texture object
	void Delete( rtt_texture_t* ptexture );

	// Performs think functions
	void Think( void );

private:
	// Creates an RTT OpenGL texture
	static void CreateTexture( rtt_texture_t* ptexture, rs_level_t level );

private:
	// Cache link
	CLinkedList<rtt_texture_t*> m_pCacheHeader;
	// Number of allocates RTTs
	Uint32 m_iNumAllocated;
};
extern CRenderToTextureCache gRTTCache;
#endif