/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "file.h"
#include "input.h"
#include "enginestate.h"

#include "texturemanager.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_text.h"

#include "uimanager.h"
#include "uielements.h"
#include "uisettingswindow.h"

#include "window.h"
#include "commands.h"

#include "r_basicdraw.h"
#include "r_main.h"
#include "system.h"
#include "file.h"
#include "console.h"
#include "texturemanager.h"
#include "config.h"
#include "cl_snd.h"

// Window description file
const Char CUISettingsWindow::WINDOW_DESC_FILE[] = "settingswindow.txt";
// Window description file
const Char CUISettingsWindow::WINDOW_OBJ_NAME[] = "SettingsWindow";
// Apply button object name
const Char CUISettingsWindow::APPLY_BUTTON_OBJ_NAME[] = "ApplyButton";
// Cancel button object name
const Char CUISettingsWindow::CANCEL_BUTTON_OBJ_NAME[] = "CancelButton";
// Settings tab object name
const Char CUISettingsWindow::SETTINGS_TAB_OBJ_NAME[] = "SettingsTab";
// Binds list tab object name
const Char CUISettingsWindow::BINDSTAB_LIST_OBJ_NAME[] = "BindsTabList";
// Binds list tab object name
const Char CUISettingsWindow::BINDSTAB_CLEAR_BTN_OBJ_NAME[] = "Bind_ClearButton";
// Binds list tab object name
const Char CUISettingsWindow::BINDSTAB_BIND_BTN_OBJ_NAME[] = "Bind_BindButton";
// Binds list tab object name
const Char CUISettingsWindow::BINDSTAB_RESTORE_DEFAULTS_BTN_OBJ_NAME[] = "Bind_RestoreDefaults";
// Video tab display device label object name
const Char CUISettingsWindow::VIDEOTAB_DISPLAY_DEVICE_LABEL_OBJ_NAME[] = "DisplayDeviceLabel";
// Video tab display device dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_DISPLAY_DEVICE_LIST_OBJ_NAME[] = "DisplayDeviceList";
// Video tab display device label object name
const Char CUISettingsWindow::VIDEOTAB_RESOLUTION_LABEL_OBJ_NAME[] = "DisplayResolutionLabel";
// Video tab display resolution dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_DISPLAY_RESOLUTION_LIST_OBJ_NAME[] = "DisplayResolutionList";
// Video tab display device label object name
const Char CUISettingsWindow::VIDEOTAB_WINDOW_MODE_LABEL_OBJ_NAME[] = "WindowModeLabel";
// Video tab display resolution dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_DISPLAY_WINDOWMODE_LIST_OBJ_NAME[] = "WindowModeList";
// Video tab anisotropy label object name
const Char CUISettingsWindow::VIDEOTAB_ANISOTROPY_LABEL_OBJ_NAME[] = "AnisotropyLabel";
// Video tab anisotropy dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_ANISOTROPY_LIST_OBJ_NAME[] = "AnisotropyList";
// Video tab antialias label object name
const Char CUISettingsWindow::VIDEOTAB_ANTIALIAS_LABEL_OBJ_NAME[] = "AntiAliasLabel";
// Video tab antialiasing dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_ANTIALIAS_LIST_OBJ_NAME[] = "AntiAliasList";
// Video tab vertical sync label object name
const Char CUISettingsWindow::VIDEOTAB_VERTICAL_SYNC_LABEL_OBJ_NAME[] = "VerticalSyncLabel";
// Video tab vertical sync dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_VERTICAL_SYNC_LIST_OBJ_NAME[] = "VerticalSyncList";
// Video tab display device label object name
const Char CUISettingsWindow::VIDEOTAB_GAMMA_LABEL_OBJ_NAME[] = "GammaLabel";
// Video tab display resolution dropdown list object name
const Char CUISettingsWindow::VIDEOTAB_GAMMA_SLIDER_OBJ_NAME[] = "GammaSlider";
// Advance tab list object name
const Char CUISettingsWindow::ADVANCED_TAB_LIST_OBJ_NAME[] = "AdvancedTabList";
// Advanced tab options list file
const Char CUISettingsWindow::ADVANCED_DESC_FILE[] = "advanced.txt";
// Advanced tab options list file
const Char CUISettingsWindow::SCROLLSURFACE_OPTION_LABEL_OBJ_NAME[] = "ScrollSurfaceOptionLabel";
// Advanced tab options list file
const Char CUISettingsWindow::SCROLLSURFACE_OPTION_LIST_OBJ_NAME[] = "ScrollSurfaceOptionList";
// Advanced tab options list file
const Char CUISettingsWindow::SCROLLSURFACE_OPTION_TAB_OBJ_NAME[] = "ScrollSurfaceOptionTab";
// Default bind file filename
const Char CUISettingsWindow::DEFAULT_BINDS_FILENAME[] = "default.cfg";
// "Reverse Mouse" tickbox object name
const Char CUISettingsWindow::MOUSE_TAB_REVERSE_MOUSE_BOX_OBJ_NAME[] = "ReverseMouseTickBox";
// "Reverse Mouse" label object name
const Char CUISettingsWindow::MOUSE_TAB_REVERSE_MOUSE_LABEL_OBJ_NAME[] = "ReverseMouseLabel";
// "Filter Mouse" tickbox object name
const Char CUISettingsWindow::MOUSE_TAB_FILTER_MOUSE_BOX_OBJ_NAME[] = "FilterMouseTickBox";
// "Filter Mouse" object name
const Char CUISettingsWindow::MOUSE_TAB_FILTER_MOUSE_LABEL_OBJ_NAME[] = "FilterMouseLabel";
// Auto-aim tickbox object name
const Char CUISettingsWindow::MOUSE_TAB_AUTOAIM_BOX_OBJ_NAME[] = "AutoAimTickBox";
// Auto-aim label object name
const Char CUISettingsWindow::MOUSE_TAB_AUTOAIM_LABEL_OBJ_NAME[] = "AutoAimLabel";
// Mouse sensitivity slider object name
const Char CUISettingsWindow::MOUSE_TAB_SENSITIVITY_SLIDER_OBJ_NAME[] = "MouseSensitivitySlider";
// Mouse sensitivity slider object name
const Char CUISettingsWindow::MOUSE_TAB_SENSITIVITY_LABEL_OBJ_NAME[] = "MouseSensitivityLabel";
// Mouse sensitivity value tab object name
const Char CUISettingsWindow::MOUSE_TAB_SENSITIVITY_TAB_OBJ_NAME[] = "MouseSensitivityTab";
// Mouse sensitivity value text object name
const Char CUISettingsWindow::MOUSE_TAB_SENSITIVITY_TEXT_OBJ_NAME[] = "MouseSensitivityText";
// Mouse filter frames slider object name
const Char CUISettingsWindow::MOUSE_TAB_FILTER_FRAMES_SLIDER_OBJ_NAME[] = "MouseFilterFramesSlider";
// Mouse filter frames slider object name
const Char CUISettingsWindow::MOUSE_TAB_FILTER_FRAMES_LABEL_OBJ_NAME[] = "MouseFilterFramesLabel";
// Mouse filter frames value tab object name
const Char CUISettingsWindow::MOUSE_TAB_FILTER_FRAMES_TAB_OBJ_NAME[] = "MouseFilterFramesTab";
// Mouse filter frames value text object name
const Char CUISettingsWindow::MOUSE_TAB_FILTER_FRAMES_TEXT_OBJ_NAME[] = "MouseFilterFramesText";
// Mouse sensitivity value tab object name
const Char CUISettingsWindow::AUDIO_TAB_MASTER_VOLUME_LABEL_OBJ_NAME[] = "MasterVolumeLabel";
// Mouse sensitivity value text object name
const Char CUISettingsWindow::AUDIO_TAB_MASTER_VOLUME_SLIDER_OBJ_NAME[] = "MasterVolumeSlider";
// Game volume label object name
const Char CUISettingsWindow::AUDIO_TAB_GAME_VOLUME_LABEL_OBJ_NAME[] = "GameVolumeLabel";
// Game volume slider object name
const Char CUISettingsWindow::AUDIO_TAB_GAME_VOLUME_SLIDER_OBJ_NAME[] = "GameVolumeSlider";
// Mouse sensitivity value tab object name
const Char CUISettingsWindow::AUDIO_TAB_MUSIC_VOLUME_LABEL_OBJ_NAME[] = "MusicVolumeLabel";
// Mouse sensitivity value text object name
const Char CUISettingsWindow::AUDIO_TAB_MUSIC_VOLUME_SLIDER_OBJ_NAME[] = "MusicVolumeSlider";
// Mouse sensitivity value text object name
const Char CUISettingsWindow::AUDIO_TAB_OCCLUSION_LABEL_OBJ_NAME[] = "SoundOcclusionLabel";
// Mouse sensitivity value text object name
const Char CUISettingsWindow::AUDIO_TAB_OCCLUSION_TICKBOX_OBJ_NAME[] = "SoundOcclusionTickBox";
// HRTF text object name
const Char CUISettingsWindow::AUDIO_TAB_HRTF_LABEL_OBJ_NAME[] = "HRTFLabel";
// HRTF tick box object name
const Char CUISettingsWindow::AUDIO_TAB_HRTF_TICKBOX_OBJ_NAME[] = "HRTFTickBox";
// Subtitles text object name
const Char CUISettingsWindow::AUDIO_TAB_SUBTITLES_LABEL_OBJ_NAME[] = "SubtitlesLabel";
// Subtitles tick box object name
const Char CUISettingsWindow::AUDIO_TAB_SUBTITLES_TICKBOX_OBJ_NAME[] = "SubtitlesTickBox";
// View bob slider object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWBOB_SLIDER_OBJ_NAME[] = "ViewBobSlider";
// View boby slider object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWBOB_LABEL_OBJ_NAME[] = "ViewBobLabel";
// View bob value tab object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWBOB_TAB_OBJ_NAME[] = "ViewBobTab";
// View bob value text object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWBOB_TEXT_OBJ_NAME[] = "ViewBobText";
// View roll slider object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWROLL_SLIDER_OBJ_NAME[] = "ViewRollSlider";
// View roll slider object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWROLL_LABEL_OBJ_NAME[] = "ViewRollLabel";
// View roll value tab object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWROLL_TAB_OBJ_NAME[] = "ViewRollTab";
// View roll value text object name
const Char CUISettingsWindow::GAMEPLAY_TAB_VIEWROLL_TEXT_OBJ_NAME[] = "ViewRollText";
// Gameplay tab list object name
const Char CUISettingsWindow::GAMEPLAY_TAB_LIST_OBJ_NAME[] = "GameplayTabList";
// Advanced tab options list file
const Char CUISettingsWindow::GAMEPLAY_DESC_FILE[] = "gameplay.txt";

// Binds file path
const Char CUISettingsWindow::BINDS_FILE_PATH[] = "scripts/keys/binds.txt";

// Current instance of the window
CUISettingsWindow* CUISettingsWindow::m_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CUISettingsWindow::CUISettingsWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIWindow(UIW_FL_MENUWINDOW, flags, width, height, originx, originy),
	m_pResolutionDropList(nullptr),
	m_pMSAADropList(nullptr),
	m_pBindsList(nullptr),
	m_pBindsListInfo(nullptr),
	m_isInBindMode(false),
	m_bindRowIndex(-1),
	m_selectedRowIndex(-1),
	m_bindsTabIndex(0),
	m_currentTabIndex(0),
	m_bReset(false),
	m_displayDeviceIndex(0),
	m_resolutionIndex(0),
	m_msaaIndex(0),
	m_bResetVideo(false),
	m_pSensitivityValueText(nullptr),
	m_pFilterFramesValueText(nullptr),
	m_pViewBobValueText(nullptr),
	m_pViewRollValueText(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUISettingsWindow::~CUISettingsWindow( void )
{
	// Probably destroyed by the UI manager
	if(m_pInstance)
		m_pInstance = nullptr;
}

//=============================================
// @brief Creates an instance of the console window
//
//=============================================
CUISettingsWindow* CUISettingsWindow::CreateInstance( void )
{
	if(m_pInstance)
		return m_pInstance;

	// Load the schema file
	ui_windowdescription_t* pWinDesc = gUIManager.LoadWindowDescriptionFile(WINDOW_OBJ_NAME, WINDOW_DESC_FILE);
	if(!pWinDesc)
	{
		Con_EPrintf("Failed to load window description '%s' for '%s'.\n", WINDOW_DESC_FILE, WINDOW_OBJ_NAME);
		return nullptr;
	}

	const ui_objectinfo_t* pWindowObject = pWinDesc->getObject(UI_OBJECT_WINDOW, WINDOW_OBJ_NAME);
	if(!pWindowObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, WINDOW_OBJ_NAME);
		return nullptr;
	}

	// Position to center of the screen
	Int32 xOrigin = gWindow.GetCenterX() - pWindowObject->getWidth()/2;
	Int32 yOrigin = gWindow.GetCenterY() - pWindowObject->getHeight()/2;

	// Create the main tab object
	CUISettingsWindow* pWindow = new CUISettingsWindow(pWindowObject->getFlags(), pWindowObject->getWidth(), pWindowObject->getHeight(), xOrigin, yOrigin);
	if(!pWindow->init(pWinDesc, pWindowObject))
	{
		Con_EPrintf("Failed to set schema '%s' for the settings window.\n", pWindowObject->getSchema().c_str());
		delete pWindow;
		return nullptr;
	}

	// Set pointer
	m_pInstance = pWindow;
	// Add to UI manager to handle
	gUIManager.AddWindow(pWindow);

	return m_pInstance;
}

