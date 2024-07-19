/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "file.h"
#include "cvar.h"
#include "system.h"
#include "brushmodel.h"

#include "cl_snd.h"
#include "console.h"
#include "snd_shared.h"
#include "cl_entity.h"
#include "com_math.h"
#include "cl_utils.h"
#include "cl_main.h"
#include "enginestate.h"
#include "modelcache.h"
#include "brushmodel.h"
#include "cl_pmove.h"
#include "networking.h"
#include "enginestate.h"
#include "cache_model.h"
#include "commands.h"
#include "config.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_basicdraw.h"
#include "r_vbm.h"
#include "r_main.h"
#include "window.h"

#ifdef _64BUILD
// OpenAL library path
static const Char OPENAL_LIBRARY_PATH[] = "x64/OpenAL32.dll";
#else
// OpenAL library path
static const Char OPENAL_LIBRARY_PATH[] = "x86/OpenAL32.dll";
#endif

// Max active tempent sounds
const Uint32 CSoundEngine::MAX_ACTIVE_TEMP_SOUNDS = 4;

// Minimum sound distance
const Float CSoundEngine::MIN_DISTANCE = 32;
// Maximum sound distance
const Float CSoundEngine::MAX_DISTANCE = 1024;

// Rever blend time
const Float CSoundEngine::REVERB_BLEND_TIME = 2;
// Sound rolloff factor
const Float CSoundEngine::SE_ROLLOFF_FACTOR = 1;
// Buffer size
const Uint32 CSoundEngine::BUFFER_SIZE = 4096*32;
// Speed of sound ingame
const Float CSoundEngine::SPEED_OF_SOUND = 17847.76902887139;

// Cached message max lifetime
const Float CSoundEngine::CACHED_MSG_DELETE_TIME = 60;

// Average nb of samples for 8-bit sounds
const Uint32 CSoundEngine::AVERAGE_SAMPLES_8BIT = 768;
// Average nb of samples for 16-bit sounds
const Uint32 CSoundEngine::AVERAGE_SAMPLES_16BIT = 2048;

// Default change time for SND_CHANGE_PITCH/SND_CHANGE_VOL flags
const Float CSoundEngine::DEFAULT_SND_CHANGE_TIME = 0.2;

// On-demand sound file size limit
const Uint32 CSoundEngine::ONDEMAND_SOUND_SIZE_LIMIT = 262144;

// Config group for sound engine
const Char CSoundEngine::SOUNDENGINE_CONFIG_GRP_NAME[] = "SoundSystem";
// Config group for sound engine
const Char CSoundEngine::SOUNDENGINE_HRTF_SETTING_NAME[] = "HRTF";

//
// EAX effects used by the engine
//
const EFXEAXREVERBPROPERTIES CSoundEngine::g_pEAXEffects[CSoundEngine::NUM_REVERBS] = 
{
	EFX_REVERB_PRESET_ROOM, EFX_REVERB_PRESET_ROOM, 
	EFX_REVERB_PRESET_SEWERPIPE, EFX_REVERB_PRESET_SEWERPIPE, EFX_REVERB_PRESET_SEWERPIPE,
	EFX_REVERB_PRESET_HANGAR, EFX_REVERB_PRESET_HANGAR, EFX_REVERB_PRESET_HANGAR,
	EFX_REVERB_PRESET_STONECORRIDOR, EFX_REVERB_PRESET_STONECORRIDOR, EFX_REVERB_PRESET_STONECORRIDOR,
	EFX_REVERB_PRESET_STONECORRIDOR, EFX_REVERB_PRESET_STONECORRIDOR, EFX_REVERB_PRESET_STONECORRIDOR,
	EFX_REVERB_PRESET_UNDERWATER, EFX_REVERB_PRESET_UNDERWATER, EFX_REVERB_PRESET_UNDERWATER,
	EFX_REVERB_PRESET_STONECORRIDOR, EFX_REVERB_PRESET_STONECORRIDOR, EFX_REVERB_PRESET_STONECORRIDOR,
	EFX_REVERB_PRESET_OUTDOORS_VALLEY, EFX_REVERB_PRESET_OUTDOORS_VALLEY, EFX_REVERB_PRESET_OUTDOORS_VALLEY,
	EFX_REVERB_PRESET_ARENA, EFX_REVERB_PRESET_ARENA, EFX_REVERB_PRESET_ARENA,
	EFX_REVERB_PRESET_SEWERPIPE, EFX_REVERB_PRESET_SEWERPIPE, EFX_REVERB_PRESET_SEWERPIPE
};

//
// Multipliers for EAX effects
//
const Float CSoundEngine::g_pEAXMultipliers[CSoundEngine::NUM_REVERBS] =
{
	1.0, /* Normal */
	1.0, /* Generic */
	0.66 /* Metal small */, 0.85 /* Metal medium */, 1.0 /* Metal large */,
	0.33 /* Tunnel small */, 0.66 /* Tunnel medium */, 1.0 /* Tunnel large */,
	0.33 /* Chamber small */, 0.66 /* Chamber medium */, 1.0 /* Chamber large */,
	0.33 /* Bright small */, 0.66 /* Bright medium */, 1.0 /* Bright large */,
	1.0 /* Underwater 1 */, 1.0 /* Bright medium */, 1.0 /* Bright large */,
	0.66 /* Concrete small */, 0.85 /* Concrete medium */, 1.0 /* Concrete large */,
	0.33 /* Big 1 */, 0.66 /* Big 2 */, 1.0 /* Big 3 */,
	0.33 /* Cavern small */, 0.66 /* Cavern medium */, 1.0 /* Cavern large */,
	1.0 /* Weird 1 */, 1.0 /* Weird 2 */, 1.0 /* Weird 3 */,
};

// Object definition
CSoundEngine gSoundEngine;

//=============================================
// @brief
//
//=============================================
inline byte Make8bit( Int16 sample )
{
	sample >>= 8;  // drop the low 8 bits
	sample ^= 0x80;  // toggle the sign bit
	return (sample & 0xFF);
}

//=============================================
// @brief
//
//=============================================
void Cmd_PlayMusic( void )
{
	if(gCommands.Cmd_Argc() < 2)
	{
		Con_Printf("playmusic usage: playmusic <file path> <time offset> <fade in time> <flags(loop/stop)>\n");
		return;
	}

	Int16 flags = 0;
	Float timeOffset = 0;
	const Char* pstrFilename = gCommands.Cmd_Argv(1);

	if(gCommands.Cmd_Argc() > 2)
	{
		// Extract time offset
		timeOffset = SDL_atof(gCommands.Cmd_Argv(2));
	}

	Float fadeintime = 0;
	if(gCommands.Cmd_Argc() > 3)
		fadeintime = SDL_atof(gCommands.Cmd_Argv(3));

	Int32 channel = 0;
	if(gCommands.Cmd_Argc() > 4)
		channel = SDL_atoi(gCommands.Cmd_Argv(4));

	if(gCommands.Cmd_Argc() > 5)
	{
		for(Uint32 i = 5; i < gCommands.Cmd_Argc(); i++)
		{
			const Char* pstrArg = gCommands.Cmd_Argv(i);
			if(!qstrcmp(pstrArg, "loop"))
				flags |= OGG_FL_LOOP;
			else if(!qstrcmp(pstrArg, "stop"))
				flags |= OGG_FL_STOP;
		}
	}

	gSoundEngine.PlayOgg(pstrFilename, channel, timeOffset, flags, fadeintime);
}

//=============================================
// @brief
//
//=============================================
void Cmd_StopMusic( void )
{
	gSoundEngine.StopOgg();
}

//=============================================
// @brief
//
//=============================================
void Cmd_SetHRTF( void )
{
	gSoundEngine.SetHRTFCommand();
}

//=============================================
// @brief Default constructor
//
//=============================================
CSoundEngine::CSoundEngine( void ):
	m_pCVarVolume(nullptr),
	m_pCVarGameVolume(nullptr),
	m_pCVarMusicVolume(nullptr),
	m_pCVarOcclusion(nullptr),
	m_pCvarDebug(nullptr),
	m_pCvarOnDemandLoad(nullptr),
	m_pDevice(nullptr),
	m_pContext(nullptr),
	m_hOpenALDLL(nullptr),
	m_isMuted(false),
	m_allSoundsPaused(false),
	m_isHRTFEnabled(false),
	alGenEffects(nullptr),
	alDeleteEffects(nullptr),
	alEffecti(nullptr),
	alEffectf(nullptr),
	alEffectfv(nullptr),
	alGenAuxiliaryEffectSlots(nullptr),
	alDeleteAuxiliaryEffectSlots(nullptr),
	alAuxiliaryEffectSloti(nullptr),
	alAuxiliaryEffectSlotf(nullptr),
	m_bInitialized(false),
	m_pPAS(nullptr),
	m_pPASBuffer(nullptr),
	m_reverbEffect(0),
	m_effectSlot(0),
	m_activeReverb(0),
	m_lastActiveReverb(0),
	m_idealReverb(0),
	m_pSentencesFile(nullptr)
{
	// Reset these also
	m_oggCallbacks.close_func = nullptr;
	m_oggCallbacks.read_func = nullptr;
	m_oggCallbacks.seek_func = nullptr;
	m_oggCallbacks.tell_func = nullptr;

	// Allocate arrays
	m_playingSoundsArray.resize(MAX_PLAYING_SOUNDS);
	m_activeSoundsArray.resize(MAX_ACTIVE_SOUNDS);
}

