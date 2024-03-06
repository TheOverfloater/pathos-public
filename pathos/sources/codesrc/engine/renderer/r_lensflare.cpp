/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_main.h"
#include "r_lensflare.h"
#include "com_math.h"
#include "system.h"
#include "brushmodel.h"
#include "enginestate.h"
#include "file.h"
#include "r_common.h"
#include "texturemanager.h"
#include "r_basicdraw.h"
#include "cl_entity.h"
#include "console.h"
#include "cl_utils.h"
#include "cl_pmove.h"
#include "r_glqueries.h"

// Object definition
CLensFlareRenderer gLensFlareRenderer;

// Number of glow tracelines
const Uint32 CLensFlareRenderer::GLOW_NUM_TRACES = 5;
// Glow interpolation speed
const Float CLensFlareRenderer::GLOW_INTERP_SPEED = 2;
// Sun glow default scale
const Float CLensFlareRenderer::SUN_GLOW_DEFAULT_SCALE = 2.0;

// Lens flare texture base path
const Char CLensFlareRenderer::FLARE_TEXTURE_PATH[] = "general/lensflare";
// Sun glare texture base path
const Char CLensFlareRenderer::SUNGLARE_TEXTURE_PATH[] = "general/sunglare.dds";

// Textures used by each halo
const Int32 CLensFlareRenderer::FLARE_HALO_TEXTURES[NB_FLARE_HALOS] = { 
	LF_TEX_HALO1, 
	LF_TEX_CORE, 
	LF_TEX_HALO1, 
	LF_TEX_HALO3, 
	LF_TEX_CORE,
	LF_TEX_HALO2 
};

// Scales for each individual halo
const Float CLensFlareRenderer::FLARE_HALO_SCALES[NB_FLARE_HALOS] = {
	1.0f,
	0.25f,
	0.25f,
	1.2f,
	0.25f,
	1.2f
};

// Alpha values used by halos
const Float CLensFlareRenderer::FLARE_HALO_ALPHAS[NB_FLARE_HALOS] = {
	1.0f,
	0.8f,
	0.5f,
	1.0f,
	0.8f,
	1.0f
};

// Distance scales used by halos
const Float CLensFlareRenderer::FLARE_HALO_DIST_SCALES[NB_FLARE_HALOS] = {
	0.1f,
	0.2f,
	0.25f,
	0.5f,
	0.65f,
	1.0f
};

//====================================
//
//====================================
CLensFlareRenderer::CLensFlareRenderer( void ):
	m_pBasicDraw(nullptr),
	m_pSunGlareTexture(nullptr),
	m_pCvarSunDebugPitch(nullptr),
	m_pCvarSunDebugRoll(nullptr)
{
	for(Uint32 i = 0; i < NB_LENSFLARE_TEXTURES; i++)
		m_pLensFlareTextures[i] = nullptr;

	memset(m_viewMatrix, 0, sizeof(m_viewMatrix));
}

//====================================
//
//====================================
CLensFlareRenderer::~CLensFlareRenderer( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CLensFlareRenderer::Init( void )
{
	m_pCvarSunDebugPitch = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "sun_debug_pitch", "0.0", "For debugging sun pitch");
	m_pCvarSunDebugRoll = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "sun_debug_roll", "0.0", "For debugging sun roll");

	return true;
}

//====================================
//
//====================================
void CLensFlareRenderer::Shutdown( void )
{
}

//====================================
//
//====================================
bool CLensFlareRenderer::InitGame( void )
{
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	for(Uint32 i = 0; i < NB_LENSFLARE_TEXTURES; i++)
	{
		CString path(FLARE_TEXTURE_PATH);
		path << (i+1) << ".dds";

		m_pLensFlareTextures[i] = pTextureManager->LoadTexture(path.c_str(), RS_GAME_LEVEL);
		if(!m_pLensFlareTextures[i])
			m_pLensFlareTextures[i] = pTextureManager->GetDummyTexture();
	}

	m_pSunGlareTexture = pTextureManager->LoadTexture(SUNGLARE_TEXTURE_PATH, RS_GAME_LEVEL);
	if(!m_pSunGlareTexture)
		m_pSunGlareTexture = pTextureManager->GetDummyTexture();

	return true;
}

//====================================
//
//====================================
void CLensFlareRenderer::ClearGame( void )
{
	// Reset sun flares
	if(!m_sunFlaresArray.empty())
		m_sunFlaresArray.clear();
}