//=============================================
// @brief Destroys the current instance of the console window
//
//=============================================
void CUISettingsWindow::DestroyInstance( void )
{
	if(!m_pInstance)
		return;

	m_pInstance->setWindowFlags(UIW_FL_KILLME);
	m_pInstance = nullptr;
}

//=============================================
// @brief Returns the current instance of the console window
//
//=============================================
CUISettingsWindow* CUISettingsWindow::GetInstance( void )
{
	return m_pInstance;
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUISettingsWindow::init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject )
{
	if(!CUIWindow::init(pWindowObject->getSchema().c_str()))
		return false;

	// Set the window's properties
	setTitle(pWindowObject->getTitle().c_str(), pWindowObject->getFont(), pWindowObject->getTitleXInset(), pWindowObject->getTitleYInset());
	setAlpha(pWindowObject->getAlpha());

	// Create the dragger
	CUIDragger* pDragger = new CUIDragger(UIEL_FL_FIXED_H, m_width-40, 30, 0, 0);
	pDragger->setParent(this);

	// Create the "Apply" button
	const ui_objectinfo_t* pApplyButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, APPLY_BUTTON_OBJ_NAME);
	if(!pApplyButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, APPLY_BUTTON_OBJ_NAME);
		return false;
	}

	CUISettingsApplyEvent* pApplyEvent = new CUISettingsApplyEvent(this);
	CUIButton* pApplyButton = new CUIButton(pApplyButtonObjectInfo->getFlags(), 
		pApplyButtonObjectInfo->getText().c_str(), 
		pApplyButtonObjectInfo->getFont(), pApplyEvent, 
		pApplyButtonObjectInfo->getWidth(), 
		pApplyButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pApplyButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pApplyButtonObjectInfo->getYOrigin());

	pApplyButton->setParent(this);
	
	if(!pApplyButton->init(pApplyButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for settings UI window.\n");
		return false;
	}

	// Create the "Cancel" button
	const ui_objectinfo_t* pCancelButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, CANCEL_BUTTON_OBJ_NAME);
	if(!pCancelButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, CANCEL_BUTTON_OBJ_NAME);
		return false;
	}

	CUISettingsCancelEvent* pCloseCallback = new CUISettingsCancelEvent(this);
	CUIButton* pCancelButton = new CUIButton(pCancelButtonObjectInfo->getFlags(), 
		pCancelButtonObjectInfo->getText().c_str(), 
		pCancelButtonObjectInfo->getFont(), pCloseCallback, 
		pCancelButtonObjectInfo->getWidth(), 
		pCancelButtonObjectInfo->getHeight(),
		pWindowObject->getXInset() + pCancelButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pCancelButtonObjectInfo->getYOrigin());

	pCancelButton->setParent(this);
	if(!pCancelButton->init(pCancelButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for settings UI window.\n");
		return false;
	}

	// Create the tabs list object
	const ui_objectinfo_t* pSettingsTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, SETTINGS_TAB_OBJ_NAME);
	if(!pSettingsTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SETTINGS_TAB_OBJ_NAME);
		return false;
	}

	CUITabSelectEvent* pSelectEvent = new CUITabSelectEvent(this);
	CUITabList* pTabList = new CUITabList(pSettingsTabObjectInfo->getFlags(), 
		pSelectEvent, 
		pSettingsTabObjectInfo->getFont(),
		pSettingsTabObjectInfo->getWidth(), 
		pSettingsTabObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pSettingsTabObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pSettingsTabObjectInfo->getYOrigin());

	pTabList->setParent(this);

	// Create the binds tab
	CUITabBody* pBindsBody = InitBindsTab(pTabList, pWinDesc, pSettingsTabObjectInfo);
	if(!pBindsBody)
		return false;

	// Create the mouse tab
	CUITabBody* pMouseTab = InitMouseTab(pTabList, pWinDesc, pSettingsTabObjectInfo);
	if(!pMouseTab)
		return false;

	// Create the audio tab
	CUITabBody* pAudioTab = InitAudioTab(pTabList, pWinDesc, pSettingsTabObjectInfo);
	if(!pAudioTab)
		return false;

	// Create the audio tab
	CUITabBody* pVideoTab = InitVideoTab(pTabList, pWinDesc, pSettingsTabObjectInfo);
	if(!pVideoTab)
		return false;

	// Create the gameplay settings tab
	CUITabBody* pGameplayTab = InitGameplayTab(pTabList, pWinDesc, pSettingsTabObjectInfo);
	if(!pGameplayTab)
		return false;

	// Create the advanced settings tab
	CUITabBody* pAdvancedTab = InitAdvancedTab(pTabList, pWinDesc, pSettingsTabObjectInfo);
	if(!pAdvancedTab)
		return false;

	return true;
}

//=============================================
// @brief Sets up the Video tab
//
//=============================================
CUITabBody* CUISettingsWindow::InitAdvancedTab( CUITabList* pTabList, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject )
{
	// Create the advanced settings tab
	CUITabBody* pAdvancedTab = pTabList->createTab("Advanced");
	if(!pAdvancedTab)
		return false;

	if(!LoadScrollableOptionsList(pAdvancedTab, pWinDesc, pTabObject, ADVANCED_TAB_LIST_OBJ_NAME, ADVANCED_DESC_FILE))
		return false;

	return pAdvancedTab;
}

