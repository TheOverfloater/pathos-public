/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "texturemanager.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_basicdraw.h"
#include "r_glextf.h"
#include "r_main.h"
#include "r_interface.h"
#include "r_text.h"
#include "window.h"
#include "r_vbm.h"
#include "r_interfacefuncs.h"

//
// Renderer interface functions
//
static r_interface_t CLDLL_RENDERFUNCS_INTERFACE = 
{
	R_GetDummyTexture,				//pfnGetDummyTexture
	R_GetDummyMaterial,				//pfnGetDummyMaterial
	R_AllocTextureIndex,			//pfnAllocTextureIndex
	R_LoadTexture,					//pfnLoadTexture
	R_LoadTextureFromMemory,		//pfnLoadTextureFromMemory
	R_LoadMaterialScript,			//pfnLoadMaterialScript
	R_Bind2DTexture,				//pfnBind2DTexture
	R_ClearBinds,					//pfnClearBinds
	R_BasicDrawIsActive,			//pfnBasicDrawIsActive
	R_EnableBasicDraw,				//pfnEnableBasicDraw
	R_DisableBasicDraw,				//pfnDisableBasicDraw
	R_BasicDrawEnableTextures,		//pfnBasicDrawEnableTextures
	R_BasicDrawDisableTextures,		//pfnBasicDrawDisableTextures
	R_BasicDrawBegin,				//pfnBasicDrawBegin
	R_BasicDrawEnd,					//pfnBasicDrawEnd
	R_BasicDrawSetProjection,		//pfnBasicDrawSetProjection
	R_BasicDrawSetModelView,		//pfnBasicDrawSetModelView
	R_BasicDrawColor4f,				//pfnBasicDrawColor4f
	R_BasicDrawColor4fv,			//pfnBasicDrawColor4fv
	R_BasicDrawTexCoord2f,			//pfnBasicDrawTexCoord2f
	R_BasicDrawTexCoord2fv,			//pfnBasicDrawTexCoord2fv
	R_BasicDrawVertex3f,			//pfnBasicDrawVertex3f
	R_BasicDrawVertex3fv,			//pfnBasicDrawVertex3fv
	R_GetExportFunctionsClass,		//pfnGetExportFunctionsClass
	R_GetProjectionMatrix,			//pfnGetProjectionMatrix
	R_GetModelViewMatrix,			//pfnGetModelViewMatrix
	R_GetViewParams,				//pfnGetViewParams
	R_GetScreenSize,				//pfnGetScreenSize
	R_GetDefaultFontSet,			//pfnGetDefaultFontSet
	R_LoadFontSet,					//pfnLoadFontSet
	R_DrawString,					//pfnDrawSimpleString
	R_DrawStringBox,				//pfnDrawStringBox
	R_BeginTextRendering,			//pfnBeginTextRendering
	R_FinishTextRendering,			//pfnFinishTextRendering
	R_DrawCharacter,				//pfnDrawCharacter
	R_GetStringSize,				//pfnGetStringSize
	R_EstimateStringHeight,			//pfnEstimateStringHeight
	R_SetStringRectangle,			//pfnSetStringRectangle
	R_GetStringDrawError,			//pfnGetStringDrawError
	R_VBMPrepareDraw,				//pfnVBMPrepareDraw
	R_VBMEndDraw,					//pfnVBMEndDraw
	R_DrawVBMModel,					//pfnDrawVBMModel
	R_VBMPrepareVSMDraw,			//pfnVBMPrepareVSMDraw
	R_VBMEndVSMDraw,				//pfnVBMEndVSMDraw
	R_DrawVBMModelVSM,				//pfnDrawVBMModelVSM
	R_VBMGetShaderError,			//pfnVBMGetShaderError
	R_GetNumRenderEntities,			//pfnGetNumRenderEntities
	R_GetRenderEntityByIndex,		//pfnGetRenderEntityByIndex
	R_SetProjectionMatrix,			//pfnSetProjectionMatrix
	R_SetModelViewMatrix,			//pfnSetModelViewMatrix
	R_DrawViewModelParticles,		//pfnDrawViewModelParticles
	R_IF_ValidateShader,			//pfnValidateShader
	R_IF_ValidateBasicDraw			//pfnValidateBasicDraw
};

