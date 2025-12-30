/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "vector.h"
#include "edict.h"
#include "gamedll.h"
#include "gdll_interface.h"
#include "file_interface.h"
#include "entities.h"
#include "baseentity.h"
#include "com_math.h"
#include "constants.h"
#include "cache_model.h"
#include "studio.h"
#include "vbm_shared.h"
#include "gamedll.h"
#include "gamevars.h"
#include "util.h"
#include "contents.h"
#include "snd_shared.h"
#include "trace.h"
#include "util.h"
#include "activity.h"
#include "animevent.h"
#include "usermsgs.h"
#include "skilldata.h"
#include "worldspawn.h"
#include "envmessage.h"
#include "lightstyles.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(worldspawn, CWorldSpawn);

//=============================================
// @brief
//
//=============================================
CWorldSpawn::CWorldSpawn( edict_t* pedict ):
	CBaseEntity( pedict ),
	m_skyboxName(NO_STRING_VALUE),
	m_chapterTitle(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CWorldSpawn::~CWorldSpawn()
{
}

//=============================================
// @brief
//
//=============================================
bool CWorldSpawn::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	// NOTE: Model is already set in SV_SpawnGame
	// Set origin via interface
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

	m_pState->solid = SOLID_BSP;
	m_pState->movetype = MOVETYPE_PUSH;

	if(m_skyboxName != NO_STRING_VALUE)
	{
		const Char* pstrSkyName = gd_engfuncs.pfnGetString(m_skyboxName);
		gd_engfuncs.pfnSetCVarString("sv_skyname", pstrSkyName);
	}

	if(m_chapterTitle != NO_STRING_VALUE)
		m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CWorldSpawn::Precache( void )
{
	// Call base class first
	CBaseEntity::Precache();

	// 0 normal
	gSVLightStyles.SetLightStyle("m", false, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_NORMAL);
	
	// 1 FLICKER (first variety)
	gSVLightStyles.SetLightStyle("mmnmmommommnonmmonqnmmo", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_FLICKER_A);
	
	// 2 SLOW STRONG PULSE
	gSVLightStyles.SetLightStyle("abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_SLOW_STRONG_PULSE);
	
	// 3 CANDLE (first variety)
	gSVLightStyles.SetLightStyle("mmmmmaaaaammmmmaaaaaabcdefgabcdefg", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_CANDLE_A);
	
	// 4 FAST STROBE
	gSVLightStyles.SetLightStyle("ma;ma;ma;ma;ma;ma", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_FAST_STROBE);
	
	// 5 GENTLE PULSE 1
	gSVLightStyles.SetLightStyle("jklmnopqrstuvwxyzyxwvutsrqponmlkj", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_GENTLE_PULSE);
	
	// 6 FLICKER (second variety)
	gSVLightStyles.SetLightStyle("n;monqn;mo;mn;mo;mo;mno", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_FLICKER_B);
	
	// 7 CANDLE (second variety)
	gSVLightStyles.SetLightStyle("mmmaaaabcdefgmmmmaaaammmaamm", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_CANDLE_B);
	
	// 8 CANDLE (third variety)
	gSVLightStyles.SetLightStyle("mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_CANDLE_C);
	
	// 9 SLOW STROBE (fourth variety)
	gSVLightStyles.SetLightStyle("aaaaaaaazzzzzzzz", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_SLOW_STROBE);
	
	// 10 FLUORESCENT FLICKER
	gSVLightStyles.SetLightStyle("mm;am;ammmm;amm;am;am;aaam;ammm;a", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_FLUORESCENT_FLICKER);

	// 11 SLOW PULSE NOT FADE TO BLACK
	gSVLightStyles.SetLightStyle("abcdefghijklmnopqrrqponmlkjihgfedcba", true, CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE, LS_SLOW_PULSE_NOBLACK);
}

//=============================================
// @brief
//
//=============================================
void CWorldSpawn::InitEntity( void )
{
	CEnvMessage* pMessage = reinterpret_cast<CEnvMessage*>(CBaseEntity::CreateEntity("env_message", nullptr));
	if(pMessage)
	{
		const Char* pstrMessage = gd_engfuncs.pfnGetString(m_chapterTitle);
		pMessage->SetMessage(pstrMessage);

		if(!pMessage->Spawn())
		{
			Util::RemoveEntity(pMessage);
			return;
		}

		pMessage->SetThink(&CBaseEntity::CallUseToggleThink);
		pMessage->SetNextThink(0.3);
		pMessage->SetSpawnFlag(CEnvMessage::FL_ONLY_ONCE);
	}
}

//=============================================
// @brief
//
//=============================================
bool CWorldSpawn::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "skyname"))
	{
		if(qstrlen(kv.value) > 0)
			m_skyboxName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "chaptertitle"))
	{
		m_chapterTitle = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "sounds")
		|| !qstrcmp(kv.keyname, "WaveHeight")
		|| !qstrcmp(kv.keyname, "MaxRange")
		|| !qstrcmp(kv.keyname, "chaptertitle")
		|| !qstrcmp(kv.keyname, "startdark")
		|| !qstrcmp(kv.keyname, "newunit")
		|| !qstrcmp(kv.keyname, "gametitle")
		|| !qstrcmp(kv.keyname, "mapteams")
		|| !qstrcmp(kv.keyname, "defaultteam")
		|| !qstrcmp(kv.keyname, "overdarken")
		|| !qstrcmp(kv.keyname, "specialfog"))
	{
		// Shut engine up about unhandled keyvalues
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CWorldSpawn::Restore( void )
{
	// Set origin via interface
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
	return true;
}