//=============================================
// @brief Loads a scrollable options list
//
//=============================================
bool CUISettingsWindow::LoadScrollableOptionsList( CUITabBody* pTab, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject, const Char* pstrTabObjectName, const Char* pstrOptionsFilename )
{
	// Create the scrollable surface for the advanced options
	const ui_objectinfo_t* pTabListObjectInfo = pWinDesc->getObject(UI_OBJECT_LIST, pstrTabObjectName);
	if(!pTabListObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, ADVANCED_TAB_LIST_OBJ_NAME);
		return nullptr;
	}
	
	CUIScrollableSurface* pScrollSurface = new CUIScrollableSurface(pTabListObjectInfo->getFlags(), 
		pTabListObjectInfo->getWidth(), 
		pTabListObjectInfo->getHeight(), 
		pTabObject->getXInset() + pTabListObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pTabListObjectInfo->getYOrigin());

	pScrollSurface->setParent(pTab);
	if(!pScrollSurface->init(pTabListObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize 'CUIScrollableSurface' for the settings window.\n");
		return false;
	}

	// Where we need to start adding stuff from
	Uint32 startIndex = m_scrollSurfaceOptionsArray.size();

	// Open the file describing the options
	Uint32 iSize = 0;
	const byte* pOptionsFile = FL_LoadFile(pstrOptionsFilename, &iSize);

	// Parse the options
	const Char* pstr = reinterpret_cast<const Char*>(pOptionsFile);
	while(pstr && (pstr - reinterpret_cast<const Char*>(pOptionsFile)) < iSize)
	{
		// Parse the cvar name
		CString cvarname;
		pstr = Common::Parse(pstr, cvarname);
		if(!pstr || cvarname.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Next character should be a bracket
		CString token;
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		if(qstrcmp(token.c_str(), "{"))
		{
			Con_EPrintf("Expected '{', got '%s' on '%s'.\n", token.c_str(), ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Next one will be the option name
		CString optiondesc;
		pstr = Common::Parse(pstr, optiondesc);
		if(!pstr || optiondesc.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Next one will be another bracket
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		if(qstrcmp(token.c_str(), "{"))
		{
			Con_EPrintf("Expected '{', got '%s' on '%s'.\n", token.c_str(), ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Next one is the object type
		CString objecttype;
		pstr = Common::Parse(pstr, objecttype);
		if(!pstr || objecttype.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		scrollsurf_option_t newOption;
		newOption.cvarname = cvarname;
		newOption.optiondesc = optiondesc;

		if(!qstrcmp(objecttype, "UIDropDownList"))
		{
			// Set type
			newOption.type = UI_OBJECT_LIST;

			while(pstr && (pstr - reinterpret_cast<const Char*>(pOptionsFile)) < iSize)
			{
				// Read in the option name
				pstr = Common::Parse(pstr, token);
				if(!pstr || token.empty())
				{
					Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
					FL_FreeFile(pOptionsFile);
					return false;
				}

				// Break if we reached the end
				if(!qstrcmp(token, "}"))
					break;

				adv_choice_t newChoice;
				newChoice.name = token;

				// Read in the option value
				pstr = Common::Parse(pstr, token);
				if(!pstr || token.empty())
				{
					Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
					FL_FreeFile(pOptionsFile);
					return false;
				}

				// Add it
				newChoice.value = token;
				newOption.choices.push_back(newChoice);
			}
		}
		else if(!qstrcmp(objecttype, "UIInputTab"))
		{
			// Set type
			newOption.type = UI_OBJECT_TAB;

			// Read in the minimum value
			pstr = Common::Parse(pstr, token);
			if(!pstr || token.empty())
			{
				Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
				FL_FreeFile(pOptionsFile);
				return false;
			}

			if(!Common::IsNumber(token))
			{
				Con_EPrintf("Minimum value '%s' in '%s' is not a numerical value.\n", ADVANCED_DESC_FILE);
				FL_FreeFile(pOptionsFile);
				return false;
			}

			newOption.minvalue = SDL_atof(token.c_str());

			// Read in the maximum value
			pstr = Common::Parse(pstr, token);
			if(!pstr || token.empty())
			{
				Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
				FL_FreeFile(pOptionsFile);
				return false;
			}

			if(!Common::IsNumber(token))
			{
				Con_EPrintf("Maximum value '%s' in '%s' is not a numerical value.\n", ADVANCED_DESC_FILE);
				FL_FreeFile(pOptionsFile);
				return false;
			}

			newOption.maxvalue = SDL_atof(token.c_str());

			// Read in the maximum value
			pstr = Common::Parse(pstr, token);
			if(!pstr || token.empty())
			{
				Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
				FL_FreeFile(pOptionsFile);
				return false;
			}

			if(qstrcmp(token.c_str(), "}"))
			{
				Con_EPrintf("Expected '}', got '%s' on '%s'.\n", token.c_str(), ADVANCED_DESC_FILE);
				FL_FreeFile(pOptionsFile);
				return false;
			}
		}
		else
		{
			Con_EPrintf("Unknown object type '%s' for '%s'.\n", objecttype.c_str(), cvarname.c_str());
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Check for EOF
		if(!pstr)
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Now read the default value in
		// Next on will be another bracket
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		if(qstrcmp(token.c_str(), "{"))
		{
			Con_EPrintf("Expected '{', got '%s' on '%s'.\n", token.c_str(), ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Read the value in
		pstr = Common::Parse(pstr, newOption.defaultvalue);
		if(!pstr || newOption.defaultvalue.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Read closing token
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		if(qstrcmp(token.c_str(), "}"))
		{
			Con_EPrintf("Expected '}', got '%s' on '%s'.\n", token.c_str(), ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Now comes another closing token
		pstr = Common::Parse(pstr, token);
		if(token.empty())
		{
			Con_EPrintf("Unexpected EOF on '%s'.\n", ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		if(qstrcmp(token.c_str(), "}"))
		{
			Con_EPrintf("Expected '}', got '%s' on '%s'.\n", token.c_str(), ADVANCED_DESC_FILE);
			FL_FreeFile(pOptionsFile);
			return false;
		}

		// Add the new option
		m_scrollSurfaceOptionsArray.push_back(newOption);
	}
	FL_FreeFile(pOptionsFile);

	// Load object descriptions
	const ui_objectinfo_t* pScrollSurfaceOptionLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, SCROLLSURFACE_OPTION_LABEL_OBJ_NAME);
	if(!pScrollSurfaceOptionLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SCROLLSURFACE_OPTION_LABEL_OBJ_NAME);
		return nullptr;
	}

	const ui_objectinfo_t* pScrollSurfaceOptionListObjectInfo = pWinDesc->getObject(UI_OBJECT_LIST, SCROLLSURFACE_OPTION_LIST_OBJ_NAME);
	if(!pScrollSurfaceOptionListObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SCROLLSURFACE_OPTION_LIST_OBJ_NAME);
		return nullptr;
	}

	const ui_objectinfo_t* pScrollSurfaceOptionTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, SCROLLSURFACE_OPTION_TAB_OBJ_NAME);
	if(!pScrollSurfaceOptionTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SCROLLSURFACE_OPTION_TAB_OBJ_NAME);
		return nullptr;
	}

	// Add the new elements to the list
	Int32 yOffset = 0;
	Uint32 heightAdd = 0;

	for(Uint32 i = startIndex; i < m_scrollSurfaceOptionsArray.size(); i++)
	{
		scrollsurf_option_t* poption = &m_scrollSurfaceOptionsArray[i];

		// Create the object holding both elements
		CUIInteractiveObject* pRowObject = new CUIInteractiveObject(UIEL_FL_NONE, m_width, pScrollSurfaceOptionListObjectInfo->getListRowHeight(), 0, yOffset);
		pRowObject->setParent(pScrollSurface);

		m_scrollSurfaceOptionsRowObjects.push_back(pRowObject);

		// Add the frist column in
		Uint32 columnWidth = pScrollSurface->getWidth()/2;

		// Add the left row column object
		CUIObject* pLeftRowObject = new CUIObject(UIEL_FL_NONE, columnWidth, pScrollSurfaceOptionListObjectInfo->getListRowHeight(), 0, 0);
		pLeftRowObject->setParent(pRowObject);

		// Set the text
		CUIText* pText = new CUIText(pScrollSurfaceOptionLabelObjectInfo->getFlags(), 
			pScrollSurfaceOptionLabelObjectInfo->getFont(), 
			poption->optiondesc.c_str(), 
			pScrollSurfaceOptionLabelObjectInfo->getXOrigin(), 
			pScrollSurfaceOptionLabelObjectInfo->getYOrigin(), 
			columnWidth);

		pText->setParent(pLeftRowObject);

		// Add the right row column object
		CUIInteractiveObject* pRightRowObject = new CUIInteractiveObject(UIEL_FL_NONE, columnWidth, pScrollSurfaceOptionListObjectInfo->getListRowHeight(), columnWidth, 0);
		pRightRowObject->setParent(pRowObject);

		// Create the appropriate object
		if(poption->type == UI_OBJECT_LIST)
		{
			// Create the toggle event
			CUIScrollSurfaceDropListToggleEvent* pToggleEvent = new CUIScrollSurfaceDropListToggleEvent(this, i);
			CUIScrollSurfaceDropListSelectEvent* pSelectEvent = new CUIScrollSurfaceDropListSelectEvent(this, i);

			CUIDropDownList* pList = new CUIDropDownList(pScrollSurfaceOptionListObjectInfo->getFlags(), 
				pSelectEvent,
				pToggleEvent,
				pScrollSurfaceOptionListObjectInfo->getFont(), 
				pScrollSurfaceOptionListObjectInfo->getWidth(), 
				pScrollSurfaceOptionListObjectInfo->getHeight(), 
				pScrollSurfaceOptionListObjectInfo->getXOrigin(), 
				pScrollSurfaceOptionListObjectInfo->getYOrigin());

			pList->setParent(pRightRowObject);

			if(!pList->init(pScrollSurfaceOptionListObjectInfo->getSchema().c_str()))
			{
				Con_EPrintf("Failed to init schema '%s' for CUIDropDownList.\n", pScrollSurfaceOptionListObjectInfo->getSchema().c_str());
				return false;
			}

			// Add each choice
			for(Uint32 j = 0; j < poption->choices.size(); j++)
			{
				adv_choice_t& choice = poption->choices[j];
				pList->addChoice(choice.name.c_str());
			}

			// Look up the cvar value for this setting
			CCVar* pCVar = gConsole.GetCVar(poption->cvarname.c_str());
			if(pCVar)
			{
				if(pCVar->GetType() == CVAR_FLOAT)
				{
					// Retreive current value
					Float curValue = pCVar->GetValue();

					Uint32 j = 0;
					for(; j < poption->choices.size(); j++)
					{
						adv_choice_t& choice = poption->choices[j];

						// Convert string to float
						Float choiceValue = SDL_atof(choice.value.c_str());
						if(choiceValue == curValue)
						{
							pList->setSelection(j);
							break;
						}
					}

					if(j == poption->choices.size())
						pList->setSelection(-1);
				}
				else
				{
					CString curvalue = pCVar->GetStrValue();

					Uint32 j = 0;
					for(; j < poption->choices.size(); j++)
					{
						adv_choice_t& choice = poption->choices[j];

						// Convert string to float
						if(!qstrcmp(choice.value, curvalue))
						{
							pList->setSelection(j);
							break;
						}
					}

					if(j == poption->choices.size())
						pList->setSelection(-1);
				}
			}
			else
			{
				// Set it to empty
				pList->setSelection(-1);
			}

			if(i == (m_scrollSurfaceOptionsArray.size()-1))
				heightAdd += poption->choices.size() * pList->getHeight();
		}
		else if(poption->type == UI_OBJECT_TAB)
		{
			// Create the tab object
			CUITextInputTab* pInputTab = new CUITextInputTab(pScrollSurfaceOptionTabObjectInfo->getFlags(), 
				nullptr,
				pScrollSurfaceOptionTabObjectInfo->getFont(),
				pScrollSurfaceOptionTabObjectInfo->getWidth(),
				pScrollSurfaceOptionTabObjectInfo->getHeight(),
				pScrollSurfaceOptionTabObjectInfo->getXOrigin(), 
				pScrollSurfaceOptionTabObjectInfo->getYOrigin());

			pInputTab->setParent(pRightRowObject);

			if(!pInputTab->init(pScrollSurfaceOptionTabObjectInfo->getSchema().c_str()))
			{
				Con_EPrintf("Failed to init schema '%s' for CUITextInputTab.\n", pScrollSurfaceOptionTabObjectInfo->getSchema().c_str());
				return false;
			}

			// Look up the cvar value for this setting
			CCVar* pCVar = gConsole.GetCVar(poption->cvarname.c_str());
			if(pCVar)
			{
				if(pCVar->GetType() == CVAR_FLOAT)
				{
					// Retreive current value
					Float curValue = pCVar->GetValue();

					CString value;
					value << curValue;

					pInputTab->setText(value.c_str());
				}
				else
				{
					CString curvalue = pCVar->GetStrValue();
					pInputTab->setText(curvalue.c_str());
				}
			}
		}

		yOffset += pRowObject->getHeight();
	}

	if(heightAdd)
		pScrollSurface->setHeight(pScrollSurface->getHeight() + heightAdd);

	return true;
}

//=============================================
// @brief Sets the clicked advanced option to be on top
//
//=============================================
void CUISettingsWindow::SetAdvancedOptionFocus( Uint32 rowIndex, bool isOpen )
{
	assert(rowIndex >= 0 && rowIndex < m_scrollSurfaceOptionsRowObjects.size());
	CUIInteractiveObject* pObject = m_scrollSurfaceOptionsRowObjects[rowIndex];

	if(isOpen)
		pObject->setFlag(UIEL_FL_ONTOP);
	else
		pObject->removeFlag(UIEL_FL_ONTOP);
}

//=============================================
// @brief Sets up the Video tab
//
//=============================================
void CUISettingsWindow::AdvancedOptionSelect( Uint32 rowIndex, Uint32 selectionIndex )
{
	assert(rowIndex >= 0 && rowIndex < m_scrollSurfaceOptionsRowObjects.size());
	CUIInteractiveObject* pObject = m_scrollSurfaceOptionsRowObjects[rowIndex];

	// Get the drop-down box
	CUIInteractiveObject* pHolderObject = reinterpret_cast<CUIInteractiveObject*>(pObject->getChildByIndex(1));
	CUIDropDownList* pList = reinterpret_cast<CUIDropDownList*>(pHolderObject->getChildByIndex(0));
	pList->setSelection(selectionIndex);

	// Retreive the option
	assert(rowIndex >= 0 && rowIndex < m_scrollSurfaceOptionsArray.size());
	scrollsurf_option_t& option = m_scrollSurfaceOptionsArray[rowIndex];

	assert(selectionIndex >= 0 && selectionIndex < option.choices.size());
	const adv_choice_t& choice = option.choices[selectionIndex];

	CString cmd;
	cmd << option.cvarname << " " << choice.value;

	AddPendingSetting(option.cvarname.c_str(), cmd.c_str());
}

//=============================================
// @brief Sets up the Video tab
//
//=============================================
CUITabBody* CUISettingsWindow::InitVideoTab( CUITabList* pTabList, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject )
{
	// Create the binds tab
	CUITabBody* pVideoTab = pTabList->createTab("Video");
	if(!pVideoTab)
		return nullptr;

	// Create the label
	const ui_objectinfo_t* pDeviceLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_DISPLAY_DEVICE_LABEL_OBJ_NAME);
	if(!pDeviceLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_DEVICE_LABEL_OBJ_NAME);
		return false;
	}

	// Create the label
	CUIText* pDisplayLabel = new CUIText(pDeviceLabelObjectInfo->getFlags(), 
		pDeviceLabelObjectInfo->getFont(), 
		pDeviceLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pDeviceLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pDeviceLabelObjectInfo->getYOrigin());

	pDisplayLabel->setParent(pVideoTab);

	// Create the display device tab
	const ui_objectinfo_t* pDeviceListObject = pWinDesc->getObject(UI_OBJECT_LIST, VIDEOTAB_DISPLAY_DEVICE_LIST_OBJ_NAME);
	if(!pDeviceListObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_DEVICE_LIST_OBJ_NAME);
		return false;
	}

	CUIDeviceSelectEvent* pDeviceSelectEvent = new CUIDeviceSelectEvent(this);
	CUIDropDownList* pDeviceDropList = new CUIDropDownList(pDeviceListObject->getFlags(), 
		pDeviceSelectEvent, 
		nullptr,
		pDeviceListObject->getFont(),
		pDeviceListObject->getWidth(), 
		pDeviceListObject->getHeight(), 
		pTabObject->getXInset() + pDeviceListObject->getXOrigin(), 
		pTabObject->getYInset() + pDeviceListObject->getYOrigin());
	pDeviceDropList->setParent(pVideoTab);

	if(!pDeviceDropList->init(pDeviceListObject->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initiate 'CUIDropDownList' for 'CUISettingsWindow'.\n");
		return nullptr;
	}

	// Create the label
	const ui_objectinfo_t* pResolutionLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_RESOLUTION_LABEL_OBJ_NAME);
	if(!pDeviceLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_RESOLUTION_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pResolutionLabel = new CUIText(pResolutionLabelObjectInfo->getFlags(), 
		pResolutionLabelObjectInfo->getFont(), 
		pResolutionLabelObjectInfo->getText().c_str(), 
		pTabObject->getXInset() + pResolutionLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pResolutionLabelObjectInfo->getYOrigin());

	pResolutionLabel->setParent(pVideoTab);

	// Create the resolution tab
	const ui_objectinfo_t* pResolutionListObject = pWinDesc->getObject(UI_OBJECT_LIST, VIDEOTAB_DISPLAY_RESOLUTION_LIST_OBJ_NAME);
	if(!pDeviceListObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_RESOLUTION_LIST_OBJ_NAME);
		return false;
	}

	CUIResolutionSelectEvent* pResSelectEvent = new CUIResolutionSelectEvent(this);
	m_pResolutionDropList = new CUIDropDownList(pResolutionListObject->getFlags(), 
		pResSelectEvent, 
		nullptr,
		pResolutionListObject->getFont(),
		pResolutionListObject->getWidth(), 
		pResolutionListObject->getHeight(), 
		pTabObject->getXInset() + pResolutionListObject->getXOrigin(), 
		pTabObject->getYInset() + pResolutionListObject->getYOrigin());

	m_pResolutionDropList->setParent(pVideoTab);

	if(!m_pResolutionDropList->init(pResolutionListObject->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initiate 'CUIDropDownList' for 'CUISettingsWindow'.\n");
		return nullptr;
	}

	// Get current indexes for device and resolution
	m_displayDeviceIndex = gWindow.GetCurrentDeviceIndex();

	// Populate the list
	Int32 nbDevices = gWindow.GetNbDisplayDevices();
	for(Int32 i = 0; i < nbDevices; i++)
	{
		// Add the device to the list
		const Char* pstrDeviceName = gWindow.GetDisplayDeviceName(i);
		pDeviceDropList->addChoice(pstrDeviceName);

		if(m_displayDeviceIndex == i)
		{
			pDeviceDropList->setSelection(i);
			PopulateResolutions(i);
		}
	}

	// Create the label
	const ui_objectinfo_t* pWindowModeLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_WINDOW_MODE_LABEL_OBJ_NAME);
	if(!pWindowModeLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_WINDOW_MODE_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pModeLabel = new CUIText(pWindowModeLabelObjectInfo->getFlags(), 
		pWindowModeLabelObjectInfo->getFont(), 
		pWindowModeLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pWindowModeLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pWindowModeLabelObjectInfo->getYOrigin());

	pModeLabel->setParent(pVideoTab);

	// Create the mode list
	const ui_objectinfo_t* pWindowModeListObject = pWinDesc->getObject(UI_OBJECT_LIST, VIDEOTAB_DISPLAY_WINDOWMODE_LIST_OBJ_NAME);
	if(!pWindowModeListObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_WINDOWMODE_LIST_OBJ_NAME);
		return false;
	}

	CUIWindowModeSelectEvent* pModeSelectEvent = new CUIWindowModeSelectEvent(this);
	CUIDropDownList* pModeDropList = new CUIDropDownList(pWindowModeListObject->getFlags(), 
		pModeSelectEvent, 
		nullptr,
		pWindowModeListObject->getFont(),
		pWindowModeListObject->getWidth(), 
		pWindowModeListObject->getHeight(), 
		pTabObject->getXInset() + pWindowModeListObject->getXOrigin(), 
		pTabObject->getYInset() + pWindowModeListObject->getYOrigin());

	pModeDropList->setParent(pVideoTab);

	if(!pModeDropList->init(pWindowModeListObject->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initiate 'CUIDropDownList' for 'CUISettingsWindow'.\n");
		return nullptr;
	}

	// Add the modes
	pModeDropList->addChoice("Fullscreen");
	pModeDropList->addChoice("Windowed");

	if(!gWindow.IsFullScreen())
		pModeDropList->setSelection(1);
	else
		pModeDropList->setSelection(0);

	// Create the slider label
	const ui_objectinfo_t* pGammaLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_GAMMA_LABEL_OBJ_NAME);
	if(!pGammaLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_GAMMA_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pText = new CUIText(pGammaLabelObjectInfo->getFlags(),
		pGammaLabelObjectInfo->getFont(),
		pGammaLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pGammaLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pGammaLabelObjectInfo->getYOrigin());

	pText->setParent(pVideoTab);

	// Create the slider
	const ui_objectinfo_t* pGammaSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, VIDEOTAB_GAMMA_SLIDER_OBJ_NAME);
	if(!pGammaSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_GAMMA_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	CUISliderAdjustEvent* pEvent = new CUISliderAdjustEvent(this, GAMMA_CVAR_NAME);
	CUISlider* pSlider = new CUISlider(pGammaSliderObjectInfo->getFlags(),
		pEvent,
		pGammaSliderObjectInfo->getWidth(),
		pGammaSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pGammaSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pGammaSliderObjectInfo->getYOrigin(),
		pGammaSliderObjectInfo->getMinValue(),
		pGammaSliderObjectInfo->getMaxValue(),
		pGammaSliderObjectInfo->getMarkerDistance());

	pSlider->setParent(pVideoTab);

	if(!pSlider->init(pGammaSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	CCVar* pCVar = gConsole.GetCVar(GAMMA_CVAR_NAME);
	if(pCVar)
	{
		if(pCVar->GetType() != CVAR_FLOAT)
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", GAMMA_CVAR_NAME);
		else
		{
			Float value = pCVar->GetValue();
			pSlider->setValue(value);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", GAMMA_CVAR_NAME);
	}

	// Create the label
	const ui_objectinfo_t* pAnisotropyLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_ANISOTROPY_LABEL_OBJ_NAME);
	if(!pAnisotropyLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_ANISOTROPY_LABEL_OBJ_NAME);
		return false;
	}

	// Create the label
	CUIText* pAnisotropyLabel = new CUIText(pAnisotropyLabelObjectInfo->getFlags(), 
		pAnisotropyLabelObjectInfo->getFont(), 
		pAnisotropyLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pAnisotropyLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pAnisotropyLabelObjectInfo->getYOrigin());

	pAnisotropyLabel->setParent(pVideoTab);

	// Create the display device tab
	const ui_objectinfo_t* pAnisotropyListObject = pWinDesc->getObject(UI_OBJECT_LIST, VIDEOTAB_ANISOTROPY_LIST_OBJ_NAME);
	if(!pAnisotropyListObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_DEVICE_LIST_OBJ_NAME);
		return false;
	}

	CUIAnisotropySelectEvent* pAnisotropySelectEvent = new CUIAnisotropySelectEvent(this);
	CUIDropDownList* pAnisotropyDropList = new CUIDropDownList(pAnisotropyListObject->getFlags(), 
		pAnisotropySelectEvent, 
		nullptr,
		pAnisotropyListObject->getFont(),
		pAnisotropyListObject->getWidth(), 
		pAnisotropyListObject->getHeight(), 
		pTabObject->getXInset() + pAnisotropyListObject->getXOrigin(), 
		pTabObject->getYInset() + pAnisotropyListObject->getYOrigin());
	pAnisotropyDropList->setParent(pVideoTab);

	if(!pAnisotropyDropList->init(pAnisotropyListObject->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initiate 'CUIDropDownList' for 'CUISettingsWindow'.\n");
		return nullptr;
	}

	// Create the label
	const ui_objectinfo_t* pAntiAliasLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_ANTIALIAS_LABEL_OBJ_NAME);
	if(!pAntiAliasLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_ANTIALIAS_LABEL_OBJ_NAME);
		return false;
	}

	// Create the label
	CUIText* pAntiAliasLabel = new CUIText(pAntiAliasLabelObjectInfo->getFlags(), 
		pAntiAliasLabelObjectInfo->getFont(), 
		pAntiAliasLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pAntiAliasLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pAntiAliasLabelObjectInfo->getYOrigin());

	pAntiAliasLabel->setParent(pVideoTab);

	// Create the display device tab
	const ui_objectinfo_t* pAntiAliasListObject = pWinDesc->getObject(UI_OBJECT_LIST, VIDEOTAB_ANTIALIAS_LIST_OBJ_NAME);
	if(!pAntiAliasListObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_DEVICE_LIST_OBJ_NAME);
		return false;
	}

	CUIAntiAliasSelectEvent* pAntiAliasSelectEvent = new CUIAntiAliasSelectEvent(this);
	m_pMSAADropList = new CUIDropDownList(pAntiAliasListObject->getFlags(), 
		pAntiAliasSelectEvent, 
		nullptr,
		pAntiAliasListObject->getFont(),
		pAntiAliasListObject->getWidth(), 
		pAntiAliasListObject->getHeight(), 
		pTabObject->getXInset() + pAntiAliasListObject->getXOrigin(), 
		pTabObject->getYInset() + pAntiAliasListObject->getYOrigin());
	m_pMSAADropList->setParent(pVideoTab);

	if(!m_pMSAADropList->init(pWindowModeListObject->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initiate 'CUIDropDownList' for 'CUISettingsWindow'.\n");
		return nullptr;
	}

	// Populate the list
	PopulateMSAAList();

	// Create the label
	const ui_objectinfo_t* pVerticalSyncLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, VIDEOTAB_VERTICAL_SYNC_LABEL_OBJ_NAME);
	if(!pVerticalSyncLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_VERTICAL_SYNC_LABEL_OBJ_NAME);
		return false;
	}

	// Create the label
	CUIText* pVerticalSyncLabel = new CUIText(pVerticalSyncLabelObjectInfo->getFlags(), 
		pVerticalSyncLabelObjectInfo->getFont(), 
		pVerticalSyncLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pVerticalSyncLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pVerticalSyncLabelObjectInfo->getYOrigin());

	pVerticalSyncLabel->setParent(pVideoTab);

	// Create the display device tab
	const ui_objectinfo_t* pVerticalSyncListObject = pWinDesc->getObject(UI_OBJECT_LIST, VIDEOTAB_VERTICAL_SYNC_LIST_OBJ_NAME);
	if(!pVerticalSyncListObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, VIDEOTAB_DISPLAY_DEVICE_LIST_OBJ_NAME);
		return false;
	}

	CUIVerticalSyncSelectEvent* pVerticalSyncSelectEvent = new CUIVerticalSyncSelectEvent(this);
	CUIDropDownList* pVerticalSyncDropList = new CUIDropDownList(pVerticalSyncListObject->getFlags(), 
		pVerticalSyncSelectEvent, 
		nullptr,
		pVerticalSyncListObject->getFont(),
		pVerticalSyncListObject->getWidth(), 
		pVerticalSyncListObject->getHeight(), 
		pTabObject->getXInset() + pVerticalSyncListObject->getXOrigin(), 
		pTabObject->getYInset() + pVerticalSyncListObject->getYOrigin());
	pVerticalSyncDropList->setParent(pVideoTab);

	if(!pVerticalSyncDropList->init(pVerticalSyncListObject->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initiate 'CUIDropDownList' for 'CUISettingsWindow'.\n");
		return nullptr;
	}

	// Populate the vsync list
	pVerticalSyncDropList->addChoice("Disabled");
	pVerticalSyncDropList->addChoice("Enabled");

	// Get vertical sync setting
	if(gWindow.IsVerticalSyncEnabled())
		pVerticalSyncDropList->setSelection(1);
	else
		pVerticalSyncDropList->setSelection(0);

	// Populate the tab
	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	Uint32 nbAnisotropySettings = pTextureManager->GetNbAnisotropySettings();
	for(Uint32 i = 0; i < nbAnisotropySettings; i++)
	{
		Uint32 value = pTextureManager->GetAnisotropySettingValue(i);

		CString valueName;
		if(value == CTextureManager::ANISOTROPY_OFF_VALUE)
			valueName = "Trilinear";
		else
			valueName << "Anisotropic " << (Int32)value << "X";

		pAnisotropyDropList->addChoice(valueName.c_str());
	}

	// Set the selection
	pCVar = gConsole.GetCVar(ANISOTROPY_CVAR_NAME);
	if(pCVar)
	{
		if(pCVar->GetType() != CVAR_FLOAT)
			Con_EPrintf("CVar '%S' for 'CUISlider' object is not of float type.\n", ANISOTROPY_CVAR_NAME);
		else
		{
			Int32 cvarValue = pCVar->GetValue();
			pAnisotropyDropList->setSelection(cvarValue);
		}
	}
	else
	{
		Con_EPrintf("CVar '%S' for 'CUISlider' not found.\n", ANISOTROPY_CVAR_NAME);
	}

	return pVideoTab;
}

//=============================================
// @brief Sets up the Video tab
//
//=============================================
CUITabBody* CUISettingsWindow::InitAudioTab( CUITabList* pTabList, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject )
{
	// Create the tab
	CUITabBody* pAudioTab = pTabList->createTab("Audio");

	// Create the slider label
	const ui_objectinfo_t* pMasterVolumeLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, AUDIO_TAB_MASTER_VOLUME_LABEL_OBJ_NAME);
	if(!pMasterVolumeLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, AUDIO_TAB_MASTER_VOLUME_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pText = new CUIText(pMasterVolumeLabelObjectInfo->getFlags(),
		pMasterVolumeLabelObjectInfo->getFont(),
		pMasterVolumeLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pMasterVolumeLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pMasterVolumeLabelObjectInfo->getYOrigin());

	pText->setParent(pAudioTab);

	// Create the slider
	const ui_objectinfo_t* pMasterVolumeSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, AUDIO_TAB_MASTER_VOLUME_SLIDER_OBJ_NAME);
	if(!pMasterVolumeSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, AUDIO_TAB_MASTER_VOLUME_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	CUISliderAdjustEvent* pEvent = new CUISliderAdjustEvent(this, VOLUME_CVAR_NAME);
	CUISlider* pSlider = new CUISlider(pMasterVolumeSliderObjectInfo->getFlags(),
		pEvent,
		pMasterVolumeSliderObjectInfo->getWidth(),
		pMasterVolumeSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pMasterVolumeSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pMasterVolumeSliderObjectInfo->getYOrigin(),
		pMasterVolumeSliderObjectInfo->getMinValue(),
		pMasterVolumeSliderObjectInfo->getMaxValue(),
		pMasterVolumeSliderObjectInfo->getMarkerDistance());

	pSlider->setParent(pAudioTab);

	if(!pSlider->init(pMasterVolumeSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	CCVar* pCVar = gConsole.GetCVar(VOLUME_CVAR_NAME);
	if(pCVar)
	{
		if(pCVar->GetType() != CVAR_FLOAT)
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", VOLUME_CVAR_NAME);
		else
		{
			Float value = pCVar->GetValue();
			pSlider->setValue(value);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", VOLUME_CVAR_NAME);
	}

	// Create the slider label
	const ui_objectinfo_t* pGameVolumeLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, AUDIO_TAB_GAME_VOLUME_LABEL_OBJ_NAME);
	if(!pGameVolumeLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, AUDIO_TAB_GAME_VOLUME_LABEL_OBJ_NAME);
		return false;
	}

	pText = new CUIText(pGameVolumeLabelObjectInfo->getFlags(),
		pGameVolumeLabelObjectInfo->getFont(),
		pGameVolumeLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pGameVolumeLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pGameVolumeLabelObjectInfo->getYOrigin());

	pText->setParent(pAudioTab);

	// Create the slider
	const ui_objectinfo_t* pGameVolumeSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, AUDIO_TAB_GAME_VOLUME_SLIDER_OBJ_NAME);
	if(!pGameVolumeSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, AUDIO_TAB_GAME_VOLUME_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	pEvent = new CUISliderAdjustEvent(this, GAME_VOLUME_CVAR_NAME);
	pSlider = new CUISlider(pGameVolumeSliderObjectInfo->getFlags(),
		pEvent,
		pGameVolumeSliderObjectInfo->getWidth(),
		pGameVolumeSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pGameVolumeSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pGameVolumeSliderObjectInfo->getYOrigin(),
		pGameVolumeSliderObjectInfo->getMinValue(),
		pGameVolumeSliderObjectInfo->getMaxValue(),
		pGameVolumeSliderObjectInfo->getMarkerDistance());

	pSlider->setParent(pAudioTab);

	if(!pSlider->init(pGameVolumeSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	pCVar = gConsole.GetCVar(GAME_VOLUME_CVAR_NAME);
	if(pCVar)
	{
		if(pCVar->GetType() != CVAR_FLOAT)
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", VOLUME_CVAR_NAME);
		else
		{
			Float value = pCVar->GetValue();
			pSlider->setValue(value);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", VOLUME_CVAR_NAME);
	}

	// Create the slider label
	const ui_objectinfo_t* pMusicVolumeLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, AUDIO_TAB_MUSIC_VOLUME_LABEL_OBJ_NAME);
	if(!pMusicVolumeLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, AUDIO_TAB_MUSIC_VOLUME_LABEL_OBJ_NAME);
		return false;
	}

	pText = new CUIText(pMusicVolumeLabelObjectInfo->getFlags(),
		pMusicVolumeLabelObjectInfo->getFont(),
		pMusicVolumeLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pMusicVolumeLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pMusicVolumeLabelObjectInfo->getYOrigin());

	pText->setParent(pAudioTab);

	// Create the slider
	const ui_objectinfo_t* pMusicVolumeSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, AUDIO_TAB_MUSIC_VOLUME_SLIDER_OBJ_NAME);
	if(!pMusicVolumeSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, AUDIO_TAB_MUSIC_VOLUME_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	pEvent = new CUISliderAdjustEvent(this, MUSIC_VOLUME_CVAR_NAME);
	pSlider = new CUISlider(pMusicVolumeSliderObjectInfo->getFlags(),
		pEvent,
		pMusicVolumeSliderObjectInfo->getWidth(),
		pMusicVolumeSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pMusicVolumeSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pMusicVolumeSliderObjectInfo->getYOrigin(),
		pMusicVolumeSliderObjectInfo->getMinValue(),
		pMusicVolumeSliderObjectInfo->getMaxValue(),
		pMusicVolumeSliderObjectInfo->getMarkerDistance());

	pSlider->setParent(pAudioTab);

	if(!pSlider->init(pMusicVolumeSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	pCVar = gConsole.GetCVar(MUSIC_VOLUME_CVAR_NAME);
	if(pCVar)
	{
		if(pCVar->GetType() != CVAR_FLOAT)
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", MUSIC_VOLUME_CVAR_NAME);
		else
		{
			Float value = pCVar->GetValue();
			pSlider->setValue(value);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", MUSIC_VOLUME_CVAR_NAME);
	}

	// Add tick box for occlusion
	if(!AddTickBox(pAudioTab, pWinDesc, pTabObject, AUDIO_TAB_OCCLUSION_TICKBOX_OBJ_NAME, AUDIO_TAB_OCCLUSION_LABEL_OBJ_NAME, "s_occlusion"))
	{
		Con_EPrintf("Failed to create tick box for settings window.\n");
		return nullptr;
	}

	// Add tick box for hrtf
	if(!AddTickBox(pAudioTab, pWinDesc, pTabObject, AUDIO_TAB_HRTF_TICKBOX_OBJ_NAME, AUDIO_TAB_HRTF_LABEL_OBJ_NAME, "s_sethrtf", CSoundEngine::SOUNDENGINE_CONFIG_GRP_NAME, CSoundEngine::SOUNDENGINE_HRTF_SETTING_NAME))
	{
		Con_EPrintf("Failed to create tick box for settings window.\n");
		return nullptr;
	}

	// Add tick box for subtitles
	if(!AddTickBox(pAudioTab, pWinDesc, pTabObject, AUDIO_TAB_SUBTITLES_TICKBOX_OBJ_NAME, AUDIO_TAB_SUBTITLES_LABEL_OBJ_NAME, "r_subtitles"))
	{
		Con_EPrintf("Failed to create tick box for settings window.\n");
		return nullptr;
	}

	return pAudioTab;
}

//=============================================
// @brief Populates the resolution list
//
//=============================================
void CUISettingsWindow::PopulateResolutions( Int32 deviceIndex )
{
	if(m_pResolutionDropList->getListSize())
		m_pResolutionDropList->clearList();

	// Get the current index
	m_resolutionIndex = -1;
	if(deviceIndex == gWindow.GetCurrentDeviceIndex())
		m_resolutionIndex = gWindow.GetCurrentResolutionIndex();

	// Get the number of resolutions
	Int32 nbResolutions = gWindow.GetNbResolutions(deviceIndex);
	for(Int32 i = 0; i < nbResolutions; i++)
	{
		Uint32 width, height;
		gWindow.GetResolutionInfo(deviceIndex, i, width, height);

		CString strResChoice;
		strResChoice << (Int32)width << "x" << (Int32)height;

		m_pResolutionDropList->addChoice(strResChoice.c_str());

		// Manage if we switched devices
		if(m_resolutionIndex == -1)
			SelectResolution(i);
		else if(m_resolutionIndex == i)
			m_pResolutionDropList->setSelection(i);
	}
}

//=============================================
// @brief Populates the MSAA list
//
//=============================================
void CUISettingsWindow::PopulateMSAAList( void )
{
	// Get MSAA setting
	m_msaaIndex = gWindow.GetCurrentMSAASetting();

	if(m_pMSAADropList->getListSize())
		m_pMSAADropList->clearList();

	// Populate MSAA tab
	Int32 nbMSAASettings = gWindow.GetNbMSAASettings();
	for(Int32 i = 0; i < nbMSAASettings; i++)
	{
		Uint32 msaaValue = gWindow.GetMSAASetting(i);

		CString strChoice;
		if(i == 0)
			strChoice << "Disabled";
		else
			strChoice << msaaValue << "x MSAA";

		m_pMSAADropList->addChoice(strChoice.c_str());

		if(m_msaaIndex == i)
			m_pMSAADropList->setSelection(i);
	}
}

//=============================================
// @brief Sets up the Binds tab
//
//=============================================
CUITabBody* CUISettingsWindow::InitBindsTab( CUITabList* pTabList, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject )
{
	m_pBindsListInfo = pWinDesc->getObject(UI_OBJECT_LIST, BINDSTAB_LIST_OBJ_NAME);
	if(!m_pBindsListInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, BINDSTAB_LIST_OBJ_NAME);
		return nullptr;
	}

	// Get the tab index
	m_bindsTabIndex = pTabList->getNbTabs();

	// Create the binds tab
	CUITabBody* pBindsTab = pTabList->createTab("Binds");
	if(!pBindsTab)
		return nullptr;
	
	m_pBindsList = new CUIList(m_pBindsListInfo->getFlags(),
		m_pBindsListInfo->getFont(),
		m_pBindsListInfo->getListRowHeight(), 
		2,
		m_pBindsListInfo->getWidth(), 
		m_pBindsListInfo->getHeight(), 
		pTabObject->getXInset() + m_pBindsListInfo->getXOrigin(), 
		pTabObject->getYInset() + m_pBindsListInfo->getYOrigin());

	m_pBindsList->setParent(pBindsTab);
	if(!m_pBindsList->init(m_pBindsListInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize 'CUIScrollableSurface' for the settings window.\n");
		return nullptr;
	}

	m_pBindsList->setHeaderColumnName(0, "Action");
	m_pBindsList->setHeaderColumnName(1, "Key");

	// Load the binds
	LoadBindsList();

	// Create the "Clear" button
	const ui_objectinfo_t* pBindClearKeyObjInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, BINDSTAB_CLEAR_BTN_OBJ_NAME);
	if(!pBindClearKeyObjInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, BINDSTAB_CLEAR_BTN_OBJ_NAME);
		return false;
	}

	CUIBindsClearBtnEvent* pClearEvent = new CUIBindsClearBtnEvent(this);
	CUIButton* pClearButton = new CUIButton(pBindClearKeyObjInfo->getFlags(), 
		pBindClearKeyObjInfo->getText().c_str(), 
		pBindClearKeyObjInfo->getFont(), pClearEvent, 
		pBindClearKeyObjInfo->getWidth(), 
		pBindClearKeyObjInfo->getHeight(),
		pTabObject->getXInset() + pBindClearKeyObjInfo->getXOrigin(), 
		pTabObject->getYInset() + pBindClearKeyObjInfo->getYOrigin());

	pClearButton->setParent(pBindsTab);

	if(!pClearButton->init(pBindClearKeyObjInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for settings UI window.\n");
		return false;
	}

	// Create the Bind Key button
	const ui_objectinfo_t* pBindBindButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, BINDSTAB_BIND_BTN_OBJ_NAME);
	if(!pBindBindButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, BINDSTAB_BIND_BTN_OBJ_NAME);
		return false;
	}

	CUIBindsBindBtnEvent* pBindEvent = new CUIBindsBindBtnEvent(this);
	CUIButton* pBindButton = new CUIButton(pBindBindButtonObjectInfo->getFlags(), 
		pBindBindButtonObjectInfo->getText().c_str(), 
		pBindBindButtonObjectInfo->getFont(), 
		pBindEvent, 
		pBindBindButtonObjectInfo->getWidth(), 
		pBindBindButtonObjectInfo->getHeight(),
		pTabObject->getXInset() + pBindBindButtonObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pBindBindButtonObjectInfo->getYOrigin());

	pBindButton->setParent(pBindsTab);

	if(!pBindButton->init(pBindBindButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for settings UI window.\n");
		return false;
	}

	// Create the "Restore Defaults" button
	const ui_objectinfo_t* pBindRestoreDefaultsButtonInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, BINDSTAB_RESTORE_DEFAULTS_BTN_OBJ_NAME);
	if(!pBindRestoreDefaultsButtonInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, BINDSTAB_RESTORE_DEFAULTS_BTN_OBJ_NAME);
		return false;
	}

	CUIBindsRestoreButtonEvent* pRestoreButtonEvent = new CUIBindsRestoreButtonEvent(this);
	CUIButton* pRestoreButton = new CUIButton(pBindRestoreDefaultsButtonInfo->getFlags(), 
		pBindRestoreDefaultsButtonInfo->getText().c_str(), 
		pBindRestoreDefaultsButtonInfo->getFont(), 
		pRestoreButtonEvent, 
		pBindRestoreDefaultsButtonInfo->getWidth(), 
		pBindRestoreDefaultsButtonInfo->getHeight(),
		pTabObject->getXInset() + pBindRestoreDefaultsButtonInfo->getXOrigin(), 
		pTabObject->getYInset() + pBindRestoreDefaultsButtonInfo->getYOrigin());

	pRestoreButton->setParent(pBindsTab);

	if(!pRestoreButton->init(pBindRestoreDefaultsButtonInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for settings UI window.\n");
		return false;
	}

	return pBindsTab;
}

//=============================================
// @brief Sets up the Binds tab
//
//=============================================
CUITabBody* CUISettingsWindow::InitMouseTab( CUITabList* pTabList, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject )
{
	// Create the tab
	CUITabBody* pMouseTab = pTabList->createTab("Mouse");

	// Add tick box for Reverse mouse
	if(!AddTickBox(pMouseTab, pWinDesc, pTabObject, MOUSE_TAB_REVERSE_MOUSE_BOX_OBJ_NAME, MOUSE_TAB_REVERSE_MOUSE_LABEL_OBJ_NAME, MOUSE_REVERSE_CVAR_NAME))
	{
		Con_EPrintf("Failed to create tick box for settings window.\n");
		return nullptr;
	}

	// Add tick box for Filter mouse
	if(!AddTickBox(pMouseTab, pWinDesc, pTabObject, MOUSE_TAB_FILTER_MOUSE_BOX_OBJ_NAME, MOUSE_TAB_FILTER_MOUSE_LABEL_OBJ_NAME, MOUSE_FILTER_CVAR_NAME))
	{
		Con_EPrintf("Failed to create tick box for settings window.\n");
		return nullptr;
	}
	
	// Add tick box for auto-aim
	if(!AddTickBox(pMouseTab, pWinDesc, pTabObject, MOUSE_TAB_AUTOAIM_BOX_OBJ_NAME, MOUSE_TAB_AUTOAIM_LABEL_OBJ_NAME, AUTOAIM_CVAR_NAME))
	{
		Con_EPrintf("Failed to create tick box for settings window.\n");
		return nullptr;
	}

	// Create the slider
	const ui_objectinfo_t* pSensitivitySliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, MOUSE_TAB_SENSITIVITY_SLIDER_OBJ_NAME);
	if(!pSensitivitySliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_SENSITIVITY_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	CUISliderAdjustEvent* pSensitivityEvent = new CUISliderAdjustEvent(this, MOUSE_SENSITIVITY_CVAR_NAME);
	CUISlider* pSensitivitySlider = new CUISlider(pSensitivitySliderObjectInfo->getFlags(),
		pSensitivityEvent,
		pSensitivitySliderObjectInfo->getWidth(),
		pSensitivitySliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pSensitivitySliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pSensitivitySliderObjectInfo->getYOrigin(),
		pSensitivitySliderObjectInfo->getMinValue(),
		pSensitivitySliderObjectInfo->getMaxValue(),
		pSensitivitySliderObjectInfo->getMarkerDistance());

	pSensitivitySlider->setParent(pMouseTab);

	if(!pSensitivitySlider->init(pSensitivitySliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	// Create the slider label
	const ui_objectinfo_t* pSensitivityLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, MOUSE_TAB_SENSITIVITY_LABEL_OBJ_NAME);
	if(!pSensitivityLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_SENSITIVITY_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pSensitivityText = new CUIText(pSensitivityLabelObjectInfo->getFlags(),
		pSensitivityLabelObjectInfo->getFont(),
		pSensitivityLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pSensitivityLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pSensitivityLabelObjectInfo->getYOrigin());

	pSensitivityText->setParent(pMouseTab);

	// Create the display for the value
	const ui_objectinfo_t* pSensitivityTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, MOUSE_TAB_SENSITIVITY_TAB_OBJ_NAME);
	if(!pSensitivityTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_SENSITIVITY_TAB_OBJ_NAME);
		return false;
	}

	CUISurface* pSensitivitySurface = new CUISurface(pSensitivityTabObjectInfo->getFlags(),
		pSensitivityTabObjectInfo->getWidth(),
		pSensitivityTabObjectInfo->getHeight(),
		pTabObject->getXInset() + pSensitivityTabObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pSensitivityTabObjectInfo->getYOrigin());

	pSensitivitySurface->setParent(pMouseTab);

	if(!pSensitivitySurface->init(pSensitivityTabObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize tab object for settings UI window.\n");
		return false;
	}

	// Create the display for the value
	const ui_objectinfo_t* pSensitivityTextObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, MOUSE_TAB_SENSITIVITY_TEXT_OBJ_NAME);
	if(!pSensitivityTextObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_SENSITIVITY_TEXT_OBJ_NAME);
		return false;
	}

	m_pSensitivityValueText = new CUIText(pSensitivityTextObjectInfo->getFlags(), 
		pSensitivityTabObjectInfo->getFont(),
		"",
		pSensitivityTabObjectInfo->getXInset() + pSensitivityTextObjectInfo->getXOrigin(),
		pSensitivityTabObjectInfo->getYInset() + pSensitivityTextObjectInfo->getYOrigin());

	m_pSensitivityValueText->setParent(pSensitivitySurface);

	CCVar* pSensitivityCVar = gConsole.GetCVar(MOUSE_SENSITIVITY_CVAR_NAME);
	if(pSensitivityCVar)
	{
		if(pSensitivityCVar->GetType() != CVAR_FLOAT)
		{
			m_pSensitivityValueText->setText("");
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", MOUSE_SENSITIVITY_CVAR_NAME);
		}
		else
		{
			Float value = pSensitivityCVar->GetValue();
			pSensitivitySlider->setValue(value);

			Char szValue[64];
			sprintf(szValue, "%0.1f", value);

			m_pSensitivityValueText->setText(szValue);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", MOUSE_SENSITIVITY_CVAR_NAME);
	}

	// Create the slider
	const ui_objectinfo_t* pFilterFramesSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, MOUSE_TAB_FILTER_FRAMES_SLIDER_OBJ_NAME);
	if(!pFilterFramesSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_FILTER_FRAMES_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	CUISliderAdjustEvent* pFilterFramesEvent = new CUISliderAdjustEvent(this, MOUSE_FILTER_FRAMES_CVAR_NAME, true);
	CUISlider* pFilterFramesSlider = new CUISlider(pFilterFramesSliderObjectInfo->getFlags(),
		pFilterFramesEvent,
		pFilterFramesSliderObjectInfo->getWidth(),
		pFilterFramesSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pFilterFramesSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pFilterFramesSliderObjectInfo->getYOrigin(),
		pFilterFramesSliderObjectInfo->getMinValue(),
		pFilterFramesSliderObjectInfo->getMaxValue(),
		pFilterFramesSliderObjectInfo->getMarkerDistance());

	pFilterFramesSlider->setParent(pMouseTab);

	if(!pFilterFramesSlider->init(pFilterFramesSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	// Create the slider label
	const ui_objectinfo_t* pFilterFramesLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, MOUSE_TAB_FILTER_FRAMES_LABEL_OBJ_NAME);
	if(!pFilterFramesLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_FILTER_FRAMES_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pFilterFramesText = new CUIText(pFilterFramesLabelObjectInfo->getFlags(),
		pFilterFramesLabelObjectInfo->getFont(),
		pFilterFramesLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pFilterFramesLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pFilterFramesLabelObjectInfo->getYOrigin());

	pFilterFramesText->setParent(pMouseTab);

	// Create the display for the value
	const ui_objectinfo_t* pFilterFramesTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, MOUSE_TAB_FILTER_FRAMES_TAB_OBJ_NAME);
	if(!pFilterFramesTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_FILTER_FRAMES_TAB_OBJ_NAME);
		return false;
	}

	CUISurface* pFilterFramesSurface = new CUISurface(pFilterFramesTabObjectInfo->getFlags(),
		pFilterFramesTabObjectInfo->getWidth(),
		pFilterFramesTabObjectInfo->getHeight(),
		pTabObject->getXInset() + pFilterFramesTabObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pFilterFramesTabObjectInfo->getYOrigin());

	pFilterFramesSurface->setParent(pMouseTab);

	if(!pFilterFramesSurface->init(pFilterFramesTabObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize tab object for settings UI window.\n");
		return false;
	}

	// Create the display for the value
	const ui_objectinfo_t* pFilterFramesTextObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, MOUSE_TAB_FILTER_FRAMES_TEXT_OBJ_NAME);
	if(!pFilterFramesTextObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, MOUSE_TAB_FILTER_FRAMES_TEXT_OBJ_NAME);
		return false;
	}

	m_pFilterFramesValueText = new CUIText(pFilterFramesTextObjectInfo->getFlags(), 
		pFilterFramesTabObjectInfo->getFont(),
		"",
		pFilterFramesTabObjectInfo->getXInset() + pFilterFramesTextObjectInfo->getXOrigin(),
		pFilterFramesTabObjectInfo->getYInset() + pFilterFramesTextObjectInfo->getYOrigin());

	m_pFilterFramesValueText->setParent(pFilterFramesSurface);

	CCVar* pFilterFramesCVar = gConsole.GetCVar(MOUSE_FILTER_FRAMES_CVAR_NAME);
	if(pFilterFramesCVar)
	{
		if(pFilterFramesCVar->GetType() != CVAR_FLOAT)
		{
			m_pFilterFramesValueText->setText("");
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", MOUSE_FILTER_FRAMES_CVAR_NAME);
		}
		else
		{
			Float value = pFilterFramesCVar->GetValue();
			pFilterFramesSlider->setValue(value);

			Char szValue[64];
			sprintf(szValue, "%d", (Int32)value);

			m_pFilterFramesValueText->setText(szValue);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", MOUSE_FILTER_FRAMES_CVAR_NAME);
	}

	return pMouseTab;
}

//=============================================
// @brief Sets up the Binds tab
//
//=============================================
CUITabBody* CUISettingsWindow::InitGameplayTab( CUITabList* pTabList, const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pTabObject )
{
	// Create the tab
	CUITabBody* pGameplayTab = pTabList->createTab("Gameplay");

	// Create the slider
	const ui_objectinfo_t* pViewBobSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, GAMEPLAY_TAB_VIEWBOB_SLIDER_OBJ_NAME);
	if(!pViewBobSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWBOB_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	CUISliderAdjustEvent* pViewBobSliderEvent = new CUISliderAdjustEvent(this, VIEW_BOB_CVAR_NAME);
	CUISlider* pViewBobSlider = new CUISlider(pViewBobSliderObjectInfo->getFlags(),
		pViewBobSliderEvent,
		pViewBobSliderObjectInfo->getWidth(),
		pViewBobSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pViewBobSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pViewBobSliderObjectInfo->getYOrigin(),
		pViewBobSliderObjectInfo->getMinValue(),
		pViewBobSliderObjectInfo->getMaxValue(),
		pViewBobSliderObjectInfo->getMarkerDistance());

	pViewBobSlider->setParent(pGameplayTab);

	if(!pViewBobSlider->init(pViewBobSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	// Create the slider label
	const ui_objectinfo_t* pViewBobLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, GAMEPLAY_TAB_VIEWBOB_LABEL_OBJ_NAME);
	if(!pViewBobLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWBOB_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pViewBobText = new CUIText(pViewBobLabelObjectInfo->getFlags(),
		pViewBobLabelObjectInfo->getFont(),
		pViewBobLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pViewBobLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pViewBobLabelObjectInfo->getYOrigin());

	pViewBobText->setParent(pGameplayTab);

	// Create the display for the value
	const ui_objectinfo_t* pViewBobTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, GAMEPLAY_TAB_VIEWBOB_TAB_OBJ_NAME);
	if(!pViewBobTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWBOB_TAB_OBJ_NAME);
		return false;
	}

	CUISurface* pViewBobSurface = new CUISurface(pViewBobTabObjectInfo->getFlags(),
		pViewBobTabObjectInfo->getWidth(),
		pViewBobTabObjectInfo->getHeight(),
		pTabObject->getXInset() + pViewBobTabObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pViewBobTabObjectInfo->getYOrigin());

	pViewBobSurface->setParent(pGameplayTab);

	if(!pViewBobSurface->init(pViewBobTabObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize tab object for settings UI window.\n");
		return false;
	}

	// Create the text
	const ui_objectinfo_t* pViewBobTextObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, GAMEPLAY_TAB_VIEWBOB_TEXT_OBJ_NAME);
	if(!pViewBobTextObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWBOB_TEXT_OBJ_NAME);
		return false;
	}

	m_pViewBobValueText = new CUIText(pViewBobTextObjectInfo->getFlags(), 
		pViewBobTabObjectInfo->getFont(),
		"",
		pViewBobTabObjectInfo->getXInset() + pViewBobTextObjectInfo->getXOrigin(),
		pViewBobTabObjectInfo->getYInset() + pViewBobTextObjectInfo->getYOrigin());

	m_pViewBobValueText->setParent(pViewBobSurface);

	CCVar* pViewBobCVar = gConsole.GetCVar(VIEW_BOB_CVAR_NAME);
	if(pViewBobCVar)
	{
		if(pViewBobCVar->GetType() != CVAR_FLOAT)
		{
			m_pViewBobValueText->setText("");
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", VIEW_BOB_CVAR_NAME);
		}
		else
		{
			Float value = pViewBobCVar->GetValue();
			pViewBobSlider->setValue(value);

			Char szValue[64];
			sprintf(szValue, "%0.1f", value);

			m_pViewBobValueText->setText(szValue);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", VIEW_BOB_CVAR_NAME);
	}

	// Create the slider
	const ui_objectinfo_t* pViewRollSliderObjectInfo = pWinDesc->getObject(UI_OBJECT_SLIDER, GAMEPLAY_TAB_VIEWROLL_SLIDER_OBJ_NAME);
	if(!pViewRollSliderObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWROLL_SLIDER_OBJ_NAME);
		return false;
	}

	// Create object
	CUISliderAdjustEvent* pViewRollSliderEvent = new CUISliderAdjustEvent(this, VIEW_ROLL_CVAR_NAME);
	CUISlider* pViewRollSlider = new CUISlider(pViewRollSliderObjectInfo->getFlags(),
		pViewRollSliderEvent,
		pViewRollSliderObjectInfo->getWidth(),
		pViewRollSliderObjectInfo->getHeight(),
		pTabObject->getXInset() + pViewRollSliderObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pViewRollSliderObjectInfo->getYOrigin(),
		pViewRollSliderObjectInfo->getMinValue(),
		pViewRollSliderObjectInfo->getMaxValue(),
		pViewRollSliderObjectInfo->getMarkerDistance());

	pViewRollSlider->setParent(pGameplayTab);

	if(!pViewRollSlider->init(pViewRollSliderObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize slider object for settings UI window.\n");
		return false;
	}

	// Create the slider label
	const ui_objectinfo_t* pViewRollLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, GAMEPLAY_TAB_VIEWROLL_LABEL_OBJ_NAME);
	if(!pViewRollLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWROLL_LABEL_OBJ_NAME);
		return false;
	}

	CUIText* pViewRollText = new CUIText(pViewRollLabelObjectInfo->getFlags(),
		pViewRollLabelObjectInfo->getFont(),
		pViewRollLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pViewRollLabelObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pViewRollLabelObjectInfo->getYOrigin());

	pViewRollText->setParent(pGameplayTab);

	// Create the display for the value
	const ui_objectinfo_t* pViewRollTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, GAMEPLAY_TAB_VIEWROLL_TAB_OBJ_NAME);
	if(!pViewRollTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWROLL_TAB_OBJ_NAME);
		return false;
	}

	CUISurface* pViewRollSurface = new CUISurface(pViewRollTabObjectInfo->getFlags(),
		pViewRollTabObjectInfo->getWidth(),
		pViewRollTabObjectInfo->getHeight(),
		pTabObject->getXInset() + pViewRollTabObjectInfo->getXOrigin(), 
		pTabObject->getYInset() + pViewRollTabObjectInfo->getYOrigin());

	pViewRollSurface->setParent(pGameplayTab);

	if(!pViewRollSurface->init(pViewRollTabObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize tab object for settings UI window.\n");
		return false;
	}

	// Create the text
	const ui_objectinfo_t* pViewRollTextObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, GAMEPLAY_TAB_VIEWROLL_TEXT_OBJ_NAME);
	if(!pViewRollTextObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, GAMEPLAY_TAB_VIEWROLL_TEXT_OBJ_NAME);
		return false;
	}

	m_pViewRollValueText = new CUIText(pViewRollTextObjectInfo->getFlags(), 
		pViewRollTabObjectInfo->getFont(),
		"",
		pViewRollTabObjectInfo->getXInset() + pViewRollTextObjectInfo->getXOrigin(),
		pViewRollTabObjectInfo->getYInset() + pViewRollTextObjectInfo->getYOrigin());

	m_pViewRollValueText->setParent(pViewRollSurface);

	CCVar* pViewRollCVar = gConsole.GetCVar(VIEW_ROLL_CVAR_NAME);
	if(pViewRollCVar)
	{
		if(pViewRollCVar->GetType() != CVAR_FLOAT)
		{
			m_pViewRollValueText->setText("");
			Con_EPrintf("CVar '%s' for 'CUISlider' object is not of float type.\n", VIEW_ROLL_CVAR_NAME);
		}
		else
		{
			Float value = pViewRollCVar->GetValue();
			pViewRollSlider->setValue(value);

			Char szValue[64];
			sprintf(szValue, "%0.1f", value);

			m_pViewRollValueText->setText(szValue);
		}
	}
	else
	{
		Con_EPrintf("CVar '%s' for 'CUISlider' not found.\n", VIEW_ROLL_CVAR_NAME);
	}

	if(!LoadScrollableOptionsList(pGameplayTab, pWinDesc, pTabObject, GAMEPLAY_TAB_LIST_OBJ_NAME, GAMEPLAY_DESC_FILE))
		return false;

	return pGameplayTab;
}

//=============================================
// @brief Sets up the Binds tab
//
//=============================================
bool CUISettingsWindow::AddTickBox( CUITabBody* pTab, 
	const ui_windowdescription_t* pWinDesc, 
	const ui_objectinfo_t* pTabObject, 
	const CString& tickBoxObjectName, 
	const CString& labelObjectName, 
	const CString& consoleObjectName,
	const Char* pstrConfigGrpName, 
	const Char* pstrConfigValueName )
{
	// Create the tick box for "reverse mouse"
	const ui_objectinfo_t* pCheckBoxObjectInfo = pWinDesc->getObject(UI_OBJECT_TICKBOX, tickBoxObjectName.c_str());
	if(!pCheckBoxObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, tickBoxObjectName.c_str());
		return false;
	}

	// Create the object
	CUITickBoxEvent* pEvent = new CUITickBoxEvent(this, consoleObjectName.c_str()); 

	CUITickBox* pTickBox = new CUITickBox(pCheckBoxObjectInfo->getFlags(),
		pEvent,
		pCheckBoxObjectInfo->getWidth(),
		pCheckBoxObjectInfo->getHeight(),
		pTabObject->getXInset() + pCheckBoxObjectInfo->getXOrigin(),
		pTabObject->getYInset() + pCheckBoxObjectInfo->getYOrigin());

	pTickBox->setParent(pTab);

	if(!pTickBox->init(pCheckBoxObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to init schema '%s' for 'CUITickBox'.\n", pCheckBoxObjectInfo->getSchema().c_str());
		return false;
	}

	pTickBox->setText(pCheckBoxObjectInfo->getText().c_str(), pCheckBoxObjectInfo->getFont(), pCheckBoxObjectInfo->getTextInset());

	if(pstrConfigGrpName && pstrConfigValueName)
	{
		bool configSetting = false;
		conf_group_t* pGroup = gConfig.FindGroup(pstrConfigGrpName);
		if(pGroup)
		{
			if(gConfig.GetInt(pGroup, pstrConfigValueName) == 1)
				configSetting = true;
		}

		pTickBox->setState(configSetting);
	}
	else
	{
		// Get the cvar's value and set the checkbox accordingly
		CCVar* pCVar = gConsole.GetCVar(consoleObjectName.c_str());
		if(pCVar)
		{
			if(pCVar->GetType() == CVAR_FLOAT)
			{
				if(pCVar->GetValue() != 0)
					pTickBox->setState(true);
				else
					pTickBox->setState(false);
			}
			else
			{
				Con_EPrintf("Console variable '%s', used by '%s', is of float type.\n", consoleObjectName.c_str(), tickBoxObjectName.c_str());
				pTickBox->setState(false);
			}
		}
		else
		{
			Con_EPrintf("Console variable '%s', used by '%s', does not exist.\n", consoleObjectName.c_str(), tickBoxObjectName.c_str());
			pTickBox->setState(false);
		}
	}

	// Create the text
	const ui_objectinfo_t* pLabelObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, labelObjectName.c_str());
	if(!pLabelObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, labelObjectName.c_str());
		return false;
	}

	CUIText* pText = new CUIText(pLabelObjectInfo->getFlags(),
		pLabelObjectInfo->getFont(),
		pLabelObjectInfo->getText().c_str(),
		pTabObject->getXInset() + pLabelObjectInfo->getXOrigin(),
		pTabObject->getYInset() + pLabelObjectInfo->getYOrigin());
	pText->setParent(pTab);

	return true;
}

