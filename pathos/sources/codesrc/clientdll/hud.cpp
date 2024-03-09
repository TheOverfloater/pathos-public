/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"

#include "hud.h"
#include "clientdll.h"
#include "huddraw.h"
#include "fontset.h"
#include "textures_shared.h"
#include "cl_entity.h"
#include "buttonbits.h"
#include "view.h"
#include "com_math.h"
#include "snd_shared.h"
#include "weapons_shared.h"
#include "input.h"
#include "gameuimanager.h"
#include "r_common.h"
#include "matrix.h"
#include "clshared.h"

// Class definition
CGameHUD gHUD;

// Time an item spends active
const Float CHUDHistory::HUD_HISTORY_DRAW_TIME = 10;
// Max history height
const Float CHUDHistory::MAX_HISTORY_HEIGHT = 400;

// HUD white color
const Vector CGameHUD::HUD_COLOR_WHITE = Vector(255, 255, 255);
// HUD gray color
const Vector CGameHUD::HUD_COLOR_GRAY = Vector(128, 128, 128);
// HUD black color
const Vector CGameHUD::HUD_COLOR_BLACK = Vector(0, 0, 0);
// HUD red color
const Vector CGameHUD::HUD_COLOR_RED = Vector(200, 0, 0);
// HUD blue color
const Vector CGameHUD::HUD_COLOR_BLUE = Vector(100, 100, 250);
// HUD orange color
const Vector CGameHUD::HUD_COLOR_ORANGE = Vector(255, 160, 0);

// History spacing
const Uint32 CHUDHistory::HISTORY_SPACING = 5;
// Text color
color32_t CHUDHistory::HISTORY_TEXT_COLOR = color32_t(150, 150, 250, 255);
// Fade time for history texts
const Float CHUDHistory::HISTORY_FADE_TIME = 1.0;

// Default HUD alpha value
const Float CGameHUD::HUD_DEFAULT_ALPHA = 128;

// Tilt amount
const Uint32 CGameHUD::TILT_AMOUNT = 6;

// Generic tab size
const Uint32 CGameHUD::TAB_GENERIC_SIZE_X = 225;
const Uint32 CGameHUD::TAB_GENERIC_SIZE_Y = 35;
const Uint32 CGameHUD::TAB_MOVENOISE_SIZE_X = 115;

// Radio message tab width
const Uint32 CGameHUD::RADIO_MSGTAB_SIZE_X = 300;
const Uint32 CGameHUD::RADIO_MSGTAB_SIZE_Y = 60;
const Uint32 CGameHUD::RADIO_MSGTAB_SPACING = 10;
const Float CGameHUD::RADIO_MSG_FADETIME = 0.5;

// Tab label size
const Uint32 CGameHUD::TAB_LABEL_SIZE_X = 75;
// Heal tab width
const Uint32 CGameHUD::TAB_HEAL_SIZE_X = 300;

// Healthkit tab width
const Uint32 CGameHUD::TAB_HEALTHKIT_SIZE_X = 110;

// Tactical icon width
const Uint32 CGameHUD::TACTICAL_ICON_X = 25;
// Tactical icon height
const Uint32 CGameHUD::TACTICAL_ICON_Y = 15;
// Tactical tab width
const Uint32 CGameHUD::TAB_TACTICAL_SIZE_X = 180;

// Weapon tab width
const Uint32 CGameHUD::TAB_WEAPON_SIZE_X = 300;
// Clip label width
const Uint32 CGameHUD::LABEL_CLIP_SIZE_X = 70;

// Percentage bar height
const Uint32 CGameHUD::PERCENTAGE_BAR_HEIGHT = 5;

// Icon size
const Uint32 CGameHUD::ICON_SIZE = 30;
// Ammo icon width
const Float CGameHUD::AMMOICON_SIZE_X = 20;
// Ammo icon height
const Float CGameHUD::AMMOICON_SIZE_Y = 12.5;

// Weapon rubicle width
const Uint32 CGameHUD::RUBICLE_WEAPONINFO_SIZE_X = 250;

// List label T width
const Uint32 CGameHUD::LIST_LABEL_T_X = 26;
// List label T height
const Uint32 CGameHUD::LIST_LABEL_T_Y = 6;
// List label S width
const Uint32 CGameHUD::LIST_LABEL_S_X = 6;
// List label S height
const Uint32 CGameHUD::LIST_LABEL_S_Y = 20;

// Weapon tab X
const Uint32 CGameHUD::LIST_WEAPONTAB_X = 200;
// Weapon tab Y
const Uint32 CGameHUD::LIST_WEAPONTAB_Y = 75;
// List edge size
const Uint32 CGameHUD::LIST_EDGE_SIZE = 2;

// Subtitle tab width
const Uint32 CGameHUD::SUBTITLE_TAB_SIZE_X = 600;
// Subtitle tab height
const Uint32 CGameHUD::SUBTITLE_TAB_SIZE_Y = 120;
// Subtitle timeout duration
const Float CGameHUD::SUBTITLE_TAB_TIMEOUT = 1;
// Subtitle tab fade out time
const Float	CGameHUD::SUBTITLE_TAB_FADETIME = 0.5;
// Subtitle tab fade in time
const Float	CGameHUD::SUBTITLE_TAB_FADEINTIME = 0.5;

// Ammo bar height
const Uint32 CGameHUD::AMMOBAR_SIZE_Y = 15;
// Ammo bar width
const Uint32 CGameHUD::AMMOBAR_SIZE_X = 50;

// Stamina fade time
const Uint32 CGameHUD::STAMINA_FADE_TIME = 2;

// Subtitle spacing
const Float CGameHUD::SUBTITLE_GAP = 25;

// Subtitle inset
const Float CGameHUD::SUBTITLE_INSET = 5;

// HUD description script path
const Char CGameHUD::HUD_DESCRIPTION_SCRIPT_PATH[] = "/scripts/hud.txt";

// Font set for HUD
const Char CGameHUD::HUD_FONT_SCHEMA_FILENAME[] = "hud_text";
// Font set for counters
const Char CGameHUD::HUD_COUNTER_FONT_SCHEMA_FILENAME[] = "hud_counter";
// Font set for subtitles
const Char CGameHUD::HUD_SUBTITLE_FONT_SCHEMA_FILENAME[] = "hud_subtitles";

weapon_mapping_t WEAPONMAPPINGS[] = {
	DEFINE_WEAPON_MAPPING(WEAPON_NONE),
	DEFINE_WEAPON_MAPPING(WEAPON_GLOCK),
	DEFINE_WEAPON_MAPPING(WEAPON_HANDGRENADE),
	DEFINE_WEAPON_MAPPING(WEAPON_KNIFE),
	0, ""
};

// Command functions
void Cmd_Slot1( void )		{ gHUD.SlotInput( 0 ); }
void Cmd_Slot2( void )		{ gHUD.SlotInput( 1 ); }
void Cmd_Slot3( void )		{ gHUD.SlotInput( 2 ); }
void Cmd_Slot4( void )		{ gHUD.SlotInput( 3 ); }
void Cmd_Slot5( void )		{ gHUD.SlotInput( 4 ); }
void Cmd_Slot6( void )		{ gHUD.SlotInput( 5 ); }
void Cmd_Slot7( void )		{ gHUD.SlotInput( 6 ); }
void Cmd_Slot8( void )		{ gHUD.SlotInput( 7 ); }
void Cmd_Slot9( void )		{ gHUD.SlotInput( 8 ); }
void Cmd_Slot10( void )		{ gHUD.SlotInput( 9 ); }
void Cmd_Close( void )		{ gHUD.UserCmd_Close(); }
void Cmd_NextWeapon( void )	{ gHUD.UserCmd_NextWeapon(); }
void Cmd_PrevWeapon( void )	{ gHUD.UserCmd_PrevWeapon(); }

//=============================================
// @brief
//
//=============================================
CGameHUD::CGameHUD( void ):
	m_pHealthIcon(nullptr),
	m_pKevlarIcon(nullptr),
	m_pTacticalIcon(nullptr),
	m_pMedkitIcon(nullptr),
	m_pStaminaIcon(nullptr),
	m_pNoiseIcon(nullptr),
	m_pAwarenessIcon(nullptr),
	m_pRadioIcon(nullptr),
	m_pNewObjectivesIcon(nullptr),
	m_pActiveWeaponInfo(nullptr),
	m_pCvarDrawSubtitles(nullptr),
	m_pActiveSelection(nullptr),
	m_pLastSelection(nullptr),
	m_pWeapon(nullptr),
	m_weaponSelection(0),
	m_isDrySelection(false),
	m_pHistory(nullptr),
	m_pFontSet(nullptr),
	m_pSubtitleSet(nullptr),
	m_pCounterFont(nullptr),
	m_stamina(0),
	m_health(0),
	m_kevlar(0),
	m_staminaFadeTime(0),
	m_isTacticalOn(false),
	m_tacticalAmount(0),
	m_healProgress(0),
	m_numMedkits(0),
	m_movementNoise(0),
	m_npcAwareness(0),
	m_newObjective(false),
	m_countdownTimerTime(0),
	m_isOnTarget(false),
	m_prevWeaponBits(0),
	m_weaponBits(0),
	m_keyBits(0),
	m_isActive(false),
	m_weaponSelectUserMSGId(0),
	m_screenWidth(0),
	m_screenHeight(0),
	m_usableObjectType(USABLE_OBJECT_DEFAULT),
	m_pCvarDrawUseReticle(nullptr)
{
	m_pHistory = new CHUDHistory(*this);
}

