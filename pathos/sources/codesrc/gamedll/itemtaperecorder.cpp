/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemtaperecorder.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_taperecorder, CItemTapeRecorder);

// Recorder model for benefactor
const Char CItemTapeRecorder::RECORDER_MODEL[] = "models/taperecorder.mdl";
// Recorder idle sequence name
const Char CItemTapeRecorder::RECORDER_IDLE_SEQ_NAME[] = "idle";
// Recorder play sequence name
const Char CItemTapeRecorder::RECORDER_PLAY_SEQ_NAME[] = "play";
// Recorder use sound name
const Char CItemTapeRecorder::RECORDER_USE_SOUND_NAME[] = "items/recorder_use.wav";
// Recorder stop sound name
const Char CItemTapeRecorder::RECORDER_STOP_SOUND_NAME[] = "items/recorder_stop.wav";
// Recorder play sound name
const Char CItemTapeRecorder::RECORDER_PLAY_SOUND_NAME[] = "items/recorder_play.wav";

//=============================================
// @brief
//
//=============================================
CItemTapeRecorder::CItemTapeRecorder( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_playbackTitle(NO_STRING_VALUE),
	m_soundFileName(NO_STRING_VALUE),
	m_duration(0),
	m_playbackBeginTime(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CItemTapeRecorder::~CItemTapeRecorder( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemTapeRecorder::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CItemTapeRecorder, m_playbackTitle, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemTapeRecorder, m_soundFileName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemTapeRecorder, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemTapeRecorder, m_playbackBeginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemTapeRecorder, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CItemTapeRecorder::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "soundfile"))
	{
		m_soundFileName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "title"))
	{
		m_playbackTitle = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CAnimatingEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CItemTapeRecorder::Precache( void )
{
	CAnimatingEntity::Precache();

	gd_engfuncs.pfnPrecacheModel(RECORDER_MODEL);
	gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_soundFileName));
	gd_engfuncs.pfnPrecacheSound(RECORDER_USE_SOUND_NAME);
	gd_engfuncs.pfnPrecacheSound(RECORDER_PLAY_SOUND_NAME);
	gd_engfuncs.pfnPrecacheSound(RECORDER_STOP_SOUND_NAME);
}

//=============================================
// @brief
//
//=============================================
bool CItemTapeRecorder::Restore( void )
{
	if(!CAnimatingEntity::Restore())
		return false;

	if(g_saveRestoreData.transitionload)
	{
		m_playbackBeginTime = 0;
		m_isActive = false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CItemTapeRecorder::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_isActive)
		return;

	Float timeOffset = g_pGameVars->time - m_playbackBeginTime;
	if(timeOffset >= m_duration)
	{
		m_playbackBeginTime = 0;
		m_isActive = false;
		return;
	}

	Util::EmitEntitySound(this, m_soundFileName, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, pPlayer, timeOffset);
	Util::EmitEntitySound(this, RECORDER_PLAY_SOUND_NAME, SND_CHAN_ITEM, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, pPlayer);
}

//=============================================
// @brief
//
//=============================================
bool CItemTapeRecorder::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;
	
	if(m_soundFileName == NO_STRING_VALUE || m_duration <= 0)
	{
		Util::RemoveEntity(this);
		return true;
	}

	m_pState->solid = SOLID_TRIGGER;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->renderfx = RenderFx_GlowAura;

	if(HasSpawnFlag(SF_START_INVISIBLE))
		m_pState->effects |= EF_NODRAW;

	if(!SetModel(RECORDER_MODEL, false))
		return false;

	// Set sequence bbox
	SetSequenceBox(false);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CItemTapeRecorder::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(HasSpawnFlag(SF_START_INVISIBLE))
	{
		if(useMode != USE_SET)
		{
			m_pState->effects &= ~EF_NODRAW;
			m_pState->spawnflags &= ~SF_START_INVISIBLE;
		}

		return;
	}

	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	if(m_isActive)
	{
		m_pState->sequence = FindSequence(RECORDER_IDLE_SEQ_NAME);
		if(m_pState->sequence == NO_SEQUENCE_VALUE)
			m_pState->sequence = 0;
		else
			ResetSequenceInfo();

		Util::EmitEntitySound(this, RECORDER_STOP_SOUND_NAME, SND_CHAN_BODY);
		Util::EmitEntitySound(this, RECORDER_PLAY_SOUND_NAME, SND_CHAN_ITEM, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_STOP);
		Util::EmitEntitySound(this, m_soundFileName, SND_CHAN_STATIC, 0, 0, 0, SND_FL_STOP, pEntity);

		m_isActive = false;
		m_playbackBeginTime = 0;

		pEntity->StopTapeTrack(gd_engfuncs.pfnGetString(m_soundFileName));

		SetThink(nullptr);
		m_pState->nextthink = 0;
	}
	else
	{
		m_pState->sequence = FindSequence(RECORDER_PLAY_SEQ_NAME);
		if(m_pState->sequence == NO_SEQUENCE_VALUE)
			m_pState->sequence = 0;
		else
			ResetSequenceInfo();

		Util::EmitEntitySound(this, m_soundFileName, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, pEntity);
		Util::EmitEntitySound(this, RECORDER_USE_SOUND_NAME, SND_CHAN_BODY);
		Util::EmitEntitySound(this, RECORDER_PLAY_SOUND_NAME, SND_CHAN_ITEM);

		SetThink(&CItemTapeRecorder::Reset);
		m_pState->nextthink = g_pGameVars->time + m_duration;
		m_playbackBeginTime = g_pGameVars->time;

		m_isActive = true;

		pEntity->PlaybackTapeTrack(gd_engfuncs.pfnGetString(m_soundFileName), m_duration, gd_engfuncs.pfnGetString(m_playbackTitle), m_pState->rendercolor, m_pState->renderamt);
	}
}

//=============================================
// @brief
//
//=============================================
void CItemTapeRecorder::Reset( void )
{
	m_pState->sequence = FindSequence(RECORDER_IDLE_SEQ_NAME);
	if(m_pState->sequence == NO_SEQUENCE_VALUE)
		m_pState->sequence = 0;
	else
		ResetSequenceInfo();

	m_isActive = false;
	m_playbackBeginTime = 0;

	Util::EmitEntitySound(this, RECORDER_STOP_SOUND_NAME, SND_CHAN_BODY);
	Util::EmitEntitySound(this, RECORDER_PLAY_SOUND_NAME, SND_CHAN_ITEM, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_STOP);
}

//=============================================
// @brief
//
//=============================================
usableobject_type_t CItemTapeRecorder::GetUsableObjectType( void )
{
	return !(m_pState->spawnflags & SF_START_INVISIBLE) ? USABLE_OBJECT_DEFAULT : USABLE_OBJECT_NONE;
}