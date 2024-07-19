/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"

#include "player.h"
#include "constants.h"
#include "snd_shared.h"
#include "net_shared.h"
#include "gameui_shared.h"
#include "util.h"
#include "buttonbits.h"
#include "ladder_shared.h"
#include "bike_shared.h"
#include "screenfade.h"

#include "triggersubwaycontroller.h"
#include "triggerkeypad.h"
#include "triggerlogin.h"
#include "playerweapon.h"
#include "envladder.h"
#include "itemmotorbike.h"
#include "triggercameramodel.h"

#include "vcontroller_shared.h"
#include "msgreader.h"
#include "game.h"
#include "constants.h"

#include "ai_nodegraph.h"
#include "ai_sounds.h"
#include "triggercameramodel.h"
#include "itemdiary.h"

// Player punch treshold for falling
const Float CPlayerEntity::PLAYER_FALL_VELOCITY_PUNCH_MIN = 350.0f;
// Player punch treshold for falling
const Float CPlayerEntity::PLAYER_FALL_VELOCITY_SAFE_LIMIT = 600.0f;
// Player punch treshold for falling
const Float CPlayerEntity::PLAYER_FALL_VELOCITY_FATAL = 1024.0f;
// Player punch treshold for falling
const Float CPlayerEntity::DAMAGE_FALL_FOR_VELOCITY = 100.0f/(CPlayerEntity::PLAYER_FALL_VELOCITY_FATAL-CPlayerEntity::PLAYER_FALL_VELOCITY_SAFE_LIMIT);
// Player falling velocity limit for producing audible sounds to NPCs
const Float CPlayerEntity::PLAYER_FALL_SOUND_LIMIT = 128;
// Player heal time
const Double CPlayerEntity::PLAYER_HEAL_TIME		= 2.5;
// Player armor damage absorption
const Float CPlayerEntity::PLAYER_ARMOR_DMG_ABSORB	= 0.6;
// Player armor damage drain
const Float CPlayerEntity::PLAYER_ARMOR_DMG_DRAIN	= 0.9;
// Player use radius
const Float CPlayerEntity::PLAYER_USE_RADIUS = 64.0f;
// Color of screen fade when hurt
const color24_t CPlayerEntity::PAIN_SCREENFADE_COLOR = color24_t(170, 4, 4);
// Time based damage delay
const Float CPlayerEntity::DROWNRECOVER_HEAL_DELAY = 2.0;
// Time based damage delay
const Float CPlayerEntity::DROWNRECOVER_MAX_HEAL = 10;
// Time based damage delay
const Float CPlayerEntity::TIMEBASED_DMG_DELAY	= 0.5;
// Time based dmg type-dmg bit associations
const Int32 CPlayerEntity::TIMEBASED_DMG_BITS[NUM_TIMEBASED_DMG] = 
{
	DMG_RADIATION,
	DMG_DROWNRECOVER,
	DMG_ACID,
	DMG_BURN,
	DMG_FREEZE
};

// Time based dmg type-dmg durations
const Float CPlayerEntity::TIMEBASED_DMG_DURATIONS[NUM_TIMEBASED_DMG] = 
{
	2,		//DMG_RADIATION
	0,		//DMG_DROWNRECOVER - Handled specially
	4,		//DMG_ACID
	2,		//DMG_BURN
	2		//DMG_FREEZE
};

// Time based dmg type-damage values
const Float CPlayerEntity::TIMEBASED_DMG_AMOUNTS[NUM_TIMEBASED_DMG] = 
{
	1.0,	//DMG_RADIATION
	0.0,	//DMG_DROWNRECOVER - Handled specially
	1.0,	//DMG_ACID
	1.0,	//DMG_BURN
	1.0		//DMG_FREEZE
};

// Flashlight toggle sound
const Char CPlayerEntity::FLASHLIGHT_TOGGLE_SOUND[] = "items/flashlight1.wav";
// Flashlight drain time
const Float CPlayerEntity::FLASHLIGHT_DRAIN_TIME = 1.2;
// Flashlight impulse value
const Int32 CPlayerEntity::FLASHLIGHT_IMPULSE_VALUE = 100;
// Player give weapons cheat
const Int32 CPlayerEntity::PLAYER_CHEATCODE_ALLWEAPONS = 101;
// Texture name tracing impulse code
const Int32 CPlayerEntity::PLAYER_CHEATCODE_TRACE_TEXTURE = 102;
// Report AI state impulse code
const Int32 CPlayerEntity::PLAYER_CHEATCODE_REPORT_AI_STATE = 103;
// Player dump globals cheat
const Int32 CPlayerEntity::PLAYER_CHEATCODE_DUMPGLOBALS = 105;
// Dump all codes cheat code
const Int32 CPlayerEntity::PLAYER_CHEATCODE_DUMP_CODES = 106;
// Get nearest node's index
const Int32 CPlayerEntity::PLAYER_CHEATCODE_GET_NEAREST_NODE_INDEX = 185;
// Show small node mins/maxs for nearest node
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_SMALL_HULL_NEAREST_NODE_MINS_MAXS = 186;
// Show fly hull node mins/maxs for nearest node
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_FLY_HULL_NEAREST_NODE_MINS_MAXS = 187;
// Show large hull node mins/maxs for nearest node
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_LARGE_HULL_NEAREST_NODE_MINS_MAXS = 188;
// Show human hull node mins/maxs for nearest node
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NEAREST_NODE_MINS_MAXS = 189;
// Show small node mins/maxs
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_MINS_MAXS = 190;
// Show fly hull node mins/maxs
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_MINS_MAXS = 191;
// Show large hull node mins/maxs
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_MINS_MAXS = 192;
// Show human hull node mins/maxs
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_MINS_MAXS = 193;
// Show small node paths
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_PATHS = 194;
// Show fly hull node paths
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_PATHS = 195;
// Show large hull node paths
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_PATHS = 196;
// Show human hull node paths
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_PATHS = 197;
// Show all node paths
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_ALL_NODE_PATHS = 198;
// Show nearest node connections
const Int32 CPlayerEntity::PLAYER_CHEATCODE_SHOW_NEAREST_NODE_CONNECTIONS = 199;
// Remove entity cheat code
const Int32 CPlayerEntity::PLAYER_CHEATCODE_REMOVE_ENTITY = 203;
// Grant all subway stops
const Int32 CPlayerEntity::PLAYER_CHEATCODE_GRANT_ALL_SUBWAY_STOPS = 204;
// Name entity in front
const Int32 CPlayerEntity::PLAYER_CHEATCODE_NAME_ENTITY_IN_FRONT = 205;

