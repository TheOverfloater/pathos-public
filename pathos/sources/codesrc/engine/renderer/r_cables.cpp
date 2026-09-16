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
#include "r_cables.h"
#include "com_math.h"
#include "system.h"
#include "brushmodel.h"
#include "enginestate.h"
#include "file.h"
#include "r_common.h"
#include "cvar.h"
#include "r_basicdraw.h"
#include "modelcache.h"
#include "sprite.h"
#include "texturemanager.h"
#include "r_lightstyles.h"
#include "r_dlights.h"

//
// Thanks to SysOp for the original cable code I relied upon
//

// Object definition
CCableRenderer gCableRenderer;

//====================================
//
//====================================
CCableRenderer::CCableRenderer( void )
{
}

//====================================
//
//====================================
CCableRenderer::~CCableRenderer( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CCableRenderer::Init( void )
{
	return true;
}

//====================================
//
//====================================
void CCableRenderer::Shutdown( void )
{
	ClearGame();
}

//====================================
//
//====================================
bool CCableRenderer::InitGL( void )
{
	return true;
}

//====================================
//
//====================================
void CCableRenderer::ClearGL( void )
{
}

//====================================
//
//====================================
bool CCableRenderer::InitGame( void )
{
	return true;
}

//====================================
//
//====================================
void CCableRenderer::ClearGame( void )
{
	if(!m_cablesArray.empty())
		m_cablesArray.clear();
}

//====================================
//
//====================================
bool CCableRenderer::DrawCables( void )
{
	if(m_cablesArray.empty())
		return true;

	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw->Enable() || !pDraw->EnableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->SetProjection(rns.view.projection.GetMatrix());
	pDraw->SetModelview(rns.view.modelview.GetMatrix());

	if(rns.fog.settings.active)
	{
		if(!pDraw->EnableFog())
		{
			Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
			pDraw->Disable();
			return false;
		}

		// We use black fog on tracers
		pDraw->SetFogParams(rns.fog.settings.color, rns.fog.settings.start, rns.fog.settings.end);
	}

	glDisable(GL_CULL_FACE);
	R_ValidateShader(pDraw);

	// Set modelview-projection matrix
	CMatrix modelViewProjectionMatrix;
	modelViewProjectionMatrix.SetMatrix(rns.view.modelview.GetMatrix());
	modelViewProjectionMatrix.MultMatrix(rns.view.projection.Transpose());

	for(Uint32 i = 0; i < m_cablesArray.size(); i++)
	{
		const cable_object_t& cable = m_cablesArray[i];

		// Bind the sprite used
		const msprite_t* psprite = cable.pmodel->getSprite();
		const mspriteframe_t *frame = Sprite_GetFrame(psprite, 0, rns.time);
		R_Bind2DTexture(GL_TEXTURE0, frame->ptexture->palloc->gl_index);

		const Vector& cablestart = cable.start;
		const Vector& cableend = cable.end;

		// Draw the beam itself
		Vector delta;
		Math::VectorSubtract(cableend, cablestart, delta);

		Float length = delta.Length() * 0.01;
		if(length < 0.5)
			length = 0.5;

		Float div = 1.0f / (cable.numsegments - 1);
		Float step = length*div;
		Float tcy1 = 0;

		Vector vbottom;
		Math::VectorMA(cablestart, 0.5, delta, vbottom);
		vbottom[2] -= cable.falldepth;

		if(cable.windx)
			vbottom.x += SDL_sin(rns.time * 1.25) * cable.windx;

		if(cable.windy)
			vbottom.y += SDL_sin(rns.time * 0.75) * cable.windy;

		// Calculate start
		Vector start, screenlast, screenstart;
		R_WorldToScreenTransform(modelViewProjectionMatrix, cablestart, screenlast);
		Vector prevpoint = cablestart;

		// Calculate 1st point
		Vector nextpoint = CalculatePoint(1, cable.numsegments, cablestart, screenlast, vbottom);
		R_WorldToScreenTransform(modelViewProjectionMatrix, nextpoint, screenstart);

		Vector tmp;
		Math::VectorSubtract(screenstart, screenlast, tmp);
		tmp[2] = 0.0; // discard z as we're in screen space

		Vector normal;
		Math::VectorNormalize(tmp);
		Math::VectorScale(rns.view.v_up, tmp[0], normal);
		Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

		Vector coord1_1, coord1_2;
		Math::VectorMA(cablestart, cable.width, normal, coord1_1);
		Math::VectorMA(cablestart, -cable.width, normal, coord1_2);

		// Calculate normal for lighting
		Vector segmentdir;
		Math::VectorSubtract(nextpoint, prevpoint, segmentdir);
		segmentdir.Normalize();

		Vector prevlightnormal;
		Math::CrossProduct(segmentdir, normal, prevlightnormal);

		// Draw as quads
		pDraw->Begin(CBasicDraw::DRAW_QUADS);

		Uint32 nbVertexes = 0;
		for(Uint32 j = 1; j <= cable.numsegments; j++)
		{
			// Calculate current point
			Vector vpoint = CalculatePoint(j, cable.numsegments, cablestart, cableend, vbottom);

			R_WorldToScreenTransform(modelViewProjectionMatrix, vpoint, screenstart);
			Math::VectorSubtract(screenstart, screenlast, tmp);

			// We don't need Z in screen-space
			tmp[2] = 0;
			Math::VectorNormalize(tmp);

			Math::VectorScale(rns.view.v_up, tmp[0], normal);
			Math::VectorMA(normal, -tmp[1], rns.view.v_right, normal);

			// Calculate final coordinates
			Vector coord2_1, coord2_2;
			Math::VectorMA(vpoint, cable.width, normal, coord2_1);
			Math::VectorMA(vpoint, -cable.width, normal, coord2_2);

			Math::VectorSubtract(vpoint, prevpoint, segmentdir);
			segmentdir.Normalize();

			Vector lightnormal;
			Math::CrossProduct(segmentdir, normal, lightnormal);
			lightnormal.Normalize();

			cable_lsample_t& sample1 = cable.pointsamples[j-1];
			cable_lsample_t& sample2 = cable.pointsamples[j];

			// Calculate texcoord
			Float tcy2 = tcy1 + step;

			// Draw as a quad
			Vector lightcolor = CalculateLighting(cable, sample1, prevpoint, prevlightnormal);
			pDraw->Color4f(lightcolor.x, lightcolor.y, lightcolor.z, 1.0);
			pDraw->TexCoord2f(1, tcy1);
			pDraw->Vertex3fv(coord1_1);

			pDraw->TexCoord2f(0, tcy1);
			pDraw->Vertex3fv(coord1_2);

			lightcolor = CalculateLighting(cable, sample2, vpoint, lightnormal);
			pDraw->Color4f(lightcolor.x, lightcolor.y, lightcolor.z, 1.0);
			pDraw->TexCoord2f(0, tcy2);
			pDraw->Vertex3fv(coord2_2);

			pDraw->TexCoord2f(1, tcy2);
			pDraw->Vertex3fv(coord2_1);

			// See if we've filled the cache
			nbVertexes += 4;
			if((nbVertexes+4) >= CBasicDraw::BASICDRAW_VERTEX_CACHE_SIZE)
			{
				pDraw->End();
				pDraw->Begin(CBasicDraw::DRAW_QUADS);
				nbVertexes = 0;
			}

			screenlast = screenstart;
			tcy1 = SDL_fmod(tcy2, 1.0);

			coord1_1 = coord2_1;
			coord1_2 = coord2_2;

			prevlightnormal = lightnormal;
			prevpoint = vpoint;
		}

		pDraw->End();
	}

	bool result = true;
	if(rns.fog.settings.active)
		result = pDraw->DisableFog();

	pDraw->Disable();

	glEnable(GL_CULL_FACE);

	// Clear any binds
	R_ClearBinds();

	return true;
}

//====================================
//
//====================================
void CCableRenderer::RefreshLighting( void )
{
	if(m_cablesArray.empty())
		return;

	for(Uint32 i = 0; i < m_cablesArray.size(); i++)
	{
		const cable_object_t& cable = m_cablesArray[i];

		Vector vbottom;
		Math::VectorMA(cable.start, 0.5, (cable.end-cable.start), vbottom);
		vbottom[2] -= cable.falldepth;

		for(Uint32 j = 0; j <= cable.numsegments; j++)
		{
			Vector vpoint = CalculatePoint(j, cable.numsegments, cable.start, cable.end, vbottom);

			cable_lsample_t& sample = cable.pointsamples[j];
			R_GetLightingForPosition(vpoint, ZERO_VECTOR, sample.diffuselight, sample.ambientlight, sample.lightdirs, sample.styles);
		}
	}
}

//====================================
//
//====================================
Vector CCableRenderer::CalculateLighting( const cable_object_t& cable, const cable_lsample_t& sample, const Vector& position, const Vector& normal )
{
	// Calculate base lighting
	const CArray<Float>* pstylesarray = gLightStyles.GetLightStyleValuesArray();

	Vector outcolor;
	for(Uint32 i = 0; i < MAX_SURFACE_STYLES; i++)
	{
		if(sample.styles[i] == NULL_LIGHTSTYLE_INDEX)
			break;

		Float stylestrength;
		if(i == 0)
			stylestrength = 1.0;
		else
			stylestrength = (*pstylesarray)[sample.styles[i]];

		Float dp = -Math::DotProduct(sample.lightdirs[i], normal);
		dp = clamp(dp, 0, 1);

		Math::VectorAdd(outcolor, sample.ambientlight[i], outcolor);
		Math::VectorMA(outcolor, dp*stylestrength, sample.diffuselight[i], outcolor);
	}

	// Add in dynamic lights
	CLinkedList<cl_dlight_t*>& dlightlist = gDynamicLights.GetLightList();

	dlightlist.begin();
	while(!dlightlist.end())
	{
		cl_dlight_t* dl = dlightlist.get();
		if(Math::CheckMinsMaxs(cable.vmins, cable.vmaxs, dl->mins, dl->maxs))
		{
			dlightlist.next();
			continue;
		}

		if(dl->pfrustum)
		{
			if(dl->pfrustum->CullBBox(cable.vmins, cable.vmaxs))
			{
				dlightlist.next();
				continue;
			}
		}

		Float rad = dl->radius * dl->radius;

		Vector dir;
		Math::VectorSubtract(dl->origin, position, dir);

		Float dist = Math::DotProduct(dir, dir);
		Float attenuation = ((dist/rad)-1) * -1;
		
		attenuation = clamp(attenuation, 0, 1);
		dir.Normalize();

		Float dp = Math::DotProduct(dir, normal);
		dp = clamp(dp, 0, 1);

		if(dl->cone_size)
		{
			Vector spotforward;
			Math::AngleVectors(dl->angles, &spotforward);

			Float spotcos = cos((dl->cone_size*2)*0.3*(M_PI*2/360));
			Float spotatten = -Math::DotProduct(spotforward, dir);
	
			spotatten = max(spotatten, spotcos);
			Float spotfactor = (spotatten - spotcos)/(1.0 - spotcos);
			attenuation *= spotfactor;
		}

		Math::VectorMA(outcolor, dp*attenuation, dl->color, outcolor);
		dlightlist.next();
	}

	return outcolor;
}

//====================================
//
//====================================
Vector CCableRenderer::CalculatePoint( Uint32 segment, Uint32 numsegments, const Vector& start, const Vector& end, const Vector& midpoint )
{
	// Calculate current point
	Float f = static_cast<Float>(segment)/static_cast<Float>(numsegments);

	Vector vpoint;
	for(Uint32 k = 0; k < 3; k++)
		vpoint[k] = start[k]*((1-f)*(1-f))+midpoint[k]*((1-f)*f*2)+end[k]*(f*f);

	return vpoint;
}

//====================================
//
//====================================
void CCableRenderer::AddCable( Int32 spritemodelindex, const Vector& start, const Vector& end, Uint32 depth, Uint32 width, Uint32 numsegments, Float windx, Float windy )
{
	const cache_model_t* pmodel = Cache_GetModel(spritemodelindex);
	if(!pmodel)
	{
		Con_EPrintf("%s - Couldn't find sprite model with index %d.\n", __FUNCTION__, spritemodelindex);
		return;
	}

	if(pmodel->type != MOD_SPRITE)
	{
		Con_EPrintf("%s - Model '%s' specified is not a sprite model.\n", __FUNCTION__, pmodel->name.c_str());
		return;
	}

	cable_object_t newcable;
	newcable.start = start;
	newcable.end = end;
	newcable.falldepth = depth;
	newcable.width = width;
	newcable.numsegments = numsegments;
	newcable.windx = windx;
	newcable.windy = windy;
	newcable.pmodel = pmodel;

	Vector vbottom;
	Math::VectorMA(start, 0.5, (end-start), vbottom);
	vbottom[2] -= depth;

	// Resize samples
	newcable.pointsamples.resize(numsegments+1);

	Vector vmins = NULL_MINS;
	Vector vmaxs = NULL_MAXS;
	for(Uint32 i = 0; i <= numsegments; i++)
	{
		Vector vpoint = CalculatePoint(i, numsegments, start, end, vbottom);

		cable_lsample_t& sample = newcable.pointsamples[i];
		R_GetLightingForPosition(vpoint, ZERO_VECTOR, sample.diffuselight, sample.ambientlight, sample.lightdirs, sample.styles);

		for(Uint32 j = 0; j < 3; j++)
		{
			if(vpoint[j] > vmaxs[j])
				vmaxs[j] = vpoint[j];

			if(vpoint[j] < vmins[j])
				vmins[j] = vpoint[j];
		}
	}

	// prevent intersection
	Math::VectorSubtract(vmins, Vector(1, 1, 1), newcable.vmins);
	Math::VectorAdd(vmaxs, Vector(1, 1, 1), newcable.vmaxs);

	Mod_FindTouchedLeafs(ens.pworld, newcable.leafnums, newcable.numleafs, newcable.vmins, newcable.vmaxs, ens.pworld->pnodes);

	// Add to the list
	m_cablesArray.push_back(newcable);
}
