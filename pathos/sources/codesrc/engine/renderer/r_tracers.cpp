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
#include "r_tracers.h"
#include "console.h"
#include "cl_main.h"
#include "r_basicdraw.h"
#include "r_common.h"

// Allocation size for tracers
const Uint32 CTracerRenderer::TRACER_ALLOC_SIZE = 256;
// Tracer texture file path
const Char CTracerRenderer::TRACER_TEXTURE_PATH[] = "general/tracer.dds";

// Object declaration
CTracerRenderer gTracers;

//====================================
//
//====================================
CTracerRenderer::CTracerRenderer( void ):
	m_pFreeTracersHeader(nullptr),
	m_pActiveTracersHeader(nullptr),
	m_pCvarDrawTracers(nullptr),
	m_pCvarGravity(nullptr)
{
}

//====================================
//
//====================================
CTracerRenderer::~CTracerRenderer( void )
{
}

//====================================
//
//====================================
bool CTracerRenderer::Init( void )
{
	m_pCvarDrawTracers = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_NONE, "r_drawtracers", "1", "Toggles drawing of tracers");
	m_pCvarGravity = gConsole.GetCVar(GRAVITY_CVAR_NAME);

	return true;
}
	
//====================================
//
//====================================
void CTracerRenderer::Shutdown( void )
{
}

//====================================
//
//====================================
bool CTracerRenderer::InitGame( void )
{
	// Allocate tracers for rendering
	AllocTracers();

	// Load tracer texture
	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	m_pTracerTexture = pTextureManager->LoadTexture(TRACER_TEXTURE_PATH, RS_GAME_LEVEL, TX_FL_NONE);
	if(!m_pTracerTexture)
	{
		Con_EPrintf("%s - Could not load '%s'.\n", __FUNCTION__, TRACER_TEXTURE_PATH);
		m_pTracerTexture = pTextureManager->GetDummyTexture();
	}

	return true;
}

//====================================
//
//====================================
void CTracerRenderer::ClearGame( void )
{
	if(m_pFreeTracersHeader)
	{
		tracer_t* pnext = m_pFreeTracersHeader;
		while(pnext)
		{
			tracer_t* pfree = pnext;
			pnext = pfree->pnext;
			delete pfree;
		}

		m_pFreeTracersHeader = nullptr;
	}

	if(m_pActiveTracersHeader)
	{
		tracer_t* pnext = m_pActiveTracersHeader;
		while(pnext)
		{
			tracer_t* pfree = pnext;
			pnext = pfree->pnext;
			delete pfree;
		}

		m_pActiveTracersHeader = nullptr;
	}
}

//====================================
//
//====================================
bool CTracerRenderer::DrawTracers( void )
{
	if(m_pCvarDrawTracers->GetValue() < 0)
		return true;

	if(!m_pActiveTracersHeader)
		return true;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable() || !pDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->SetProjection(rns.view.projection.GetMatrix());
	pDraw->SetModelview(rns.view.modelview.GetMatrix());

	R_Bind2DTexture(GL_TEXTURE0, m_pTracerTexture->palloc->gl_index);

	if(rns.fog.settings.active)
	{
		if(!pDraw->EnableFog())
		{
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			pDraw->Disable();
			return false;
		}

		// We use black fog on beams
		pDraw->SetFogParams(rns.fog.settings.color, rns.fog.settings.start, rns.fog.settings.end);
	}

	R_ValidateShader(pDraw);

	// Set modelview-projection matrix
	CMatrix modelViewProjectionMatrix;
	modelViewProjectionMatrix.SetMatrix(rns.view.modelview.GetMatrix());
	modelViewProjectionMatrix.MultMatrix(rns.view.projection.Transpose());

	// Draw the tracers
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	pDraw->Begin(CBasicDraw::DRAW_QUADS);

	// Based on code by Enko
	Vector forward, up, right;
	Math::AngleVectors(rns.view.v_angles, &forward, &right, &up);

	Math::VectorScale(right, 1.5, right);
	Math::VectorScale(up, 1.5, up);

	Uint32 nbVertexes = 0;
	tracer_t* pnext = m_pActiveTracersHeader;
	while(pnext)
	{
		Float attenuation = pnext->die - rns.time;
		if(attenuation > 0.1)
			attenuation = 0.1;

		Vector orgvel;
		Float scale = attenuation * pnext->length;
		Math::VectorMA(pnext->origin, scale, pnext->velocity, orgvel);

		Vector start = pnext->origin;
		Vector end = orgvel;

		// Calc mins maxs
		Vector mins(NULL_MINS);
		Vector maxs(NULL_MAXS);
		for(Uint32 i = 0; i < 3; i++)
		{
			if(start[i] < mins[i])
				mins[i] = start[i];

			if(start[i] > maxs[i])
				maxs[i] = start[i];

			if(end[i] < mins[i])
				mins[i] = end[i];

			if(end[i] > maxs[i])
				maxs[i] = end[i];
		}

		// Pad with width
		Math::VectorSubtract(mins, Vector(pnext->width, pnext->width, pnext->width), mins);
		Math::VectorAdd(maxs, Vector(pnext->width, pnext->width, pnext->width), maxs);

		if(rns.view.frustum.CullBBox(mins, maxs))
		{
			pnext = pnext->pnext;
			continue;
		}

		Vector screenlast, screen;
		R_WorldToScreenTransform(modelViewProjectionMatrix, start, screen);
		R_WorldToScreenTransform(modelViewProjectionMatrix, end, screenlast);

		Vector tmp;
		Math::VectorSubtract(screen, screenlast, tmp);

		tmp[0] *= pnext->width;
		tmp[1] *= pnext->width;
		tmp[2] = 0;
		tmp.Normalize();

		Vector normal;
		Math::VectorScale(up, tmp[0], normal);
		Math::VectorMA(normal, -tmp[1], right, normal);

		Vector color;
		Math::VectorScale(pnext->color, pnext->alpha*pnext->masteralpha, color);

		// Draw the quad
		Math::VectorAdd(start, normal, tmp);
		pDraw->Color4f(0, 0, 0, 0);
		pDraw->TexCoord2f(0, 0);
		pDraw->Vertex3fv(tmp);

		Math::VectorAdd(end, normal, tmp);
		pDraw->Color4f(color.x, color.y, color.z, 1.0);
		pDraw->TexCoord2f(0, 1);
		pDraw->Vertex3fv(tmp);

		Math::VectorSubtract(end, normal, tmp);
		pDraw->Color4f(color.x, color.y, color.z, 1.0);
		pDraw->TexCoord2f(1, 1);
		pDraw->Vertex3fv(tmp);

		Math::VectorSubtract(start, normal, tmp);
		pDraw->Color4f(0, 0, 0, 0);
		pDraw->TexCoord2f(1, 0);
		pDraw->Vertex3fv(tmp);

		// See if we've filled the cache
		nbVertexes += 4;
		if((nbVertexes+4) >= CBasicDraw::BASICDRAW_VERTEX_CACHE_SIZE)
		{
			pDraw->End();
			pDraw->Begin(CBasicDraw::DRAW_QUADS);
			nbVertexes = 0;
		}

		// Move to next
		pnext = pnext->pnext;
	}

	pDraw->End();

	bool result = true;
	if(rns.fog.settings.active)
		result = pDraw->DisableFog();

	pDraw->Disable();

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	// Clear any binds
	R_ClearBinds();

	return result;
}