//=============================================
// @brief Destructor
//
//=============================================
CSoundEngine::~CSoundEngine( void )
{
	Shutdown();
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::Init( void )
{
	//
	// Load dll and create context
	//
	if(!m_hOpenALDLL)
	{
		m_hOpenALDLL = SDL_LoadObject(OPENAL_LIBRARY_PATH);
		if(!m_hOpenALDLL)
		{
			CString str;
			str << "Failed to load " << OPENAL_LIBRARY_PATH;
			MessageBox(nullptr, str.c_str(), "Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
			return false;
		}
	}

	m_pDevice = alcOpenDevice(nullptr);
	if(!m_pDevice)
	{
		Con_EPrintf("%s - alcOpenDevice failed.\n", __FUNCTION__);
		return false;
	}

	ALCint hrtfSetting = ALC_FALSE;
	conf_group_t* pGroup = gConfig.FindGroup(SOUNDENGINE_CONFIG_GRP_NAME);
	if(pGroup)
	{
		if(gConfig.GetInt(pGroup, SOUNDENGINE_HRTF_SETTING_NAME) == 1)
			hrtfSetting = ALC_TRUE;
	}

	m_isHRTFEnabled = (hrtfSetting == ALC_TRUE) ? true : false;

	// Initialize with HRTF
	ALCint contextAttribs[] = {
		ALC_HRTF_SOFT, hrtfSetting, /* request HRTF */
		0 /* end of list */
    };

    m_pContext = alcCreateContext(m_pDevice, contextAttribs);
    if(!m_pContext || !alcMakeContextCurrent(m_pContext))
    {
        if(m_pContext)
		{
			alcDestroyContext(m_pContext);
		}

		alcCloseDevice(m_pDevice);
		Con_EPrintf("Sound engine failed to create context.\n");
		return false;
    }

	//
	// Load extended functions
	//

	alGenEffects					= static_cast<LPALGENEFFECTS>(alGetProcAddress("alGenEffects"));
	alDeleteEffects					= static_cast<LPALDELETEEFFECTS>(alGetProcAddress("alDeleteEffects"));
	alEffecti						= static_cast<LPALEFFECTI>(alGetProcAddress("alEffecti"));
	alEffectf						= static_cast<LPALEFFECTF>(alGetProcAddress("alEffectf"));
	alEffectfv						= static_cast<LPALEFFECTFV>(alGetProcAddress("alEffectfv"));

	alGenAuxiliaryEffectSlots		= static_cast<LPALGENAUXILIARYEFFECTSLOTS>(alGetProcAddress("alGenAuxiliaryEffectSlots"));
	alDeleteAuxiliaryEffectSlots	= static_cast<LPALDELETEAUXILIARYEFFECTSLOTS>(alGetProcAddress("alDeleteAuxiliaryEffectSlots"));
	alAuxiliaryEffectSloti			= static_cast<LPALAUXILIARYEFFECTSLOTI>(alGetProcAddress("alAuxiliaryEffectSloti"));
	alAuxiliaryEffectSlotf			= static_cast<LPALAUXILIARYEFFECTSLOTF>(alGetProcAddress("alAuxiliaryEffectSlotf"));

	if(!alGenEffects || !alDeleteEffects || !alEffecti || !alEffectf 
		|| !alEffectfv || !alGenAuxiliaryEffectSlots || !alDeleteAuxiliaryEffectSlots
		|| !alAuxiliaryEffectSloti || !alAuxiliaryEffectSlotf)
	{
		Con_EPrintf("Sound engine failed to load functions. Your openal.dll might be out of date.\n");
		return false;
	}

	// Create cvars
	m_pCVarVolume		= gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), VOLUME_CVAR_NAME, "1", "Controls master volume.");
	m_pCVarGameVolume	= gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), GAME_VOLUME_CVAR_NAME, "1", "Controls game volume.");
	m_pCVarMusicVolume	= gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), MUSIC_VOLUME_CVAR_NAME, "1", "Controls music volume.");
	m_pCVarOcclusion	= gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "s_occlusion", "1", "Controls sound occlusion dimming.");
	m_pCvarDebug		= gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "s_debug", "0", "Draws sound debug points.");
	m_pCvarOnDemandLoad	= gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "s_ondemand", "0", "Load larger audio files on-demand.");
	
	// Set distance model
	alDistanceModel(AL_LINEAR_DISTANCE);

	// Set doppler factor
	alDopplerFactor(1.0);
	alDopplerVelocity(SPEED_OF_SOUND);

	// Create the reverb slot
	alGenEffects(1, &m_reverbEffect);
	alEffecti(m_reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	alGenAuxiliaryEffectSlots(1, &m_effectSlot);
	alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, m_reverbEffect);
	alAuxiliaryEffectSlotf(m_effectSlot, AL_EFFECTSLOT_GAIN, 1.0);

	// Set up callback structure
	m_oggCallbacks.read_func = AR_readOgg;
	m_oggCallbacks.seek_func = AR_seekOgg;
	m_oggCallbacks.close_func = AR_closeOgg;
	m_oggCallbacks.tell_func = AR_tellOgg;

	PrintStats();

	// Allocate PAS buffer
	Uint32 pasBufferSize = ens.visbuffersize;
	m_pPASBuffer = new byte[pasBufferSize];
	memset(m_pPASBuffer, 0, sizeof(byte)*pasBufferSize);

	// Load sentences
	if(!LoadSentences())
	{
		Con_EPrintf("Error loading sentences.\n");
		return false;
	}

	gCommands.CreateCommand("playmusic", Cmd_PlayMusic, "Plays a music track");
	gCommands.CreateCommand(STOP_MUSIC_CMD_NAME, Cmd_StopMusic, "Stops any music tracks");
	gCommands.CreateCommand("s_sethrtf", Cmd_SetHRTF, "Set HRTF setting");

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::Shutdown( void )
{
	// Kill any playing sounds
	StopOgg();

	// Reset sounds
	ResetEngine(true);

	if(!m_msgCacheList.empty())
	{
		m_msgCacheList.begin();
		while(!m_msgCacheList.end())
		{
			delete m_msgCacheList.get();
			m_msgCacheList.next();
		}

		m_msgCacheList.clear();
	}

	if(!m_updateMsgCacheList.empty())
	{
		m_updateMsgCacheList.begin();
		while(!m_updateMsgCacheList.end())
		{
			delete m_updateMsgCacheList.get();
			m_updateMsgCacheList.next();
		}

		m_updateMsgCacheList.clear();
	}

	if(m_effectSlot)
	{
		// Clear reverb effect
		alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
		alDeleteAuxiliaryEffectSlots(1, &m_effectSlot);
		alDeleteEffects(1, &m_reverbEffect);
	}

	if(m_pContext)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_pContext);
	}

	if(m_pDevice)
	{
		alcCloseDevice(m_pDevice);
	}

	if(m_hOpenALDLL)
	{
		SDL_UnloadObject(m_hOpenALDLL);
		m_hOpenALDLL = nullptr;
	}

	if(m_pPASBuffer)
	{
		delete[] m_pPASBuffer;
		m_pPASBuffer = nullptr;
	}

	if(m_pSentencesFile)
	{
		delete m_pSentencesFile;
		m_pSentencesFile = nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::PrintStats( void )
{
	// Print OpenAL version
	const ALchar* openALVersion = alGetString(AL_VERSION);
	Con_Printf("OpenAL version %s detected.\n", openALVersion);
	Con_Printf("Sound engine initialization was successful.\n");

	// Print status of HRTF
	ALCint hrtfStatus;
	alcGetIntegerv(m_pDevice, ALC_HRTF_STATUS_SOFT, 1, &hrtfStatus);
	switch(hrtfStatus)
	{
		case ALC_HRTF_DISABLED_SOFT:				Con_Printf("Head-related transfer function mixing is disabled.\n"); break;
		case ALC_HRTF_ENABLED_SOFT:					Con_Printf("Head-related transfer function mixing is enabled.\n"); break;
		case ALC_HRTF_DENIED_SOFT:					Con_Printf("Head-related transfer function mixing is denied.\n"); break;
		case ALC_HRTF_REQUIRED_SOFT:				Con_Printf("Head-related transfer function mixing is enabled.\n"); break;
		case ALC_HRTF_HEADPHONES_DETECTED_SOFT:		Con_Printf("Head-related transfer function mixing is enabled for headphones.\n"); break;
		case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT :		Con_Printf("Head-related transfer function mixing is disabled due to unsupported output format.\n"); break;
		default:									Con_Printf("Unknown HRTF status received from OpenAL.\n"); break;
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::InitGame( void )
{
	// Reset muted state
	m_isMuted = false;
	// Reset paused states
	m_allSoundsPaused = false;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::ResetGame( void )
{
	// Reset muted state
	m_isMuted = false;
	// Reset paused states
	m_allSoundsPaused = false;

	// Reset gameplay sounds
	ResetEngine(false);
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::FreeResources( rs_level_t level, bool clearall )
{
	if(!m_cachedSoundsList.empty())
	{
		m_cachedSoundsList.begin();
		while(!m_cachedSoundsList.end())
		{
			snd_cache_t* pcache = m_cachedSoundsList.get();
			if(pcache->level > level)
			{
				m_cachedSoundsList.next();
				continue;
			}

			// Delete any sounds playing this file
			for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
			{
				if(m_activeSoundsArray[i].pcache == pcache)
				{
					m_activeSoundsArray[i].flags |= SND_FL_KILLME;
					m_activeSoundsArray[i].pcache = nullptr;
				}
			}

			m_cachedSoundsList.remove(m_cachedSoundsList.get_link());
			m_cachedSoundsList.next();

			if(pcache->pbuffers)
				alDeleteBuffers(pcache->numbuffers, pcache->pbuffers);

			if(pcache->pdata)
				delete[] pcache->pdata;

			if(pcache->pbuffers)
				delete[] pcache->pbuffers;

			delete pcache;
		}
	}

	if(!m_cachedOggFilesList.empty())
	{
		m_cachedOggFilesList.begin();
		while(!m_cachedOggFilesList.end())
		{
			snd_oggcache_t* pcache = m_cachedOggFilesList.get();
			if(pcache->level > level)
			{
				m_cachedOggFilesList.next();
				continue;
			}

			if(pcache->pfileptr)
				delete[] pcache->pfileptr;

			delete pcache;

			m_cachedOggFilesList.remove(m_cachedOggFilesList.get_link());
			m_cachedOggFilesList.next();
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::ResetEngine( bool clearall )
{
	// Clear all playing music tracks
	StopOgg(MUSIC_CHANNEL_ALL, clearall ? true : false);

	if(!m_msgCacheList.empty())
	{
		m_msgCacheList.begin();
		while(!m_msgCacheList.end())
		{
			delete m_msgCacheList.get();
			m_msgCacheList.next();
		}
		m_msgCacheList.clear();
	}

	if(!m_updateMsgCacheList.empty())
	{
		m_updateMsgCacheList.begin();
		while(!m_updateMsgCacheList.end())
		{
			delete m_updateMsgCacheList.get();
			m_updateMsgCacheList.next();
		}
		m_updateMsgCacheList.clear();
	}

	FreeResources(RS_GAME_LEVEL, clearall);

	if(clearall)
		FreeResources(RS_APP_LEVEL, clearall);

	// Reset values
	m_activeReverb = 0;
}

/*
====================
RemovePlaying

====================
*/
void CSoundEngine::RemovePlaying( CSoundEngine::snd_active_t& sound )
{
	if(!sound.pplaying)
		return;

	// Clear from OpenAL
	alSourceStop(sound.pplaying->sourceindex);
	alDeleteSources(1, &sound.pplaying->sourceindex);

	// Reset datapos
	sound.datapos = -1;

	// Clear from system
	(*sound.pplaying) = snd_playing_t();
	sound.pplaying = nullptr;
}

//=============================================
// @brief
//
//=============================================
CSoundEngine::snd_active_t* CSoundEngine::AllocSound( const Char *sample, entindex_t entindex, Int32 flags, Int32 channel )
{
	// Don't let tempents clutter up or more than one hud sound play
	if((flags & SND_FL_TEMPENT) || (flags & SND_FL_RADIO))
	{
		Uint32 numfound = 0;
		for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
		{
			snd_active_t& sound = m_activeSoundsArray[i];
			if(!sound.active)
				continue;

			if(sound.flags & SND_FL_KILLME)
				continue;

			if(sound.flags & (SND_FL_TEMPENT|SND_FL_RADIO))
			{
				if((sound.flags & SND_FL_TEMPENT) && (flags & SND_FL_TEMPENT))
				{
					if(++numfound == MAX_ACTIVE_TEMP_SOUNDS)
						return nullptr;
				}
				else if((sound.flags & SND_FL_RADIO) && (flags & SND_FL_RADIO))
				{
					sound.flags |= SND_FL_KILLME;
					break;
				}
			}
		}
	}

	// See if we can clear on this ent
	if(entindex)
	{
		for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
		{
			snd_active_t& sound = m_activeSoundsArray[i];
			if(!sound.active)
				continue;

			if(sound.flags & (SND_FL_KILLME|SND_FL_RADIO|SND_FL_DIALOUGE))
				continue;

			if(sound.flags & SND_FL_MOTORBIKE && !( flags & SND_FL_MOTORBIKE ))
				continue;

			if(sound.channel == SND_CHAN_STATIC)
				continue;

			if((sound.entindex == entindex) && (sound.channel == channel))
			{
				sound.flags |= SND_FL_KILLME;
				break;
			}
		}
	}

	// Find a free slot
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(!m_activeSoundsArray[i].active)
		{
			return &m_activeSoundsArray[i];
		}
	}

	// Clear an unimportant slot
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		snd_active_t& sound = m_activeSoundsArray[i];
		if(sound.flags & (SND_FL_RADIO|SND_FL_DIALOUGE|SND_FL_MOTORBIKE|SND_FL_AMBIENT))
			continue;

		if(sound.channel == SND_CHAN_VOICE)
			continue;

		KillSound(sound);
		return &sound;
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::KillSound( snd_active_t& sound )
{
	// Shut mouth
	if(sound.channel == SND_CHAN_VOICE)
		sound.pentity->mouth.mouthopen = 0;

	// Remove from openal
	if(sound.pplaying)
		RemovePlaying(sound);

	// Remove any sentences
	if(sound.psentence)
	{
		cls.dllfuncs.pfnRemoveSubtitle(sound.psentence->name.c_str());
	}
	else if(sound.pcache)
	{
		CString filename;
		Common::Basename(sound.pcache->name.c_str(), filename);
		cls.dllfuncs.pfnRemoveSubtitle(filename.c_str());
	}

	if(sound.pcache && sound.pcache->ondemand)
	{
		// Decrement reference counter
		sound.pcache->refcounter--;

		if(sound.pcache->refcounter <= 0)
		{
			snd_cache_t* pcache = sound.pcache;
			alDeleteBuffers(pcache->numbuffers, pcache->pbuffers);

			if(pcache->pdata)
			{
				delete[] pcache->pdata;
				pcache->pdata = nullptr;
			}

			if(pcache->pbuffers)
			{
				delete[] pcache->pbuffers;
				pcache->pbuffers = nullptr;
			}
		}
	}

	// Clear data
	sound = snd_active_t();
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::CalcMouth8( snd_active_t& sound )
{
	if(sound.pcache->channels > 1)
		return;

	snd_cache_t *pcache = sound.pcache;
	mouth_t *pmouth = &sound.pentity->mouth;

	Uint32 i = 0;
	Int32 mouthavg = 0;
	for(; i < AVERAGE_SAMPLES_8BIT; i++)
	{
		Int32 sndPos = sound.datapos - (AVERAGE_SAMPLES_8BIT/3) + i;
		if(sndPos < 0)
			continue;

		if(sndPos > pcache->length)
			break;

		byte *pdata = pcache->pdata+sndPos;
		mouthavg += abs((*pdata)-127);
	}

	mouthavg = (mouthavg-10)/static_cast<Float>(i);
	if(mouthavg < 0) mouthavg = 0;
	if(mouthavg > 255) mouthavg = 255;

	Uint32 lastval = pmouth->mouthopen;
	pmouth->mouthopen = ((lastval+mouthavg)/2);
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::CalcMouth16( snd_active_t& sound )
{
	if(sound.pcache->channels > 1)
		return;

	snd_cache_t *pcache = sound.pcache;
	mouth_t *pmouth = &sound.pentity->mouth;

	Uint32 i = 0;
	Int32 mouthavg = 0;
	for(; i < AVERAGE_SAMPLES_16BIT; i++)
	{
		Int32 sndPos = sound.datapos - (AVERAGE_SAMPLES_16BIT/3) + i*sizeof(Int16);
		if(sndPos < 0)
			continue;

		if(sndPos > pcache->length)
			break;

		// Convert it to 8-bit
		Int16 sample = Common::ByteToUint16(pcache->pdata+sndPos);
		byte _8bitdata = Make8bit(sample);

		mouthavg += abs((_8bitdata & 0xFF)-127);
	}

	mouthavg = (mouthavg-10)/ static_cast<Float>(i);
	if(mouthavg < 0) mouthavg = 0;
	if(mouthavg > 255) mouthavg = 255;

	Int32 lastval = pmouth->mouthopen;
	pmouth->mouthopen = ((lastval+mouthavg)/2);
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::ShouldPlay( snd_active_t *psound, const Vector& vieworg, Double frametime, Int32 imsgnum, Float gameVolume ) const
{
	if(!frametime)
		return false;

	if(!gWindow.IsActive())
		return false;

	if(m_allSoundsPaused && !(psound->flags & SND_FL_MENU))
		return false;

	if(gameVolume <= 0 && !(psound->flags & SND_FL_MENU))
		return false;

	if(!(psound->flags & SND_FL_2D))
	{
		Float dist = (vieworg-psound->origin).Length2D();
		if(dist > psound->radius)
			return false;

		if(psound->leafnum)
		{
			if(!(m_pPAS[psound->leafnum >> 3] & (1 << (psound->leafnum&7))))
				return false;
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::DrawNormal( void )
{
	// Draw debug blobs
	if(m_pCvarDebug->GetValue() < 1)
		return true;

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_POINT_SMOOTH);
	glPointSize(15);

	CBasicDraw* pDraw = CBasicDraw::GetInstance();

	if(!pDraw->Enable() || !pDraw->DisableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->SetProjection(rns.view.projection.GetMatrix());
	pDraw->SetModelview(rns.view.modelview.GetMatrix());

	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(!m_activeSoundsArray[i].active)
			continue;

		snd_active_t* psound = &m_activeSoundsArray[i];

		if(psound->pplaying)
			pDraw->Color4f(0.0, 1.0, 0.0, 1.0);
		else
			pDraw->Color4f(1.0, 0.0, 0.0, 1.0);

		pDraw->Begin(GL_POINTS);
		pDraw->Vertex3fv(psound->origin);
		pDraw->End();

		if(psound->leafnum)
			pDraw->Color4f(0.0, 0.0, 1.0, 1.0);
		else
			pDraw->Color4f(1.0, 0.0, 0.0, 1.0);

		pDraw->Begin(GL_POINTS);
		pDraw->Vertex3fv(psound->origin+Vector(0, 0, 5));
		pDraw->End();
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_POINT_SMOOTH);

	pDraw->Disable();
	return true;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::UpdateReverb( ref_params_t *pparams )
{
	// Check for underwater
	m_idealReverb = m_activeReverb;
	if(pparams->waterlevel == WATERLEVEL_FULL)
		m_idealReverb = 15;

	if(!m_idealReverb)
		return;

	if(m_idealReverb == m_lastActiveReverb)
		return;

	if(m_idealReverb < 0 || m_idealReverb > NUM_REVERBS)
		m_idealReverb = 0;

	Float fxMultiplier = g_pEAXMultipliers[m_idealReverb];

	alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
	alEffectf(m_reverbEffect, AL_EAXREVERB_DENSITY, g_pEAXEffects[m_idealReverb].flDensity * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_DIFFUSION, g_pEAXEffects[m_idealReverb].flDiffusion * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_GAIN, g_pEAXEffects[m_idealReverb].flGain * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_GAINHF, g_pEAXEffects[m_idealReverb].flGainHF);
	alEffectf(m_reverbEffect, AL_EAXREVERB_GAINLF, g_pEAXEffects[m_idealReverb].flGainLF);
	alEffectf(m_reverbEffect, AL_EAXREVERB_DECAY_TIME, g_pEAXEffects[m_idealReverb].flDecayTime * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_DECAY_HFRATIO, g_pEAXEffects[m_idealReverb].flDecayHFRatio);
	alEffectf(m_reverbEffect, AL_EAXREVERB_DECAY_LFRATIO, g_pEAXEffects[m_idealReverb].flDecayLFRatio);
	alEffectf(m_reverbEffect, AL_EAXREVERB_REFLECTIONS_GAIN, g_pEAXEffects[m_idealReverb].flReflectionsGain * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_REFLECTIONS_DELAY, g_pEAXEffects[m_idealReverb].flReflectionsDelay * fxMultiplier);
	alEffectfv(m_reverbEffect, AL_EAXREVERB_REFLECTIONS_PAN, g_pEAXEffects[m_idealReverb].flReflectionsPan);
	alEffectf(m_reverbEffect, AL_EAXREVERB_LATE_REVERB_GAIN, g_pEAXEffects[m_idealReverb].flLateReverbGain * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_LATE_REVERB_DELAY, g_pEAXEffects[m_idealReverb].flLateReverbDelay * fxMultiplier);
	alEffectfv(m_reverbEffect, AL_EAXREVERB_LATE_REVERB_PAN, g_pEAXEffects[m_idealReverb].flLateReverbPan);
	alEffectf(m_reverbEffect, AL_EAXREVERB_ECHO_TIME, g_pEAXEffects[m_idealReverb].flEchoTime * fxMultiplier);
	alEffectf(m_reverbEffect, AL_EAXREVERB_ECHO_DEPTH, g_pEAXEffects[m_idealReverb].flEchoDepth);
	alEffectf(m_reverbEffect, AL_EAXREVERB_MODULATION_TIME, g_pEAXEffects[m_idealReverb].flModulationTime);
	alEffectf(m_reverbEffect, AL_EAXREVERB_MODULATION_DEPTH, g_pEAXEffects[m_idealReverb].flModulationDepth);
	alEffectf(m_reverbEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, g_pEAXEffects[m_idealReverb].flAirAbsorptionGainHF);
	alEffectf(m_reverbEffect, AL_EAXREVERB_HFREFERENCE, g_pEAXEffects[m_idealReverb].flHFReference);
	alEffectf(m_reverbEffect, AL_EAXREVERB_LFREFERENCE, g_pEAXEffects[m_idealReverb].flLFReference);
	alEffectf(m_reverbEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, g_pEAXEffects[m_idealReverb].flRoomRolloffFactor);
	alEffecti(m_reverbEffect, AL_EAXREVERB_DECAY_HFLIMIT, g_pEAXEffects[m_idealReverb].iDecayHFLimit);
	alAuxiliaryEffectSloti(m_effectSlot, AL_EFFECTSLOT_EFFECT, m_reverbEffect);
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::ClearCache( void )
{
	m_updateMsgCacheList.begin();
	while(!m_updateMsgCacheList.end())
	{
		s_msg_cache_t* pmsg = m_updateMsgCacheList.get();

		cl_entity_t *pEntity = nullptr;
		if(pmsg->type != SND_CT_PLAYAMBIENTSOUND
			&& pmsg->type != SND_CT_APPLYEFFECT)
		{
			pEntity = CL_GetEntityByIndex(pmsg->entindex);
			if(!pEntity)
			{
				m_updateMsgCacheList.next();
				continue;
			}

			if(!pEntity->pmodel)
			{
				m_updateMsgCacheList.next();
				continue;
			}
		}

		// Calculate offset since start
		Double timeoffset = (cls.cl_time - pmsg->cachetime + pmsg->timeoffs);
		if(pmsg->looping || timeoffset < pmsg->duration)
		{
			// Play the sound
			UpdateSound(nullptr, &pmsg->origin, pmsg->flags, pmsg->channel, pmsg->volume, pmsg->pitch, pmsg->atten, pEntity, pmsg->entindex, pmsg->svindex, timeoffset);
		}

		// Remove it
		delete m_updateMsgCacheList.get();
		m_updateMsgCacheList.remove(m_updateMsgCacheList.get_link());
		m_updateMsgCacheList.next();
	}

	m_msgCacheList.begin();
	while(!m_msgCacheList.end())
	{
		s_msg_cache_t* pmsg = m_msgCacheList.get();

		cl_entity_t *pEntity = nullptr;
		if(pmsg->type != SND_CT_PLAYAMBIENTSOUND
			&& pmsg->type != SND_CT_APPLYEFFECT)
		{
			pEntity = CL_GetEntityByIndex(pmsg->entindex);
			if(!pEntity)
			{
				m_msgCacheList.next();
				continue;
			}

			if(!pEntity->pmodel)
			{
				m_msgCacheList.next();
				continue;
			}
		}

		// Handle if we got muted
		if(m_isMuted && !(pmsg->flags & (SND_FL_MUTEIGNORE|SND_FL_MENU)))
		{
			delete m_msgCacheList.get();
			m_msgCacheList.remove(m_msgCacheList.get_link());
			m_msgCacheList.next();
			continue;
		}

		// Apply effect or play sound
		if(pmsg->type == SND_CT_APPLYEFFECT)
		{
			// Only apply and clear if the sound effect is playing
			if(!IsSoundPlaying(pmsg->entindex, pmsg->svindex, pmsg->channel))
			{
				// Remove if over the limit
				if(pmsg->cachetime+CACHED_MSG_DELETE_TIME < cls.cl_time)
				{
					Con_Printf("%s - Effect with entity %d, server index %d and channel %d timed out.\n", __FUNCTION__, pmsg->entindex, pmsg->svindex, pmsg->channel);
					delete m_msgCacheList.get();
					m_msgCacheList.remove(m_msgCacheList.get_link());
				}

				m_msgCacheList.next();
				continue;
			}

			// Apply the effect
			ApplySoundEffect(pmsg->entindex, pmsg->svindex, pmsg->channel, pmsg->effect, pmsg->effectduration, pmsg->targetvalue);
		}
		else
		{
			// Calculate offset since start
			Double timeoffset = (cls.cl_time - pmsg->cachetime + pmsg->timeoffs);
			if(pmsg->looping || timeoffset < pmsg->duration)
			{
				// Play the sound
				PlaySound(nullptr, &pmsg->origin, pmsg->flags, pmsg->channel, pmsg->volume, pmsg->pitch, pmsg->atten, pEntity, pmsg->entindex, pmsg->svindex, timeoffset);
			}
		}
		
		// Remove it
		delete m_msgCacheList.get();
		m_msgCacheList.remove(m_msgCacheList.get_link());
		m_msgCacheList.next();
	}
}

//=============================================
// @brief
//
//=============================================
CSoundEngine::snd_cache_t* CSoundEngine::PrecacheSound( const Char *sample, Int32 serverindex, rs_level_t level, bool isforplayback )
{
	CString filepath;

	if(sample)
	{
		if(sample[0] == '*')
			sample++;

		if(!qstrstr(sample, SOUND_FOLDER_BASE_PATH) && !qstrstr(sample, "sound\\"))
			filepath << SOUND_FOLDER_BASE_PATH;

		if(sample[0] == '*')
			filepath << &sample[1];
		else
			filepath << sample;

		m_cachedSoundsList.begin();
		while(!m_cachedSoundsList.end())
		{
			snd_cache_t* pcache = m_cachedSoundsList.get();
			if(!qstrcmp(pcache->name, filepath.c_str()))
			{
				// Fix up the cache index if needed
				if(serverindex != -1 && pcache->svindex == -1)
					pcache->svindex = serverindex;

				if(!pcache->pbuffers)
				{
					if(!LoadSoundData(filepath.c_str(), pcache, serverindex, isforplayback))
					{
						m_cachedSoundsList.remove(m_cachedSoundsList.get_link());
						delete pcache;
						return nullptr;
					}
				}

				return pcache;
			}

			m_cachedSoundsList.next();
		}
	}
	else
	{
		m_cachedSoundsList.begin();
		while(!m_cachedSoundsList.end())
		{
			snd_cache_t* pcache = m_cachedSoundsList.get();
			if(pcache->svindex == serverindex)
			{
				if(!pcache->pbuffers)
				{
					if(!LoadSoundData(pcache->name.c_str(), pcache, serverindex, isforplayback))
					{
						m_cachedSoundsList.remove(m_cachedSoundsList.get_link());
						delete pcache;
						return nullptr;
					}
				}

				return pcache;
			}
			m_cachedSoundsList.next();
		}

		return nullptr;
	}

	// Allocate new entry
	snd_cache_t* pcache = new snd_cache_t();
	pcache->name = filepath.c_str();
	pcache->level = level;
	pcache->index = m_cachedSoundsList.size()-1;
	pcache->svindex = serverindex;
	pcache->loopbegin = -1;

	if(!LoadSoundData(filepath.c_str(), pcache, serverindex, isforplayback))
	{
		delete pcache;
		return nullptr;
	}

	m_cachedSoundsList.add(pcache);

	return pcache;
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::LoadSoundData( const Char *sample, snd_cache_t* pcache, Int32 serverindex, bool keepdata )
{
	// See if it exists
	Uint32 isize = 0;
	const byte *pfile = FL_LoadFile(sample, &isize);
	if(!pfile)
	{
		for(Uint32 i = 0; i < m_missingArray.size(); i++)
		{
			if(!qstrcmp(sample, m_missingArray[i].filename.c_str()))
				return nullptr;
		}

		snd_missing_t newMissing;
		newMissing.filename = sample;
		newMissing.sv_index = serverindex;
		m_missingArray.push_back(newMissing);

		Con_Printf("%s - Could not precache: %s.\n", __FUNCTION__, sample);
		return nullptr;
	}

	if(strncmp(reinterpret_cast<const Char *>(pfile), "RIFF", 4))
	{
		Con_Printf("%s - %s is not a valid .WAV file!\n", __FUNCTION__, sample);
		FL_FreeFile(pfile);
		return nullptr;
	}

	// Allocate new data
	const byte *pbegin = pfile + 12;
	const byte *pend = pfile + isize;

	while(1)
	{
		if(pbegin >= pend)
			break;

		DWORD ilength = Common::ByteToInt32(pbegin+4);
		Common::ScaleByte(&ilength);

		if(!strncmp(reinterpret_cast<const char*>(pbegin), "fmt ", 4))
		{
			pcache->channels = Common::ByteToUint16(pbegin+10);
			pcache->samplerate = Common::ByteToInt32(pbegin+12);
			pcache->bitspersample = Common::ByteToUint16(pbegin+22);
		}

		if(!strncmp(reinterpret_cast<const char*>(pbegin), "cue ", 4))
		{
			// Only grab offset from the first cue point
			pcache->loopbegin = Common::ByteToInt32(pbegin+32)*(pcache->bitspersample>>3);
		}

		if(!strncmp(reinterpret_cast<const char*>(pbegin), "data", 4))
		{
			pcache->dataoffset = (pbegin+8)-pfile;
			pcache->length = Common::ByteToInt32(pbegin+4);
		}

		pbegin = pbegin + 8 + ilength;
	}

	Int32 format = 0;
	if(pcache->channels == 1)
	{
		if(pcache->bitspersample == 8)
			format = AL_FORMAT_MONO8;
		else
			format = AL_FORMAT_MONO16;
	}
	else
	{
		if(pcache->bitspersample == 8)
			format = AL_FORMAT_STEREO8;
		else
			format = AL_FORMAT_STEREO16;
	}
		
	if(pcache->loopbegin > 0)
		pcache->numbuffers = 2;
	else
		pcache->numbuffers = 1;

	if(m_pCvarOnDemandLoad->GetValue() >= 1.0 && isize >= ONDEMAND_SOUND_SIZE_LIMIT)
	{
		Con_DPrintf("%s - Sound '%s' marked for on-demand load.\n", __FUNCTION__, sample);
		pcache->ondemand = true;
	}
	else
		pcache->ondemand = false;

	if(!pcache->ondemand || keepdata)
	{
		pcache->pbuffers = new ALuint[pcache->numbuffers];
		alGenBuffers(pcache->numbuffers, pcache->pbuffers);

		if(pcache->loopbegin > 0)
		{
			alBufferData(pcache->pbuffers[0], format, pfile+pcache->dataoffset, pcache->loopbegin, pcache->samplerate);
			alBufferData(pcache->pbuffers[1], format, pfile+pcache->loopbegin, pcache->length-pcache->loopbegin+2, pcache->samplerate);
		}
		else
		{
			alBufferData(pcache->pbuffers[0], format, pfile+pcache->dataoffset, pcache->length, pcache->samplerate);
		}

		pcache->pdata = new byte[isize];
		memcpy(pcache->pdata, pfile, sizeof(byte)*isize);
	}

	FL_FreeFile(pfile);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::PrecacheServerSound( const Char *sample, Int32 serverindex )
{
	if(sample[0] == '!')
	{
		if(!m_pSentencesFile)
			return false;

		const CSentencesFile::sentence_t* psentence = m_pSentencesFile->GetSentenceDefinition(sample);
		for(Uint32 i = 0; i < psentence->chunks.size(); i++)
		{
			CString filepath;
			CSentencesFile::sent_chunk_t *pchunk = psentence->chunks[i];

			filepath << SOUND_FOLDER_BASE_PATH << psentence->folder << PATH_SLASH_CHAR << pchunk->soundname << ".WAV";

			PrecacheSound(filepath.c_str(), -1, RS_GAME_LEVEL, false);
		}

		return true;
	}
	else
	{
		CString filepath;
		if(sample[0] == '*')
			filepath << SOUND_FOLDER_BASE_PATH << sample + 1;
		else
			filepath << SOUND_FOLDER_BASE_PATH << sample;

		if(filepath.find(0, ".wav") != -1 
			|| filepath.find(0, ".WAV") != -1)
		{
			if(PrecacheSound(filepath.c_str(), serverindex, RS_GAME_LEVEL, false) != nullptr)
				return true;
		}
		else if(filepath.find(0, ".ogg") != -1 
			|| filepath.find(0, ".OGG") != -1)
		{
			if(PrecacheOgg(filepath.c_str(), RS_GAME_LEVEL) != nullptr)
				return true;
		}
		else
		{
			Con_Printf("%s - Unknown file type '%s'.\n", __FUNCTION__, filepath.c_str());
			return false;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::GetSoundCache( const Char *sample, Int32 svindex, snd_cache_t*& psample, const CSentencesFile::sentence_t *& psentence, CString& filepath )
{
	if(!sample)
	{
		if(svindex < 0)
		{
			Int32 sentindex = abs(svindex) - 1;
			psentence = m_pSentencesFile->GetSentenceDefinition(sentindex);
			if(!psentence)
			{
				Con_Printf("%s - Invalid sentence index %d.\n", __FUNCTION__, sentindex);
				return false;
			}

			filepath << SOUND_FOLDER_BASE_PATH << psentence->folder.c_str() 
				<< PATH_SLASH_CHAR << psentence->chunks[0]->soundname.c_str() << ".wav";
		}
		else
		{
			// All non-precached sounds will be assumed to be game-level sounds
			psample = PrecacheSound(nullptr, svindex, RS_GAME_LEVEL, true);

			if(!psample)
			{
				// See if it's already logged as missing
				for(Uint32 i = 0; i < m_missingArray.size(); i++)
				{
					if(m_missingArray[i].sv_index == svindex)
						return false;
				}

				// Houston, we have a problem
				Con_Printf("%s -Failed to cache server sound with index %d\n", __FUNCTION__, svindex );
				return false;
			}
			else
			{
				// Get it from sample
				filepath = psample->name;
			}
		}
	}
	else
	{
		filepath << SOUND_FOLDER_BASE_PATH;

		if(sample[0] == '*')
			filepath << sample + 1;
		else
			filepath << sample;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::UpdateSound( const Char *sample, const Vector* pOrigin, Int32 flags, Int32 channel, Float volume, Int32 pitch, Float attenuation, cl_entity_t *entity, entindex_t entindex, Int32 svindex, Float timeoffs )
{
	CString filepath;
	snd_cache_t* psample;
	const CSentencesFile::sentence_t* psentence = nullptr;

	// Retrieve info
	if(!GetSoundCache(sample, svindex, psample, psentence, filepath))
		return;
	
	// Do not allow non-update messages
	if(!(flags & (SND_FL_STOP|SND_FL_CHANGE_VOLUME|SND_FL_CHANGE_PITCH)))
	{
		Con_EPrintf("%s - Non-update call recieved for sound '%s'.\n", __FUNCTION__, filepath.c_str());
		return;
	}

	// TRUE if a sound was updated
	bool wasUpdated = false;

	// Only change the sound if needed
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(!m_activeSoundsArray[i].active)
			continue;

		snd_active_t* psnd = &m_activeSoundsArray[i];

		if(psnd->entindex == entindex
			&& psnd->channel == channel)
		{
			if((flags & SND_FL_STOP) 
				|| (sample && !qstrcmp(psnd->pcache->name, filepath))
				|| psnd->pcache->svindex == svindex)
			{
				if(flags & SND_FL_STOP)
				{
					// Just kill the sound
					KillSound(*psnd);
				}
				else
				{
					if(flags & SND_FL_CHANGE_PITCH
						&& psnd->pitch != pitch
						&& psnd->targetpitch != pitch)
					{
						psnd->pitchchangetime = -1;
						psnd->targetpitch = pitch;
						psnd->prevpitch = psnd->pitch;
						psnd->pitchchangeduration = DEFAULT_SND_CHANGE_TIME;
					}

					if(flags & SND_FL_CHANGE_VOLUME
						&& psnd->volume != volume
						&& psnd->targetvolume != volume)
					{
						psnd->volchangetime = -1;
						psnd->targetvolume = volume;
						psnd->prevvolume = psnd->volume;
						psnd->volchangeduration = DEFAULT_SND_CHANGE_TIME;
					}
				}

				wasUpdated = true;
				break;
			}
		}
	}

	if(!wasUpdated)
	{
		m_msgCacheList.begin();
		while(!m_msgCacheList.end())
		{
			s_msg_cache_t* pmsg = m_msgCacheList.get();

			if(pmsg->channel == channel 
				&& pmsg->entindex == entindex
				&& pmsg->svindex == svindex)
			{
				if(flags & SND_FL_STOP)
				{
					// Delete the message itself
					delete m_msgCacheList.get();
					m_msgCacheList.remove(m_msgCacheList.get_link());
				}
				else
				{
					if(flags & SND_FL_CHANGE_PITCH)
					{
						// Update pitch on the cached message
						pmsg->pitch = pitch;
					}

					if(flags & SND_FL_CHANGE_VOLUME)
					{
						// Update volume on the cached message
						pmsg->volume = volume;
					}
				}

				wasUpdated = true;
			}

			m_msgCacheList.next();
		}

		if(!wasUpdated)
			Con_VPrintf("%s - Couldn't update sound '%s' on entity %d with flags %d.\n", __FUNCTION__, filepath.c_str(), entindex, flags);
	}

	if((flags & SND_FL_STOP) && (flags & SND_FL_HAS_SUBTITLES)) 
	{
		if(psentence)
		{
			cls.dllfuncs.pfnRemoveSubtitle(psentence->name.c_str());
		}
		else
		{
			CString filename;
			Common::Basename(psample->name.c_str(), filename);
			cls.dllfuncs.pfnRemoveSubtitle(filename.c_str());
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::PlaySound( const Char *sample, const Vector* pOrigin, Int32 flags, Int32 channel, Float volume, Int32 pitch, Float attenuation, cl_entity_t *entity, entindex_t entindex, Int32 svindex, Float timeoffs )
{
	if(!CL_CanPlayGameSounds() && !(flags & SND_FL_MENU))
	{
		if(sample)
			Con_Printf("%s - Attempted to play game sound '%s' without running game state.\n", __FUNCTION__, sample);
		else
			Con_Printf("%s - Attempted to play game sound with server index %d without running game state.\n", __FUNCTION__, svindex);

		return;
	}

	if(volume < 0 || volume > VOL_NORM)
	{
		if(sample)
			Con_Printf("%s - Invalid volume %f for sound effect with name '%s'.\n", __FUNCTION__, volume, sample);
		else
			Con_Printf("%s - Invalid volume %f for sound effect with server index %d.\n", __FUNCTION__, volume, svindex);

		return;
	}

	CString filepath;
	snd_cache_t *psample = nullptr;
	const CSentencesFile::sentence_t *psentence = nullptr;

	// Retrieve info
	if(!GetSoundCache(sample, svindex, psample, psentence, filepath))
		return;

	// Do not allow update calls here
	if(flags & (SND_FL_STOP|SND_FL_CHANGE_VOLUME|SND_FL_CHANGE_PITCH))
	{
		Con_EPrintf("%s - Update call recieved for sound '%s'.\n", __FUNCTION__, filepath.c_str());
		return;
	}

	// Sound engine is muted, don't play anything except muteall sounds
	if(m_isMuted && !(flags & (SND_FL_MUTEIGNORE|SND_FL_MENU)))
		return;

	if(!psample)
	{
		// All non-precached sounds will be assumed to be game-level sounds
		psample = PrecacheSound(filepath.c_str(), svindex, RS_GAME_LEVEL, true);

		if(!psample)
			return;
	}

	snd_active_t *psound = AllocSound(psample->name.c_str(), entindex, flags, channel);
	if(!psound)
	{
		if(!(flags & SND_FL_TEMPENT))
			Con_Printf("%s - Could not allocate sound slot for '%s' for entity %d.\n", filepath.c_str(), entindex);

		return;
	}

	if(m_pCvarDebug->GetValue() >= 2)
		Con_Printf("%s - Playing sound %s for entity %d.\n", __FUNCTION__, psample->name.c_str(), psound->entindex);

	//
	// Only put it in the list now
	// The sound is played only later
	psound->volume = volume;
	psound->pcache = psample;
	psound->channel = channel;
	psound->entindex = entindex;
	psound->pentity = entity;
	psound->flags = flags;
	psound->pitch = clamp(pitch, MIN_PITCH, MAX_PITCH);
	psound->mainpitch = pitch;
	psound->active = true;
	psound->psentence = psentence;
	psound->datapos = -1;
	psound->time = -1;
	psound->timeoffs = timeoffs;
	psound->cachename = psample->name;
	
	if(psample->ondemand)
	{
		// Increase refcounter on cache
		psample->refcounter++;
	}

	if(psound->psentence)
	{
		psound->pchunk = psentence->chunks[0];
		psound->volume = (static_cast<Float>(psound->pchunk->volume)/100.0f)*psound->volume;
		psound->pitch = (static_cast<Float>(psound->pchunk->pitch)/100.0f)*psound->mainpitch;

		if(psound->pchunk->start)
			psound->datapos = psample->length*(static_cast<Float>(psound->pchunk->start)/100.0f);
	}

	if(psound->flags & SND_FL_RADIUS)
	{
		psound->radius = attenuation;
	}
	else
	{
		if(attenuation != ATTN_NONE)
		{
			if(attenuation >= ATTN_NORM)
				psound->radius = MAX_DISTANCE + (1.0 - attenuation) * (0.5*MAX_DISTANCE);
			else
				psound->radius = MAX_DISTANCE + (1.0 - attenuation) * (4.0*MAX_DISTANCE);
	
			if(psound->radius < 0)
				psound->radius = 0;
		}
	}

	if(pOrigin && psound->radius)
	{
		Math::VectorCopy(*pOrigin, psound->origin);
	}
	else if(entity && psound->radius)
	{
		// some entities, even if removed, are still in the entlist
		// only their msgnum doesn't match, so we have to set it here
		if(entity->pmodel && Cache_GetModelType(*entity->pmodel) == MOD_BRUSH)
		{
			const brushmodel_t* pbrushmodel = entity->pmodel->getBrushmodel();

			Vector vmins = entity->curstate.origin + pbrushmodel->mins;
			Vector vmaxs = entity->curstate.origin + pbrushmodel->maxs;
			Math::VectorScale((vmins+vmaxs), 0.5, psound->origin);
		}
		else
		{
			Math::VectorCopy(entity->curstate.origin, psound->origin);
		}
	}

	if((psound->flags & SND_FL_RADIUS) && psound->radius < 64)
		Con_Printf("Warning: Sound %s with small radius %.0f at %.0f %.0f %.0f.\n", 
		psound->pcache->name.c_str(), psound->radius, 
		psound->origin.x, psound->origin.y, psound->origin.z);

	if(!entity && !pOrigin || !psound->radius)
		psound->flags |= SND_FL_2D;

	if(psound->flags & SND_FL_MENU)
		psound->flags |= SND_FL_2D;

	if(psound->pentity && (psound->pentity == CL_GetLocalPlayer() 
		|| psound->pentity == cls.dllfuncs.pfnGetViewModel()))
		psound->flags |= SND_FL_2D;

	if(!entity && !(psound->flags & SND_FL_2D) 
		&& !(psound->flags & SND_FL_OCCLUSIONLESS))
	{
		brushmodel_t* pworld = ens.pworld;
		if(pworld)
		{
			const mleaf_t *pleaf = Mod_PointInLeaf(*pOrigin, *pworld);
			if(pleaf->contents != CONTENTS_SOLID)
				psound->leafnum = pleaf - pworld->pleafs - 1;
		}
	}

	if(flags & SND_FL_SUB_ONLY_RADIUS)
	{
		cl_entity_t* pplayer = CL_GetLocalPlayer();
		if((pplayer->curstate.origin - psound->origin).Length() > psound->radius)
			return;
	}

	if(psentence)
	{
		Int32 bytepersec = psample->channels * (psample->samplerate) * (psample->bitspersample>>3);
		Float length = (static_cast<Float>(psample->length)/static_cast<Float>(bytepersec)) - timeoffs;

		if(cls.dllfuncs.pfnAddSubtitle(psentence->name.c_str(), length))
			psound->flags |= SND_FL_HAS_SUBTITLES;
	}
	else
	{
		Int32 bytepersec = psample->channels * (psample->samplerate) * (psample->bitspersample>>3);
		Float length = (static_cast<Float>(psample->length)/ static_cast<Float>(bytepersec)) - timeoffs;

		CString filename;
		Common::Basename(psample->name.c_str(), filename);
		if(cls.dllfuncs.pfnAddSubtitle(filename.c_str(), length))
			psound->flags |= SND_FL_HAS_SUBTITLES;
	}
}

//=============================================
// @brief
//
//=============================================
Float CSoundEngine::CalcGain( const Vector& vieworg, snd_active_t *psound, Float multval, Float gameVolume ) const
{
	Float volume = psound->volume;
	if(!(psound->flags & SND_FL_MENU))
		volume *= gameVolume;

	if(psound->flags & (SND_FL_DIALOUGE|SND_FL_RADIO|SND_FL_DIM_OTHERS|SND_FL_DIM_OTHERS_LIGHT))
		return volume;

	if(!(psound->flags & SND_FL_2D))
	{ 
		Float flrefdist = psound->radius*0.1;
		Float fldistance = (vieworg-psound->origin).Length();
		if(fldistance > psound->radius)
			fldistance = psound->radius;

		Float flgain = (1-SE_ROLLOFF_FACTOR*(fldistance-flrefdist)/(psound->radius-flrefdist));

		if(m_pCVarOcclusion->GetValue() >= 1 && !(psound->flags & (SND_FL_OCCLUSIONLESS|SND_FL_2D)))
		{
			trace_t tr;
			CL_PlayerTrace(vieworg, psound->origin, FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, tr);
			if(tr.fraction != 1.0)
				flgain = flgain*0.5;
		}

		return volume*flgain*multval;
	}
	else
		return volume*multval;
}

//=============================================
// @brief
//
//=============================================
Int32 CSoundEngine::GetSyncOffset( const snd_cache_t *pcache, Uint32 index )
{
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(i == index)
			continue;

		if(!m_activeSoundsArray[i].active)
			continue;

		if(m_activeSoundsArray[i].flags & SND_FL_KILLME)
			continue;

		if(m_activeSoundsArray[i].pcache != pcache)
			continue;

		if(!m_activeSoundsArray[i].pplaying)
			continue;

		snd_active_t *psound = &m_activeSoundsArray[i];
		snd_playing_t *pplaying = psound->pplaying;
		alGetSourcei(pplaying->sourceindex, AL_BYTE_OFFSET, &psound->datapos);
		
		return psound->datapos;
	}

	return -1;
}

//=============================================
// @brief
//
//=============================================
CSoundEngine::stream_result_t CSoundEngine::Stream( ALuint buffer, snd_music_t& track )
{
	static Char data[BUFFER_SIZE];
	Int32 size = 0;
	Int32 section;

	while(size < BUFFER_SIZE)
	{
		Int32 result = ov_read(&track.stream, data+size, BUFFER_SIZE-size, 0, 2, 1, &section);
		if(result < 0)
		{
			switch(result)
			{
			case OV_EBADLINK:
				Con_EPrintf("%s - Error streaming '%s'. Corrupt or invalid data encountered.\n", __FUNCTION__, track.pfile->filepath.c_str());
				break;
			case OV_EINVAL:
				Con_EPrintf("%s - Error streaming '%s'. Invalid or corrupt file.\n", __FUNCTION__, track.pfile->filepath.c_str());
				break;
			case OV_HOLE:
				Con_EPrintf("%s - Error streaming '%s'. Stream was interrupted.\n", __FUNCTION__, track.pfile->filepath.c_str());
				break;
			}

			return STREAM_ERROR;
		}
		else if(result > 0)
		{
			// Streaming was successful, so keep at it
			size += result;
		}
		else
		{
			// Reached stream end
			break;
		}
	}

	if(!size)
	{
		// Reached EOF
		return STREAM_EOF;
	}
	else
	{
		alBufferData(buffer, track.format, data, size, track.info->rate);
		return STREAM_OK;
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::Update( ref_params_t *pparams )
{
	if(CL_CanPlayGameSounds())
	{
		//
		// Set reverb
		//
		UpdateReverb(pparams);

		//
		// attempt to add any cached sounds
		//
		ClearCache();
	}

	if(CL_CanPlayGameSounds())
	{
		//
		// Set listener properties
		//
		Float flOrientation[6];
		Vector vForward, vUp;
		Math::AngleVectors(pparams->v_angles, &vForward, nullptr, &vUp);

		for(Uint32 i = 0; i < 3; i++)
		{
			flOrientation[i] = vForward[i];
			flOrientation[3+i] = vUp[i];
		}

		// Retreive local player
		cl_entity_t* pPlayer = CL_GetLocalPlayer();
		if(!pPlayer)
			return;

		Vector& velocity = pPlayer->curstate.velocity;

		alListenerfv(AL_ORIENTATION, flOrientation);
		alListenerfv(AL_POSITION, pparams->v_origin);
		alListenerf(AL_METERS_PER_UNIT, 0.01905f);
		alListener3f(AL_VELOCITY, velocity[0], velocity[1], velocity[2]);
	}

	// Set volume
	alListenerf(AL_GAIN, m_pCVarVolume->GetValue());

	// Get game volume cvar
	Float gameVolume = m_pCVarGameVolume->GetValue();

	//
	// Set PHS/PAS
	//
	if(CL_CanPlayGameSounds())
	{
		const mleaf_t *pleaf = Mod_PointInLeaf(pparams->v_origin, *ens.pworld); 
		m_pPAS = Mod_LeafPAS(m_pPASBuffer, ens.visbuffersize, *pleaf, *ens.pworld);
	}

	//
	// Manage sounds
	//

	// Get local player and his messagenum
	Int32 imsgnum = 0;
	Float flmultval = 1.0;
	cl_entity_t *pplayer = nullptr;

	// Only run these if the game is active
	if(CL_CanPlayGameSounds())
	{
		pplayer = CL_GetLocalPlayer();
		if(!pplayer)
			return;

		imsgnum = pplayer->curstate.msg_num;

		// Lower volume if specific sounds are playing
		for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
		{
			if(!m_activeSoundsArray[i].active)
				continue;

			if(m_activeSoundsArray[i].flags & (SND_FL_RADIO|SND_FL_DIM_OTHERS))
			{
				flmultval = 0.1;
				break;
			}
			else if(m_activeSoundsArray[i].flags & (SND_FL_DIALOUGE|SND_FL_DIM_OTHERS_LIGHT))
			{
				flmultval = 0.5;
				break;
			}
		}
	}

	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		snd_active_t *psound = &m_activeSoundsArray[i];

		// Don't bother if sound is not active
		if(!psound->active)
			continue;

		// If muted, block out everything
		if(m_isMuted && !(psound->flags & (SND_FL_MUTEIGNORE|SND_FL_MENU)))
			psound->flags |= SND_FL_KILLME;

		// Kill dialouge if we fully entered water
		if((psound->flags & SND_FL_DIALOUGE) && pparams->waterlevel >= WATERLEVEL_FULL)
			psound->flags |= SND_FL_KILLME;

		// Do not play game sounds if we're not ingame
		if(!CL_CanPlayGameSounds() && !(psound->flags & SND_FL_MENU))
			psound->flags |= SND_FL_KILLME;

		// Protect against crash due to cache pointer being removed
		// at ResetEngine level
		if(!psound->pcache)
			psound->flags |= SND_FL_KILLME;

		// Do not apply fade effects on menu sounds
		if(!(psound->flags & SND_FL_MENU))
		{
			// Perform sound fade effects
			if(psound->volchangeduration)
			{
				if(psound->volchangetime == -1)
				{
					// Just beginning change
					psound->volchangetime = pparams->time;
					psound->prevvolume = psound->volume;
				}
				else
				{
					// See if we've exceeded the change time
					if(psound->volchangetime+psound->volchangeduration <= pparams->time)
					{
						// Clear these
						psound->volchangeduration = 0;
						psound->volchangetime = 0;
						psound->prevvolume = 0;

						// Set final volume
						psound->volume = psound->targetvolume;
						psound->targetvolume = 0;

						// Kill sound if volume is zero
						if(!psound->volume)
							psound->flags |= SND_FL_KILLME;
					}
					else
					{
						// Calculate blended volume
						Double frac = (pparams->time - psound->volchangetime)/psound->volchangeduration;
						psound->volume = psound->prevvolume*(1.0-frac) + psound->targetvolume*frac;
					}
				}
			}

			// Perform pitch change effects
			if(psound->pitchchangeduration)
			{
				if(psound->pitchchangetime == -1)
				{
					// Just beginning change
					psound->pitchchangetime = pparams->time;
					psound->prevpitch = psound->pitch;
				}
				else
				{
					// See if we've exceeded the change time
					if(psound->pitchchangetime+psound->pitchchangeduration <= pparams->time)
					{
						// Clear these
						psound->pitchchangeduration = 0;
						psound->pitchchangetime = 0;
						psound->prevpitch = 0;

						// Set final volume
						psound->pitch = psound->targetpitch;
						psound->targetpitch = 0;

						// Kill sound if pitch is zero
						if(!psound->pitch)
							psound->flags |= SND_FL_KILLME;
					}
					else
					{
						// Calculate blended volume
						Double frac = (pparams->time - psound->pitchchangetime)/psound->pitchchangeduration;
						psound->pitch = psound->prevpitch*(1.0-frac) + psound->targetpitch*frac;
					}
				}
			}
		}

		// Kill sounds flagged for death
		if(psound->flags & SND_FL_KILLME)
		{
			KillSound(*psound);
			continue;
		}

		// Set time if it's null
		if(psound->time == -1)
		{
			if(psound->flags & SND_FL_MENU)
				psound->time = ens.time;
			else
				psound->time = pparams->time;
		}

		// This will be used later on
		snd_cache_t *pcache = psound->pcache;
		Float pitch = clamp((psound->pitch/(Float)PITCH_NORM) * g_pCvarTimeScale->GetValue(), 0.5, 5.0);
		Int32 bytepersec = pcache->channels * (pcache->samplerate*pitch) * (pcache->bitspersample>>3);

		Double frametime;
		if(psound->flags & SND_FL_MENU)
			frametime = ens.frametime;
		else
			frametime = pparams->frametime;

		//
		// advance if not paused
		if(frametime && psound->pplaying)
		{
			// Get sound position from OpenAL
			alGetSourcei(psound->pplaying->sourceindex, AL_BYTE_OFFSET, &psound->datapos);

			// Take length cutoff into consideration
			Int32 endpos = pcache->length;
			if(psound->psentence && psound->pchunk->end)
				endpos = endpos*(static_cast<Float>(psound->pchunk->end)/100.0f);

			ALenum state;
			alGetSourcei(psound->pplaying->sourceindex, AL_SOURCE_STATE, &state);
			if(state == AL_STOPPED || (endpos <= psound->datapos))
			{
				if(psound->pchunk && psound->pchunk->pnext)
				{
					if(psound->pchunk->delay)
					{
						if(!psound->delaytime)
							psound->delaytime = pparams->time;

						if((psound->delaytime + psound->pchunk->delay) > pparams->time)
							continue;
					}

					if(psound->pplaying)
						RemovePlaying(*psound);

					psound->pchunk = psound->pchunk->pnext;
					psound->pitch = psound->mainpitch*(static_cast<Float>(psound->pchunk->pitch)/100.0f);
					psound->volume = psound->volume*(static_cast<Float>(psound->pchunk->volume)/100.0f);
					psound->datapos = pcache->length*(static_cast<Float>(psound->pchunk->start)/100.0f);

					CString strSound;
					strSound << SOUND_FOLDER_BASE_PATH << psound->psentence->folder << PATH_SLASH_CHAR << psound->pchunk->soundname << ".WAV";

					pcache = PrecacheSound(strSound.c_str(), -1, RS_GAME_LEVEL, true);
					if(!pcache)
					{
						KillSound(m_activeSoundsArray[i]);
						continue;
					}

					if(m_pCvarDebug->GetValue() >= 2)
						Con_Printf("%s - Playing chunk %s of sentence %s for entity %d.\n", __FUNCTION__,
						psound->pchunk->soundname.c_str(), psound->psentence->name.c_str(), psound->entindex);

					psound->pcache = pcache;
				}
				else
				{
					// over, kill it
					KillSound(*psound);
					continue;
				}
			}
		}
		else if(frametime)
		{
			Double playingTime; 
			if(psound->flags & SND_FL_MENU)
				playingTime = ens.time - psound->time;
			else
				playingTime = pparams->time - psound->time;

			// This non-playing, non-looped sound is already done, just remove it
			Int32 dataPosition = bytepersec*playingTime;
			if(dataPosition >= pcache->length && pcache->loopbegin == -1)
			{
				KillSound(*psound);
				continue;
			}
		}

		//
		// update origin and leaf for entity tied sounds
		if(psound->pentity && !(psound->flags & SND_FL_2D))
		{
			const cache_model_t *pmodel = psound->pentity->pmodel;
			if(pmodel && pmodel->type == MOD_BRUSH && psound->pentity->curstate.renderfx != RenderFx_SoundOrg)
			{	
				const brushmodel_t* pbrushmodel = pmodel->getBrushmodel();

				Vector vmins = psound->pentity->curstate.origin + pbrushmodel->mins;
				Vector vmaxs = psound->pentity->curstate.origin + pbrushmodel->maxs;
				Math::VectorScale((vmins+vmaxs), 0.5, psound->origin);
			}
			else if(psound->pentity == pplayer || psound->pentity == cls.dllfuncs.pfnGetViewModel())
			{
				Math::VectorCopy(pparams->v_origin, psound->origin);
			}
			else if(pmodel && pmodel->type == MOD_VBM)
			{
				if(!psound->psentence || !gVBMRenderer.GetBonePosition(psound->pentity, "Bip01 Head", psound->origin))
				{
					// get actual mins/maxs
					const vbmcache_t* pcachemdl = psound->pentity->pmodel->getVBMCache();
					if(pcachemdl->pstudiohdr)
					{
						if(psound->pentity->curstate.sequence >= pcachemdl->pstudiohdr->numseq)
							psound->pentity->curstate.sequence = pcachemdl->pstudiohdr->numseq;
			
						const mstudioseqdesc_t *pseqdesc = pcachemdl->pstudiohdr->getSequence(psound->pentity->curstate.sequence);
						Vector vmins = psound->pentity->curstate.origin + pseqdesc->bbmin;
						Vector vmaxs = psound->pentity->curstate.origin + pseqdesc->bbmax;
						Math::VectorScale((vmins+vmaxs), 0.5, psound->origin);
					}
					else
					{
						// If all else fails, go to default behavior
						Math::VectorCopy(psound->pentity->curstate.origin, psound->origin);
					}
				}
			}
			else
			{
				Math::VectorCopy(psound->pentity->curstate.origin, psound->origin);
			}
		}

		// Calculate mouth if needed
		if(psound->pentity && psound->pentity->curstate.msg_num == imsgnum)
		{
			if(psound->pentity->pmodel && psound->pentity->pmodel->type == MOD_VBM)
			{
				if(psound->channel == SND_CHAN_VOICE)
				{
					if(pcache->bitspersample == 16)
						CalcMouth16(*psound);
					else
						CalcMouth8(*psound);
				}
			}
		}

		if((psound->pentity != pplayer) && !(psound->flags & (SND_FL_2D|SND_FL_OCCLUSIONLESS)))
		{
			if(psound->pentity ||  psound->leafnum == 0)
			{
				// update leaf we're on
				const mleaf_t *pleaf = Mod_PointInLeaf(psound->origin, *ens.pworld);

				if(pleaf->contents != CONTENTS_SOLID)
					psound->leafnum = pleaf-ens.pworld->pleafs-1;
				else
					psound->leafnum = 0;
			}
		}

		if(psound->pplaying)
		{
			if(ShouldPlay(psound, pparams->v_origin, frametime, imsgnum, gameVolume))
			{
				if(!(psound->flags & SND_FL_2D))
				{
					// only update for non-2d sounds
					alSourcefv(psound->pplaying->sourceindex, AL_POSITION, psound->origin);
				}

				if(!(psound->flags & (SND_FL_REVERBLESS|SND_FL_MENU|SND_FL_RADIO)))
				{
					if(m_idealReverb)
						alSource3i(psound->pplaying->sourceindex, AL_AUXILIARY_SEND_FILTER, m_effectSlot, 0, 0);
					else
						alSource3i(psound->pplaying->sourceindex, AL_AUXILIARY_SEND_FILTER, AL_EFFECT_NULL, 0, 0);
				}

				Float flgain = CalcGain(pparams->v_origin, psound, flmultval, gameVolume);

				alSourcef(psound->pplaying->sourceindex, AL_GAIN, flgain);
				alSourcef(psound->pplaying->sourceindex, AL_PITCH, pitch);

				if(psound->pcache->loopbegin > 0)
				{
					Int32 processed;
					alGetSourcei(psound->pplaying->sourceindex, AL_BUFFERS_PROCESSED, &processed);
					while(processed--)
					{
						ALuint buffer;
						alSourceUnqueueBuffers(psound->pplaying->sourceindex, 1, &buffer);
						alSourcei(psound->pplaying->sourceindex, AL_LOOPING, AL_TRUE);
					}
				}
			}
			else
			{	
				RemovePlaying(*psound);
				continue;
			}
		}
		else if(ShouldPlay(psound, pparams->v_origin, frametime, imsgnum, gameVolume))
		{
			// Shouldn't happen
			if(!psound->pcache)
			{
				KillSound(m_activeSoundsArray[i]);
				continue;
			}

			// Try and find an empty slot
			snd_playing_t *pplaying = nullptr;
			for(Int32 j = 0; j < MAX_PLAYING_SOUNDS; j++)
			{
				if(!m_playingSoundsArray[j].psound)
				{
					pplaying = &m_playingSoundsArray[j];
					break;
				}
			}

			// Overflow
			if(!pplaying)
			{
				Con_Printf("%s - Sound system overflow!\n", __FUNCTION__);
				continue;
			}

			// Tie to sound
			psound->pplaying = pplaying;
			pplaying->psound = psound;

			alGenSources(1, &pplaying->sourceindex);

			alSourcei(pplaying->sourceindex, AL_MAX_DISTANCE, psound->radius);
			alSourcei(pplaying->sourceindex, AL_REFERENCE_DISTANCE, psound->radius*0.2);
			alSourcei(pplaying->sourceindex, AL_LOOPING, (pcache->loopbegin == 0) ? AL_TRUE : AL_FALSE);

			if(psound->pentity)
			{
				// Set velocity for doppler
				Vector velocity = psound->pentity->curstate.velocity;
				alSource3f(pplaying->sourceindex, AL_VELOCITY, velocity[0], velocity[1], velocity[2]); 
			}
			else
			{
				alSource3f(pplaying->sourceindex, AL_VELOCITY, 0, 0, 0); 
			}
			
			// Sync up other sources playing the same sound if it's an ambient sound
			if(pcache->loopbegin != -1 && psound->flags & SND_FL_AMBIENT)
			{
				// Try to find an already playing sound
				psound->datapos = GetSyncOffset(psound->pcache, i);
			}

			// If it wasn't set, recalculate the sound position
			if(psound->datapos == -1)
			{
				Double playingTime;
				if(!(psound->flags & SND_FL_MENU))
					playingTime = (pparams->time - psound->time) + psound->timeoffs;
				else
					playingTime = (ens.time - psound->time) + psound->timeoffs;

				// Calculate total data offset
				psound->datapos = bytepersec*playingTime;

				// Manage looped sounds specifically
				if(pcache->loopbegin > 0 && psound->datapos > pcache->loopbegin)
				{
					// Only a single buffer
					alSourceQueueBuffers(pplaying->sourceindex, 1, &psound->pcache->pbuffers[1]);

					Int32 newdatapos = psound->datapos - pcache->loopbegin;
					psound->datapos = newdatapos%(pcache->length-pcache->loopbegin);

					// We want the last buffer to loop
					alSourcei(psound->pplaying->sourceindex, AL_LOOPING, AL_TRUE);
				}
				else
				{
					// Just cap for sounds with a single buffer
					if(psound->datapos >= pcache->length)
					{
						if(pcache->loopbegin != -1)
						{
							// Rewind the sound
							psound->datapos = psound->datapos % pcache->length;
						}
						else
						{
							KillSound(*psound);
							continue;
						}
					}

					// Queue all buffers
					alSourceQueueBuffers(pplaying->sourceindex, psound->pcache->numbuffers, psound->pcache->pbuffers);
				}
			}
			else
			{
				// Queue all buffers
				alSourceQueueBuffers(pplaying->sourceindex, psound->pcache->numbuffers, psound->pcache->pbuffers);
			}

			// Set reverbe is not radio, menu or reverbless
			if(!(psound->flags & (SND_FL_REVERBLESS|SND_FL_RADIO|SND_FL_MENU)))
			{
				if(m_idealReverb != m_activeReverb)
				{
					if(m_idealReverb)
						alSource3i(psound->pplaying->sourceindex, AL_AUXILIARY_SEND_FILTER, m_effectSlot, 0, 0);
					else
						alSource3i(psound->pplaying->sourceindex, AL_AUXILIARY_SEND_FILTER, AL_EFFECT_NULL, 0, 0);
				}
			}

			// Set position and rolloff
			if(psound->flags & (SND_FL_2D|SND_FL_MENU))
			{
				alSourcei(pplaying->sourceindex, AL_ROLLOFF_FACTOR, 0);
				alSource3f(pplaying->sourceindex, AL_POSITION, 0, 0, 0);
				alSourcei(pplaying->sourceindex, AL_SOURCE_RELATIVE, AL_TRUE);
			}
			else
			{
				alSourcei(pplaying->sourceindex, AL_ROLLOFF_FACTOR, SE_ROLLOFF_FACTOR);
				alSourcefv(pplaying->sourceindex, AL_POSITION, psound->origin);
				alSourcei(pplaying->sourceindex, AL_SOURCE_RELATIVE, AL_FALSE);
			}

			Float flgain = CalcGain(pparams->v_origin, psound, flmultval, gameVolume);
			Float setpitch = clamp((psound->pitch/(Float)PITCH_NORM), 0.01, 5.0);

			alSourcef(pplaying->sourceindex, AL_GAIN, flgain);
			alSourcef(pplaying->sourceindex, AL_PITCH, setpitch);
			alSourcei(pplaying->sourceindex, AL_BYTE_OFFSET, psound->datapos);

			// Lastly tell it to play
			alSourcePlay(pplaying->sourceindex);
		}
	}

	//	
	// Set this at end
	//
	m_lastActiveReverb = m_idealReverb;
		
	//
	// Set up music
	//
	UpdateMusicPlayback(pparams, flmultval);
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::UpdateMusicPlayback( const ref_params_t* pparams, Float flmultval )
{
	if(m_musicTracksList.empty())
		return;

	m_musicTracksList.begin();
	while(!m_musicTracksList.end())
	{
		snd_music_t* ptrack = m_musicTracksList.get();
		if(!UpdateMusicTrackPlayback((*ptrack), pparams, flmultval))
		{
			alSourceStop(ptrack->source);
			alDeleteSources(1, &ptrack->source);
			alDeleteBuffers(2, ptrack->buffers);
			ov_clear(&ptrack->stream);
			delete ptrack;

			m_musicTracksList.remove(m_musicTracksList.get_link());
			m_musicTracksList.next();
			continue;
		}

		m_musicTracksList.next();
	}
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::UpdateMusicTrackPlayback ( snd_music_t& track, const ref_params_t* pparams, Float flmultval )
{
	// Prevent music playback while loading
	if(ens.isloading || !gWindow.IsActive())
	{
		if(track.source)
		{
			ALenum state;
			alGetSourcei(track.source, AL_SOURCE_STATE, &state);
			if(state != AL_PAUSED && state != AL_STOPPED)
				alSourcePause(track.source);
		}

		return true;
	}

	if(!track.source)
	{
		track.starttime = (track.flags & OGG_FL_MENU) ? ens.time : cls.cl_time;
		track.info = ov_info(&track.stream, -1);
		if(track.info->channels == 1)
			track.format = AL_FORMAT_MONO16;
		else
			track.format = AL_FORMAT_STEREO16;
		
		// allocate source and buffers
		alGenSources(1, &track.source);
		alGenBuffers(2, track.buffers);

		alSource3f(track.source, AL_POSITION, 0.0, 0.0, 0.0);
		alSource3f(track.source, AL_VELOCITY, 0.0, 0.0, 0.0);
		alSource3f(track.source, AL_DIRECTION, 0.0, 0.0, 0.0);
		alSourcef(track.source, AL_ROLLOFF_FACTOR, 0.0);
		alSourcei(track.source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSourcei(track.source, AL_LOOPING, AL_FALSE);
		
		stream_result_t streamResult = Stream(track.buffers[0], track);
		if(streamResult != STREAM_OK)
			return false;

		streamResult = Stream(track.buffers[1], track);
		if(streamResult == STREAM_ERROR)
			return false;

		alSourceQueueBuffers(track.source, (streamResult != STREAM_EOF) ? 2 : 1, track.buffers);
		alSourcePlay(track.source);
	}

	Double time = (track.flags & OGG_FL_MENU) ? ens.time : cls.cl_time;

	Float musicVolume = 1.0;
	if(track.fadeouttime != 0 && track.fadeoutduration > 0)
	{
		if(track.fadeouttime == -1)
			track.fadeouttime = time;
		else if((track.fadeouttime + track.fadeoutduration) <= time)
			return false;

		Float frac = (time - track.fadeouttime)/track.fadeoutduration;
		musicVolume *= (1.0 - frac);
	}
	else if(track.fadeinduration > 0 && (track.starttime + track.fadeinduration) > time)
	{
		Float frac = (time - track.starttime)/track.fadeinduration;
		musicVolume *= frac;
	}

	Float volume = 1.0;
	if(track.unpausefadebegin && track.unpausefadetime > 0)
	{
		Double reftime = (track.flags & OGG_FL_MENU) ? ens.time : cls.cl_time;

		if(track.unpausefadebegin == -1)
		{
			// Begin fading out
			track.unpausefadebegin = reftime;
		}
		else if((track.unpausefadebegin + track.unpausefadetime) > reftime)
		{
			// Calculate new volume
			volume = (ens.time - track.unpausefadebegin) / track.unpausefadetime;
		}
		else
		{
			track.unpausefadebegin = 0;
			track.unpausefadetime = 0;
		}
	}

	alSourcef(track.source, AL_GAIN, m_pCVarMusicVolume->GetValue()*flmultval*musicVolume*volume);

	ALenum state;
	alGetSourcei(track.source, AL_SOURCE_STATE, &state);

	if(track.channel != MUSIC_CHANNEL_MENU && pparams->paused)
	{
		if(state != AL_PAUSED)
			alSourcePause(track.source);

		return true;
	}
	else if(track.paused)
	{
		if(track.unpausefadebegin)
		{
			Double reftime = (track.flags & OGG_FL_MENU) ? ens.time : cls.cl_time;
			if((track.unpausefadebegin + track.unpausefadetime) < reftime)
			{
				alSourcePause(track.source);
				return true;
			}
		}
		else
		{
			alSourcePause(track.source);
			return true;
		}
	}

	if(state == AL_PAUSED)
	{
		alSourcePlay(track.source);
	}
	else if(state == AL_STOPPED)
	{
		stream_result_t streamResult = STREAM_OK;

		Int32 processed;
		alGetSourcei(track.source, AL_BUFFERS_PROCESSED, &processed);
		while(processed--)
		{
			ALuint buffer;
			alSourceUnqueueBuffers(track.source, 1, &buffer);

			streamResult = Stream(buffer, track);
			if(streamResult == STREAM_OK)
				alSourceQueueBuffers(track.source, 1, &buffer);
			else
				break;
		}

		if(streamResult != STREAM_OK)
		{
			if(streamResult == STREAM_EOF)
			{
				if(track.flags & OGG_FL_LOOP)
				{
					// Reset seek to beginning
					ov_raw_seek(&track.stream, 0);

					streamResult = Stream(track.buffers[0], track);
					if(streamResult != STREAM_OK)
						return false;

					streamResult = Stream(track.buffers[1], track);
					if(streamResult == STREAM_ERROR)
						return false;

					alSourceQueueBuffers(track.source, (streamResult != STREAM_EOF) ? 2 : 1, track.buffers);
					alSourcePlay(track.source);
				}
				else
				{
					// Just shut down the playback
					return false;
				}
			}
			else
			{
				// Encountered an error
				return false;
			}
		}
		else
		{
			alSourcePlay(track.source);
			return true;
		}
	}

	Int32 processed;
	alGetSourcei(track.source, AL_BUFFERS_PROCESSED, &processed);
	while(processed--)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(track.source, 1, &buffer);
		stream_result_t streamResult = Stream(buffer, track);

		if(streamResult == STREAM_OK)
			alSourceQueueBuffers(track.source, 1, &buffer);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::StopAllPlaying( void )
{
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(m_activeSoundsArray[i].pplaying)
			RemovePlaying(m_activeSoundsArray[i]);
	}

	if(!m_musicTracksList.empty())
	{
		m_musicTracksList.begin();
		while(!m_musicTracksList.end())
		{
			snd_music_t* ptrack = m_musicTracksList.get();

			ALenum state;
			alGetSourcei(ptrack->source, AL_SOURCE_STATE, &state);
			if(state == AL_PLAYING)
				alSourcePause(ptrack->source);

			m_musicTracksList.next();
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::CacheMessage( const Vector* pOrigin, Int32 svindex, Int32 channel, Int32 flags, entindex_t entindex, Int32 pitch, Float vol, Float attn, bool isambient, Float timeoffs )
{
	snd_cache_t *pcache = nullptr;
	if(svindex >= 0)
	{
		pcache = PrecacheSound(nullptr, svindex, RS_GAME_LEVEL, false);
		if(!pcache)
			return;
	}

	// If it's stop, clear all previous sounds
	if(flags & SND_FL_STOP)
	{
		m_msgCacheList.begin();
		while(!m_msgCacheList.end())
		{
			s_msg_cache_t* pmsg = m_msgCacheList.get();

			if(pmsg->channel == channel 
				&& pmsg->entindex == entindex
				&& pmsg->svindex == svindex)
			{
				// Remove it
				delete m_msgCacheList.get();
				m_msgCacheList.remove(m_msgCacheList.get_link());
			}

			m_msgCacheList.next();
		}
	}
	else
	{
		m_updateMsgCacheList.begin();
		while(!m_updateMsgCacheList.end())
		{
			s_msg_cache_t* pmsg = m_updateMsgCacheList.get();

			if(pmsg->channel == channel 
				&& pmsg->entindex == entindex
				&& pmsg->svindex == svindex
				&& (pmsg->flags & SND_FL_STOP))
			{
				// Remove it
				delete m_updateMsgCacheList.get();
				m_updateMsgCacheList.remove(m_updateMsgCacheList.get_link());
			}

			m_updateMsgCacheList.next();
		}
	}

	// Determine type
	snd_cachetype_t type = isambient ? SND_CT_PLAYAMBIENTSOUND : SND_CT_PLAYENTITYSOUND;

	// Determine sound duration
	Float soundduration = -1;
	if(pcache && pcache->loopbegin == -1)
	{
		Float soundpitch = clamp((pitch/(Float)PITCH_NORM), 0.5, 5.0);
		Int32 bytepersec = pcache->channels * (pcache->samplerate*soundpitch) * (pcache->bitspersample>>3);
		soundduration = static_cast<Float>(pcache->length)/ static_cast<Float>(bytepersec);
	}
	else if(svindex < 0)
	{
		Int32 sentenceindex = SDL_abs(svindex) - 1;
		soundduration = m_pSentencesFile->GetSentenceDuration(sentenceindex);
	}

	if(!soundduration)
		return;

	// See if we can update any existing entries
	m_msgCacheList.begin();
	while(!m_msgCacheList.end())
	{
		s_msg_cache_t* pcheckmsg = m_msgCacheList.get();

		if( pcheckmsg->channel == channel 
			&& pcheckmsg->entindex == entindex
			&& pcheckmsg->svindex == svindex
			&& pcheckmsg->flags == flags
			&& pcheckmsg->type == type)
		{
			// Update the sound
			pcheckmsg->atten = attn;
			pcheckmsg->channel = channel;
			pcheckmsg->flags = flags;
			pcheckmsg->entindex = entindex;
			pcheckmsg->pitch = pitch;
			pcheckmsg->volume = vol;
			pcheckmsg->svindex = svindex;
			pcheckmsg->timeoffs = timeoffs;
			pcheckmsg->cachetime = cls.cl_time;
			pcheckmsg->duration = soundduration;
			pcheckmsg->looping = (pcache && pcache->loopbegin != -1) ?  true : false;

			if(pOrigin)
				Math::VectorCopy((*pOrigin), pcheckmsg->origin);
			return;
		}

		m_msgCacheList.next();
	}

	s_msg_cache_t *pmsg = nullptr;
	if(flags & (SND_FL_STOP|SND_FL_CHANGE_PITCH|SND_FL_CHANGE_VOLUME))
	{
		pmsg = new s_msg_cache_t;
		m_updateMsgCacheList.radd(pmsg);
	}
	else
	{
		pmsg = new s_msg_cache_t;
		m_msgCacheList.radd(pmsg);
	}

	// add to cache
	pmsg->atten = attn;
	pmsg->channel = channel;
	pmsg->flags = flags;
	pmsg->entindex = entindex;
	pmsg->pitch = pitch;
	pmsg->volume = vol;
	pmsg->type = type;
	pmsg->svindex = svindex;
	pmsg->timeoffs = timeoffs;
	pmsg->cachetime = cls.cl_time;
	pmsg->duration = soundduration;
	pmsg->looping = (pcache && pcache->loopbegin != -1) ?  true : false;

	if(pOrigin)
		Math::VectorCopy((*pOrigin), pmsg->origin);
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::CacheEffect( entindex_t entindex, Int32 svindex, Int32 channel, snd_effects_t effect, Float duration, Float targetvalue )
{
	snd_cache_t *pcache = PrecacheSound(nullptr, svindex, RS_GAME_LEVEL, false);
	if(pcache)
	{
		if(pcache->loopbegin == -1)
			return;
	}

	m_msgCacheList.begin();
	while(!m_msgCacheList.end())
	{
		s_msg_cache_t* pcheckmsg = m_msgCacheList.get();

		if(pcheckmsg->channel == channel 
			&& pcheckmsg->entindex == entindex
			&& pcheckmsg->svindex == svindex
			&& pcheckmsg->effect == effect
			&& pcheckmsg->type == SND_CT_APPLYEFFECT)
		{
			pcheckmsg->effectduration = duration;
			pcheckmsg->targetvalue = targetvalue;
			return;
		}

		m_msgCacheList.next();
	}

	s_msg_cache_t* pnewmsg = new s_msg_cache_t;
	m_msgCacheList.radd(pnewmsg);

	pnewmsg->entindex = entindex;
	pnewmsg->svindex = svindex;
	pnewmsg->channel = channel;
	pnewmsg->effect = effect;
	pnewmsg->effectduration = duration;
	pnewmsg->targetvalue = targetvalue;
	pnewmsg->type = SND_CT_APPLYEFFECT;
	pnewmsg->cachetime = cls.cl_time;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::PlayOgg( const Char *sample, Int32 channel, Float timeOffset, Int32 flags, Float fadeInTime )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	m_musicTracksList.begin();
	while(!m_musicTracksList.end())
	{
		snd_music_t* ptrack = m_musicTracksList.get();
		if(ptrack->channel == channel)
		{
			alSourceStop(ptrack->source);
			alDeleteSources(1, &ptrack->source);
			alDeleteBuffers(2, ptrack->buffers);
			ov_clear(&ptrack->stream);
			delete ptrack;

			m_musicTracksList.remove(m_musicTracksList.get_link());
			m_musicTracksList.next();
			continue;
		}

		m_musicTracksList.next();
	}

	if(flags & OGG_FL_STOP)
		return;

	if(!sample || sample[0] == 0)
		return;

	CString filepath;
	filepath << SOUND_FOLDER_BASE_PATH << sample;

	snd_oggcache_t *pfile = PrecacheOgg(filepath.c_str(), RS_GAME_LEVEL);
	if(!pfile)
		return;

	// Make sure this gets reset
	pfile->pcurptr = pfile->pfileptr;

	// Add new track to list
	snd_music_t* ptrack = new snd_music_t;
	ptrack->filename = sample;
	ptrack->pfile = pfile;
	ptrack->flags = flags;
	ptrack->fadeinduration = fadeInTime;
	ptrack->channel = channel;
	
	if(ov_open_callbacks(pfile, &ptrack->stream, nullptr, 0, m_oggCallbacks) < 0)
	{
		Con_Printf("%s - Decode error on music track '%s'.\n", __FUNCTION__, sample);
		delete ptrack;
		return;
	}

	// If we have a time offset, seek the position
	if(timeOffset > 0)
	{
		Double duration = ov_time_total(&ptrack->stream, -1);

		// Make sure seek works
		if(duration == OV_EINVAL)
		{
			Con_Printf("%s - Decode error on music track '%s'.\n", __FUNCTION__, sample);
			ov_clear(&ptrack->stream);
			delete ptrack;
			return;
		}
	
		// Account for looping
		Float _timeOffset = timeOffset;
		if((ptrack->flags & OGG_FL_LOOP) && _timeOffset > duration)
			_timeOffset -= SDL_floor(_timeOffset/duration) * duration;

		// Make sure position is valid
		if(_timeOffset > duration)
		{
			Con_Printf("%s - Bad time offset %f on file '%s' with duration %.2f.\n", __FUNCTION__, _timeOffset, sample, duration);
			ov_clear(&ptrack->stream);
			delete ptrack;
			return;
		}

		// Seek the offset position
		if(ov_time_seek(&ptrack->stream, _timeOffset) < 0)
		{
			Con_Printf("%s - Bad seek on file '%s' with offset %.2f.\n", __FUNCTION__, sample, _timeOffset);
			ov_clear(&ptrack->stream);
			delete ptrack;
			return;
		}
	}

	// Add to list
	m_musicTracksList.add(ptrack);
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::StopOggFade( const Char *sample, Int32 channel, Float fadeTime )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	if(m_musicTracksList.empty())
		return;

	snd_music_t* ptrack = nullptr;

	m_musicTracksList.begin();
	while(!m_musicTracksList.end())
	{
		snd_music_t* _ptrack = m_musicTracksList.get();
		if(_ptrack->channel == channel)
		{
			ptrack = _ptrack;
			break;
		}

		m_musicTracksList.next();
	}

	if(!ptrack)
		return;

	if(fadeTime > 0)
	{
		if(qstrcmp(ptrack->filename, sample))
		{
			Con_EPrintf("%s - Called with incorrect file. Currently playing: '%s', file specified: '%s'.\n", __FUNCTION__, ptrack->filename.c_str(), sample);
			return;
		}

		// Music will fade out over time
		ptrack->fadeoutduration = fadeTime;
		ptrack->fadeouttime = -1;
	}
	else
	{
		alSourceStop(ptrack->source);
		alDeleteSources(1, &ptrack->source);
		alDeleteBuffers(2, ptrack->buffers);
		ov_clear(&ptrack->stream);
		
		m_musicTracksList.remove(ptrack);
		delete ptrack;
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::StopOgg( Int32 channel, bool menuAlso )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	if(m_musicTracksList.empty())
		return;

	m_musicTracksList.begin();
	while(!m_musicTracksList.end())
	{
		snd_music_t* ptrack = m_musicTracksList.get();
		if((channel == MUSIC_CHANNEL_ALL 
			&& (ptrack->channel != MUSIC_CHANNEL_MENU || menuAlso)) 
			|| ptrack->channel == channel)
		{
			alSourceStop(ptrack->source);
			alDeleteSources(1, &ptrack->source);
			alDeleteBuffers(2, ptrack->buffers);
			ov_clear(&ptrack->stream);
			delete ptrack;

			m_musicTracksList.remove(m_musicTracksList.get_link());
			m_musicTracksList.next();
			continue;
		}

		m_musicTracksList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::PauseOggChannel( Int32 channel )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	if(m_musicTracksList.empty())
		return;

	m_musicTracksList.begin();
	while(!m_musicTracksList.end())
	{
		snd_music_t* ptrack = m_musicTracksList.get();
		if(channel == MUSIC_CHANNEL_ALL || ptrack->channel == channel)
		{
			alSourcePause(ptrack->source);
			ptrack->paused = true;
			ptrack->unpausefadebegin = 0;
			ptrack->unpausefadetime = 0;
		}

		m_musicTracksList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::UnPauseOggChannel( Int32 channel, Float fadetime )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	if(m_musicTracksList.empty())
		return;

	m_musicTracksList.begin();
	while(!m_musicTracksList.end())
	{
		snd_music_t* ptrack = m_musicTracksList.get();
		if(channel == MUSIC_CHANNEL_ALL || ptrack->channel == channel)
		{
			ptrack->paused = false;

			if(fadetime > 0)
			{
				ptrack->unpausefadetime = fadetime;
				ptrack->unpausefadebegin = -1;
			}
		}
		m_musicTracksList.next();
	}
}

//=============================================
// @brief
//
//=============================================
snd_oggcache_t* CSoundEngine::PrecacheOgg( const Char *sample, rs_level_t level )
{
	m_cachedOggFilesList.begin();
	while(!m_cachedOggFilesList.end())
	{
		snd_oggcache_t* pcache = m_cachedOggFilesList.get();
		if(!qstrcmp(pcache->filepath, sample))
			return pcache;

		m_cachedOggFilesList.next();
	}

	Uint32 fileSize = 0;
	const byte *pFile = FL_LoadFile(sample, &fileSize);
	if(!pFile)
	{
		Con_EPrintf("Failed to load %s\n", sample);
		return nullptr;
	}

	snd_oggcache_t* pNew = new snd_oggcache_t();
	m_cachedOggFilesList.add(pNew);

	// Copy data
	pNew->pfileptr = new byte[fileSize];
	memcpy(pNew->pfileptr, pFile, sizeof(byte)*fileSize);
	FL_FreeFile(pFile);

	pNew->level = level;
	pNew->filepath = sample;
	pNew->pcurptr = pNew->pfileptr;
	pNew->filesize = fileSize;

	return pNew;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::StopSound( entindex_t entindex, Int32 channel )
{
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(!m_activeSoundsArray[i].active)
			continue;

		if(m_activeSoundsArray[i].entindex == entindex && m_activeSoundsArray[i].channel == channel)
		{
			KillSound(m_activeSoundsArray[i]);
			break;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::ApplySoundEffect( entindex_t entindex, Int32 svindex, Int32 channel, snd_effects_t effect, Float duration, Float targetvalue )
{
	const Char* pstrSoundFile = GetSoundFileForServerIndex(svindex);
	if(!pstrSoundFile)
	{
		Con_Printf("%s - Couldn't find sound file with server index '%d'.\n", __FUNCTION__, svindex);
		return;
	}

	ApplySoundEffect(entindex, pstrSoundFile, channel, effect, duration, targetvalue);
}

//=============================================
// @brief
//
//=============================================
const Char* CSoundEngine::GetSoundFileForServerIndex( Int32 serverindex )
{
	m_cachedSoundsList.begin();
	while(!m_cachedSoundsList.end())
	{
		snd_cache_t* pcache = m_cachedSoundsList.get();
		if(pcache->svindex == serverindex)
			return pcache->name.c_str();

		m_cachedSoundsList.next();
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::IsSoundPlaying( entindex_t entindex, Int32 svindex, Int32 channel )
{
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		snd_active_t& snd = m_activeSoundsArray[i];
		if(!snd.active || (snd.flags & SND_FL_KILLME))
			continue;

		if(snd.entindex == entindex && snd.channel == channel && snd.pcache->svindex == svindex)
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::IsSoundPlaying( entindex_t entindex, const Char *sample, Int32 channel )
{
	// Build filepath
	CString filepath;
	if(!qstrstr(sample, SOUND_FOLDER_BASE_PATH) && !qstrstr(sample, "sound\\"))
		filepath << SOUND_FOLDER_BASE_PATH;

	filepath << sample;

	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		snd_active_t& snd = m_activeSoundsArray[i];
		if(!snd.active)
			continue;

		if(snd.entindex == entindex && snd.channel == channel && !qstrcmp(snd.pcache->name, filepath))
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::ApplySoundEffect( entindex_t entindex, const Char *sample, Int32 channel, snd_effects_t effect, Float duration, Float targetvalue )
{
	if(!entindex || entindex == NO_ENTITY_INDEX)
	{
		Con_Printf("%s - Invalid entity index %d for sound '%s'.\n", __FUNCTION__, entindex, sample);
		return;
	}

	if(!duration)
	{
		Con_Printf("%s - No duration specified for entity index %d with sound '%s'.\n", __FUNCTION__, entindex, sample);
		return;
	}

	// Build filepath
	CString filepath;
	if(!qstrstr(sample, SOUND_FOLDER_BASE_PATH) && !qstrstr(sample, "sound\\"))
		filepath << SOUND_FOLDER_BASE_PATH;

	filepath << sample;

	// Find the sound with the specified file and entindex
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		snd_active_t& snd = m_activeSoundsArray[i];
		if(!snd.active || (snd.flags & SND_FL_KILLME))
			continue;

		if(snd.entindex == entindex && snd.channel == channel && !qstrcmp(snd.pcache->name, filepath))
		{
			switch(effect)
			{
			case SND_EF_CHANGE_VOLUME:
				{
					snd.volchangetime = -1;
					snd.volchangeduration = duration;
					snd.targetvolume = targetvalue;
				}
				break;
			case SND_EF_CHANGE_PITCH:
				{
					snd.pitchchangetime = -1;
					snd.pitchchangeduration = duration;
					snd.targetpitch = clamp(targetvalue, MIN_PITCH, MAX_PITCH);
				}
				break;
			default:
				Con_Printf("%s - Unknown effect type '%d' specified for sound '%s'.\n", __FUNCTION__, static_cast<Int32>(effect), sample);
				break;
			}

			break;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::FreeEntity( entindex_t entindex )
{
	for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
	{
		if(!m_activeSoundsArray[i].active)
			continue;

		// Protect against crash due to cache pointer being removed
		// at ResetEngine level
		if(!m_activeSoundsArray[i].pcache)
			continue;

		// Only remove looping sounds
		if(m_activeSoundsArray[i].pcache->loopbegin == -1)
			continue;

		if(m_activeSoundsArray[i].entindex == entindex)
			m_activeSoundsArray[i].flags |= SND_FL_KILLME;
	}
}

//=============================================
// @brief
//
//=============================================
bool CSoundEngine::LoadSentences( void )
{
	const byte* pfile = FL_LoadFile(SENTENCES_FILE_PATH);
	if(!pfile)
	{
		Con_EPrintf("%s - Could not load '%s'.\n", __FUNCTION__, SENTENCES_FILE_PATH);
		return false;
	}

	if(!m_pSentencesFile)
		m_pSentencesFile = new CSentencesFile(Sentences_PrecacheSound, Sentences_GetSoundDuration);

	// Process the file contents
	bool result = m_pSentencesFile->Init(pfile);

	// Free file
	FL_FreeFile(pfile);

	// Print any warnings
	Uint32 warningNb = m_pSentencesFile->GetNbWarnings();
	for(Uint32 i = 0; i < warningNb; i++)
		Con_Printf(m_pSentencesFile->GetWarning(i));

	if(!result)
	{
		Con_EPrintf("%s - Failed to init sentences.\n", __FUNCTION__);
		delete m_pSentencesFile;
		m_pSentencesFile = nullptr;
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::SetActiveReverb( Int32 reverb )
{
	m_activeReverb = reverb;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::SetMuted( bool muted )
{
	m_isMuted = muted;
}

//=============================================
// @brief
//
//=============================================
void CSoundEngine::SetPauseAllSounds( bool paused )
{
	m_allSoundsPaused = paused;

	if(m_allSoundsPaused)
	{
		for(Uint32 i = 0; i < m_activeSoundsArray.size(); i++)
		{
			if(m_activeSoundsArray[i].flags & SND_FL_MENU)
				continue;

			if(m_activeSoundsArray[i].pplaying)
				RemovePlaying(m_activeSoundsArray[i]);
		}
	}
}

//=============================================
// @brief Returns the duration of a sentence
//
//=============================================
const CSentencesFile::sentence_t* CSoundEngine::GetSentence( Int32 index ) const
{
	if(!m_pSentencesFile)
		return nullptr;

	return m_pSentencesFile->GetSentenceDefinition(index);
}

//=============================================
// @brief Set HRTF command
//
//=============================================
void CSoundEngine::SetHRTFCommand( void )
{
	if(gCommands.Cmd_Argc() <= 1)
	{
		Con_Printf("s_sethrtf usage - s_sethrtf <0 or 1 for disable/enable>.\n");
		return;
	}

	bool enable = SDL_atoi(gCommands.Cmd_Argv(1)) == 0 ? false : true;

	conf_group_t* pGroup = gConfig.FindGroup(SOUNDENGINE_CONFIG_GRP_NAME);
	if(!pGroup)
		pGroup = gConfig.CreateGroup(SOUNDENGINE_CONFIG_GRP_NAME, CConfig::SYSTEM_CONFIG_FILENAME, CONF_GRP_SYSTEM);

	gConfig.SetValue(pGroup, SOUNDENGINE_HRTF_SETTING_NAME, static_cast<Int32>(enable), true);
	if(m_isHRTFEnabled != enable)
		Con_Printf("HRTF setting will only take effect after application restart.\n");
	else
		Con_Printf("HRTF setting not changed.\n");
}

//=============================================
// @brief Precache function for sentences file
//
//=============================================
Int32 Sentences_PrecacheSound( const Char* pstrFilename )
{
	if(pstrFilename[0] == '!')
	{
		if(gSoundEngine.PrecacheServerSound(pstrFilename, NO_POSITION))
			return 0; // Dummy value
		else
			return NO_PRECACHE;
	}
	else
	{
		if(gSoundEngine.PrecacheSound(pstrFilename, NO_POSITION, RS_GAME_LEVEL, false))
			return 0; // Dummy value
		else
			return NO_PRECACHE;
	}
}

//=============================================
// @brief GetDuration function for sentences file
//
//=============================================
Float Sentences_GetSoundDuration( const Char* pstrFilename, Uint32 pitch )
{
	if(pstrFilename[0] == '!')
	{
		Int32 index = SDL_atoi(pstrFilename + 1);
		const CSentencesFile::sentence_t* psentence = gSoundEngine.GetSentence(index);

		Float duration = 0;
		for(Uint32 i = 0; i < psentence->chunks.size(); i++)
		{
			const CSentencesFile::sent_chunk_t* pchunk = psentence->chunks[i];
			CString filepath;
			filepath << SOUND_FOLDER_BASE_PATH << psentence->folder << PATH_SLASH_CHAR << pchunk->soundname << ".wav";

			CSoundEngine::snd_cache_t* pcache = gSoundEngine.PrecacheSound(pchunk->soundname.c_str(), NO_POSITION, RS_GAME_LEVEL, false);
			if(pcache)
			{
				Float soundpitch = clamp((pitch/(Float)PITCH_NORM), 0.5, 5.0);
				Int32 bytepersec = pcache->channels * (pcache->samplerate*soundpitch) * (pcache->bitspersample>>3);
				duration += static_cast<Float>(pcache->length)/static_cast<Float>(bytepersec);
			}
		}

		return duration;
	}
	else
	{
		CSoundEngine::snd_cache_t* pcache = gSoundEngine.PrecacheSound(pstrFilename, NO_POSITION, RS_GAME_LEVEL, false);
		if(!pcache)
			return 0;

		Float soundpitch = clamp((pitch/(Float)PITCH_NORM), 0.5, 5.0);
		Int32 bytepersec = pcache->channels * (pcache->samplerate*soundpitch) * (pcache->bitspersample>>3);
		return static_cast<Float>(pcache->length)/ static_cast<Float>(bytepersec);
	}
}
