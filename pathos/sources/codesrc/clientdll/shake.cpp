/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// includes
#include "includes.h"
#include "shake.h"
#include "clientdll.h"
#include "com_math.h"

// Class definition
CScreenShake gShake;

void Cmd_ScreenChake( void )
{
	if(cl_engfuncs.pfnCmd_Argc() < 4)
	{
		cl_engfuncs.pfnCon_Printf("screenshake usage: applyshake <amplitude> <frequency> <duration>.\n");
		return;
	}

	Float amplitude = SDL_atof(cl_engfuncs.pfnCmd_Argv(1));
	Float frequency = SDL_atof(cl_engfuncs.pfnCmd_Argv(2));
	Float duration = SDL_atof(cl_engfuncs.pfnCmd_Argv(3));

	gShake.AddScreenShake(amplitude, duration, frequency);
}

//====================================
//
//====================================
CScreenShake::CScreenShake ( void )
	: m_shakeAngle( 0 )
{
}

//====================================
//
//====================================
CScreenShake::~CScreenShake ( void )
{
}

//====================================
//
//====================================
bool CScreenShake::Init ( void )
{
	cl_engfuncs.pfnCreateCommand("screenshake", Cmd_ScreenChake, nullptr);
	return true;
}

//====================================
//
//====================================
void CScreenShake::ClearGame ( void )
{
	m_shakesList.clear();
}

//====================================
//
//====================================
void CScreenShake::CalcShake ( void )
{
	// Reset shake offset
	m_shakeOffset.Clear();
	// Clear angle
	m_shakeAngle = 0;

	Double time = cl_engfuncs.pfnGetClientTime();
	Double frametime = cl_engfuncs.pfnGetFrameTime();

	m_shakesList.begin();
	while(!m_shakesList.end())
	{
		screenshake_t& shake = m_shakesList.get();
		if (!shake.endtime)
		{
			m_shakesList.remove(m_shakesList.get_link());
			m_shakesList.next();
			continue;
		}

		// Try and retire the shake
		if ((time > shake.endtime) || shake.duration <= 0 || shake.amplitude <= 0 || shake.frequency <= 0)
		{
			m_shakesList.remove(m_shakesList.get_link());
			m_shakesList.next();
			continue;
		}

		if (time > shake.nextshake)
		{
			shake.shakebegin = time;
			shake.nextshake = time + (1.0f / shake.frequency);
			for (Uint32 i = 0; i < 3; i++)
				shake.offset[i] = Common::RandomFloat( -shake.amplitude, shake.amplitude );

			shake.angle = Common::RandomFloat( -shake.amplitude, shake.amplitude );
		}

		Double fraction = (shake.endtime - time) / shake.duration;
		Double freq = fraction ? (shake.frequency / fraction) : 0;
		fraction = (fraction * fraction) * SDL_sin(time * freq);

		// Reduce fade time by closeness to default refresh rate
		Float fadeModulator = (60 - _max(60, shake.frequency)) / 60;
		
		Double fadetime = 0;
		if(fadeModulator > 0)
		{
			Double shaketime = shake.nextshake - shake.shakebegin;
			fadetime = shaketime * (0.25 * fadeModulator);
			if(fadetime > 0)
			{
				Float startfade = clamp((time - shake.shakebegin)/fadetime, 0.0, 1.0);
				fraction *= startfade;
			}
		}

		if(fadeModulator > 0 && fadetime > 0)
		{
			Double fadebegintime = shake.nextshake-fadetime;
			Float endfade = (1.0 - clamp((time-fadebegintime)/fadetime, 0.0, 1.0));
			fraction *= endfade;
		}

		// Calculate values
		Math::VectorMA(m_shakeOffset, fraction, shake.offset, m_shakeOffset); 
		m_shakeAngle += shake.angle * fraction;
		
		// Calculate amplitude
		shake.amplitude -= shake.amplitude * ( frametime / (shake.duration * shake.frequency) );

		// Go to next
		m_shakesList.next();
	}
}

//====================================
//
//====================================
void CScreenShake::ApplyShake( Vector& origin, Vector& angles, Float factor )
{
	// Apply the offset
	Math::VectorMA( origin, factor * 0.5, m_shakeOffset, origin );

	// Apply the angle
	angles[ROLL] += m_shakeAngle * factor;

	// Apply the angle
	angles[1] += m_shakeAngle * factor * 0.25;
}

//====================================
//
//====================================
screenshake_t* CScreenShake::AllocShake ( void )
{
	return &m_shakesList.add(screenshake_t())->_val;
}

//====================================
//
//====================================
void CScreenShake::AddScreenShake( Float amplitude, Float duration, Float frequency )
{
	screenshake_t* pshake = AllocShake();
	if(!pshake)
		return;

	pshake->amplitude = amplitude;
	pshake->frequency = frequency;
	pshake->duration = duration;
	pshake->endtime = cl_engfuncs.pfnGetClientTime() + pshake->duration;
}
