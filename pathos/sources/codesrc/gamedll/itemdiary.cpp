/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemdiary.h"
#include "triggercameramodel.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_diary, CItemDiary);

// Blend time for diary
const Float CItemDiary::DIARY_BLEND_TIME = 1.0;
// Diary model for benefactor
const Char CItemDiary::DIARY_MODEL_BENEFACTOR[] = "models/props/diary_benefactor.mdl";
// Diary model for benefactor
const Char CItemDiary::DIARY_MODEL_RADFORD[] = "models/props/diary_emanations.mdl";
// Diary entry sequence name
const Char CItemDiary::DIARY_ENTRY_SEQ_NAME[] = "book_enter";
// Diary loop sequence name
const Char CItemDiary::DIARY_LOOP_SEQ_NAME[] = "book_loop";
// Diary exit sequence name
const Char CItemDiary::DIARY_EXIT_SEQ_NAME[] = "book_exit";
// Diary rest sequence name
const Char CItemDiary::DIARY_REST_SEQ_NAME[] = "idle";

//=============================================
// @brief
//
//=============================================
CItemDiary::CItemDiary( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_duration(0),
	m_diaryType(DIARY_BENEFACTOR),
	m_state(DIARY_OFF),
	m_isDisabled(false),
	m_soundFile(NO_STRING_VALUE),
	m_playerSound(NO_STRING_VALUE),
	m_entryTrigger(NO_STRING_VALUE),
	m_pCameraModel(nullptr),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CItemDiary::~CItemDiary( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_diaryType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_state, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_isDisabled, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_soundFile, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_playerSound, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_entryTrigger, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_pCameraModel, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemDiary, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CItemDiary::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "soundfile"))
	{
		m_soundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "entrytarget"))
	{
		m_entryTrigger = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "playersound"))
	{
		m_playerSound = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "type"))
	{
		m_diaryType = SDL_atoi(kv.value);
		return true;
	}
	else
		return CAnimatingEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::Precache( void )
{
	CAnimatingEntity::Precache();

	gd_engfuncs.pfnPrecacheModel(DIARY_MODEL_BENEFACTOR);
	gd_engfuncs.pfnPrecacheModel(DIARY_MODEL_RADFORD);
	gd_engfuncs.pfnPrecacheSound("items/diary_noise.wav");

	if(m_soundFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_soundFile));

	if(m_playerSound != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_playerSound));
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(m_state == DIARY_PLAYBACK && m_pPlayer && pPlayer == m_pPlayer)
		Util::EmitEntitySound(m_pPlayer, m_soundFile, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_DIALOUGE, pPlayer);
	
	if(!m_isDisabled)
		Util::EmitEntitySound(this, "items/diary_noise.wav", SND_CHAN_STATIC, (m_state == DIARY_PLAYBACK) ? 0.4 : VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, pPlayer);
}

