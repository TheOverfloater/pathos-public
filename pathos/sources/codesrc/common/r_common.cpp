/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "r_common.h"
#include "brushmodel_shared.h"
#include "cl_entity.h"
#include "matrix.h"
#include "cache_model.h"
#include "studio.h"
#include "vbm_shared.h"
#include "r_glsl.h"
#include "textures_shared.h"

const Char* SCREEN_RATIO_STRINGS[NB_SCREENRATIO_STRINGS] = {
	"4_3",		// SCR_RATIO_NORMAL
	"16_9",		// SCR_RATIO_WIDESCREEN
	"21_9"		// SCR_RATIO_ULTRAWIDE
};

//=============================================
// @brief
//
//=============================================
void R_AllocBlock ( Uint32 w, Uint32 h, Uint32 &x, Uint32 &y, Uint32& width, Uint32 &height, Uint32*& pallocations )
{ 
	Uint32 ibest1 = height;
	for(Uint32 i = 0; i < width-w; i++)
	{
		Uint32 ibest2 = 0;

		Uint32 j = 0;
		for(; j < w; j++)
		{
			if(pallocations[i+j] >= ibest1)
				break;

			if(pallocations[i+j] > ibest2)
				ibest2 = pallocations[i+j];
		}

		if(j == w)
		{
			x= i;
			y = ibest1 = ibest2;
		}
	}

	if((ibest1+h) > height)
	{
		if(height == width)
		{
			// Increase height only if sizes match
			height *= 2;
		}
		else
		{
			// If sizes don't match, then the next to be increased is width
			Uint32 newwidth = width*2;
			Uint32* pnewalloc = new Uint32[newwidth];

			for(Uint32 i = 0; i < width; i++)
				pnewalloc[i] = pallocations[i];

			for(Uint32 i = width; i < newwidth; i++)
				pnewalloc[i] = 0;

			// Delete original and set ptr
			delete[] pallocations;
			pallocations = pnewalloc;
			width = newwidth;
		}

		// try again
		R_AllocBlock(w, h, x, y, width, height, pallocations);
		return;
	}

	for(Uint32 i = 0; i < w; i++)
		pallocations[x+i] = ibest1+h;
}

//=============================================
// @brief
//
//=============================================
void R_BuildLightmap( Uint16 light_s, Uint16 light_t, const color24_t *psamples, const msurface_t *psurface, color32_t *pout, Int32 index, Uint32 sizex, Float overdarken, bool isvectormap, bool fullbright )
{
	const Uint32 smax = (psurface->extents[0] / psurface->lightmapdivider) + 1;
	const Uint32 tmax = (psurface->extents[1] / psurface->lightmapdivider) + 1;
	const Uint32 size = smax*tmax;
	
	color24_t *blocklights = new color24_t[size];
	color24_t *pblock = blocklights;

	if(!psamples || fullbright)
	{
		for (Uint32 j = 0; j < size; j++)
		{
			pblock[j].r = 128;
			pblock[j].g = 128;
			pblock[j].b = 128;
		}
	}
	else
	{
		const color24_t *psrc = psamples + size * index;

		for (Uint32 j = 0; j < size; j++)
		{
			color24_t lightcolor;
			if(isvectormap)
			{
				lightcolor.r = psrc[j].r;
				lightcolor.b = psrc[j].b;

				Int16 greenColor = psrc[j].g;
				greenColor = (greenColor-128);
				lightcolor.g = (-1.0 * greenColor)+128;
			}
			else
			{
				lightcolor.r = psrc[j].r;
				lightcolor.g = psrc[j].g;
				lightcolor.b = psrc[j].b;
			}

			pblock[j].r = _max(lightcolor.r, 255);
			pblock[j].g = _max(lightcolor.g, 255);
			pblock[j].b = _max(lightcolor.b, 255);
		}

		// Do not perform this on lightvecs
		if(overdarken > 0 && !isvectormap && index == 0)
		{
			const color24_t* prefsrc = psamples;
			for (Uint32 i = 0; i < size; i++)
			{
				Vector color = Vector(prefsrc[i].r, prefsrc[i].g, prefsrc[i].b);
				Math::VectorScale(color, 1.0f / 255.0f, color);

				// Darken pixels with low values, helps make maps darker despite overbright
				Float dot = Math::DotProduct(color, Vector(0.2126, 0.7152, 0.0722));
				Float flintensity = _max((dot * 255)/overdarken, 1);

				pblock[i].r = pblock[i].r*flintensity;
				pblock[i].g = pblock[i].g*flintensity;
				pblock[i].b = pblock[i].b*flintensity;
			}
		}
	}

	if(pout)
	{
		color32_t *pdest = pout + light_t * sizex + light_s;

		for (Uint32 i = 0; i < tmax; i++, pdest += sizex)
		{
			for (Uint32 j = 0; j < smax; j++)
			{
				pdest[j].r = pblock->r;
				pdest[j].g = pblock->g;
				pdest[j].b = pblock->b;
				pdest[j].a = 255;
				pblock++;
			}
		}
	}

	delete[] blocklights;
	return;
}