//=============================================
// @brief Loads the binds list
//
//=============================================
void CUISettingsWindow::LoadBindsList( void )
{
	// Parse the binds list
	const Char* pstrFile = reinterpret_cast<const Char*>(FL_LoadFile(BINDS_FILE_PATH));
	if(!pstrFile)
	{
		Con_EPrintf("Failed to load binds list.\n");
		return;
	}

	CString bind;
	CString name;
	CString line;

	// Parse the script line by line
	Uint32 lineNum = 0;
	const Char* pstr = pstrFile;
	while(pstr)
	{
		pstr = Common::ReadLine(pstr, line);
		if(line.empty())
			continue;

		// Read in the bind name
		const Char* ppstr = Common::Parse(line.c_str(), bind);
		if(!ppstr)
		{
			Con_EPrintf("Incomplete line %d in %s.", lineNum, BINDS_FILE_PATH);
			lineNum++;
			continue;
		}

		ppstr = Common::Parse(ppstr, name);
		if(name.empty())
		{
			Con_EPrintf("Incomplete line %d in %s.", lineNum, BINDS_FILE_PATH);
			lineNum++;
			continue;
		}

		// Create a new bind
		if(!qstrcmp("separator", bind))
		{
			// Create the separator
			m_pBindsList->createSeparator(32, name.c_str(), 24);
		}
		else
		{
			// Create the action
			CUIBindsRowEvent* pEvent = new CUIBindsRowEvent(this);

			// Create the new row
			CUIListRow* pRow = m_pBindsList->createNewRow(pEvent, 8);

			CUIText* pTextName = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pBindsListInfo->getFont(), name.c_str(), 0, 0, 0);
			pRow->setColumnContents(0, pTextName);
			pEvent->SetRowIndex(pRow->getIndex());

			// Set the key name
			CString keyNameValue = gInput.GetKeynameForBind(bind.c_str());
			CUIText* pTextKeyName = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pBindsListInfo->getFont(), keyNameValue.c_str(), 0, 0, 0);
			pRow->setColumnContents(1, pTextKeyName);

			// Save it to the array
			m_bindsCommandArray.push_back(bind);
		}

		lineNum++;
	}

	FL_FreeFile(pstrFile);
}