//=============================================
// @brief
//
//=============================================
bool CItemDiary::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	m_pState->solid = SOLID_TRIGGER;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->renderfx = RenderFx_Diary;

	bool result = true;
	switch(m_diaryType)
	{
	case DIARY_BENEFACTOR:
		result = SetModel(DIARY_MODEL_BENEFACTOR, false);
		break;
	case DIARY_RADFORD:
		result = SetModel(DIARY_MODEL_RADFORD, false);
		break;
	default:
		Util::EntityConPrintf(m_pEdict, "Invalid diary type specified.\n");
		return false;
	}

	if(!result)
		return false;

	// Set sequence bbox
	SetSequenceBox(false);

	// If we're set to be disabled
	if(HasSpawnFlag(FL_START_DISABLED))
	{
		m_pState->effects |= EF_NODRAW;
		m_isDisabled = true;
	}

	m_pCameraModel = CTriggerCameraModel::CreateCameraModel(this, DIARY_BLEND_TIME, DIARY_REST_SEQ_NAME, DIARY_ENTRY_SEQ_NAME, DIARY_LOOP_SEQ_NAME, DIARY_EXIT_SEQ_NAME);
	if(!m_pCameraModel)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't create cameramodel entity.\n");
		return false;
	}

	switch(m_diaryType)
	{
	case DIARY_BENEFACTOR:
		m_pCameraModel->SetBodyGroup(1, 2);
		break;
	case DIARY_RADFORD:
		m_pCameraModel->SetBodyGroup(1, 3);
		break;
	}

	if(!m_pCameraModel->Spawn())
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_isDisabled)
	{
		if(pCaller->IsPlayer())
			return;

		Util::EmitEntitySound(this, "items/diary_noise.wav", SND_CHAN_STATIC);
		m_pState->effects &= ~EF_NODRAW;
		m_isDisabled = false;
		return;
	}

	if(!pCaller->IsPlayer())
		return;

	m_pCameraModel->CallUse(this, this, USE_ON, 0);

	if(HasSpawnFlag(FL_STAY_DISABLED))
		m_pState->solid = SOLID_NOT;

	SetThink(&CItemDiary::Enter);
	m_pState->nextthink = g_pGameVars->time + 1.0;

	// Set player ptr
	m_pPlayer = pCaller;
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::Enter( void )
{
	m_pCameraModel->CallUse(this, this, USE_ON, 0);
	m_state = DIARY_ENTER;
	m_pState->effects |= EF_NODRAW;

	Util::EmitEntitySound(this, "items/diary_noise.wav", SND_CHAN_STATIC, 0.4, ATTN_NORM, PITCH_NORM, SND_FL_CHANGE_VOLUME);

	SetThink(nullptr);
	m_pState->nextthink = 0;
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::AdvanceState( void )
{
	if(m_state == DIARY_ENTER)
	{
		// So player can cancel
		m_pPlayer->BeginDiaryPlayback(gd_engfuncs.pfnGetString(m_soundFile), m_duration, this);
		// Playback the sound for the player
		Util::EmitEntitySound(m_pPlayer, m_soundFile, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_DIALOUGE);

		SetThink(&CItemDiary::Exit);
		m_pState->nextthink = g_pGameVars->time + m_duration+0.5;
		m_state = DIARY_PLAYBACK;

		if(m_entryTrigger != NO_STRING_VALUE)
		{
			Util::FireTargets(gd_engfuncs.pfnGetString(m_entryTrigger), this, this, USE_TOGGLE, 0);
			m_entryTrigger = NO_STRING_VALUE;
		}
	}
	else if(m_state == DIARY_LEAVE)
	{
		Util::EmitEntitySound(this, "items/diary_noise.wav", SND_CHAN_STATIC, VOL_NORM, ATTN_IDLE, PITCH_NORM, SND_FL_CHANGE_VOLUME);

		m_pState->effects &= ~EF_NODRAW;
		m_state = DIARY_OFF;

		if(m_pFields->target != NO_STRING_VALUE)
		{
			Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->target), this, this, USE_TOGGLE, 0);
			m_pFields->target = NO_STRING_VALUE;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::Exit( void )
{
	if(m_playerSound != NO_STRING_VALUE)
	{
		Util::EmitEntitySound(m_pPlayer, gd_engfuncs.pfnGetString(m_playerSound), SND_CHAN_STATIC);
		m_playerSound = NO_STRING_VALUE;
	}

	// Tell it to leave
	m_pCameraModel->CallUse(this, this, USE_ON, 0);

	SetThink(nullptr);
	m_pState->nextthink = 0;
	m_state = DIARY_LEAVE;

	// Remove no interrupt spawnflag if set
	if(HasSpawnFlag(FL_CANNOT_INTERRUPT))
		m_pState->spawnflags &= ~FL_CANNOT_INTERRUPT;

	m_pPlayer = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CItemDiary::StopDiaryPlayback( void )
{
	Exit();
}

//=============================================
// @brief
//
//=============================================
usableobject_type_t CItemDiary::GetUsableObjectType( void )
{
	return m_isDisabled ? USABLE_OBJECT_NONE : USABLE_OBJECT_DEFAULT;
}