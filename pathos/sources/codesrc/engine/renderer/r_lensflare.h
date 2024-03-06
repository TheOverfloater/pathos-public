/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_LENSFLARE_H
#define R_LENSFLARE_H

#include "cl_entity.h"

class CBasicDraw;
struct en_texture_t;

/*
=================================
CLensFlareRenderer

=================================
*/
class CLensFlareRenderer
{
public:
	struct sunflare_t
	{
		// Sun flare pitch
		Float pitch;
		// Sun flare roll
		Float roll;
		// Sun flare scale
		Float scale;
		// Sun flare color
		Vector color;
		// TRUE if sun flare is part of a portal skybox
		bool portal;
		// Entity index of sunflare
		entindex_t entindex;

		// Sun glow state
		glowstate_t glowstate;
	};

public:
	// Number of halos drawn
	static const Uint32 NB_FLARE_HALOS = 6;
	// Number of glow tracelines
	static const Uint32 GLOW_NUM_TRACES;
	// Glow interpolation speed
	static const Float GLOW_INTERP_SPEED;
	// Sun glow default scale
	static const Float SUN_GLOW_DEFAULT_SCALE;

public:
	// Lens flare texture base path
	static const Char FLARE_TEXTURE_PATH[];
	// Sun glare texture base path
	static const Char SUNGLARE_TEXTURE_PATH[];
	// Textures used by each halo
	static const Int32 FLARE_HALO_TEXTURES[NB_FLARE_HALOS];
	// Scales for each individual halo
	static const Float FLARE_HALO_SCALES[NB_FLARE_HALOS];
	// Alpha values used by halos
	static const Float FLARE_HALO_ALPHAS[NB_FLARE_HALOS];
	// Distance scales used by halos
	static const Float FLARE_HALO_DIST_SCALES[NB_FLARE_HALOS];

public:
	enum lensflare_textures_t
	{
		LF_TEX_CORE = 0,
		LF_TEX_HALO1,
		LF_TEX_HALO2,
		LF_TEX_HALO3,

		NB_LENSFLARE_TEXTURES
	};

public:
	CLensFlareRenderer( void );
	~CLensFlareRenderer( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes game objects
	bool InitGame( void );
	// Shuts down game objects
	void ClearGame( void );

	// Draws the particles
	bool DrawLensFlares( void );

public:
	// Sets sun flare properties
	void SetSunFlare( entindex_t entindex, bool active, Float pitch, Float roll, Float scale, const Vector& color, bool portalSunFlare );

	// Pre-draw function for rendering glow dots
	void PreDrawFunction( void );
	// Draw function for rendering glow dots
	void DrawFunction( const Vector& origin );

private:
	// Draws a single lens flare effect
	bool DrawLensFlare( Int32 key, const Vector& origin, const Vector& rendercolor, Float renderamt, Float scale, glowstate_t& glowstate, Int32 renderfx, bool isSun, bool portalSun );
	// Draws a single quad on the screen
	void DrawQuad( const Vector& position, const Vector& color, en_texture_t* ptexture, Float scale, Float alpha );
	// Draws the sun flare
	void DrawSunFlare( sunflare_t& sunflare );

private:
	// Modelview matrix
	CMatrix m_modelViewMatrix;
	// Projection matrix
	CMatrix m_projectionMatrix;
	// Lens flare textures
	en_texture_t* m_pLensFlareTextures[NB_LENSFLARE_TEXTURES];
	// Sun glare texture
	en_texture_t* m_pSunGlareTexture;
	// BasicDraw instance
	CBasicDraw* m_pBasicDraw;

	// View matrix
	Float m_viewMatrix[3][4];

	// Toggles expensive glow tracelining
	CCVar*	m_pCvarSunDebugPitch;
	// Toggles expensive glow tracelining
	CCVar*	m_pCvarSunDebugRoll;

	// Sunflare array
	CArray<sunflare_t> m_sunFlaresArray;
};
extern CLensFlareRenderer gLensFlareRenderer;
extern void LF_PreRender( void* pContext );
extern void LF_DrawFunction( void* pContext, const Vector& origin );
#endif //R_LENSFLARE_H