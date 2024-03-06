/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "plattrainentity.h"

// Number of legacy move sounds
const Uint32 CPlatTrainEntity::NB_LEGACY_MOVE_SOUNDS = 15;
// Legacy move sounds
const Char* CPlatTrainEntity::LEGACY_MOVE_SOUNDS[NB_LEGACY_MOVE_SOUNDS] =
{
	"plats/bigmove1.wav",
	"plats/bigmove2.wav",
	"plats/elevmove1.wav",
	"plats/elevmove2.wav",
	"plats/elevmove3.wav",
	"plats/freightmove1.wav",
	"plats/freightmove2.wav",
	"plats/heavymove1.wav",
	"plats/rackmove1.wav",
	"plats/railmove1.wav",
	"plats/squeekmove1.wav",
	"plats/talkmove1.wav",
	"plats/talkmove2.wav",
	"plats/platmove1.wav",
	"plats/subwayloop.wav"
};

// Number of legacy stop sounds
const Uint32 CPlatTrainEntity::NB_LEGACY_STOP_SOUNDS = 9;
// Legacy stop sounds
const Char* CPlatTrainEntity::LEGACY_STOP_SOUNDS[NB_LEGACY_STOP_SOUNDS] = 
{
	"plats/bigstop1.wav",
	"plats/bigstop2.wav",
	"plats/freightstop1.wav",
	"plats/heavystop2.wav",
	"plats/rackstop1.wav",
	"plats/railstop1.wav",
	"plats/squeekstop1.wav",
	"plats/talkstop1.wav",
	"plats/platstop1.wav"
};

//=============================================
// @brief
//
//=============================================
CPlatTrainEntity::CPlatTrainEntity( edict_t* pedict ):
	CToggleEntity(pedict),
	m_moveSound(0),
	m_stopSound(0),
	m_volume(0)
{
}

//=============================================
// @brief
//
//=============================================
CPlatTrainEntity::~CPlatTrainEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlatTrainEntity::DeclareSaveFields( void )
{
	// Call base class to do it first
	CToggleEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CPlatTrainEntity, m_volume, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CPlatTrainEntity::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "height"))
	{
		m_height = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "rotation"))
	{
		m_finalAngle.x = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "movesnd"))
	{
		m_moveSound = SDL_atoi(kv.value);
		if(m_moveSound < 0 || m_moveSound > NB_LEGACY_MOVE_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid value %d set for '%s'.\n", m_moveSound, kv.keyname);
			m_moveSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "movesnd_custom"))
	{
		m_moveSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "stopsnd"))
	{
		m_stopSound = SDL_atoi(kv.value);
		if(m_stopSound < 0 || m_stopSound > NB_LEGACY_STOP_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid value %d set for '%s'.\n", m_stopSound, kv.keyname);
			m_stopSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "stopsnd_custom"))
	{
		m_stopSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "volume"))
	{
		m_volume = SDL_atof(kv.value);
		return true;
	}
	else
		return CToggleEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CPlatTrainEntity::Spawn( void )
{
	if(m_moveSoundFile == NO_STRING_VALUE && m_moveSound > 0)
	{
		const Char* pstrSoundFile = LEGACY_MOVE_SOUNDS[m_moveSound - 1];
		m_moveSoundFile = gd_engfuncs.pfnAllocString(pstrSoundFile);
	}

	if(m_stopSoundFile == NO_STRING_VALUE && m_stopSound > 0)
	{
		const Char* pstrSoundFile = LEGACY_STOP_SOUNDS[m_stopSound - 1];
		m_stopSoundFile = gd_engfuncs.pfnAllocString(pstrSoundFile);
	}

	if(!CToggleEntity::Spawn())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlatTrainEntity::Precache( void )
{
	if(m_moveSoundFile != NO_STRING_VALUE)
	{
		const Char* pstrSoundFile = gd_engfuncs.pfnGetString(m_moveSoundFile);
		gd_engfuncs.pfnPrecacheSound(pstrSoundFile);
	}

	if(m_stopSoundFile != NO_STRING_VALUE)
	{
		const Char* pstrSoundFile = gd_engfuncs.pfnGetString(m_stopSoundFile);
		gd_engfuncs.pfnPrecacheSound(pstrSoundFile);
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlatTrainEntity::IsTogglePlat( void )
{
	if(HasSpawnFlag(FL_PLAT_TOGGLE))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CPlatTrainEntity::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_pState->velocity.IsZero() && m_moveSoundFile != NO_STRING_VALUE)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, m_volume, ATTN_NORM, PITCH_NORM, SND_FL_NONE, pPlayer);
}