//====================================
//
//====================================
en_texture_t* R_GetDummyTexture( void )
{
	return CTextureManager::GetInstance()->GetDummyTexture();
}

//====================================
//
//====================================
en_material_t* R_GetDummyMaterial( void )
{
	return CTextureManager::GetInstance()->GetDummyMaterial();
}

//====================================
//
//====================================
en_texalloc_t* R_AllocTextureIndex( rs_level_t level )
{
	return CTextureManager::GetInstance()->GenTextureIndex(level);
}

//====================================
//
//====================================
en_texture_t* R_LoadTexture( const Char* pstrFilename, rs_level_t level, Int32 flags, const GLint* pborder )
{
	return CTextureManager::GetInstance()->LoadTexture(pstrFilename, level, flags, pborder);
}

//====================================
//
//====================================
en_texture_t* R_LoadTextureFromMemory( const Char* pstrTextureName, rs_level_t level, Int32 flags, const byte* pdata, Uint32 width, Uint32 height, Uint32 bpp )
{
	return CTextureManager::GetInstance()->LoadFromMemory(pstrTextureName, level, flags, pdata, width, height, bpp);
}

//====================================
//
//====================================
en_material_t* R_LoadMaterialScript( const Char* pstrFilename, rs_level_t level )
{
	return CTextureManager::GetInstance()->LoadMaterialScript(pstrFilename, level);
}

//====================================
//
//====================================
bool R_EnableBasicDraw( void )
{
	return CBasicDraw::GetInstance()->Enable();
}

//====================================
//
//====================================
void R_DisableBasicDraw( void )
{
	return CBasicDraw::GetInstance()->Disable();
}

//====================================
//
//====================================
bool R_BasicDrawEnableTextures( void )
{
	return CBasicDraw::GetInstance()->EnableTexture();
}

//====================================
//
//====================================
bool R_BasicDrawDisableTextures( void )
{
	return CBasicDraw::GetInstance()->DisableTexture();
}

//====================================
//
//====================================
void R_BasicDrawBegin( Int32 primitiveType )
{
	CBasicDraw::GetInstance()->Begin(primitiveType);
}

//====================================
//
//====================================
void R_BasicDrawEnd( void )
{
	CBasicDraw::GetInstance()->End();
}

//====================================
//
//====================================
void R_BasicDrawSetProjection( const Float* pMatrix )
{
	CBasicDraw::GetInstance()->SetProjection(pMatrix);
}

//====================================
//
//====================================
void R_BasicDrawSetModelView( const Float* pMatrix )
{
	CBasicDraw::GetInstance()->SetModelview(pMatrix);
}

//====================================
//
//====================================
void R_BasicDrawColor4f( Float r, Float g, Float b, Float a )
{
	CBasicDraw::GetInstance()->Color4f(r, g, b, a);
}

//====================================
//
//====================================
void R_BasicDrawColor4fv( const Float* pfc )
{
	CBasicDraw::GetInstance()->Color4fv(pfc);
}

//====================================
//
//====================================
void R_BasicDrawTexCoord2f( Float u, Float v )
{
	CBasicDraw::GetInstance()->TexCoord2f(u, v);
}

//====================================
//
//====================================
void R_BasicDrawTexCoord2fv( const Float* ptc )
{
	CBasicDraw::GetInstance()->TexCoord2fv(ptc);
}

//====================================
//
//====================================
void R_BasicDrawVertex3f( Float x, Float y, Float z )
{
	CBasicDraw::GetInstance()->Vertex3f(x, y, z);
}

