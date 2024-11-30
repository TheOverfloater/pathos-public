/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envdlight.h"
#include "lightstyles.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_dlight, CEnvDLight);

//=============================================
// @brief
//
//=============================================
CEnvDLight::CEnvDLight( edict_t* pedict ):
	CPointEntity(pedict),
	m_pattern(NO_STRING_VALUE),
	m_style(0),
	m_interpolate(0),
	m_framerate(0),
	m_oscillationh(0),
	m_oscillationv(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvDLight::~CEnvDLight( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_pattern, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_style, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_framerate, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_interpolate, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_oscillationh, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_oscillationv, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvDLight, m_baseOrigin, EFIELD_COORD));
}

//=============================================
// @brief
//
//=============================================
bool CEnvDLight::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "oscillationv"))
	{
		m_oscillationv = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "oscillationh"))
	{
		m_oscillationh = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "lightstyle"))
	{
		m_style = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "interp"))
	{
		m_interpolate = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "pattern"))
	{
		m_pattern = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "framerate"))
	{
		m_framerate = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvDLight::Restore( void )
{
	if(!CPointEntity::Restore())
		return false;

	if(m_pattern != NO_STRING_VALUE)
	{
		// Manage the lightstyle
		ManageLightStyle();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvDLight::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// Set empty sprite texture
	if(!SetModel(NULL_SPRITE_FILENAME, false))
		return false;

	if(HasSpawnFlag(FL_START_ON) || m_pFields->targetname == NO_STRING_VALUE)
		m_pState->effects &= ~EF_NODRAW;
	else
		m_pState->effects |= EF_NODRAW;

	// Mark with the proper renderfx
	SetLightRenderFx();

	// Set mins/maxs
	SetMinsMaxs();

	if(m_pattern != NO_STRING_VALUE)
	{
		// Manage the lightstyle
		ManageLightStyle();
	}

	// Use frame to store lightstyle
	m_pState->frame = m_style;
	// Set base origin
	m_baseOrigin = m_pState->origin;

	// Manage oscillation
	if(m_oscillationh || m_oscillationv)
	{
		m_pState->flags |= FL_ALWAYSTHINK;
		m_pState->nextthink = m_pState->ltime + 0.1;
		SetThink(&CEnvDLight::OscillateThink);

		// Disable static flag
		if(m_pState->skin)
			m_pState->skin = 0;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::SetLightRenderFx( void )
{
	m_pState->rendertype = RT_ENVDLIGHT;
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::SetMinsMaxs( void )
{
	// Set mins/maxs
	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = -m_pState->renderamt;
		maxs[i] = m_pState->renderamt;
	}

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	switch(useMode)
	{
	case USE_OFF:
		m_pState->effects |= EF_NODRAW;
		break;
	case USE_ON:
		m_pState->effects &= ~EF_NODRAW;
		break;
	case USE_TOGGLE:
	default:
		if(m_pState->effects & EF_NODRAW)
			m_pState->effects &= ~EF_NODRAW;
		else
			m_pState->effects |= EF_NODRAW;
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::SetValues( const Vector& origin, const Vector& color, Uint32 radius )
{
	m_pState->origin = origin;
	m_pState->rendercolor = color;
	m_pState->renderamt = radius;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::ManageLightStyle( void )
{
	if(m_pattern == NO_STRING_VALUE)
		return;

	if(!m_framerate)
		m_framerate = CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE;

	// Get the pattern string
	const Char* pstrpattern = gd_engfuncs.pfnGetString(m_pattern);
	if(!pstrpattern)
		return;

	// Add the new lightstyle
	m_style = gSVLightStyles.AddCustomLightStyle(pstrpattern, m_interpolate, m_framerate);
}

//=============================================
// @brief
//
//=============================================
void CEnvDLight::OscillateThink( void )
{
	Vector newOrigin;
	Float oscillatevar = SDL_sin(g_pGameVars->time*2) - SDL_sin(g_pGameVars->time*5);
	newOrigin.x = m_baseOrigin.x + SDL_sin(g_pGameVars->time)*m_oscillationh*oscillatevar;
	newOrigin.y = m_baseOrigin.y + SDL_sin(g_pGameVars->time)*m_oscillationh*(1.0-oscillatevar);
	newOrigin.z = m_baseOrigin.z + SDL_sin(g_pGameVars->time)*m_oscillationv;

	trace_t tr;
	Util::TraceLine(m_pState->origin, newOrigin, true, false, nullptr, tr);
	if(tr.noHit() || tr.allSolid() || tr.startSolid())
		gd_engfuncs.pfnSetOrigin(m_pEdict, newOrigin);
	else
		gd_engfuncs.pfnSetOrigin(m_pEdict, tr.endpos);

	
	m_pState->nextthink = m_pState->ltime + 0.1;
}

//=============================================
// @brief
//
//=============================================
bool CEnvDLight::ShouldOverrideKeyValue( const Char* pstrKeyValue )
{
	// framerate is handled by this entity specially
	if(!qstrcmp(pstrKeyValue, "framerate"))
		return true;
	else
		return false;
}


//=============================================
// @brief
//
//=============================================
CEnvDLight* CEnvDLight::SpawnLight( const Vector& origin, const Vector& color, Uint32 radius )
{
	edict_t* pedict = gd_engfuncs.pfnCreateEntity("env_dlight");
	if(!pedict)
	{
		gd_engfuncs.pfnCon_Printf("Failed to create env_dlight entity.\n");
		return nullptr;
	}

	CEnvDLight* pdlight = reinterpret_cast<CEnvDLight*>(CBaseEntity::GetClass(pedict));
	if(!pdlight)
	{
		gd_engfuncs.pfnCon_Printf("Failed to create env_dlight entity.\n");
		return nullptr;
	}

	// Set values for light
	pdlight->SetValues(origin, color, radius);

	// Spawn the entity
	DispatchSpawn(pedict);
	return pdlight;
}