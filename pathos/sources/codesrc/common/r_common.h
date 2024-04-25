/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef R_COMMON_H
#define R_COMMON_H

#include "carray.h"

struct msurface_t;
struct cl_entity_t;
struct en_texture_t;

class CMatrix;
class CGLSLShader;

extern Int32 R_StyleIndex ( const msurface_t *psurface, Uint32 style );
extern void R_AllocBlock ( Uint32 w, Uint32 h, Uint32 &x, Uint32 &y, Uint32 width, Uint32 &height, Uint32* pallocations );
extern color24_t *R_BuildLightmap( Uint16 light_s, Uint16 light_t, const color24_t *psamples, const msurface_t *psurface, color32_t *pout, Int32 index, Uint32 sizex, Float overdarken, bool isvectormap = false, bool fullbright = false );
extern bool R_IsEntityMoved( const cl_entity_t& entity );
extern bool R_IsEntityRotated( const cl_entity_t& entity );
extern bool R_IsEntityTransparent( const cl_entity_t& entity, bool ignoreVBMFlags = false );
extern void R_RotateForEntity( CMatrix& matrix, const cl_entity_t& entity );
extern bool R_CheckShaderVertexAttribute( Int32 attribindex, const Char* pstrattribname, const CGLSLShader* pshader, void	(*pfnErrorPopup)( const Char *fmt, ... ) );
extern bool R_CheckShaderDeterminator( Int32 attribindex, const Char* pstrattribname, const CGLSLShader* pshader, void	(*pfnErrorPopup)( const Char *fmt, ... ) );
extern bool R_CheckShaderUniform( Int32 attribindex, const Char* pstrattribname, const CGLSLShader* pshader, void	(*pfnErrorPopup)( const Char *fmt, ... ) );
extern bool R_WorldToScreenTransform( CMatrix& matrix, const Vector& src, Vector& screenstart );

extern Int32 R_GetRelativeX( Int32 xPos, Int32 baseWidth, Int32 windowWidth );
extern Int32 R_GetRelativeY( Int32 yPos, Int32 baseHeight, Int32 windowHeight );
extern void R_ResizeTextureToPOT( Uint32& outwidth, Uint32& outheight, byte*& pdata );
extern void R_FlipTexture( Uint32 width, Uint32 height, Uint32 bpp, bool fliph, bool flipv, byte*& pdata );
#endif