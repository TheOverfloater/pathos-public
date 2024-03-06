/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcplat.h"
#include "plattrainentity.h"
#include "plattrigger.h"

// Default T length
const Float CFuncPlat::DEFAULT_T_LENGTH = 80;
// Default T width
const Float CFuncPlat::DEFAULT_T_WIDTH = 10;
// Default speed
const Float CFuncPlat::DEFAULT_SPEED = 150;
// Default volume
const Float CFuncPlat::DEFAULT_VOLUME = 0.85;
// Return delay time
const Float CFuncPlat::RETURN_DELAY_TIME = 3.0f;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

//=============================================
// @brief
//
//=============================================
CFuncPlat::CFuncPlat( edict_t* pedict ):
	CPlatTrainEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CFuncPlat::~CFuncPlat( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncPlat::Spawn( void )
{
	if(!CPlatTrainEntity::Spawn())
		return false;

	if(!Setup())
		return false;

	if(m_pFields->targetname != NO_STRING_VALUE)
	{
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_position1);
		m_toggleState = TS_AT_TOP;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_position2);
		m_toggleState = TS_AT_BOTTOM;
	}

	if(!IsTogglePlat())
		CPlatTrigger::SpawnPlatTrigger(this);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncPlat::Setup( void )
{
	if(!m_tLength)
		m_tLength = DEFAULT_T_LENGTH;

	if(!m_tWidth)
		m_tWidth = DEFAULT_T_WIDTH;

	m_pState->angles.Clear();
	m_pState->solid = SOLID_BSP;
	m_pState->movetype = MOVETYPE_PUSH;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_position1 = m_pState->origin;
	m_position2 = m_pState->origin;

	if(m_height)
		m_position2.z = m_pState->origin.z - m_height;
	else
		m_position2.z = m_pState->origin.z - m_pState->size.z + 8;

	if(!m_pState->speed)
		m_pState->speed = DEFAULT_SPEED;

	if(!m_volume)
		m_volume = DEFAULT_VOLUME;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::PlatUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(IsTogglePlat())
	{
		bool isOn = (m_toggleState == TS_AT_BOTTOM) ? true : false;
		if(!ShouldToggle(useMode, isOn))
			return;

		if(m_toggleState == TS_AT_TOP)
			GoDown();
		else if(m_toggleState == TS_AT_BOTTOM)
			GoUp();
	}
	else
	{
		SetUse(nullptr);

		if(m_toggleState == TS_AT_TOP)
			GoDown();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::CallBlocked( CBaseEntity* pBlocker )
{
	// Nudge the entity if it's not an npc
	if(pBlocker->IsNPC() && !pBlocker->IsPlayer())
		pBlocker->GroundEntityNudge();

	if(m_moveSoundFile)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, m_volume);

	// Check for issues
	if(m_toggleState != TS_GOING_UP && m_toggleState != TS_GOING_DOWN)
		Util::EntityConPrintf(m_pEdict, "Expected to be at going up or going down, but state is %d instead.\n", m_toggleState);

	if(m_toggleState == TS_GOING_UP)
		GoUp();
	else if(m_toggleState == TS_GOING_DOWN)
		GoDown();
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::CallGoDown( void )
{
	GoDown();
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::CallHitTop( void )
{
	HitTop();
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::CallHitBottom( void )
{
	HitBottom();
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::GoUp( void )
{
	if(m_moveSoundFile)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, m_volume);

	if(m_toggleState != TS_AT_BOTTOM && m_toggleState != TS_GOING_DOWN)
		Util::EntityConPrintf(m_pEdict, "Expected to be at bottom or going down, but state is %d instead.\n", m_toggleState);

	m_toggleState = TS_GOING_UP;
	SetMoveDone(&CFuncPlat::CallHitTop);
	LinearMove(m_position1, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::GoDown( void )
{
	if(m_moveSoundFile)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, m_volume);

	if(m_toggleState != TS_AT_TOP && m_toggleState != TS_GOING_UP)
		Util::EntityConPrintf(m_pEdict, "Expected to be at top or going up, but state is %d instead.\n", m_toggleState);

	m_toggleState = TS_GOING_DOWN;
	SetMoveDone(&CFuncPlat::CallHitBottom);
	LinearMove(m_position2, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::HitTop( void )
{
	if(m_moveSoundFile)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, 0, 0, 0, SND_FL_STOP);

	if(m_stopSoundFile)
		Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);

	if(m_toggleState != TS_GOING_UP)
		Util::EntityConPrintf(m_pEdict, "Expected to be going up, but state is %d instead.\n", m_toggleState);

	m_toggleState = TS_AT_TOP;

	if(!IsTogglePlat())
	{
		// Return after a delay
		SetThink(&CFuncPlat::CallGoDown);
		m_pState->nextthink = m_pState->ltime + RETURN_DELAY_TIME;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPlat::HitBottom( void )
{
	if(m_moveSoundFile)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, 0, 0, 0, SND_FL_STOP);

	if(m_stopSoundFile)
		Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);

	if(m_toggleState != TS_GOING_DOWN)
		Util::EntityConPrintf(m_pEdict, "Expected to be going down, but state is %d instead.\n", m_toggleState);

	m_toggleState = TS_AT_BOTTOM;
}