// Cheat codes and their descriptions
const CPlayerEntity::cheatinfo_t CPlayerEntity::PLAYER_CHEAT_DESCRIPTIONS[] =
{
	cheatinfo_t("noclip", "Toggles noclip mode"),
	cheatinfo_t("god", "Toggles godmode"),
	cheatinfo_t("notarget", "Toggles notarget mode"),
	cheatinfo_t("give", "Gives an item by name"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_ALLWEAPONS, "Gives all weapons to player"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_TRACE_TEXTURE, "Get texture name in front"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_REPORT_AI_STATE, "Get AI state from NPC in front"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_DUMPGLOBALS, "Dumps all global entities"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_DUMP_CODES, "Dumps all codes saved"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_GET_NEAREST_NODE_INDEX, "Get nearest info_node's index"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_SMALL_HULL_NEAREST_NODE_MINS_MAXS, "Show small hull node mins/maxs for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_FLY_HULL_NEAREST_NODE_MINS_MAXS, "Show fly hull node mins/maxs for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_LARGE_HULL_NEAREST_NODE_MINS_MAXS, "Show large hull node mins/maxs for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NEAREST_NODE_MINS_MAXS, "Show human hull node mins/maxs for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_MINS_MAXS, "Show small hull node mins/maxs"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_MINS_MAXS, "Show fly hull node mins/maxs"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_MINS_MAXS, "Show large hull node mins/maxs"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_MINS_MAXS, "Show human hull node mins/maxs"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_PATHS, "Show small hull paths for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_PATHS, "Show fly hull paths for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_PATHS, "Show large hull paths for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_PATHS, "Show human hull paths for nearest node"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_ALL_NODE_PATHS, "Show all node paths in level"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_SHOW_NEAREST_NODE_CONNECTIONS, "Show nearest node and all connections"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_REMOVE_ENTITY, "Remove entity in front"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_GRANT_ALL_SUBWAY_STOPS, "Grant all subway stops"),
	cheatinfo_t(CPlayerEntity::PLAYER_CHEATCODE_NAME_ENTITY_IN_FRONT, "Name entity in front"),
};

// Player drown delay
const Float CPlayerEntity::PLAYER_DROWN_DELAY_TIME = 30;
// Player drown damage ramp-up
const Float CPlayerEntity::PLAYER_DROWN_RAMP_AMOUNT = 5;
// Player drown damage ramp-up limit
const Float CPlayerEntity::PLAYER_DROWN_RAMP_LIMIT = 20;
// Player flashlight drain speed
const Float CPlayerEntity::PLAYER_FLASHLIGHT_DRAIN_TIME = 120; // Two minutes
// Player shoulder flashlight drain speed
const Float CPlayerEntity::PLAYER_FLASHLIGHT_DRAIN_TIME_SHOULDERLIGHT = 60; // One minute
// Player flashlight drain speed
const Float CPlayerEntity::PLAYER_FLASHLIGHT_CHARGE_TIME = 20; // Half a minute
// Max kevlar a player can carry
const Float CPlayerEntity::MAX_PLAYER_KEVLAR = 100;
// Player awareness field of view
const Float CPlayerEntity::PLAYER_NPC_AWARENESS_FIELD_OF_VIEW = 0.5;
// Max player sound radius
const Float CPlayerEntity::PLAYER_SOUND_MAX_RADIUS = 2048;
// Sound radius decay speed
const Float CPlayerEntity::PLAYER_SND_RADIUS_DECAY_SPEED = 512;
// Weapon sound radius decay speed
const Float CPlayerEntity::PLAYER_WEAPON_SND_RADIUS_DECAY_SPEED = 2048;
// Weapon flash decay speed
const Float CPlayerEntity::PLAYER_WEAPON_FLASH_DECAY_SPEED = 256;

// TRUE if using cheat commands(for debugging purposes)
bool CPlayerEntity::m_cheatCommandUsed = false;

//=============================================
// @brief
//
//=============================================
bool ClientCommand( edict_t* pclient )
{
	CBaseEntity* pClientEntity = CBaseEntity::GetClass(pclient);
	if(!pClientEntity || !pClientEntity->IsPlayer())
		return false;

	const Char* pstrCmd = gd_engfuncs.pfnCmd_Argv(0);

	if(!qstrcmp(pstrCmd, "say"))
	{
		if(gd_engfuncs.pfnCmd_Argc() < 2)
		{
			gd_engfuncs.pfnClientPrintf(pclient, "say usage: say <text>.\n");
			return true;
		}

		pClientEntity->HostSay(gd_engfuncs.pfnCmd_Argv(1), false);
		return true;
	}
	else if(!qstrcmp(pstrCmd, "say_team"))
	{
		if(gd_engfuncs.pfnCmd_Argc() < 2)
		{
			gd_engfuncs.pfnClientPrintf(pclient, "say_team usage: say_team <text>.\n");
			return true;
		}

		pClientEntity->HostSay(gd_engfuncs.pfnCmd_Argv(1), true);
		return true;
	}
	else if(!qstrcmp(pstrCmd, "fire"))
	{
		if(!AreCheatsEnabled())
		{
			gd_engfuncs.pfnCon_Printf("Cheats not enabled.\n");
			return true;
		}

		if(gd_engfuncs.pfnCmd_Argc() == 1)
		{
			Vector forward;
			Math::AngleVectors(pclient->state.viewangles, &forward, nullptr, nullptr);

			Vector veceyes = pclient->state.origin + pclient->state.view_offset;
			Vector vecend = veceyes + forward * 1024;

			trace_t tr;
			Util::TraceLine(veceyes, vecend, false, true, pclient, tr);
			if(tr.fraction == 1.0 || tr.hitentity == NO_ENTITY_INDEX || tr.hitentity == 0)
				return true;

			edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
			if(Util::IsNullEntity(pedict))
				return true;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			pEntity->CallUse(pClientEntity, pClientEntity, USE_TOGGLE, 0);
		}
		else
		{
			const Char* pstrtargetname = gd_engfuncs.pfnCmd_Argv(1);
			Util::FireTargets(pstrtargetname, pClientEntity, pClientEntity, USE_TOGGLE, 0);
		}
		return true;
	}
	else if(!qstrcmp(pstrCmd, "give"))
	{
		if(!AreCheatsEnabled())
		{
			gd_engfuncs.pfnCon_Printf("Cheats not enabled.\n");
			return true;
		}

		pClientEntity->GiveItemByName(gd_engfuncs.pfnCmd_Argv(1), 1, false);
		return true;
	}
	else if(!qstrcmp(pstrCmd, "drop"))
	{
		pClientEntity->DropCurrentWeapon();
		return true;
	}
	else if(!qstrcmp(pstrCmd, "lastinv"))
	{
		pClientEntity->SelectPreviousWeapon();
		return true;
	}
	else if(!qstrcmp(pstrCmd, "setdaystage"))
	{
		if(!AreCheatsEnabled())
		{
			gd_engfuncs.pfnCon_Printf("Cheats not enabled.\n");
			return true;
		}

		daystage_t stage = DAYSTAGE_NORMAL;
		const Char* pstrValue = gd_engfuncs.pfnCmd_Argv(1);
		if(!qstrcmp(pstrValue, "normal"))
		{
			stage = DAYSTAGE_NORMAL;
		}
		else if(!qstrcmp(pstrValue, "nightstage"))
		{
			stage = DAYSTAGE_NIGHTSTAGE;
		}
		else if(!qstrcmp(pstrValue, "daylightreturn"))
		{
			stage = DAYSTAGE_DAYLIGHT_RETURN;
		}
		else
		{
			gd_engfuncs.pfnCon_Printf("Invalid day stage specified.\n");
			return true;
		}

		pClientEntity->SetDayStage(stage);
		return true;
	}
	else if(!qstrcmp(pstrCmd, "objectives"))
	{
		pClientEntity->SpawnObjectivesWindow();
		return true;
	}
	else
	{
		// Unrecognized command
		return false;
	}
}

//=============================================
// @brief
//
//=============================================
void ClientPreThink( edict_t* pclient )
{
	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(CBaseEntity::GetClass(pclient));
	
	pPlayer->PreCmdThink();
}

//=============================================
// @brief
//
//=============================================
void ClientPostThink( edict_t* pclient )
{
	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(CBaseEntity::GetClass(pclient));
	
	pPlayer->PostCmdThink();
}

//=============================================
// @brief
//
//=============================================
void CmdStart( const usercmd_t& cmd, edict_t* pclient )
{
}

//=============================================
// @brief
//
//=============================================
void CmdEnd( edict_t* pclient )
{
}

//=============================================
// @brief
//
//=============================================
bool ClientConnect( edict_t* pclient, const Char* pname, const Char* paddress, CString& rejectReason )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void ClientDisconnected( edict_t* pclient )
{

}

//=============================================
// @brief
//
//=============================================
void PM_PlayStepSound( entindex_t entindex, const Char* pstrMaterialName, bool stepleft, Float volume, const Vector& origin )
{
	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(entindex);
	if(!pedict || !(pedict->state.flags & FL_CLIENT))
		return;

	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(CBaseEntity::GetClass(pedict));
	if(!pPlayer)
		return;

	pPlayer->PlayStepSound(pstrMaterialName, stepleft, volume, origin);
}

//=============================================
// @brief
//
//=============================================
void PM_PlaySound( entindex_t entindex, Int32 channel, const Char* psample, Float volume, Float attenuation, Int32 pitch, Int32 flags )
{
	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(entindex);
	if(!pedict || !(pedict->state.flags & FL_CLIENT))
		return;

	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(CBaseEntity::GetClass(pedict));
	if(!pPlayer)
		return;

	pPlayer->PlaySound(channel, psample, volume, attenuation, pitch, flags);
}

// Link player class
LINK_ENTITY_TO_CLASS(player, CPlayerEntity);

//=============================================
// @brief
//
//=============================================
CPlayerEntity::CPlayerEntity( edict_t* pedict ):
	CBaseEntity( pedict ),
	m_pCameraEntity(nullptr),
	m_dayStage(DAYSTAGE_NORMAL),
	m_clientDayStageState(DAYSTAGE_NORMAL),
	m_specialFogEnabled(false),
	m_clientSpecialFogEnabled(false),
	m_lastIllumination(0),
	m_pTriggerCameraEntity(nullptr),
	m_pLoginEntity(nullptr),
	m_pKeypadEntity(nullptr),
	m_pSubwayController(nullptr),
	m_subwayFlags(0),
	m_hasActiveUIWindows(false),
	m_numSavedPasscodes(0),
	m_forceHolster(false),
	m_isInDreamSequence(false),
	m_forceSlowMove(false),
	m_walkedSlowBeforeSlowMove(false),
	m_walkedSlowBeforeWeaponSlowdown(false),
	m_previousWeaponSlowdownState(false),
	m_saveGameTitle(NO_STRING_VALUE),
	m_roomType(0),
	m_clientRoomType(0),
	m_fov(0),
	m_clientFOV(0),
	m_clientConeIndex(0),
	m_nextActionTime(0),
	m_prevFrameButtons(0),
	m_buttonsPressed(0),
	m_buttonsReleased(0),
	m_clientHealth(0),
	m_reloadTime(0),
	m_deathTime(0),
	m_clientStamina(0),
	m_clientKevlar(0),
	m_fallingVelocity(0),
	m_isHUDVisible(false),
	m_clientHUDVisible(false),
	m_movementNoise(0),
	m_clientMovementNoise(0),
	m_pLadderEntity(nullptr),
	m_ladderState(LADDER_STATE_INACTIVE),
	m_ladderUpdateTime(0),
	m_nextLadderMoveTime(0),
	m_ladderMoveDirection(0),
	m_clientLadderMoveDirection(0),
	m_bikeUpdateTime(0),
	m_bikeState(0),
	m_clientBikeState(0),
	m_bikeAcceleration(0),
	m_pBikeEntity(nullptr),
	m_prevFlags(0),
	m_prevButtons(0),
	m_leanTime(0),
	m_leanState(0),
	m_pActiveWeapon(nullptr),
	m_pClientActiveWeapon(nullptr),
	m_pNextWeapon(nullptr),
	m_pPreviousWeapon(nullptr),
	m_pLastWeapon(nullptr),
	m_hasNewWeapon(false),
	m_weaponFlashBrightness(0),
	m_weaponSoundRadius(0),
	m_stepSoundRadius(0),
	m_clientViewModel(NO_STRING_VALUE),
	m_pWeaponsList(nullptr),
	m_weaponFullPromptTime(0),
	m_numMedkits(0),
	m_numClientMedkits(0),
	m_healProgress(-1),
	m_clientHealProgress(0),
	m_forceWeaponUpdate(false),
	m_isWeaponHolstered(false),
	m_flashlightBattery(0),
	m_clientFlashlightBattery(0),
	m_hasShoulderLight(false),
	m_damageTypes(0),
	m_lastImpactedHitGroup(0),
	m_lastDmgAmount(0),
	m_lastDamageTime(0),
	m_nextPainSoundTime(0),
	m_drownDamageAmount(0),
	m_drownDamageHealed(0),
	m_isUnderwaterSoundPlaying(false),
	m_underwaterTime(0),
	m_lastWaterDamageTime(0),
	m_lastWaterDamage(0),
	m_nextSwimSoundTime(0),
	m_prevWaterLevel(WATERLEVEL_NONE),
	m_dialougePlaybackTime(0),
	m_tapeTrackFile(NO_STRING_VALUE),
	m_tapeTrackPlayBeginTime(0),
	m_tapeTrackDuration(0),
	m_tapePlaybackTitle(NO_STRING_VALUE),
	m_tapePlaybackAlpha(0),
	m_currentDiaryTrackName(NO_STRING_VALUE),
	m_diaryTrackPlayBeginTime(0),
	m_diaryTrackDuration(0),
	m_highestAwarenessLevel(0),
	m_clientHighestAwarenessLevel(0),
	m_objectivesNewFlags(0),
	m_lastObjectiveAddTime(0),
	m_currentUsableObjectType(USABLE_OBJECT_NONE),
	m_countdownTimerEndTime(0),
	m_countdownTimerTitle(NO_STRING_VALUE),
	m_delayedGlobalTriggerTime(0),
	m_delayedGlobalTriggerTarget(NO_STRING_VALUE),
	m_isOnTarget(false)
{
	for(Uint32 i = 0; i < MAX_SAVED_PASSCODES; i++)
		m_savedPasscodes[i] = NO_STRING_VALUE;

	for(Uint32 i = 0; i < NUM_TIMEBASED_DMG; i++)
	{
		m_timeBasedDmgTime[i] = 0;
		m_lastTimeBasedDmgTime[i] = 0;
	}

	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
	{
		m_ammoCounts[i] = 0;
		m_clientAmmoCounts[i] = 0;
	}

	for(Uint32 i = 0; i < GAMEUI_MAX_OBJECTIVES; i++)
		m_objectivesArray[i] = NO_STRING_VALUE;
}

//=============================================
// @brief
//
//=============================================
CPlayerEntity::~CPlayerEntity()
{
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_flashlightBattery, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_prevButtons, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_buttonsPressed, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_buttonsReleased, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_numMedkits, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_dropOrigin, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_dropAngles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_bikeState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_bikeVelocity, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_bikeAcceleration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_lastDmgAmount, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pWeaponsList, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pActiveWeapon, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pPreviousWeapon, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pNextWeapon, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CPlayerEntity, m_ammoCounts, EFIELD_INT32, MAX_AMMO_TYPES));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_drownDamageAmount, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_drownDamageHealed, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_fallingVelocity, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_isHUDVisible, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_fov, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_subwayFlags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_isWeaponHolstered, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pBikeEntity, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_ladderState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_ladderMoveDirection, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pLadderEntity, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_ladderDestOrigin, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_ladderDestAngles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_pCameraEntity, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_forceHolster, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_isInDreamSequence, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_hasShoulderLight, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_dayStage, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_specialFogEnabled, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_healProgress, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_forceSlowMove, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_walkedSlowBeforeSlowMove, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_walkedSlowBeforeWeaponSlowdown, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_previousWeaponSlowdownState, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_saveGameTitle, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CPlayerEntity, m_savedPasscodes, EFIELD_STRING, MAX_SAVED_PASSCODES));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_numSavedPasscodes, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_lastWaterDamageTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_lastWaterDamage, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_underwaterTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_nextSwimSoundTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_roomType, EFIELD_INT32));

	// MUSIC_CHANNEL_0
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[0].begintime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[0].filename, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[0].flags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[0].channel, EFIELD_INT32));
	// MUSIC_CHANNEL_1
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[1].begintime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[1].filename, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[1].flags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[1].channel, EFIELD_INT32));
	// MUSIC_CHANNEL_2
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[2].begintime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[2].filename, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[2].flags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[2].channel, EFIELD_INT32));
	// MUSIC_CHANNEL_3
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[3].begintime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[3].filename, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[3].flags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_musicPlaybackInfoArray[3].channel, EFIELD_INT32));

	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_prevWaterLevel, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_tapePlaybackTitle, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_tapeTrackFile, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_tapeTrackPlayBeginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_tapeTrackDuration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_tapeTitleColor, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_tapePlaybackAlpha, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CPlayerEntity, m_objectivesArray, EFIELD_STRING, GAMEUI_MAX_OBJECTIVES));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_objectivesNewFlags, EFIELD_INT16));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_countdownTimerEndTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_countdownTimerTitle, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_delayedGlobalTriggerTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_delayedGlobalTriggerTarget, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerEntity, m_dialougePlaybackTime, EFIELD_TIME));
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	InitStepSounds();

	// Precache footstep sounds
	CArray<CString> soundsList;
	m_pstepSound.GetPrecacheList(soundsList);

	if(!soundsList.empty())
	{
		for(Uint32 i = 0; i < soundsList.size(); i++)
			gd_engfuncs.pfnPrecacheSound(soundsList[i].c_str());
	}

	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->movetype = MOVETYPE_WALK;
	m_pState->view_offset = VEC_VIEW;
	m_pState->stamina = 1.0;
	m_pState->health = 100;
	m_pState->maxhealth = 100;
	m_pState->gravity = 1.0;
	m_pState->fov = m_fov = 0;
	m_reloadTime = 0;
	m_pState->takedamage = TAKEDAMAGE_YES;
	m_pState->armorvalue = 0;
	m_pState->flags |= FL_CLIENT;
	m_pState->deadstate = DEADSTATE_NONE;
	m_pState->friction = 1.0;

	if(!SetModel("models/player.mdl", false))
		return false;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

	if(m_pState->flags & FL_DUCKING)
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HULL_MIN, VEC_HULL_MAX);
	
	// Set these
	m_flashlightBattery = 1.0;
	m_forceWeaponUpdate = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel("models/player.mdl");
	gd_engfuncs.pfnPrecacheModel(CTriggerCameraModel::V_SEQUENCES_MODELNAME);

	// Precache footstep sounds
	CArray<CString> soundsList;
	m_pstepSound.GetPrecacheList(soundsList);

	if(!soundsList.empty())
	{
		for(Uint32 i = 0; i < soundsList.size(); i++)
			gd_engfuncs.pfnPrecacheSound(soundsList[i].c_str());
	}

	Util::PrecacheFixedNbSounds("player/pl_surface_light%d.wav", 3);
	Util::PrecacheFixedNbSounds("player/pl_surface_medium%d.wav", 3);
	Util::PrecacheFixedNbSounds("player/pl_surface_heavy%d.wav", 3);
	Util::PrecacheFixedNbSounds("player/pl_water_surface%d.wav", 3);

	gd_engfuncs.pfnPrecacheSound("player/pl_underwater.wav");
	gd_engfuncs.pfnPrecacheSound("player/pl_die_heartbeat.wav");

	Util::PrecacheFixedNbSounds("player/pl_swim%d.wav", 4);
	Util::PrecacheFixedNbSounds("player/pl_wade%d.wav", 4);

	Util::PrecacheFixedNbSounds("misc/acid_sizzle%d.wav", 3);

	Util::PrecacheFixedNbSounds("player/pl_pain%d.wav", 3);
	Util::PrecacheFixedNbSounds("player/pl_die%d.wav", 3);
	Util::PrecacheFixedNbSounds("player/pl_drown%d.wav", 3);

	gd_engfuncs.pfnPrecacheSound("misc/ear_ringing.wav");
	gd_engfuncs.pfnPrecacheSound(FLASHLIGHT_TOGGLE_SOUND);

	if(m_tapeTrackFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_tapeTrackFile));

	for(Uint32 i = 0; i < NB_MUSIC_CHANNELS; i++)
	{
		music_data_t& track = m_musicPlaybackInfoArray[i];
		if(track.filename == NO_STRING_VALUE)
			continue;

		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(track.filename));
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::Restore( void )
{
	if(!CBaseEntity::Restore())
		return false;

	if(m_pState->flags & FL_DUCKING)
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HULL_MIN, VEC_HULL_MAX);

	InitStepSounds();

	// Precache footstep sounds
	CArray<CString> soundsList;
	m_pstepSound.GetPrecacheList(soundsList);

	if(!soundsList.empty())
	{
		for(Uint32 i = 0; i < soundsList.size(); i++)
			gd_engfuncs.pfnPrecacheSound(soundsList[i].c_str());
	}

	// Restore nightstage
	if(m_dayStage != DAYSTAGE_NORMAL)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setdaystage, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(m_dayStage);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientDayStageState = m_dayStage;
	}

	// Restore ladder
	if(m_ladderState == LADDER_STATE_ACTIVE)
	{
		m_pState->viewangles = m_pState->angles = m_ladderDestAngles;
		m_pState->fixangles = true;
		m_ladderMoveDirection = LADDER_RESTING;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteInt16(m_pLadderEntity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteInt16(m_ladderState);
			gd_engfuncs.pfnMsgWriteByte(m_ladderMoveDirection);
		gd_engfuncs.pfnUserMessageEnd();
	}
	else if(m_ladderState == LADDER_STATE_ENTERING || m_ladderState == LADDER_STATE_LEAVING)
	{
		if(m_ladderState == LADDER_STATE_LEAVING)
		{
			// Position player approximately
			gd_engfuncs.pfnSetOrigin(m_pEdict, m_ladderDestOrigin);
			m_pState->viewangles = m_pState->angles = m_ladderDestAngles;
			m_pState->fixangles = true;
		}

		m_pState->flags &= ~FL_FROZEN;
		m_pState->flags &= ~FL_ON_LADDER;
		m_ladderState = LADDER_STATE_CLEANUP;
		m_ladderUpdateTime = g_pGameVars->time;

		ClearLadder();
	}

	// Restore bike
	if(m_bikeState != BIKE_SV_INACTIVE)
	{
		// See if we had a bike state
		if(m_bikeState == BIKE_SV_ENTERING_LERP || m_bikeState == BIKE_SV_ENTERING || m_bikeState == BIKE_SV_ACTIVE)
		{
			m_clientBikeState = m_bikeState = BIKE_SV_ACTIVE; // Force it into active state
			
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteByte(BIKE_SV_RESTORE);
				gd_engfuncs.pfnMsgWriteInt16(m_pBikeEntity->GetEntityIndex());
				gd_engfuncs.pfnMsgWriteFloat(m_bikeAcceleration);
			gd_engfuncs.pfnUserMessageEnd();

			m_pState->velocity = m_bikeVelocity;
			SetAiment(m_pBikeEntity);

			if(m_bikeState == BIKE_SV_ENTERING_LERP)
			{
				gd_engfuncs.pfnSetOrigin(m_pEdict, m_pBikeEntity->GetOrigin()+Vector(0, 0, VEC_HULL_MAX[2]));
				SetAngles(m_pBikeEntity->GetAngles());
				SetViewAngles(m_pBikeEntity->GetAngles());

				m_pBikeEntity->SetFollow();
			}

			m_pState->fuser1 = TRUE;
			m_pState->fuser2 = m_bikeAcceleration;
		}
		else if(m_bikeState == BIKE_SV_LEAVING || m_bikeState == BIKE_SV_LEAVING_LERP || m_bikeState == BIKE_SV_CLEANUP)
		{
			m_clientBikeState = m_bikeState = BIKE_SV_INACTIVE;
			m_pState->fuser1 = FALSE;
			SetAiment(nullptr);

			m_pBikeEntity->PlayerLeave();
			m_pBikeEntity = nullptr;

			gd_engfuncs.pfnSetOrigin(m_pEdict, m_dropOrigin);
			SetAngles(m_dropAngles);
			SetViewAngles(m_dropAngles);
		}

		if(m_bikeState == BIKE_SV_INACTIVE)
			ToggleBikeBlockers(false);

		SetControlEnable(true);
	}

	// Update viewmodel
	if(m_pFields->viewmodel != m_clientViewModel)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.viewmodel, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(VMODEL_SET_MODEL);
			gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->viewmodel));
		gd_engfuncs.pfnUserMessageEnd();

		m_clientViewModel = m_pFields->viewmodel;
	}

	// Restore music
	for(Uint32 i = 0; i < NB_MUSIC_CHANNELS; i++)
	{
		music_data_t& track = m_musicPlaybackInfoArray[i];
		if(track.filename == NO_STRING_VALUE)
			continue;

		if(!track.begintime)
		{
			track.filename = NO_STRING_VALUE;
			track.duration = 0;
			track.flags = 0;
			track.channel = 0;
			continue;
		}

		const Char* pstrFilename = gd_engfuncs.pfnGetString(track.filename);

		track.duration = gd_engfuncs.pfnGetSoundDuration(pstrFilename, PITCH_NORM);
		if(!track.duration)
			gd_engfuncs.pfnCon_Printf("Failed to get duration for ogg file '%s'.\n", pstrFilename);

		if(!track.duration || !(track.flags & OGG_FL_LOOP) 
			&& track.duration < (g_pGameVars->time - track.begintime))
		{
			track.filename = NO_STRING_VALUE;
			track.duration = 0;
			track.flags = 0;
			track.channel = 0;
			track.begintime = 0;
		}
		else
		{
			Float timeOffset = g_pGameVars->time - track.begintime;
			gd_engfuncs.pfnPlayMusic(pstrFilename, track.channel, track.flags, timeOffset, 0, m_pEdict->clientindex);
		}
	}

	// Restore tape playback
	if(m_tapeTrackFile)
	{
		if((g_pGameVars->time - m_tapeTrackPlayBeginTime) < m_tapeTrackDuration)
		{
			Float timeoffset = g_pGameVars->time - m_tapeTrackPlayBeginTime;
			Float remaintime = m_tapeTrackDuration - timeoffset;

			Util::EmitEntitySound(this, gd_engfuncs.pfnGetString(m_tapeTrackFile), SND_CHAN_STREAM, VOL_NORM, ATTN_NORM, PITCH_NORM, (SND_FL_REVERBLESS|SND_FL_DIM_OTHERS_LIGHT), this, timeoffset);

			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.radiomessage, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
				gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_tapePlaybackTitle));
				gd_engfuncs.pfnMsgWriteSmallFloat(remaintime);
				gd_engfuncs.pfnMsgWriteByte(m_tapeTitleColor.x);
				gd_engfuncs.pfnMsgWriteByte(m_tapeTitleColor.y);
				gd_engfuncs.pfnMsgWriteByte(m_tapeTitleColor.z);
				gd_engfuncs.pfnMsgWriteByte(m_tapePlaybackAlpha);
			gd_engfuncs.pfnUserMessageEnd();
		}
		else
		{
			m_tapeTrackFile = NO_STRING_VALUE;
			m_tapePlaybackTitle = NO_STRING_VALUE;
			m_tapeTrackPlayBeginTime = 0;
			m_tapeTrackDuration = 0;
			m_tapeTitleColor.Clear();
			m_tapePlaybackAlpha = 0;
		}
	}

	if(m_objectivesNewFlags != 0)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.newobjective, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(true);
		gd_engfuncs.pfnUserMessageEnd();
	}

	if(m_countdownTimerEndTime)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetcountdowntimer, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteDouble(m_countdownTimerEndTime);
			gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_countdownTimerTitle));
		gd_engfuncs.pfnUserMessageEnd();
	}

	// Force re-send of all weapon types
	m_forceWeaponUpdate = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	Float dmgamount = SDL_ceil(amount);
	if(!dmgamount)
		return false;

	// Cancel any healing
	m_healProgress = -1;

	// Don't sustain damage from allies
	if(pAttacker && pAttacker->IsNPC() && pAttacker != this
		&& (GetRelationship(pAttacker) == R_ALLY 
		|| GetRelationship(pAttacker) == R_FRIEND 
		|| GetRelationship(pAttacker) == R_NONE))
		return false;

	// Play bullet impact sounds
	if(dmgamount > 0 && (damageFlags & DMG_BULLET))
	{
		CString soundfile;
		switch(Common::RandomLong(0, 1))
		{
		case 1: 
			soundfile = "impact/bullet_hit_flesh2.wav"; 
			break;
		case 0: 
		default:
			soundfile = "impact/bullet_hit_flesh1.wav"; 
			break;
		}

		Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_BODY);
	}

	// don't do anything else if dead
	if(!IsAlive())
		return false;

	m_damageTypes |= damageFlags;

	// Get attck vector
	Vector direction;
	if(pInflictor && !Util::IsNullEntity(pInflictor->GetEdict()))
	{
		direction = (pInflictor->GetCenter() - Vector(0, 0, 10) - GetCenter()).Normalize();
		gMultiDamage.SetAttackDirection(direction);
	}

	// Set dmg inflictor
	if(pInflictor)
		m_pState->dmginflictor = pInflictor->GetEntityIndex();

	m_pState->dmgtaken = dmgamount;

	// Ignore damage in godmode
	if(m_pState->flags & FL_GODMODE)
		return false;

	// If player, move him around
	if(pInflictor && !Util::IsNullEntity(pInflictor->GetEdict()) 
		&& m_pState->movetype == MOVETYPE_WALK 
		&& (!pAttacker || pAttacker->GetSolidity() != SOLID_TRIGGER))
		m_pState->velocity += direction*(-Util::GetDamageForce(*m_pEdict, dmgamount));

	// Apply damage
	m_pState->health -= dmgamount;

	if(m_pState->health <= 0)
	{
		if(damageFlags & DMG_ALWAYSGIB)
			Killed(pAttacker, GIB_ALWAYS);
		else if(damageFlags & (DMG_NEVERGIB))
			Killed(pAttacker, GIB_NEVER);
		else
			Killed(pAttacker, GIB_NORMAL);

		return false;
	}
	else if(m_pState->deadstate != DEADSTATE_DEAD)
	{
		// Play pain sounds
		PlayPainSound();
	}

	// Reset counters on time based damages
	if(pInflictor != this && pAttacker != this)
	{
		for(Uint32 i = 0; i < NUM_TIMEBASED_DMG; i++)
		{
			if(!(damageFlags & DMG_TIMEBASED_INFLICTED) && (damageFlags & TIMEBASED_DMG_BITS[i]))
			{
				m_timeBasedDmgTime[i] = g_pGameVars->time;
				m_lastTimeBasedDmgTime[i] = g_pGameVars->time;
			}
		}
	}

	// Apply kick and fade depending on damage taken
	if(m_pState->health > 0 && dmgamount > 0)
	{
		if(m_pState->health < 30)
		{
			// Critical damage
			ApplyAxisPunch(0, -35);
			ApplyAxisPunch(1, -25);
			ApplyDirectAxisPunch(0, 1.0);
			ApplyDirectAxisPunch(1, 0.7);

			// Apply screen fade
			Util::ScreenFadePlayer(m_pEdict, PAIN_SCREENFADE_COLOR, 2, 0, 140, FL_FADE_IN|FL_FADE_MODULATE, 1);
		}
		else if(m_lastDmgAmount > 25)
		{
			// Major damage
			ApplyAxisPunch(0, -25);
			ApplyAxisPunch(1, -15);
			ApplyDirectAxisPunch(0, 0.8);
			ApplyDirectAxisPunch(1, 0.4);

			// Apply screen fade
			Util::ScreenFadePlayer(m_pEdict, PAIN_SCREENFADE_COLOR, 1, 0, 120, FL_FADE_IN|FL_FADE_MODULATE, 1);
		}
		else
		{
			// Trivial damage
			ApplyAxisPunch(0, -15);
			ApplyAxisPunch(1, -5);

			// Apply screen fade
			Util::ScreenFadePlayer(m_pEdict, PAIN_SCREENFADE_COLOR, 0.5, 0, 100, FL_FADE_IN|FL_FADE_MODULATE, 1);
		}
	}

	m_lastDamageTime = g_pGameVars->time;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	if(m_pState->takedamage == TAKEDAMAGE_NO)
		return;

	Float _damage = damage;
	if(!(damageFlags & (DMG_FALL|DMG_DROWN)))
	{
		// Calculat drain and absorpotion
		Float armordrain = PLAYER_ARMOR_DMG_DRAIN * _damage;
		Float newdamage = PLAYER_ARMOR_DMG_ABSORB * _damage;

		if(armordrain > m_pState->armorvalue)
		{
			newdamage = ((armordrain-m_pState->armorvalue)/_damage)*_damage;
			armordrain = m_pState->armorvalue;
		}

		m_pState->armorvalue -= armordrain;
		_damage = newdamage;
	}

	m_lastImpactedHitGroup = tr.hitgroup;

	// Adjust damage by skill cvars based on hitgroup
	Float dmgadjust = 1.0;
	switch(tr.hitgroup)
	{
	case HITGROUP_GENERIC:
		break;
	case HITGROUP_HEAD:
		dmgadjust = gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgMultiplierHead);
		break;
	case HITGROUP_CHEST:
		dmgadjust = gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgMultiplierChest);
		break;
	case HITGROUP_STOMACH:
		dmgadjust = gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgMultiplierStomach);
		break;
	case HITGROUP_LEFT_ARM:
	case HITGROUP_RIGHT_ARM:
		dmgadjust = gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgMultiplierArm);
		break;
	case HITGROUP_LEFT_LEG:
	case HITGROUP_RIGHT_LEG:
		dmgadjust = gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgMultiplierLeg);
		break;
	default:
		break;
	}

	// Adjust damage by multiplier
	_damage *= dmgadjust;

	// Create a decal on player/view model
	Util::CreateVBMDecal(tr.endpos, tr.plane.normal, "shot_human", m_pEdict, FL_DECAL_NORMAL_PERMISSIVE);

	// Spawn blood particles and decals
	Util::SpawnBloodParticles(tr, GetBloodColor(), IsPlayer());
	// Create decals on walls or other npcs
	SpawnBloodDecals(_damage, direction, tr, damageFlags);

	// Manage multidamage
	gMultiDamage.AddDamage(this, _damage, damageFlags);
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GetRelationship( CBaseEntity* pOther )
{
	Int32 myClassification = GetClassification();
	assert(myClassification >= 0 && myClassification < NB_CLASSIFICATIONS);
	Int32 enemyClassification = pOther->GetClassification();
	assert(enemyClassification >= 0 && enemyClassification < NB_CLASSIFICATIONS);

	return NPC_RELATIONS_TABLE[myClassification][enemyClassification];
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode )
{
	// Exit bike if we were on it
	if(m_bikeState)
	{
		m_pBikeEntity->PlayerLeave();

		m_clientBikeState = BIKE_SV_CLEANUP;
		m_bikeState = BIKE_SV_INACTIVE;

		m_bikeUpdateTime = g_pGameVars->time + 0.1;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(BIKE_SV_CLEANUP);
		gd_engfuncs.pfnUserMessageEnd();

		Util::EmitAmbientSound(m_pState->origin, "bike/bike_crash_fatal.wav");
		m_pBikeEntity->CallUse(nullptr, nullptr, USE_OFF, 0);

		if(IsFlashlightOn())
			TurnFlashlightOff();
	}

	// Exit ladder if we were on it
	if(m_ladderState != LADDER_STATE_INACTIVE)
	{
		m_ladderState = LADDER_STATE_INACTIVE;
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteInt16(0);
			gd_engfuncs.pfnMsgWriteInt16(LADDER_STATE_CLEANUP);
		gd_engfuncs.pfnUserMessageEnd();

		ClearLadder();
	}

	if(m_pActiveWeapon)
		m_pActiveWeapon->Holster();

	// Clear player sounds
	gAISounds.ClearEmitterSounds(this);

	m_pState->deadstate = DEADSTATE_DYING;
	m_pState->movetype = MOVETYPE_TOSS;
	m_pState->flags &= ~FL_ONGROUND;

	if(m_pState->velocity.z < 10)
		m_pState->velocity.z += Common::RandomFloat(0, 300);

	m_clientHealth = 0;
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudhealth, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(m_clientHealth);
	gd_engfuncs.pfnUserMessageEnd();

	// Reset current weapon on client
	ClearCurrentWeapon();

	// Reset FOV
	m_pState->fov = 0;
	m_fov = 0;
	m_clientFOV = 0;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setfov, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(0);
	gd_engfuncs.pfnUserMessageEnd();

	// If not killed by black hole, fade out and blur view
	if(!(m_damageTypes & (DMG_BLACKHOLE)))
	{
		Util::ScreenFadePlayer(m_pEdict, color24_t(0, 0, 0), 4, 4, 255, FL_FADE_OUT | FL_FADE_MODULATE | FL_FADE_CLEARGAME);
		m_reloadTime = g_pGameVars->time + 7;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motionblur, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(TRUE);
			gd_engfuncs.pfnMsgWriteSmallFloat(0);
			gd_engfuncs.pfnMsgWriteByte(FALSE);
		gd_engfuncs.pfnUserMessageEnd();

		PlayDeathSound();

		// Critical damage
		ApplyAxisPunch(0, -35);
		ApplyAxisPunch(1, -25);
		ApplyDirectAxisPunch(0, 1.0);
		ApplyDirectAxisPunch(1, 0.7);

		// Apply screen fade
		Util::ScreenFadePlayer(m_pEdict, PAIN_SCREENFADE_COLOR, 2, 0, 140, FL_FADE_IN|FL_FADE_MODULATE, 1);
	}
	else if(m_damageTypes & DMG_BLACKHOLE)
	{
		// Black hole just blanks the screen
		Util::ScreenFadePlayer(m_pEdict, BLACK_COLOR, 0.01, 10, 255, FL_FADE_STAYOUT);
		// Mute all sounds
		gd_engfuncs.pfnSetMuteAllSounds(true);
	}
	
	// Set angles to neutral except yaw
	m_pState->angles[PITCH] = 0;
	m_pState->angles[ROLL] = 0;

	// Set death time and flag
	m_pState->flags |= FL_DEAD;
	m_deathTime = g_pGameVars->time;

	// Play death animation
	if(!(m_damageTypes & (DMG_BLACKHOLE|DMG_FALL|DMG_DROWN|DMG_CRUSH)) && (m_pState->groundent != NO_ENTITY_INDEX))
	{
		m_pState->velocity.Clear();
		m_pState->avelocity.Clear();

		CString sequenceName;

		Vector forward;
		Math::AngleVectors(m_pState->angles, &forward, nullptr, nullptr);
		forward[2] = 0;
		forward.Normalize();

		const Vector& attackDirection = gMultiDamage.GetAttackDirection();
		Float dp = Math::DotProduct(forward, attackDirection);

		Vector originOffset;
		if(m_pState->flags & FL_DUCKING)
		{
			if(dp < 0)
				sequenceName << "die_ducking_forward" << (Int32)Common::RandomLong(1, 3);
			else
				sequenceName << "die_ducking_backwards" << (Int32)Common::RandomLong(1, 2);

			originOffset = Vector(0, 0, SDL_fabs(VEC_DUCK_HULL_MIN[2]));
		}
		else
		{
			if(dp < 0)
				sequenceName << "die_standing_forward" << (Int32)Common::RandomLong(1, 2);
			else
				sequenceName << "die_standing_backwards" << (Int32)Common::RandomLong(1, 2);

			originOffset = Vector(0, 0, SDL_fabs(VEC_HULL_MIN[2]));
		}

		CTriggerCameraModel* pDeathCamera = CTriggerCameraModel::CreateCameraModel(this, 0.1, sequenceName.c_str(), sequenceName.c_str(), nullptr, sequenceName.c_str());
		if(!pDeathCamera)
		{
			gd_engfuncs.pfnCon_Printf("couldn't create cameramodel.\n");
			return;
		}

		pDeathCamera->RemoveSpawnFlag(CTriggerCameraModel::FL_FOLLOW_PLAYER);
		pDeathCamera->SetSpawnFlag(CTriggerCameraModel::FL_LEAVE_ON_TRIGGER);
		pDeathCamera->SetSpawnFlag(CTriggerCameraModel::FL_ALWAYS_DRAW);

		Vector cameraOrigin = m_pState->origin - originOffset;
		pDeathCamera->SetOrigin(cameraOrigin);
		pDeathCamera->SetAngles(m_pState->angles);

		pDeathCamera->CallUse(this, this, USE_TOGGLE, 0);
	}
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GetIllumination( void )
{
	if((m_lastLightOrigin-m_pState->origin).Length2D() > LIGHT_UPDATE_DISTANCE)
	{
		m_lastIllumination = Util::GetIllumination(m_pState->origin);
		if(m_lastIllumination > 255)
			m_lastIllumination = 255;

		m_lastLightOrigin = m_pState->origin;
	}

	if(m_pState->effects & (EF_DIMLIGHT|EF_SHOULDERLIGHT))
		return clamp(m_lastIllumination + 64, 0, 255);
	else
		return m_lastIllumination;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetControlEnable( bool enable )
{
	if(enable)
		m_pState->flags &= ~FL_FROZEN;
	else
		m_pState->flags |= FL_FROZEN;
}

//=============================================
// @brief Set player paralysis bitflag
//
//=============================================
void CPlayerEntity::SetPlayerParalyzed( bool enable )
{
	if(!enable)
		m_pState->flags &= ~FL_PARALYZED;
	else
		m_pState->flags |= FL_PARALYZED;
}

//=============================================
// @brief Tells whether player is paralyzed
//
//=============================================
bool CPlayerEntity::GetIsPlayerParalyzed( void )
{
	return (m_pState->flags & FL_PARALYZED) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetCameraEntity( CTriggerCameraModel* pCamera )
{
	m_pCameraEntity = pCamera;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::DeadThink( void )
{
	// Reload if we have passed the time
	if(m_reloadTime)
	{
		if(m_reloadTime <= g_pGameVars->time)
		{
			gd_engfuncs.pfnServerCommand("reload\n");
			return;
		}
	}

	// Reduce velocity
	if(m_pState->flags & FL_ONGROUND)
	{
		Float forwardvel = m_pState->velocity.Length() - 20;
		if(forwardvel <= 0)
			m_pState->velocity.Clear();
		else
			m_pState->velocity = forwardvel*m_pState->velocity.Normalize();
	}

	// Remove all weapons
	RemoveAllWeapons();

	// If onground, set movetype to none
	if(m_pState->movetype != MOVETYPE_NONE && (m_pState->flags & FL_ONGROUND))
		m_pState->movetype = MOVETYPE_NONE;

	if(m_pState->deadstate == DEADSTATE_DYING)
		m_pState->deadstate = DEADSTATE_DEAD;

	// Reset framerate
	m_pState->framerate = 0;

	// Give a one second delay will we can reload
	if((g_pGameVars->time - m_deathTime) < 1.0)
		return;

	if(m_pState->buttons)
	{
		gd_engfuncs.pfnServerCommand("reload\n");
		return;
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::IsAlive( void ) const
{
	return (m_pState->deadstate == DEADSTATE_NONE && m_pState->health > 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::IsOnMotorBike( void ) const
{
	return (m_pBikeEntity != nullptr) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GetClassification( void ) const
{
	return CLASS_PLAYER;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetFallingVelocity( Float velocity )
{
	m_pState->fallvelocity = velocity;
	m_fallingVelocity = velocity;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlayerUseThink( void )
{
	if(m_pLadderEntity || m_pBikeEntity || m_diaryEntity)
	{
		if(m_currentPlayerUsableObject)
		{
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetusableobject, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(USABLE_OBJECT_NONE); // Tell the HUD to stop displaying
			gd_engfuncs.pfnUserMessageEnd();

			m_currentPlayerUsableObject.reset();
		}

		return;
	}

	// Look up an entity to use
	Vector forward;
	Math::AngleVectors(m_pState->viewangles, &forward, nullptr, nullptr);

	edict_t* pClosest = nullptr;
	Float closestdp = VIEW_FIELD_NARROW;

	// Determine mins/maxs
	Vector mins = m_pState->origin - Vector(PLAYER_USE_RADIUS, PLAYER_USE_RADIUS, PLAYER_USE_RADIUS);
	Vector maxs = m_pState->origin + Vector(PLAYER_USE_RADIUS, PLAYER_USE_RADIUS, PLAYER_USE_RADIUS);

	Float playerUseDistance = Vector(PLAYER_USE_RADIUS, PLAYER_USE_RADIUS, PLAYER_USE_RADIUS).Length();

	Vector viewOrigin = m_pState->origin + m_pState->view_offset;
	
	edict_t* pedict = Util::FindEntityInBBox(nullptr, mins, maxs);
	while(pedict)
	{
		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->GetEntityFlags() & (FL_ENTITY_PLAYER_USABLE|FL_ENTITY_ONOFF_USE|FL_ENTITY_CONTINOUS_USE))
		{
			Vector endPosition = viewOrigin + forward * playerUseDistance;

			Int32 savedSolidity = -1;
			if(pEntity->GetSolidity() == SOLID_NOT
				|| pEntity->GetSolidity() == SOLID_TRIGGER)
			{
				// Remember this
				savedSolidity = pEntity->GetSolidity();
				if(pEntity->IsBrushModel())
					pEntity->SetSolidity(SOLID_BSP);
				else
					pEntity->SetSolidity(SOLID_BBOX);

				pEntity->SetOrigin(pEntity->GetOrigin());
			}

			trace_t tr;
			Util::TraceLine(viewOrigin, endPosition, false, true, m_pEdict, tr);

			if(tr.noHit() || tr.startSolid() || tr.allSolid())
			{
				if(savedSolidity != -1)
				{
					pEntity->SetSolidity((solid_t)savedSolidity);
					pEntity->SetOrigin(pEntity->GetOrigin());
				}

				pedict = Util::FindEntityInBBox(pedict, mins, maxs);
				continue;
			}

			// Some entities may overlap
			if(tr.hitentity != pedict->entindex)
			{
				trace_t tr2;
				Util::TraceModel(pEntity, viewOrigin, endPosition, true, HULL_POINT, tr2);
				if(tr2.noHit() || SDL_fabs(tr2.fraction - tr.fraction) > 0.001)
				{
					if(savedSolidity != -1)
					{
						pEntity->SetSolidity((solid_t)savedSolidity);
						pEntity->SetOrigin(pEntity->GetOrigin());
					}

					pedict = Util::FindEntityInBBox(pedict, mins, maxs);
					continue;
				}
			}

			if(savedSolidity != -1)
			{
				pEntity->SetSolidity((solid_t)savedSolidity);
				pEntity->SetOrigin(pEntity->GetOrigin());
			}

			Vector vectoobject = (pEntity->GetBrushModelCenter() - GetEyePosition());
			vectoobject = Util::ClampVectorToBox(vectoobject, pEntity->GetSize() * 0.5);
			Float dp = Math::DotProduct(vectoobject, forward);
			if(dp > closestdp)
			{
				pClosest = pedict;
				closestdp = dp;
			}
		}

		// Find next
		pedict = Util::FindEntityInBBox(pedict, mins, maxs);
	}

	if(pClosest != m_currentPlayerUsableObject.get())
	{
		if(pClosest)
		{
			CBaseEntity* pEntity = CBaseEntity::GetClass(pClosest);
			usableobject_type_t type = pEntity->GetUsableObjectType();

			if(type != USABLE_OBJECT_NONE)
			{
				pEntity->GetUseReticleMinsMaxs(m_currentUsableObjectMins, m_currentUsableObjectMaxs, this);
				m_currentUsableObjectType = type;

				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetusableobject, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteByte(m_currentUsableObjectType);
				for(Uint32 i = 0; i < 3; i++)
					gd_engfuncs.pfnMsgWriteFloat(m_currentUsableObjectMins[i]);
				for(Uint32 i = 0; i < 3; i++)
					gd_engfuncs.pfnMsgWriteFloat(m_currentUsableObjectMaxs[i]);
				gd_engfuncs.pfnUserMessageEnd();
			}
			else
			{
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetusableobject, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteByte(USABLE_OBJECT_NONE); // Tell the HUD to stop displaying
				gd_engfuncs.pfnUserMessageEnd();
			}
		}
		else
		{
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetusableobject, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(USABLE_OBJECT_NONE); // Tell the HUD to stop displaying
			gd_engfuncs.pfnUserMessageEnd();
		}

		m_currentPlayerUsableObject = pClosest;
	}
	else if(m_currentPlayerUsableObject)
	{
		usableobject_type_t type = m_currentPlayerUsableObject->GetUsableObjectType();
		
		Vector objectMins, objectMaxs;
		m_currentPlayerUsableObject->GetUseReticleMinsMaxs(objectMins, objectMaxs, this);

		if(type != m_currentUsableObjectType
			|| !Math::VectorCompare(m_currentUsableObjectMins, objectMins)
			|| !Math::VectorCompare(m_currentUsableObjectMins, objectMaxs))
		{
			if(type != USABLE_OBJECT_NONE)
			{
				m_currentUsableObjectMins = objectMins;
				m_currentUsableObjectMaxs = objectMaxs;
				m_currentUsableObjectType = type;

				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetusableobject, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteByte(m_currentUsableObjectType);
				for(Uint32 i = 0; i < 3; i++)
					gd_engfuncs.pfnMsgWriteFloat(m_currentUsableObjectMins[i]);
				for(Uint32 i = 0; i < 3; i++)
					gd_engfuncs.pfnMsgWriteFloat(m_currentUsableObjectMaxs[i]);
				gd_engfuncs.pfnUserMessageEnd();
			}
			else
			{
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetusableobject, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteByte(USABLE_OBJECT_NONE); // Tell the HUD to stop displaying
				gd_engfuncs.pfnUserMessageEnd();

				m_currentUsableObjectMins.Clear();
				m_currentUsableObjectMaxs.Clear();
			}

			m_currentUsableObjectType = type;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlayerUse( void )
{
	if(!((m_pState->buttons | m_buttonsPressed | m_buttonsReleased) & IN_USE))
		return;

	if(m_pLadderEntity)
	{
		if(m_pLadderEntity->VerifyMove(m_pState->origin, this, 1)
			&& m_pLadderEntity->VerifyMove(m_pState->origin, this, -1))
		{
			if(m_buttonsPressed & IN_USE)
				InitLeaveLadder(LADDER_VR_MOVE_EXIT_USE);
		}
		return;
	}

	// If we're on the bike, exit
	if(m_bikeState)
	{
		if((m_buttonsPressed & IN_USE) && m_bikeState == BIKE_SV_ACTIVE)
			LeaveBike();

		// Prevent any key pressings
 		return;
	}

	if(m_currentPlayerUsableObject)
	{
		Int32 entflags = m_currentPlayerUsableObject->GetEntityFlags();
		if((m_pState->buttons & IN_USE) 
			&& (entflags & FL_ENTITY_CONTINOUS_USE) 
			|| (m_buttonsPressed & IN_USE) 
			&& (entflags & (FL_ENTITY_PLAYER_USABLE|FL_ENTITY_ONOFF_USE)))
			m_currentPlayerUsableObject->CallUse(this, this, USE_SET, 1);
		else if((m_buttonsReleased & IN_USE) && (entflags & FL_ENTITY_ONOFF_USE))
			m_currentPlayerUsableObject->CallUse(this, this, USE_SET, 0);
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::Jump( void )
{
	// No jumping on env_ladders
	if(m_pLadderEntity || (m_pState->flags & FL_WATERJUMP) || !(m_pState->buttons & IN_FORWARD))
		return;

	if(m_bikeState != BIKE_SV_INACTIVE || m_pBikeEntity)
		return;

	if(m_pState->waterlevel >= WATERLEVEL_MID || !(m_pState->buttons & IN_JUMP))
		return;

	if(!(m_pState->flags & FL_ONGROUND) || m_pState->groundent == NO_ENTITY_INDEX)
		return;

	if(m_pState->stamina < PLAYER_MIN_STAMINA)
		return;

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward, nullptr, nullptr);

	m_pState->stamina -= (gSkillData.GetSkillCVarSetting(g_skillcvars.skillStaminaJumpDrain)/100.0f);
	if(m_pState->stamina < 0)
		m_pState->stamina = 0;

	if(m_pState->groundent != NO_ENTITY_INDEX)
		m_pState->velocity = m_pState->velocity + m_pState->basevelocity;

	m_stepSoundRadius += (m_pState->flags & IN_DUCK) ? 100: 500;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::WaterMove( void )
{
	// Don't run if dead or noclipping
	if(m_pState->health < 0 || m_pState->movetype == MOVETYPE_NOCLIP)
		return;

	if(m_pState->waterlevel != WATERLEVEL_FULL)
	{
		if(m_isUnderwaterSoundPlaying)
		{
			// Stop sound on client side
			gd_engfuncs.pfnPlayEntitySound(GetEntityIndex(), "player/pl_underwater.wav", SND_FL_STOP, SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, 0, GetClientIndex());
			m_isUnderwaterSoundPlaying = false;

			Float underwaterTime = g_pGameVars->time - m_underwaterTime;
			m_underwaterTime = 0;
			m_lastWaterDamage = 0;
			m_lastWaterDamageTime = 0;

			// Only make sounds if it was over five seconds
			if(underwaterTime > 5)
			{
				CString soundfile;

				if(underwaterTime <= PLAYER_DROWN_DELAY_TIME*0.5)
				{
					// Play light gasping sounds
					switch(Common::RandomLong(0, 2))
					{
					case 0: soundfile = "player/pl_surface_light1.wav"; break;
					case 1: soundfile = "player/pl_surface_light2.wav"; break;
					case 2: soundfile = "player/pl_surface_light3.wav"; break;
					}
				}
				else if(underwaterTime <= PLAYER_DROWN_DELAY_TIME)
				{
					// Play light gasping sounds
					switch(Common::RandomLong(0, 2))
					{
					case 0: soundfile = "player/pl_surface_medium1.wav"; break;
					case 1: soundfile = "player/pl_surface_medium2.wav"; break;
					case 2: soundfile = "player/pl_surface_medium3.wav"; break;
					}
				}
				else
				{
					// Play light gasping sounds
					switch(Common::RandomLong(0, 2))
					{
					case 0: soundfile = "player/pl_surface_heavy1.wav"; break;
					case 1: soundfile = "player/pl_surface_heavy2.wav"; break;
					case 2: soundfile = "player/pl_surface_heavy3.wav"; break;
					}
				}

				Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_VOICE);
			}

			// Play splash sound
			CString soundfile;
			switch(Common::RandomLong(0, 2))
			{
			case 0: soundfile = "player/pl_water_surface1.wav"; break;
			case 1: soundfile = "player/pl_water_surface2.wav"; break;
			case 2: soundfile = "player/pl_water_surface3.wav"; break;
			}

			Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_ITEM);
		}
		else
		{
			if(m_lastWaterDamageTime)
				m_lastWaterDamageTime = 0;

			if(m_lastWaterDamage)
				m_lastWaterDamage = 0;

			if(m_underwaterTime)
				m_underwaterTime = 0;
		}

		if(m_drownDamageAmount > m_drownDamageHealed)
		{
			// Set drown damage
			m_damageTypes |= DMG_DROWNRECOVER;
			m_damageTypes &= ~DMG_DROWN;

			if(!m_timeBasedDmgTime[TBD_DROWN_RECOVER] && !m_lastTimeBasedDmgTime[TBD_DROWN_RECOVER])
			{
				m_timeBasedDmgTime[TBD_DROWN_RECOVER] = g_pGameVars->time;
				m_lastTimeBasedDmgTime[TBD_DROWN_RECOVER] = g_pGameVars->time;
			}
		}
	}
	else if(!m_isInDreamSequence)
	{
		// If underwater time isn't set, set it
		if(!m_underwaterTime)
			m_underwaterTime = g_pGameVars->time;

		// Don't restore drown dmg when underwater
		m_damageTypes &= ~DMG_DROWNRECOVER;
		m_timeBasedDmgTime[TBD_DROWN_RECOVER] = 0;

		if(m_underwaterTime + PLAYER_DROWN_DELAY_TIME < g_pGameVars->time)
		{
			if(m_lastWaterDamageTime < g_pGameVars->time)
			{
				m_lastWaterDamage += PLAYER_DROWN_RAMP_AMOUNT;
				if(m_lastWaterDamage > PLAYER_DROWN_RAMP_LIMIT)
					m_lastWaterDamage = PLAYER_DROWN_RAMP_LIMIT;

				TakeDamage(nullptr, nullptr, m_lastWaterDamage, DMG_DROWN);
				m_drownDamageAmount += m_lastWaterDamage;
				m_lastWaterDamageTime = g_pGameVars->time + 1;
			}
		}
		else
		{
			// Clear drown damage
			m_damageTypes &= ~DMG_DROWN;
		}

		if(!m_isUnderwaterSoundPlaying)
		{
			// Make a splash
			if(m_pState->waterlevel != m_prevWaterLevel)
			{
				Vector forward, up;
				Math::AngleVectors(m_pState->viewangles, &forward, nullptr, &up);
				Vector splashOrigin = m_pState->origin + m_pState->view_offset + forward*8;
				Util::CreateParticles("water_impact_cluster.txt", splashOrigin, up, PART_SCRIPT_CLUSTER);
			}

			// Shut up voice channel
			Util::EmitEntitySound(this, "common/null.wav", SND_CHAN_VOICE);

			// Play underwater ambience
			Util::EmitEntitySound(this, "player/pl_underwater.wav", SND_CHAN_STATIC);
			m_isUnderwaterSoundPlaying = true;
		}
	}

	// Update waterlevel changes
	if(m_pState->waterlevel != m_prevWaterLevel)
	{
		if(m_pState->waterlevel == WATERLEVEL_FULL && m_prevWaterLevel < WATERLEVEL_FULL)
			m_underwaterTime = g_pGameVars->time;

		m_prevWaterLevel = m_pState->waterlevel;
	}

	if(m_pState->waterlevel == WATERLEVEL_NONE)
	{
		if(m_pState->flags & FL_INWATER)
			m_pState->flags &= ~FL_INWATER;
		
		return;
	}

	// Play specific movement sounds if not in dream sequence
	if(!m_isInDreamSequence)
	{
		if(m_pState->waterlevel == WATERLEVEL_FULL)
		{
			if(m_nextSwimSoundTime < g_pGameVars->time)
			{
				CString soundfile;
				switch(Common::RandomLong(0, 3))
				{
				case 0: soundfile = "player/pl_swim1.wav"; break;
				case 1: soundfile = "player/pl_swim2.wav"; break;
				case 2: soundfile = "player/pl_swim3.wav"; break;
				case 3: soundfile = "player/pl_swim4.wav";break;
				}

				Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_ITEM);
				m_nextSwimSoundTime = g_pGameVars->time + Common::RandomFloat(3, 8);
			}
		}
		else if(m_pState->waterlevel > WATERLEVEL_LOW && !(m_pState->flags & FL_ONGROUND) && !m_pState->velocity.IsZero())
		{
			if(m_nextSwimSoundTime < g_pGameVars->time)
			{
				CString soundfile;
				switch(Common::RandomLong(0, 3))
				{
				case 0: soundfile = "player/pl_wade1.wav"; break;
				case 1: soundfile = "player/pl_wade2.wav"; break;
				case 2: soundfile = "player/pl_wade3.wav"; break;
				case 3: soundfile = "player/pl_wade4.wav";break;
				}

				Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_ITEM);
				m_nextSwimSoundTime = g_pGameVars->time + Common::RandomFloat(2, 4);
			}
		}
	}

	// Do damage for special water types
	if(m_pState->watertype == CONTENTS_LAVA)
	{
		if(m_lastDamageTime + 1 < g_pGameVars->time)
			TakeDamage(nullptr, nullptr, 10*m_pState->waterlevel, DMG_BURN);
	}
	else if(m_pState->watertype == CONTENTS_SLIME)
	{
		if(m_lastDamageTime + 1 < g_pGameVars->time)
			TakeDamage(nullptr, nullptr, 4*m_pState->waterlevel, DMG_ACID);
	}

	if(!(m_pState->flags & FL_INWATER))
		m_pState->flags |= FL_INWATER;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::IsOnLadder( void ) const
{
	return (m_pState->movetype == MOVETYPE_FLY) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::UpdateFlashlight( void )
{
	if(IsFlashlightOn() && !m_pBikeEntity)
	{
		// Subtract each type individually
		if(m_pState->effects & EF_DIMLIGHT)
			m_flashlightBattery -= g_pGameVars->frametime/PLAYER_FLASHLIGHT_DRAIN_TIME;

		if(m_pState->effects & EF_SHOULDERLIGHT)
			m_flashlightBattery -= g_pGameVars->frametime/PLAYER_FLASHLIGHT_DRAIN_TIME_SHOULDERLIGHT;

		// Cap at zero
		if(m_flashlightBattery <= 0)
		{
			TurnFlashlightOff();
			m_flashlightBattery = 0;

			if(m_pActiveWeapon && m_pActiveWeapon->HasFlashlight() && (m_pState->effects & EF_DIMLIGHT))
				m_pActiveWeapon->FlashlightToggle(true);
		}
	}
	else if(m_flashlightBattery != 1.0)
	{
		m_flashlightBattery += g_pGameVars->frametime/PLAYER_FLASHLIGHT_CHARGE_TIME;
		if(m_flashlightBattery >= 1.0)
			m_flashlightBattery = 1.0;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PreCmdThink( void )
{
	// If we're on a camera, disable player
	if(m_pCameraEntity && !(m_pState->flags & FL_FROZEN))
		SetControlEnable(false);
	
	// Manage button states
	Int32 buttonsChanged = m_prevFrameButtons ^ m_pState->buttons;
	m_buttonsPressed = buttonsChanged & m_pState->buttons;
	m_buttonsReleased = buttonsChanged & (~m_pState->buttons);

	if(m_pState->flags & FL_DUCKING || m_pState->health <= 0)
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HULL_MIN, VEC_HULL_MAX);

	// Use think
	PlayerUseThink();

	// Perform ladder thinking
	LadderThink();
	
	// Update sprinting
	SprintThink();

	// Update leaning states
	LeanThink();

	// Update weapons
	WeaponPreFrameThink();
	
	// Update watermove
	WaterMove();

	// Updates flashlight
	UpdateFlashlight();

	// Update client data laast
	UpdateClientData();

	// Updates time based damages
	UpdateTimeBasedDamages();

	// Update bike
	BikeThink();

	// Update healing
	HealThink();

	// Update tape playback
	TapePlaybackThink();

	// Update diary playback
	DiaryPlaybackThink();

	// Update npc awareness
	NPCAwarenessThink();

	// Keep an eye on the global delayed trigger
	DelayedGlobalTriggerThink();

	// Update autoaim
	AutoAimThink();

	// Update dying player state
	if(m_pState->deadstate != DEADSTATE_NONE)
		DeadThink();

	// Alter walkmove when needed
	if(!m_forceSlowMove)
	{
		if(m_pActiveWeapon && (m_pActiveWeapon->ShouldPlayerMoveSlowly() != m_previousWeaponSlowdownState))
		{
			bool desiredState = m_pActiveWeapon->ShouldPlayerMoveSlowly();
			if(desiredState)
			{
				if(m_pState->flags & FL_SLOWMOVE)
					m_walkedSlowBeforeWeaponSlowdown = true;
				else
				{
					m_pState->flags |= FL_SLOWMOVE;
					m_walkedSlowBeforeWeaponSlowdown = false;
				}
			}
			else if(!m_walkedSlowBeforeWeaponSlowdown)
			{
				m_pState->flags &= ~FL_SLOWMOVE;
			}

			m_previousWeaponSlowdownState = desiredState;
		}
		else if(m_buttonsPressed & IN_WALKMODE)
		{
			if(m_pState->flags & FL_SLOWMOVE)
				m_pState->flags &= ~FL_SLOWMOVE;
			else
				m_pState->flags |= FL_SLOWMOVE;
		}
	}

	// Manage jumping
	if(m_pState->buttons & IN_JUMP)
		Jump();

	// Set fall velocity
	if(!(m_pState->flags & FL_ONGROUND))
		m_fallingVelocity = -m_pState->velocity.z;

	// Set HUD visibility
	if(m_pState->weapons == WEAPON_NONE || m_forceHolster || m_pActiveWeapon && m_pActiveWeapon->ShouldHideHUD())
		m_isHUDVisible = false;
	else
		m_isHUDVisible = true;

	// Update music playback
	for(Uint32 i = 0; i < NB_MUSIC_CHANNELS; i++)
	{
		music_data_t& track = m_musicPlaybackInfoArray[i];

		if(track.begintime 
			&& track.filename != NO_STRING_VALUE 
			&& !(track.flags & OGG_FL_LOOP))
		{
			if(g_pGameVars->time - track.begintime > track.duration)
			{
				track.duration = 0;
				track.begintime = 0;
				track.filename = NO_STRING_VALUE;
				track.flags = 0;
				track.channel = 0;
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PostCmdThink( void )
{
	if(!IsAlive())
		return;

	if(m_pState->flags & FL_DUCKING || m_pState->health <= 0)
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HULL_MIN, VEC_HULL_MAX);

	// Update weapon
	WeaponPostFrameThink();

	// Manage fall damage
	if((m_pState->flags & FL_ONGROUND) && m_pState->health > 0 && m_fallingVelocity >= PLAYER_FALL_VELOCITY_PUNCH_MIN)
	{
		// Apply damage if possible
		if(m_fallingVelocity > PLAYER_FALL_VELOCITY_SAFE_LIMIT)
		{
			Float falldmg = m_fallingVelocity -= PLAYER_FALL_VELOCITY_SAFE_LIMIT;
			falldmg *= DAMAGE_FALL_FOR_VELOCITY;

			// Play splatter sounds if over health
			if(falldmg > m_pState->health)
			{
				CString soundfile;
				switch(Common::RandomLong(0, 1))
				{
				case 0: soundfile = "impact/gib_01.wav"; break;
				case 1: soundfile = "impact/gib_02.wav"; break;
				}

				Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_WEAPON);
			}

			if(falldmg > 0)
			{
				// Apply damage
				TakeDamage(nullptr, nullptr, falldmg, DMG_FALL);
			}
		}
	}

	if(m_pState->flags & FL_ONGROUND)
	{
		if(m_fallingVelocity > PLAYER_FALL_SOUND_LIMIT)
			gAISounds.AddSound(AI_SOUND_PLAYER, GetNavigablePosition(), m_fallingVelocity, VOL_NORM, 0.2);
		
		// Reset falling velocity
		m_fallingVelocity = 0;
	}

	// Emit NPC awareness sound
	UpdatePlayerSound();

	// Set prev-frame buttons
	m_prevFrameButtons = m_pState->buttons;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::UpdatePlayerSound( void )
{
	// Set defaults
	Int32 soundTypeBits = AI_SOUND_NONE;
	Float soundRadius = 0;

	if(m_stepSoundRadius)
	{
		m_stepSoundRadius -= PLAYER_SND_RADIUS_DECAY_SPEED * g_pGameVars->frametime;
		if(m_stepSoundRadius < 0)
			m_stepSoundRadius = 0;

		if(m_stepSoundRadius > soundRadius)
			soundRadius = m_stepSoundRadius;
	}

	if(m_weaponSoundRadius > soundRadius)
	{
		soundRadius = m_weaponSoundRadius;
		soundTypeBits |= AI_SOUND_COMBAT;
	}

	if(m_weaponSoundRadius > 0)
	{
		m_weaponSoundRadius -= PLAYER_WEAPON_SND_RADIUS_DECAY_SPEED * g_pGameVars->frametime;
		if(m_weaponSoundRadius < 0)
			m_weaponSoundRadius = 0;
	}

	ai_sound_t* psound = gAISounds.GetSoundForEmitter(this);
	if(!psound)
		return;

	psound->position = GetNavigablePosition();
	psound->typeflags = (AI_SOUND_PLAYER|soundTypeBits);
	psound->volume = VOL_NORM;
	psound->radius = soundRadius;
	psound->life = g_pGameVars->time + 0.1;

	if(m_weaponFlashBrightness)
	{
		m_weaponFlashBrightness -= PLAYER_WEAPON_FLASH_DECAY_SPEED * g_pGameVars->frametime;
		if(m_weaponFlashBrightness < 0)
			m_weaponFlashBrightness = 0;
	}

	m_movementNoise = soundRadius;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::WeaponPreFrameThink( void )
{
	// Do not allow thinking until next action time
	if(m_nextActionTime > g_pGameVars->time)
		return;

	// Do not allow thinking until next weapon think time
	if(m_pActiveWeapon && m_pActiveWeapon->GetWeaponNextThinkTime() > g_pGameVars->time)
		return;

	if(m_pNextWeapon)
	{
		if(ShouldHolster())
		{
			m_pNextWeapon = nullptr;
			return;
		}

		m_pPreviousWeapon = m_pActiveWeapon;
		m_pActiveWeapon = m_pNextWeapon;
		m_pNextWeapon = nullptr;

		if(m_pActiveWeapon)
		{
			m_pActiveWeapon->Deploy();
			m_pActiveWeapon->UpdateClientData(this);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::WeaponPostFrameThink( void )
{
	// Manage any impulse commands
	ManageImpulseCommands();

	if(!m_pActiveWeapon || m_pNextWeapon)
		return;

	// Cancel reload if we should holster
	if(ShouldHolster() && m_pActiveWeapon->IsReloading())
		m_pActiveWeapon->CancelReload();

	// Do not allow thinking until next action time
	if(m_nextActionTime > g_pGameVars->time)
		return;

	// Do not allow thinking until next weapon think time
	if(m_pActiveWeapon->GetWeaponNextThinkTime() > g_pGameVars->time)
		return;

	// If out of ammo and is exhaustible, switch to another weapon
	if(m_pActiveWeapon->GetWeaponFlags() & FL_WEAPON_EXHAUSTIBLE && GetAmmoCount(m_pActiveWeapon->GetAmmoIndex()) <= 0)
	{
		CPlayerWeapon* pFree = m_pActiveWeapon;
		m_pActiveWeapon = nullptr;
		m_pNextWeapon = GetNextBestWeapon(pFree);
		RemoveWeapon(pFree);
		return;
	}

	if(!m_pActiveWeapon)
		return;

	// Perform weapon think functions
	m_pActiveWeapon->PostThink();

	// NOTE: m_pActiveWeapon might be set to null by PostThink
	if(m_pActiveWeapon)
	{
		// Manage forced holstering
		if(ShouldHolster() && m_pFields->viewmodel != NO_STRING_VALUE)
			m_pActiveWeapon->Retire();
		else if(!ShouldHolster() && m_pFields->viewmodel == NO_STRING_VALUE)
			m_pActiveWeapon->Deploy();
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::UpdateTimeBasedDamages( void )
{
	if(!(m_damageTypes & DMG_TIMEBASED))
		return;

	for(Uint32 i = 0; i < NUM_TIMEBASED_DMG; i++)
	{
		if(!(m_damageTypes & TIMEBASED_DMG_BITS[i]))
			continue;

		// Get duration
		Float duration;
		if(i == TBD_DROWN_RECOVER)
		{
			// Duration depends on drown damage amount
			Float drowndiff = m_drownDamageAmount-m_drownDamageHealed;
			duration = SDL_ceil(drowndiff / DROWNRECOVER_MAX_HEAL) * DROWNRECOVER_HEAL_DELAY;
		}
		else
		{
			// Comes from static list
			duration = TIMEBASED_DMG_DURATIONS[i];
		}

		// Clear if possible
		if(m_timeBasedDmgTime[i]+duration < g_pGameVars->time)
		{
			m_timeBasedDmgTime[i] = 0;
			m_lastTimeBasedDmgTime[i] = 0;
			m_damageTypes &= ~TIMEBASED_DMG_BITS[i];
			continue;
		}

		// Drown recovery is handled specially
		if(i == TBD_DROWN_RECOVER)
		{
			if(m_lastTimeBasedDmgTime[i] + DROWNRECOVER_HEAL_DELAY > g_pGameVars->time)
				continue;

			if(m_drownDamageAmount > m_drownDamageHealed)
			{
				Float healamount = m_drownDamageAmount-m_drownDamageHealed;
				if(healamount > DROWNRECOVER_MAX_HEAL)
					healamount = DROWNRECOVER_MAX_HEAL;

				TakeHealth(healamount, DMG_GENERIC);
				m_drownDamageHealed += healamount;
				m_lastTimeBasedDmgTime[i] = g_pGameVars->time;
			}
			else
			{
				m_timeBasedDmgTime[i] = 0;
				m_lastTimeBasedDmgTime[i] = 0;
				m_damageTypes &= ~TIMEBASED_DMG_BITS[i];
			}
		}
		else
		{
			// Don't hurt until next one
			if(m_lastTimeBasedDmgTime[i] + TIMEBASED_DMG_DELAY > g_pGameVars->time)
				continue;

			// Play sizzle sounds for acid
			if(i == TBD_ACID)
			{
				CString soundfile;
				switch(Common::RandomLong(0, 2))
				{
				case 0: soundfile = "misc/acid_sizzle1.wav"; break;
				case 1: soundfile = "misc/acid_sizzle2.wav"; break;
				case 2: soundfile = "misc/acid_sizzle3.wav"; break;
				}

				Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_ITEM);
			}

			Float damage = TIMEBASED_DMG_AMOUNTS[i];
			TakeDamage(nullptr, nullptr, damage, (TIMEBASED_DMG_BITS[i]|DMG_TIMEBASED_INFLICTED));

			m_lastTimeBasedDmgTime[i] = g_pGameVars->time;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SprintThink( void )
{
	if((m_pState->buttons & IN_SPRINT) && m_pState->stamina > 0 && !m_pBikeEntity && m_pState->buttons & IN_FORWARD
		&& (m_pState->velocity.Length() > ((m_pState->flags & FL_DUCKING) ? 50 : PLAYER_EXHAUST_SPEED)))
	{
		Float staminadrain = gSkillData.GetSkillCVarSetting(g_skillcvars.skillStaminaSprintDrain);
		m_pState->stamina -= g_pGameVars->frametime * (staminadrain/100.0f);
		if(m_pState->stamina < 0)
			m_pState->stamina = 0;
	}
	else if(g_pGameVars->frametime > 0 && m_pState->stamina < 1.0)
	{
		Float staminagain = gSkillData.GetSkillCVarSetting(g_skillcvars.skillStaminaSprintDrain);
		m_pState->stamina += g_pGameVars->frametime * (staminagain/100.0f);
		if(m_pState->stamina > 1.0)
			m_pState->stamina = 1.0;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::UpdateClientData( void )
{
	// Update roomtype if needed
	if(m_roomType != m_clientRoomType)
	{
		gd_engfuncs.pfnSetRoomType(m_roomType);
		m_clientRoomType = m_roomType;
	}

	// Update HUD visibility
	if(m_isHUDVisible != m_clientHUDVisible)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetactive, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(m_isHUDVisible);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientHUDVisible = m_isHUDVisible;
	}

	// Update bike
	if(m_bikeState != m_clientBikeState)
	{
		m_clientBikeState = m_bikeState;

		if(m_bikeState)
		{
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteByte(m_bikeState);
			gd_engfuncs.pfnUserMessageEnd();
		}
	}

	// Update nightstage
	if(m_dayStage != m_clientDayStageState)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setdaystage, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(m_dayStage);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientDayStageState = m_dayStage;
	}

	// Restore special fog
	if(m_specialFogEnabled != m_clientSpecialFogEnabled)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setspecialfog, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(m_specialFogEnabled);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientSpecialFogEnabled = m_specialFogEnabled;
	}

	// Update viewmodel
	if(m_pFields->viewmodel != m_clientViewModel)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.viewmodel, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(VMODEL_SET_MODEL);
			gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->viewmodel));
		gd_engfuncs.pfnUserMessageEnd();

		m_clientViewModel = m_pFields->viewmodel;
	}

	// Update medkit counts
	if(m_numMedkits != m_numClientMedkits)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudhealthkit, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(TRUE);
			gd_engfuncs.pfnMsgWriteByte(m_numMedkits);
		gd_engfuncs.pfnUserMessageEnd();

		m_numClientMedkits = m_numMedkits;
	}

	// Update heal progress
	if(m_healProgress != m_clientHealProgress)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudhealthkit, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(FALSE);
			gd_engfuncs.pfnMsgWriteSmallFloat(m_healProgress*100.0f);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientHealProgress = m_healProgress;
	}

	// Update FOV
	if(m_clientFOV != m_fov)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setfov, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(m_fov);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientFOV = m_fov;
	}

	// Update client health
	if(m_pState->health != m_clientHealth)
	{
		Int32 health = m_pState->health;
		if(health < 0)
			health = 0;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudhealth, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(health);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientHealth = m_pState->health;
	}

	// Update stamina
	if(m_pState->stamina != m_clientStamina)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudstamina, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte((Int32)clamp(m_pState->stamina*255, 0, 255));
		gd_engfuncs.pfnUserMessageEnd();

		m_clientStamina = m_pState->stamina;
	}

	// Update kevlar value
	if(m_pState->armorvalue != m_clientKevlar)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudkevlar, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte((Int32)clamp(m_pState->armorvalue, 0, 255));
		gd_engfuncs.pfnUserMessageEnd();

		m_clientKevlar = m_pState->armorvalue;
	}

	// Update flashlight
	if(m_flashlightBattery != m_clientFlashlightBattery)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setflashlight, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(IsFlashlightOn());
			gd_engfuncs.pfnMsgWriteByte((Int32)clamp(m_flashlightBattery*255, 0, 255));
		gd_engfuncs.pfnUserMessageEnd();

		m_clientFlashlightBattery = m_flashlightBattery;
	}

	// Update movement speed
	if(m_movementNoise != m_clientMovementNoise)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.movementnoise, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteInt16(m_movementNoise);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientMovementNoise = m_movementNoise;
	}

	// Update npc awareness
	if(m_highestAwarenessLevel != m_clientHighestAwarenessLevel)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.npcawareness, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteSmallFloat(m_highestAwarenessLevel);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientHighestAwarenessLevel = m_highestAwarenessLevel;
	}

	// Check for weapon updates
	if(m_forceWeaponUpdate)
	{
		// Reset
		m_forceWeaponUpdate = false;

		// Update all weapon data
		for(Uint32 i = 0; i < NUM_WEAPONS; i++)
		{
			weaponinfo_t& weapon = CPlayerWeapon::GetWeaponInfo((weaponid_t)i);
			if(!weapon.id)
				continue;

			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudweaponlist, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteString(weapon.name.c_str());
				gd_engfuncs.pfnMsgWriteChar(CPlayerWeapon::GetAmmoTypeIndex(weapon.ammo.c_str()));
				gd_engfuncs.pfnMsgWriteInt16(weapon.maxammo);
				gd_engfuncs.pfnMsgWriteInt16(weapon.maxclip);
				gd_engfuncs.pfnMsgWriteByte(weapon.slot);
				gd_engfuncs.pfnMsgWriteByte(weapon.position);
				gd_engfuncs.pfnMsgWriteByte(weapon.id);
				gd_engfuncs.pfnMsgWriteByte(weapon.flags);
			gd_engfuncs.pfnUserMessageEnd();
		}
	}

	// Opportunistically update client data for weapons
	if(m_pWeaponsList)
		m_pWeaponsList->UpdateClientData(this);

	m_pClientActiveWeapon = m_pActiveWeapon;

	// Update ammo counts
	UpdateClientAmmoCounts();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::AddPasscode( const Char* pstrid, const Char* pstrpasscode )
{
	// Make sure there's anything to save
	if(!pstrid || !qstrlen(pstrid) || !pstrpasscode || !qstrlen(pstrpasscode))
		return;

	// Make sure passcode isn't too long
	if(qstrlen(pstrpasscode) > MAX_PASSCODE_LENGTH)
	{
		gd_engfuncs.pfnClientPrintf(m_pEdict, "Passcode '%s' is too long.\n", pstrpasscode);
		return;
	}

	// Make sure it's not already in
	for(Uint32 i = 0; i < m_numSavedPasscodes; i++)
	{
		const Char* pstrentry = gd_engfuncs.pfnGetString(m_savedPasscodes[i]);
		const Char* pstrseparator = qstrstr(pstrentry, ";");
		if(!pstrseparator)
			continue; // shouldn't happen

		Uint32 idlength = pstrseparator-pstrentry;
		if(!idlength)
			continue; // again, shouldn't happen

		if(!qstrncmp(pstrentry, pstrid, idlength))
			return;
	}

	if(m_numSavedPasscodes >= MAX_SAVED_PASSCODES)
	{
		gd_engfuncs.pfnClientPrintf(m_pEdict, "Exceeded MAX_SAVED_PASSCODES.\n");
		return;
	}

	// Add a new entry
	CString entry;
	entry << pstrid << ";" << pstrpasscode;

	// Save it
	m_savedPasscodes[m_numSavedPasscodes] = gd_engfuncs.pfnAllocString(entry.c_str());
	m_numSavedPasscodes++;
}

//=============================================
// @brief
//
//=============================================
const Char* CPlayerEntity::GetPasscodeForId( const Char* pstrid )
{
	if(!pstrid)
		return nullptr;

	for(Uint32 i = 0; i < m_numSavedPasscodes; i++)
	{
		if(m_savedPasscodes[i] == NO_STRING_VALUE)
			continue;

		const Char* pstrentry = gd_engfuncs.pfnGetString(m_savedPasscodes[i]);
		const Char* pstrseparator = qstrstr(pstrentry, ";");
		if(!pstrseparator)
			continue; // shouldn't happen

		Uint32 idlength = pstrseparator-pstrentry;
		if(!idlength)
			continue; // again, shouldn't happen

		if(!qstrncmp(pstrentry, pstrid, idlength))
			return (pstrseparator+1);
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::DumpAllCodes( void )
{
	for(Uint32 i = 0; i < m_numSavedPasscodes; i++)
	{
		if(m_savedPasscodes[i] == NO_STRING_VALUE)
			continue;

		const Char* pstrentry = gd_engfuncs.pfnGetString(m_savedPasscodes[i]);
		const Char* pstrseparator = qstrstr(pstrentry, ";");
		if(!pstrseparator)
			continue; // shouldn't happen

		Uint32 idlength = pstrseparator-pstrentry;
		if(!idlength)
			continue; // again, shouldn't happen

		CString codeid(pstrentry, idlength);
		CString code(pstrseparator+1, qstrlen(pstrseparator+1));

		gd_engfuncs.pfnClientPrintf(m_pEdict, "%d - Code id: %s, code: %s.\n", (Int32)(i+1), codeid.c_str(), code.c_str());
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SpawnKeypadWindow( const Char* pstrid, const Char* pstrkeypadcode, CTriggerKeypad* pkeypad, bool staytillnext )
{
	// If id is null, get it from entity
	CString keypadcode;
	if(!pstrid || !qstrlen(pstrid))
	{
		if(pstrkeypadcode && qstrlen(pstrkeypadcode)  > 0)
		{
			// Make sure passcode isn't too long
			if(qstrlen(pstrkeypadcode) > MAX_PASSCODE_LENGTH)
			{
				gd_engfuncs.pfnClientPrintf(m_pEdict, "Passcode '%s' is too long.\n", pstrkeypadcode);
				return;
			}

			keypadcode = pstrkeypadcode;
		}
	}
	else
	{
		// Get from the saved list
		keypadcode = GetPasscodeForId(pstrid);
	}

	// Remember the keypad entity
	m_pKeypadEntity = pkeypad;

	// Write message to client
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategameuiwindow, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(GAMEUI_KEYPADWINDOW);
		gd_engfuncs.pfnMsgWriteString(keypadcode.c_str());
		gd_engfuncs.pfnMsgWriteByte(staytillnext ? 1 : 0);
	gd_engfuncs.pfnUserMessageEnd();

	m_hasActiveUIWindows = true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SpawnSubwayWindow( subwayline_t type, CTriggerSubwayController* pcontroller, bool isdummy )
{
	Int32 flags = m_subwayFlags;
	if(isdummy)
		flags |= FL_SUBWAY_DISABLED;

	CString scriptfilepath;
	scriptfilepath << GAMEUI_SCRIPT_BASE_PATH << PATH_SLASH_CHAR << SUBWAYWINDOW_SCRIPT_SUBFOLDER_NAME << PATH_SLASH_CHAR << "subway";

	switch(type)
	{
	case SUBWAYLINE_BERGEN_ECKHART:
		scriptfilepath << "_bergen_eckhart.txt";
		break;
	case SUBWAYLINE_KASSAR_STILLWELL:
		scriptfilepath << "_kassar_stillwell.txt";
		break;
	case SUBWAYLINE_MARSHALL_LYNE:
		scriptfilepath << "_marshall_lyne.txt";
		break;
	default:
		{
			gd_engfuncs.pfnClientPrintf(m_pEdict, "Unknown subway line specified.\n");
			return;
		}
		break;
	}

	// Write message to client
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategameuiwindow, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(GAMEUI_SUBWAYWINDOW);
		gd_engfuncs.pfnMsgWriteString(scriptfilepath.c_str());
		gd_engfuncs.pfnMsgWriteByte(flags);
		gd_engfuncs.pfnMsgWriteByte(type);
	gd_engfuncs.pfnUserMessageEnd();

	// Remember this
	m_pSubwayController = pcontroller;
	m_hasActiveUIWindows = true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SpawnTextWindow( const Char* pstrfilepath, const Char* pstrpasscode, const Char* pstrid )
{
	// Add pass code if present
	if(pstrpasscode && qstrlen(pstrpasscode) > 0 && pstrid && qstrlen(pstrid) > 0)
		AddPasscode(pstrid, pstrpasscode);

	// Write message to client
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategameuiwindow, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(GAMEUI_TEXTWINDOW);
		gd_engfuncs.pfnMsgWriteString(pstrfilepath);
		if(pstrpasscode && qstrlen(pstrpasscode))
		{
			gd_engfuncs.pfnMsgWriteByte(TRUE);
			gd_engfuncs.pfnMsgWriteString(pstrpasscode);
		}
		else
			gd_engfuncs.pfnMsgWriteByte(FALSE);
	gd_engfuncs.pfnUserMessageEnd();

	m_hasActiveUIWindows = true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SpawnLoginWindow( const Char* pstruser, const Char* pstrpasscode, const Char* pstrid, CTriggerLogin* plogin, bool staytillnext )
{
	CString passcode;
	if(!pstrid || !qstrlen(pstrid))
	{
		if(!pstrpasscode)
		{
			gd_engfuncs.pfnClientPrintf(m_pEdict, "No passcode or passcode id specified.\n");
			return;
		}

		if(qstrlen(pstrpasscode) > MAX_PASSCODE_LENGTH)
		{
			gd_engfuncs.pfnClientPrintf(m_pEdict, "Code '%s' is too long.\n", pstrpasscode);
			return;
		}

		passcode = pstrpasscode;
	}
	else
	{
		const Char* _pstrpasscode = GetPasscodeForId(pstrid);
		if(_pstrpasscode)
			passcode = _pstrpasscode;

		if(passcode.length() > MAX_PASSCODE_LENGTH)
		{
			gd_engfuncs.pfnClientPrintf(m_pEdict, "Code '%s' is too long.\n", pstrpasscode);
			return;
		}
	}

	// Write message to client
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategameuiwindow, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(GAMEUI_LOGINWINDOW);
		gd_engfuncs.pfnMsgWriteString(pstruser);
		gd_engfuncs.pfnMsgWriteString(passcode.c_str());
		gd_engfuncs.pfnMsgWriteByte(staytillnext ? 1 : 0);
	gd_engfuncs.pfnUserMessageEnd();

	m_pLoginEntity = plogin;
	m_hasActiveUIWindows = true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetSubwayFlag( Int32 flag )
{
	m_subwayFlags |= flag;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::DestroyGameUIWindows( void )
{
	if(!m_hasActiveUIWindows)
		return;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategameuiwindow, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(GAMEUI_KILLWINDOWS);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ManageGameUIEvent( CMSGReader& reader )
{
	gameui_player_event_t eventtype = (gameui_player_event_t)reader.ReadByte();
  	switch(eventtype)
	{
	case GAMEUIEVENT_CLOSED_ALL_WINDOWS:
		m_hasActiveUIWindows = false;
		break;
	case GAMEUIEVENT_CODE_MATCHES:
		{
			gameui_windows_t windowtype = (gameui_windows_t)reader.ReadByte();
			switch(windowtype)
			{
			case GAMEUI_KEYPADWINDOW:
				{
					if(m_pKeypadEntity)
					{
						// Just fire the target
						m_pKeypadEntity->FireTarget(this);
						m_pKeypadEntity = nullptr;
					}
				}
				break;
			case GAMEUI_LOGINWINDOW:
				{
					if(m_pLoginEntity)
					{
						// Just fire the target
						m_pLoginEntity->FireTarget(this);
						m_pLoginEntity = nullptr;
					}
				}
				break;
			}
		}
		break;
	case GAMEUIEVENT_SUBWAY_SELECTION:
		{
			const Char* pstrDestination = reader.ReadString();

			if(m_pSubwayController)
			{
				if(!pstrDestination || !qstrlen(pstrDestination))
				{
					gd_engfuncs.pfnClientPrintf(m_pEdict, "No subway destination specified.\n");
					return;
				}

				// Trigger the subway target
				m_pSubwayController->FireTarget(this, pstrDestination);
				m_pSubwayController = nullptr;
			}
		}
		break;
	case GAMEUIEVENT_READ_OBJECTIVE:
		{
			const Char* pstrObjectiveIdentifier = reader.ReadString();

			for(Uint32 i = 0; i < GAMEUI_MAX_OBJECTIVES; i++)
			{
				if(m_objectivesArray[i] == NO_STRING_VALUE)
					break;

				if(!qstrcmp(gd_engfuncs.pfnGetString(m_objectivesArray[i]), pstrObjectiveIdentifier))
				{
					m_objectivesNewFlags &= ~(1<<i);
					break;
				}
			}
		}
		break;
	default:
		gd_engfuncs.pfnClientPrintf(m_pEdict, "Unknown or unhandled window type sent from client.\n");
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::InitStepSounds( void )
{
	Uint32 filesize = 0;
	const byte* pfile = gd_filefuncs.pfnLoadFile(FOOTSTEP_SCRIPT_FILE, &filesize);
	if(!pfile)
	{
		gd_engfuncs.pfnClientPrintf(m_pEdict, "%s - Could not load '%s'.\n", __FUNCTION__, FOOTSTEP_SCRIPT_FILE);
		return;
	}

	if(!m_pstepSound.Init(reinterpret_cast<const Char*>(pfile), filesize))
		gd_engfuncs.pfnClientPrintf(m_pEdict, "%s - Failed to initialize CStepSound with '%s': %s.\n", __FUNCTION__, FOOTSTEP_SCRIPT_FILE, m_pstepSound.GetInfoString().c_str());

	if(!m_pstepSound.IsInfoStringEmpty())
		gd_engfuncs.pfnCon_Printf(m_pstepSound.GetInfoString().c_str());

	gd_filefuncs.pfnFreeFile(pfile);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlayStepSound( const Char* pstrMaterialName, bool stepleft, Float volume, const Vector& origin )
{
	// Get appropriate sound
	CStepSound::foot_t foot = stepleft ? CStepSound::FOOT_LEFT : CStepSound::FOOT_RIGHT;
	const CArray<CString>* pSoundsArray = m_pstepSound.GetFootSoundList(foot, pstrMaterialName);
	if(!pSoundsArray || pSoundsArray->empty())
		return;

	Uint32 irand = Common::RandomLong(0, pSoundsArray->size()-1);
	const CString& sound = (*pSoundsArray)[irand];

	for(Int32 i = 0; i < g_pGameVars->maxclients; i++)
	{
		// Send for each client, except the predicted one
		if(i == g_pGameVars->predict_player)
			continue;

		gd_engfuncs.pfnPlayEntitySound(m_pState->entindex, sound.c_str(), SND_FL_NONE, SND_CHAN_BODY, volume, ATTN_NORM, PITCH_NORM, 0, i);
	}

	// Update step sound
	Float soundVolume = 0;
	if(m_pState->flags & FL_ONGROUND)
	{
		soundVolume = m_pState->velocity.Length()*4;
		if(soundVolume > PLAYER_SOUND_MAX_RADIUS)
			soundVolume = PLAYER_SOUND_MAX_RADIUS;

		if(m_pState->flags & FL_SLOWMOVE)
			soundVolume *= 0.25;
		else if(m_pState->flags & FL_DUCKING)
			soundVolume *= 0.3;
	}

	if(soundVolume > m_stepSoundRadius)
		m_stepSoundRadius = soundVolume;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlaySound( Int32 channel, const Char* psample, Float volume, Float attenuation, Int32 pitch, Int32 flags )
{
	for(Int32 i = 0; i < g_pGameVars->maxclients; i++)
	{
		// Send for each client, except the predicted one
		if(i == g_pGameVars->predict_player)
			continue;

		gd_engfuncs.pfnPlayEntitySound(m_pState->entindex, psample, flags, channel, volume, attenuation, pitch, 0, i);
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::HostSay( const Char* pstrText, bool teamonly )
{
	const Char* pstrnetname = gd_engfuncs.pfnGetString(m_pFields->netname);
	if(!pstrnetname)
		return;

	for(Int32 i = 1; i < gd_engfuncs.pfnGetNbEdicts(); i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(!(pedict->state.flags & FL_CLIENT))
			break;

		if(pedict == m_pEdict)
			continue;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.saytext, nullptr, pedict);
			gd_engfuncs.pfnMsgWriteString(pstrnetname);
			gd_engfuncs.pfnMsgWriteString(pstrText);
		gd_engfuncs.pfnUserMessageEnd();
	}

	// Send to client who wrote this
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.saytext, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteString(pstrnetname);
		gd_engfuncs.pfnMsgWriteString(pstrText);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetForceHolster( bool forceholster )
{
	m_forceHolster = forceholster;
	if(!m_forceHolster && !m_pActiveWeapon)
	{
		m_pActiveWeapon = GetNextBestWeapon(nullptr);
		if(m_pActiveWeapon)
			m_pActiveWeapon->Deploy();
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetForceSlowMove( bool forceslowmove, bool nosprinting )
{
	m_forceSlowMove = forceslowmove;
	if(!m_forceSlowMove)
	{
		Int32 removeFlags = 0;
		if(nosprinting)
			removeFlags |= FL_NO_SPRINT;
		if(!m_walkedSlowBeforeSlowMove)
			removeFlags |= FL_SLOWMOVE;

		m_pState->flags &= ~removeFlags;
		m_walkedSlowBeforeSlowMove = false;
		m_walkedSlowBeforeWeaponSlowdown = false;
	}
	else
	{
		m_walkedSlowBeforeSlowMove = (m_pState->flags & FL_SLOWMOVE) ? true : false;
		m_walkedSlowBeforeWeaponSlowdown = false;

		Int32 setFlags = FL_SLOWMOVE;
		if(nosprinting)
			setFlags |= FL_NO_SPRINT;

		m_pState->flags |= setFlags;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetIsInDream( bool isindream )
{
	m_isInDreamSequence = isindream;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetSaveGameTitle( const Char* pstrtitle )
{
	m_saveGameTitle = gd_engfuncs.pfnAllocString(pstrtitle);
}

//=============================================
// @brief
//
//=============================================
const Char* CPlayerEntity::GetSaveGameTitle( void )
{
	return gd_engfuncs.pfnGetString(m_saveGameTitle);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetRoomType( Int32 roomtype )
{
	m_roomType = roomtype;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SelectWeapon( const Char* pstrWeaponName )
{
	if(!pstrWeaponName || !qstrlen(pstrWeaponName))
		return;

	CPlayerWeapon* pWeapon = m_pWeaponsList;
	while(pWeapon)
	{
		if(!qstrcmp(pWeapon->GetClassName(), pstrWeaponName))
			break;

		pWeapon = pWeapon->GetNextWeapon();
	}

	if(!pWeapon)
		return;

	if(m_pActiveWeapon == pWeapon)
		return;

	if(m_pActiveWeapon)
	{
		if(!m_pNextWeapon)
			m_pActiveWeapon->Holster();

		if(g_pCvarWeaponHolster->GetValue() >= 1)
		{
			// Mark next weapon to use
			m_pNextWeapon = pWeapon;
		}
		else
		{
			// Set pointers
			m_pPreviousWeapon = m_pActiveWeapon;
			m_pActiveWeapon = pWeapon;

			// Make weapon deploy immediately
			m_pActiveWeapon->Deploy();
			m_pActiveWeapon->UpdateClientData(this);
		}
	}
	else
	{
		m_pNextWeapon = pWeapon;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SelectWeapon( weaponid_t weaponId )
{
	if(weaponId == WEAPON_NONE)
		return;

	CPlayerWeapon* pWeapon = m_pWeaponsList;
	while(pWeapon)
	{
		if(pWeapon->GetId() == weaponId)
			break;

		pWeapon = pWeapon->GetNextWeapon();
	}

	if(!pWeapon)
		return;

	if(m_pActiveWeapon)
	{
		if(!m_pNextWeapon)
			m_pActiveWeapon->Holster();

		if(g_pCvarWeaponHolster->GetValue() >= 1)
		{
			// Mark next weapon to use
			m_pNextWeapon = pWeapon;
		}
		else
		{
			// Set pointers
			m_pPreviousWeapon = m_pActiveWeapon;
			m_pActiveWeapon = pWeapon;

			// Make weapon deploy immediately
			m_pActiveWeapon->Deploy();
			m_pActiveWeapon->UpdateClientData(this);
		}
	}
	else
	{
		m_pNextWeapon = pWeapon;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SelectPreviousWeapon( void )
{
	if(!m_pPreviousWeapon)
		return;

	if(m_pActiveWeapon && !m_pActiveWeapon->CanHolster())
		return;

	if(m_pActiveWeapon)
	{
		if(!m_pNextWeapon)
			m_pActiveWeapon->Holster();

		if(g_pCvarWeaponHolster->GetValue() >= 1)
		{
			// Mark next weapon to use
			m_pNextWeapon = m_pPreviousWeapon;
		}
		else
		{
			// Set pointers
			CPlayerWeapon* pPrev = m_pPreviousWeapon;
			m_pPreviousWeapon = m_pActiveWeapon;
			m_pActiveWeapon = pPrev;

			// Make weapon deploy immediately
			m_pActiveWeapon->Deploy();
			m_pActiveWeapon->UpdateClientData(this);
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::HasAnyWeapons( void ) const
{
	if(!m_pWeaponsList)
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::GiveItemByName( const Char* pstrClassname, Uint32 amount, bool removeunneeded )
{
	if(!AreCheatsEnabled())
		return;

	for(Uint32 i = 0; i < amount; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnCreateEntity(pstrClassname);
		if(Util::IsNullEntity(pedict))
		{
			gd_engfuncs.pfnClientPrintf(m_pEdict, "Could not create item '%s'.\n", pstrClassname);
			return;
		}

		CBaseEntity* pItem = CBaseEntity::GetClass(pedict);
		if(!pItem)
			return;

		pItem->SetOrigin(m_pState->origin);

		Vector angles = m_pState->angles;
		angles[ROLL] = angles[PITCH] = 0;
		pItem->SetAngles(angles);

		// For debugging on NPCs
		m_cheatCommandUsed = true;

		DispatchSpawn(pedict);
		DispatchTouch(pedict, m_pEdict);

		m_cheatCommandUsed = false;

		if(removeunneeded)
		{
			if(!(pedict->state.effects & EF_NODRAW))
				pItem->FlagForRemoval();
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ManageFlashlight( void )
{
	if(m_pBikeEntity)
	{
		if(IsFlashlightOn())
			TurnFlashlightOff();
		else
			TurnFlashlightOn();
	}
	else if(m_hasShoulderLight)
	{
		Util::EmitEntitySound(this, FLASHLIGHT_TOGGLE_SOUND, SND_CHAN_WEAPON);

		if(IsFlashlightOn(false, true))
			TurnFlashlightOff(false, true);
		else
			TurnFlashlightOn(true);
	}
	else if(m_pActiveWeapon 
		&& !ShouldHolster())
	{
		if(!m_pActiveWeapon->HasFlashlight())
		{
			CPlayerWeapon* pWeapon = m_pWeaponsList;
			while(pWeapon)
			{
				if(pWeapon->HasFlashlight())
					break;

				pWeapon = pWeapon->GetNextWeapon();
			}

			if(pWeapon)
			{
				pWeapon->SetFlashlightRequest(true);
				m_pActiveWeapon->Holster();

				if(g_pCvarWeaponHolster->GetValue() >= 1)
				{
					// Mark next weapon to use
					m_pNextWeapon = pWeapon;
				}
				else
				{
					// Set pointers
					m_pPreviousWeapon = m_pActiveWeapon;
					m_pActiveWeapon = pWeapon;

					// Make weapon deploy immediately
					m_pActiveWeapon->Deploy();
					m_pActiveWeapon->UpdateClientData(this);
				}

				return;
			}
		}
		else
		{
			// Tell it to toggle the flashlight
			m_pActiveWeapon->FlashlightToggle();
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::IsFlashlightOn( bool onlyDimLight, bool onlyShoulderLight ) const
{
	if(onlyShoulderLight)
		return (m_pState->effects & EF_SHOULDERLIGHT) ? true : false;
	else if(onlyDimLight)
		return (m_pState->effects & EF_DIMLIGHT) ? true : false;
	else
		return (m_pState->effects & (EF_DIMLIGHT|EF_SHOULDERLIGHT)) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::TurnFlashlightOn( bool isShoulderLight )
{
	if(isShoulderLight && !m_hasShoulderLight)
		return false;
	
	if(!m_flashlightBattery)
		return false;

	if(isShoulderLight)
		m_pState->effects |= EF_SHOULDERLIGHT;
	else
		m_pState->effects |= EF_DIMLIGHT;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setflashlight, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(TRUE);
		gd_engfuncs.pfnMsgWriteByte((Int32)clamp(m_flashlightBattery*255, 0, 255));
	gd_engfuncs.pfnUserMessageEnd();

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::TurnFlashlightOff( bool onlyDimLight, bool onlyShoulderLight )
{
	if(onlyShoulderLight)
		m_pState->effects &= ~EF_SHOULDERLIGHT;
	else if(onlyDimLight)
		m_pState->effects &= ~EF_DIMLIGHT;
	else
		m_pState->effects &= ~(EF_SHOULDERLIGHT|EF_DIMLIGHT);

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setflashlight, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(FALSE);
		gd_engfuncs.pfnMsgWriteByte((Int32)clamp(m_flashlightBattery*255, 0, 255));
	gd_engfuncs.pfnUserMessageEnd();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ManageImpulseCommands( void )
{
	// Do not allow use while disabled
	if(m_pState->flags & FL_FROZEN)
		return;

	// Manage player use
	PlayerUse();

	switch(m_pState->impulse)
	{
	case FLASHLIGHT_IMPULSE_VALUE:
		ManageFlashlight();
		break;
	default:
		ManageCheatImpulseCommands(m_pState->impulse);
		break;
	}

	// Reset this
	m_pState->impulse = 0;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ManageCheatImpulseCommands( Int32 impulse )
{
	if(!impulse)
		return;

	if(!AreCheatsEnabled() && impulse != PLAYER_CHEATCODE_REPORT_AI_STATE)
		return;

	switch(impulse)
	{
	case PLAYER_CHEATCODE_ALLWEAPONS:
		{
			GiveItemByName("item_kevlar", 5, true);
			GiveItemByName("item_healthkit", 5, true);
			GiveItemByName("item_shoulderlight", 1, true);

			GiveItemByName("weapon_knife", 1, true);
			GiveItemByName("weapon_glock", 1, true);
			GiveItemByName("ammo_glock_clip", 4, true);
			GiveItemByName("item_glock_silencer", 1, true);
			GiveItemByName("item_glock_flashlight", 1, true);
			GiveItemByName("weapon_handgrenade", 4, true);
		}
		break;
	case PLAYER_CHEATCODE_DUMP_CODES:
		DumpAllCodes();
		break;
	case PLAYER_CHEATCODE_DUMPGLOBALS:
		DumpGlobals();
		break;
	case PLAYER_CHEATCODE_TRACE_TEXTURE:
		{
			Vector forward;
			Math::AngleVectors(m_pState->viewangles, &forward, nullptr, nullptr);

			Vector traceBegin = m_pState->origin + m_pState->view_offset;
			Vector traceEnd = traceBegin + forward * 8192;

			trace_t tr;
			Util::TraceLine(traceBegin, traceEnd, true, false, m_pEdict, tr);

			if(!tr.noHit() && !tr.startSolid() && !tr.allSolid() && tr.hitentity != NO_ENTITY_INDEX)
			{
				const Char* pstrTextureName = Util::TraceTexture(tr.hitentity, tr.endpos, tr.plane.normal);
				if(pstrTextureName)
				{
					edict_t* pentity = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
					if(pentity)
					{
						const Char* pstrClassName = gd_engfuncs.pfnGetString(pentity->fields.classname);
						gd_engfuncs.pfnClientPrintf(m_pEdict, "Entity: %s - Texture %s.\n", pstrClassName, pstrTextureName);
					}
				}
			}
		}
		break;
	case PLAYER_CHEATCODE_REPORT_AI_STATE:
		{
			Vector forward;
			Math::AngleVectors(m_pState->viewangles, &forward, nullptr, nullptr);

			Vector traceBegin = m_pState->origin + m_pState->view_offset;
			Vector traceEnd = traceBegin + forward * 8192;

			trace_t tr;
			Util::TraceLine(traceBegin, traceEnd, false, true, m_pEdict, tr);
			if(tr.noHit() || tr.startSolid() || tr.allSolid())
			{
				gd_engfuncs.pfnCon_Printf("Nothing was hit.\n");
				return;
			}

			CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
			if(!pEntity || !pEntity->IsNPC())
			{
				gd_engfuncs.pfnCon_Printf("Entity was null or not an NPC.\n");
				return;
			}

			pEntity->ReportAIState();
		}
		break;
	case PLAYER_CHEATCODE_GET_NEAREST_NODE_INDEX:
		{
			Int32 nodeIndex = gNodeGraph.GetNearestNode(GetNavigablePosition());
			if(nodeIndex == NO_POSITION)
			{
				// No node was found
				gd_engfuncs.pfnCon_Printf("No nearby node.\n");
			}
			else
			{
				const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
				gd_engfuncs.pfnCon_Printf("Nearest node index at %.2f %.2f %.2f is %d, pre-link index was %d.\n", pNode->origin[0], pNode->origin[1], pNode->origin[2], nodeIndex, pNode->origindex);
			}
		}
		break;
	case PLAYER_CHEATCODE_SHOW_SMALL_HULL_NEAREST_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNearestNodeBBox(m_pState->origin, NODE_SMALL_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_FLY_HULL_NEAREST_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNearestNodeBBox(m_pState->origin, NODE_FLY_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_LARGE_HULL_NEAREST_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNearestNodeBBox(m_pState->origin, NODE_LARGE_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NEAREST_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNearestNodeBBox(m_pState->origin, NODE_HUMAN_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNodeBBoxes(NODE_SMALL_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNodeBBoxes(NODE_FLY_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNodeBBoxes(NODE_LARGE_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_MINS_MAXS:
		{
			gNodeGraph.ShowNodeBBoxes(NODE_HUMAN_HULL);
		}
		break;
	case PLAYER_CHEATCODE_SHOW_SMALL_HULL_NODE_PATHS:
		{
			CBaseEntity* pEntity = CBaseEntity::CreateEntity("node_viewer_small", m_pState->origin, m_pState->angles, nullptr);
			if(pEntity)
				pEntity->Spawn();
		}
		break;
	case PLAYER_CHEATCODE_SHOW_FLY_HULL_NODE_PATHS:
		{
			CBaseEntity* pEntity = CBaseEntity::CreateEntity("node_viewer_fly", m_pState->origin, m_pState->angles, nullptr);
			if(pEntity)
				pEntity->Spawn();
		}
		break;
	case PLAYER_CHEATCODE_SHOW_LARGE_HULL_NODE_PATHS:
		{
			CBaseEntity* pEntity = CBaseEntity::CreateEntity("node_viewer_large", m_pState->origin, m_pState->angles, nullptr);
			if(pEntity)
				pEntity->Spawn();
		}
		break;
	case PLAYER_CHEATCODE_SHOW_HUMAN_HULL_NODE_PATHS:
		{
			CBaseEntity* pEntity = CBaseEntity::CreateEntity("node_viewer_human", m_pState->origin, m_pState->angles, nullptr);
			if(pEntity)
				pEntity->Spawn();
		}
		break;
	case PLAYER_CHEATCODE_SHOW_ALL_NODE_PATHS:
		{
			gNodeGraph.ShowAllNodeConnections();
		}
		break;
	case PLAYER_CHEATCODE_REMOVE_ENTITY:
		{
			Vector traceBegin = m_pState->origin + m_pState->view_offset;
			CBaseEntity* pEntity = Util::FindEntityAtDirection(traceBegin, m_pState->viewangles, m_pEdict);
			if(pEntity && pEntity->GetTakeDamage() != TAKEDAMAGE_NO)
			{
				const Char* pstrClassname = pEntity->GetClassName();
				const Char* pstrTargetname = pEntity->GetTargetName();

				if(pstrTargetname && qstrlen(pstrTargetname))
					gd_engfuncs.pfnCon_Printf("Removing entity %s with targetname %s.\n", pstrClassname, pstrTargetname);
				else
					gd_engfuncs.pfnCon_Printf("Removing entity %s.\n", pstrClassname);

				Util::RemoveEntity(pEntity);
			}
		}
		break;
	case PLAYER_CHEATCODE_GRANT_ALL_SUBWAY_STOPS:
		{
			m_subwayFlags = (FL_SUBWAY_GOT_BERGENST|FL_SUBWAY_GOT_IBMANNST|FL_SUBWAY_GOT_ECKHARTST|FL_SUBWAY_GOT_MARSHALLST);
		}
		break;
	case PLAYER_CHEATCODE_NAME_ENTITY_IN_FRONT:
		{
			Vector traceBegin = m_pState->origin + m_pState->view_offset;
			CBaseEntity* pEntity = Util::FindEntityAtDirection(traceBegin, m_pState->viewangles, m_pEdict);
			if(pEntity)
			{
				const Char* pstrClassname = pEntity->GetClassName();
				const Char* pstrTargetname = pEntity->GetTargetName();
				const Char* pstrTarget = pEntity->GetTarget();
				const Char* pstrModelname = pEntity->GetModelName();

				CString str;
				str << "Entity in front - Classname: " << pstrClassname << " - Entity index: " << pEntity->GetEntityIndex();
				if(pstrTargetname && qstrlen(pstrTargetname))
					str << " - Targetname: " << pstrTargetname;
				if(pstrTarget && qstrlen(pstrTarget))
					str << " - Target: " << pstrTarget;
				if(pstrModelname && qstrlen(pstrModelname))
					str << " - Model: " << pstrModelname << ", Model index: " << pEntity->GetModelIndex();

				gd_engfuncs.pfnCon_Printf("%s.\n", str.c_str());
			}
		}
		break;
	}
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GiveAmmo( Int32 amount, const Char* pstrammoname, Int32 max, bool display, CBaseEntity* pentity )
{
	if(!pstrammoname || !qstrlen(pstrammoname))
		return NO_AMMO_INDEX;

	if(!CanHaveAmmo(pstrammoname, max))
		return NO_AMMO_INDEX;

	Int32 ammoindex = CPlayerWeapon::GetAmmoTypeIndex(pstrammoname);
	if(ammoindex < 0 || ammoindex >= MAX_AMMO_TYPES)
		return NO_AMMO_INDEX;

	// Calculate how much we need to add
	Int32 ammomissing = max - m_ammoCounts[ammoindex];
	Int32 numadd = (amount > ammomissing) ? ammomissing : amount;
	if(!numadd)
		return ammoindex;

	// Add to cache
	m_ammoCounts[ammoindex] += numadd;

	// Play sound if needed
	if(!pentity->HasSpawnFlag(CPlayerWeapon::FL_WEAPON_NO_NOTICE))
		Util::EmitEntitySound(this, AMMO_PICKUP_SOUND, SND_CHAN_ITEM);

	if(display)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudammopickup, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteString(pentity->GetClassName());
			gd_engfuncs.pfnMsgWriteByte(numadd);
		gd_engfuncs.pfnUserMessageEnd();
	}

	return ammoindex;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GetAmmoCount( Int32 ammotype ) const
{
	if(ammotype == NO_AMMO_INDEX)
		return NO_AMMO_INDEX;

	assert(ammotype < MAX_AMMO_TYPES);
	return m_ammoCounts[ammotype];
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::RemoveAmmo( Int32 ammotype, Uint32 numremove )
{
	if(ammotype == NO_AMMO_INDEX)
		return;

	assert(ammotype < MAX_AMMO_TYPES);
	m_ammoCounts[ammotype] -= _max(m_ammoCounts[ammotype], (Int32)numremove);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetAmmoCount( Int32 ammotype, Int32 ammocount )
{
	if(ammotype == NO_AMMO_INDEX)
		return;

	assert(ammotype < MAX_AMMO_TYPES);
	m_ammoCounts[ammotype] = ammocount;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::UpdateClientAmmoCounts( void )
{
	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if(m_ammoCounts[i] == m_clientAmmoCounts[i])
			continue;

		assert(m_ammoCounts[i] >= 0 && m_ammoCounts[i] < 255);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudammocount, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(i);
			gd_engfuncs.pfnMsgWriteByte(clamp(m_ammoCounts[i], 0, 254)); // 255 is reserved for -1 maxammo
		gd_engfuncs.pfnUserMessageEnd();

		m_clientAmmoCounts[i] = m_ammoCounts[i];
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::RemovePlayerWeapon( CPlayerWeapon* pWeapon )
{
	if(m_pActiveWeapon == pWeapon)
	{
		pWeapon->Holster();
		pWeapon->ClearThinkFunctions();
		ClearCurrentWeapon();

		m_pActiveWeapon = nullptr;
		m_pFields->viewmodel = 0;
	}
	else if(m_pPreviousWeapon == pWeapon)
		m_pPreviousWeapon = nullptr;
	else if(m_pNextWeapon == pWeapon)
		m_pNextWeapon = nullptr;

	CPlayerWeapon* pPrev = nullptr;
	CPlayerWeapon* pNext = m_pWeaponsList;
	while(pNext)
	{
		if(pNext == pWeapon)
		{
			if(!pPrev)
			{
				// Reset header to next one in link
				m_pWeaponsList = pNext->GetNextWeapon();
			}
			else
			{
				// Remove from link
				pPrev->SetNextWeapon(pNext->GetNextWeapon());
			}

			pNext->SetNextWeapon(nullptr);
			RemoveWeaponBit(pWeapon->GetId());
			return true;
		}

		pPrev = pNext;
		pNext = pNext->GetNextWeapon();
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::CanHaveWeapon( CPlayerWeapon* pWeapon ) const
{
	if(m_pState->deadstate != DEADSTATE_NONE)
		return false;
	
	const Char* pstrAmmoType = pWeapon->GetAmmoTypeName();
	if(qstrlen(pWeapon->GetAmmoTypeName()) > 0)
	{
		if(!CanHaveAmmo(pstrAmmoType, pWeapon->GetMaxAmmo()))
		{
			if(HasPlayerWeapon(pWeapon->GetClassName()))
				return false;
		}
	}
	else
	{
		if(HasPlayerWeapon(pWeapon->GetClassName()))
			return false;
	}
	
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::AddPlayerWeapon( CPlayerWeapon* pWeapon )
{
	if(!CanPickupWeapon(pWeapon->GetHUDSlot(), (weaponid_t)pWeapon->GetId()))
		return false;

	CPlayerWeapon* pPlayerWeapon = m_pWeaponsList;
	while(pPlayerWeapon)
	{
		if(!qstrcmp(pPlayerWeapon->GetClassName(), pWeapon->GetClassName()))
		{
			if(pWeapon->AddDuplicate(pPlayerWeapon))
			{
				pPlayerWeapon->UpdateClientData(this);
				if(m_pActiveWeapon)
					m_pActiveWeapon->UpdateClientData(this);

				if(pWeapon->ShouldRemove(pPlayerWeapon))
					pWeapon->FlagForRemoval();
			}

			return false;
		}

		pPlayerWeapon = pPlayerWeapon->GetNextWeapon();
	}

	// Add to list
	pWeapon->AddToPlayer(this);
	pWeapon->SetNextWeapon(m_pWeaponsList);
	m_pWeaponsList = pWeapon;

	// Emit sound
	if(!pWeapon->HasSpawnFlag(CPlayerWeapon::FL_WEAPON_NO_NOTICE))
		Util::EmitEntitySound(this, WEAPON_PICKUP_SOUND, SND_CHAN_ITEM);

	if(!m_pActiveWeapon || (pWeapon->GetWeaponFlags() & FL_WEAPON_AUTO_DRAW) && m_pActiveWeapon->CanHolster())
		SwitchToWeapon(pWeapon);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::ShouldSwitchToWeapon( CPlayerWeapon* pWeapon )
{
	if(!m_pActiveWeapon)
		return true;

	if(!m_pActiveWeapon->CanHolster())
		return false;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::SwitchToWeapon( CPlayerWeapon* pWeapon )
{
	if(!pWeapon->CanDeploy())
		return false;

	if(m_pActiveWeapon)
		m_pActiveWeapon->Holster();

	if(g_pCvarWeaponHolster->GetValue() >= 1)
	{
		// Mark next weapon to use
		m_pNextWeapon = pWeapon;
	}
	else
	{
		// Set pointers
		m_pPreviousWeapon = m_pActiveWeapon;
		m_pActiveWeapon = pWeapon;

		// Make weapon deploy immediately
		m_pActiveWeapon->Deploy();
		m_pActiveWeapon->UpdateClientData(this);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::CanHaveAmmo( const Char* pstrammotype, Int32 maxammo ) const
{
	if(!pstrammotype || !qstrlen(pstrammotype))
		return false;

	// Get ammo index for ammo typename
	Int32 ammoindex = CPlayerWeapon::GetAmmoTypeIndex(pstrammotype);
	if(ammoindex == NO_AMMO_INDEX)
		return false;

	// Check ammo counts
	if(GetAmmoCount(ammoindex) < maxammo)
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::ShouldHolster( void ) const
{
	if(m_pState->waterlevel >= WATERLEVEL_FULL)
		return true;

	if(m_pCameraEntity && !m_pCameraEntity->HasSpawnFlag(CTriggerCameraModel::FL_KEEP_VIEWMODEL))
		return true;

	if(m_forceHolster)
		return true;

	if(m_pLadderEntity)
		return true;

	if(m_pBikeEntity)
		return true;

	if(IsOnLadder())
		return true;

	if(m_healProgress != -2 && m_healProgress != -1)
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon* CPlayerEntity::GetNextWeapon( void )
{
	return m_pNextWeapon;
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon* CPlayerEntity::GetActiveWeapon( void )
{
	return m_pActiveWeapon;
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon* CPlayerEntity::GetClientActiveWeapon( void )
{
	return m_pClientActiveWeapon;
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon* CPlayerEntity::GetWeaponList( void )
{
	return m_pWeaponsList;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ApplyAxisPunch( Int32 axis, Float value )
{
	assert(axis >= 0 && axis <= 2);
	m_pState->punchamount[axis] += value;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ApplyDirectAxisPunch( Int32 axis, Float value )
{
	assert(axis >= 0 && axis <= 2);
	m_pState->punchangles[axis] += value;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetViewModel( const Char* pstrviewmodel )
{
	if(!pstrviewmodel || !qstrlen(pstrviewmodel))
		m_pFields->viewmodel = NO_STRING_VALUE;
	else
		m_pFields->viewmodel = gd_engfuncs.pfnAllocString(pstrviewmodel);

	if(m_pFields->viewmodel != m_clientViewModel)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.viewmodel, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(VMODEL_SET_MODEL);
			gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->viewmodel));
		gd_engfuncs.pfnUserMessageEnd();

		m_clientViewModel = m_pFields->viewmodel;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetClientViewModel( const Char* pstrviewmodel )
{
	if(!pstrviewmodel || !qstrlen(pstrviewmodel))
		m_clientViewModel = NO_STRING_VALUE;
	else
		m_clientViewModel = gd_engfuncs.pfnAllocString(pstrviewmodel);
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::ForceWeaponUpdate( void )  const
{ 
	return m_forceWeaponUpdate; 
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetForceWeaponUpdate( bool update )
{
	m_forceWeaponUpdate = update;
}

//=============================================
// @brief
//
//=============================================
Uint32 CPlayerEntity::GetClientConeIndex( void ) const
{
	return m_clientConeIndex;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetClientConeIndex( Uint32 coneIndex )
{
	m_clientConeIndex = coneIndex;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetFOV( Int32 value )
{
	m_fov = value;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GetFOV( void ) const
{
	return m_fov;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerEntity::GetClientFOV( void ) const
{
	return m_clientFOV;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetLeanOffset( Int32 buttons ) const
{
	if(!(m_pState->buttons & IN_LEAN))
		return ZERO_VECTOR;

	return m_curLeanOffset;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::UseMedkit( void )
{
	if(!m_numMedkits)
		return;

	if(m_pState->health >= m_pState->maxhealth)
		return;

	// Play sound
	Util::EmitEntitySound(this, "items/medkit_use.wav", SND_CHAN_VOICE);
	
	// Get heal amount
	Float healamount = gSkillData.GetSkillCVarSetting(g_skillcvars.skillHealthkitHealAmount);
	if(gSkillData.GetSkillLevel() == SKILL_HARD)
		healamount += Common::RandomLong(-5, 5);

	TakeHealth(healamount, DMG_GENERIC);
	m_numMedkits--;

	UpdateClientData();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlayDeathSound( void )
{
	if(m_pState->waterlevel < WATERLEVEL_FULL)
	{
		CString soundfile;
		switch(Common::RandomLong(0, 2))
		{
		case 0: soundfile = "player/pl_die1.wav"; break;
		case 1: soundfile = "player/pl_die2.wav"; break;
		case 2: soundfile = "player/pl_die3.wav"; break;
		}

		Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_VOICE);
	}

	Util::EmitEntitySound(this, "player/pl_die_heartbeat.wav", SND_CHAN_BODY);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlayPainSound( void )
{
	if(m_nextPainSoundTime > g_pGameVars->time)
		return;

	CString soundfile;
	if(m_pState->waterlevel < WATERLEVEL_FULL)
	{
		switch(Common::RandomLong(0, 2))
		{
		case 0: soundfile = "player/pl_pain1.wav"; break;
		case 1: soundfile = "player/pl_pain2.wav"; break;
		case 2: soundfile = "player/pl_pain3.wav"; break;
		}

		m_nextPainSoundTime = g_pGameVars->time + Common::RandomLong(1, 2);
	}
	else
	{
		switch(Common::RandomLong(0, 2))
		{
		case 0: soundfile = "player/pl_drown1.wav"; break;
		case 1: soundfile = "player/pl_drown2.wav"; break;
		case 2: soundfile = "player/pl_drown3.wav"; break;
		}

		m_nextPainSoundTime = g_pGameVars->time + Common::RandomLong(2, 4);
	}

	Util::EmitEntitySound(this, soundfile.c_str(), SND_CHAN_VOICE);
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::TakeHealth( Float amount, Int32 damageFlags )
{
	if(m_pState->takedamage == TAKEDAMAGE_NO)
		return false;

	// Clear out damages on these types
	m_damageTypes &= ~(damageFlags & DMG_TIMEBASED);
	return CBaseEntity::TakeHealth(amount, damageFlags);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::HealThink( void )
{
	if(m_pState->buttons & IN_HEAL && m_pState->velocity.IsZero() 
		&& m_pState->health < m_pState->maxhealth && m_numMedkits > 0 
		&& !m_pBikeEntity && !m_pLadderEntity)
	{
		if(m_healProgress == -1)
			m_healProgress = 0;

		if(m_healProgress != -2)
		{
			m_healProgress += (1.0f/PLAYER_HEAL_TIME)*g_pGameVars->frametime;
			if(m_healProgress >= 1.0)
			{
				UseMedkit();
				m_healProgress = -2;
			}
		}
	}
	else
	{
		// Reset
		m_healProgress = -1;
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::AddMedkit( const Char* pstrClassname, bool noNotice )
{
	if(!pstrClassname || !qstrlen(pstrClassname))
		return false;

	if(m_numMedkits >= gSkillData.GetSkillCVarSetting(g_skillcvars.skillMaxHealthkits))
		return false;

	if(!noNotice)
	{
		// Play sound
		Util::EmitEntitySound(this, "items/pickup_medkit.wav", SND_CHAN_VOICE);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteString(pstrClassname);
		gd_engfuncs.pfnUserMessageEnd();
	}

	m_numMedkits++;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::AddKevlar( const Char* pstrClassname, bool noNotice )
{
	if(!pstrClassname || !qstrlen(pstrClassname))
		return false;

	if(m_pState->armorvalue >= MAX_PLAYER_KEVLAR)
		return false;

	if(!noNotice)
	{
		// Play sound
		Util::EmitEntitySound(this, "items/pickup_kevlar.wav", SND_CHAN_VOICE);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteString(pstrClassname);
		gd_engfuncs.pfnUserMessageEnd();
	}

	Float kevlaradd = gSkillData.GetSkillCVarSetting(g_skillcvars.skillKevlarAmount);
	Float kevlarmissing = MAX_PLAYER_KEVLAR - m_pState->armorvalue;
	if(kevlaradd > kevlarmissing)
		kevlaradd = kevlarmissing;

	m_pState->armorvalue += kevlaradd;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::AddShoulderLight( const Char* pstrClassname, bool noNotice )
{
	if(m_hasShoulderLight)
		return false;

	if(!pstrClassname || !qstrlen(pstrClassname))
		return false;

	if(!noNotice)
	{
		// Play sound
		Util::EmitEntitySound(this, "items/pickup_weapon.wav", SND_CHAN_VOICE);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteString(pstrClassname);
		gd_engfuncs.pfnUserMessageEnd();
	}

	m_hasShoulderLight = true;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::RemoveAllWeapons( void )
{
	if(m_pActiveWeapon)
	{
		m_pActiveWeapon->Holster();
		ClearCurrentWeapon();
	}

	m_pPreviousWeapon = nullptr;

	if(m_pWeaponsList)
	{
		CPlayerWeapon* pWeapon = m_pWeaponsList;
		while(pWeapon)
		{
			CPlayerWeapon* pFree = pWeapon;
			pWeapon = pFree->GetNextWeapon();

			pFree->DropWeapon(true);
		}
		m_pWeaponsList = nullptr;
	}
	
	m_pState->weapons = 0;
	m_numMedkits = 0;

	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
		m_ammoCounts[i] = 0;

	// Force client data update
	UpdateClientData();

	// Update on client to be null
	ClearCurrentWeapon();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ClearCurrentWeapon( void )
{
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudcurrentweapon, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(1);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteInt16(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
	gd_engfuncs.pfnUserMessageEnd();

	m_pFields->viewmodel = NO_STRING_VALUE;
	m_pActiveWeapon = nullptr;
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon* CPlayerEntity::GetNextBestWeapon( CPlayerWeapon* pWeapon )
{
	Int32 bestWeight = 0;
	CPlayerWeapon* pBestWeapon = nullptr;

	// Go through player's list of weapons
	CPlayerWeapon* pPlayerWeapon = m_pWeaponsList;
	while(pPlayerWeapon)
	{
		if(pWeapon == pPlayerWeapon)
		{
			pPlayerWeapon = pPlayerWeapon->GetNextWeapon();
			continue;
		}

		// Get weapon's weight
		Int32 weight = pPlayerWeapon->GetWeight();

		if(pWeapon && weight != -1 && weight == pWeapon->GetWeight() && pPlayerWeapon->CanDeploy())
		{
			// Same category, so switch immediately
			pBestWeapon = pPlayerWeapon;
			break;
		}
		else if(weight > bestWeight && pPlayerWeapon->CanDeploy())
		{
			// This is a better one
			bestWeight = weight;
			pBestWeapon = pPlayerWeapon;
		}

		pPlayerWeapon = pPlayerWeapon->GetNextWeapon();
	}

	return pBestWeapon;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::AddFullAmmoDual( CPlayerWeapon* pWeapon ) const
{
	CPlayerWeapon* pPlayerWeapon = m_pWeaponsList;
	while(pPlayerWeapon)
	{
		if(!qstrcmp(pPlayerWeapon->GetClassName(), pWeapon->GetClassName()))
			return pPlayerWeapon->AddFullAmmoDual(pWeapon);

		pPlayerWeapon = pPlayerWeapon->GetNextWeapon();
	}
	
	return false;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::CanPickupWeapon( Int32 slot, weaponid_t weaponid )
{
	assert(slot >= 0 && slot < MAX_WEAPON_SLOTS);

	if(!m_pWeaponsList)
		return true;

	Uint32 nbslotweapons = 0;
	CPlayerWeapon* pWeapon = m_pWeaponsList;
	while(pWeapon)
	{
		if(pWeapon->GetId() == weaponid)
			return true;

		if(pWeapon->GetHUDSlot() == slot)
			nbslotweapons++;

		pWeapon = pWeapon->GetNextWeapon();
	}

	if(nbslotweapons >= WEAPON_SLOT_LIMITS[slot])
	{
		if(m_weaponFullPromptTime <= g_pGameVars->time)
		{
			switch(slot)
			{
			case 0:
				Util::ShowMessage("SLOT_FULL_MELEE", this);
				break;
			case 1:
				Util::ShowMessage("SLOT_FULL_SECONDARY", this);
				break;
			case 2:
				Util::ShowMessage("SLOT_FULL_PRIMARY", this);
				break;
			case 3:
				Util::ShowMessage("SLOT_FULL_HEAVY", this);
				break;
			case 4:
				Util::ShowMessage("SLOT_FULL_EXPLOSIVE", this);
				break;
			}

			m_weaponFullPromptTime = g_pGameVars->time + 1;
		}

		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetWeaponBit( Int64 weaponid )
{
	m_pState->weapons |= (1i64<<weaponid);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::RemoveWeaponBit( Int64 weaponid )
{
	m_pState->weapons &= ~(1i64<<weaponid);
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetWeaponDropPosition( void )
{
	Vector forward, right, up;
	Math::AngleVectors(m_pState->viewangles, &forward, &right, &up);

	Vector dropPosition = m_pState->origin + m_pState->view_offset - up*8 + forward * 8;
	return dropPosition;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::DropCurrentWeapon( void )
{
	if(!m_pActiveWeapon || !m_pActiveWeapon->CanHolster() || m_nextActionTime > g_pGameVars->time)
		return;

	if(m_pActiveWeapon->GetWeaponNextThinkTime() > g_pGameVars->time)
		return;

	Vector forward, right, up;
	Math::AngleVectors(m_pState->viewangles, &forward, &right, &up);

	Vector traceBegin = m_pState->origin + m_pState->view_offset;
	Vector traceEnd = GetWeaponDropPosition();

	trace_t tr;
	Util::TraceLine(traceBegin, traceEnd, false, false, m_pEdict, tr);
	if(!tr.noHit() || tr.allSolid() || tr.startSolid())
	{
		Util::ShowMessage("DROP_NO_SPACE", this);
		return;
	}

	m_pActiveWeapon->Holster();
	m_pActiveWeapon->FlagDrop();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::RemoveWeapon( CPlayerWeapon* pWeapon )
{
	// Find next best weapon if we're dropping our current one
	if(pWeapon == m_pActiveWeapon)
	{
		CPlayerWeapon* pBestWeapon = GetNextBestWeapon(m_pActiveWeapon);

		if(g_pCvarWeaponHolster->GetValue() >= 1)
		{
			// Mark next weapon to use
			m_pNextWeapon = pBestWeapon;
		}
		else
		{
			// Set pointers
			if(m_pPreviousWeapon == pWeapon)
				m_pPreviousWeapon = nullptr;

			m_pActiveWeapon = pBestWeapon;

			// Make weapon deploy immediately
			m_pActiveWeapon->Deploy();
			m_pActiveWeapon->UpdateClientData(this);
		}

		ClearCurrentWeapon();
	}

	// Remove bit
	m_pState->weapons &= ~(1i64<<pWeapon->GetId());

	if(pWeapon == m_pWeaponsList)
	{
		// Reset header
		m_pWeaponsList = pWeapon->GetNextWeapon();
	}
	else
	{
		CPlayerWeapon* pPlayerWeapon = m_pWeaponsList->GetNextWeapon();
		CPlayerWeapon* pPrevWeapon = m_pWeaponsList;
		while(pPlayerWeapon)
		{
			if(pPlayerWeapon == pWeapon)
			{
				pPrevWeapon->SetNextWeapon(pWeapon->GetNextWeapon());
				break;
			}

			pPrevWeapon = pPlayerWeapon;
			pPlayerWeapon = pPlayerWeapon->GetNextWeapon();
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::HasPlayerWeapon( const Char* pstrWeaponClassName ) const
{
	if(!m_pWeaponsList)
		return false;

	CPlayerWeapon* pNext = m_pWeaponsList;
	while(pNext)
	{
		if(!qstrcmp(pNext->GetClassName(), pstrWeaponClassName))
			return true;

		pNext = pNext->GetNextWeapon();
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetWeaponVolume( Uint32 volume )
{
	m_weaponSoundRadius = volume;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetWeaponFlashBrightness( Uint32 brightness )
{
	m_weaponFlashBrightness = brightness;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::EnterLadder( CEnvLadder *pLadder )
{
	m_ladderState = LADDER_STATE_ENTERING;
	m_ladderUpdateTime = g_pGameVars->time + LADDER_ENTER_BOTTOM_TIME;

	m_pLadderEntity = pLadder;
	m_pLadderEntity->EnterLadder(this);
	m_pLadderEntity->RepositionPlayer(this, &m_ladderDestOrigin, &m_ladderDestAngles);

	m_pState->flags |= FL_FROZEN;

	int iAnimation = m_pLadderEntity->GetEntryAnimation();
	switch(iAnimation)
	{
		case LADDER_ENTER_BOTTOM: 
			m_ladderUpdateTime = g_pGameVars->time + LADDER_ENTER_BOTTOM_TIME; 
			break;
		case LADDER_ENTER_RIGHT: 
			m_ladderUpdateTime = g_pGameVars->time + LADDER_ENTER_RIGHT_TIME; 
			break;
		case LADDER_ENTER_LEFT: 
			m_ladderUpdateTime = g_pGameVars->time + LADDER_ENTER_LEFT_TIME; 
			break;
		case LADDER_ENTER_TOP: 
			m_ladderUpdateTime = g_pGameVars->time + LADDER_ENTER_TOP_TIME; 
			break;
	}

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteInt16(pLadder->GetEntityIndex());
		gd_engfuncs.pfnMsgWriteInt16(LADDER_STATE_ENTERING);
		gd_engfuncs.pfnMsgWriteByte(iAnimation);
		gd_engfuncs.pfnMsgWriteFloat(m_ladderDestOrigin[0]);
		gd_engfuncs.pfnMsgWriteFloat(m_ladderDestOrigin[1]);
		gd_engfuncs.pfnMsgWriteFloat(m_ladderDestOrigin[2]);
		gd_engfuncs.pfnMsgWriteFloat(m_ladderDestAngles[0]);
		gd_engfuncs.pfnMsgWriteFloat(m_ladderDestAngles[1]);
		gd_engfuncs.pfnMsgWriteFloat(m_ladderDestAngles[2]);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::InitLeaveLadder( ladder_verify_codes_t exitcode )
{
	ladder_exitpoints_t iExit = LADDER_EXIT_UNAVAILABLE;
	Float flLeaveTime = 0;
	Float flDiff = 0;
	Vector vOrigin, vAngles;

	if(exitcode == LADDER_VR_MOVE_EXIT_USE)
	{
		if(m_nextLadderMoveTime && m_nextLadderMoveTime >= g_pGameVars->time)
			return; // No exiting until we're done moving

		iExit = m_pLadderEntity->CanUseExit(&vAngles, &vOrigin, &flLeaveTime, &flDiff);
		if(iExit == LADDER_EXIT_UNAVAILABLE)
			return;
	}
	else
	{
		if(exitcode == LADDER_VR_MOVE_EXIT_TOP)
		{
			if(m_pLadderEntity->HasSpawnFlag(CEnvLadder::FL_TOP_ACCESS) && m_pLadderEntity->GetExitVectors(LADDER_EXIT_TOP, &vOrigin, &vAngles, &flDiff))
			{
				flLeaveTime = LADDER_LEAVE_TOP_TIME;
				iExit = LADDER_EXIT_TOP;
			}
			else if(m_pLadderEntity->GetExitVectors(LADDER_EXIT_LEFT, &vOrigin, &vAngles, &flDiff))
			{
				flLeaveTime = LADDER_LEAVE_LEFT_TIME;
				iExit = LADDER_EXIT_LEFT;
			}
			else if(m_pLadderEntity->GetExitVectors(LADDER_EXIT_RIGHT, &vOrigin, &vAngles, &flDiff))
			{
				flLeaveTime = LADDER_LEAVE_RIGHT_TIME;
				iExit = LADDER_EXIT_RIGHT;
			}
			else
			{
				// No exit found
				return;
			}
		}
		else if(exitcode == LADDER_VR_MOVE_EXIT_BOTTOM)
		{
			if(m_pLadderEntity->GetExitVectors(LADDER_EXIT_BOTTOM, &vOrigin, &vAngles, &flDiff))
			{
				flLeaveTime = LADDER_LEAVE_BOTTOM_TIME;
				iExit = LADDER_EXIT_BOTTOM;
			}
			else if(m_pLadderEntity->GetExitVectors(LADDER_EXIT_LEFT, &vOrigin, &vAngles, &flDiff))
			{
				flLeaveTime = LADDER_LEAVE_LEFT_TIME;
				iExit = LADDER_EXIT_LEFT;
			}
			else if(m_pLadderEntity->GetExitVectors(LADDER_EXIT_RIGHT, &vOrigin, &vAngles, &flDiff))
			{
				flLeaveTime = LADDER_LEAVE_RIGHT_TIME;
				iExit = LADDER_EXIT_RIGHT;
			}
			else
			{
				// No exit found
				return;
			}
		}
	}

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteInt16(m_pLadderEntity->GetEntityIndex());
		gd_engfuncs.pfnMsgWriteInt16(LADDER_STATE_LEAVING);
		gd_engfuncs.pfnMsgWriteByte(iExit);
		gd_engfuncs.pfnMsgWriteFloat(flDiff);
		gd_engfuncs.pfnMsgWriteFloat(vOrigin[0]);
		gd_engfuncs.pfnMsgWriteFloat(vOrigin[1]);
		gd_engfuncs.pfnMsgWriteFloat(vOrigin[2]);
		gd_engfuncs.pfnMsgWriteFloat(vAngles[0]);
		gd_engfuncs.pfnMsgWriteFloat(vAngles[1]);
		gd_engfuncs.pfnMsgWriteFloat(vAngles[2]);
	gd_engfuncs.pfnUserMessageEnd();

	m_pState->flags |= FL_FROZEN;

	m_ladderState = LADDER_STATE_LEAVING;
	m_ladderUpdateTime = g_pGameVars->time + flLeaveTime;

	m_ladderDestOrigin = vOrigin;
	m_ladderDestAngles = vAngles;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::ClearLadder( void )
{
	m_pState->flags &= ~FL_ON_LADDER;
	m_pLadderEntity->ExitLadder();
	m_pLadderEntity = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::LadderThink( void )
{
	if(m_ladderUpdateTime)
	{
		if(m_ladderUpdateTime <= g_pGameVars->time)
		{
			m_ladderUpdateTime = 0;

			if(m_ladderState == LADDER_STATE_ENTERING)
			{
				// Position player approximately
				gd_engfuncs.pfnSetOrigin(m_pEdict, m_ladderDestOrigin);
				m_pState->viewangles = m_pState->angles = m_ladderDestAngles;
				m_pState->fixangles = true;

				m_pState->flags &= ~FL_FROZEN;
				m_pState->flags |= FL_ON_LADDER;
				m_ladderState = LADDER_STATE_ACTIVE;
				m_ladderMoveDirection = LADDER_RESTING;

				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteInt16(m_pLadderEntity->GetEntityIndex());
					gd_engfuncs.pfnMsgWriteInt16(m_ladderState);
					gd_engfuncs.pfnMsgWriteByte(m_ladderMoveDirection);
				gd_engfuncs.pfnUserMessageEnd();
			}
			else if(m_ladderState == LADDER_STATE_LEAVING)
			{
				gd_engfuncs.pfnSetOrigin(m_pEdict, m_ladderDestOrigin);
				m_pState->viewangles = m_pState->angles = m_ladderDestAngles;
				m_pState->fixangles = true;

				ClearLadder();

				m_ladderState = LADDER_STATE_CLEANUP;
				m_ladderUpdateTime = g_pGameVars->time + 0.1;
			}
			else if(m_ladderState == LADDER_STATE_CLEANUP)
			{
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteInt16(0);
					gd_engfuncs.pfnMsgWriteInt16(LADDER_STATE_CLEANUP);
				gd_engfuncs.pfnUserMessageEnd();

				m_ladderState = LADDER_STATE_INACTIVE;
				m_pState->flags &= ~FL_FROZEN;
			}
		}
	}

	if(m_ladderState == LADDER_STATE_ACTIVE)
	{
		if(m_nextLadderMoveTime)
		{
			if(m_nextLadderMoveTime < g_pGameVars->time)
			{
				// Reposition the player
				Vector vForward, vRight;
				Math::AngleVectors(m_pLadderEntity->GetAngles(), &vForward, &vRight, nullptr);
				Vector vOffset = vForward * LADDER_FORWARD_OFFSET;

				Vector vOrigin = m_pLadderEntity->GetOrigin() + vOffset;
				vOrigin[2] = m_pState->origin.z;

				Vector vResult;
				if(m_ladderMoveDirection == LADDER_MOVE_UP)
					vResult = vOrigin + Vector(0, 0, LADDER_STEP_SIZE);
				else
					vResult = vOrigin - Vector(0, 0, LADDER_STEP_SIZE);

				m_pState->origin = vResult;
				gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

				m_ladderMoveDirection = LADDER_RESTING;
				m_clientLadderMoveDirection = LADDER_RESET;
				m_nextLadderMoveTime = 0;
			}
		}
		else
		{
			if(m_pState->buttons & (IN_FORWARD|IN_BACK))
			{
				Int32 iDirection = 0;
				if(m_pState->buttons & IN_FORWARD) iDirection += 1;
				if(m_pState->buttons & IN_BACK) iDirection -= 1;

				ladder_verify_codes_t result = m_pLadderEntity->VerifyMove(m_pState->origin, this, iDirection);
				if(result != LADDER_VR_NOMOVE)
				{
					if(result == LADDER_VR_MOVE_VALID)
					{
						// Move is valid, go forth
						m_ladderMoveDirection = (iDirection > 0) ? LADDER_MOVE_UP : LADDER_MOVE_DOWN;
						m_nextLadderMoveTime = g_pGameVars->time + LADDER_STEP_TIME;
					}
					else
					{
						InitLeaveLadder(result);
						return;
					}
				}
			}

			if(m_ladderMoveDirection != m_clientLadderMoveDirection)
			{
				m_clientLadderMoveDirection = m_ladderMoveDirection;
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.ladder, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteInt16(m_pLadderEntity->GetEntityIndex());
					gd_engfuncs.pfnMsgWriteInt16(m_ladderState);
					gd_engfuncs.pfnMsgWriteByte(m_ladderMoveDirection);
				gd_engfuncs.pfnUserMessageEnd();
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetNPCAwareness( Float awareness, CBaseEntity* pNPC, Float timeoutDelay )
{
	if(!m_npcAwarenessList.empty())
	{
		m_npcAwarenessList.begin();
		while(!m_npcAwarenessList.end())
		{
			npc_awarenessinfo_t& info = m_npcAwarenessList.get();
			if(info.pnpc && info.pnpc.get() == pNPC->GetEdict())
			{
				if(awareness > 0)
				{
					info.awareness = awareness;
					info.lasttime = g_pGameVars->time;
					return;
				}
				else
				{
					m_npcAwarenessList.remove(m_npcAwarenessList.get_link());
					return;
				}
			}

			m_npcAwarenessList.next();
		}
	}

	if(awareness <= 0)
		return;

	npc_awarenessinfo_t info;
	info.awareness = awareness;
	info.pnpc = pNPC;
	info.timeoutdelay = timeoutDelay;
	info.lasttime = g_pGameVars->time;

	m_npcAwarenessList.add(info);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::NPCAwarenessThink( void )
{
	// Reset this
	m_highestAwarenessLevel = 0;
	Double lasthighesttime = 0;

	if(m_npcAwarenessList.empty())
		return;

	m_npcAwarenessList.begin();
	while(!m_npcAwarenessList.end())
	{
		const npc_awarenessinfo_t& info = m_npcAwarenessList.get();
		if(!info.pnpc || !info.pnpc->IsAlive() || info.lasttime + info.timeoutdelay <= g_pGameVars->time)
		{
			m_npcAwarenessList.remove(m_npcAwarenessList.get_link());
			m_npcAwarenessList.next();
			continue;
		}
		else if(!m_highestAwarenessLevel || (info.pnpc->GetEnemy() == this && info.pnpc->CheckConditions(AI_COND_SEE_CLIENT)))
		{
			if(info.lasttime > lasthighesttime || info.awareness > m_highestAwarenessLevel)
			{
				m_highestAwarenessLevel = info.awareness;
				lasthighesttime = info.lasttime;
			}
		}

		m_npcAwarenessList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::LeanThink( void )
{
 	Int32 flagsChanged = (m_prevFlags ^ m_pState->flags);
	m_prevFlags = m_pState->flags;

	Int32 buttonsChanged = ( m_prevButtons ^ m_pState->buttons );
	m_prevButtons = m_pState->buttons;

	if(m_bikeState != BIKE_SV_INACTIVE)
		return;

	if (m_pState->buttons & IN_LEAN)
	{
		if(!m_leanState || (buttonsChanged & IN_MOVELEFT) || (buttonsChanged & IN_MOVERIGHT) 
			|| (buttonsChanged & IN_FORWARD) || (buttonsChanged & IN_BACK) || (flagsChanged & FL_DUCKING))
		{
			m_prevLeanAngles = m_curLeanAngles;
			m_prevLeanOffset = m_curLeanOffset;
			m_leanTime = g_pGameVars->time;
			m_leanState = true;
		}

		// Determine directions
		Float leanUp = 0;
		Float leanSide = 0;

		if( m_pState->buttons & IN_MOVELEFT )
			leanSide -= 1.0;

		if( m_pState->buttons & IN_MOVERIGHT )
			leanSide += 1.0;

		if(m_pState->buttons & IN_FORWARD)
		{
			if(!(m_pState->flags & FL_DUCKING))
				leanUp = 0.3;
			else
				leanUp = 1.0;
		}
		else if(m_pState->buttons & IN_BACK)
		{
			leanUp = -0.2;
		}
		
		// Calculate complete offset
		Vector v_forward, v_right, v_up;
		Math::AngleVectors(m_pState->viewangles, &v_forward, &v_right, &v_up);
		v_right = v_right + v_up*(-0.2*leanSide);
		v_right = v_right + v_forward*(0.3*leanSide);

		m_idealLeanOffset = v_right*leanSide*LEAN_DISTANCE_SIDE;
		m_idealLeanOffset = m_idealLeanOffset + v_up*leanUp*LEAN_DISTANCE_UP;

		Vector traceOrg = (m_pState->origin + m_pState->view_offset);
		Vector traceDest1 = (m_pState->origin + m_pState->view_offset)+m_idealLeanOffset;
		Vector traceDest2 = traceDest1 + m_idealLeanOffset + v_forward*4;

		trace_t tr, tr_cp;
		Util::TraceLine(traceOrg, traceDest1, false, true, m_pEdict, tr);
		Util::TraceLine(traceOrg, traceDest2, false, true, m_pEdict, tr_cp);

		Float flTraceFraction = 1.0;
		if(tr.fraction != 1.0)
			flTraceFraction = tr.fraction*0.75;
		else if(tr_cp.fraction != 1.0)
			flTraceFraction = tr_cp.fraction*0.75;

		m_idealLeanAngles[1] = leanSide*4;
		m_idealLeanAngles[2] = leanSide*16;
		m_idealLeanAngles[0] = leanUp*6;

		// Calculate fraction
		Double time = clamp((g_pGameVars->time - m_leanTime), 0.0, LEAN_TIME);
		Float leanFraction = Common::SplineFraction( time, (1.0/LEAN_TIME) );

		// It must be recalculated every frame
		if(leanFraction > 1.0)
			leanFraction = 1.0;

		// Interpolate offset
		m_curLeanOffset = m_idealLeanOffset*leanFraction*flTraceFraction;
		m_curLeanOffset = m_curLeanOffset+m_prevLeanOffset*(1.0-leanFraction);

		// Interpolate tilt
		m_curLeanAngles = m_idealLeanAngles*leanFraction;
		m_curLeanAngles = m_curLeanAngles+m_prevLeanAngles*(1.0-leanFraction);
	}
	else
	{
		// Move back if we have to
		if(m_leanState)
		{
			m_prevLeanAngles = m_curLeanAngles;
			m_idealLeanAngles.Clear();
			m_prevLeanOffset = m_curLeanOffset;
			m_idealLeanOffset.Clear();
			m_leanTime = g_pGameVars->time;
			m_leanState = false;
		}

		if(m_leanTime)
		{
			// Calculate lean interpolation
			Float time = clamp((g_pGameVars->time - m_leanTime), 0.0, LEAN_TIME);
			Float leanFraction = Common::SplineFraction( time, (1.0/LEAN_TIME) );

			if(leanFraction >= 1.0)
			{
				m_curLeanOffset.Clear();
				m_curLeanAngles.Clear();
				m_leanTime = 0;
				return;
			}

			Vector v_forward;
			Math::AngleVectors(m_pState->viewangles, &v_forward, nullptr, nullptr);

			Vector traceOrg = (m_pState->origin + m_pState->view_offset);
			Vector traceDest1 = (m_pState->origin + m_pState->view_offset)+m_idealLeanOffset;
			Vector traceDest2 = traceDest1 + m_idealLeanOffset + v_forward*4;

			trace_t tr, tr_cp;
			Util::TraceLine(traceOrg, traceDest1, false, true, m_pEdict, tr);
			Util::TraceLine(traceOrg, traceDest2, false, true, m_pEdict, tr);

			Double flTraceFraction = 1.0;
			if(tr.fraction != 1.0)
				flTraceFraction = tr.fraction*0.75;
			else if(tr_cp.fraction != 1.0)
				flTraceFraction = tr_cp.fraction*0.75;

			// Interpolate offset
			m_curLeanOffset = m_idealLeanOffset*leanFraction;
			m_curLeanOffset = m_curLeanOffset+m_prevLeanOffset*(1.0-leanFraction)*flTraceFraction;

			// Interpolate tilt
			m_curLeanAngles = m_idealLeanAngles*leanFraction;
			m_curLeanAngles = m_curLeanAngles+m_prevLeanAngles*(1.0-leanFraction);
		}
	}
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetLeanAngle( void ) const
{
	if(!(m_pState->buttons & IN_LEAN) && !m_leanTime)
		return ZERO_VECTOR;

	return m_curLeanAngles;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetLeanOffset( void ) const
{
	if(!(m_pState->buttons & IN_LEAN) && !m_leanTime)
		return ZERO_VECTOR;

	return m_curLeanOffset;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetGunPosition( void ) const
{
	Vector origin = m_pState->origin + m_pState->view_offset;
	origin = origin + GetLeanOffset();

	return origin;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::AutoAimThink( void )
{
	if(m_pBikeEntity || m_pCameraEntity || m_pLadderEntity 
		|| !IsAlive() || !ShouldUseAutoAim() || !m_pActiveWeapon 
		|| (m_pActiveWeapon->GetWeaponFlags() & FL_WEAPON_NO_AUTO_AIM))
	{
		if(!m_autoAimVector.IsZero())
		{
			// Clear these
			m_autoAimVector.Clear();
			m_lastAutoAimVector.Clear();
			m_isOnTarget = false;

			// Clear on client
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setautoaimvector, nullptr, m_pEdict);
				gd_engfuncs.pfnMsgWriteSmallFloat(0);
				gd_engfuncs.pfnMsgWriteSmallFloat(0);
				gd_engfuncs.pfnMsgWriteByte(FALSE);
			gd_engfuncs.pfnUserMessageEnd();
		}

		return;
	}

	// Forward vector of view
	Vector forward, right, up;
	Math::AngleVectors(m_pState->viewangles, &forward, &right, &up);

	// Gun position
	Vector srcVector = GetGunPosition();
	// Player's water level
	Int32 playerWaterlevel = m_pState->waterlevel;
	// Reference distance
	const Float REFERENCE_DISTANCE = 16384;
	
	// Last best autoaim target values
	Float lastBestDotProduct = m_pActiveWeapon->GetAutoAimDegrees();
	CBaseEntity* pLastBestEntity = nullptr;
	Vector lastBestDirection;

	// Go through all entities
	for(Uint32 i = 0; i < g_pGameVars->numentities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity || !pEntity->IsAutoAimable(this))
			continue;

		Int32 entityWaterlevel = pEntity->GetWaterLevel();
		if((entityWaterlevel == WATERLEVEL_FULL && playerWaterlevel != WATERLEVEL_FULL)
			|| (entityWaterlevel != WATERLEVEL_FULL && playerWaterlevel == WATERLEVEL_FULL))
			continue;

		Vector entityCenter = pEntity->GetBodyTarget(srcVector);
		Vector direction = (entityCenter - srcVector).Normalize();

		if(Math::DotProduct(direction, forward) < 0)
			continue;

		Float dp = SDL_fabs(Math::DotProduct(direction, right));
		dp += SDL_fabs(Math::DotProduct(direction, up));
		dp *= 1.0 + 0.2 * ((entityCenter - srcVector).Length() / REFERENCE_DISTANCE);
		if(dp > lastBestDotProduct)
			continue;

		trace_t tr;
		Util::TraceLine(srcVector, entityCenter, false, true, m_pEdict, tr);
		if(!tr.noHit() && tr.hitentity != pEntity->GetEntityIndex())
			continue;

		if(GetRelationship(pEntity) <= R_NONE)
			continue;

		lastBestDotProduct = dp;
		pLastBestEntity = pEntity;
		lastBestDirection = direction;
	}

	if(pLastBestEntity)
	{
		Vector bestAngles = Math::VectorToAngles(lastBestDirection);
		bestAngles.x = -bestAngles.x;
		bestAngles = bestAngles - m_pState->viewangles;
		m_autoAimVector = bestAngles;
		m_isOnTarget = true;
	}
	else if(!m_autoAimVector.IsZero())
	{
		m_autoAimVector.Clear();
		m_isOnTarget = false;
	}

	// After setting, limit angles
	for(Uint32 i = 0; i < 2; i++)
	{
		// Normalize angles
		if(m_autoAimVector[i] > 180.0f)
			m_autoAimVector[i] -= 360.0f;
		else if(m_autoAimVector[i] < -180.0f)
			m_autoAimVector[i] += 360.0f;

		// Limit to 25 degrees max
		if(m_autoAimVector[i] > 25)
			m_autoAimVector[i] = 25;
		else if(m_autoAimVector[i] < -25)
			m_autoAimVector[i] = 25;
	}

	// Update autoaim vector if needed
	if(!Math::VectorCompare(m_autoAimVector, m_lastAutoAimVector))
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setautoaimvector, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteFloat(m_autoAimVector[0]);
			gd_engfuncs.pfnMsgWriteFloat(m_autoAimVector[1]);
			gd_engfuncs.pfnMsgWriteByte(m_isOnTarget ? TRUE : FALSE);
		gd_engfuncs.pfnUserMessageEnd();

		m_lastAutoAimVector = m_autoAimVector;
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::ShouldUseAutoAim( void ) const
{
	return (g_pCvarAutoAim->GetValue() >= 1) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetGunAngles( bool addPunch ) const
{
	Vector result = GetViewAngles() + GetLeanAngle();
	if(ShouldUseAutoAim() && !m_autoAimVector.IsZero())
		result += m_autoAimVector;
	else if(addPunch)
		result += GetPunchAngle();

	return result;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetEyePosition( bool addlean ) const
{
	Vector origin = m_pState->origin + m_pState->view_offset;

	if(addlean)
		return origin + GetLeanOffset(m_pState->buttons);
	else 
		return origin;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::CanBlackHolePull( void ) const
{
	if(m_pBikeEntity)
		return false;

	if(m_pCameraEntity)
		return false;

	if(m_pState->flags & FL_FROZEN)
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetNavigablePosition( void ) const
{
	if(m_pState->flags & FL_DUCKING)
		return Vector( m_pState->origin.x, m_pState->origin.y, m_pState->origin.z + VEC_DUCK_HULL_MIN[2] );
	else
		return Vector( m_pState->origin.x, m_pState->origin.y, m_pState->origin.z + VEC_HULL_MIN[2] );
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetVISEyePosition( void ) const
{
	Vector viewOrigin;

	if(m_pCameraEntity)
	{
		// Origin comes from attachment 0 of trigger_cameramodel
		CTriggerCameraModel* pCamera = reinterpret_cast<CTriggerCameraModel*>(m_pCameraEntity);
		pCamera->GetAttachment(0, viewOrigin);
	}
	else if(m_pTriggerCameraEntity)
	{
		// Origin comes from trigger_camera's origin
		viewOrigin = m_pTriggerCameraEntity->GetOrigin();
	}
	else
	{
		// Normal player view origin
		viewOrigin = m_pState->origin + m_pState->view_offset;
	}

	return viewOrigin;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::GetAdditionalVISPosition( Vector& outVector ) const
{
	if(m_ladderState != LADDER_STATE_LEAVING
		&& m_ladderState != LADDER_STATE_ENTERING)
		return false;

	outVector = m_ladderDestOrigin + VEC_VIEW;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::DumpCheatCodes( void )
{
	if(!AreCheatsEnabled())
		return;

	Uint32 num = sizeof(CPlayerEntity::PLAYER_CHEAT_DESCRIPTIONS)/sizeof(CPlayerEntity::cheatinfo_t);
	for(Uint32 i = 0; i < num; i++)
	{
		const CPlayerEntity::cheatinfo_t& cheat = CPlayerEntity::PLAYER_CHEAT_DESCRIPTIONS[i];
		gd_engfuncs.pfnCon_Printf("Cheat: %s - %s.\n", cheat.cheatcode.c_str(), cheat.description.c_str());
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::BikeTouch( CBaseEntity *pOther )
{
	if(!m_bikeAcceleration)
		return;

	if(!pOther->IsNPC())
		return;

	// Gib small enemies
	if(pOther->GetSize().z <= 24)
	{
		pOther->Killed(this, GIB_ALWAYS);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::EnterBike( CItemMotorBike *pEntity )
{
	m_pBikeEntity = pEntity;
	m_pState->flags |= FL_ON_BIKE;
	SetAiment(m_pBikeEntity);

	SetControlEnable(false);
	ToggleBikeBlockers(true);
	m_clientBikeState = m_bikeState = BIKE_SV_ENTERING_LERP; // entering

	if(IsFlashlightOn())
		TurnFlashlightOff();

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(m_bikeState);
		gd_engfuncs.pfnMsgWriteInt16(m_pBikeEntity->GetEntityIndex());
	gd_engfuncs.pfnUserMessageEnd();

	m_bikeUpdateTime = g_pGameVars->time + BIKE_ENTER_TIME;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::LeaveBike( void )
{
	if(!m_pState->velocity.IsZero())
		return;

	if(!FindBikeDropSpot(m_dropAngles, m_dropOrigin))
		return;

	if(IsFlashlightOn())
		TurnFlashlightOff();

	SetControlEnable(false);
	ToggleBikeBlockers(false);
	m_clientBikeState = m_bikeState = BIKE_SV_LEAVING; // entering
	m_pState->flags &= ~FL_ON_BIKE;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(m_bikeState);
	gd_engfuncs.pfnUserMessageEnd();

	Double fleavetime = m_pBikeEntity->GetLeaveTime();
	m_bikeUpdateTime = g_pGameVars->time + fleavetime;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::FindBikeDropSpot( Vector& angles, Vector& origin )
{
	trace_t tr;
	Vector forward, right;
	Math::AngleVectors(m_pBikeEntity->GetAngles(), &forward, &right, nullptr);
	angles = m_pState->angles;

	// Re-enable solidity on bike for test
	m_pBikeEntity->SetSolidity(SOLID_BBOX);
	m_pBikeEntity->SetMoveType(MOVETYPE_TOSS);
	m_pBikeEntity->SetOwner(nullptr);
	m_pBikeEntity->SetOrigin(m_pBikeEntity->GetOrigin());
	
	// try left first
	bool result = false;
	for(Float dist = 16; dist < 64; dist += 16)
	{
		Vector vecSpot = m_pState->origin + right*dist*2.5;
		Util::TraceHull(vecSpot+Vector(0, 0, 4), vecSpot-Vector(0, 0, 16), false, false, HULL_HUMAN, m_pEdict, tr);

		if(!tr.allSolid() && !tr.startSolid() && !tr.noHit() && tr.fraction > 0)
		{
			angles.y += 45;
			angles.y = Math::AngleMod(angles.y);

			origin = tr.endpos;
			result = true;
			break;
		}
	}

	if(!result)
	{
		for(Float dist = 16; dist < 64; dist += 16)
		{
			// try right
			Vector vecSpot = m_pState->origin - right*dist*2.5;
			Util::TraceHull(vecSpot+Vector(0, 0, 4), vecSpot-Vector(0, 0, 16), false, false, HULL_HUMAN, m_pEdict, tr);
			if(!tr.allSolid() && !tr.startSolid() && !tr.noHit() && tr.fraction > 0)
			{
				angles.y -= 45;
				if(angles.y < 0)
					angles.y += 360;

				origin = tr.endpos;
				result = true;
				break;
			}
		}
	}

	if(!result)
	{
		for(Float dist = 16; dist < 64; dist += 16)
		{
			// try back
			Vector vecSpot = m_pState->origin - forward*dist*2.5;
			Util::TraceHull(vecSpot+Vector(0, 0, 4), vecSpot-Vector(0, 0, 16), false, false, HULL_HUMAN, m_pEdict, tr);
			if(!tr.allSolid() && !tr.startSolid() && !tr.noHit() && tr.fraction > 0)
			{
				origin = tr.endpos;
				result = true;
				break;
			}
		}
	}

	m_pBikeEntity->SetSolidity(SOLID_NOT);
	m_pBikeEntity->SetMoveType(MOVETYPE_FOLLOW);
	m_pBikeEntity->SetOwner(this);
	m_pBikeEntity->SetOrigin(m_pBikeEntity->GetOrigin());
	return result;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::BikeThink( void )
{
	// Handle updates
	if(m_bikeUpdateTime)
	{
		if(m_bikeUpdateTime <= g_pGameVars->time)
		{
			// Clear this here
			m_bikeUpdateTime = 0;
			if(m_bikeState == BIKE_SV_ENTERING_LERP)
			{
				// Make it follow the player
				m_pBikeEntity->SetFollow();
				m_bikeState = BIKE_SV_ENTERING;

				// Move player to the bike entity
				gd_engfuncs.pfnSetOrigin(m_pEdict, m_pBikeEntity->GetOrigin()+Vector(0, 0, VEC_HULL_MAX[2]));
				SetAngles(m_pBikeEntity->GetAngles());
				SetViewAngles(m_pBikeEntity->GetAngles());

				m_bikeUpdateTime = g_pGameVars->time + BIKE_ENTER_TIME;
			}
			else if(m_bikeState == BIKE_SV_ENTERING)
			{
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteByte(m_bikeState);
				gd_engfuncs.pfnUserMessageEnd();

				m_clientBikeState = m_bikeState = BIKE_SV_ACTIVE;
				Float flentertime = m_pBikeEntity->GetEnterTime();
				m_bikeUpdateTime = g_pGameVars->time + flentertime;
			}
			else if(m_bikeState == BIKE_SV_ACTIVE)
			{
				SetControlEnable(true);
				m_pState->fuser1 = TRUE;

				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteByte(m_bikeState);
				gd_engfuncs.pfnUserMessageEnd();
			}
			else if(m_bikeState == BIKE_SV_LEAVING)
			{
				m_clientBikeState = m_bikeState = BIKE_SV_LEAVING_LERP;
				m_bikeUpdateTime = g_pGameVars->time + BIKE_LEAVE_TIME;

				Vector clDropOrg = m_dropOrigin + VEC_VIEW;
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteByte(m_bikeState);
					for(Uint32 i = 0; i < 3; i++)
						gd_engfuncs.pfnMsgWriteFloat(clDropOrg[i]);
					for(Uint32 i = 0; i < 3; i++)
						gd_engfuncs.pfnMsgWriteFloat(m_dropAngles[i]);
				gd_engfuncs.pfnUserMessageEnd();
			}
			else if(m_bikeState == BIKE_SV_LEAVING_LERP)
			{
				// leave time for client to clean up
				m_clientBikeState = m_bikeState = BIKE_SV_CLEANUP;
				m_bikeUpdateTime = g_pGameVars->time + 0.2;
				m_pBikeEntity->PlayerLeave();

				gd_engfuncs.pfnSetOrigin(m_pEdict, m_dropOrigin);
				SetAngles(m_dropAngles);
				SetViewAngles(m_dropAngles);

				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.motorbike, nullptr, m_pEdict);
					gd_engfuncs.pfnMsgWriteByte(BIKE_SV_INACTIVE);
				gd_engfuncs.pfnUserMessageEnd();
			}
			else if(m_bikeState == BIKE_SV_CLEANUP)
			{
				SetControlEnable(true);
				m_pState->fuser1 = FALSE;
				SetAiment(nullptr);
				
				m_pBikeEntity = nullptr;
				m_bikeState = BIKE_SV_INACTIVE;
			}
		}
	}
	
	// Manage collisions
	if(m_bikeState == BIKE_SV_ACTIVE)
	{
		Float flLastVelocity = m_bikeVelocity.Length();
		Float flVelocity = m_pState->velocity.Length();
		Float flDiff = abs(flLastVelocity - flVelocity);

		if( flDiff > MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED )
		{
			trace_t tr;
			Vector traceEnd = m_pState->origin + m_bikeVelocity * g_pGameVars->frametime;
			Util::TraceHull(m_pState->origin, traceEnd, false, false, HULL_HUMAN, m_pEdict, tr);

			Vector velDir = m_pState->velocity;
			velDir.Normalize();

			// Determine if we can slide off a bit
			if(Math::DotProduct(velDir, tr.plane.normal) < 0.6 && tr.fraction != 1.0)
			{
				// Save previous fraction
				Double groundFraction = tr.fraction;
				Vector raisedOrigin = m_pState->origin + Vector(0, 0, 18);
				traceEnd = raisedOrigin + m_bikeVelocity * g_pGameVars->frametime;
				Util::TraceHull(m_pState->origin, traceEnd, false, false, HULL_HUMAN, m_pEdict, tr);

				if(groundFraction >= tr.fraction && flDiff >= MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)
				{
					Float impactDamage = (flDiff-MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)/(MOTORBIKE_FATAL_COLLISON_SPEED-MOTORBIKE_MINIMUM_DAMAGE_COLLISON_SPEED)*100;
					TakeDamage(m_pBikeEntity, m_pBikeEntity, impactDamage, DMG_FALL);
				}
			}
		}

		// Copy over value sent by client
		m_bikeAcceleration = m_pState->fuser2;
		// Store current velocity
		m_bikeVelocity = m_pState->velocity;
	}
}

//=============================================
// @brief
//
//=============================================
daystage_t CPlayerEntity::GetDayStage( void ) const
{
	return (daystage_t)m_dayStage;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetDayStage( daystage_t daystage )
{
	m_dayStage = daystage;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetSpecialFog( bool specialfogenabled )
{
	m_specialFogEnabled = specialfogenabled;
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SetViewEntity( CBaseEntity* pEntity )
{
	Int32 entityindex = pEntity ? pEntity->GetEntityIndex() : NO_ENTITY_INDEX;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.setviewentity, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteInt16(entityindex);
	gd_engfuncs.pfnUserMessageEnd();

	m_pTriggerCameraEntity = pEntity;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::IsInView( CBaseEntity* pOther ) const
{
	return Util::IsInViewCone(GetEyePosition(), m_pState->angles, PLAYER_NPC_AWARENESS_FIELD_OF_VIEW, pOther->GetCenter());
}

//=============================================
// @brief
//
//=============================================
bool CPlayerEntity::IsInView( const Vector& position ) const
{
	return Util::IsInViewCone(GetEyePosition(), m_pState->angles, PLAYER_NPC_AWARENESS_FIELD_OF_VIEW, position);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::PlayMusic( const Char* pstrFilename, Int32 channel, Float fadeInTime, Int32 flags )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	if(channel == MUSIC_CHANNEL_MENU)
	{
		gd_engfuncs.pfnCon_Printf("%s - Music channel '%d' is reserved for menu music.\n", __FUNCTION__, channel);
		return;
	}

	if(flags & OGG_FL_STOP)
	{
		for(Int32 i = 0; i < NB_MUSIC_CHANNELS; i++)
		{
			music_data_t& track = m_musicPlaybackInfoArray[i];
			if(track.channel == channel)
			{
				track.filename = NO_STRING_VALUE;
				track.begintime = 0;
				track.flags = 0;
				track.channel = 0;

				gd_engfuncs.pfnStopMusic(m_pEdict->clientindex, nullptr, channel, 0);
			}
		}

		return;
	}

	music_data_t* ptrack = nullptr;

	// Try to find a track with the same channel
	for(Int32 i = 0; i < NB_MUSIC_CHANNELS; i++)
	{
		music_data_t& track = m_musicPlaybackInfoArray[i];
		if(track.channel == channel)
		{
			ptrack = &track;
			break;
		}
	}

	// Try to find a freee channel
	if(!ptrack)
	{
		for(Int32 i = 0; i < NB_MUSIC_CHANNELS; i++)
		{
			music_data_t& track = m_musicPlaybackInfoArray[i];
			if(track.filename == NO_STRING_VALUE)
			{
				ptrack = &track;
				break;
			}
		}
	}

	if(!ptrack)
	{
		gd_engfuncs.pfnCon_Printf("Failed to find free slot for '%s' on channel %d.\n", pstrFilename, channel);
		return;
	}

	ptrack->duration = gd_engfuncs.pfnGetSoundDuration(pstrFilename, PITCH_NORM);
	if(!ptrack->duration)
	{
		gd_engfuncs.pfnCon_Printf("Failed to get duration for ogg file '%s'.\n", pstrFilename);
		return;
	}

	ptrack->filename = gd_engfuncs.pfnAllocString(pstrFilename);
	ptrack->begintime = g_pGameVars->time;
	ptrack->flags = flags;
	ptrack->channel = channel;

	gd_engfuncs.pfnPlayMusic(pstrFilename, ptrack->channel, flags, 0, fadeInTime, m_pEdict->clientindex);
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::StopMusic( const Char* pstrFilename, Int32 channel, Float fadeTime )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}

	// Try to find a track with the same channel
	for(Int32 i = 0; i < NB_MUSIC_CHANNELS; i++)
	{
		music_data_t& track = m_musicPlaybackInfoArray[i];
		if(track.channel == channel)
		{
			track.filename = NO_STRING_VALUE;
			track.begintime = 0;
			track.flags = 0;
			track.channel = 0;

			gd_engfuncs.pfnStopMusic(m_pEdict->clientindex, pstrFilename, channel, fadeTime);
		}
	}
}

//=============================================
// @brief Tells if a cheat command is active
//
//=============================================
bool CPlayerEntity::IsUsingCheatCommand( void )
{
	return m_cheatCommandUsed;
}

//=============================================
// @brief Returns blood color setting
//
//=============================================
bloodcolor_t CPlayerEntity::GetBloodColor( void ) 
{ 
	return (bloodcolor_t)BLOOD_RED; 
}

//=============================================
// @brief Unducks the player
//
//=============================================
void CPlayerEntity::UnDuckPlayer( void )
{
	if(!(m_pState->flags & FL_DUCKING))
		return;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HULL_MIN, VEC_HULL_MAX);
	m_pState->view_offset = VEC_VIEW;

	Float offsetZ = SDL_fabs(VEC_HULL_MIN[2]) - SDL_fabs(VEC_DUCK_HULL_MIN[2]);
	m_pState->origin.z += offsetZ;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
	m_pState->flags &= ~FL_DUCKING;
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerEntity::GetBodyTarget( const Vector& targetingPosition )
{
	return (GetCenter()*0.5) + (GetEyePosition()*0.5);
}

//=============================================
// @brief Begins playback of a tape track
//
//=============================================
void CPlayerEntity::PlaybackTapeTrack( const Char* pstrTrackFilename, Float duration, const Char* pstrPlaybackTitle, const Vector& titleColor, Float titleAlpha )
{
	CString checkpath;
	checkpath << SOUND_FOLDER_BASE_PATH << pstrTrackFilename;
	if(!gd_filefuncs.pfnFileExists(checkpath.c_str()))
	{
		gd_engfuncs.pfnClientPrintf(m_pEdict, "%s - File '%s' does not exist.\n", __FUNCTION__, pstrTrackFilename);
		return;
	}

	if(m_tapeTrackFile)
	{
		// Shut up previous sound
		Util::EmitEntitySound(this, gd_engfuncs.pfnGetString(m_tapeTrackFile), SND_CHAN_STREAM, 0, 0, 0, SND_FL_STOP, this);
	}

	m_tapeTrackFile = gd_engfuncs.pfnAllocString(pstrTrackFilename);

	if(pstrPlaybackTitle && pstrPlaybackTitle[0] != '\0')
		m_tapePlaybackTitle = gd_engfuncs.pfnAllocString(pstrPlaybackTitle);
	else
		m_tapePlaybackTitle = gd_engfuncs.pfnAllocString("Casette playback");

	m_tapeTrackPlayBeginTime = g_pGameVars->time;
	m_tapeTrackDuration = duration;

	Util::EmitEntitySound(this, pstrTrackFilename, SND_CHAN_STREAM, VOL_NORM, ATTN_NORM, PITCH_NORM, (SND_FL_REVERBLESS|SND_FL_DIM_OTHERS_LIGHT), this);

	m_tapeTitleColor = titleColor;
	m_tapePlaybackAlpha = titleAlpha;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.radiomessage, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
		gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_tapePlaybackTitle));
		gd_engfuncs.pfnMsgWriteSmallFloat(m_tapeTrackDuration);
		gd_engfuncs.pfnMsgWriteByte(m_tapeTitleColor.x);
		gd_engfuncs.pfnMsgWriteByte(m_tapeTitleColor.y);
		gd_engfuncs.pfnMsgWriteByte(m_tapeTitleColor.z);
		gd_engfuncs.pfnMsgWriteByte(m_tapePlaybackAlpha);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief Stops playback of a tape track
//
//=============================================
void CPlayerEntity::StopTapeTrack( const Char* pstrTrackFilename )
{
	if((m_tapeTrackPlayBeginTime+m_tapeTrackDuration) < g_pGameVars->time 
		|| m_tapeTrackFile == NO_STRING_VALUE 
		|| qstrcmp(gd_engfuncs.pfnGetString(m_tapeTrackFile), pstrTrackFilename))
		return;

	// Stop playback
	Util::EmitEntitySound(this, pstrTrackFilename, SND_CHAN_STREAM, 0, 0, 0, SND_FL_STOP, this);

	// Remove radio icon
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.radiomessage, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
		gd_engfuncs.pfnMsgWriteString("/0");
		gd_engfuncs.pfnMsgWriteSmallFloat(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteByte(0);
	gd_engfuncs.pfnUserMessageEnd();

	m_tapeTrackFile = NO_STRING_VALUE;
	m_tapePlaybackTitle = NO_STRING_VALUE;
	m_tapeTrackPlayBeginTime = 0;
	m_tapeTrackDuration = 0;
	m_tapeTitleColor.Clear();
	m_tapePlaybackAlpha = 0;
}

//=============================================
// @brief Begins diary playback
//
//=============================================
void CPlayerEntity::BeginDiaryPlayback( const Char* pstrFilename, Float duration, CBaseEntity* pDiaryEntity )
{
	m_currentDiaryTrackName = gd_engfuncs.pfnAllocString(pstrFilename);
	m_diaryTrackPlayBeginTime = g_pGameVars->time;
	m_diaryTrackDuration = duration;
	m_diaryEntity = pDiaryEntity;
}

//=============================================
// @brief Performs think functions while playing diary
//
//=============================================
void CPlayerEntity::DiaryPlaybackThink( void )
{
	if(m_currentDiaryTrackName == NO_STRING_VALUE
		|| !m_diaryTrackPlayBeginTime
		|| !m_diaryTrackDuration
		|| !m_diaryEntity)
		return;

	if((m_diaryTrackPlayBeginTime + m_diaryTrackDuration) <= g_pGameVars->time)
	{
		m_currentDiaryTrackName = NO_STRING_VALUE;
		m_diaryTrackPlayBeginTime = 0;
		m_diaryTrackDuration = 0;
		m_diaryEntity.reset();
		return;
	}

	if(m_diaryEntity->GetSpawnFlags() & CItemDiary::FL_CANNOT_INTERRUPT)
		return;

	if(m_pState->buttons & IN_USE)
	{
		gd_engfuncs.pfnApplySoundEffect(GetEntityIndex(), gd_engfuncs.pfnGetString(m_currentDiaryTrackName), SND_CHAN_VOICE, SND_EF_CHANGE_VOLUME, 1, 0, GetClientIndex());

		m_diaryEntity->StopDiaryPlayback();
		m_diaryEntity.reset();

		m_currentDiaryTrackName = NO_STRING_VALUE;
		m_diaryTrackPlayBeginTime = 0;
		m_diaryTrackDuration = 0;
		return;
	}
}

//=============================================
// @brief Performs think functions for tape track playback
//
//=============================================
void CPlayerEntity::TapePlaybackThink( void )
{
	if(m_tapeTrackFile == NO_STRING_VALUE)
		return;

	if((g_pGameVars->time - m_tapeTrackPlayBeginTime) > m_tapeTrackDuration)
	{
		m_tapeTrackFile = NO_STRING_VALUE;
		m_tapePlaybackTitle = NO_STRING_VALUE;
		m_tapeTrackPlayBeginTime = 0;
		m_tapeTrackDuration = 0;
		m_tapeTitleColor.Clear();
		m_tapePlaybackAlpha = 0;
	}
}

//=============================================
// @brief Sets dialouge duration for player
//
//=============================================
void CPlayerEntity::SetDialougeDuration( Float duration )
{
	m_dialougePlaybackTime = g_pGameVars->time + duration;
}

//=============================================
// @brief Adds a new mission objective
//
//=============================================
void CPlayerEntity::AddMissionObjective( const Char* pstrObjectiveIdentifier, bool notify )
{
	if(!pstrObjectiveIdentifier || !qstrlen(pstrObjectiveIdentifier))
		return;

	Uint32 nbObjectives = 0;
	for(Uint32 i = 0; i < GAMEUI_MAX_OBJECTIVES; i++)
	{
		if(m_objectivesArray[i] == NO_STRING_VALUE)
			break;

		const Char* pstrCheckObjective = gd_engfuncs.pfnGetString(m_objectivesArray[i]);
		if(!qstrcmp(pstrCheckObjective, pstrObjectiveIdentifier))
		{
			gd_engfuncs.pfnCon_Printf("%s - Objective '%s' already in list.\n", __FUNCTION__, pstrObjectiveIdentifier);
			return;
		}

		nbObjectives++;
	}

	if(nbObjectives >= GAMEUI_MAX_OBJECTIVES)
	{
		gd_engfuncs.pfnCon_EPrintf("%s - Exceeded GAMEUI_MAX_OBJECTIVES.\n", __FUNCTION__);
		return;
	}

	Int16 shiftedNewBitFlags = 0;
	for(Uint32 i = (GAMEUI_MAX_OBJECTIVES-1); i > 0; i--)
	{
		if(m_objectivesArray[i-1] == NO_STRING_VALUE)
			continue;

		m_objectivesArray[i] = m_objectivesArray[i-1];
		if(m_objectivesNewFlags & (1<<(i-1)))
			shiftedNewBitFlags |= (1<<i);
	}

	m_objectivesArray[0] = gd_engfuncs.pfnAllocString(pstrObjectiveIdentifier);
	m_objectivesNewFlags = (shiftedNewBitFlags | (1<<0));

	if(notify && m_lastObjectiveAddTime != g_pGameVars->time)
	{
		Util::ShowMessage("OBJECTIVES_UPDATED", this);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.newobjective, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(true);
		gd_engfuncs.pfnUserMessageEnd();

		m_lastObjectiveAddTime = g_pGameVars->time;
	}
}

//=============================================
// @brief Removes a new mission objective
//
//=============================================
void CPlayerEntity::RemoveMissionObjective( const Char* pstrObjectiveIdentifier, bool notify )
{
	if(!pstrObjectiveIdentifier || !qstrlen(pstrObjectiveIdentifier))
		return;

	Uint32 i = 0;
	for(; i < GAMEUI_MAX_OBJECTIVES; i++)
	{
		if(m_objectivesArray[i] == NO_STRING_VALUE)
			continue;

		const Char* pstrCheckObjective = gd_engfuncs.pfnGetString(m_objectivesArray[i]);
		if(!qstrcmp(pstrCheckObjective, pstrObjectiveIdentifier))
		{
			m_objectivesArray[i] = NO_STRING_VALUE;
			break;
		}
	}

	if(i == GAMEUI_MAX_OBJECTIVES)
	{
		gd_engfuncs.pfnCon_Printf("%s - Objective '%s' not in list.\n", __FUNCTION__, pstrObjectiveIdentifier);
		return;
	}

	// Remove flag
	m_objectivesNewFlags &= ~(1<<i);

	// Re-sort objectives array
	Uint32 newNbObjectives = 0;
	string_t sortedObjectivesArray[GAMEUI_MAX_OBJECTIVES];
	Int16 sortedObjectivesNewFlags = 0;

	for(i = 0; i < GAMEUI_MAX_OBJECTIVES; i++)
	{
		if(m_objectivesArray[i] == NO_STRING_VALUE)
			continue;

		if(m_objectivesNewFlags & (1<<i))
			sortedObjectivesNewFlags |= (1<<newNbObjectives);

		sortedObjectivesArray[newNbObjectives] = m_objectivesArray[i];
		newNbObjectives++;
	}

	i = 0;
	for(; i < newNbObjectives; i++)
		m_objectivesArray[i] = sortedObjectivesArray[i];

	for(; i < GAMEUI_MAX_OBJECTIVES; i++)
		m_objectivesArray[i] = NO_STRING_VALUE;

	newNbObjectives = sortedObjectivesNewFlags;

	if(notify && m_lastObjectiveAddTime != g_pGameVars->time)
	{
		Util::ShowMessage("OBJECTIVES_UPDATED", this);

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.newobjective, nullptr, m_pEdict);
			gd_engfuncs.pfnMsgWriteByte(true);
		gd_engfuncs.pfnUserMessageEnd();

		m_lastObjectiveAddTime = g_pGameVars->time;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerEntity::SpawnObjectivesWindow( void )
{
	Uint32 nbObjectives = 0;
	for(Uint32 i = 0; i < GAMEUI_MAX_OBJECTIVES; i++)
	{
		if(m_objectivesArray[i] == NO_STRING_VALUE)
			break;

		nbObjectives++;
	}

	// Write message to client
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategameuiwindow, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteByte(GAMEUI_OBJECTIVESWINDOW);
		gd_engfuncs.pfnMsgWriteByte(nbObjectives);
		gd_engfuncs.pfnMsgWriteByte(m_objectivesNewFlags);
		for(Uint32 i = 0; i < nbObjectives; i++)
			gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_objectivesArray[i]));
	gd_engfuncs.pfnUserMessageEnd();

	m_hasActiveUIWindows = true;
}

//=============================================
// @brief Sets countdown timer
//
//=============================================
void CPlayerEntity::SetCountdownTimer( Float duration, const Char* pstrTitle )
{
	if(!duration)
		return;

	m_countdownTimerEndTime = g_pGameVars->time + duration;
	if(pstrTitle && qstrlen(pstrTitle))
		m_countdownTimerTitle = gd_engfuncs.pfnAllocString(pstrTitle);
	else
		m_countdownTimerTitle = NO_STRING_VALUE;

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetcountdowntimer, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteDouble(m_countdownTimerEndTime);
		gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_countdownTimerTitle));
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief Clears countdown timer
//
//=============================================
void CPlayerEntity::ClearCountdownTimer( void )
{
	m_countdownTimerEndTime = 0;
	m_countdownTimerTitle = NO_STRING_VALUE;

	// Just clear on client
	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudsetcountdowntimer, nullptr, m_pEdict);
		gd_engfuncs.pfnMsgWriteDouble(0);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief Sets a global delayed trigger
//
//=============================================
void CPlayerEntity::SetGlobalDelayedTrigger( Float delay, const Char* pstrTargetName )
{
	if(!delay || !pstrTargetName || !qstrlen(pstrTargetName))
		return;

	m_delayedGlobalTriggerTime = g_pGameVars->time + delay;
	m_delayedGlobalTriggerTarget = gd_engfuncs.pfnAllocString(pstrTargetName);
}

//=============================================
// @brief Clears a global delayed trigger
//
//=============================================
void CPlayerEntity::ClearGlobalDelayedTrigger( void )
{
	m_delayedGlobalTriggerTime = 0;
	m_delayedGlobalTriggerTarget = NO_STRING_VALUE;
}

//=============================================
// @brief Think function for the global trigger
//
//=============================================
void CPlayerEntity::DelayedGlobalTriggerThink( void )
{
	if(!m_delayedGlobalTriggerTime || m_delayedGlobalTriggerTarget == NO_STRING_VALUE)
		return;

	if(m_delayedGlobalTriggerTime <= g_pGameVars->time)
	{
		UseTargets(this, USE_TOGGLE, 0, m_delayedGlobalTriggerTarget);
		ClearGlobalDelayedTrigger();
	}
}