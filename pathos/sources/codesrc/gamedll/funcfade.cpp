/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcfade.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_fade, CFuncFade);

//=============================================
// @brief
//
//=============================================
CFuncFade::CFuncFade( edict_t* pedict ):
	CFuncWall(pedict),
	m_fadeDuration(0),
	m_targetAlpha(0),
	m_fadeBeginTime(0),
	m_startAlpha(0)
{
}

//=============================================
// @brief
//
//=============================================
CFuncFade::~CFuncFade( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncFade::Spawn( void )
{
	if(!CFuncWall::Spawn())
		return false;

	m_pState->rendermode = RENDER_TRANSTEXTURE_LIT;
	m_startAlpha = m_pState->renderamt;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncFade::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CFuncFade, m_fadeDuration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncFade, m_targetAlpha, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncFade, m_fadeBeginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncFade, m_startAlpha, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
void CFuncFade::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Set to always think
	m_pState->flags |= FL_ALWAYSTHINK;
	m_pState->nextthink = m_pState->ltime + 0.1;
	SetThink(&CFuncFade::Think);

	m_fadeBeginTime = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CFuncFade::Think( void )
{
	Double delta = (g_pGameVars->time - m_fadeBeginTime)/m_fadeDuration;
	if(delta >= 1.0)
	{
		m_pState->renderamt = m_targetAlpha;
		if(m_pState->renderamt == 255)
			m_pState->rendermode = RENDER_NORMAL;

		SetThink(nullptr);
		m_pState->nextthink = 0;
	}
	else
	{
		m_pState->renderamt = m_targetAlpha*delta + m_startAlpha*(1.0 - delta);
		m_pState->nextthink = m_pState->ltime + 0.1;
	}
}

//=============================================
// @brief
//
//=============================================
bool CFuncFade::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "fadetime"))
	{
		m_fadeDuration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "targetalpha"))
	{
		m_targetAlpha = SDL_atof(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}