//=============================================
// @brief Resets the binds list
//
//=============================================
void CUISettingsWindow::ResetBindsList( void )
{
	if(m_bindsCommandArray.empty())
		return;

	for(Uint32 i = 0; i < m_bindsCommandArray.size(); i++)
	{
		const Char* pstrCmdName = m_bindsCommandArray[i].c_str();
		if(!pstrCmdName)
			continue;

		const Char* keyName = gInput.GetKeynameForBind(pstrCmdName);
		CUIText* pText = reinterpret_cast<CUIText*>(m_pBindsList->getRowColumnObject(i, 1));

		if(keyName)
			pText->setText(keyName);
		else
			pText->setText("");
	}
}

//=============================================
// @brief Sets focus on a specific row
//
//=============================================
void CUISettingsWindow::SetFocusOnBindsRow( Uint32 rowIndex )
{
	m_pBindsList->setHighlightOnRow(rowIndex, true);
	m_selectedRowIndex = rowIndex;
}

//=============================================
// @brief Sets focus on a specific row
//
//=============================================
void CUISettingsWindow::AddPendingSetting( const Char* pstrId, const Char* pstrCmd )
{
	// Overwrite an existing command if possible
	if(!m_queuedCommandsArray.empty())
	{
		for(Uint32 i = 0; i < m_queuedCommandsArray.size(); i++)
		{
			setting_t& setting = m_queuedCommandsArray[i];
			if(!qstrcmp(setting.id, pstrId))
			{
				setting.cmd = pstrCmd;
				return;
			}
		}
	}

	// Add a new command
	setting_t newSetting;
	newSetting.id = pstrId;
	newSetting.cmd = pstrCmd;

	m_queuedCommandsArray.push_back(newSetting);
}

