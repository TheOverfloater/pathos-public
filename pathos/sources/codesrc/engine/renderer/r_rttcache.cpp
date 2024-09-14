/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "r_rttcache.h"
#include "system.h"
#include "textures_shared.h"
#include "texturemanager.h"
#include "r_main.h"

// Class declaration
CRenderToTextureCache gRTTCache;

// Time until an unused texture is freed
const Float CRenderToTextureCache::TEXTURE_RELEASE_DELAY = 15;

//=======================================
// CRenderToTextureCache :: CRenderToTextureCache
// Purpose:
//=======================================
CRenderToTextureCache :: CRenderToTextureCache ( void ):
	m_iNumAllocated(0)
{
}

//=======================================
// CRenderToTextureCache :: ~CRenderToTextureCache
// Purpose:
//=======================================
CRenderToTextureCache :: ~CRenderToTextureCache ( void )
{
	Clear(RS_APP_LEVEL);
}

//=======================================
// CRenderToTextureCache :: InitGL
// Purpose:
//=======================================
void CRenderToTextureCache :: InitGL ( void )
{
	m_pCacheHeader.begin();
	while(!m_pCacheHeader.end())
	{
		rtt_texture_t* ptexture = m_pCacheHeader.get();
		CreateTexture(ptexture, ptexture->level);

		m_pCacheHeader.next();
	}
}

//=======================================
// CRenderToTextureCache :: Clear
// Purpose:
//=======================================
void CRenderToTextureCache :: Clear ( rs_level_t level )
{
	if(m_pCacheHeader.empty())
		return;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	Uint32 numDeleted = 0;
	m_pCacheHeader.begin();
	while(!m_pCacheHeader.end())
	{
		rtt_texture_t* pfree = m_pCacheHeader.get();
		if(pfree->level > level)
		{
			m_pCacheHeader.next();
			continue;
		}

		pTextureManager->DeleteAllocation(pfree->palloc);
		m_pCacheHeader.remove(m_pCacheHeader.get_link());
		delete pfree;

		numDeleted++;
		m_pCacheHeader.next();
	}

	if(level >= RS_WINDOW_LEVEL && numDeleted != m_iNumAllocated)
		Con_EPrintf("CRenderToTextureCache: numDeleted != m_iNumAllocated");
}

//=======================================
// CRenderToTextureCache :: Think
// Purpose:
//=======================================
void CRenderToTextureCache :: Think ( void )
{
	if(m_pCacheHeader.empty())
		return;

	m_pCacheHeader.begin();
	while(!m_pCacheHeader.end())
	{
		rtt_texture_t* ptexture = m_pCacheHeader.get();
		if(rns.time - ptexture->freetime > TEXTURE_RELEASE_DELAY)
		{
			Con_VPrintf("%s - Released texture with %d width and %d height.\n", __FUNCTION__, ptexture->width, ptexture->height);
			Delete(ptexture);
		}

		m_pCacheHeader.next();
	}
}

//=======================================
// CRenderToTextureCache :: Shutdown
// Purpose:
//=======================================
void CRenderToTextureCache :: Shutdown ( void )
{
	Clear(RS_APP_LEVEL);
}

//=======================================
// CRenderToTextureCache :: Alloc
// Purpose:
//=======================================
rtt_texture_t* CRenderToTextureCache :: Alloc( Uint32 width, Uint32 height, bool rectangle, GLenum internalformat, rs_level_t level )
{
	// Seek an available texture
	m_pCacheHeader.begin();
	while(!m_pCacheHeader.end())
	{
		rtt_texture_t* ptexture = m_pCacheHeader.get();
		if(ptexture->width == width && ptexture->height == height && ptexture->level == level
			&& ptexture->rectangle == rectangle && ptexture->internalformat == internalformat)
		{
			m_pCacheHeader.remove(m_pCacheHeader.get_link());
			return ptexture;
		}

		m_pCacheHeader.next();
	}

	// Allocate a new texture
	rtt_texture_t* pnew = new rtt_texture_t();
	m_iNumAllocated++;

	pnew->width = width;
	pnew->height = height;
	pnew->rectangle = rectangle;
	pnew->internalformat = internalformat;
	pnew->level = level;

	CreateTexture(pnew, level);

	return pnew;
}

//=======================================
// CRenderToTextureCache :: CreateTexture
// Purpose:
//=======================================
void CRenderToTextureCache :: CreateTexture( rtt_texture_t* ptexture, rs_level_t level )
{
	// Create the OGL texture
	GLenum target = ptexture->rectangle ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;
	ptexture->palloc = CTextureManager::GetInstance()->GenTextureIndex(level);

	GLint textureBound;
	if(!ptexture->rectangle)
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBound);
	else
		glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &textureBound);

	glBindTexture(target, ptexture->palloc->gl_index);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, ptexture->rectangle ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, ptexture->rectangle ? GL_NEAREST : GL_LINEAR);
	glTexImage2D(target, 0, ptexture->internalformat, ptexture->width, ptexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(target, textureBound);
}

//=======================================
// CRenderToTextureCache :: Free
// Purpose:
//=======================================
void CRenderToTextureCache :: Free ( rtt_texture_t* ptexture )
{
	// Add it back to the stack
	m_pCacheHeader.add(ptexture);
	// Set time we were freed
	ptexture->freetime = rns.time;
}

//=======================================
// CRenderToTextureCache :: Delete
// Purpose:
//=======================================
void CRenderToTextureCache :: Delete ( rtt_texture_t* ptexture )
{
	// Make sure it's removed from the stack
	m_pCacheHeader.remove(ptexture);
	
	// Delete it from GL memory
	CTextureManager::GetInstance()->DeleteAllocation(ptexture->palloc);
	m_iNumAllocated--;

	// Delete object
	delete ptexture;
}