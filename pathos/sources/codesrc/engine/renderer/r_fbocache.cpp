/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "r_fbocache.h"
#include "system.h"
#include "textures_shared.h"
#include "texturemanager.h"
#include "r_main.h"
#include "r_glextf.h"
#include "r_fbo.h"

CFBOCache gFBOCache;

// Time until an FBO is freed
const float CFBOCache::FBO_FREE_DELAY = 30;

//====================================
//
//====================================
CFBOCache::CFBOCache(void)
{
}

//====================================
//
//====================================
CFBOCache::~CFBOCache(void)
{
	Shutdown();
}

//====================================
//
//====================================
bool CFBOCache::Init(void)
{
	return true;
}

//====================================
//
//====================================
void CFBOCache::Shutdown(void)
{
	ClearGL();
}

//====================================
//
//====================================
bool CFBOCache::InitGame(void)
{
	return true;
}

//====================================
//
//====================================
void CFBOCache::ClearGame(void)
{
	ClearGL();
}

//====================================
//
//====================================
bool CFBOCache::InitGL(void)
{
	return true;
}

//====================================
//
//====================================
void CFBOCache::ClearGL(void)
{
	if (m_cacheList.empty())
		return;

	m_cacheList.begin();
	while (!m_cacheList.end())
	{
		cache_fbo_t* pfree = m_cacheList.get();
		gGLExtF.glDeleteFramebuffers(1, &pfree->fbo.fboid);
		delete pfree;

		m_cacheList.next();
	}

	m_cacheList.clear();
}

//====================================
//
//====================================
void CFBOCache::Think(void)
{
	if (m_cacheList.empty())
		return;
	
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	assert(pTextureManager != nullptr);

	m_cacheList.begin();
	while (!m_cacheList.end())
	{
		cache_fbo_t* pnext = m_cacheList.get();
		if (pnext->freetime && (pnext->freetime + FBO_FREE_DELAY) < rns.time)
		{
			cache_fbo_t* pfree = m_cacheList.get();
			m_cacheList.remove(m_cacheList.get_link());
			m_cacheList.next();

			gGLExtF.glDeleteFramebuffers(1, &pfree->fbo.fboid);

			delete pfree;
			continue;
		}

		m_cacheList.next();
	}
}

//====================================
//
//====================================
CFBOCache::cache_fbo_t* CFBOCache::Alloc(Uint32 width, Uint32 height, bool depthbuffer)
{
	// Seek an available FBO
	m_cacheList.begin();
	while (!m_cacheList.end())
	{
		cache_fbo_t* pfbo = m_cacheList.get();
		if (pfbo->width == width && pfbo->height == height
			&& ((depthbuffer && pfbo->fbo.pdepth != nullptr)
				|| (!depthbuffer && pfbo->fbo.pdepth == nullptr)))
		{
			m_cacheList.remove(m_cacheList.get_link());
			pfbo->freetime = 0;
			return pfbo;
		}

		m_cacheList.next();
	}

	cache_fbo_t* pnew = new cache_fbo_t;
	pnew->width = width;
	pnew->height = height;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	assert(pTextureManager != nullptr);

	GLint textureBound;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBound);

	// Create screen texture
	pnew->fbo.ptexture1 = pTextureManager->GenTextureIndex(RS_WINDOW_LEVEL);
	glBindTexture(GL_TEXTURE_2D, pnew->fbo.ptexture1->gl_index);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pnew->width, pnew->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (depthbuffer)
	{
		pnew->fbo.pdepth = pTextureManager->GenTextureIndex(RS_WINDOW_LEVEL);
		glBindTexture(GL_TEXTURE_2D, pnew->fbo.pdepth->gl_index);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, pnew->width, pnew->height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	gGLExtF.glGenFramebuffers(1, &pnew->fbo.fboid);
	gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, pnew->fbo.fboid);
	gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pnew->fbo.ptexture1->gl_index, 0);

	if (depthbuffer)
		gGLExtF.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pnew->fbo.pdepth->gl_index, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	GLenum eStatus = gGLExtF.glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (eStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		Con_Printf("%s - FBO creation failed. Code returned: %d.\n", __FUNCTION__, glGetError());

		pTextureManager->DeleteAllocation(pnew->fbo.ptexture1);

		if (pnew->fbo.pdepth)
			pTextureManager->DeleteAllocation(pnew->fbo.pdepth);

		gGLExtF.glDeleteFramebuffers(1, &pnew->fbo.fboid);

		delete pnew;
		return nullptr;
	}

	if (rns.pboundfbo)
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, rns.pboundfbo->fboid);
	else
		gGLExtF.glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, textureBound);

	return pnew;
}

//====================================
//
//====================================
void CFBOCache::Free(cache_fbo_t* pfbo)
{
	pfbo->freetime = rns.time;
	m_cacheList.add(pfbo);
}