//=============================================
// @brief Enters bind mode
//
//=============================================
void CUISettingsWindow::EnterBindMode( Uint32 rowIndex )
{
	// Hide the mouse
	gInput.HideMouse();
	m_isInBindMode = true;

	m_bindRowIndex = rowIndex;
	m_pBindsList->setHighlightOnRow(rowIndex, true);
	m_pBindsList->setHighlightOnRowColumn(1);
}

//=============================================
// @brief Leaves bind mode
//
//=============================================
void CUISettingsWindow::LeaveBindMode( void )
{
	// Restore the mouse
	gInput.ShowMouse();
	m_isInBindMode = false;

	// Reset highlight on column
	if(m_bindRowIndex != -1)
	{
		m_pBindsList->setHighlightOnRowColumn(-1);
		m_bindRowIndex = -1;
	}
}

//=============================================
// @brief Handles think functionalities
//
//=============================================
void CUISettingsWindow::postThink( void )
{
	// Perform regular think functions
	CUIWindow::postThink();

	// Reset if needed
	if(m_bReset)
	{
		ResetBindsList();
		m_bReset = false;
	}
}

//=============================================
// @brief Handles a mouse button event
//
//=============================================
bool CUISettingsWindow::mouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	if(m_currentTabIndex == m_bindsTabIndex)
	{
		if(m_isInBindMode)
		{
			if(keyDown)
			{
				// Get the key name
				const Char* pstrKeyname = gInput.GetMouseButtonName(button);
				if(pstrKeyname)
					SetBind(pstrKeyname, m_bindsCommandArray[m_bindRowIndex].c_str());

				LeaveBindMode();
			}

			return true;
		}
	}

	// Call base class to handle this normally
	return CUIWindow::mouseButtonEvent(mouseX, mouseY, button, keyDown);
}

