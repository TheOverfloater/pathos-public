/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_entity.h"
#include "file_interface.h"
#include "r_main.h"
#include "r_basicdraw.h"
#include "r_beams.h"
#include "r_main.h"
#include "r_common.h"

#include "cvar.h"
#include "console.h"
#include "system.h"
#include "cl_main.h"
#include "com_math.h"
#include "file.h"
#include "enginestate.h"
#include "modelcache.h"
#include "cl_utils.h"
#include "sprite.h"
#include "textures_shared.h"

//
// Beam rendering code
// Special Thanks to Enko for providing the sources I referenced to recode this
//

// Number of beams allocated at once
const Uint32 CBeamRenderer::BEAM_ALLOC_SIZE = 128;
// Number of beam positions allocated at once
const Uint32 CBeamRenderer::BEAM_POSITIONS_ALLOC_SIZE = 4096;
// Distance at which a new beam position is allocated for BEAMFOLLOW
const Float CBeamRenderer::BEAM_POSITION_SEGMENT_DISTANCE = 32.0f;
// Minimum number of beam segments
const Uint32 CBeamRenderer::MIN_NB_BEAM_SEGMENTS = 3;
// Fraction of length at which end/start fades
const Float CBeamRenderer::BEAM_FADE_FRACTION = 0.1;

// Object definition
CBeamRenderer gBeamRenderer;

//====================================
//
//====================================
CBeamRenderer::CBeamRenderer( void ):
	m_pFreeBeamHeader(nullptr),
	m_pActiveBeamHeader(nullptr),
	m_lastBeamIndex(0),
	m_pFreeBeamPositionsHeader(nullptr),
	m_pBasicDraw(nullptr),
	m_pCvarDrawBeams(nullptr)
{
	// Allocate initial nb of beams
	AllocateBeams();

	// Allocate beam positions
	AllocateBeamPositions();
}

//====================================
//
//====================================
CBeamRenderer::~CBeamRenderer( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CBeamRenderer::Init( void )
{
	m_pCvarDrawBeams = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_NONE, "r_drawbeams", "1", "Toggles drawing of beams");
	return true;
}

//====================================
//
//====================================
void CBeamRenderer::Shutdown( void )
{
	ClearGame();

	if(m_pFreeBeamHeader)
	{
		beam_t* pbeam = m_pFreeBeamHeader;
		while(pbeam)
		{
			beam_t* pfree = pbeam;
			pbeam = pfree->pnext;

			delete pfree;
		}
		m_pFreeBeamHeader = nullptr;
	}

	if(m_pFreeBeamPositionsHeader)
	{
		beam_position_t* pposition = m_pFreeBeamPositionsHeader;
		while(pposition)
		{
			beam_position_t* pfree = pposition;
			pposition = pfree->pnext;

			delete pfree;
		}
		m_pFreeBeamPositionsHeader = nullptr;
	}
}

//====================================
//
//====================================
bool CBeamRenderer::InitGL( void )
{
	return true;
}

//====================================
//
//====================================
void CBeamRenderer::ClearGL( void )
{
}

//====================================
//
//====================================
bool CBeamRenderer::InitGame( void )
{
	return true;
}

//====================================
//
//====================================
void CBeamRenderer::ClearGame( void )
{
	FreeActiveBeams();
}

//====================================
//
//====================================
void CBeamRenderer::FreeActiveBeams( void )
{
	if(!m_pActiveBeamHeader)
		return;

	beam_t* pnext = m_pActiveBeamHeader;
	while(pnext)
	{
		beam_t* pfree = pnext;
		pnext = pfree->pnext;

		FreeBeam(pfree);
	}
}

//====================================
//
//====================================
void CBeamRenderer::AllocateBeams( void )
{
	for(Uint32 i = 0; i < BEAM_ALLOC_SIZE; i++)
	{
		beam_t* pnew = new beam_t;

		if(m_pFreeBeamHeader)
		{
			m_pFreeBeamHeader->pprev = pnew;
			pnew->pnext = m_pFreeBeamHeader;
		}
		else
			pnew->pnext = nullptr;

		m_pFreeBeamHeader = pnew;
	}
}