//====================================
//
//====================================
void R_BasicDrawVertex3fv( const Float* pfv )
{
	CBasicDraw::GetInstance()->Vertex3fv(pfv);
}

//====================================
//
//====================================
const CGLExtF& R_GetExportFunctionsClass( void )
{
	return gGLExtF;
}

//====================================
//
//====================================
CMatrix& R_GetProjectionMatrix( void )
{
	return rns.view.projection;
}

//====================================
//
//====================================
CMatrix& R_GetModelViewMatrix( void )
{
	return rns.view.modelview;
}

//====================================
//
//====================================
const ref_params_t&	R_GetViewParams( void )
{
	return rns.view.params;
}

//====================================
//
//====================================
const font_set_t* R_GetDefaultFontSet( void )
{
	return gText.GetDefaultFont();
}

//====================================
//
//====================================
const font_set_t* R_LoadFontSet( const Char *pstrFilename, Int32 fontSize )
{
	return gText.LoadFont(pstrFilename, fontSize);
}

//====================================
//
//====================================
void R_GetScreenSize( Uint32& scrwidth, Uint32& scrheight )
{
	scrwidth = gWindow.GetWidth();
	scrheight = gWindow.GetHeight();
}

//====================================
//
//====================================
bool R_BasicDrawIsActive( void )
{
	return CBasicDraw::GetInstance()->IsActive();
}

//====================================
//
//====================================
void R_GetStringSize( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height, Int32 *ymin )
{
	gText.GetStringSize(pset, pstring, width, height, ymin);
}

//====================================
//
//====================================
Int32 R_EstimateStringHeight( const font_set_t *pset, const Char *pstrString, Uint32 minlineheight )
{
	return gText.EstimateHeight(pset, pstrString, minlineheight);
}

//====================================
//
//====================================
void R_SetStringRectangle( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety )
{
	gText.SetRectangle(minx, miny, maxx, maxy, insetx, insety);
}

//====================================
//
//====================================
const Char* R_GetStringDrawError( void )
{
	return gText.GetShaderError();
}

//====================================
//
//====================================
bool R_VBMPrepareDraw( void )
{
	return gVBMRenderer.PrepareDraw();
}

//====================================
//
//====================================
void R_VBMEndDraw( void )
{
	gVBMRenderer.EndDraw();
}

//====================================
//
//====================================
bool R_DrawVBMModel( cl_entity_t* pentity, Int32 renderflags )
{
	return gVBMRenderer.DrawModel(renderflags, pentity);
}

//====================================
//
//====================================
bool R_VBMPrepareVSMDraw( cl_dlight_t* pdlight )
{
	return gVBMRenderer.PrepareVSM(pdlight);
}

//====================================
//
//====================================
void R_VBMEndVSMDraw( void )
{
	gVBMRenderer.EndVSM();
}

//====================================
//
//====================================
const Char* R_VBMGetShaderError( void )
{
	return gVBMRenderer.GetShaderErrorString();
}

//====================================
//
//====================================
bool R_DrawVBMModelVSM( cl_entity_t* pentity, cl_dlight_t* pdlight )
{
	return gVBMRenderer.DrawModelVSM(pentity, pdlight);
}

//====================================
//
//====================================
Uint32 R_GetNumRenderEntities( void )
{
	return rns.objects.numvisents;
}

//====================================
//
//====================================
cl_entity_t* R_GetRenderEntityByIndex( Uint32 index )
{
	if(index >= rns.objects.numvisents)
		return nullptr;

	return rns.objects.pvisents[index];
}

//====================================
//
//====================================
void R_IF_ValidateShader( CGLSLShader* pShader )
{
	R_ValidateShader(pShader);
}

//====================================
//
//====================================
void R_IF_ValidateBasicDraw( void )
{
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw)
		return;

	R_ValidateShader(pDraw);
}

//====================================
//
//====================================
void R_InitRenderInterface( r_interface_t &renderFuncs )
{
	renderFuncs = CLDLL_RENDERFUNCS_INTERFACE;
}