//====================================
//
//====================================
void CLensFlareRenderer::DrawSunFlare( sunflare_t& sunflare )
{
	// Set up the sun entity's origin
	Vector sunForward;
	Vector sunAngles;
	
	Float sunPitchCvarValue = m_pCvarSunDebugPitch->GetValue();
	Float sunRollCvarValue = m_pCvarSunDebugRoll->GetValue();

	if(sunPitchCvarValue)
		sunAngles.x = sunPitchCvarValue;
	else
		sunAngles.x = sunflare.pitch;

	if(sunRollCvarValue)
		sunAngles.y = sunRollCvarValue;
	else
		sunAngles.y = sunflare.roll;

	Math::AngleVectors(sunAngles, &sunForward);

	Vector sunSkyPosition;
	Math::VectorMA(rns.view.v_origin, 8192, sunForward, sunSkyPosition);

	if(!DrawLensFlare(sunflare.entindex, sunSkyPosition, sunflare.color, 255, sunflare.scale, sunflare.glowstate, RenderFx_None, true, sunflare.portal))
	{
		// Clear this if we do not want to render
		if(sunflare.glowstate.lastfrac.size() > rns.renderpassidx)
			sunflare.glowstate.lastfrac[rns.renderpassidx] = 0;
	}
}

//====================================
//
//====================================
bool CLensFlareRenderer::DrawLensFlares( void )
{
	// Enable basic draw
	m_pBasicDraw = CBasicDraw::GetInstance();
	if(!m_pBasicDraw->Enable() || !m_pBasicDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", m_pBasicDraw->GetShaderError());
		return false;
	}

	m_projectionMatrix.LoadIdentity();
	m_projectionMatrix.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, (Float)0.1, 100);

	m_modelViewMatrix.LoadIdentity();
	m_modelViewMatrix.Scale(1.0f/(Float)rns.screenwidth, 1.0f/(Float)rns.screenheight, 1.0);

	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_ONE, GL_ONE);

	// Set the view matrix
	Math::AngleMatrix(rns.view.v_angles, m_viewMatrix);

	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		cl_entity_t* pEntity = rns.objects.pvisents[i];

		if(pEntity->curstate.rendertype != RT_LENSFLARE)
			continue;

		if(!DrawLensFlare(pEntity->entindex, 
			pEntity->curstate.origin, 
			pEntity->curstate.rendercolor, 
			pEntity->curstate.renderamt, 
			pEntity->curstate.scale, 
			pEntity->glowstate, 
			pEntity->curstate.renderfx,
			false,
			false))
		{
			// Clear this if we do not want to render
			if(pEntity->glowstate.lastfrac.size() > rns.renderpassidx)
				pEntity->glowstate.lastfrac[rns.renderpassidx] = 0;
		}
	}

	// Draw sun flares
	for(Uint32 i = 0; i < m_sunFlaresArray.size(); i++)
		DrawSunFlare(m_sunFlaresArray[i]);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	m_pBasicDraw->Disable();

	R_ReleaseRenderPassQueryObjects(rns.renderpassidx, GL_QUERY_LENSFLARES);
	return true;
}