//=============================================
// @brief Handles a keyboard button event
//
//=============================================
bool CUISettingsWindow::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(m_currentTabIndex == m_bindsTabIndex)
	{
		if(m_isInBindMode)
		{
			if(keyDown)
			{
				// Handle key binding
				if(button == SDL_SCANCODE_ESCAPE)
				{
					LeaveBindMode();
					return true;
				}

				// Get the key name
				const Char* pstrKeyname = gInput.GetKeynameForScancode(button);
				if(pstrKeyname)
					SetBind(pstrKeyname, m_bindsCommandArray[m_bindRowIndex].c_str());

				LeaveBindMode();
			}

			return true;
		}
		else if(keyDown)
		{
			if(m_selectedRowIndex != -1)
			{
				if(button == SDL_SCANCODE_DELETE)
				{
					ClearSelectedKey();
					return true;
				}

				if(button == SDL_SCANCODE_RETURN)
				{
					EnterBindMode(m_selectedRowIndex);
					return true;
				}
			}
		}
	}

	// Call base class to handle this normally
	return CUIWindow::keyEvent(button, mod, keyDown);
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SetBind( const Char* pstrKeyname, const Char* pstrBind )
{
	// Make sure no other row has this key set
	Int32 nbRows = m_pBindsList->getNbRows();
	for(Int32 i = 0; i < nbRows; i++)
	{
		if(i == m_bindRowIndex)
			continue;

		CUIText* pText = reinterpret_cast<CUIText*>(m_pBindsList->getRowColumnObject(i, 1));
		if(!qstrcmp(pText->getText(), pstrKeyname))
		{
			// Note: The bind class will manage unbinding from other keys
			pText->setText("");
		}
	}

	// Set the new key name in the column
	CUIText* pText = reinterpret_cast<CUIText*>(m_pBindsList->getRowColumnObject(m_bindRowIndex, 1));
	pText->setText(pstrKeyname);

	// Create the command and add it to the buffer
	CString newCmd;
	newCmd << "bind \"" << pstrKeyname << "\" \"" << pstrBind << "\"";

	// Get the bind id
	const Char* pstrId = m_bindsCommandArray[m_bindRowIndex].c_str();
	AddPendingSetting(pstrId, newCmd.c_str());
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::ClearSelectedKey( void )
{
	if(m_selectedRowIndex == -1)
		return;

	CUIText* pText = reinterpret_cast<CUIText*>(m_pBindsList->getRowColumnObject(m_selectedRowIndex, 1));
	if(!pText || pText->getText()[0] == '\0')
		return;
	
	// Create the command and add it to the buffer
	CString newCmd;
	newCmd << "unbind \"" << pText->getText() << "\"";

	// Get the bind id
	const Char* pstrId = m_bindsCommandArray[m_selectedRowIndex].c_str();
	AddPendingSetting(pstrId, newCmd.c_str());

	pText->setText("");
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::BindSelectedKey( void )
{
	if(m_selectedRowIndex != -1)
		EnterBindMode(m_selectedRowIndex);
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::ApplyChanges( void )
{
	if(m_queuedCommandsArray.empty())
		return;

	for(Uint32 i = 0; i < m_queuedCommandsArray.size(); i++)
	{
		// Send to the command buffer
		CString& cmd = m_queuedCommandsArray[i].cmd;
		gCommands.AddCommand(cmd.c_str());
	}

	// Clear the array
	m_queuedCommandsArray.clear();

	// Reset video also if needed
	if(m_bResetVideo)
	{
		gCommands.AddCommand("_vid_restart");
		m_bResetVideo = false;
	}

	m_bReset = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SelectDevice( Int32 deviceIndex )
{
	// Skip if nothing was changed
	if(deviceIndex == m_displayDeviceIndex)
		return;

	// Add the command to the queue
	CString cmd;
	cmd << "_vid_setdisplay " << (Int32)deviceIndex;

	AddPendingSetting("Video.DisplayDevice", cmd.c_str());
	PopulateResolutions(deviceIndex);
	m_displayDeviceIndex = deviceIndex;

	if(!m_bResetVideo)
		m_bResetVideo = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SelectResolution( Int32 resIndex )
{
	// Skip if nothing was changed
	if(m_resolutionIndex == resIndex)
		return;

	Uint32 width, height;
	gWindow.GetResolutionInfo(m_displayDeviceIndex, resIndex, width, height);

	// Add the commands to the queue
	CString cmd;
	cmd << "_vid_setwidth " << (Int32)width;
	AddPendingSetting("Video.ScreenWidth", cmd.c_str());

	cmd.clear();
	cmd << "_vid_setheight " << (Int32)height;
	AddPendingSetting("Video.ScreenHeight", cmd.c_str());

	// Set current resolution
	m_resolutionIndex = resIndex;

	if(!m_bResetVideo)
		m_bResetVideo = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SelectWindowMode( Int32 modeIdx )
{
	// Add the commands to the queue
	CString cmd;

	if(modeIdx == 0)
		cmd << "_vid_setfullscreen";
	else
		cmd << "_vid_setwindowed";

	AddPendingSetting("Video.ScreenMode", cmd.c_str());

	if(!m_bResetVideo)
		m_bResetVideo = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SelectAnisotropy( Int32 anisotropyIdx )
{
	// Make sure it's valid
	if(anisotropyIdx < 0 || anisotropyIdx >= (Int32)CTextureManager::GetInstance()->GetNbAnisotropySettings())
		return;

	// Add command to queue
	CString cmd;
	cmd << ANISOTROPY_CVAR_NAME << " " << anisotropyIdx;

	AddPendingSetting("Video.AnisotropicFiltering", cmd.c_str());
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SelectAntiAliasSetting( Int32 msaaSetting )
{
	// Make sure it's valid
	if(msaaSetting < 0 || msaaSetting >= m_pMSAADropList->getListSize())
		return;

	// Add command to queue
	CString cmd;
	cmd << "_vid_setmsaa " << (Int32)msaaSetting;
	AddPendingSetting("Video.SetMSAA", cmd.c_str());

	if(!m_bResetVideo)
		m_bResetVideo = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SelectVerticalSyncSetting( Int32 msaaSetting )
{
	bool enable = (msaaSetting == 1) ? true : false;
	if(gWindow.IsVerticalSyncEnabled() == enable)
		return;

	// Add command to queue
	CString cmd;
	cmd << "_vid_setvsync " << (enable ? 1 : 0);
	AddPendingSetting("Video.SetVSync", cmd.c_str());

	if(!m_bResetVideo)
		m_bResetVideo = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::RestoreDefaultBinds( void )
{
	CString cmd;
	cmd << "exec " << DEFAULT_BINDS_FILENAME;

	gCommands.AddCommand(cmd.c_str());

	// So we reset on the next frame
	m_bReset = true;
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::CVarChanged( const Char* pstrCvarName, Float value )
{
	CString cmd;
	cmd << pstrCvarName << " " << value;

	AddPendingSetting(pstrCvarName, cmd.c_str());
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SetMouseSensitivityTabText( const Char* pstrText )
{
	m_pSensitivityValueText->setText(pstrText);
}

//=============================================
// @brief Sets mouse filter frames tab text
//
//=============================================
void CUISettingsWindow::SetMouseFilterFramesTabText( const Char* pstrText )
{
	m_pFilterFramesValueText->setText(pstrText);
}

//=============================================
// @brief
//
//=============================================
void CUISettingsWindow::SetViewRollTabText( const Char* pstrText )
{
	m_pViewRollValueText->setText(pstrText);
}

//=============================================
// @brief Sets mouse filter frames tab text
//
//=============================================
void CUISettingsWindow::SetViewBobTabText( const Char* pstrText )
{
	m_pViewBobValueText->setText(pstrText);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISettingsCancelEvent::PerformAction( Int32 param )
{
	if(m_pWindow)
		m_pWindow->setWindowFlags(CUIWindow::UIW_FL_KILLME);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISettingsApplyEvent::PerformAction( Int32 param )
{
	if(m_pWindow)
		m_pWindow->ApplyChanges();
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
bool CUIBindsRowEvent::MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT && !keyDown)
		return false;

	if(keyDown)
	{
		Float interval = (Float)ens.time - m_lastClickTime;
		if(interval < 0.5)
		{
			// Enter bind mode
			m_pWindow->EnterBindMode(m_rowIndex);
		}
		else
		{
			// Set focus to this row
			m_pWindow->SetFocusOnBindsRow(m_rowIndex);
		}

		m_lastClickTime = ens.time;
	}

	return true;
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUITabSelectEvent::PerformAction( Int32 param )
{
	if(m_pWindow)
		m_pWindow->SetCurrentTabIndex(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIBindsClearBtnEvent::PerformAction( Int32 param )
{
	if(m_pWindow)
		m_pWindow->ClearSelectedKey();
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIBindsRestoreButtonEvent::PerformAction( Int32 param )
{
	if(m_pWindow)
		m_pWindow->RestoreDefaultBinds();
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIBindsBindBtnEvent::PerformAction( Int32 param )
{
	if(m_pWindow)
		m_pWindow->BindSelectedKey();
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIDeviceSelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;

	m_pWindow->SelectDevice(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIResolutionSelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;

	m_pWindow->SelectResolution(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIWindowModeSelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;
	
	m_pWindow->SelectWindowMode(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIAnisotropySelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;

	m_pWindow->SelectAnisotropy(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIAntiAliasSelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;

	m_pWindow->SelectAntiAliasSetting(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIVerticalSyncSelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;

	m_pWindow->SelectVerticalSyncSetting(param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIScrollSurfaceDropListToggleEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;
	
	m_pWindow->SetAdvancedOptionFocus(m_rowIndex, (param == 0) ? false : true);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUIScrollSurfaceDropListSelectEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;
	
	m_pWindow->AdvancedOptionSelect(m_rowIndex, param);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUITickBoxEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;
	
	m_pWindow->CVarChanged(m_cvarName.c_str(), (param == 0) ? 0 : 1);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISliderAdjustEvent::PerformAction( Int32 param )
{
	if(!m_pWindow)
		return;
	
	Float value = (Float)param/100.0f;
	if(m_isInteger)
		value = (Int32)value;

	m_pWindow->CVarChanged(m_cvarName.c_str(), value);

	// TODO: find a better solution to this shit
	if(!qstrcmp(m_cvarName, MOUSE_SENSITIVITY_CVAR_NAME))
	{
		Char szValue[64];
		sprintf(szValue, "%0.1f", value);
		m_pWindow->SetMouseSensitivityTabText(szValue);
	}
	else if(!qstrcmp(m_cvarName, MOUSE_FILTER_FRAMES_CVAR_NAME))
	{
		Char szValue[64];
		sprintf(szValue, "%d", (Int32)value);
		m_pWindow->SetMouseFilterFramesTabText(szValue);
	}
	else if(!qstrcmp(m_cvarName, VIEW_ROLL_CVAR_NAME))
	{
		Char szValue[64];
		sprintf(szValue, "%0.1f", value);
		m_pWindow->SetViewRollTabText(szValue);
	}
	else if(!qstrcmp(m_cvarName, VIEW_BOB_CVAR_NAME))
	{
		Char szValue[64];
		sprintf(szValue, "%0.1f", value);
		m_pWindow->SetViewBobTabText(szValue);
	}
}
