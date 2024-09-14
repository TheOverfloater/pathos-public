/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "screenfade.h"
#include "r_rttcache.h"

enum pp_shadertypes_t
{
	SHADER_GAMMA = 0,
	SHADER_BLUR_H,
	SHADER_BLUR_V,
	SHADER_DISTORT,
	SHADER_MBLUR,
	SHADER_ENVFADE,
	SHADER_GRAIN,
	SHADER_NORMAL,
	SHADER_BLOOM_DARKEN,
	SHADER_BLOOM_APPLY,
	SHADER_BLOOM_BLUR_H,
	SHADER_BLOOM_BLUR_V
};

struct pp_shader_attribs
{
	pp_shader_attribs():
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_tcscale(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_offset(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_gamma(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_screenwidth(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_screenheight(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_timer(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_offsetdivider(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture1rect(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture2rect(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_darken_steps(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_brighten_multiplier(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_blur_brightness(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_blur_radius(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_brightness_treshold(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_type(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_rectangle(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32	u_modelview;
	Int32	u_projection;
	Int32	u_tcscale;

	Int32	u_offset;
	Int32	u_color;
	Int32	u_gamma;

	Int32	u_screenwidth;
	Int32	u_screenheight;
	Int32	u_timer;
	Int32	u_offsetdivider;

	Int32	u_texture1rect;
	Int32	u_texture2rect;

	Int32	u_darken_steps;
	Int32	u_brighten_multiplier;
	Int32	u_blur_brightness;
	Int32	u_blur_radius;
	Int32	u_brightness_treshold;

	Int32	a_origin;
	Int32	a_texcoord;

	Int32 d_type;
	Int32 d_rectangle;
};

/*
====================
CPostProcess

====================
*/
class CPostProcess
{
public:
	CPostProcess( void );
	~CPostProcess( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Clears OpenGL objects
	void ClearGL( void );
	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

	// Draws postprocess effects
	bool Draw( void );

private:
	// Draws gamma correction
	bool DrawGamma( void );
	// Draws gaussian blur
	bool DrawBlur( void );
	// Draws distortion effect
	bool DrawDistortion( void );
	// Draws motion blur
	bool DrawMotionBlur( void );
	// Draws fade effects
	bool DrawFade( screenfade_t& fade );
	// Draws screen film grain
	bool DrawFilmGrain( void );
	// Draw bloom
	bool DrawBloom( void );

	// Fetches screen contents
	static void FetchScreen( rtt_texture_t** ptarget );

	// Creates motion blur texture
	void ClearMotionBlur( void );

	// Creates the screen texture
	void CreateBlurScreenTexture( void );

public:
	// Toggle motion blur effect message
	void SetMotionBlur( bool active, Float blurfade, bool override );
	// Reads fade message
	void SetFade( Uint32 layerindex, Float duration, Float holdtime, Int32 flags, const color24_t& color, byte alpha, Float timeoffset );
	// Sets gaussian blur
	void SetGaussianBlur( bool active, Float alpha );

private:
	// Gamma cvar
	CCVar*			m_pCvarGamma;
	// Blur FBO binding
	fbobind_t		m_blurFBO;

	// TRUE if gaussian blur is active
	bool			m_gaussianBlurActive;
	// TRUE if motion blur is active
	bool			m_motionBlurActive;
	// TRUe if need to override motion blur
	bool			m_blurOverride;
	
	// True if first game of motion blur
	bool			m_isFirstFrame;
	// Motion blur fade amount
	Float			m_blurFade;

	// Last time we were in water
	Float			m_lastWaterTime;
	// Gaussian blur alpha
	Float			m_gaussianBlurAlpha;

private:
	// Screenfade information
	screenfade_t	m_fadeLayersArray[MAX_FADE_LAYERS];

private:
	// Pointer to shader object
	class CGLSLShader*	m_pShader;
	// Pointer to VBO object
	class CVBO*			m_pVBO;

	// Filmgrain cvar
	CCVar*			m_pCvarFilmGrain;
	// Postporcess cvar
	CCVar*			m_pCvarPostProcess;
	// Bloom cvar
	CCVar*			m_pCvarBloom;
	// Bloom darken steps cvar
	CCVar*			m_pCvarBloomDarkenSteps;
	// Bloom darken brightness multiplier cvar
	CCVar*			m_pCvarBloomDarkenMultiplier;
	// Bloom blur steps cvar
	CCVar*			m_pCvarBloomBlurSteps;
	// Bloom brighten multiplier cvar
	CCVar*			m_pCvarBloomBrightenMultiplier;
	// Bloom darken steps cvar
	CCVar*			m_pCvarBloomBrightnessTreshold;

	// Screen RTT
	rtt_texture_t*	m_pScreenRTT;

	// Screen texture pointer
	en_texalloc_t*	m_pBlurScreenTexture;

	// Shader attributes
	pp_shader_attribs m_attribs;
};

extern CPostProcess gPostProcess;
#endif