//=============================================
// @brief
//
//=============================================
CGameHUD::~CGameHUD( void )
{
	ClearGame();
	Shutdown();
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::Init( void ) 
{
	cl_engfuncs.pfnCreateCommand("slot1", Cmd_Slot1, nullptr);
	cl_engfuncs.pfnCreateCommand("slot2", Cmd_Slot2, nullptr);
	cl_engfuncs.pfnCreateCommand("slot3", Cmd_Slot3, nullptr);
	cl_engfuncs.pfnCreateCommand("slot4", Cmd_Slot4, nullptr);
	cl_engfuncs.pfnCreateCommand("slot5", Cmd_Slot5, nullptr);
	cl_engfuncs.pfnCreateCommand("slot6", Cmd_Slot6, nullptr);
	cl_engfuncs.pfnCreateCommand("slot7", Cmd_Slot7, nullptr);
	cl_engfuncs.pfnCreateCommand("slot8", Cmd_Slot8, nullptr);
	cl_engfuncs.pfnCreateCommand("slot9", Cmd_Slot9, nullptr);
	cl_engfuncs.pfnCreateCommand("slot10", Cmd_Slot10, nullptr);
	cl_engfuncs.pfnCreateCommand("cancelselect", Cmd_Close, nullptr);
	cl_engfuncs.pfnCreateCommand("invnext", Cmd_NextWeapon, nullptr);
	cl_engfuncs.pfnCreateCommand("invprev", Cmd_PrevWeapon, nullptr);

	m_weaponInfo.Reset();
	m_pHistory->Reset();

	m_pCvarDrawSubtitles = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_subtitles", "1", "Controls subtitle rendering");
	m_pCvarDrawUseReticle = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "hud_usereticle", "1", "Toggle rendering of use reticle");

	// Create new usermsgs for weapon select
	m_weaponSelectUserMSGId = cl_engfuncs.pfnRegisterClientUserMessage("SelectWeapon", -1);
	if(!m_weaponSelectUserMSGId)
		cl_engfuncs.pfnCon_Printf("%s - Failed to register message 'SelectWeapon'.\n", __FUNCTION__);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::Shutdown( void ) 
{
	ClearGame();

	if(m_pHistory)
	{
		delete m_pHistory;
		m_pHistory = nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::InitGame( void ) 
{
	// Retrieve screen size
	cl_renderfuncs.pfnGetScreenSize(m_screenWidth, m_screenHeight);

	// Load HUD script
	if(!LoadHUDScript())
	{
		cl_engfuncs.pfnCon_EPrintf("Failed to initialize HUD.\n");
		return false;
	}

	// Set icon textures
	m_pHealthIcon = GetIconTextureByName("health");
	m_pKevlarIcon = GetIconTextureByName("kevlar");
	m_pTacticalIcon = GetIconTextureByName("tactical");
	m_pMedkitIcon = GetIconTextureByName("medkit");
	m_pStaminaIcon = GetIconTextureByName("stamina");
	m_pRadioIcon = GetIconTextureByName("radio");
	m_pNewObjectivesIcon = GetIconTextureByName("newobjectives");
	m_pNoiseIcon = GetIconTextureByName("noise");
	m_pAwarenessIcon = GetIconTextureByName("awareness");

	// Load subtitles
	if(!InitSubtitles())
	{
		cl_engfuncs.pfnCon_Printf("%s - Failed to load subtitles.\n", __FUNCTION__);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::InitGL( void ) 
{
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(HUD_FONT_SCHEMA_FILENAME, screenHeight);
	if(!m_pFontSet)
	{
		cl_engfuncs.pfnCon_Printf("%s - Failed to load font schema '%s'.\n", __FUNCTION__, HUD_FONT_SCHEMA_FILENAME);
		return false;
	}

	m_pCounterFont = cl_engfuncs.pfnGetResolutionSchemaFontSet(HUD_COUNTER_FONT_SCHEMA_FILENAME, screenHeight);
	if(!m_pCounterFont)
	{
		cl_engfuncs.pfnCon_Printf("%s - Failed to load font schema '%s'.\n", __FUNCTION__, HUD_COUNTER_FONT_SCHEMA_FILENAME);
		return false;
	}

	m_pSubtitleSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(HUD_SUBTITLE_FONT_SCHEMA_FILENAME, screenHeight);
	if(!m_pSubtitleSet)
	{
		cl_engfuncs.pfnCon_Printf("%s - Failed to load font schema '%s'.\n", __FUNCTION__, HUD_SUBTITLE_FONT_SCHEMA_FILENAME);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::ClearGL( void ) 
{
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::ClearGame( void ) 
{
	// Reset these
	m_pActiveSelection = nullptr;
	m_pWeapon = nullptr;
	m_isDrySelection = false;

	m_weaponInfo.Reset();

	if(m_pHistory)
		m_pHistory->Reset();

	m_isActive = false;

	m_prevWeaponBits = 0;
	m_weaponBits = 0;
	m_npcAwareness = 0;
	m_movementNoise = 0;
	m_numMedkits = 0;
	m_healProgress = 0;
	m_tacticalAmount = 0;
	m_isTacticalOn = false;
	m_staminaFadeTime = 0;
	m_kevlar = 0;
	m_health = 0;
	m_stamina = 0;

	m_newObjective = false;

	if(!m_subtitlesArray.empty())
		m_subtitlesArray.clear();

	if(!m_activeSubtitlesList.empty())
		m_activeSubtitlesList.clear();

	if(!m_radioMessagesList.empty())
		m_radioMessagesList.clear();

	if(!m_iconInfoArray.empty())
		m_iconInfoArray.clear();

	if(!m_weaponInfoArray.empty())
		m_weaponInfoArray.clear();

	if(!m_itemInfoArray.empty())
		m_itemInfoArray.clear();

	if(!m_countdownTimerTitle.empty())
		m_countdownTimerTitle.clear();

	m_countdownTimerTime = 0;

	m_idealAutoAimVector.Clear();
	m_currentAutoAimVector.Clear();
	m_isOnTarget = false;

	m_usableObjectMins.Clear();
	m_usableObjectMaxs.Clear();
	m_usableObjectType = USABLE_OBJECT_NONE;
}

//=============================================
// @brief Returns an icon texture by name 
//
//=============================================
en_texture_t* CGameHUD::GetIconTextureByName( const Char* pstrName )
{
	for(Uint32 i = 0; i < m_iconInfoArray.size(); i++)
	{
		if(!qstrcmp(pstrName, m_iconInfoArray[i].name))
			return m_iconInfoArray[i].ptexture;
	}

	cl_engfuncs.pfnCon_EPrintf("%s - Icon definition '%s' not found.\n", __FUNCTION__, pstrName);
	return cl_renderfuncs.pfnGetDummyTexture();
}

//=============================================
// @brief Returns an item description by name
//
//=============================================
const Char* CGameHUD::GetItemDescriptionByName( const Char* pstrName )
{
	for(Uint32 i = 0; i < m_itemInfoArray.size(); i++)
	{
		if(!qstrcmp(m_itemInfoArray[i].name, pstrName))
			return m_itemInfoArray[i].description.c_str();
	}

	return "NA";
}

//=============================================
// @brief Returns an item description by weapon id
//
//=============================================
const Char* CGameHUD::GetItemDescriptionByWeaponId( weaponid_t weaponid )
{
	for(Uint32 i = 0; i < m_itemInfoArray.size(); i++)
	{
		if(m_itemInfoArray[i].weaponid == weaponid)
			return m_itemInfoArray[i].description.c_str();
	}

	return "NA";
}

//=============================================
// @brief Returns a weapon info structure based on id
//
//=============================================
const CGameHUD::hud_weaponinfo_t* CGameHUD::GetWeaponInfoById( weaponid_t weaponid )
{
	for(Uint32 i = 0; i < m_weaponInfoArray.size(); i++)
	{
		if(m_weaponInfoArray[i].weaponid == weaponid)
			return &m_weaponInfoArray[i];
	}

	return nullptr;
}

//=============================================
// @brief Returns a weapon icon texture based on id
//
//=============================================
en_texture_t* CGameHUD::GetWeaponIconById( weaponid_t weaponid )
{
	for(Uint32 i = 0; i < m_weaponInfoArray.size(); i++)
	{
		if(m_weaponInfoArray[i].weaponid == weaponid)
			return m_weaponInfoArray[i].pweapon_icon;
	}

	return cl_renderfuncs.pfnGetDummyTexture();
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::InitSubtitles( void ) 
{
	Uint32 isize = 0;
	const Char *pfile = reinterpret_cast<const Char *>(cl_filefuncs.pfnLoadFile("scripts/subtitles.txt", &isize));
	if(!pfile || !isize)
		return false;
	
	CString szname;
	CString szline;
	CString sztoken;
	
	// Reserver 2048 entires
	m_subtitlesArray.reserve(2048);

	const Char *pscan = pfile;
	while(pscan)
	{
		Float r, g, b;
		Float delay;	
		
		// Reset these to defaults
		delay = 0;
		r = g = b = 255;

		// try reading any options
		while(TRUE)
		{
			// Parse the line in
			pscan = Common::ReadLine(pscan, szline);
			if(!pscan && szline.empty())
				break;

			// Read the token in
			const Char* pslinescan = Common::Parse(szline.c_str(), sztoken);
			if(sztoken.empty())
				continue;

			if(!qstrcmp(sztoken, "{"))
				break;

			if(!qstrcmp(sztoken, "$delay"))
			{
				// check for errors
				if(!pslinescan)
				{
					cl_engfuncs.pfnCon_Printf("Error in subtitles.txt: at token %s, value expected", sztoken.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				// parse the delay
				Common::Parse(pslinescan, sztoken);
				delay = SDL_atof(sztoken.c_str());
			}
			else if(!qstrcmp(sztoken, "$color"))
			{
				// check for errors
				if(!pslinescan)
				{
					cl_engfuncs.pfnCon_Printf("Error in subtitles.txt: at token %s, value expected", sztoken.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				Int32 ir, ig, ib;
				Int32 numread = sscanf(pslinescan, "%d %d %d", &ir, &ig, &ib);

				if(numread != 3)
				{
					cl_engfuncs.pfnCon_Printf("Error in subtitles.txt: at token %s, 3 values expected, got %d", sztoken.c_str(), numread);
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}
				r = ir; g = ig; b = ib;
			}
			else
			{
				// anything else is the identifier of the subtitle
				szname = sztoken;
			}
		}

		// Break if we reached the eof
		if(!pscan)
			break;

		if(qstrcmp(sztoken, "{"))
		{
			cl_engfuncs.pfnCon_Printf("Error in subtitles.txt: invalid token at %s, expected {.\n", sztoken.c_str());
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		subtitle_t newsubtitle;

		// set values
		newsubtitle.r = r;
		newsubtitle.g = g;
		newsubtitle.b = b;
		newsubtitle.delay = delay;
		newsubtitle.szname = szname;
		newsubtitle.szname.tolower();

		// Find the end
		const Char *pend = qstrstr(pscan, "}");
		if(!pend)
		{
			cl_engfuncs.pfnCon_Printf("Error in subtitles.txt: definition %s is missing the closing (}) brace.\n", sztoken.c_str());
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		// Check for user errors
		const Char *ptest = pscan;
		while(ptest < pend)
		{
			if(*ptest == '{')
			{
				cl_engfuncs.pfnCon_Printf("Error in subtitles.txt: definition %s is missing the closing (}) brace.\n", sztoken.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			ptest++;
		}
	
		Int32 length = (pend-1)-pscan;
		newsubtitle.subtitle.assign(pscan, length);
		
		// Add to the list
		m_subtitlesArray.push_back(newsubtitle);

		pscan = ptest;
		pscan++; // skip brace
	}

	// Resize to actual size
	m_subtitlesArray.resize(m_subtitlesArray.size());

	cl_filefuncs.pfnFreeFile(pfile);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::Think( void )
{
	Double currenttime = cl_engfuncs.pfnGetClientTime();
	Double frametime = currenttime - m_lastThinkTime;
	m_lastThinkTime = currenttime;

	// Don't render if player is dead
	if ( m_health <= 0 )
		return;

	// Update autoaim
	CL_UpdateAutoAim(frametime, m_idealAutoAimVector, m_currentAutoAimVector);

	// Update weapon bits
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	m_weaponBits = pplayer->curstate.weapons;

	// get button bits
	m_keyBits = CL_GetButtonBits( false );

	if ( m_weaponBits != m_prevWeaponBits )
	{
		m_prevWeaponBits = m_weaponBits;

		for (Int32 i = MAX_WEAPONS-1; i > 0; i-- )
		{
			weapon_t *pweapon = m_weaponInfo.GetWeapon(i);

			if(pweapon)
			{
				if (m_weaponBits & (1<<pweapon->weaponid))
					m_weaponInfo.AssignWeapon(pweapon);
				else
					m_weaponInfo.RemoveWeapon(pweapon);
			}
		}
	}

	if (m_pActiveSelection || m_isDrySelection)
	{
		// has the player selected one?
		if (m_keyBits & IN_ATTACK)
		{
			if (!m_isDrySelection)
			{
				if(m_weaponSelectUserMSGId)
				{
					cl_engfuncs.pfnClientUserMessageBegin(m_weaponSelectUserMSGId);
					cl_engfuncs.pfnMsgWriteString(m_pActiveSelection->name.c_str());
					cl_engfuncs.pfnClientUserMessageEnd();
				}

				m_weaponSelection = m_pActiveSelection->weaponid;
			}

			m_pLastSelection = m_pActiveSelection;
			m_pActiveSelection = nullptr;
			m_isDrySelection = false;
			m_keyBits &= ~IN_ATTACK;

			PlaySound("common/wpn_select.wav");
		}

		// Update button bits
		CL_ResetButtonBits(m_keyBits);
	}
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawNormal( void ) 
{
	// We need to grab these in DrawNormal, when we still have
	// access to the world-space matrices
	m_modelviewMatrix.SetMatrix(cl_renderfuncs.pfnGetModelViewMatrix().GetMatrix());
	m_projectionMatrix.SetMatrix(cl_renderfuncs.pfnGetProjectionMatrix().GetMatrix());
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::Draw( void ) 
{
	// Don't draw HUD is we have an active window
	if(gGameUIManager.HasActiveWindows())
		return true;

	Float sizex, sizey;

	if(!gHUDDraw.SetupDraw())
	{
		gHUDDraw.ManageErrorMessage();
		return false;
	}

	Double fltime = cl_engfuncs.pfnGetClientTime();

	if (m_isActive)
	{
		Float x = gHUDDraw.ScaleX(50);
		Float y = gHUDDraw.ScaleY(BASE_RESOLUTION_Y-50-TAB_GENERIC_SIZE_Y);

		// Draw armor
		if(!DrawTab_Bar(x, y, nullptr, m_pKevlarIcon, m_kevlar, TAB_GENERIC_SIZE_X, 1.0, &sizex, &sizey))
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}

		y -= sizey+gHUDDraw.ScaleY(10);

		// Draw health bar
		if(!DrawTab_Bar(x, y, nullptr, m_pHealthIcon, m_health, TAB_GENERIC_SIZE_X, 1.0, &sizex, &sizey))
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}
		
		if(m_staminaFadeTime >= fltime)
		{
			// Shift x and reset y
			x += sizex+gHUDDraw.ScaleX(10);
			y = gHUDDraw.ScaleY(BASE_RESOLUTION_Y-50-TAB_GENERIC_SIZE_Y);

			// calculate alpha
			Float alpha = (m_staminaFadeTime - fltime)/STAMINA_FADE_TIME;
			if(alpha > 1.0)
				alpha = 1.0;

			// Draw stamina bar
			if(!DrawTab_Bar(x, y, nullptr, m_pStaminaIcon, m_stamina, TAB_GENERIC_SIZE_X, alpha, &sizex, &sizey))
			{
				gHUDDraw.ManageErrorMessage();
				gHUDDraw.FinishDraw();
				return false;
			}
		}

		// Draw the noise tab
		if(!DrawNoiseTab())
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}

		// Draw the NPC awareness tab
		if(!DrawNPCAwarenessTab())
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}

		// Draw heal bar if we're healing
		if(m_healProgress != -2 && m_healProgress != -1)
		{
			Float center = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X*0.5 - TAB_HEAL_SIZE_X*0.5);
			if(!DrawTab_Bar(center, y, nullptr, nullptr, m_healProgress*100, gHUDDraw.ScaleXRelative(TAB_HEAL_SIZE_X), 1.0))
				return false;
		}

		if (m_pWeapon && m_pWeapon->weaponid != 0)
		{
			// Draw weapon tab
			if(!DrawWeaponTab())
			{
				gHUDDraw.ManageErrorMessage();
				gHUDDraw.FinishDraw();
				return false;
			}
		}

		// Draw healthkit tab
		x = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X) - gHUDDraw.ScaleX(50) - gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X);
		y = gHUDDraw.ScaleY(BASE_RESOLUTION_Y - 50 - TAB_GENERIC_SIZE_Y*3 - CHUDDraw::HUD_EDGE_SIZE*4 - 5 - 2);
		if(!DrawHealthkitTab(x, y))
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}

		// Draw the tactical tab
		x += gHUDDraw.ScaleX(TAB_HEALTHKIT_SIZE_X+5+CHUDDraw::HUD_EDGE_SIZE*2);
		if(!DrawTacticalTab(x, y))
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}

		// Draw the list
		if(!DrawWeaponList())
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}

		// Draw the crosshair
		if(!DrawCrosshair())
		{
			gHUDDraw.ManageErrorMessage();
			gHUDDraw.FinishDraw();
			return false;
		}
	}

	// Draw any subtitles
	if(!DrawSubtitles())
	{
		gHUDDraw.ManageErrorMessage();
		gHUDDraw.FinishDraw();
		return false;
	}

	// Draw any radio messages
	if(!DrawRadioMessages())
	{
		gHUDDraw.ManageErrorMessage();
		gHUDDraw.FinishDraw();
		return false;
	}

	// Draw radio icon
	if(m_newObjective)
	{
		en_texture_t* picon = m_pNewObjectivesIcon;

		Float widthScale = gHUDDraw.ScaleXRelative(picon->width);
		Float heightScale = gHUDDraw.ScaleXRelative(picon->height);

		Float x = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X) - gHUDDraw.ScaleX(50) - widthScale;
		Float y = gHUDDraw.ScaleY(50);

		Float flalpha = SDL_fabs(SDL_sin(fltime*4));
		gHUDDraw.SetColor(HUD_COLOR_BLUE*(0.25+flalpha*0.75), 255);

		glBlendFunc(GL_ONE, GL_ONE);
		gHUDDraw.SetOrigin(x, y);
		gHUDDraw.SetSize(widthScale, heightScale);
	
		if(!gHUDDraw.DrawQuad(picon))
			return false;
	}

	// Draw any radio messages
	if(!DrawUsableObjectReticle())
	{
		gHUDDraw.ManageErrorMessage();
		gHUDDraw.FinishDraw();
		return false;
	}

	// Draw countdown timer
	if(!DrawCountdownTimer())
	{
		gHUDDraw.ManageErrorMessage();
		gHUDDraw.FinishDraw();
		return false;
	}

	// Finish rendering
	gHUDDraw.FinishDraw();

	// Draw history last
	if(!m_pHistory->Draw())
	{
		gHUDDraw.ManageErrorMessage();
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::AddSubtitle( const Char *szname, Float holdtime )
{
	CString szsubtitlename(szname);
	szsubtitlename.tolower();

	subtitle_t *psubtitle = nullptr;
	for(Uint32 i = 0; i < m_subtitlesArray.size(); i++)
	{
		if(!qstrcmp(m_subtitlesArray[i].szname, szsubtitlename))
		{
			psubtitle = &m_subtitlesArray[i];
			break;
		}
	}

	if(!psubtitle)
		return false;

	AddActiveSubtitle(psubtitle, holdtime, SUBTITLE_TAB_TIMEOUT);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::RemoveSubtitle( const Char *szname )
{
	CString szsubtitlename(szname);
	szsubtitlename.tolower();

	if(!m_activeSubtitlesList.empty())
	{
		m_activeSubtitlesList.begin();
		while(!m_activeSubtitlesList.end())
		{
			active_subtitle_t& subtitle = m_activeSubtitlesList.get();
			if(!qstrcmp(subtitle.psubtitle->szname, szname))
			{
				m_activeSubtitlesList.remove(m_activeSubtitlesList.get_link());
				return;
			}

			m_activeSubtitlesList.next();
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::AddActiveSubtitle( subtitle_t* psubtitle, Float holdtime, Float timeout )
{
	if(!m_activeSubtitlesList.empty())
	{
		m_activeSubtitlesList.begin();
		while(!m_activeSubtitlesList.end())
		{
			const active_subtitle_t& subtitle = m_activeSubtitlesList.get();
			if(subtitle.psubtitle == psubtitle)
				return;

			m_activeSubtitlesList.next();
		}
	}

	active_subtitle_t newsubtitle;

	Float fltime = cl_engfuncs.pfnGetClientTime();
	newsubtitle.holdtime = fltime + holdtime;
	newsubtitle.timeout = newsubtitle.holdtime + timeout;
	newsubtitle.spawntime = fltime;
	newsubtitle.psubtitle = psubtitle;

	// Add system into pointer array
	m_activeSubtitlesList.add(newsubtitle);
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawSubtitles( void ) 
{
	if(m_pCvarDrawSubtitles->GetValue() < 1)
		return true;

	if(m_activeSubtitlesList.empty())
		return true;

	// vertical offset
	Float baseycoord = gHUDDraw.ScaleY(BASE_RESOLUTION_Y) - gHUDDraw.ScaleY(SUBTITLE_GAP) * 4;
	Float fltime = cl_engfuncs.pfnGetClientTime();

	m_activeSubtitlesList.begin();
	while(!m_activeSubtitlesList.end())
	{
		active_subtitle_t& subtitle = m_activeSubtitlesList.get();

		if(subtitle.spawntime + subtitle.psubtitle->delay > fltime)
		{
			m_activeSubtitlesList.next();
			continue;
		}

		if(!subtitle.timeout || subtitle.timeout <= fltime)
		{
			m_activeSubtitlesList.remove(m_activeSubtitlesList.get_link());
			m_activeSubtitlesList.next();
			continue;
		}

		Float flalhpa = 1.0;
		if((subtitle.timeout-SUBTITLE_TAB_FADETIME) <= fltime)
		{
			Float begintime = subtitle.timeout-SUBTITLE_TAB_FADETIME;
			flalhpa = 1.0-((fltime-begintime)/SUBTITLE_TAB_FADETIME);
		}
		else if((subtitle.spawntime + subtitle.psubtitle->delay + SUBTITLE_TAB_FADEINTIME) >= fltime)
		{
			Float begintime = subtitle.spawntime + subtitle.psubtitle->delay;
			flalhpa = (fltime - begintime) / SUBTITLE_TAB_FADEINTIME;
		}

		Float xsize = gHUDDraw.ScaleX(SUBTITLE_TAB_SIZE_X);
		Float yrefsize = gHUDDraw.ScaleY(SUBTITLE_TAB_SIZE_Y);

		Float ysize;
		Float ysizefontfrac = yrefsize / (Float)m_pSubtitleSet->fontsize;
		if((ysizefontfrac - SDL_floor(ysizefontfrac)) >= 0.5)
			ysize = SDL_ceil(ysizefontfrac) * m_pSubtitleSet->fontsize;
		else
			ysize = SDL_floor(ysizefontfrac) * m_pSubtitleSet->fontsize;

		ysize += SUBTITLE_INSET * 2;

		Float xcenter = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X/2);
		Float xcoord = xcenter - xsize/2;

		Int32 offset = 0;
		cl_renderfuncs.pfnSetStringRectangle(0, 0, xsize, (ysize*flalhpa), SUBTITLE_INSET, SUBTITLE_INSET);
		Float height = cl_renderfuncs.pfnEstimateStringHeight(m_pSubtitleSet, subtitle.psubtitle->subtitle.c_str(), m_pSubtitleSet->fontsize);

		if((height+gHUDDraw.ScaleY(SUBTITLE_GAP)) < ysize)
		{
			ysize = (height+gHUDDraw.ScaleY(SUBTITLE_GAP));
		}
		else if(height > (ysize-gHUDDraw.ScaleY(SUBTITLE_GAP)))
		{
			Float diff = height - (ysize-gHUDDraw.ScaleY(SUBTITLE_GAP));
			Int32 numlines = diff/(Float)m_pSubtitleSet->fontsize;

			Float flfrac = fltime-(subtitle.spawntime+subtitle.psubtitle->delay);
			flfrac = flfrac/(subtitle.holdtime-(subtitle.spawntime+subtitle.psubtitle->delay));
			offset += (numlines)*flfrac;

			if(offset > numlines)
				offset = numlines;
		}

		// Set the final one
		Float ycoord = baseycoord - (ysize * flalhpa);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gHUDDraw.SetOrigin(xcoord, ycoord);
		gHUDDraw.SetSize(xsize, ysize * flalhpa);
		gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA*flalhpa);
		if(!gHUDDraw.DrawBody())
			return false;

		if(subtitle.holdtime && subtitle.holdtime > fltime)
		{
			gHUDDraw.FinishDraw();

			color32_t textcolor(subtitle.psubtitle->r, subtitle.psubtitle->g, subtitle.psubtitle->b, 255 * flalhpa);
			if(!cl_renderfuncs.pfnDrawStringBox(0, 0, xsize, (ysize*flalhpa), SUBTITLE_INSET, SUBTITLE_INSET, false, textcolor, xcoord, ycoord, subtitle.psubtitle->subtitle.c_str(), m_pSubtitleSet, offset, m_pSubtitleSet->fontsize, 0))
				return false;

			if(!gHUDDraw.SetupDraw())
				return false;
		}

		// Go upwards
		baseycoord -= (ysize * flalhpa) + gHUDDraw.ScaleY(SUBTITLE_GAP) * flalhpa;

		// Continue to next
		m_activeSubtitlesList.next();
	}

	cl_renderfuncs.pfnSetStringRectangle(0, 0, 0, 0, 0, 0);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawCrosshair( void ) 
{
	if(!m_pWeapon)
		return true;

	if(m_pWeapon->weaponid == WEAPON_KNIFE || m_pWeapon->weaponid == WEAPON_HANDGRENADE)
		return true;

	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	
	Float coordX, coordY;
	if(!m_currentAutoAimVector.IsZero())
	{
		Vector angles = cl_engfuncs.pfnGetViewAngles();
		angles += m_currentAutoAimVector;

		Vector forward;
		Math::AngleVectors(angles, &forward);
		Vector offsetPos = g_viewOrigin + forward;

		Uint32 screenWidth, screenHeight;
		cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

		Float vOrigin [] = { offsetPos[0], offsetPos[1], offsetPos[2], 1.0 };

		// Multiply with modelview
		Float viewPos[4];
		Math::MatMult4(m_modelviewMatrix.GetMatrix(), vOrigin, viewPos);

		// Multiply with projection
		Float screenCoords[4];
		Math::MatMult4(m_projectionMatrix.GetMatrix(), viewPos, screenCoords);

		// Calculate uniform values
		coordX = (screenCoords[0]/screenCoords[3])*0.5 + 0.5;
		coordX *= screenWidth;
		if(coordX > screenWidth)
			coordX = screenWidth;
		else if(coordX < 0)
			coordX = 0;

		coordY = (screenCoords[1]/screenCoords[3])*0.5 + 0.5;
		coordY *= screenHeight;
		coordY = (screenHeight - coordY);

		if(coordY > screenHeight)
			coordY = screenHeight;
		else if(coordY < 0)
			coordY = 0;
	}
	else
	{
		coordX = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X/2);
		coordY = gHUDDraw.ScaleY(BASE_RESOLUTION_Y/2);
	}

	Vector vspread = Weapon_GetConeSize(m_pWeapon->cone,
		gDefaultView.GetLeanOffset(),
		pplayer->curstate.velocity,
		pplayer->curstate.punchangles);

	Math::VectorScale(vspread, 500, vspread);

	Float mod = (1/(tan(M_PI/180*(90/2))));
	Int32 dir = ((vspread.Length() * coordX) / 500) * mod;

	Float in_barsize = gHUDDraw.ScaleX(9);
	Float out_barsize = gHUDDraw.ScaleX(6);
	
	Vector crosshairColor;
	if(m_isOnTarget)
		crosshairColor = HUD_COLOR_RED;
	else
		crosshairColor = HUD_COLOR_WHITE;

	// Inner cross
	gHUDDraw.SetColor(crosshairColor, 255);

	gHUDDraw.SetSize(in_barsize, 1);
	gHUDDraw.SetOrigin(coordX+1-in_barsize/2, coordY);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	gHUDDraw.SetSize(1, in_barsize);
	gHUDDraw.SetOrigin(coordX, coordY-in_barsize/2);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);

	// Outer bars
	gHUDDraw.SetSize(1, out_barsize);
	gHUDDraw.SetOrigin(coordX, coordY-out_barsize*2-dir);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	gHUDDraw.SetOrigin(coordX, coordY+out_barsize+dir);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	gHUDDraw.SetSize(out_barsize, 1);
	gHUDDraw.SetOrigin(coordX-out_barsize*2-dir, coordY);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	gHUDDraw.SetOrigin(coordX+out_barsize+dir, coordY);
	if(!gHUDDraw.DrawQuad(nullptr))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawWeaponList( void ) 
{
	if(!m_pActiveSelection && !m_isDrySelection)
		return true;

	Int32 iActiveSlot;
	if ( m_isDrySelection )
		iActiveSlot = -1;	// current slot has no weapons
	else 
		iActiveSlot = m_pActiveSelection->slot;

	Float x = gHUDDraw.ScaleX(50);
	Float y = gHUDDraw.ScaleY(50);

	Double fltime = cl_engfuncs.pfnGetClientTime();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(Int32 i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		weapon_t *pweapon = m_weaponInfo.GetSlotFirstWeapon(i);

		// Draw the slot label
		gHUDDraw.SetOrigin(x, y);
		gHUDDraw.SetSize(gHUDDraw.ScaleX(LIST_LABEL_T_X), gHUDDraw.ScaleY(LIST_LABEL_T_Y));
		gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		y += gHUDDraw.ScaleY(LIST_LABEL_T_Y);
		gHUDDraw.SetOrigin(x, y);
		gHUDDraw.SetSize(gHUDDraw.ScaleX(LIST_LABEL_S_X), gHUDDraw.ScaleY(LIST_LABEL_S_Y));
		gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		if(!pweapon)
		{
			y += gHUDDraw.ScaleX(LIST_LABEL_S_Y + CHUDDraw::HUD_EDGE_SIZE);
			continue;
		}

		x += gHUDDraw.ScaleX(LIST_LABEL_S_X);

		Float cur_x = x;
		for(Int32 j = 0; j < MAX_SLOT_POSITIONS; j++)
		{
			pweapon = m_weaponInfo.GetWeaponFromSlot( i, j );

			if ( !pweapon || !pweapon->weaponid )
				continue;

			// Draw body background
			gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), y+gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE));
			gHUDDraw.SetSize(gHUDDraw.ScaleX(LIST_WEAPONTAB_X-CHUDDraw::HUD_EDGE_SIZE*2), gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-CHUDDraw::HUD_EDGE_SIZE*2));
			gHUDDraw.SetColor(HUD_COLOR_BLACK[0], HUD_COLOR_BLACK[1], HUD_COLOR_BLACK[2], HUD_DEFAULT_ALPHA);
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			// Draw weapon image
			if(m_weaponInfo.WeaponHasAmmo(pweapon))
			{
				if (m_pActiveSelection == pweapon)
				{
					Float flalpha = SDL_fabs(SDL_sin(fltime*4));
					gHUDDraw.SetColor(HUD_COLOR_BLUE*(0.25+flalpha*0.75), 255);
				}
				else
				{
					gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);
				}
			}
			else
			{
				Float flalpha = SDL_fabs(SDL_sin(fltime*4));
				gHUDDraw.SetColor(HUD_COLOR_RED*(0.25+flalpha*0.75), 255);
			}

			gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), y+gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE));
			gHUDDraw.SetSize(gHUDDraw.ScaleX(LIST_WEAPONTAB_X-CHUDDraw::HUD_EDGE_SIZE*2), gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-CHUDDraw::HUD_EDGE_SIZE*2));
			glBlendFunc(GL_ONE, GL_ONE);

			weaponid_t weaponId = (weaponid_t)pweapon->weaponid;
			if(!gHUDDraw.DrawQuad(GetWeaponIconById(weaponId)))
				return false;

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (m_pActiveSelection == pweapon)
				gHUDDraw.SetColor(HUD_COLOR_BLUE, 255);
			else
				gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);

			// Right border
			gHUDDraw.SetOrigin(cur_x, y);
			gHUDDraw.SetSize(gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-CHUDDraw::HUD_EDGE_SIZE));
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			// Top border
			gHUDDraw.SetSize(gHUDDraw.ScaleX(LIST_WEAPONTAB_X-CHUDDraw::HUD_EDGE_SIZE), gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE));
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			// Left border
			gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(LIST_WEAPONTAB_X-CHUDDraw::HUD_EDGE_SIZE), y);
			gHUDDraw.SetSize(gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-CHUDDraw::HUD_EDGE_SIZE));
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			// Bottom border
			gHUDDraw.SetOrigin(cur_x, y+gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-CHUDDraw::HUD_EDGE_SIZE));
			gHUDDraw.SetSize(gHUDDraw.ScaleX(LIST_WEAPONTAB_X), gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE));
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
			
			if(pweapon->ammotype != -1 && pweapon->maxammo != -1)
			{
				// Draw ammo bar
				Float bar = (Float)m_weaponInfo.GetAmmoCount(pweapon->ammotype)/(Float)pweapon->maxammo;
				Float bar_fraction = 1/gHUDDraw.ScaleX(AMMOBAR_SIZE_X);
				Float frac = bar-(floor(bar/bar_fraction)/gHUDDraw.ScaleX(AMMOBAR_SIZE_X));
				bar -= frac;

				gHUDDraw.SetColor(HUD_COLOR_GRAY, HUD_DEFAULT_ALPHA);
				gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), y+gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-AMMOBAR_SIZE_Y));
				gHUDDraw.SetSize(gHUDDraw.ScaleX(AMMOBAR_SIZE_X-CHUDDraw::HUD_EDGE_SIZE), gHUDDraw.ScaleY(AMMOBAR_SIZE_Y-CHUDDraw::HUD_EDGE_SIZE));
				if(!gHUDDraw.DrawQuad(nullptr))
					return false;

				if(bar > 0)
				{
					gHUDDraw.SetColor(HUD_COLOR_WHITE, HUD_DEFAULT_ALPHA);
					gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), y+gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-AMMOBAR_SIZE_Y));
					gHUDDraw.SetSize(gHUDDraw.ScaleX(AMMOBAR_SIZE_X-CHUDDraw::HUD_EDGE_SIZE*2)*bar, gHUDDraw.ScaleY(AMMOBAR_SIZE_Y-CHUDDraw::HUD_EDGE_SIZE));
					if(!gHUDDraw.DrawQuad(nullptr))
						return false;
				}

				// Draw ammo bar border
				if (m_pActiveSelection == pweapon)
					gHUDDraw.SetColor(HUD_COLOR_BLUE, 255);
				else
					gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);

				gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), y+gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-AMMOBAR_SIZE_Y-CHUDDraw::HUD_EDGE_SIZE));
				gHUDDraw.SetSize(gHUDDraw.ScaleX(AMMOBAR_SIZE_X-CHUDDraw::HUD_EDGE_SIZE), gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE));
				if(!gHUDDraw.DrawQuad(nullptr))
					return false;

				gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(AMMOBAR_SIZE_X-CHUDDraw::HUD_EDGE_SIZE), y+gHUDDraw.ScaleY(LIST_WEAPONTAB_Y-AMMOBAR_SIZE_Y-CHUDDraw::HUD_EDGE_SIZE));
				gHUDDraw.SetSize(gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), gHUDDraw.ScaleY(AMMOBAR_SIZE_Y));
				if(!gHUDDraw.DrawQuad(nullptr))
					return false;
			}

			cur_x += gHUDDraw.ScaleY(LIST_WEAPONTAB_X);
		}

		y += gHUDDraw.ScaleX(LIST_WEAPONTAB_Y + CHUDDraw::HUD_EDGE_SIZE);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawTab_Bar( Float x, Float y, Char *sztext, en_texture_t *picon, Float bar, Int32 width, Float alpha, Float *ox, Float *oy, bool reverseColor ) 
{
	Float cur_x = x;
	Float cur_y = y;
	
	Double fltime = cl_engfuncs.pfnGetClientTime();

	if(bar > 100)
		bar = 100;

	// Main body
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gHUDDraw.SetOrigin(cur_x, cur_y);
	gHUDDraw.SetSize(gHUDDraw.ScaleX(width), gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y));
	gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA * alpha);
	if(!gHUDDraw.DrawBody())
		return false;

	// Draw icon
	bool shouldFlash = false;
	if(!reverseColor)
		shouldFlash = (bar > 25) ? false : true;
	else
		shouldFlash = (bar < 75) ? false : true;

	if(!shouldFlash)
	{
		gHUDDraw.SetColor(HUD_COLOR_WHITE * alpha, 255);
	}
	else
	{
		Float flalpha = SDL_fabs(SDL_sin(fltime*4));
		gHUDDraw.SetColor(HUD_COLOR_RED*(0.25+flalpha*0.75) * alpha, 255);
	}

	Float edge = 0;
	Float flsize = 0;
	if(picon)
	{
		Float widthScale = picon->height > picon->width ? ((Float)picon->width/(Float)picon->height) : 1.0;
		Float heightScale = picon->width > picon->height ? ((Float)picon->height/(Float)picon->width) : 1.0;

		edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-gHUDDraw.ScaleY(ICON_SIZE)*heightScale;
		edge = edge/2.0f;

		flsize = gHUDDraw.ScaleX(ICON_SIZE);

		glBlendFunc(GL_ONE, GL_ONE);
		gHUDDraw.SetOrigin(cur_x+edge, cur_y+edge);
		gHUDDraw.SetSize(flsize*widthScale, gHUDDraw.ScaleY(ICON_SIZE)*heightScale);
	
		if(!gHUDDraw.DrawQuad(picon))
			return false;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if(sztext)
	{
		Char *pscan = sztext;
		while(*pscan != '\0')
		{
			flsize += m_pFontSet->glyphs[*pscan].advancex;
			pscan++;
		}
		edge = gHUDDraw.ScaleX(4);
	}

	Float bar_edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-gHUDDraw.ScaleY(PERCENTAGE_BAR_HEIGHT);
	bar_edge = bar_edge/2.0f;

	Float bar_x = cur_x+flsize+edge*2;
	if(!picon)
		bar_x += bar_edge; 

	Float bar_size = (cur_x+gHUDDraw.ScaleX(width))-bar_x-bar_edge;

	// Draw empty
	if(bar < 100)
	{
		gHUDDraw.SetOrigin(bar_x, cur_y+bar_edge);
		gHUDDraw.SetSize(bar_size, gHUDDraw.ScaleY(PERCENTAGE_BAR_HEIGHT));
		gHUDDraw.SetColor(HUD_COLOR_GRAY, 255 * alpha);
		if(!gHUDDraw.DrawBody())
			return false;
	}

	// Draw full
	if(bar > 0)
	{
		if(shouldFlash)
		{
			Vector vColor;
			Float flsin = SDL_fabs(SDL_sin(fltime*4));
			Math::VectorScale(HUD_COLOR_RED, flsin, vColor);
			Math::VectorMA(vColor, 1.0-flsin, HUD_COLOR_WHITE, vColor);
			gHUDDraw.SetColor(vColor, 255 * alpha);
		}
		else
		{
			gHUDDraw.SetColor(HUD_COLOR_WHITE, 255 * alpha);
		}	

		Float size = bar_size*(bar/100);
		gHUDDraw.SetOrigin(bar_x, cur_y+bar_edge);
		gHUDDraw.SetSize(size, gHUDDraw.ScaleY(PERCENTAGE_BAR_HEIGHT));
		if(!gHUDDraw.DrawBody())
			return false;
	}

	// Draw text if we have any
	if(sztext)
	{
		Float font_edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-m_pFontSet->fontsize;
		font_edge = font_edge/2.0f;		

		gHUDDraw.SetOrigin(cur_x+gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE), cur_y+gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE));
		gHUDDraw.SetSize(flsize+font_edge, gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE)*2);
		gHUDDraw.SetColor(HUD_COLOR_WHITE, 255 * alpha);
		if(!gHUDDraw.DrawBody())
			return false;

		if(!gHUDDraw.DrawText(HUD_COLOR_BLACK, 255 * alpha, cur_x+font_edge, cur_y+font_edge*0.5+m_pFontSet->fontsize, sztext, m_pFontSet))
			return false;
	}

	if(ox) *ox = gHUDDraw.ScaleX(width);
	if(oy) *oy = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawHealthkitTab( Float x, Float y ) 
{
	// Main body
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gHUDDraw.SetOrigin(x, y);
	gHUDDraw.SetSize(gHUDDraw.ScaleX(TAB_HEALTHKIT_SIZE_X), gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y));
	gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
	if(!gHUDDraw.DrawBody())
		return false;

	// Draw the Icon
	Float edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-gHUDDraw.ScaleY(ICON_SIZE);
	edge = edge/2.0f;

	glBlendFunc(GL_ONE, GL_ONE);
	gHUDDraw.SetOrigin(x+edge, y+edge);
	gHUDDraw.SetSize(gHUDDraw.ScaleX(ICON_SIZE), gHUDDraw.ScaleY(ICON_SIZE));

	// Draw icon
	if(m_numMedkits > 0)
	{
		gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);
	}
	else
	{
		gHUDDraw.SetColor(HUD_COLOR_RED, 255);
	}

	if(!gHUDDraw.DrawQuad(m_pMedkitIcon))
		return false;

	CString digit;
	digit << m_numMedkits;

	Uint32 width = 0;
	Uint32 height = 0;
	Int32 yMin = 0;
	cl_renderfuncs.pfnGetStringSize(m_pCounterFont, digit.c_str(), &width, &height, &yMin);

	Float digitSpace = gHUDDraw.ScaleX(TAB_HEALTHKIT_SIZE_X) - (edge + gHUDDraw.ScaleX(ICON_SIZE));
	Float xCoord = x + edge + gHUDDraw.ScaleX(ICON_SIZE) + digitSpace * 0.5 - width * 0.5;
	Float yCoord = y + gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y) * 0.5f + (Float)height * 0.5;

	if(!gHUDDraw.DrawText(m_numMedkits > 0 ? HUD_COLOR_WHITE : HUD_COLOR_RED, 255, xCoord, yCoord, digit.c_str(), m_pCounterFont))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawTacticalTab( Float x, Float y ) 
{
	Float cur_x = x;
	Float cur_y = y;

	Double fltime = cl_engfuncs.pfnGetClientTime();

	// Main body
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gHUDDraw.SetOrigin(cur_x, cur_y);
	gHUDDraw.SetSize(gHUDDraw.ScaleX(TAB_TACTICAL_SIZE_X), gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y));
	gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
	if(!gHUDDraw.DrawBody())
		return false;

	Float alpha = 1.0;
	if(!m_isTacticalOn)
		alpha *= 0.5;

	// Draw icon
	if(m_tacticalAmount > 25 || !m_isTacticalOn)
	{
		gHUDDraw.SetColor(HUD_COLOR_WHITE*alpha, 255);
	}
	else
	{
		Float flalpha = SDL_fabs(SDL_sin(fltime*4));
		gHUDDraw.SetColor(HUD_COLOR_RED*(0.25+flalpha*0.75), 255);
	}

	Float edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-gHUDDraw.ScaleY(TACTICAL_ICON_Y);
	edge = edge/2.0f;

	Float flsize = gHUDDraw.ScaleX(TACTICAL_ICON_X);

	glBlendFunc(GL_ONE, GL_ONE);

	gHUDDraw.SetOrigin(cur_x+edge, cur_y+edge);
	gHUDDraw.SetSize(flsize, gHUDDraw.ScaleY(TACTICAL_ICON_Y));
	if(!gHUDDraw.DrawQuad(m_pTacticalIcon))
		return false;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Float bar_edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y)-gHUDDraw.ScaleY(PERCENTAGE_BAR_HEIGHT);
	bar_edge = bar_edge/2.0f;

	Float bar_x = cur_x+flsize+edge*2;
	Float bar_size = (cur_x+gHUDDraw.ScaleX(TAB_TACTICAL_SIZE_X))-bar_x-gHUDDraw.ScaleX(8);

	// Draw empty
	if(m_tacticalAmount < 100)
	{
		gHUDDraw.SetOrigin(bar_x, cur_y+bar_edge);
		gHUDDraw.SetSize(bar_size, gHUDDraw.ScaleY(PERCENTAGE_BAR_HEIGHT));
		gHUDDraw.SetColor(HUD_COLOR_GRAY, 255);
		if(!gHUDDraw.DrawBody())
			return false;
	}

	// Draw full
	if(m_tacticalAmount > 0)
	{
		if(m_tacticalAmount <= 25)
		{
			Vector vColor;
			Float flsin = SDL_fabs(SDL_sin(fltime*4));
			Math::VectorScale(HUD_COLOR_RED, flsin, vColor);
			Math::VectorMA(vColor, 1.0-flsin, HUD_COLOR_WHITE, vColor);
			gHUDDraw.SetColor(vColor, 255);
		}
		else
		{
			gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);
		}	

		Float size = bar_size*(m_tacticalAmount/100);
		gHUDDraw.SetOrigin(bar_x, cur_y+bar_edge);
		gHUDDraw.SetSize(size, PERCENTAGE_BAR_HEIGHT);
		if(!gHUDDraw.DrawBody())
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawWeaponTab( void ) 
{
	CString digits;
	CString name, ammo;
	en_texture_t *picon;

	if(!m_pWeapon)
		return true;

	Double fltime = cl_engfuncs.pfnGetClientTime();

	bool dual = false; // No dual weapons in public release
	GetWeaponInfo(m_pWeapon->weaponid, dual, picon, name, ammo);

	Float x = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X) - gHUDDraw.ScaleX(50) - gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X);
	Float y = BASE_RESOLUTION_Y-50-TAB_GENERIC_SIZE_Y*2-CHUDDraw::HUD_EDGE_SIZE*2-2;
	y = gHUDDraw.ScaleY(y);

	Float ammoInnerTabSize = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y-CHUDDraw::HUD_EDGE_SIZE*2);

	Float weaponInfoTabXCoord = 0, weaponInfoTabYCoord = 0;
	Float weaponInfoTabWidth = 0, weaponInfoTabHeight = 0;

	Float ammoInfoTabXCoord = 0, ammoInfoTabYCoord = 0;
	Float ammoInfoTabWidth = 0, ammoInfoTabHeight = 0;

	Float counter1XCoord = 0, counter1YCoord = 0;
	Float counter1Width = 0, counter1Height = 0;

	Float counter2XCoord = 0, counter2YCoord = 0;
	Float counter2Width = 0, counter2Height = 0;

	Float ammoTabXCoord = 0, ammoTabYCoord = 0;
	Float ammoTabWidth = 0, ammoTabHeight = 0;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(m_pWeapon->ammotype != -1)
	{
		weaponInfoTabXCoord = x;
		weaponInfoTabYCoord = y;

		if(m_pWeapon->clip >= 0)
		{
			weaponInfoTabWidth = gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X*0.55-CHUDDraw::HUD_EDGE_SIZE*2);
			weaponInfoTabHeight = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y);

			gHUDDraw.SetOrigin(weaponInfoTabXCoord, weaponInfoTabYCoord);
			gHUDDraw.SetSize(weaponInfoTabWidth, weaponInfoTabHeight);
			gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
			if(!gHUDDraw.DrawBody())
				return false;

			// Draw ammo icon tab
			ammoInfoTabXCoord = x+gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X*0.55+CHUDDraw::HUD_EDGE_SIZE*2);
			ammoInfoTabYCoord = y;

			ammoInfoTabWidth = gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X*0.45-CHUDDraw::HUD_EDGE_SIZE*2);
			ammoInfoTabHeight = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y);

			gHUDDraw.SetOrigin(ammoInfoTabXCoord, ammoInfoTabYCoord);
			gHUDDraw.SetSize(ammoInfoTabWidth, ammoInfoTabHeight);
			gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
			if(!gHUDDraw.DrawBody())
				return false;
		}
		else
		{
			weaponInfoTabWidth = gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X);
			weaponInfoTabHeight = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y);

			// Draw weapon name tab only
			gHUDDraw.SetOrigin(weaponInfoTabXCoord, weaponInfoTabYCoord);
			gHUDDraw.SetSize(weaponInfoTabWidth, weaponInfoTabHeight);
			gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
			if(!gHUDDraw.DrawBody())
				return false;
		}

		// Draw ammo counters tab background
		y += gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y+CHUDDraw::HUD_EDGE_SIZE*2+2);

		ammoTabXCoord = x;
		ammoTabYCoord = y;
		ammoTabWidth = gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X);
		ammoTabHeight = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y);

		gHUDDraw.SetOrigin(ammoTabXCoord, ammoTabYCoord);
		gHUDDraw.SetSize(ammoTabWidth, ammoTabHeight);
		gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
		if(!gHUDDraw.DrawBody())
			return false;

		if(m_pWeapon->clip >= 0)
		{
			Int32 clip = dual ? m_pWeapon->dualclipleft : m_pWeapon->clip;
			Int32 clippercentage = ((float)clip / (float)m_pWeapon->maxclip) * 100;
			if(clippercentage <= 25)
			{
				Float flalpha = SDL_fabs(SDL_sin(fltime*4));
				gHUDDraw.SetColor(HUD_COLOR_RED, 128+flalpha*128);
			}
			else
			{
				gHUDDraw.SetColor(HUD_COLOR_WHITE, HUD_DEFAULT_ALPHA);
			}

			// Draw white clip tab//right clip
			counter1XCoord = x + gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE);
			counter1YCoord = y + gHUDDraw.ScaleY(CHUDDraw::HUD_EDGE_SIZE);
			counter1Width = gHUDDraw.ScaleX(LABEL_CLIP_SIZE_X);
			counter1Height = ammoInnerTabSize;

			gHUDDraw.SetOrigin(counter1XCoord, counter1YCoord);
			gHUDDraw.SetSize(counter1Width, counter1Height);
			if(!gHUDDraw.DrawBody())
				return false;

			ammoTabXCoord = counter1XCoord + counter1Width;
			ammoTabWidth -= counter1Width;
		}

		if(dual)//right clip 
		{
			Int32 clippercentage = ((float)m_pWeapon->dualclipright / (float)m_pWeapon->maxclip) * 100;
			if(clippercentage <= 25)
			{
				Float flalpha = SDL_fabs(SDL_sin(fltime*4));
				gHUDDraw.SetColor(HUD_COLOR_RED, 128+flalpha*128);
			}
			else
			{
				gHUDDraw.SetColor(HUD_COLOR_WHITE, HUD_DEFAULT_ALPHA);
			}

			counter2XCoord = counter1XCoord + gHUDDraw.ScaleX(LABEL_CLIP_SIZE_X+CHUDDraw::HUD_EDGE_SIZE*2+2);
			counter2YCoord = counter1YCoord;

			counter2Width = counter1Width;
			counter2Height = counter1Height;

			gHUDDraw.SetOrigin(counter2XCoord, counter2YCoord);
			if(!gHUDDraw.DrawBody())
				return false;

			ammoTabXCoord = counter2XCoord + counter2Width;
			ammoTabWidth -= counter2Width + gHUDDraw.ScaleX(CHUDDraw::HUD_EDGE_SIZE*2+2);
		}

		if(m_pWeapon->clip >= 0)
		{
			// Draw the ammo icon
			x = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X) - gHUDDraw.ScaleX(50) - gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X);
			y = BASE_RESOLUTION_Y-50-TAB_GENERIC_SIZE_Y*2-CHUDDraw::HUD_EDGE_SIZE*2-2;
			y = gHUDDraw.ScaleY(y);

			Float edge = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y-AMMOICON_SIZE_Y);
			edge = edge/2;

			Float ammoIconXCoord = x + gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X) - edge - gHUDDraw.ScaleX(AMMOICON_SIZE_X);
			Float ammoIconYCoord = y+edge;

			Float ammoIconWidth = gHUDDraw.ScaleX(AMMOICON_SIZE_X);
			Float ammoIconHeight = gHUDDraw.ScaleY(AMMOICON_SIZE_Y);
			ammoInfoTabWidth -= ammoIconWidth + edge;

			gHUDDraw.SetOrigin(ammoIconXCoord, ammoIconYCoord);
			gHUDDraw.SetSize(ammoIconWidth, ammoIconHeight);

			gHUDDraw.SetColor(HUD_COLOR_WHITE, 255);

			glBlendFunc(GL_ONE, GL_ONE);
			if(!gHUDDraw.DrawQuad(picon))
				return false;
		}
	}
	else
	{
		weaponInfoTabXCoord = x;
		weaponInfoTabYCoord = y;

		weaponInfoTabWidth = gHUDDraw.ScaleX(TAB_WEAPON_SIZE_X);
		weaponInfoTabHeight = gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y);

		// Draw weapon name tab only
		gHUDDraw.SetOrigin(weaponInfoTabXCoord, weaponInfoTabYCoord);
		gHUDDraw.SetSize(weaponInfoTabWidth, weaponInfoTabHeight);
		gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);
		if(!gHUDDraw.DrawBody())
			return false;
	}

	// Draw the weapon's name
	Uint32 width = 0;
	Uint32 height = 0;
	cl_renderfuncs.pfnGetStringSize(m_pFontSet, name.c_str(), &width, &height, nullptr);

	x = weaponInfoTabXCoord + weaponInfoTabWidth * 0.5f - width * 0.5f;
	y = weaponInfoTabYCoord + weaponInfoTabHeight * 0.5f + height * 0.5f;

	if(!gHUDDraw.DrawText(HUD_COLOR_WHITE, 255, x, y, name.c_str(), m_pFontSet))
		return false;

	if(m_pWeapon->ammotype != -1)
	{
		// Draw the caliber name
		cl_renderfuncs.pfnGetStringSize(m_pFontSet, ammo.c_str(), &width, &height, nullptr);

		x = ammoInfoTabXCoord + ammoInfoTabWidth * 0.5f - width * 0.5f;
		y = ammoInfoTabYCoord + ammoInfoTabHeight * 0.5f + height * 0.5f;

		if(!gHUDDraw.DrawText(HUD_COLOR_WHITE, 255, x, y, ammo.c_str(), m_pFontSet))
			return false;

		if(m_pWeapon->clip >= 0)
		{
			// Draw the clip counter
			digits.clear();
			if(dual)
				digits << m_pWeapon->dualclipleft;
			else
				digits << m_pWeapon->clip;

			cl_renderfuncs.pfnGetStringSize(m_pCounterFont, digits.c_str(), &width, &height, nullptr);

			x = counter1XCoord + counter1Width * 0.5f - width * 0.5f;
			y = counter1YCoord + counter1Height * 0.5f + height * 0.5f;

			if(!gHUDDraw.DrawText(HUD_COLOR_BLACK, 255, x, y, digits.c_str(), m_pCounterFont))
				return false;

			if(dual) // right clip
			{
				digits.clear();
				digits << m_pWeapon->dualclipright;
				cl_renderfuncs.pfnGetStringSize(m_pCounterFont, digits.c_str(), &width, &height, nullptr);

				x = counter2XCoord + counter2Width * 0.5f - width * 0.5f;
				y = counter2YCoord + counter2Height * 0.5f + height * 0.5f;

				if(!gHUDDraw.DrawText(HUD_COLOR_BLACK, 255, x, y, digits.c_str(), m_pCounterFont))
					return false;
			}
		}

		// Draw ammo counter
		digits.clear();
		digits << (Int32)m_weaponInfo.GetAmmoCount(m_pWeapon->ammotype);
		cl_renderfuncs.pfnGetStringSize(m_pCounterFont, digits.c_str(), &width, &height, nullptr);

		x = ammoTabXCoord + ammoTabWidth * 0.5f - width * 0.5f;
		y = ammoTabYCoord + ammoTabHeight * 0.5f + height * 0.5f;

		if(!gHUDDraw.DrawText(HUD_COLOR_WHITE, 255, x, y, digits.c_str(), m_pCounterFont))
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::GetWeaponInfo( Int32 id, bool dual, en_texture_t *&picon, CString& name, CString& cartridge ) const
{
	if(!m_pActiveWeaponInfo)
	{
		name = "NA";
		cartridge.clear();
		picon = cl_renderfuncs.pfnGetDummyTexture();
	}
	else
	{
		if(dual)
		{
			if(!m_pActiveWeaponInfo->description_dual.empty())
				name = m_pActiveWeaponInfo->description_dual;
			else
				name = "NA";
		}
		else
		{
			if(!m_pActiveWeaponInfo->description.empty())
				name = m_pActiveWeaponInfo->description;
			else
				name = "NA";
		}

		cartridge = m_pActiveWeaponInfo->description_ammo;
		picon = m_pActiveWeaponInfo->pammo_icon;
	}
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetStamina( Float stamina )
{
	m_stamina = stamina;
	m_staminaFadeTime = cl_engfuncs.pfnGetClientTime() + STAMINA_FADE_TIME;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetHealth( Float health )
{
	m_health = health;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetKevlar( Float kevlar )
{
	m_kevlar = kevlar;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetMovementNoise( Float noise )
{
	m_movementNoise = noise;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetNPCAwareness( Float awareness )
{
	m_npcAwareness = awareness;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetNewObjective( bool newObjective )
{
	m_newObjective = newObjective;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetHealthkitNumber( Uint32 numhealthkits )
{
	m_numMedkits = numhealthkits;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetHealProgress( Float progress )
{
	m_healProgress = progress;
	if(m_healProgress > 1.0)
		m_healProgress = 1.0;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetUsableObjectMinsMaxs( const Vector& mins, const Vector& maxs, usableobject_type_t type )
{
	m_usableObjectMins = mins;
	m_usableObjectMaxs = maxs;
	m_usableObjectType = type;
}

//=============================================
// @brief Sets countdown timer
//
//=============================================
void CGameHUD::SetCountdownTime( Double endTime, const Char* pstrTitle )
{
	if(!m_countdownTimerTitle.empty())
		m_countdownTimerTitle.clear();

	if(!endTime)
	{
		// Clear timer
		m_countdownTimerTime = 0;
	}
	else
	{
		if(pstrTitle)
			m_countdownTimerTitle = pstrTitle;

		m_countdownTimerTime = endTime;
	}
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::LoadHUDScript( void )
{
	const Char* pfile = reinterpret_cast<const Char*>(cl_filefuncs.pfnLoadFile(HUD_DESCRIPTION_SCRIPT_PATH, nullptr));
	if(!pfile)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - failed to load '%s'.\n", __FUNCTION__, HUD_DESCRIPTION_SCRIPT_PATH);
		return false;
	}

	if(!m_iconInfoArray.empty())
		m_iconInfoArray.clear();

	if(!m_itemInfoArray.empty())
		m_itemInfoArray.clear();

	if(!m_weaponInfoArray.empty())
		m_weaponInfoArray.clear();

	// Texture flags used by all HUD textures
	Int32 textureflags = (TX_FL_NOMIPMAPS|TX_FL_CLAMP_S|TX_FL_CLAMP_T);

	Char token[256];
	const Char* pstr = pfile;
	while(pstr)
	{
		// Read in first token
		pstr = Common::Parse(pstr, token);
		if(!pstr || !qstrlen(token))
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF.\n", __FUNCTION__);
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		if(token[0] != '$')
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Expected a token starting with '$', got '%s' instead.\n", __FUNCTION__, token);
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		// Remember for later
		CString typeToken = token;
		// To hold all key->pair pairs
		CArray<hud_infopair_t> infoPairArray;

		// Next token needs to be a {
		pstr = Common::Parse(pstr, token);
		if(!pstr || !qstrlen(token))
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading '%s'.\n", __FUNCTION__, typeToken.c_str());
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		if(qstrcmp(token, "{"))
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Expected '{' token while reading '%s', got '%s' instead.\n", __FUNCTION__, typeToken.c_str(), token);
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		// Loop on elements
		while(true)
		{
			pstr = Common::Parse(pstr, token);
			// Check for end of definition
			if(!qstrcmp(token, "}"))
				break;

			if(!pstr || !qstrlen(token))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading '%s'.\n", __FUNCTION__, typeToken.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			// Save key
			CString keyToken = token;

			// Read value
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading '%s'.\n", __FUNCTION__, typeToken.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			hud_infopair_t newPair;
			newPair.key = keyToken;
			newPair.value = token;
			infoPairArray.push_back(newPair);
		}

		// Now add the element to the HUD's list
		if(!qstrcmp(typeToken, "$icon"))
		{
			// Current item's index as it'll be set
			Uint32 index = m_iconInfoArray.size();

			hud_iconinfo_t newIcon;
			newIcon.name = GetValueForKey(infoPairArray, "name");
			if(newIcon.name.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No 'name' setting specified for '%s' object by index '%d'.\n", __FUNCTION__, typeToken.c_str(), index);
				continue;
			}

			// Retrieve texture path
			CString textureName;
			textureName = GetValueForKey(infoPairArray, "texture");
			if(textureName.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No texture name specified for '%s' object '%s'.\n", __FUNCTION__, typeToken.c_str(), newIcon.name.c_str());
			}
			else
			{
				newIcon.ptexture = cl_renderfuncs.pfnLoadTexture(textureName.c_str(), RS_GAME_LEVEL, textureflags, nullptr);
				if(!newIcon.ptexture)
					cl_engfuncs.pfnCon_EPrintf("%s - Could not load texture '%s' for '%s' object '%s'.\n", __FUNCTION__, textureName.c_str(), typeToken.c_str(), newIcon.name.c_str());
			}

			if(!newIcon.ptexture)
				newIcon.ptexture = cl_renderfuncs.pfnGetDummyTexture();

			Uint32 i = 0;
			for(; i < m_iconInfoArray.size(); i++)
			{
				if(!qstrcmp(m_iconInfoArray[i].name, newIcon.name))
				{
					cl_engfuncs.pfnCon_EPrintf("%s - '%s' object '%s' redefinition.\n", __FUNCTION__, typeToken.c_str(), newIcon.name.c_str());
					break;
				}
			}

			if(i == m_iconInfoArray.size())
				m_iconInfoArray.push_back(newIcon);
		}
		else if(!qstrcmp(typeToken, "$weapon"))
		{
			// Current item's index as it'll be set
			Uint32 index = m_iconInfoArray.size();

			hud_weaponinfo_t newWeapon;
			CString name = GetValueForKey(infoPairArray, "name");
			if(name.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No 'name' setting specified for '%s' object by index '%d'.\n", __FUNCTION__, typeToken.c_str(), index);
				continue;
			}
			else
			{
				Int32 i = 0;
				while(true)
				{
					if(!qstrlen(WEAPONMAPPINGS[i].name))
					{
						// Reached end
						break;
					}
					else if(!qstrcmp(name, WEAPONMAPPINGS[i].name))
					{
						newWeapon.weaponid = (weaponid_t)WEAPONMAPPINGS[i].id;
						break;
					}

					i++;
				}

				if(newWeapon.weaponid == WEAPON_NONE)
				{
					cl_engfuncs.pfnCon_EPrintf("%s - Weapon '%s' not recognized.\n", __FUNCTION__, name.c_str());
					continue;
				}
			}

			// Retrieve weaponicon_texture texture path
			CString textureName;
			textureName = GetValueForKey(infoPairArray, "weaponicon_texture");
			if(textureName.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No texture name specified for '%s' object '%s'.\n", __FUNCTION__, typeToken.c_str(), name.c_str());
			}
			else
			{
				newWeapon.pweapon_icon = cl_renderfuncs.pfnLoadTexture(textureName.c_str(), RS_GAME_LEVEL, textureflags, nullptr);
				if(!newWeapon.pweapon_icon)
					cl_engfuncs.pfnCon_EPrintf("%s - Could not load texture '%s' for '%s' object '%s'.\n", __FUNCTION__, textureName.c_str(), typeToken.c_str(), name.c_str());
			}

			if(!newWeapon.pweapon_icon)
				newWeapon.pweapon_icon = cl_renderfuncs.pfnGetDummyTexture();

			// See if ammoammoicon_texture is specified
			textureName = GetValueForKey(infoPairArray, "ammoicon_texture");
			if(!textureName.empty())
			{
				newWeapon.pammo_icon = cl_renderfuncs.pfnLoadTexture(textureName.c_str(), RS_GAME_LEVEL, textureflags, nullptr);
				if(!newWeapon.pammo_icon)
				{
					cl_engfuncs.pfnCon_EPrintf("%s - Could not load texture '%s' for '%s' object '%s'.\n", __FUNCTION__, textureName.c_str(), typeToken.c_str(), name.c_str());
					newWeapon.pammo_icon = cl_renderfuncs.pfnGetDummyTexture();
				}
			}

			// Get description
			newWeapon.description = GetValueForKey(infoPairArray, "description");
			if(newWeapon.description.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No description specified for '%s' object '%s'.\n", __FUNCTION__, typeToken.c_str(), name.c_str());
				newWeapon.description = "NA";
			}

			// Set all other optional ones
			newWeapon.description_dual = GetValueForKey(infoPairArray, "description_dual");
			newWeapon.description_ammo = GetValueForKey(infoPairArray, "description_ammo");

			Uint32 i = 0;
			for(; i < m_weaponInfoArray.size(); i++)
			{
				if(m_weaponInfoArray[i].weaponid == newWeapon.weaponid)
				{
					cl_engfuncs.pfnCon_EPrintf("%s - '%s' object '%s' redefinition.\n", __FUNCTION__, typeToken.c_str(), name.c_str());
					break;
				}
			}

			if(i == m_weaponInfoArray.size())
				m_weaponInfoArray.push_back(newWeapon);
		}
		else if(!qstrcmp(typeToken, "$item"))
		{
			// Current item's index as it'll be set
			Uint32 index = m_itemInfoArray.size();

			hud_iteminfo_t newItem;
			newItem.name = GetValueForKey(infoPairArray, "name");
			if(newItem.name.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No 'name' setting specified for '%s' object by index '%d'.\n", __FUNCTION__, typeToken.c_str(), index);
				continue;
			}

			// Get description
			CString weaponid = GetValueForKey(infoPairArray, "weaponid");
			if(!weaponid.empty())
			{
				Int32 i = 0;
				while(true)
				{
					if(!qstrlen(WEAPONMAPPINGS[i].name))
					{
						cl_engfuncs.pfnCon_EPrintf("%s - Weapon '%s' not recognized for '%s' object '%s'.\n", __FUNCTION__, typeToken.c_str(), newItem.name.c_str());
						break;
					}
					else if(!qstrcmp(weaponid, WEAPONMAPPINGS[i].name))
					{
						newItem.weaponid = (weaponid_t)WEAPONMAPPINGS[i].id;
						break;
					}

					i++;
				}
			}

			Uint32 i = 0;
			for(; i < m_itemInfoArray.size(); i++)
			{
				if(!qstrcmp(m_itemInfoArray[i].name, newItem.name))
					break;
			}

			if(i != m_itemInfoArray.size())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - '%s' object '%s' redefinition.\n", __FUNCTION__, typeToken.c_str(), newItem.name.c_str());
				continue;
			}

			if(newItem.weaponid != WEAPON_NONE)
			{
				i = 0;
				for(; i < m_itemInfoArray.size(); i++)
				{
					if(m_itemInfoArray[i].weaponid == newItem.weaponid)
						break;
				}

				if(i != m_itemInfoArray.size())
				{
					cl_engfuncs.pfnCon_EPrintf("%s - '%s' object '%s' redefinition.\n", __FUNCTION__, typeToken.c_str(), WEAPONMAPPINGS[newItem.weaponid].name);
					continue;
				}
			}

			// Get description
			newItem.description = GetValueForKey(infoPairArray, "description");
			if(newItem.description.empty())
			{
				cl_engfuncs.pfnCon_EPrintf("%s - No description specified for '%s' object '%s'.\n", __FUNCTION__, typeToken.c_str(), newItem.name.c_str());
				newItem.description = "NA";
			}

			m_itemInfoArray.push_back(newItem);
		}
		else
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Object '%s' not recognized.\n", __FUNCTION__, typeToken.c_str());
			continue;
		}
	}

	// Release the file
	cl_filefuncs.pfnFreeFile(pfile);
	return true;
}

//=============================================
// @brief Returns value for a key
//
//=============================================
const Char* CGameHUD::GetValueForKey( const CArray<hud_infopair_t>& srcArray, const Char* pstrKey )
{
	for(Uint32 i = 0; i < srcArray.size(); i++)
	{
		if(!qstrcmp(srcArray[i].key, pstrKey))
			return srcArray[i].value.c_str();
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetCurrentWeapon( Int32 state, Int32 id, Uint32 clip, Uint32 clipright, Uint32 clipleft, Uint32 cone )
{
	if(id == -1)
		return;

	weapon_t* pweapon = m_weaponInfo.GetWeapon(id);
	if(!pweapon)
		return;

	pweapon->clip = clip;
	pweapon->dualclipleft = clipleft;
	pweapon->dualclipright = clipright;
	pweapon->cone = cone;

	if(state != 0)
	{
		m_pWeapon = pweapon;
		m_pActiveWeaponInfo = GetWeaponInfoById((weaponid_t)id);
	}
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::AddWeaponList( const Char* pstrname, Int32 ammotype, Int32 maxammo, Int32 maxclip, Uint32 slot, Uint32 slotposition, Uint32 weaponid, Int32 flags )
{
	weapon_t weapon;
	weapon.name = pstrname;
	weapon.ammotype = ammotype;
	weapon.maxammo = maxammo;
	weapon.maxclip = maxclip;
	weapon.slot = slot;
	weapon.slotposition = slotposition;
	weapon.weaponid = weaponid;
	weapon.flags = flags;

	weapon.clip = 0;
	weapon.dualclipright = 0;
	weapon.dualclipleft = 0;
	weapon.cone = 0;

	// Reset this
	m_prevWeaponBits = 0;

	m_weaponInfo.AddWeapon(weapon);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetAmmoCount( Uint32 ammoIndex, Uint32 ammoCount )
{
	m_weaponInfo.SetAmmoCount(ammoIndex, ammoCount);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::AmmoPickup( const Char* pstrentityname, Uint32 count )
{
	m_pHistory->AddElement(CHUDHistory::HISTORY_AMMO, pstrentityname, count);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::WeaponPickup( Int32 id )
{
	m_pHistory->AddElement(CHUDHistory::HISTORY_WEAPON, id);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::ItemPickup( const Char* pstrentityname )
{
	m_pHistory->AddElement(CHUDHistory::HISTORY_ITEM, pstrentityname);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetActive( bool active )
{
	m_isActive = active;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetTacticalBattery( bool ison, Float amount )
{
	m_isTacticalOn = ison;
	m_tacticalAmount = amount;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SlotInput( Int32 iSlot )
{
	SelectWeaponSlot(iSlot, false, 1);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::UserCmd_Close(void)
{
	if (m_pActiveSelection || m_isDrySelection)
	{
		m_pLastSelection = m_pActiveSelection;
		m_pActiveSelection = nullptr;
		m_isDrySelection = false;
		PlaySound("common/wpn_hudoff.wav");
	}
	else
	{
		cl_engfuncs.pfnClientCommand("escape");
	}
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::UserCmd_NextWeapon( void )
{
	if(m_health < 0 || !m_isActive)
		return;

	if ( !m_pActiveSelection || m_isDrySelection )
	{
		m_pActiveSelection = m_pWeapon;
		m_isDrySelection = false;
	}

	Int32 position = 0;
	Int32 slot = 0;
	if (m_pActiveSelection)
	{
		position = m_pActiveSelection->slotposition + 1;
		slot = m_pActiveSelection->slot;
	}

	for(Int32 i = 0; i <= 1; i++)
	{
		for(; slot < MAX_WEAPON_SLOTS; slot++)
		{
			for(; position < MAX_SLOT_POSITIONS; position++)
			{
				weapon_t* pweapon = m_weaponInfo.GetWeaponFromSlot(slot, position);
				if(pweapon)
				{
					m_pActiveSelection = pweapon;
					m_isDrySelection = false;
					return;
				}
			}

			position = 0;
		}
		
		slot = 0;
	}

	m_pActiveSelection = nullptr;
	m_isDrySelection = false;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::UserCmd_PrevWeapon( void )
{
	if(m_health < 0 || !m_isActive)
		return;

	if (!m_pActiveSelection || m_isDrySelection)
	{
		m_pActiveSelection = m_pWeapon;
		m_isDrySelection = false;
	}

	Int32 position = MAX_SLOT_POSITIONS-1;
	Int32 slot = MAX_WEAPON_SLOTS-1;
	if ( m_pActiveSelection )
	{
		position = m_pActiveSelection->slotposition - 1;
		slot = m_pActiveSelection->slot;
	}
	
	for(Int32 i = 0; i <= 1; i++)
	{
		for(; slot >= 0; slot--)
		{
			for(; position >= 0; position--)
			{
				weapon_t* pweapon = m_weaponInfo.GetWeaponFromSlot(slot, position);
				if(pweapon)
				{
					m_pActiveSelection = pweapon;
					m_isDrySelection = false;
					return;
				}
			}

			position = MAX_SLOT_POSITIONS-1;
		}
		
		slot = MAX_WEAPON_SLOTS-1;
	}

	m_pActiveSelection = nullptr;
	m_isDrySelection = false;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::IsActive( void ) const
{
	if(!m_isActive)
		return false;

	if(!HasAnyWeapons())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SelectWeaponSlot( Uint32 slot, bool advance, Int32 direction )
{
	assert(slot < MAX_WEAPON_SLOTS);

	if(!gHUD.IsActive())
		return;

	weapon_t* pweapon = nullptr; 
	if(!m_pActiveSelection || m_isDrySelection || slot != m_pActiveSelection->slot)
	{
		PlaySound("common/wpn_hudon.wav");
		pweapon = m_weaponInfo.GetSlotFirstWeapon(slot);
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav");
		pweapon = m_weaponInfo.GetNextSlotWeapon(m_pActiveSelection->slot, m_pActiveSelection->slotposition);
	}

	if(!pweapon)
	{
		m_pActiveSelection = nullptr;
		m_isDrySelection = true;
	}
	else
	{
		m_pActiveSelection = pweapon;
		m_isDrySelection = false;
	}
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::PlaySound( const Char* pstrfilepath )
{
	// Play menu sound
	cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, pstrfilepath, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_MENU, 0);
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::AddRadioMessage( const Char* pstrcallername, const color32_t& color, Float lifetime, entindex_t entindex )
{
	m_radioMessagesList.begin();
	while(!m_radioMessagesList.end())
	{
		const radiomessage_t& msg = m_radioMessagesList.get();
		if(msg.entindex == entindex)
		{
			m_radioMessagesList.remove(m_radioMessagesList.get_link());
			break;
		}

		m_radioMessagesList.next();
	}

	if(!lifetime || !pstrcallername || pstrcallername[0] == '\0')
		return;

	radiomessage_t newmsg;
	newmsg.callername = pstrcallername;
	newmsg.color = color;
	newmsg.life = cl_engfuncs.pfnGetClientTime() + lifetime;
	newmsg.spawntime = cl_engfuncs.pfnGetClientTime();
	newmsg.entindex = entindex;

	m_radioMessagesList.add(newmsg);
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawNoiseTab( void )
{
	Float x = gHUDDraw.ScaleX(50);
	Float y = gHUDDraw.ScaleY(BASE_RESOLUTION_Y-50-TAB_GENERIC_SIZE_Y) - 2*(gHUDDraw.ScaleY(10) + gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y));

	Float tabFullness = (m_movementNoise / PLAYER_MAX_NOISE_LEVEL)*100;

	// Draw speed tab
	return DrawTab_Bar(x, y, nullptr, m_pNoiseIcon, tabFullness, TAB_MOVENOISE_SIZE_X, 1.0, nullptr, nullptr, true);
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawNPCAwarenessTab( void )
{
	Float x = gHUDDraw.ScaleX(50);
	Float y = gHUDDraw.ScaleY(BASE_RESOLUTION_Y-95-TAB_GENERIC_SIZE_Y) - 2*(gHUDDraw.ScaleY(10) + gHUDDraw.ScaleY(TAB_GENERIC_SIZE_Y));

	Float tabFullness = m_npcAwareness*100;

	// Draw speed tab
	return DrawTab_Bar(x, y, nullptr, m_pAwarenessIcon, tabFullness, TAB_MOVENOISE_SIZE_X, 1.0, nullptr, nullptr, true);
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawUsableObjectReticle( void )
{
	if(m_pCvarDrawUseReticle->GetValue() < 1)
		return true;

	if(m_usableObjectType == USABLE_OBJECT_NONE)
		return true;

	if(m_usableObjectMins.IsZero() && m_usableObjectMaxs.IsZero())
		return true;

	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	Vector vTemp;
	static Vector vBounds[8];
	for (Uint32 i = 0; i < 8; i++)
	{
		if ( i & 1 ) vTemp[0] = m_usableObjectMins[0];
		else vTemp[0] = m_usableObjectMaxs[0];
		if ( i & 2 ) vTemp[1] = m_usableObjectMins[1];
		else vTemp[1] = m_usableObjectMaxs[1];
		if ( i & 4 ) vTemp[2] = m_usableObjectMins[2];
		else vTemp[2] = m_usableObjectMaxs[2];
		Math::VectorCopy( vTemp, vBounds[i] );
	}

	Float screenMins[2] = {999999, 999999};
	Float screenMaxs[2] = {-999999, -999999};

	// Calculate screen mins/maxs
	bool allOccluded = true;
	for(Uint32 i = 0; i < 8; i++)
	{
		Float vOrigin [] = { vBounds[i][0], vBounds[i][1], vBounds[i][2], 1.0 };

		// Multiply with modelview
		Float viewPos[4];
		Math::MatMult4(m_modelviewMatrix.GetMatrix(), vOrigin, viewPos);

		// Multiply with projection
		Float screenCoords[4];
		Math::MatMult4(m_projectionMatrix.GetMatrix(), viewPos, screenCoords);

		// See if it's z-clipped
		if(screenCoords[3] <= 0)
			continue;

		// Calculate uniform values
		Float coordX = (screenCoords[0]/screenCoords[3])*0.5 + 0.5;
		coordX *= screenWidth;
		if(coordX > screenWidth)
			coordX = screenWidth;
		else if(coordX < 0)
			coordX = 0;

		Float coordY = (screenCoords[1]/screenCoords[3])*0.5 + 0.5;
		coordY *= screenHeight;
		coordY = (screenHeight - coordY);

		if(coordY > screenHeight)
			coordY = screenHeight;
		else if(coordY < 0)
			coordY = 0;

		if(coordX > screenMaxs[0])
			screenMaxs[0] = coordX;
		if(coordY > screenMaxs[1])
			screenMaxs[1] = coordY;

		if(coordX < screenMins[0])
			screenMins[0] = coordX;
		if(coordY < screenMins[1])
			screenMins[1] = coordY;

		allOccluded = false;
	}

	if(allOccluded)
		return true;

	// Draw the reticle based on the mins/maxs
	Float boxX = screenMins[0];
	Float boxY = screenMins[1];
	Float boxWidth = screenMaxs[0] - screenMins[0];
	Float boxHeight = screenMaxs[1] - screenMins[1];

	const Float outwardDistanceRef = 15;
	Float outwardDistance = gHUDDraw.ScaleX(outwardDistanceRef);

	Double time = cl_engfuncs.pfnGetClientTime();
	Float outwardMotion = time * 3 * outwardDistance;
	outwardMotion = (Int32)outwardMotion % (Int32)outwardDistance;
	outwardMotion = gHUDDraw.ScaleX(outwardMotion);

	boxX -= outwardMotion;
	if(boxX < 0)
		boxX = 0;

	boxWidth += outwardMotion * 2;
	if(boxX + boxWidth > screenWidth)
		boxWidth = screenWidth - boxX;

	boxY -= outwardMotion;
	if(boxY < 0)
		boxY = 0;

	boxHeight += outwardMotion * 2;
	if(boxY + boxHeight > screenHeight)
		boxHeight = screenHeight - boxY;

	Vector reticleColor;
	switch(m_usableObjectType)
	{
	case USABLE_OBJECT_DEFAULT:
		reticleColor = HUD_COLOR_BLUE;
		break;
	case USABLE_OBJECT_LOCKED:
		reticleColor = HUD_COLOR_ORANGE;
		break;
	case USABLE_OBJECT_UNUSABLE:
		reticleColor = HUD_COLOR_RED;
		break;
	}

	// Set color of reticle
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gHUDDraw.SetColor(reticleColor, 255);

	Float baseEdgeSize = 2;
	Float edgeSize = gHUDDraw.ScaleX(baseEdgeSize);
	if(edgeSize < 1)
		edgeSize = 1;

	if(boxWidth > 0 && boxHeight > 0)
	{
		// Size of a reticle corner
		const Float reticleCornerSize = 35;
		// Size of a corner adjusted
		Float adjCornerSize = gHUDDraw.ScaleX(reticleCornerSize) + outwardMotion;

		Float maxSize = (boxWidth > boxHeight) ? boxHeight : boxWidth;
		maxSize = maxSize * 0.5f;

		if(adjCornerSize > maxSize)
			adjCornerSize = maxSize;

		// Draw left upper reticle corner
		{
			gHUDDraw.SetOrigin(boxX, boxY);
			gHUDDraw.SetSize(adjCornerSize, edgeSize);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			gHUDDraw.SetOrigin(boxX, boxY);
			gHUDDraw.SetSize(edgeSize, adjCornerSize);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}

		// Draw left lower reticle corner
		{
			Float width = edgeSize;
			Float height = adjCornerSize;

			Float originX = boxX;
			Float originY = boxY + boxHeight - adjCornerSize;

			gHUDDraw.SetOrigin(originX, originY);
			gHUDDraw.SetSize(width, height);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			originY = boxY + boxHeight - edgeSize;
			originX = boxX;

			height = edgeSize;
			width = adjCornerSize;

			gHUDDraw.SetOrigin(originX, originY);
			gHUDDraw.SetSize(width, height);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}

		// Draw right upper reticle corner
		{
			Float originX = boxX + boxWidth - adjCornerSize;
			Float originY = boxY;

			Float width = adjCornerSize;
			Float height = edgeSize;

			gHUDDraw.SetOrigin(originX, originY);
			gHUDDraw.SetSize(width, height);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			originX = boxX + boxWidth - edgeSize;
			originY = boxY;

			width = edgeSize;
			height = adjCornerSize;

			gHUDDraw.SetOrigin(originX, originY);
			gHUDDraw.SetSize(width, height);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}
		
		// Draw right lower reticle corner
		{
			Float originX = boxX + boxWidth - edgeSize;
			Float originY = boxY + boxHeight - adjCornerSize;

			Float width = edgeSize;
			Float height = adjCornerSize;

			gHUDDraw.SetOrigin(originX, originY);
			gHUDDraw.SetSize(width, height);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;

			originX = boxX + boxWidth - adjCornerSize;
			originY = boxY + boxHeight - edgeSize;

			height = edgeSize;
			width = adjCornerSize;

			gHUDDraw.SetOrigin(originX, originY);
			gHUDDraw.SetSize(width, height);

			// Draw the quad
			if(!gHUDDraw.DrawQuad(nullptr))
				return false;
		}
	}

	return true;
}

//=============================================
// @brief Draws countdowntimer
//
//=============================================
bool CGameHUD::DrawCountdownTimer( void )
{
	if(m_countdownTimerTitle.empty() && !m_countdownTimerTime)
		return true;

	Double time = cl_engfuncs.pfnGetClientTime();
	Double timeLeft = m_countdownTimerTime - time;
	if(timeLeft < 0)
		timeLeft = 0;

	Int32 hours = timeLeft / (60.0f * 60.0f);
	Int32 minutes = (timeLeft / 60.0f) - (hours * 60);
	Int32 seconds = SDL_floor(timeLeft) - (hours * 60.0f * 60.0f) - (minutes * 60.0f);
	Int32 milliseconds = ((timeLeft - SDL_floor(timeLeft)) * 1000) * 100 / 100;

	// hours, minutes, seconds + semicolon
	// everything uses 0 as advancex
	Int32 digitWidth = m_pCounterFont->glyphs['0'].advancex;
	Int32 counterWidth = digitWidth * 12;

	Int32 totalWidth = counterWidth;
	Int32 totalHeight = m_pCounterFont->fontsize + CHUDDraw::HUD_EDGE_SIZE;

	Float textDrawPosition = 0;
	if(!m_countdownTimerTitle.empty())
	{
		// Determine width
		Uint32 width = 0;
		cl_renderfuncs.pfnGetStringSize(m_pFontSet, m_countdownTimerTitle.c_str(), &width, nullptr, nullptr);
		width += CHUDDraw::HUD_EDGE_SIZE;
		if(width > totalWidth)
			totalWidth = width;

		// Set draw position for text
		textDrawPosition = m_screenWidth / 2.0f - width / 2.0f;

		// Just use fontsize
		totalHeight += m_pFontSet->fontsize;
	}

	Float positionX = m_screenWidth / 2.0f - totalWidth / 2.0f;
	Float positionY = gHUDDraw.ScaleY(50);

	// Draw the quad
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gHUDDraw.SetOrigin(positionX, positionY);
	gHUDDraw.SetSize(totalWidth, totalHeight);
	gHUDDraw.SetColor(HUD_COLOR_BLACK, HUD_DEFAULT_ALPHA);

	// Draw the body
	if(!gHUDDraw.DrawBody())
		return false;

	Float drawPositionY = positionY;

	// Draw the title text
	if(!m_countdownTimerTitle.empty())
	{
		drawPositionY += m_pFontSet->fontsize;
		if(!gHUDDraw.DrawText(HUD_COLOR_WHITE, 255, textDrawPosition, drawPositionY, m_countdownTimerTitle.c_str(), m_pFontSet))
			return false;
	}

	gHUDDraw.FinishDraw();

	// Draw the counter itself
	CString counterText;

	// Hours
	if(hours < 10)
		counterText << "0";

	counterText << hours << ":";

	// Minutes
	if(minutes < 10)
		counterText << "0";

	counterText << minutes << ":";

	// Seconds
	if(seconds < 10)
		counterText << "0";

	counterText << seconds << ":";

	// Milliseconds
	if(milliseconds < 10)
		counterText << "00";
	else if(milliseconds < 100)
		counterText << "0";
	counterText << milliseconds;

	drawPositionY += m_pCounterFont->fontsize;

	// Begin text rendering
	if(!cl_renderfuncs.pfnBeginTextRendering(m_pCounterFont))
	{
		cl_engfuncs.pfnErrorPopup("Shader error: %s.", cl_renderfuncs.pfnGetStringDrawError());
		return false;
	}

	Vector color;
	if(minutes <= 0 && (Int32)(time*2) % 2 == 1)
		color = HUD_COLOR_RED;
	else
		color = HUD_COLOR_WHITE;

	Float drawPositionX = m_screenWidth / 2.0f - counterWidth / 2.0f;
	const Char* pstr = counterText.c_str();
	while(*pstr)
	{
		Float nudgeSize = 0;
		Int32 characterSize = m_pCounterFont->glyphs[*pstr].advancex;
		if(characterSize < digitWidth)
			nudgeSize = (digitWidth - characterSize) / 2.0f;

		if(!cl_renderfuncs.pfnDrawCharacter(m_pCounterFont, drawPositionX + nudgeSize, drawPositionY, *pstr, color.x, color.y, color.z, 255 ))
			return false;

		drawPositionX += digitWidth;
		pstr++;
	}
	
	// Finish rendering
	cl_renderfuncs.pfnFinishTextRendering(m_pCounterFont);

	if(!gHUDDraw.SetupDraw())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::DrawRadioMessages( void )
{
	if(m_radioMessagesList.empty())
		return true;

	Double time = cl_engfuncs.pfnGetClientTime();

	// Size of the radio tab
	Uint32 radiotabwidth = gHUDDraw.ScaleY(RADIO_MSGTAB_SIZE_X);
	Uint32 radiotabheight = gHUDDraw.ScaleY(RADIO_MSGTAB_SIZE_Y);

	// Base X Position
	Int32 baseXPosition = gHUDDraw.ScaleX(BASE_RESOLUTION_X) - radiotabwidth - gHUDDraw.ScaleX(RADIO_MSGTAB_SPACING);
	Int32 baseYPosition = gHUDDraw.ScaleY(BASE_RESOLUTION_Y) / 2.0f;

	// Size of the radio icon
	Uint32 radioiconwidth = gHUDDraw.ScaleX(m_pRadioIcon->width);
	Uint32 radioiconheight = gHUDDraw.ScaleY(m_pRadioIcon->height);
	Uint32 radioiconspacing = (radiotabheight-radioiconheight)/2.0f;

	// Calculate the total height of all messages
	Uint32 totalheight = 0;
	m_radioMessagesList.begin();
	while(!m_radioMessagesList.end())
	{
		const radiomessage_t& msg = m_radioMessagesList.get();

		Float msgalpha = 1.0;
		if(msg.spawntime + RADIO_MSG_FADETIME > time)
			msgalpha = (time - msg.spawntime)/RADIO_MSG_FADETIME;
		else if(msg.life - RADIO_MSG_FADETIME < time)
			msgalpha = 1.0 - ((time-(msg.life - RADIO_MSG_FADETIME))/RADIO_MSG_FADETIME);

		// Remove dead ones
		if(msg.life <= cl_engfuncs.pfnGetClientTime())
		{
			m_radioMessagesList.remove(m_radioMessagesList.get_link());
			m_radioMessagesList.next();
			continue;
		}

		// Offset this
		totalheight += (radiotabheight + gHUDDraw.ScaleY(RADIO_MSGTAB_SPACING))*msgalpha;

		m_radioMessagesList.next();
	}

	// Center vertically
	baseYPosition -= totalheight * 0.5;

	// Now draw the radio message tabs
	Uint32 offset = 0;
	m_radioMessagesList.begin();
	while(!m_radioMessagesList.end())
	{
		radiomessage_t& msg = m_radioMessagesList.get();
	
		Float msgalpha = 1.0;
		if(msg.spawntime + RADIO_MSG_FADETIME > time)
			msgalpha = (time - msg.spawntime)/RADIO_MSG_FADETIME;
		else if(msg.life - RADIO_MSG_FADETIME < time)
			msgalpha = 1.0 - ((time-(msg.life - RADIO_MSG_FADETIME))/RADIO_MSG_FADETIME);

		Int32 yPosition = baseYPosition + offset;

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gHUDDraw.SetOrigin(baseXPosition, yPosition);
		gHUDDraw.SetSize(radiotabwidth, radiotabheight);
		gHUDDraw.SetColor(msg.color.r, msg.color.g, msg.color.b, msg.color.a*msgalpha);

		// Draw the quad
		if(!gHUDDraw.DrawQuad(nullptr))
			return false;

		// Now determine the icon position
		Int32 radioIconXPosition = baseXPosition + radiotabwidth - radioiconwidth - radioiconspacing;
		Int32 radioIconYPosition = yPosition + radioiconspacing;

		glBlendFunc(GL_ONE, GL_ONE);
		gHUDDraw.SetOrigin(radioIconXPosition, radioIconYPosition);
		gHUDDraw.SetSize(radioiconwidth, radioiconheight);
		gHUDDraw.SetColor(HUD_COLOR_WHITE*msgalpha, 255*msgalpha);

		// Draw the icon
		if(!gHUDDraw.DrawQuad(m_pRadioIcon))
			return false;

		// Determine text position
		Uint32 textHeight = cl_renderfuncs.pfnEstimateStringHeight(m_pSubtitleSet, msg.callername.c_str(), 0);
		Int32 textXPosition = baseXPosition + gHUDDraw.ScaleX(RADIO_MSGTAB_SPACING);
		Int32 textYPosition = yPosition + (radiotabheight/2.0f) + (textHeight/2.0f);

		if(!gHUDDraw.DrawText(HUD_COLOR_WHITE, msg.color.a*msgalpha, textXPosition, textYPosition, msg.callername.c_str(), m_pSubtitleSet))
			return false;

		// Offset this
		offset += (radiotabheight + gHUDDraw.ScaleY(RADIO_MSGTAB_SPACING))*msgalpha;

		m_radioMessagesList.next();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGameHUD::SetAutoaimVector( Float autoAimX, Float autoAimY, bool isOnTarget )
{
	m_idealAutoAimVector[0] = autoAimX;
	m_idealAutoAimVector[1] = autoAimY;
	m_isOnTarget = isOnTarget;
}

//=============================================
// @brief
//
//=============================================
bool CGameHUD::HasAnyWeapons( void ) const
{
	for(Uint32 i = 0; i < NUM_WEAPONS; i++)
	{
		if(m_weaponBits & (1<<(i)))
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
CWeaponInfo::CWeaponInfo( void )
{
	Reset();
}

//=============================================
// @brief
//
//=============================================
CWeaponInfo::~CWeaponInfo( void )
{
}

//=============================================
// @brief
//
//=============================================
void CWeaponInfo::Reset( void )
{
	for(Uint32 i = 0; i < MAX_WEAPONS; i++)
		m_weaponsArray[i] = weapon_t();

	memset(m_pSlotsArray, 0, sizeof(m_pSlotsArray));

	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
		m_ammoCounts[i] = 0;
}

//=============================================
// @brief
//
//=============================================
void CWeaponInfo::AddWeapon( const weapon_t& weapon )
{
	assert(weapon.weaponid < MAX_WEAPONS);
	assert(weapon.slotposition < MAX_SLOT_POSITIONS);
	assert(weapon.slot < MAX_WEAPON_SLOTS);

	// Set weapon info
	m_weaponsArray[weapon.weaponid] = weapon;
}

//=============================================
// @brief
//
//=============================================
void CWeaponInfo::AssignWeapon( weapon_t* pweapon )
{
	if(pweapon->weaponid == WEAPON_NONE)
		return;

	assert(pweapon->weaponid < MAX_WEAPONS);
	assert(pweapon->slotposition < MAX_SLOT_POSITIONS);
	assert(pweapon->slot < MAX_WEAPON_SLOTS);

	m_pSlotsArray[pweapon->slot][pweapon->slotposition] = pweapon;
}

//=============================================
// @brief
//
//=============================================
weapon_t* CWeaponInfo::GetWeapon( Uint32 id )
{
	assert(id < MAX_WEAPONS);
	return &m_weaponsArray[id];
}

//=============================================
// @brief
//
//=============================================
void CWeaponInfo::RemoveWeapon( weapon_t* pweapon )
{
	if(pweapon->weaponid == WEAPON_NONE)
		return;

	assert(pweapon->weaponid < MAX_WEAPONS);
	assert(pweapon->slotposition < MAX_SLOT_POSITIONS);
	assert(pweapon->slot < MAX_WEAPON_SLOTS);

	m_pSlotsArray[pweapon->slot][pweapon->slotposition] = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CWeaponInfo::RemoveAllWeapons( void )
{
	for(Uint32 i = 0; i < MAX_WEAPONS; i++)
	{
		if(m_weaponsArray[i].weaponid)
			RemoveWeapon(&m_weaponsArray[i]);
	}
}

//=============================================
// @brief
//
//=============================================
weapon_t* CWeaponInfo::GetWeaponFromSlot( Uint32 slot, Uint32 position )
{
	assert(slot < MAX_WEAPON_SLOTS);
	assert(position < MAX_SLOT_POSITIONS);

	return m_pSlotsArray[slot][position];
}

//=============================================
// @brief
//
//=============================================
weapon_t* CWeaponInfo::GetSlotFirstWeapon( Uint32 slot )
{
	assert(slot < MAX_WEAPON_SLOTS);

	for(Uint32 i = 0; i < MAX_SLOT_POSITIONS; i++)
	{
		if(m_pSlotsArray[slot][i])
			return m_pSlotsArray[slot][i];
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
weapon_t* CWeaponInfo::GetNextSlotWeapon( Uint32 slot, Uint32 position )
{
	assert(slot < MAX_WEAPON_SLOTS);
	assert(position < MAX_SLOT_POSITIONS);

	if((position+1) >= MAX_SLOT_POSITIONS)
		return GetSlotFirstWeapon(slot);

	weapon_t* pweapon = nullptr;
	for(Uint32 i = position+1; i < MAX_SLOT_POSITIONS; i++)
	{
		if(m_pSlotsArray[slot][i])
		{
			pweapon = m_pSlotsArray[slot][i];
			break;
		}
	}

	if(!pweapon)
		pweapon = GetSlotFirstWeapon(slot);

	return pweapon;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponInfo::WeaponHasAmmo( weapon_t* pweapon ) const
{
	if(!pweapon)
		return false;

	if(pweapon->maxammo == -1)
		return true;

	if(pweapon->ammotype == -1 || pweapon->clip > 0 || GetAmmoCount(pweapon->ammotype) > 0 || pweapon->dualclipleft || pweapon->dualclipright)
		return true;
	
	return false;
}

//=============================================
// @brief
//
//=============================================
Uint32 CWeaponInfo::GetAmmoCount( Int32 ammoId ) const
{
	assert(ammoId < MAX_AMMO_TYPES);

	if(ammoId < 0)
		return 0;

	return m_ammoCounts[ammoId];
}

//=============================================
// @brief
//
//=============================================
void CWeaponInfo::SetAmmoCount( Int32 ammoId, Uint32 count )
{
	assert(ammoId < MAX_AMMO_TYPES);

	if(ammoId == -1)
		return;

	m_ammoCounts[ammoId] = count;
}

//=============================================
// @brief
//
//=============================================
CHUDHistory::CHUDHistory( CGameHUD& mainHUD ):
	m_hud(mainHUD)
{
}

//=============================================
// @brief
//
//=============================================
CHUDHistory::~CHUDHistory( void )
{
}

//=============================================
// @brief
//
//=============================================
void CHUDHistory::Reset( void )
{
	if(!m_historyList.empty())
		m_historyList.clear();
}

//=============================================
// @brief
//
//=============================================
void CHUDHistory::AddElement( Uint32 type, Uint32 id, Uint32 count )
{
	if(type == HISTORY_AMMO && !count)
		return;

	weaponid_t weaponId = (weaponid_t)id;
	const Char* pstrDescription = m_hud.GetItemDescriptionByWeaponId(weaponId);
	if(!pstrDescription)
		return;

	hudhistory_t newhistory;
	newhistory.count = count;
	newhistory.description = pstrDescription;
	newhistory.die = cl_engfuncs.pfnGetClientTime() + HUD_HISTORY_DRAW_TIME;
	newhistory.hascount = (type == HISTORY_AMMO) ? true : false;

	m_historyList.add(newhistory);
}

//=============================================
// @brief
//
//=============================================
void CHUDHistory::AddElement( Uint32 type, const Char* pstrname, Uint32 count )
{
	if(type == HISTORY_AMMO && !count)
		return;

	const Char* pstrDescription = m_hud.GetItemDescriptionByName(pstrname);
	if(!pstrDescription)
		return;

	hudhistory_t newhistory;
	newhistory.count = count;
	newhistory.description = pstrDescription;
	newhistory.die = cl_engfuncs.pfnGetClientTime() + HUD_HISTORY_DRAW_TIME;
	newhistory.hascount = (type == HISTORY_AMMO) ? true : false;

	m_historyList.add(newhistory);
}

//=============================================
// @brief
//
//=============================================
bool CHUDHistory::HasActiveElements( void )
{
	return m_historyList.empty() ? false : true;
}

//=============================================
// @brief
//
//=============================================
bool CHUDHistory::Draw( void )
{
	if(!HasActiveElements())
		return true;

	Float height = 0;
	Float xcoord = gHUDDraw.ScaleXRelative(BASE_RESOLUTION_X - 50);
	Float ycoord = gHUDDraw.ScaleY(BASE_RESOLUTION_Y - 250 - MAX_HISTORY_HEIGHT);

	Double fltime = cl_engfuncs.pfnGetClientTime();
	const font_set_t* pset = cl_renderfuncs.pfnGetDefaultFontSet();

	m_historyList.begin();
	while(!m_historyList.end())
	{
		hudhistory_t& history = m_historyList.get();
		if(history.die <= fltime || height + pset->fontsize > MAX_HISTORY_HEIGHT)
		{
			m_historyList.remove(m_historyList.get_link());
			m_historyList.next();
			continue;
		}

		Float textwidth = 0;
		const Char* pstr = history.description.c_str();
		while(*pstr)
		{
			textwidth += pset->glyphs[*pstr].advancex;
			pstr++;
		}

		color32_t color = HISTORY_TEXT_COLOR;
		Double fadeBeginTime = history.die - HISTORY_FADE_TIME;
		if(fadeBeginTime <= fltime)
			color.a *= 1.0 - (fltime - fadeBeginTime) / HISTORY_FADE_TIME;

		Float textxcoord = xcoord - textwidth;
		if(!cl_renderfuncs.pfnDrawSimpleString(color, textxcoord, ycoord, history.description.c_str(), pset))
		{
			cl_engfuncs.pfnErrorPopup("Shader error: %s.", cl_renderfuncs.pfnGetStringDrawError());
			return false;
		}

		ycoord += pset->fontsize + HISTORY_SPACING;
		height += pset->fontsize + HISTORY_SPACING;

		m_historyList.next();
	}

	return true;
}
