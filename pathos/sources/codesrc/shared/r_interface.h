/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_INTERFACE_H
#define R_INTERFACE_H

class CGLExtF;
class CMatrix;

struct ref_params_t;
struct font_set_t;
struct cl_dlight_t;
struct cl_entity_t;

#include "textures_shared.h"
#include "common.h"
#include "vbm_shared.h"

struct r_interface_t
{
	// Retreives the dummy texture
	en_texture_t*		(*pfnGetDummyTexture)( void );
	// Retreives the dummy material script
	en_material_t*		(*pfnGetDummyMaterial) ( void );

	// Creates a texture index allocation
	en_texalloc_t*		(*pfnAllocTextureIndex)( rs_level_t level );
	// Loads a texture
	en_texture_t*		(*pfnLoadTexture)( const Char* pstrFilename, rs_level_t level, Int32 flags, const GLint* pborder );
	// Loads a texture from memory
	en_texture_t*		(*pfnLoadTextureFromMemory)( const Char* pstrTextureName, rs_level_t level, Int32 flags, const byte* pdata, Uint32 width, Uint32 height, Uint32 bpp );
	// Loads a material script
	en_material_t*		(*pfnLoadMaterialScript)( const Char* pstrFilename, rs_level_t level );

	// Binds a texture to a texture unit
	void				(*pfnBind2DTexture)( Int32 texture, Uint32 id, bool force );
	// Clears all memorized binds
	void				(*pfnClearBinds)( void );

	// Tells if basic draw is active
	bool				(*pfnBasicDrawIsActive)( void );
	// Enable basic draw
	bool				(*pfnEnableBasicDraw)( void );
	// Disable basic draw
	void				(*pfnDisableBasicDraw)( void );
	// Enable textures in basic draw
	bool				(*pfnBasicDrawEnableTextures)( void );
	// Disable textures in basic draw
	bool				(*pfnBasicDrawDisableTextures)( void );

	// Resets states for primitive rendering
	void				(*pfnBasicDrawBegin)( Int32 primitiveType );
	// Renders the primitives
	void				(*pfnBasicDrawEnd)( void );

	// Sets the projection matrix
	void				(*pfnBasicDrawSetProjection)( const Float* pMatrix );
	// Sets the modelview matrix
	void				(*pfnBasicDrawSetModelView)( const Float* pMatrix );

	// Sets a vertex's color
	void				(*pfnBasicDrawColor4f)( Float r, Float g, Float b, Float a );
	void				(*pfnBasicDrawColor4fv)( const Float* pfc );
	// Sets a vertex's texcoords
	void				(*pfnBasicDrawTexCoord2f)( Float u, Float v );
	void				(*pfnBasicDrawTexCoord2fv)( const Float* ptc );
	// Sets a vertex's origin
	void				(*pfnBasicDrawVertex3f)( Float x, Float y, Float z );
	void				(*pfnBasicDrawVertex3fv)( const Float* pfv );

	// Retreives the class holding the export functions
	const CGLExtF&		(*pfnGetExportFunctionsClass)( void );

	// Retreives the current projection matrix
	CMatrix&			(*pfnGetProjectionMatrix)( void );
	// Retreives the current modelview matrix
	CMatrix&			(*pfnGetModelViewMatrix)( void );

	// Retreives view params structure
	const ref_params_t&	(*pfnGetViewParams)( void );
	// Retreives window width and height
	void				(*pfnGetScreenSize)( Uint32& scrwidth, Uint32& scrheight );


	// Gets the default font set
	const font_set_t*	(*pfnGetDefaultFontSet)( void );
	// Loads a font set
	const font_set_t*	(*pfnLoadFontSet)( const Char *pstrFilename, Int32 fontSize );
	// Draws a simple string on the screen
	bool				(*pfnDrawSimpleString)( color32_t color, Int32 x, Int32 y, const Char* pstrString, const font_set_t* pfont );
	// Draws a string box on the screen
	bool				(*pfnDrawStringBox)( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int16 insetx, Int16 insety, bool reverse, color32_t color, Int32 x, Int32 y, const Char* pstrString, const font_set_t* pfont, Uint32 offset, Uint32 minlineheight, Uint32 xoffset );
	// Sets up for text rendering
	bool				(*pfnBeginTextRendering)( const font_set_t* pfontset );
	// Ends text rendering
	void				(*pfnFinishTextRendering)( const font_set_t* pfontset );
	// Draws a single character
	bool				(*pfnDrawCharacter)( const font_set_t* pfontset, Int32 x, Int32 y, Char character, Uint32 r, Uint32 g, Uint32 b, Uint32 a );
	// Returns the size of a string in pixels
	void				(*pfnGetStringSize)( const font_set_t *pset, const Char *pstring, Uint32 *width, Uint32 *height, Int32 *ymin );
	// Estimates height of a string
	Int32				(*pfnEstimateStringHeight)( const font_set_t *pset, const Char *pstrString, Uint32 minlineheight );
	// Sets the string drawing rectangle
	void				(*pfnSetStringRectangle)( Int16 minx, Int16 miny, Int16 maxx, Int16 maxy, Int32 insetx, Int32 insety );
	// Returns the error string from the text renderer class
	const Char*			(*pfnGetStringDrawError)( void );

	// Prepares for vbm rendering
	bool				(*pfnVBMPrepareDraw)( void );
	// Finishes vbm rendering
	void				(*pfnVBMEndDraw)( void );
	// Renders a VBM model
	bool				(*pfnDrawVBMModel)( cl_entity_t* pentity, Int32 renderflags );

	// Prepares for vbm vsm rendering
	bool				(*pfnVBMPrepareVSMDraw)( cl_dlight_t* pdlight );
	// Finishes vbm vsm rendering
	void				(*pfnVBMEndVSMDraw)( void );
	// Renders a VBM model for vsm
	bool				(*pfnDrawVBMModelVSM)( cl_entity_t* pentity, cl_dlight_t* pdlight );

	// Returns the error string from the VBM shader object
	const Char*			(*pfnGetVBMShaderError)( void );

	// Returns the number of entities to be rendered
	Uint32				(*pfnGetNumRenderEntities)( void );
	// Returns a renderable entity
	cl_entity_t*		(*pfnGetRenderEntityByIndex)( Uint32 index );

	// Sets the projection matrix
	void				(*pfnSetProjectionMatrix)( Float znear, Float fovY );
	// Sets the modelview matrix
	void				(*pfnSetModelViewMatrix)( const Vector& origin, const Vector& angles );
	
	// Draws particles for view model
	bool				(*pfnDrawViewModelParticles)( void );

	// Validates shader states for a shader
	void				(*pfnValidateShader)( class CGLSLShader* pShader );
	// Validates shader states for basic drawing
	void				(*pfnValidateBasicDraw)( void );
};

#endif //R_INTERFACE_H