//====================================
//
//====================================
bool CLensFlareRenderer::DrawLensFlare( Int32 key, const Vector& origin, const Vector& rendercolor, Float renderamt, Float scale, glowstate_t& glowstate, Int32 renderfx, bool isSun, bool portalSun )
{
	Float vOrigin [] = { origin[0], origin[1], origin[2], 1.0 };

	// Multiply with modelview
	Float viewPos[4];
	Math::MatMult4(rns.view.modelview.Transpose(), vOrigin, viewPos);

	// Multiply with projection
	Float screenCoords[4];
	Math::MatMult4(rns.view.projection.Transpose(), viewPos, screenCoords);

	// See if it's z-clipped
	if(screenCoords[3] <= 0)
		return false;

	// Calculate uniform values
	Float coordX = (screenCoords[0]/screenCoords[3])*0.5 + 0.5;
	coordX *= rns.screenwidth;

	Float coordY = (screenCoords[1]/screenCoords[3])*0.5 + 0.5;
	coordY *= rns.screenheight;
	coordY = coordY;

	// Turn into vectors
	Vector screenPosition(coordX, rns.screenheight - coordY, 0);
	Vector screenCenter(rns.screenwidth * 0.5, rns.screenheight * 0.5, 0);
	screenCenter.y = rns.screenheight - screenCenter.y;

	if(screenPosition.x < 0 || screenPosition.x > rns.screenwidth)
		return false;

	if(screenPosition.y < 0 || screenPosition.y > rns.screenheight)
		return false;

	// Discard Z component
	// Position to screen center is inverse of screen position
	Vector vectorToCenter = screenCenter - screenPosition;
	// Get length of the vector
	Float length = vectorToCenter.Length() * 0.8;

	Vector directionFromLight = vectorToCenter;
	directionFromLight.Normalize();

	Vector color;
	Math::VectorScale(rendercolor, 1.0f/255.0f, color);

	Float screenXFadeMins = rns.screenwidth * 0.1;
	Float screenXFadeMaxs = rns.screenwidth * 0.9;

	Float screenYFadeMins = rns.screenheight * 0.1;
	Float screenYFadeMaxs = rns.screenheight * 0.9;

	Float fadeX = 1.0;
	if(screenPosition.x < rns.screenwidth * 0.1)
		fadeX = screenPosition.x / (Float)screenXFadeMins;
	else if(screenPosition.x > screenXFadeMaxs)
		fadeX = 1.0 - ((screenPosition.x - screenXFadeMaxs) / (rns.screenwidth - screenXFadeMaxs));

	if(fadeX < 0)
		return false;

	Float fadeY = 1.0;
	if(screenPosition.y < rns.screenheight * 0.1)
		fadeY = screenPosition.y / (Float)screenYFadeMins;
	else if(screenPosition.y > screenYFadeMaxs)
		fadeY = 1.0 - ((screenPosition.y - screenYFadeMaxs) / (rns.screenheight - screenYFadeMaxs));

	if(fadeY < 0)
		return false;

	// Calculate occlusion
	bool useQueries = (g_pCvarOcclusionQueries->GetValue() >= 1 && (!isSun || !portalSun)) ? true : false;
	bool traceAll = (g_pCvarTraceGlow->GetValue() >= 1 || renderfx == RenderFx_TraceGlow) ? true : false;
	Float occlusionFactor = R_CalcOcclusionFactor(origin, key, 
		m_pLensFlareTextures[LF_TEX_CORE]->width, scale, 
		GLOW_INTERP_SPEED, GLOW_NUM_TRACES, GL_QUERY_LENSFLARES,
		glowstate, useQueries,
		traceAll, isSun, 
		portalSun, m_viewMatrix, 
		reinterpret_cast<void*>(this), 
		LF_PreRender, LF_DrawFunction );

	if(occlusionFactor <= 0)
		return false;

	m_pBasicDraw->SetProjection(m_projectionMatrix.GetMatrix());
	m_pBasicDraw->SetModelview(m_modelViewMatrix.GetMatrix());

	Float mainAlpha = occlusionFactor * fadeX * fadeY * renderamt / 255.0f;

	glDisable(GL_DEPTH_TEST);

	if(isSun)
	{
		m_pBasicDraw->SetColorMultiplier(2.0);
		DrawQuad(screenPosition, color, m_pSunGlareTexture, scale, mainAlpha);
		m_pBasicDraw->SetColorMultiplier(1.0);
	}

	// Draw the central one
	DrawQuad(screenPosition, color, m_pLensFlareTextures[LF_TEX_CORE], scale, mainAlpha);

	// Draw other parts
	for(Uint32 i = 0; i < NB_FLARE_HALOS; i++)
	{
		Vector flarePosition = screenPosition + directionFromLight * (length*2*FLARE_HALO_DIST_SCALES[i]);
		DrawQuad(flarePosition, color, m_pLensFlareTextures[FLARE_HALO_TEXTURES[i]], scale*FLARE_HALO_SCALES[i], mainAlpha*FLARE_HALO_ALPHAS[i]);
	}

	glEnable(GL_DEPTH_TEST);

	return true;
}

