/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_main.h"
#include "cl_main.h"
#include "system.h"
#include "common.h"
#include "r_lightstyles.h"

// Custom lightstyle start index(for dynamic lights)
const Uint32 CLightStyleManager::CUSTOM_LIGHTSTYLE_START_INDEX = 255;
// Default lightstyle framerate
const Char CLightStyleManager::DEFAULT_LIGHTSTYLE_FRAMERATE = 10;
// Maximum lightstyle string length
const Uint32 CLightStyleManager::MAX_STYLESTRING = 64;

// Object definition
CLightStyleManager gLightStyles;

//====================================
//
//====================================
CLightStyleManager::CLightStyleManager( void )
{
}

//====================================
//
//====================================
CLightStyleManager::~CLightStyleManager( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CLightStyleManager::Init( void )
{
	return true;
}

//====================================
//
//====================================
void CLightStyleManager::Shutdown( void )
{
	ClearGame();
}

//====================================
//
//====================================
bool CLightStyleManager::InitGame( void )
{
	return true;
}

//====================================
//
//====================================
void CLightStyleManager::ClearGame( void )
{
	ResetStyles();
}

//====================================
//
//====================================
void CLightStyleManager::ResetStyles( void )
{
	if(m_lightStyles.empty())
		return;
		
	m_lightStyles.clear();
}

//====================================
//
//====================================
void CLightStyleManager::AddCustomLightStyle( Uint32 index, Int32 framerate, bool interpolate, const Char* pstring )
{
	if(index < CUSTOM_LIGHTSTYLE_START_INDEX)
	{
		Con_Printf("%s - Invalid lightstyle index %d for custom lightstyle.\n", __FUNCTION__, index);
		return;
	}

	Uint32 length = qstrlen(pstring);
	if(length+1 >= MAX_STYLESTRING)
	{
		Con_Printf("Error: Lightstyle with index %d is too long.\n", index);
		return;
	}

	Uint32 insertPosition = m_lightStyles.size();
	m_lightStyles.resize(m_lightStyles.size()+1);
	lightstyle_t& style = m_lightStyles[insertPosition];

	style.map.resize(length+1);
	
	for(Uint32 i = 0; i < length; i++)
		style.map[i] = pstring[i];

	style.map[length] = '\0';

	style.length = length;
	style.framerate = framerate;
	style.interp = interpolate;
	style.index = index;

	if(!style.framerate)
		style.framerate = DEFAULT_LIGHTSTYLE_FRAMERATE;

	// Ensure array size is proper
	if(m_lightStyleValues.size() <= style.index)
		m_lightStyleValues.resize(style.index+1);
}

//====================================
//
//====================================
void CLightStyleManager::SetLightStyle( Uint32 index, Int32 framerate, bool interpolate, const Char* pstring )
{
	if(index >= CUSTOM_LIGHTSTYLE_START_INDEX)
	{
		Con_Printf("%s - Invalid lightstyle index %d for non-custom lightstyle.\n", __FUNCTION__, index);
		return;
	}

	Uint32 length = qstrlen(pstring);
	if(length+1 >= MAX_STYLESTRING)
	{
		Con_Printf("Error: Lightstyle with index %d is too long.\n", index);
		return;
	}

	Uint32 insertPosition = m_lightStyles.size();
	m_lightStyles.resize(m_lightStyles.size()+1);
	lightstyle_t& style = m_lightStyles[insertPosition];

	style.map.resize(length+1);
	
	for(Uint32 i = 0; i < length; i++)
		style.map[i] = pstring[i];

	style.map[length] = '\0';

	style.length = length;
	style.framerate = framerate;
	style.interp = interpolate;
	style.index = index;

	if(!style.framerate)
		style.framerate = DEFAULT_LIGHTSTYLE_FRAMERATE;

	// Ensure array size is proper
	if(m_lightStyleValues.size() <= style.index)
		m_lightStyleValues.resize(style.index+1);
}

//====================================
//
//====================================
void CLightStyleManager::AnimateStyles( void )
{
	for(Uint32 i = 0; i < m_lightStyles.size(); i++)
	{
		lightstyle_t& style = m_lightStyles[i];
		if(!style.length)
		{
			m_lightStyleValues[style.index] = 1.0;
			continue;
		}

		if(style.interp)
		{
			const Float frame = (rns.time*style.framerate);
			const Float interp = frame - floor(frame);

			Int32 i1 = static_cast<Int32>(frame) % style.length;
			Int32 i2 = (static_cast<Int32>(frame) + 1) % style.length;

			const Int32 v1 = (style.map[i1] - 'a')*22;
			const Int32 v2 = (style.map[i2] - 'a')*22;

			m_lightStyleValues[style.index] = ((static_cast<Float>(v1))*(1.0-interp)) + ((static_cast<Float>(v2))*interp);
			m_lightStyleValues[style.index] = m_lightStyleValues[style.index] / 256.0f;
		}
		else
		{
			const Float frame = (rns.time*style.framerate);
			Int32 i1 = static_cast<Int32>(frame) % style.length;
			const Int32 v = (style.map[i1] - 'a')*22;

			m_lightStyleValues[style.index] = static_cast<Float>(v)/256.0f;
		}
	}
}

//====================================
//
//====================================
void CLightStyleManager::ApplyLightStyle( cl_dlight_t* dl, Vector& color )
{
	if(dl->lightstyle == 0)
		return;

	if(dl->lightstyle < 1 || dl->lightstyle >= m_lightStyleValues.size())
	{
		Con_Printf("Warning: Dynamic light at %.0f %.0f %.0f with invalid style index %d.\n", dl->origin.x, dl->origin.y, dl->origin.z, dl->lightstyle);
		return;
	}

	assert(dl->lightstyle < m_lightStyleValues.size());
	Math::VectorScale(color, m_lightStyleValues[dl->lightstyle], color);
}

//====================================
//
//====================================
CArray<Float>* CLightStyleManager::GetLightStyleValuesArray( void ) 
{ 
	return &m_lightStyleValues; 
}

//====================================
//
//====================================
Float CLightStyleManager::GetLightStyleValue( Uint32 styleIndex )
{
	if(styleIndex >= m_lightStyleValues.size())
		return 0;
	else
		return m_lightStyleValues[styleIndex];
}