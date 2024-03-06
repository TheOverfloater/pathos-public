/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_INTERFACEFUNCS_H
#define R_INTERFACEFUNCS_H

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

struct cl_dlight_t;

extern en_texture_t* R_GetDummyTexture( void );
extern en_material_t* R_GetDummyMaterial( void );
extern en_texalloc_t* R_AllocTextureIndex( rs_level_t level );
extern en_texture_t* R_LoadTexture( const Char* pstrFilename, rs_level_t level, Int32 flags, const GLint* pborder );
extern en_texture_t* R_LoadTextureFromMemory( const Char* pstrTextureName, rs_level_t level, Int32 flags, const byte* pdata, Uint32 width, Uint32 height, Uint32 bpp );
extern en_material_t* R_LoadMaterialScript( const Char* pstrFilename, rs_level_t level );
extern bool R_EnableBasicDraw( void );
extern void R_DisableBasicDraw( void );
extern bool R_BasicDrawEnableTextures( void );
extern bool R_BasicDrawDisableTextures( void );
extern void R_BasicDrawBegin( Int32 primitiveType );
extern void R_BasicDrawEnd( void );
extern void R_BasicDrawSetProjection( const Float* pMatrix );
extern void R_BasicDrawSetModelView( const Float* pMatrix );
extern void R_BasicDrawColor4f( Float r, Float g, Float b, Float a );
extern void R_BasicDrawColor4fv( const Float* pfc );
extern void R_BasicDrawTexCoord2f( Float u, Float v );
extern void R_BasicDrawTexCoord2fv( const Float* ptc );
extern void R_BasicDrawVertex3f( Float x, Float y, Float z );
extern void R_BasicDrawVertex3fv( const Float* pfv );
extern const CGLExtF& R_GetExportFunctionsClass( void );
extern CMatrix& R_GetProjectionMatrix( void );
extern CMatrix& R_GetModelViewMatrix( void );
extern const ref_params_t&	R_GetViewParams( void );
extern const font_set_t* R_GetDefaultFontSet( void );
extern const font_set_t* R_LoadFontSet( const Char *pstrFilename, Int32 fontSize );
extern void R_GetScreenSize( Uint32& scrwidth, Uint32& scrheight );
extern bool R_BasicDrawIsActive( void );
extern void R_GetStringSize( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height, Int32 *ymin );
extern Int32 R_EstimateStringHeight( const font_set_t *pset, const Char *pstrString, Uint32 minlineheight );
extern void R_SetStringRectangle( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety );
extern const Char* R_GetStringDrawError( void );
extern bool R_VBMPrepareDraw( void );
extern void R_VBMEndDraw( void );
extern bool R_DrawVBMModel( cl_entity_t* pentity, Int32 renderflags );
extern bool R_VBMPrepareVSMDraw( cl_dlight_t* pdlight );
extern void R_VBMEndVSMDraw( void );
extern bool R_DrawVBMModelVSM( cl_entity_t* pentity, cl_dlight_t* pdlight );
extern const Char* R_VBMGetShaderError( void );
extern void R_InitRenderInterface( r_interface_t &renderFuncs );
extern Uint32 R_GetNumRenderEntities( void );
extern cl_entity_t* R_GetRenderEntityByIndex( Uint32 index );
extern void R_IF_ValidateShader( CGLSLShader* pShader );
extern void R_IF_ValidateBasicDraw( void );
#endif //R_INTERFACEFUNCS_H