//====================================
//
//====================================
void CBeamRenderer::FreeBeam( beam_t* pbeam )
{
	if(!pbeam->pprev) 
		m_pActiveBeamHeader = pbeam->pnext;
	else 
		pbeam->pprev->pnext = pbeam->pnext;

	if(pbeam->pnext) 
		pbeam->pnext->pprev = pbeam->pprev;

	if(pbeam->ppositions)
		FreeActiveBeamPositions(pbeam);

	// Clear this beam
	(*pbeam) = beam_t();

	// Link to freed beams
	pbeam->pnext = m_pFreeBeamHeader;
	if(m_pFreeBeamHeader)
		m_pFreeBeamHeader->pprev = pbeam;

	m_pFreeBeamHeader = pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::AllocBeam( void )
{
	// Allocate if needed
	if(!m_pFreeBeamHeader)
		AllocateBeams();

	beam_t* pnew = m_pFreeBeamHeader;
	m_pFreeBeamHeader = pnew->pnext;
	if(m_pFreeBeamHeader)
		m_pFreeBeamHeader->pprev = nullptr;

	// Clear beam state
	(*pnew) = beam_t();

	// Set beam index
	pnew->index = m_lastBeamIndex;
	m_lastBeamIndex++;

	// Add beam into pointer array
	if(m_pActiveBeamHeader)
	{
		m_pActiveBeamHeader->pprev = pnew;
		pnew->pnext = m_pActiveBeamHeader;
	}
	m_pActiveBeamHeader = pnew;
	return pnew;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::FindBeamByEntity( const cl_entity_t* pentity )
{
	if(!pentity)
		return nullptr;

	beam_t* pnext = m_pActiveBeamHeader;
	while(pnext)
	{
		if(pnext->pserverentity && pnext->pserverentity == pentity)
			return pnext;

		pnext = pnext->pnext;
	}

	return nullptr;
}

//====================================
//
//====================================
void CBeamRenderer::FreeActiveBeamPositions( beam_t* pbeam )
{
	if(!pbeam->ppositions)
		return;

	beam_position_t* pnext = pbeam->ppositions;
	while(pnext)
	{
		beam_position_t* pfree = pnext;
		pnext = pfree->pnext;

		FreeBeamPosition(pbeam, pfree);
	}
}

//====================================
//
//====================================
void CBeamRenderer::AllocateBeamPositions( void )
{
	for(Uint32 i = 0; i < BEAM_ALLOC_SIZE; i++)
	{
		beam_position_t* pnew = new beam_position_t;

		if(m_pFreeBeamPositionsHeader)
		{
			m_pFreeBeamPositionsHeader->pprev = pnew;
			pnew->pnext = m_pFreeBeamPositionsHeader;
		}
		else
			pnew->pnext = nullptr;

		m_pFreeBeamPositionsHeader = pnew;
	}
}

//====================================
//
//====================================
void CBeamRenderer::FreeBeamPosition( beam_t* pbeam, beam_position_t* pposition )
{
	if(!pposition->pprev) 
		pbeam->ppositions = pposition->pnext;
	else 
		pposition->pprev->pnext = pposition->pnext;

	if(pposition->pnext) 
		pposition->pnext->pprev = pposition->pprev;

	// Clear this beam
	(*pposition) = beam_position_t();

	// Link to freed beams
	pposition->pnext = m_pFreeBeamPositionsHeader;
	if(m_pFreeBeamPositionsHeader)
		m_pFreeBeamPositionsHeader->pprev = pposition;

	m_pFreeBeamPositionsHeader = pposition;
}

//====================================
//
//====================================
beam_position_t* CBeamRenderer::AllocBeamPosition( beam_t* pbeam )
{
	// Allocate if needed
	if(!m_pFreeBeamPositionsHeader)
		AllocateBeamPositions();

	beam_position_t* pnew = m_pFreeBeamPositionsHeader;
	m_pFreeBeamPositionsHeader = pnew->pnext;
	if(m_pFreeBeamPositionsHeader)
		m_pFreeBeamPositionsHeader->pprev = nullptr;

	// Clear beam state
	(*pnew) = beam_position_t();

	// Add beam into pointer array
	if(pbeam->ppositions)
	{
		pbeam->ppositions->pprev = pnew;
		pnew->pnext = pbeam->ppositions;
	}
	pbeam->ppositions = pnew;

	return pnew;
}

//====================================
//
//====================================
bool CBeamRenderer::CullBeam( const Vector& src, const Vector& end, bool pvsOnly )
{
	if(!ens.pworld)
		return false;

	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(src[i] < end[i])
		{
			mins[i] = src[i];
			maxs[i] = end[i];
		}
		else
		{
			mins[i] = end[i];
			maxs[i] = src[i];
		}
	}

	// Avoid zero size
	Math::VectorSubtract(mins, Vector(1, 1, 1), mins);
	Math::VectorAdd(maxs, Vector(1, 1, 1), maxs);

	CArray<Uint32> leafsArray;
	Mod_FindTouchedLeafs(ens.pworld, leafsArray, mins, maxs, ens.pworld->pnodes);
	if(leafsArray.empty())
		return false;

	if(!Common::CheckVisibility(leafsArray, rns.pvisbuffer))
		return true;

	if(!pvsOnly)
	{
		if(rns.view.frustum.CullBBox(mins, maxs))
			return true;
	}

	return false;
}

//====================================
//
//====================================
void CBeamRenderer::Update( void )
{
	if(!m_pActiveBeamHeader)
		return;

	// Retire any old beams
	cl_entity_t* pplayer = CL_GetLocalPlayer();

	beam_t* pnext = m_pActiveBeamHeader;
	while(pnext)
	{
		// Free server entity beams not in PVS scope
		if(pnext->pserverentity && pnext->pserverentity->curstate.msg_num != pplayer->curstate.msg_num)
		{
			beam_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeBeam(pfree);
			continue;
		}

		// Free any non-permanent beams that have run out of time
		if(!(pnext->flags & FL_BEAM_FOREVER) && pnext->die < cls.cl_time || (pnext->flags & FL_BEAM_KILLED))
		{
			beam_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeBeam(pfree);
			continue;
		}

		pnext = pnext->pnext;
	}
}

//====================================
//
//====================================
bool CBeamRenderer::DrawBeams( void )
{
	if(m_pCvarDrawBeams->GetValue() < 1.0)
		return true;

	if(!m_pActiveBeamHeader)
		return true;

	// Set modelview-projection matrix
	m_modelViewProjectionMatrix.SetMatrix(rns.view.modelview.GetMatrix());
	m_modelViewProjectionMatrix.MultMatrix(rns.view.projection.Transpose());

	// Enable basic draw
	m_pBasicDraw = CBasicDraw::GetInstance();
	if(!m_pBasicDraw->Enable() || !m_pBasicDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", m_pBasicDraw->GetShaderError());
		return false;
	}

	m_pBasicDraw->SetProjection(rns.view.projection.GetMatrix());
	m_pBasicDraw->SetModelview(rns.view.modelview.GetMatrix());
	m_pBasicDraw->SetColorMultiplier(2.0);

	if(rns.fog.settings.active)
	{
		if(!m_pBasicDraw->EnableFog())
		{
			Sys_ErrorPopup("Shader error: %s.\n", m_pBasicDraw->GetShaderError());
			m_pBasicDraw->Disable();
			return false;
		}

		// We use black fog on beams
		m_pBasicDraw->SetFogParams(ZERO_VECTOR, rns.fog.settings.start, rns.fog.settings.end);
	}

	glDisable(GL_CULL_FACE);

	if(m_pActiveBeamHeader)
	{
		// Draw tempentity beams
		beam_t* pnext = m_pActiveBeamHeader;
		while(pnext)
		{
			if(pnext->flags & FL_BEAM_KILLED)
			{
				pnext = pnext->pnext;
				continue;
			}

			DrawBeam(pnext);
			pnext = pnext->pnext;
		}
	}

	glEnable(GL_CULL_FACE);

	bool result = true;
	if(rns.fog.settings.active)
		result = m_pBasicDraw->DisableFog();

	m_pBasicDraw->SetColorMultiplier(1.0);
	m_pBasicDraw->Disable();
	return result;
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamSegments( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments, Int32 flags, Int32 beamindex )
{
	// Don't draw invalid beams
	if(numsegments < 2)
		return;

	// Cap at max
	Uint32 _numsegments = numsegments;
	if(_numsegments > MAX_BEAM_SEGMENTS)
		_numsegments = MAX_BEAM_SEGMENTS;

	Float length = delta.Length() * 0.01;
	if(length < 0.5)
		length = 0.5;

	Float div = 1.0f / (_numsegments - 1);
	Float step = length*div;
	Float tcy1 = SDL_fmod(frequency*speed, 1.0);

	Float _scale;
	if(flags & FL_BEAM_SINENOISE)
	{
		if(_numsegments < 16)
		{
			_numsegments = 16;
			length = 1.6f;
		}
		else
			length = _numsegments * 0.1f;

		_scale = scale * 100.0f;
	}
	else
		_scale = scale * length;

	Vector start, screenlast, screenstart;
	R_WorldToScreenTransform(m_modelViewProjectionMatrix, src, screenlast);
	Math::VectorMA(src, div, delta, start);
	R_WorldToScreenTransform(m_modelViewProjectionMatrix, start, screenstart);

	Vector tmp;
	Math::VectorSubtract(screenstart, screenlast, tmp);
	tmp[2] = 0.0; // discard z as we're in screen space

	Vector normal;
	tmp.Normalize();
	Math::VectorScale(rns.view.v_up, tmp[0], normal);
	Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

	Vector coord1_1, coord1_2;
	Math::VectorMA(src, width, normal, coord1_1);
	Math::VectorMA(src, -width, normal, coord1_2);

	// Resample the noise waveform
	Int32 noisestep = (Int32)((Float)MAX_BEAM_SEGMENTS * div * 65536.0f);

	Int32 noiseindex;
	if(flags & FL_BEAM_SINENOISE)
		noiseindex = 0;
	else
		noiseindex = noisestep;

	Float brightness1;
	if(flags & FL_BEAM_SHADEIN || !(flags & FL_BEAM_NO_FADE))
		brightness1 = 0.0f;
	else
		brightness1 = 1.0f;

	Vector beamforward = delta;
	beamforward.Normalize();

	Vector beamright, beamup;
	Math::GetUpRight(beamforward, beamright, beamup);

	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	for(Uint32 i = 1; i < _numsegments; i++)
	{
		Float fraction = i * div;

		Float brightness2 = 1.0;
		if(flags & FL_BEAM_SHADEIN)
		{
			// Fade from start to end
			brightness2 = fraction;
		}
		else if(flags & FL_BEAM_SHADEOUT)
		{
			// Fade from end to start
			brightness2 = 1.0 - fraction;
		}
		else if(!(flags & FL_BEAM_NO_FADE))
		{
			// Apply some small fade to the end and beginning
			Float endsfraction = (Float)(i - 1) / (Float)(_numsegments - 2);
			Float startfade = clamp(endsfraction / BEAM_FADE_FRACTION, 0.0, 1.0);
			Float endfade = clamp((1.0 - endsfraction) / BEAM_FADE_FRACTION, 0.0, 1.0);
			brightness2 *= startfade * endfade;
		}
		else
		{
			// No fading whatsoever
			brightness2 = 1.0;
		}

		Math::VectorMA(src, fraction, delta, start);

		// Apply noise if any
		ApplySegmentNoise(start, beamindex, i, _scale, _numsegments, beamup, beamright, flags);

		R_WorldToScreenTransform(m_modelViewProjectionMatrix, start, screenstart);
		Math::VectorSubtract(screenstart, screenlast, tmp);

		// We don't need Z in screen-space
		tmp[2] = 0;
		tmp.Normalize();

		Math::VectorScale(rns.view.v_up, tmp[0], normal);
		Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

		// Calculate final coordinates
		Vector coord2_1, coord2_2;
		Math::VectorMA(start, width, normal, coord2_1);
		Math::VectorMA(start, -width, normal, coord2_2);

		// Calculate texcoord
		Float tcy2 = tcy1 + step;

		// triangle 1
		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(0, tcy1);
		m_pBasicDraw->Vertex3fv(coord1_2);

		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(1, tcy1);
		m_pBasicDraw->Vertex3fv(coord1_1);

		m_pBasicDraw->Brightness1f(brightness2);
		m_pBasicDraw->TexCoord2f(0, tcy2);
		m_pBasicDraw->Vertex3fv(coord2_2);

		// triangle 2
		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(1, tcy1);
		m_pBasicDraw->Vertex3fv(coord1_1);

		m_pBasicDraw->Brightness1f(brightness2);
		m_pBasicDraw->TexCoord2f(0, tcy2);
		m_pBasicDraw->Vertex3fv(coord2_2);

		m_pBasicDraw->Brightness1f(brightness2);
		m_pBasicDraw->TexCoord2f(1, tcy2);
		m_pBasicDraw->Vertex3fv(coord2_1);

		screenlast = screenstart;
		noiseindex += noisestep;
		brightness1 = brightness2;
		tcy1 = SDL_fmod(tcy2, 1.0);

		coord1_1 = coord2_1;
		coord1_2 = coord2_2;
	}

	m_pBasicDraw->End();
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamTesla( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Float endwidth, Uint32 numsegments, Int32 flags, Int32 beamindex )
{
	// Don't draw invalid beams
	if(numsegments < 2)
		return;

	// Cap at max
	Uint32 _numsegments = numsegments;
	if(_numsegments > MAX_BEAM_SEGMENTS)
		_numsegments = MAX_BEAM_SEGMENTS;

	Float length = delta.Length() * 0.01;
	if(length < 0.5)
		length = 0.5;

	Float div = 1.0f / (_numsegments - 1);
	Float step = length*div;
	Float tcy1 = SDL_fmod(frequency*speed, 1.0);

	Float _scale;
	if(flags & FL_BEAM_SINENOISE)
	{
		if(_numsegments < 16)
		{
			_numsegments = 16;
			length = 1.6f;
		}
		else
			length = _numsegments * 0.1f;

		_scale = scale * 100.0f;
	}
	else
		_scale = scale * length;

	Vector start, screenlast, screenstart;
	R_WorldToScreenTransform(m_modelViewProjectionMatrix, src, screenlast);
	Math::VectorMA(src, div, delta, start);
	R_WorldToScreenTransform(m_modelViewProjectionMatrix, start, screenstart);

	Vector tmp;
	Math::VectorSubtract(screenstart, screenlast, tmp);
	tmp[2] = 0.0; // discard z as we're in screen space

	Vector normal;
	tmp.Normalize();
	Math::VectorScale(rns.view.v_up, tmp[0], normal);
	Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

	Vector coord1_1, coord1_2;
	Math::VectorMA(src, width, normal, coord1_1);
	Math::VectorMA(src, -width, normal, coord1_2);

	// Resample the noise waveform
	Int32 noisestep = (Int32)((Float)MAX_BEAM_SEGMENTS * div * 65536.0f);

	Int32 noiseindex;
	if(flags & FL_BEAM_SINENOISE)
		noiseindex = 0;
	else
		noiseindex = noisestep;

	Float brightness1;
	if(flags & FL_BEAM_SHADEIN || !(flags & FL_BEAM_NO_FADE))
		brightness1 = 0.0f;
	else
		brightness1 = 1.0f;

	R_ValidateShader(m_pBasicDraw);

	// Number of branches
	Uint32 numbranches = 0;
	Float branchwidth = 0;
	Float branchendwidth = 0;
	Vector branchstart, branchend;

	Vector beamforward = delta;
	beamforward.Normalize();

	Vector beamright, beamup;
	Math::GetUpRight(beamforward, beamright, beamup);

	m_pBasicDraw->Begin(GL_TRIANGLES);
	for(Uint32 i = 1; i < _numsegments; i++)
	{
		Float fraction = i * div;

		Float brightness2 = 1.0;
		if(flags & FL_BEAM_SHADEIN)
		{
			// Fade from start to end
			brightness2 = fraction;
		}
		else if(flags & FL_BEAM_SHADEOUT)
		{
			// Fade from end to start
			brightness2 = 1.0 - fraction;
		}
		else if(!(flags & FL_BEAM_NO_FADE))
		{
			// Apply some small fade to the end and beginning
			Float endsfraction = (Float)(i - 1) / (Float)(_numsegments - 2);
			Float startfade = clamp(endsfraction / BEAM_FADE_FRACTION, 0.0, 1.0);
			Float endfade = clamp((1.0 - endsfraction) / BEAM_FADE_FRACTION, 0.0, 1.0);
			brightness2 *= startfade * endfade;
		}
		else
		{
			// No fading whatsoever
			brightness2 = 1.0;
		}

		Math::VectorMA(src, fraction, delta, start);

		// Apply noise if any
		ApplySegmentNoise(start, beamindex, i, _scale, _numsegments, beamup, beamright, flags);

		R_WorldToScreenTransform(m_modelViewProjectionMatrix, start, screenstart);
		Math::VectorSubtract(screenstart, screenlast, tmp);

		// We don't need Z in screen-space
		tmp[2] = 0;
		tmp.Normalize();

		Math::VectorScale(rns.view.v_up, tmp[0], normal);
		Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

		// Calculate segment width
		Float segmentwidth;
		if(endwidth == width)
			segmentwidth = width*2;
		else
			segmentwidth = ((fraction * (endwidth-width))+width)*2;

		for(Uint32 j = 0; j < numbranches; j++)
			segmentwidth *= 0.5;

		// Calculate final coordinates
		Vector coord2_1, coord2_2;
		Math::VectorMA(start, segmentwidth, normal, coord2_1);
		Math::VectorMA(start, -segmentwidth, normal, coord2_2);

		// Calculate texcoord
		Float tcy2 = tcy1 + step;

		// triangle 1
		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(0, tcy1);
		m_pBasicDraw->Vertex3fv(coord1_2);

		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(1, tcy1);
		m_pBasicDraw->Vertex3fv(coord1_1);

		m_pBasicDraw->Brightness1f(brightness2);
		m_pBasicDraw->TexCoord2f(0, tcy2);
		m_pBasicDraw->Vertex3fv(coord2_2);

		// triangle 2
		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(1, tcy1);
		m_pBasicDraw->Vertex3fv(coord1_1);

		m_pBasicDraw->Brightness1f(brightness2);
		m_pBasicDraw->TexCoord2f(0, tcy2);
		m_pBasicDraw->Vertex3fv(coord2_2);

		m_pBasicDraw->Brightness1f(brightness2);
		m_pBasicDraw->TexCoord2f(1, tcy2);
		m_pBasicDraw->Vertex3fv(coord2_1);

		screenlast = screenstart;
		noiseindex += noisestep;
		brightness1 = brightness2;
		tcy1 = SDL_fmod(tcy2, 1.0);

		coord1_1 = coord2_1;
		coord1_2 = coord2_2;

		if(i == (Uint32)(numsegments * 0.5))
		{
			branchwidth = segmentwidth  * 0.25;
			if(branchwidth > 0.25)
			{
				numbranches++;

				branchstart = start;
				branchend = src + delta + (rns.view.v_up * 32) + (rns.view.v_right * 32);
				branchend -= branchstart;

				branchendwidth = endwidth;
				for(Uint32 j = 0; j < numbranches; j++)
					branchendwidth *= 0.5;
			}
		}
	}

	m_pBasicDraw->End();

	if(numbranches > 0)
	{
		Int32 branchflags = (flags & ~FL_BEAM_NO_FADE) | FL_BEAM_SHADEOUT;
		DrawBeamTesla(branchstart, branchend, branchwidth, scale, frequency, speed, noisespeed, branchendwidth, numsegments, branchflags, beamindex);
	}
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamTorus( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments, Int32 beamindex, Int32 flags )
{
	// Skip invalid beams
	if(numsegments < 2)
		return;

	// Cap at max
	Uint32 _numsegments = numsegments;
	if(_numsegments > MAX_BEAM_SEGMENTS)
		_numsegments = MAX_BEAM_SEGMENTS;

	Float length = delta.Length() * 0.01;
	if(length < 0.5)
		length = 0.5;

	Float div = 1.0f / (_numsegments - 1);
	Float step = length*div;
	Float last = SDL_fmod(frequency*speed, 1.0);
	Float _scale = scale * length;

	// Resample the noise waveform
	Int32 noisestep = (Int32)((Float)MAX_BEAM_SEGMENTS * div * 65536.0f);
	Int32 noiseindex = 0;

	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	Float tc1y = 0;
	Vector screenlast, coord1_1, coord1_2;
	for(Uint32 i = 0; i < (_numsegments + 1); i++)
	{
		Float frac = i * div;
		Float s = SDL_sinf(frac * 2 * M_PI);
		Float c = SDL_cosf(frac * 2 * M_PI);

		Vector start;
		start[0] = s * frequency * delta[2] + src[0];
		start[1] = c * frequency * delta[2] + src[1];
		start[2] = src[2];

		// Apply noise if any
		ApplySegmentNoise(start, beamindex, i, _scale, _numsegments, rns.view.v_up, rns.view.v_right, flags);

		// Transform to screen space
		Vector screenstart;
		R_WorldToScreenTransform(m_modelViewProjectionMatrix, start, screenstart);

		if(i > 0)
		{
			Vector tmp;
			Math::VectorSubtract(screenstart, screenlast, tmp);
			tmp[2] = 0;
			tmp.Normalize();

			Vector normal;
			Math::VectorScale(rns.view.v_up, tmp[0], normal);
			Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

			Vector coord2_1, coord2_2;
			Math::VectorMA(start, width, normal, coord2_1);
			Math::VectorMA(start, -width, normal, coord2_2);

			last = last + step;
			Float tc2y = last;

			if(i > 1)
			{
				// triangle 1
				m_pBasicDraw->TexCoord2f(0, tc1y);
				m_pBasicDraw->Vertex3fv(coord1_2);

				m_pBasicDraw->TexCoord2f(1, tc1y);
				m_pBasicDraw->Vertex3fv(coord1_1);

				m_pBasicDraw->TexCoord2f(0, tc2y);
				m_pBasicDraw->Vertex3fv(coord2_2);

				// triangle 2
				m_pBasicDraw->TexCoord2f(1, tc1y);
				m_pBasicDraw->Vertex3fv(coord1_1);

				m_pBasicDraw->TexCoord2f(0, tc2y);
				m_pBasicDraw->Vertex3fv(coord2_2);

				m_pBasicDraw->TexCoord2f(1, tc2y);
				m_pBasicDraw->Vertex3fv(coord2_1);
			}

			// For next segment draw
			coord1_1 = coord2_1;
			coord1_2 = coord2_2;
			tc1y = tc2y;
		}

		screenlast = screenstart;
		last = SDL_fmod(last, 1.0);
		noiseindex += noisestep;
	}

	m_pBasicDraw->End();
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamDisk( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments )
{
	// Skip invalid beams
	if(numsegments < 2)
		return;

	// Cap at max
	Uint32 _numsegments = numsegments;
	if(_numsegments > MAX_BEAM_SEGMENTS)
		_numsegments = MAX_BEAM_SEGMENTS;

	Float length = delta.Length() * 0.01;
	if(length < 0.5)
		length = 0.5;

	Float div = 1.0f / (_numsegments - 1);
	Float step = length*div;
	Float last = SDL_fmod(frequency*speed, 1.0);
	Float w = frequency * delta[2];

	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	Float tc1y = 0;
	Vector coord1_1, coord1_2;
	for(Uint32 i = 0; i < _numsegments; i++)
	{
		Float frac = i * div;
		Float v = (frac+frac)*M_PI;
		Float s = SDL_sinf(v);
		Float c = SDL_cosf(v);

		Vector coord2_1, coord2_2;
		coord2_1 = src;
		coord2_2[0] = s * w + src[0];
		coord2_2[1] = c * w + src[1];
		coord2_2[2] = src[2];
		Float tc2y = last;
		
		if(i > 0)
		{
			// triangle 1
			m_pBasicDraw->TexCoord2f(0, tc1y);
			m_pBasicDraw->Vertex3fv(coord1_2);

			m_pBasicDraw->TexCoord2f(1, tc1y);
			m_pBasicDraw->Vertex3fv(coord1_1);

			m_pBasicDraw->TexCoord2f(0, tc2y);
			m_pBasicDraw->Vertex3fv(coord2_2);

			// triangle 2
			m_pBasicDraw->TexCoord2f(1, tc1y);
			m_pBasicDraw->Vertex3fv(coord1_1);

			m_pBasicDraw->TexCoord2f(0, tc2y);
			m_pBasicDraw->Vertex3fv(coord2_2);

			m_pBasicDraw->TexCoord2f(1, tc2y);
			m_pBasicDraw->Vertex3fv(coord2_1);
		}
		
		coord1_1 = coord2_1;
		coord1_2 = coord2_2;
		tc1y = last;

		last = SDL_fmod(last + step, 1.0);
	}

	m_pBasicDraw->End();
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamCylinder( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments )
{
	// Skip invalid beams
	if(numsegments < 2)
		return;

	// Cap at max
	Uint32 _numsegments = numsegments;
	if(_numsegments > MAX_BEAM_SEGMENTS)
		_numsegments = MAX_BEAM_SEGMENTS;

	Float length = delta.Length() * 0.01;
	if(length < 0.5)
		length = 0.5;

	Float div = 1.0f / (_numsegments - 1);
	Float step = length*div;
	Float last = SDL_fmod(frequency*speed, 1.0);

	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	Float tc1y = 0;
	Vector coord1_1, coord1_2;
	for(Uint32 i = 0; i < (_numsegments + 1); i++)
	{
		Float frac = i*div;
		Float v = frac * 2 * M_PI;

		Float s = SDL_sin(v);
		Float c = SDL_cos(v);

		Vector coord2_1;
		coord2_1[0] = s * frequency * delta[2] + src[0];
		coord2_1[1] = c * frequency * delta[2] + src[1];
		coord2_1[2] = width + src[2];

		Vector coord2_2;
		coord2_2[0] = s * frequency * (delta[2] + width) + src[0];
		coord2_2[1] = c * frequency * (delta[2] + width) + src[1];
		coord2_2[2] = src[2] - width;

		Float tc2y = last;

		if(i > 1)
		{
			// triangle 1
			m_pBasicDraw->TexCoord2f(0, tc1y);
			m_pBasicDraw->Vertex3fv(coord1_2);

			m_pBasicDraw->TexCoord2f(1, tc1y);
			m_pBasicDraw->Vertex3fv(coord1_1);

			m_pBasicDraw->TexCoord2f(0, tc2y);
			m_pBasicDraw->Vertex3fv(coord2_2);

			// triangle 2
			m_pBasicDraw->TexCoord2f(1, tc1y);
			m_pBasicDraw->Vertex3fv(coord1_1);

			m_pBasicDraw->TexCoord2f(0, tc2y);
			m_pBasicDraw->Vertex3fv(coord2_2);

			m_pBasicDraw->TexCoord2f(1, tc2y);
			m_pBasicDraw->Vertex3fv(coord2_1);
		}

		coord1_1 = coord2_1;
		coord1_2 = coord2_2;
		tc1y = tc2y;

		last = SDL_fmod(last+step, 1.0);
	}

	m_pBasicDraw->End();
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamRing( const Vector& src, const Vector& delta, Float width, Float amplitude, Float frequency, Float speed, Float noisespeed, Uint32 numsegments, Int32 beamindex, Int32 flags )
{
	// Skip invalid beams
	if(numsegments < 2)
		return;

	// Cap at max
	Uint32 _numsegments = numsegments;
	if(_numsegments > MAX_BEAM_SEGMENTS * 8)
		_numsegments = MAX_BEAM_SEGMENTS * 8;

	Float length = delta.Length() * M_PI * 0.01;
	if(length < 0.5)
		length = 0.5;

	Float div = 1.0f / (_numsegments - 1);
	Float step = length * div / 8.0f;
	Float last = SDL_fmod(frequency*speed, 1.0);
	Float scale = amplitude * length / 8.0f;

	Int32 noisestep = (Int32)(MAX_BEAM_SEGMENTS * div * 65536.0f) * 8;
	Int32 noiseindex = 0;
	
	Vector _delta, center;
	Math::VectorScale(delta, 0.5, _delta);
	Math::VectorAdd(src, _delta, center);

	Vector xaxis = delta;
	Float radius = xaxis.Length();

	Vector last1;
	last1[0] = last1[1] = radius;
	last1[2] = scale;

	Vector tmp, screenstart;
	Math::VectorAdd(center, last1, tmp);
	Math::VectorSubtract(center, last1, screenstart);

	if(!ens.pworld)
		return;

	if(CullBeam(screenstart, tmp, false))
		return;

	Vector yaxis;
	yaxis[0] = xaxis[1];
	yaxis[1] = -xaxis[0];
	yaxis[2] = 0.0f;
	yaxis.Normalize();

	Math::VectorScale(yaxis, radius, yaxis);

	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	Float tc1y = 0;
	Vector screenlast;
	Vector coord1_1, coord1_2;
	for(Uint32 i = 0; i < _numsegments + 1; i++)
	{
		Float frac = i * div;

		Float x = SDL_sinf(frac*2*M_PI);
		Float y = SDL_cosf(frac*2*M_PI);

		Vector start;
		for(Uint32 k = 0; k < 3; k++)
			start[k] = xaxis[k] * x + yaxis[k] * y + center[k];

		// Apply noise if any
		ApplySegmentNoise(start, beamindex, i, scale, _numsegments, rns.view.v_up, rns.view.v_right, flags);

		R_WorldToScreenTransform(m_modelViewProjectionMatrix, start, screenstart);

		if(i > 0)
		{
			Math::VectorSubtract(screenstart, screenlast, tmp);
			tmp[2] = 0;
			tmp.Normalize();

			Vector normal;
			Math::VectorScale(rns.view.v_up, tmp[0], normal);
			Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

			Vector coord2_1, coord2_2;
			Math::VectorMA(start, width, normal, coord2_1);
			Math::VectorMA(start, -width, normal, coord2_2);
			last += step;

			if(i > 1)
			{
				// triangle 1
				m_pBasicDraw->TexCoord2f(1, tc1y);
				m_pBasicDraw->Vertex3fv(coord1_2);

				m_pBasicDraw->TexCoord2f(0, tc1y);
				m_pBasicDraw->Vertex3fv(coord1_1);

				m_pBasicDraw->TexCoord2f(1, last);
				m_pBasicDraw->Vertex3fv(coord2_2);

				// triangle 2
				m_pBasicDraw->TexCoord2f(0, tc1y);
				m_pBasicDraw->Vertex3fv(coord1_1);

				m_pBasicDraw->TexCoord2f(1, last);
				m_pBasicDraw->Vertex3fv(coord2_2);

				m_pBasicDraw->TexCoord2f(0, last);
				m_pBasicDraw->Vertex3fv(coord2_1);
			}

			coord1_1 = coord2_1;
			coord1_2 = coord2_2;
			tc1y = last;
		}

		screenlast = screenstart;
		last = SDL_fmod(last, 1.0);
		noiseindex += noisestep;
	}

	m_pBasicDraw->End();
}

//====================================
//
//====================================
cache_model_t* CBeamRenderer::GetSpriteModel( Int32 modelindex )
{
	if(modelindex <= 0)
		return nullptr;

	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
		return nullptr;

	if(pmodel->type != MOD_SPRITE)
	{
		Con_EPrintf("Beam was assigned a model that is not MOD_SPRITE.\n");
		return nullptr;
	}

	return pmodel;
}

//====================================
//
//====================================
void CBeamRenderer::BindSprite( cache_model_t* pmodel, beam_t* pbeam )
{
	// Check for recent load
	if(!pmodel->isloaded)
		R_LoadSprite(pmodel);

	// Get the sprite model
	const msprite_t *pspritemodel = pmodel->getSprite();
	// Bind the appropriate texture
	const mspriteframe_t* pframe = Sprite_GetFrame(pspritemodel, pbeam->frame, rns.time);

	// Bind to GL
	R_Bind2DTexture(GL_TEXTURE0_ARB, pframe->ptexture->palloc->gl_index);
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeam( beam_t* pbeam )
{
	cache_model_t* pmodel = GetSpriteModel(pbeam->modelindex1);
	if(!pmodel)
	{
		pbeam->die = rns.time;
		return;
	}

	// Update frequency
	pbeam->frequency += rns.frametime;

	// Update start and/or end positions
	if(pbeam->flags & (FL_BEAM_STARTENTITY|FL_BEAM_ENDENTITY))
	{
		if(pbeam->flags & FL_BEAM_STARTENTITY)
		{
			cl_entity_t* pstartentity = GetBeamAttachmentEntity(pbeam->startentity_index);
			if(!pstartentity)
				return;

			if(pstartentity->pmodel && !pbeam->startentidentifier)
			{
				// Set start entity identifier if it was not set yet
				pbeam->startentidentifier = pstartentity->identifier;
			}
			else if(!pstartentity->pmodel && !(pbeam->flags & FL_BEAM_KILLED))
			{
				// Clear the beam if the entity has no model
				pbeam->flags = FL_BEAM_KILLED;
				pbeam->die = rns.time;
			}

			// Verify that the identifier is the same
			if(pbeam->startentidentifier == pstartentity->identifier)
			{
				pbeam->source = GetBeamAttachmentPoint(pstartentity, pbeam->attachment1);
				pbeam->flags |= FL_BEAM_START_VISIBLE;
			}
			else if(!(pbeam->flags & FL_BEAM_KILLED))
			{
				// Entity's model has changed, clear it
				pbeam->flags = FL_BEAM_KILLED;
				pbeam->die = rns.time;
			}
		}

		if(pbeam->flags & FL_BEAM_ENDENTITY)
		{
			cl_entity_t* pendentity = GetBeamAttachmentEntity(pbeam->endentity_index);
			if(!pendentity)
				return;

			if(pendentity->pmodel && !pbeam->endentidentifier)
			{
				// Set start entity identifier if it was not set yet
				pbeam->endentidentifier = pendentity->identifier;
			}
			else if(!pendentity->pmodel && !(pbeam->flags & FL_BEAM_KILLED))
			{
				// Clear the beam if the entity has no model
				pbeam->flags = FL_BEAM_KILLED;
				pbeam->die = rns.time;
			}

			// Verify that the identifier is the same
			if(pbeam->endentidentifier == pendentity->identifier)
			{
				// Get beam end position
				pbeam->target = GetBeamAttachmentPoint(pendentity, pbeam->attachment2);
				pbeam->flags |= FL_BEAM_END_VISIBLE;
			}
			else if(!(pbeam->flags & FL_BEAM_KILLED))
			{
				// Entity's model has changed, clear it
				pbeam->flags = FL_BEAM_KILLED;
				pbeam->die = rns.time;
			}
		}

		if((pbeam->flags & FL_BEAM_STARTENTITY) && !(pbeam->flags & FL_BEAM_START_VISIBLE))
			return;

		// Compute segments from the new endpoints
		Vector difference;
		Math::VectorSubtract(pbeam->target, pbeam->source, difference);

		Float difflength = difference.Length();
		if(difflength > 0.0000001)
			pbeam->delta = difference;

		if(pbeam->amplitude < 0.5)
			pbeam->numsegments = difflength * 0.075f + 3.0f; // One per 16 pixels
		else
			pbeam->numsegments = difflength * 0.25f + 3.0f; // One per 4 pixels

		if(pbeam->numsegments < MIN_NB_BEAM_SEGMENTS)
			pbeam->numsegments = MIN_NB_BEAM_SEGMENTS;
	}

	// Check if the beam has any length
	if(!(pbeam->source-pbeam->target).Length())
		return;

	// Enable blending if set
	if(!(pbeam->flags & FL_BEAM_SOLID))
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
	}

	if(pbeam->type != BEAM_POINTS || !CullBeam(pbeam->source, pbeam->target, false))
	{
		BindSprite(pmodel, pbeam);

		pbeam->transparency = pbeam->die - rns.time + pbeam->frequency;
		if(pbeam->transparency)
			pbeam->transparency = 1.0f - (pbeam->frequency/pbeam->transparency);

		Float alpha;
		if(pbeam->flags & FL_BEAM_FADEIN)
			alpha = pbeam->transparency;
		else if(pbeam->flags & FL_BEAM_FADEOUT)
			alpha = (1.0 - pbeam->transparency);
		else
			alpha = 1.0;

		if(pbeam->type != BEAM_VAPORTRAIL)
		{
			// Set base color
			Vector color;
			Math::VectorScale(pbeam->color1, pbeam->brightness, color);

			m_pBasicDraw->Color4f(color.x, color.y, color.z, alpha * pbeam->brightness);
		}

		switch(pbeam->type)
		{
		case BEAM_POINTS:
			DrawBeamSegments(pbeam->source, pbeam->delta, pbeam->width, pbeam->amplitude, pbeam->frequency, pbeam->speed, pbeam->noisespeed, pbeam->numsegments, pbeam->flags, pbeam->index);
			break;
		case BEAM_TORUS:
			DrawBeamTorus(pbeam->source, pbeam->delta, pbeam->width, pbeam->amplitude, pbeam->frequency, pbeam->speed, pbeam->noisespeed, pbeam->numsegments, pbeam->index, pbeam->flags);
			break;
		case BEAM_DISK:
			DrawBeamDisk(pbeam->source, pbeam->delta, pbeam->width, pbeam->amplitude, pbeam->frequency, pbeam->speed, pbeam->noisespeed, pbeam->numsegments);
			break;
		case BEAM_CYLINDER:
			DrawBeamCylinder(pbeam->source, pbeam->delta, pbeam->width, pbeam->amplitude, pbeam->frequency, pbeam->speed, pbeam->noisespeed, pbeam->numsegments);
			break;
		case BEAM_FOLLOW:
			DrawBeamFollow(pbeam);
			break;
		case BEAM_RING:
			DrawBeamRing(pbeam->source, pbeam->delta, pbeam->width, pbeam->amplitude, pbeam->frequency, pbeam->speed, pbeam->noisespeed, pbeam->numsegments, pbeam->index, pbeam->flags);
			break;
		case BEAM_TESLA:
			DrawBeamTesla(pbeam->source, pbeam->delta, pbeam->width, pbeam->amplitude, pbeam->frequency, pbeam->speed, pbeam->noisespeed, pbeam->width * 0.5, pbeam->numsegments, pbeam->flags, pbeam->index);
			break;
		case BEAM_VAPORTRAIL:
			DrawBeamVaporTrail(pbeam, (alpha * pbeam->brightness));
			break;
		}
	}

	// Disable blending if set
	if(!(pbeam->flags & FL_BEAM_SOLID))
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamFollow( beam_t* pbeam )
{
	// Release any dead positions
	beam_position_t* pnext = pbeam->ppositions;
	while(pnext)
	{
		if(pnext->life < rns.time)
		{
			beam_position_t* pfree = pnext;
			pnext = pfree->pnext;

			FreeBeamPosition(pbeam, pfree);
			continue;
		}
		
		pnext = pnext->pnext;
	}

	// re-set pointer to base
	pnext = pbeam->ppositions;
	beam_position_t *pnew = nullptr;

	Float distance = 0;
	if(pbeam->flags & FL_BEAM_STARTENTITY)
	{
		if(pnext)
		{
			// Check if the distance is big enough to allocate
			Float length = (pnext->position - pbeam->source).Length();
			if(length >= BEAM_POSITION_SEGMENT_DISTANCE)
				pnew = AllocBeamPosition(pbeam);
		}
		else
		{
			// Allocate the starting position
			pnew = AllocBeamPosition(pbeam);
		}
	}

	if(pnew)
	{
		pnew->position = pbeam->source;
		pbeam->die = pnew->life = rns.time + pbeam->amplitude;
		pnext = pnew;
	}

	if(!pnext)
		return;

	Vector delta;
	Vector screenlast;
	Vector screenstart;

	if(!pnew && distance != 0)
	{
		delta = pbeam->source;
		R_WorldToScreenTransform(m_modelViewProjectionMatrix, pbeam->source, screenlast);
		R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->position, screenstart);
	}
	else if(pnext->pnext)
	{
		delta = pnext->position;
		R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->position, screenlast);
		R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->pnext->position, screenstart);
		pnext = pnext->pnext;
	}
	else
	{
		// Nothing to draw yet
		return;
	}

	Vector tmp;
	Math::VectorSubtract(screenstart, screenlast, tmp);
	tmp[2] = 0;
	tmp.Normalize();

	Vector normal;
	Math::VectorScale(rns.view.v_up, tmp[0], normal);
	Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

	Vector coord1_1, coord1_2;
	Math::VectorMA(delta, pbeam->width, normal, coord1_1);
	Math::VectorMA(delta, -pbeam->width, normal, coord1_2);

	Float last = 0.0f;
	Float step = 1.0f;

	Float div = 1.0/pbeam->amplitude;
	Float fraction = (pbeam->die - rns.time)*div;

	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	while(pnext)
	{
		// Remember the coordinates for rendering
		Float brightness1 = fraction;
		Float tc1y = last;

		R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->position, screenstart);
		Math::VectorSubtract(screenstart, screenlast, tmp);
		tmp[2] = 0.0f;
		tmp.Normalize();

		Math::VectorScale(rns.view.v_up, tmp[0], normal);
		Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

		Vector coord2_1, coord2_2;
		Math::VectorMA(pnext->position, pbeam->width, normal, coord2_1);
		Math::VectorMA(pnext->position, -pbeam->width, normal, coord2_2);

		last += step;

		if(pnext->pnext)
			fraction = (pnext->life - rns.time)*div;
		else
			fraction = 0;

		// Triangle 1
		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(0, tc1y);
		m_pBasicDraw->Vertex3fv(coord1_2);

		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(1, tc1y);
		m_pBasicDraw->Vertex3fv(coord1_1);

		m_pBasicDraw->Brightness1f(fraction);
		m_pBasicDraw->TexCoord2f(0, last);
		m_pBasicDraw->Vertex3fv(coord2_2);

		// triangle 2
		m_pBasicDraw->Brightness1f(brightness1);
		m_pBasicDraw->TexCoord2f(1, tc1y);
		m_pBasicDraw->Vertex3fv(coord1_1);

		m_pBasicDraw->Brightness1f(fraction);
		m_pBasicDraw->TexCoord2f(0, last);
		m_pBasicDraw->Vertex3fv(coord2_2);

		m_pBasicDraw->Brightness1f(fraction);
		m_pBasicDraw->TexCoord2f(1, last);
		m_pBasicDraw->Vertex3fv(coord2_1);

		screenlast = screenstart;
		last = SDL_fmod(last, 1.0);
		coord1_1 = coord2_1;
		coord1_2 = coord2_2;

		pnext = pnext->pnext;
	}

	m_pBasicDraw->End();

	// Update positions on beam segments
	beam_position_t* p = pbeam->ppositions;
	while(p)
	{
		Math::VectorMA(p->position, rns.frametime, p->velocity, p->position);
		p = p->pnext;
	}
}

//====================================
//
//====================================
void CBeamRenderer::DrawBeamVaporTrail( beam_t* pbeam, Float fadealpha )
{
	// Release any dead positions
	beam_position_t* pnext = pbeam->ppositions;
	while(pnext)
	{
		if(pnext->life && pnext->life < rns.time)
		{
			beam_position_t* pfree = pnext;
			pnext = pfree->pnext;

			FreeBeamPosition(pbeam, pfree);
			continue;
		}
		
		pnext = pnext->pnext;
	}

	// re-set pointer to base
	pnext = pbeam->ppositions;
	if(!pnext || !pnext->pnext)
		return;

	Vector delta;
	Vector screenlast;
	Vector screenstart;

	delta = pnext->position;
	R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->position, screenlast);
	R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->pnext->position, screenstart);
	pnext = pnext->pnext;

	Vector tmp;
	Math::VectorSubtract(screenstart, screenlast, tmp);
	tmp[2] = 0;
	tmp.Normalize();

	Vector normal;
	Math::VectorScale(rns.view.v_up, tmp[0], normal);
	Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

	Vector coord1_1, coord1_2;
	Math::VectorMA(delta, pbeam->width, normal, coord1_1);
	Math::VectorMA(delta, -pbeam->width, normal, coord1_2);
	Vector lastposition = delta;

	Float last = 0.0f;
	Float step = 1.0f;

	Float div = 1.0/pbeam->amplitude;
	Float fraction = (pbeam->die - rns.time)*div;

	// Create the segments
	Uint32 segmentIndex = 0;
	while(pnext)
	{
		// Remember the coordinates for rendering
		Float brightness1 = fraction;
		Float tc1y = last;

		R_WorldToScreenTransform(m_modelViewProjectionMatrix, pnext->position, screenstart);
		Math::VectorSubtract(screenstart, screenlast, tmp);
		tmp[2] = 0.0f;
		tmp.Normalize();

		Math::VectorScale(rns.view.v_up, tmp[0], normal);
		Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

		Vector coord2_1, coord2_2;
		Math::VectorMA(pnext->position, pbeam->width, normal, coord2_1);
		Math::VectorMA(pnext->position, -pbeam->width, normal, coord2_2);

		last += step;

		if(pnext->pnext)
			fraction = (pbeam->die - rns.time)*div;
		else
			fraction = 0;

		beamsegment_t seg;
		seg.brightness = brightness1;
		seg.tcy = tc1y;
		seg.coord1 = coord1_1;
		seg.coord2 = coord1_2;
		seg.center = lastposition;

		// Add to array
		pbeam->drawsegments[segmentIndex] = seg;
		segmentIndex++;

		seg.brightness = fraction;
		seg.tcy = last;
		seg.coord1 = coord2_1;
		seg.coord2 = coord2_2;
		seg.center = pnext->position;

		// Add to array
		pbeam->drawsegments[segmentIndex] = seg;
		segmentIndex++;

		lastposition = pnext->position;
		screenlast = screenstart;
		last = SDL_fmod(last, 1.0);
		coord1_1 = coord2_1;
		coord1_2 = coord2_2;

		pnext = pnext->pnext;
	}

	Float blendfactor;
	if(!pbeam->colorfadetime)
	{
		// No blending, just draw lightmapped beam
		blendfactor = 1.0;
	}
	else
	{
		// Draw the fully lit beam
		if(rns.time < (pbeam->spawntime + pbeam->colorfadedelay))
			blendfactor = 0.0;
		else if(rns.time < (pbeam->spawntime + pbeam->colorfadedelay + pbeam->colorfadetime))
			blendfactor = (rns.time - (pbeam->spawntime + pbeam->colorfadedelay)) / pbeam->colorfadetime;
		else
			blendfactor = 1.0;

		if(blendfactor < 1.0)
			DrawVaporTrailSegments(pbeam->drawsegments, pbeam->color1, (1.0 - blendfactor), false, fadealpha);
	}

	// Draw the lightmapped trail
	if(blendfactor > 0.0)
	{
		if(pbeam->modelindex2 != pbeam->modelindex1)
		{
			cache_model_t* pmodel = GetSpriteModel(pbeam->modelindex2);
			if(!pmodel)
				return;

			BindSprite(pmodel, pbeam);
		}

		DrawVaporTrailSegments(pbeam->drawsegments, pbeam->color2, blendfactor, true, fadealpha);
	}

	// Update positions on beam segments
	beam_position_t* p = pbeam->ppositions;
	while(p)
	{
		if(p->acceleration > 0 && blendfactor > 0.15)
			Math::VectorMA(p->velocity, p->acceleration * blendfactor * rns.frametime, p->winddirection, p->velocity);

		Math::VectorMA(p->position, rns.frametime, p->velocity, p->position);
		p = p->pnext;
	}
}

//====================================
//
//====================================
void CBeamRenderer::DrawVaporTrailSegments( const CArray<beamsegment_t>& segments, Vector color, Float alpha, bool takelighting, Float fadealpha )
{
	Vector color1, color2;
	if(!takelighting)
	{
		Math::VectorScale(color, alpha, color1);
		Math::VectorScale(color, alpha, color2);
	}
		
	R_ValidateShader(m_pBasicDraw);

	m_pBasicDraw->Begin(GL_TRIANGLES);

	for(Uint32 i = 0; i < segments.size(); i += 2)
	{
		beamsegment_t& seg1 = segments[i];
		beamsegment_t& seg2 = segments[i+1];

		if(takelighting)
		{
			Vector lcolor;
			Vector end = seg1.center - Vector(0, 0, 8192);
			if(Mod_RecursiveLightPoint(ens.pworld, ens.pworld->pnodes, seg1.center, end, lcolor))
				color1 = lcolor * color;
			else
				color1 = color;

			end = seg2.center - Vector(0, 0, 8192);
			if(Mod_RecursiveLightPoint(ens.pworld, ens.pworld->pnodes, seg2.center, end, lcolor))
				color2 = lcolor * color;
			else
				color2 = color;

			Math::VectorScale(color1, alpha, color1);
			Math::VectorScale(color2, alpha, color2);
		}

		// Triangle 1
		m_pBasicDraw->Color4f(color1.x, color1.y, color1.z, fadealpha);
		m_pBasicDraw->Brightness1f(seg1.brightness);
		m_pBasicDraw->TexCoord2f(0, seg1.tcy);
		m_pBasicDraw->Vertex3fv(seg1.coord2);

		m_pBasicDraw->Color4f(color1.x, color1.y, color1.z, fadealpha);
		m_pBasicDraw->Brightness1f(seg1.brightness);
		m_pBasicDraw->TexCoord2f(1, seg1.tcy);
		m_pBasicDraw->Vertex3fv(seg1.coord1);

		m_pBasicDraw->Color4f(color2.x, color2.y, color2.z, fadealpha);
		m_pBasicDraw->Brightness1f(seg2.brightness);
		m_pBasicDraw->TexCoord2f(0, seg2.tcy);
		m_pBasicDraw->Vertex3fv(seg2.coord2);

		// triangle 2
		m_pBasicDraw->Color4f(color1.x, color1.y, color1.z, fadealpha);
		m_pBasicDraw->Brightness1f(seg1.brightness);
		m_pBasicDraw->TexCoord2f(1, seg1.tcy);
		m_pBasicDraw->Vertex3fv(seg1.coord1);

		m_pBasicDraw->Color4f(color2.x, color2.y, color2.z, fadealpha);
		m_pBasicDraw->Brightness1f(seg2.brightness);
		m_pBasicDraw->TexCoord2f(0, seg2.tcy);
		m_pBasicDraw->Vertex3fv(seg2.coord2);

		m_pBasicDraw->Color4f(color2.x, color2.y, color2.z, fadealpha);
		m_pBasicDraw->Brightness1f(seg2.brightness);
		m_pBasicDraw->TexCoord2f(1, seg2.tcy);
		m_pBasicDraw->Vertex3fv(seg2.coord1);
	}

	m_pBasicDraw->End();
}

//====================================
//
//====================================
cl_entity_t* CBeamRenderer::GetBeamAttachmentEntity( Int32 index )
{
	return CL_GetEntityByIndex(index);
}

//====================================
//
//====================================
Vector CBeamRenderer::GetBeamAttachmentPoint( cl_entity_t* pentity, Int32 attachment )
{
	if(attachment == NO_ATTACHMENT_INDEX || attachment < 0)
		return pentity->curstate.origin;
	else
		return pentity->getAttachment(attachment);
}

//====================================
//
//====================================
void CBeamRenderer::KillEntityBeams( entindex_t entindex )
{
	if(!m_pActiveBeamHeader)
		return;

	beam_t* pnext = m_pActiveBeamHeader;
	while(pnext)
	{
		// Only do anything if startentity_index matches
		if(pnext->startentity_index != entindex)
		{
			pnext = pnext->pnext;
			continue;
		}

		// Keep beamfollow till it fades out
		if(pnext->type == BEAM_FOLLOW)
		{
			// Remove these flags so we no longer update the beam using the entity
			pnext->flags &= ~(FL_BEAM_STARTENTITY|FL_BEAM_ENDENTITY);

			// Move onto next
			pnext = pnext->pnext;
			continue;
		}
		else
		{
			// Beam can be killed
			beam_t* pfree = pnext;
			pnext = pfree->pnext;
			FreeBeam(pfree);
		}
	}
}

//====================================
//
//====================================
void CBeamRenderer::AddBeamEntity( cl_entity_t* pentity )
{
	// See if a beam already exists tied to this entity
	beam_t* pbeam = FindBeamByEntity(pentity);
	if(!pbeam)
	{
		pbeam = AllocBeam();
		if(!pbeam)
			return;

		// Set these only once, as they won't change
		pbeam->pserverentity = pentity;
		pbeam->flags |= FL_BEAM_FOREVER;
	}

	// Set the beam entity up. This needs to be called each frame, in case the server entity changes
	// in some way
	Float speed = pentity->curstate.fuser1 * 0.1;
	Float amplitude = pentity->curstate.fuser2 * 0.01;
	Float width = pentity->curstate.scale;
	Float noisespeed = pentity->curstate.fuser3 * 0.1;
	Float brightness = R_RenderFxBlend(pentity)/255.0f;
	Int32 beamflags = pentity->curstate.iuser2;

	// Get start/end position
	BeamSetup(pbeam, pentity->curstate.origin, pentity->curstate.vuser1, pentity->pmodel->cacheindex, 0.0, width, amplitude, brightness, speed, noisespeed, beamflags);
	
	// Get color
	Math::VectorScale(pentity->curstate.rendercolor, 1.0f/255.0f, pbeam->color1);

	// Set values specific to beam type
	switch(pentity->curstate.iuser1)
	{
	case BEAM_MSG_BEAMENTPOINT:
		{
			if(pbeam->flags & FL_BEAM_TESLA)
				pbeam->type = BEAM_TESLA;
			else
				pbeam->type = BEAM_POINTS;

			pbeam->flags |= FL_BEAM_ENDENTITY;
			pbeam->startentity_index = NO_ENTITY_INDEX;
			pbeam->endentity_index = pentity->curstate.iuser4;
			pbeam->attachment1 = pentity->curstate.iuser5;
		}
		break;
	case BEAM_MSG_BEAMENTS:
		{
			if(pbeam->flags & FL_BEAM_TESLA)
				pbeam->type = BEAM_TESLA;
			else
				pbeam->type = BEAM_POINTS;

			pbeam->flags |= (FL_BEAM_STARTENTITY|FL_BEAM_ENDENTITY);
			pbeam->startentity_index = pentity->curstate.iuser3;
			pbeam->endentity_index = pentity->curstate.iuser4;
			pbeam->attachment1 = pentity->curstate.iuser5;
			pbeam->attachment2 = pentity->curstate.iuser6;
		}
		break;

	case BEAM_MSG_BEAMPOINTS:
		{
			if(beamflags & FL_BEAM_TESLA)
				pbeam->type = BEAM_TESLA;
			else
				pbeam->type = BEAM_POINTS;
		}
		break;
	case BEAM_MSG_BEAMSPRITE:
	case BEAM_MSG_BEAMTORUS:
	case BEAM_MSG_BEAMDISK:
	case BEAM_MSG_BEAMCYLINDER:
	case BEAM_MSG_BEAMFOLLOW:
	case BEAM_MSG_BEAMRING:
	case BEAM_MSG_KILLENTITYBEAMS:
		break;
	}
}

//====================================
//
//====================================
bool CBeamRenderer::BeamSetup( beam_t* pbeam, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags )
{
	cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	if(!pmodel)
	{
		Con_EPrintf("%s - Failed to get model with index %d for beam.\n", __FUNCTION__, modelindex);
		return false;
	}

	if(pmodel->type != MOD_SPRITE)
	{
		Con_EPrintf("%s - Model '%s' specified for beam is not a sprite.\n", __FUNCTION__, pmodel->name.c_str());
		return false;
	}

	const msprite_t* psprite = pmodel->getSprite();

	// Do not allow these flags to be set by user
	pbeam->flags |= flags & ~(FL_BEAM_STARTENTITY|FL_BEAM_ENDENTITY|FL_BEAM_FOREVER);

	// Default type
	pbeam->type = BEAM_POINTS;
	pbeam->modelindex1 = modelindex;
	pbeam->frame = 0;
	pbeam->framerate = 0;
	pbeam->framecount = psprite->frames.size();
	pbeam->spawntime = cls.cl_time;

	pbeam->source = src;
	pbeam->target = end;
	Math::VectorSubtract(pbeam->target, pbeam->source, pbeam->delta);

	pbeam->frequency = speed * cls.cl_time;
	pbeam->die = cls.cl_time + life;
	pbeam->width = width;
	pbeam->amplitude = amplitude;
	pbeam->speed = speed;
	pbeam->brightness = brightness;
	pbeam->color1 = Vector(1, 1, 1);
	pbeam->noisespeed = noisespeed;
	if(!pbeam->noisespeed)
		pbeam->noisespeed = 1.0;

	Float length = pbeam->delta.Length();
	if(amplitude >= 0.5)
		pbeam->numsegments = (Uint32)(length * 0.25 + 3.0);
	else
		pbeam->numsegments = (Uint32)(length * 0.075 + 3.0);

	if(pbeam->numsegments < MIN_NB_BEAM_SEGMENTS)
		pbeam->numsegments = MIN_NB_BEAM_SEGMENTS;

	return true;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamLightning( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags )
{
	beam_t* pbeam = AllocBeam();
	if(!pbeam)
		return nullptr;

	if(!BeamSetup(pbeam, src, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags))
	{
		FreeBeam(pbeam);
		return nullptr;
	}

	return pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamCirclePoints( beam_types_t type, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	beam_t* pbeam = BeamLightning(src, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags);
	if(!pbeam)
		return nullptr;

	pbeam->type = type;
	if(!life)
		pbeam->flags |= FL_BEAM_FOREVER;

	SetBeamAttributes(pbeam, r, g, b, framerate, startframe);
	return pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamEntityPoint( entindex_t startentity, Int32 attachment, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	cl_entity_t* pentity = GetBeamAttachmentEntity(startentity);
	if(pentity && (!life || pentity->pmodel))
	{
		beam_t* pbeam = BeamLightning(ZERO_VECTOR, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags);
		if(!pbeam)
			return nullptr;

		pbeam->attachment1 = attachment;
		pbeam->flags |= FL_BEAM_STARTENTITY;

		if(!life)
			pbeam->flags |= FL_BEAM_FOREVER;

		pbeam->startentity_index = startentity;
		pbeam->endentity_index = NO_ENTITY_INDEX;

		SetBeamAttributes(pbeam, r, g, b, framerate, startframe);
		return pbeam;
	}

	return nullptr;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamEntities( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	cl_entity_t* pstartentity = GetBeamAttachmentEntity(startentity);
	if(!pstartentity)
		return nullptr;

	cl_entity_t* pendentity = GetBeamAttachmentEntity(endentity);
	if(!pendentity)
		return nullptr;

	if(life != 0 && (!pstartentity->pmodel || !pendentity->pmodel))
		return nullptr;

	beam_t* pbeam = BeamLightning(ZERO_VECTOR, ZERO_VECTOR, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags);
	if(!pbeam)
		return nullptr;

	pbeam->type = BEAM_POINTS;
	pbeam->flags |= (FL_BEAM_STARTENTITY|FL_BEAM_ENDENTITY);

	if(!life)
		pbeam->flags |= FL_BEAM_FOREVER;

	pbeam->startentity_index = startentity;
	pbeam->endentity_index = endentity;
	pbeam->attachment1 = attachment1;
	pbeam->attachment2 = attachment2;

	SetBeamAttributes(pbeam, r, g, b, framerate, startframe);
	return pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamFollow( entindex_t startentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float brightness, Float r, Float g, Float b )
{
	cl_entity_t* pstartentity = GetBeamAttachmentEntity(startentity);
	if(!pstartentity)
		return nullptr;

	beam_t* pbeam = BeamLightning(ZERO_VECTOR, ZERO_VECTOR, modelindex, life, width, life, brightness, 1.0, 1.0, FL_BEAM_NONE);
	if(!pbeam)
		return nullptr;

	pbeam->type = BEAM_FOLLOW;
	pbeam->flags |= FL_BEAM_STARTENTITY;
	pbeam->startentity_index = startentity;
	pbeam->attachment1 = attachment;

	SetBeamAttributes(pbeam, r, g, b, 1.0, 0);
	return pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamVaporTrail( const Vector& src, const Vector& end, Int32 modelindex1, Int32 modelindex2, Float colorfadedelay, Float colorfadetime, Float life, Float width, Float brightness, Float r1, Float g1, Float b1, Float r2, Float g2, Float b2, Int32 flags )
{
	beam_t* pbeam = BeamLightning(src, end, modelindex1, life, width, life, brightness, 1.0, 1.0, flags);
	if(!pbeam)
		return nullptr;

	pbeam->type = BEAM_VAPORTRAIL;
	pbeam->color2 = Vector(r2, g2, b2);
	pbeam->modelindex2 = modelindex2;
	pbeam->colorfadedelay = colorfadedelay;
	pbeam->colorfadetime = colorfadetime;

	SetBeamAttributes(pbeam, r1, g1, b1, 1.0, 0);

	Float division = 16; // Every 16 units
	Float beamlength = (src - end).Length();
	Uint32 divisions = beamlength / (Float)division;

	pbeam->drawsegments.resize(divisions*2);

	Float fraction = 1.0f/divisions;
	for(Float f = 0; f <= 1.0; f += fraction)
	{
		beam_position_t* p = AllocBeamPosition(pbeam);
		p->position = src * (1.0 - f) + end * f;

		p->winddirection = Vector(Common::RandomFloat(-4, 5), Common::RandomFloat(-5, 5), Common::RandomFloat(-5, 5));
		p->winddirection.Normalize();

		p->acceleration = Common::RandomFloat(0, 0.8) * SDL_sin(fraction * divisions * 0.5);
		if(p->acceleration < 0.2)
			p->acceleration = 0.2;
		else if(p->acceleration > 0.8)
			p->acceleration = 0.8;

		p->life = 0;
	}

	return pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamPoints( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	if(life != 0 && CullBeam(src, end, true))
		return nullptr;

	beam_t* pbeam = BeamLightning(src, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags);
	if(!pbeam)
		return nullptr;

	if(!life)
		pbeam->flags |= FL_BEAM_FOREVER;

	SetBeamAttributes(pbeam, r, g, b, framerate, startframe);
	return pbeam;
}

//====================================
//
//====================================
beam_t* CBeamRenderer::BeamRing( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	cl_entity_t* pstartentity = GetBeamAttachmentEntity(startentity);
	if(!pstartentity)
		return nullptr;

	cl_entity_t* pendentity = GetBeamAttachmentEntity(endentity);
	if(!pendentity)
		return nullptr;

	if(life != 0 && (!pstartentity->pmodel || !pendentity->pmodel))
		return nullptr;

	beam_t* pbeam = BeamLightning(ZERO_VECTOR, ZERO_VECTOR, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags);
	if(!pbeam)
		return nullptr;

	pbeam->type = BEAM_RING;
	pbeam->flags |= (FL_BEAM_STARTENTITY|FL_BEAM_ENDENTITY);
	
	if(!life)
		pbeam->flags |= FL_BEAM_FOREVER;

	pbeam->startentity_index = startentity;
	pbeam->endentity_index = endentity;
	pbeam->attachment1 = attachment1;
	pbeam->attachment2 = attachment2;

	SetBeamAttributes(pbeam, r, g, b, framerate, startframe);
	return pbeam;
}

//====================================
//
//====================================
void CBeamRenderer::SetBeamAttributes( beam_t* pbeam, Float r, Float g, Float b, Float framerate, Uint32 startframe )
{
	pbeam->framerate = framerate;
	pbeam->frame = (Float)startframe;
	pbeam->color1.x = r;
	pbeam->color1.y = g;
	pbeam->color1.z = b;
}

//====================================
//
//====================================
void CBeamRenderer::ApplySegmentNoise( Vector& start, Uint32 beamindex, Uint32 i, Float scale, Uint32 numsegments, const Vector& up, const Vector& right, Int32 flags )
{
	if(scale <= 0)
		return;

	// Calculate noise based on distance from center
	Float noiseamount;
	Uint32 halfsegcnt = (Uint32)(numsegments*0.5);
	if(i < halfsegcnt)
		noiseamount = (Float)i / (Float)halfsegcnt;
	else
		noiseamount = 1.0 - ((Float)(i-halfsegcnt) / (Float)halfsegcnt);

	// Scale up noise amount a bit
	noiseamount = (noiseamount * sqrt(noiseamount))*10;

	// Speed factor is based on beam count
	Float speed;
	if(flags & FL_BEAM_VARIABLE_NOISE)
		speed = ((beamindex % 4) + 1) * 0.05;
	else
		speed = 0;

	// Beam direction
	Float dir;
	if(flags & FL_BEAM_VARIABLE_DIR)
		dir = (beamindex % 2) == 0 ? -1 : 1;
	else
		dir = 1;

	// Beam noise step
	Float noisestep;
	if(flags & FL_BEAM_VARIABLE_NOISE)
		noisestep = (Float)beamindex * 2;
	else
		noisestep = 0;

	// Calculate noise on up vector
	Math::VectorMA(start, SDL_sin((Float)i*0.15 + (rns.time*dir) * 20 * (1.0 + speed) + noisestep)*noiseamount*scale, up, start);

	// Calculate noise on right vector
	Math::VectorMA(start, SDL_cos((Float)i*0.25 + (rns.time*dir) * 30 * (1.0 + speed) + noisestep)*noiseamount*scale, right, start);
}