//=============================================
// @brief
//
//=============================================
bool R_IsEntityMoved( const cl_entity_t& entity )
{
	if(entity.curstate.angles[0] || entity.curstate.angles[1] || entity.curstate.angles[2] || 
		entity.curstate.origin[0] || entity.curstate.origin[1] || entity.curstate.origin[2] || 
		entity.curstate.frame || entity.curstate.effects & EF_CONVEYOR)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool R_IsEntityRotated( const cl_entity_t& entity )
{
	if(entity.curstate.angles[0] || entity.curstate.angles[1] || entity.curstate.angles[2])
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool R_IsEntityTransparent( const cl_entity_t& entity, bool ignoreVBMFlags )
{
	// Do not & with 255 since RENDER_TRANSALPHA_UNLIT needs to be rendered differently.
	if(entity.curstate.rendermode != RENDER_NORMAL
		&& entity.curstate.rendermode != RENDER_TRANSALPHA)
		return true;

	if(entity.pmodel->type == MOD_VBM && !ignoreVBMFlags)
	{
		const vbmcache_t* pcache = entity.pmodel->getVBMCache();
		return VBM_HasTransparentParts(pcache->pvbmhdr, entity.curstate.body, entity.curstate.skin);
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void R_RotateForEntity( CMatrix& matrix, const cl_entity_t& entity )
{
	matrix.Translate(entity.curstate.origin[0], entity.curstate.origin[1], entity.curstate.origin[2]);

	matrix.Rotate(entity.curstate.angles[1],  0, 0, 1);
	matrix.Rotate(entity.curstate.angles[0],  0, 1, 0);
	matrix.Rotate(entity.curstate.angles[2],  1, 0, 0);
}

//=============================================
// @brief
//
//=============================================
bool R_CheckShaderVertexAttribute( Int32 attribindex, const Char* pstrattribname, const CGLSLShader* pshader, void (*pfnErrorPopup)( const Char *fmt, ... ) )
{
	if(attribindex == CGLSLShader::PROPERTY_UNAVAILABLE)
	{
		CString errorPopup;
		errorPopup << __FUNCTION__ << " - ";
		const Char* pstrError = pshader->GetError();
		if(pstrError)
			errorPopup << pstrError;

		errorPopup << "\nError initializing vertex attribute '" << pstrattribname << "' for shader '" << pshader->GetShaderScriptName() << "'";
		pfnErrorPopup(errorPopup.c_str());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool R_CheckShaderDeterminator( Int32 attribindex, const Char* pstrattribname, const CGLSLShader* pshader, void (*pfnErrorPopup)( const Char *fmt, ... ) )
{
	if(attribindex == CGLSLShader::DETERMINATOR_UNDEFINED)
	{
		CString errorPopup;
		errorPopup << __FUNCTION__ << " - ";
		const Char* pstrError = pshader->GetError();
		if(pstrError)
			errorPopup << pstrError;

		errorPopup << "\nError initializing determinator '" << pstrattribname << "' for shader '" << pshader->GetShaderScriptName() << "'";
		pfnErrorPopup(errorPopup.c_str());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool R_CheckShaderUniform( Int32 attribindex, const Char* pstrattribname, const CGLSLShader* pshader, void (*pfnErrorPopup)( const Char *fmt, ... ) )
{
	if(attribindex == CGLSLShader::PROPERTY_UNAVAILABLE)
	{
		CString errorPopup;
		errorPopup << __FUNCTION__ << " - ";
		const Char* pstrError = pshader->GetError();
		if(pstrError)
			errorPopup << pstrError;

		errorPopup << "\nError initializing uniform '" << pstrattribname << "' for shader '" << pshader->GetShaderScriptName() << "'";
		pfnErrorPopup(errorPopup.c_str());
		return false;
	}

	return true;
}

//=============================================
//
// Function: GetRelativeX
//=============================================
Int32 R_GetRelativeX( Int32 xPos, Int32 baseWidth, Int32 windowWidth )
{
	Float outPos = (static_cast<Float>(xPos)/ static_cast<Float>(baseWidth));
	outPos = outPos*windowWidth;
	return static_cast<Int32>(outPos);
}

//=============================================
//
// Function: GetRelativeY
//=============================================
Int32 R_GetRelativeY( Int32 yPos, Int32 baseHeight, Int32 windowHeight )
{
	Float outPos = (static_cast<Float>(yPos)/ static_cast<Float>(baseHeight));
	outPos = outPos*windowHeight;
	return static_cast<Int32>(outPos);
}

//=============================================
//
// Function:
//=============================================
void R_ResizeTextureToPOT( Uint32& outwidth, Uint32& outheight, byte*& pdata )
{
	color32_t pix1, pix2, pix3, pix4;

	// convert texture to power of 2
	Uint32 width = outwidth;
	Uint32 height = outheight;
	for (outwidth = 1; outwidth < width && SDL_abs(outwidth-width) > 128; outwidth <<= 1);
	for (outheight = 1; outheight < height && SDL_abs(outheight-height) > 128; outheight <<= 1);

	// Allocate arrays
	Int32* prow1 = new Int32[outheight];
	Int32* prow2 = new Int32[outheight];
	Int32* pcol1 = new Int32[outwidth];
	Int32* pcol2 = new Int32[outwidth];

	color32_t* psrcdata = reinterpret_cast<color32_t*>(pdata);
	byte* poutdata = new byte[(outwidth*outheight*4*sizeof(byte))];
	byte* pout = poutdata;

	for (Uint32 i = 0; i < outwidth; i++)
	{
		pcol1[i] = static_cast<Int32>((i + 0.25) * (width / (Float)outwidth));
		pcol2[i] = static_cast<Int32>((i + 0.75) * (width / (Float)outwidth));
	}

	for (Uint32 i = 0; i < outheight; i++)
	{
		prow1[i] = static_cast<Int32>((i + 0.25) * (height / (Float)outheight)) * width;
		prow2[i] = static_cast<Int32>((i + 0.75) * (height / (Float)outheight)) * width;
	}

	for (Uint32 i = 0; i < outheight; i++)
	{
		for (Uint32 j = 0; j < outwidth; j++, pout += 4)
		{
			pix1 = psrcdata[prow1[i] + pcol1[j]];
			pix2 = psrcdata[prow1[i] + pcol2[j]];
			pix3 = psrcdata[prow2[i] + pcol1[j]];
			pix4 = psrcdata[prow2[i] + pcol2[j]];

			pout[0] = (pix1.r + pix2.r + pix3.r + pix4.r)>>2;
			pout[1] = (pix1.g + pix2.g + pix3.g + pix4.g)>>2;
			pout[2] = (pix1.b + pix2.b + pix3.b + pix4.b)>>2;
			pout[3] = (pix1.a + pix2.a + pix3.a + pix4.a)>>2;
		}
	}

	delete[] prow1;
	delete[] prow2;
	delete[] pcol1;
	delete[] pcol2;

	delete[] pdata;
	pdata = poutdata;
}

//=============================================
//
// Function:
//=============================================
void R_ResizeTexture32( Uint32 width, Uint32 height, Uint32 targetwidth, Uint32 targetheight, const color32_t* pindata, color32_t*& poutdata )
{
	color32_t pix1, pix2, pix3, pix4;

	// Allocate arrays
	Int32* prow1 = new Int32[targetheight];
	Int32* prow2 = new Int32[targetheight];
	Int32* pcol1 = new Int32[targetwidth];
	Int32* pcol2 = new Int32[targetwidth];

	poutdata = new color32_t[targetwidth*targetheight];
	byte* pout = reinterpret_cast<byte*>(poutdata);

	for (Uint32 i = 0; i < targetwidth; i++)
	{
		pcol1[i] = static_cast<Int32>((i + 0.25) * (width / static_cast<Float>(targetwidth)));
		pcol2[i] = static_cast<Int32>((i + 0.75) * (width / static_cast<Float>(targetwidth)));
	}

	for (Uint32 i = 0; i < targetheight; i++)
	{
		prow1[i] = static_cast<Int32>((i + 0.25) * (height / static_cast<Float>(targetheight))) * width;
		prow2[i] = static_cast<Int32>((i + 0.75) * (height / static_cast<Float>(targetheight))) * width;
	}

	for (Uint32 i = 0; i < targetheight; i++)
	{
		for (Uint32 j = 0; j < targetwidth; j++, pout += 4)
		{
			pix1 = pindata[prow1[i] + pcol1[j]];
			pix2 = pindata[prow1[i] + pcol2[j]];
			pix3 = pindata[prow2[i] + pcol1[j]];
			pix4 = pindata[prow2[i] + pcol2[j]];

			pout[0] = (pix1.r + pix2.r + pix3.r + pix4.r)>>2;
			pout[1] = (pix1.g + pix2.g + pix3.g + pix4.g)>>2;
			pout[2] = (pix1.b + pix2.b + pix3.b + pix4.b)>>2;
			pout[3] = (pix1.a + pix2.a + pix3.a + pix4.a)>>2;
		}
	}

	delete[] prow1;
	delete[] prow2;
	delete[] pcol1;
	delete[] pcol2;
}

//=============================================
//
// Function:
//=============================================
void R_ResizeTexture24( Uint32 width, Uint32 height, Uint32 targetwidth, Uint32 targetheight, const color24_t* pindata, color24_t*& poutdata )
{
	color24_t pix1, pix2, pix3, pix4;

	// Allocate arrays
	Int32* prow1 = new Int32[targetheight];
	Int32* prow2 = new Int32[targetheight];
	Int32* pcol1 = new Int32[targetwidth];
	Int32* pcol2 = new Int32[targetwidth];

	byte* pout = reinterpret_cast<byte*>(poutdata);

	for (Uint32 i = 0; i < targetwidth; i++)
	{
		pcol1[i] = static_cast<Int32>((i + 0.25) * (width / static_cast<Float>(targetwidth)));
		pcol2[i] = static_cast<Int32>((i + 0.75) * (width / static_cast<Float>(targetwidth)));
	}

	for (Uint32 i = 0; i < targetheight; i++)
	{
		prow1[i] = static_cast<Int32>((i + 0.25) * (height / static_cast<Float>(targetheight))) * width;
		prow2[i] = static_cast<Int32>((i + 0.75) * (height / static_cast<Float>(targetheight))) * width;
	}

	for (Uint32 i = 0; i < targetheight; i++)
	{
		for (Uint32 j = 0; j < targetwidth; j++, pout += 3)
		{
			pix1 = pindata[prow1[i] + pcol1[j]];
			pix2 = pindata[prow1[i] + pcol2[j]];
			pix3 = pindata[prow2[i] + pcol1[j]];
			pix4 = pindata[prow2[i] + pcol2[j]];

			pout[0] = (pix1.r + pix2.r + pix3.r + pix4.r)>>2;
			pout[1] = (pix1.g + pix2.g + pix3.g + pix4.g)>>2;
			pout[2] = (pix1.b + pix2.b + pix3.b + pix4.b)>>2;
		}
	}

	delete[] prow1;
	delete[] prow2;
	delete[] pcol1;
	delete[] pcol2;
}

//=============================================
//
// Function:
//=============================================
void R_FlipTexture( Uint32 width, Uint32 height, Uint32 bpp, bool fliph, bool flipv, byte*& pdata )
{
	// Flip vertically and/or horizontally if needed
	Uint32 outputSize = width*height*bpp;
	byte *pflipped = new byte[outputSize];
	for(Uint32 i = 0; i < height; i++)
	{
		byte *dst = pflipped + i*width*bpp;

		const byte *src;
		if(flipv)
			src = pdata + (height-i-1)*width*bpp;
		else
			src = pdata + i*width*bpp;

		if(fliph)
		{
			for(Uint32 j = 0; j < width; j++)
				memcpy(&dst[j*bpp], &src[((width-j-1)*bpp)], sizeof(byte)*bpp);
		}
		else
			memcpy(dst, src, sizeof(byte)*width*bpp);
	}

	delete[] pdata;
	pdata = pflipped;
}

//====================================
//
//====================================
bool R_WorldToScreenTransform( CMatrix& matrix, const Vector& src, Vector& screenstart )
{
	Vector scrcoords;

	const Float *pWorldToScreenMatrix = matrix.GetMatrix();
	scrcoords[0] = pWorldToScreenMatrix[0] * src[0] 
		+ pWorldToScreenMatrix[4] * src[1] 
		+ pWorldToScreenMatrix[8] * src[2] 
		+ pWorldToScreenMatrix[12];

	scrcoords[1] = pWorldToScreenMatrix[1] * src[0] 
		+ pWorldToScreenMatrix[5] * src[1] 
		+ pWorldToScreenMatrix[9] * src[2] 
		+ pWorldToScreenMatrix[13];

	scrcoords[2] = pWorldToScreenMatrix[3] * src[0] 
		+ pWorldToScreenMatrix[7] * src[1]
		+ pWorldToScreenMatrix[11] * src[2] 
		+ pWorldToScreenMatrix[15];

	screenstart[0] = scrcoords[0];
	screenstart[1] = scrcoords[1];

	Float zclipped;
	if(!scrcoords[2])
	{
		zclipped = scrcoords[2];
	}
	else
	{
		zclipped = 1.0 / scrcoords[2];
		screenstart[0] = scrcoords[0] * (1.0 / scrcoords[2]);
		screenstart[1] = scrcoords[1] * zclipped;
	}

	return (zclipped <= 0) ? true : false;
}

//====================================
//
//====================================
void R_SetMatrixData( const Float *pin, Float* pout, bool transpose )
{
	if(transpose)
	{
		pout[0] = pin[0]; pout[1] = pin[4]; pout[2] = pin[8]; pout[3] = pin[12];
		pout[4] = pin[1]; pout[5] = pin[5]; pout[6] = pin[9]; pout[7] = pin[13];
		pout[8] = pin[2]; pout[9] = pin[6]; pout[10] = pin[10]; pout[11] = pin[14];
		pout[12] = pin[3]; pout[13] = pin[7]; pout[14] = pin[11]; pout[15] = pin[15];
	}
	else
	{
		pout[0] = pin[0]; pout[1] = pin[1]; pout[2] = pin[2]; pout[3] = pin[3];
		pout[4] = pin[4]; pout[5] = pin[5]; pout[6] = pin[6]; pout[7] = pin[7];
		pout[8] = pin[8]; pout[9] = pin[9]; pout[10] = pin[10]; pout[11] = pin[11];
		pout[12] = pin[12]; pout[13] = pin[13]; pout[14] = pin[14]; pout[15] = pin[15];
	}
}

//====================================
//
//====================================
CString R_GetAspectRatio( Uint32 width, Uint32 height )
{
	Float ratio = static_cast<Float>(width) / static_cast<Float>(height);
	if(ratio < 1.5)
		return "4_3";
	else if(ratio > 1.5 && ratio <= 1.8)
		return "16_9";
	else
		return "21_9";
}