//====================================
//
//====================================
void CTracerRenderer::Update( void )
{
	if(!m_pActiveTracersHeader)
		return;

	// Fetch and determine gravity
	Float gravity = m_pCvarGravity->GetValue();
	gravity *= cls.frametime;

	// Determine velocity
	Float scale = 1.0 - cls.frametime * 0.9;
	if(scale < 0)
		scale = 0;

	// Draw the particles
	tracer_t* pnext = m_pActiveTracersHeader;
	while(pnext)
	{
		// Free any tracers
		if(pnext->die < cls.cl_time)
		{
			tracer_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeTracer(pfree);
			continue;
		}

		// Add velocity to the tracer
		Math::VectorMA(pnext->origin, cls.frametime, pnext->velocity, pnext->origin);

		switch(pnext->type)
		{
		case TRACER_GRAVITY:
			{
				pnext->velocity[0] *= scale;
				pnext->velocity[1] *= scale;
				pnext->velocity[2] -= gravity;
				pnext->alpha = (pnext->die - cls.cl_time) * 2;
				if(pnext->alpha > 1)
					pnext->alpha = 1;
			}
			break;
		case TRACER_SLOW_GRAVITY:
			{
				pnext->velocity[0] *= scale;
				pnext->velocity[1] *= scale;
				pnext->velocity[2] -= gravity * 0.05f;
				pnext->alpha = (pnext->die - cls.cl_time) * 2;
				if(pnext->alpha > 1)
					pnext->alpha = 1;
			}
			break;
		}

		pnext = pnext->pnext;
	}
}

//====================================
//
//====================================
tracer_t* CTracerRenderer::CreateTracer( const Vector& origin, const Vector& velocity, const Vector& color, Float alpha, Float width, Float length, Float life, tracer_type_t type )
{
	tracer_t* pnew = AllocTracer();
	pnew->origin = origin;
	pnew->velocity = velocity;
	pnew->color = color;
	pnew->masteralpha = alpha;
	pnew->alpha = 1.0f;
	pnew->width = width;
	pnew->length = length;
	pnew->die = cls.cl_time + life;
	pnew->type = type;

	return pnew;
}

//====================================
//
//====================================
void CTracerRenderer::AllocTracers( void )
{
	// Allocate particles
	for(Uint32 i = 0; i < TRACER_ALLOC_SIZE; i++)
	{
		tracer_t* pnew = new tracer_t;

		if(m_pFreeTracersHeader)
			m_pFreeTracersHeader->pprev = pnew;
		pnew->pnext = m_pFreeTracersHeader;

		m_pFreeTracersHeader = pnew;
	}
}

//====================================
//
//====================================
tracer_t* CTracerRenderer::AllocTracer( void )
{
	if(!m_pFreeTracersHeader)
		AllocTracers();

	tracer_t* pnew = m_pFreeTracersHeader;
	m_pFreeTracersHeader = pnew->pnext;
	if(m_pFreeTracersHeader)
		m_pFreeTracersHeader->pprev = nullptr;

	// Clear tracer state
	(*pnew) = tracer_t();

	// Add into pointer array
	if(m_pActiveTracersHeader)
	{
		m_pActiveTracersHeader->pprev = pnew;
		pnew->pnext = m_pActiveTracersHeader;
	}
	m_pActiveTracersHeader = pnew;

	return pnew;
}

//====================================
//
//====================================
void CTracerRenderer::FreeTracer( tracer_t* ptracer )
{
	if(!ptracer->pprev) 
		m_pActiveTracersHeader = ptracer->pnext;
	else 
		ptracer->pprev->pnext = ptracer->pnext;

	if(ptracer->pnext) 
		ptracer->pnext->pprev = ptracer->pprev;

	// Clear this particle
	(*ptracer) = tracer_t();

	// Link to freed entities
	ptracer->pnext = m_pFreeTracersHeader;
	if(m_pFreeTracersHeader)
		m_pFreeTracersHeader->pprev = ptracer;

	m_pFreeTracersHeader = ptracer;
}