//====================================
//
//====================================
void CLensFlareRenderer::DrawQuad( const Vector& position, const Vector& color, en_texture_t* ptexture, Float scale, Float alpha )
{
	Float width = ptexture->width;
	Float height = ptexture->height;

	Float up = height * 0.5;
	Float down = -up;

	Float right = width * 0.5;
	Float left = -right;

	R_Bind2DTexture(GL_TEXTURE0_ARB, ptexture->palloc->gl_index);
	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	Vector _color(color);
	Math::VectorScale(_color, alpha, _color);

	// Draw first triangle
	Vector vpoint = position + Vector(0, up, 0) * scale;
	vpoint = vpoint + Vector(left, 0, 0) * scale;
	m_pBasicDraw->Color4f(_color.x, _color.y, _color.z, 1.0);
	m_pBasicDraw->TexCoord2f(0.0, 0.0);
	m_pBasicDraw->Vertex3f(vpoint.x, vpoint.y, -1.0f);

	vpoint = position + Vector(0, up, 0) * scale;
	vpoint = vpoint + Vector(right, 0, 0) * scale;
	m_pBasicDraw->Color4f(_color.x, _color.y, _color.z, 1.0);
	m_pBasicDraw->TexCoord2f(1.0, 0.0);
	m_pBasicDraw->Vertex3f(vpoint.x, vpoint.y, -1.0f);

	vpoint = position + Vector(0, down, 0) * scale;
	vpoint = vpoint + Vector(right, 0, 0) * scale;
	m_pBasicDraw->Color4f(_color.x, _color.y, _color.z, 1.0);
	m_pBasicDraw->TexCoord2f(1.0, 1.0);
	m_pBasicDraw->Vertex3f(vpoint.x, vpoint.y, -1.0f);

	// Draw second triangle
	vpoint = position + Vector(0, up, 0) * scale;
	vpoint = vpoint + Vector(left, 0, 0) * scale;
	m_pBasicDraw->Color4f(_color.x, _color.y, _color.z, 1.0);
	m_pBasicDraw->TexCoord2f(0.0, 0.0);
	m_pBasicDraw->Vertex3f(vpoint.x, vpoint.y, -1.0f);

	vpoint = position + Vector(0, down, 0) * scale;
	vpoint = vpoint + Vector(right, 0, 0) * scale;
	m_pBasicDraw->Color4f(_color.x, _color.y, _color.z, 1.0);
	m_pBasicDraw->TexCoord2f(1.0, 1.0);
	m_pBasicDraw->Vertex3f(vpoint.x, vpoint.y, -1.0f);

	vpoint = position + Vector(0, down, 0) * scale;
	vpoint = vpoint + Vector(left, 0, 0) * scale;
	m_pBasicDraw->Color4f(_color.x, _color.y, _color.z, 1.0);
	m_pBasicDraw->TexCoord2f(0.0, 1.0);
	m_pBasicDraw->Vertex3f(vpoint.x, vpoint.y, -1.0f);

	m_pBasicDraw->End();
}

//====================================
//
//====================================
void CLensFlareRenderer::PreDrawFunction( void )
{
	m_pBasicDraw->SetProjection(rns.view.projection.GetMatrix());
	m_pBasicDraw->SetModelview(rns.view.modelview.GetMatrix());
}

//====================================
//
//====================================
void CLensFlareRenderer::DrawFunction( const Vector& origin )
{
	m_pBasicDraw->Begin(GL_POINTS);
	m_pBasicDraw->Color4f(1.0, 1.0, 1.0, 1.0);
	m_pBasicDraw->Vertex3fv(origin);
	m_pBasicDraw->End(); 
}

//====================================
//
//====================================
void CLensFlareRenderer::SetSunFlare( entindex_t entindex, bool active, Float pitch, Float roll, Float scale, const Vector& color, bool portalSunFlare )
{
	if(!active)
	{
		for(Uint32 i = 0; i < m_sunFlaresArray.size(); i++)
		{
			if(m_sunFlaresArray[i].entindex == entindex)
			{
				m_sunFlaresArray.erase(i);
				return;
			}
		}
	}

	sunflare_t* pflare = nullptr;
	for(Uint32 i = 0; i < m_sunFlaresArray.size(); i++)
	{
		if(m_sunFlaresArray[i].entindex == entindex)
		{
			pflare = &m_sunFlaresArray[i];
			break;
		}
	}

	if(!pflare)
	{
		int32_t insertindex = m_sunFlaresArray.size();
		m_sunFlaresArray.resize(m_sunFlaresArray.size()+1);
		pflare = &m_sunFlaresArray[insertindex];
	}


	pflare->pitch = pitch;
	pflare->roll = roll;
	pflare->scale = scale;
	pflare->color = color;

	if(pflare->color.IsZero())
		pflare->color = Vector(255, 255, 255);

	if(!pflare->scale)
		pflare->scale = SUN_GLOW_DEFAULT_SCALE;

	pflare->portal = portalSunFlare;
}

//====================================
//
//====================================
void LF_PreRender( void* pContext )
{
	reinterpret_cast<CLensFlareRenderer*>(pContext)->PreDrawFunction();
}

//====================================
//
//====================================
void LF_DrawFunction( void* pContext, const Vector& origin )
{
	reinterpret_cast<CLensFlareRenderer*>(pContext)->DrawFunction(origin);
}