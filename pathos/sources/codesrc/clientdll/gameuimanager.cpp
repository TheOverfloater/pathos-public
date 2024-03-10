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
#include "gameuimanager.h"
#include "gameuielements.h"
#include "clientdll.h"
#include "efxapi.h"
#include "input.h"

#include "gameuitextwindow.h"
#include "gameuiloginwindow.h"
#include "gameuikeypadwindow.h"
#include "gameuisubwaywindow.h"
#include "gameuiobjectiveswindow.h"
#include "huddraw.h"
#include "gameuiwindows_shared.h"
#include "gameui_shared.h"

// Blur time for background
const Float CGameUIManager::BACKGROUND_BLUR_TIME = 2.0f;
// Default font schema of the game UI
const Char CGameUIManager::DEFAULT_TEXT_SCHEMA[] = "gameuidefault";

// Object declaration
CGameUIManager gGameUIManager;

//=============================================
//
//
//=============================================
CGameUIManager::CGameUIManager( void ):
	m_pActiveWindow(nullptr),
	m_blurFadeTime(0),
	m_isBlurActive(false),
	m_uiServerUserMsgId(0),
	m_pCvarBorders(nullptr),
	m_pFontSet(nullptr)
{
	CGameUIObject::SetGameUIManager(this);
}

//=============================================
//
//
//=============================================
CGameUIManager::~CGameUIManager( void )
{
	ClearGame();
}

//=============================================
//
//
//=============================================
bool CGameUIManager::Init( void )
{
	// Register the user message
	m_uiServerUserMsgId = cl_engfuncs.pfnRegisterClientUserMessage("GameUIMessage", -1);

	// For toggling borders
	m_pCvarBorders = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_SAVE, "ui_borders", "1", "Toggle borders on game UI windows");

	return true;
}

//=============================================
//
//
//=============================================
void CGameUIManager::Shutdown( void )
{
	ClearGame();
}

//=============================================
//
//
//=============================================
bool CGameUIManager::InitGame( void )
{
	// Precache sounds used by UI
	cl_engfuncs.pfnPrecacheSound(GAMEUI_OK_SOUND, RS_GAME_LEVEL);
	cl_engfuncs.pfnPrecacheSound(GAMEUI_FAIL_SOUND, RS_GAME_LEVEL);
	cl_engfuncs.pfnPrecacheSound(GAMEUI_BLIP_SOUND, RS_GAME_LEVEL);

	return true;
}

//=============================================
//
//
//=============================================
void CGameUIManager::ClearGame( void )
{
	if(m_pActiveWindow)
	{
		delete m_pActiveWindow;
		m_pActiveWindow = nullptr;
	}

	if(!m_pNextWindowList.empty())
	{
		m_pNextWindowList.begin();
		while(!m_pNextWindowList.end())
		{
			// Delete the window
			CGameUIWindow* pwindow = m_pNextWindowList.get();
			delete pwindow;

			m_pNextWindowList.remove(m_pNextWindowList.get_link());
			m_pNextWindowList.next();
		}

		m_pNextWindowList.clear();
	}

	m_blurFadeTime = 0;
	m_isBlurActive = false;
}

//=============================================
//
//
//=============================================
bool CGameUIManager::InitGL( void )
{
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(DEFAULT_TEXT_SCHEMA, screenHeight);
	if(!m_pFontSet)
		m_pFontSet = cl_renderfuncs.pfnGetDefaultFontSet();

	// Destroy any active windows, because if we resize the screen,
	// the size will no longer be valid
	RespawnWindow();

	// Nothing else to do
	return true;
}

//=============================================
//
//
//=============================================
void CGameUIManager::ClearGL( void )
{
}

//=============================================
//
//
//=============================================
CGameUIWindow* CGameUIManager::SpawnWindow( gameui_windows_t windowtype )
{
	Uint32 screenwidth, screenheight;
	cl_renderfuncs.pfnGetScreenSize(screenwidth, screenheight);

	Int32 flags = m_pCvarBorders->GetValue() >= 1 ? CGameUIWindow::FL_WINDOW_NONE : CGameUIWindow::FL_WINDOW_NO_BGBORDERS;

	CGameUIWindow* pWindow = nullptr;
	switch(windowtype)
	{
	case GAMEUI_TEXTWINDOW:
			pWindow = new CGameUITextWindow(flags, 0, 0, screenwidth, screenheight);
		break;
	case GAMEUI_KEYPADWINDOW:
			pWindow = new CGameUIKeypadWindow(flags, 0, 0, screenwidth, screenheight);
		break;
	case GAMEUI_LOGINWINDOW:
			pWindow = new CGameUILoginWindow(flags, 0, 0, screenwidth, screenheight);
		break;
	case GAMEUI_SUBWAYWINDOW:
			pWindow = new CGameUISubwayWindow(flags, 0, 0, screenwidth, screenheight);
		break;
	case GAMEUI_OBJECTIVESWINDOW:
			pWindow = new CGameUIObjectivesWindow(flags, 0, 0, screenwidth, screenheight);
		break;
	};

	if(!pWindow)
	{
		cl_engfuncs.pfnCon_Printf("%s - %d is not a valid window.\n", __FUNCTION__, (Int32)windowtype);
		return nullptr;
	}

	// Initialize it
	pWindow->init();

	if(m_pActiveWindow)
	{
		// If we have active windows, put this in the quue
		m_pNextWindowList.radd(pWindow);
	}
	else
	{
		// Show the mouse
		cl_engfuncs.pfnShowMouse();
		cl_engfuncs.pfnSetShouldHideMouse(false);

		// Assign as current window
		m_pActiveWindow = pWindow;

		// Reset these
		CL_ResetPressedInputs();
	}

	if(m_pActiveWindow && (m_pActiveWindow->getWindowFlags() & CGameUIWindow::FL_WINDOW_WAIT_TILL_NEXT))
	{
		// Destroy current window
		DestroyActiveWindow();
	}

	// Give back a ptr so the client can initialize it's data
	return pWindow;
}

//=============================================
//
//
//=============================================
void CGameUIManager::DestroyActiveWindow( void )
{
	if(!m_pActiveWindow)
		return;

	// Call OnRemove function
	m_pActiveWindow->onRemove();

	delete m_pActiveWindow;
	m_pActiveWindow = nullptr;

	if(!m_pNextWindowList.empty())
	{
		// Get first available window
		m_pNextWindowList.begin();
		CGameUIWindow* pWindow = m_pNextWindowList.get();
		m_pNextWindowList.remove(m_pNextWindowList.get_link());

		// Set as active window
		m_pActiveWindow = pWindow;
	}
	else
	{
		// Hide mouse
		cl_engfuncs.pfnHideMouse();
		cl_engfuncs.pfnResetMouse();
		cl_engfuncs.pfnSetShouldHideMouse(true);

		// Tell server we're done with any windows
		cl_engfuncs.pfnClientUserMessageBegin(m_uiServerUserMsgId);
			cl_engfuncs.pfnMsgWriteByte(GAMEUIEVENT_CLOSED_ALL_WINDOWS);
		cl_engfuncs.pfnClientUserMessageEnd();
	}
}

//=============================================
//
//
//=============================================
void CGameUIManager::RespawnWindow( void )
{
	if(!m_pActiveWindow)
		return;

	// Get window type
	gameui_windows_t type = m_pActiveWindow->getWindowType();
	
	// If type is unknown, just delete it
	if(type == GAMEUI_WINDOW_NONE)
	{
		DestroyActiveWindow();
		return;
	}

	switch(type)
	{
	case GAMEUI_TEXTWINDOW:
		{
			// Get the current window state
			CString textfilename, passcode;
			CGameUITextWindow* pActiveWindow = reinterpret_cast<CGameUITextWindow*>(m_pActiveWindow);
			pActiveWindow->getInformation(textfilename, passcode);

			// Destroy it
			DestroyActiveWindow();

			// Create a new one
			pActiveWindow = reinterpret_cast<CGameUITextWindow*>(SpawnWindow(type));
			if(!pActiveWindow)
				return;

			pActiveWindow->initData(textfilename.c_str(), passcode.c_str());
		}
		break;
	case GAMEUI_KEYPADWINDOW:
		{
			// Get the current window state
			bool stayTillNext;
			CString passcode, input;
			CGameUIKeypadWindow* pActiveWindow = reinterpret_cast<CGameUIKeypadWindow*>(m_pActiveWindow);
			pActiveWindow->getInformation(passcode, input, stayTillNext);

			// Destroy it
			DestroyActiveWindow();

			// Create a new one
			pActiveWindow = reinterpret_cast<CGameUIKeypadWindow*>(SpawnWindow(type));
			if(!pActiveWindow)
				return;

			pActiveWindow->initData(passcode.c_str(), input.c_str(), stayTillNext);
		}
		break;
	case GAMEUI_LOGINWINDOW:
		{
			// Get the current window state
			bool stayTillNext;
			CString username, password, usernameinput, passwordinput;
			CGameUILoginWindow* pActiveWindow = reinterpret_cast<CGameUILoginWindow*>(m_pActiveWindow);
			pActiveWindow->getInformation(username, password, usernameinput, passwordinput, stayTillNext);

			// Destroy it
			DestroyActiveWindow();

			// Create a new one
			pActiveWindow = reinterpret_cast<CGameUILoginWindow*>(SpawnWindow(type));
			if(!pActiveWindow)
				return;

			pActiveWindow->initData(username.c_str(), password.c_str(), usernameinput.c_str(), passwordinput.c_str(), stayTillNext);
		}
		break;
	case GAMEUI_SUBWAYWINDOW:
		{
			// Get the current window state
			Int32 flags;
			CString scriptfile;
			CGameUISubwayWindow* pActiveWindow = reinterpret_cast<CGameUISubwayWindow*>(m_pActiveWindow);
			pActiveWindow->getInformation(scriptfile, flags);

			// Destroy it
			DestroyActiveWindow();

			// Create a new one
			pActiveWindow = reinterpret_cast<CGameUISubwayWindow*>(SpawnWindow(type));
			if(!pActiveWindow)
				return;

			pActiveWindow->initData(scriptfile.c_str(), flags);
		}
		break;
	case GAMEUI_OBJECTIVESWINDOW:
		{
			// Get the current window state
			CString selectedObjective;
			CArray<CString> objectivesArray;
			Int16 newObjectiveBits;

			CGameUIObjectivesWindow* pActiveWindow = reinterpret_cast<CGameUIObjectivesWindow*>(m_pActiveWindow);
			pActiveWindow->getInformation(objectivesArray, selectedObjective, newObjectiveBits);

			// Destroy it
			DestroyActiveWindow();

			// Create a new one
			pActiveWindow = reinterpret_cast<CGameUIObjectivesWindow*>(SpawnWindow(type));
			if(!pActiveWindow)
				return;

			pActiveWindow->initData(objectivesArray, selectedObjective.c_str(), newObjectiveBits);
		}
		break;
	}
}

//=============================================
//
//
//=============================================
void CGameUIManager::Think( void )
{
	Double engineTime = cl_engfuncs.pfnGetEngineTime();
	Double clientTime = cl_engfuncs.pfnGetClientTime();

	if(m_pActiveWindow)
	{
		// Check for delay remove windows
		if(m_pActiveWindow->getWindowFlags() & CGameUIWindow::FL_WINDOW_DELAY_REMOVE)
		{
			Double removetime = m_pActiveWindow->getWindowRemoveTime();
			if(removetime <= clientTime)
				DestroyActiveWindow();
		}

		if(m_pActiveWindow)
		{
			if(m_pActiveWindow->getWindowFlags() & CGameUIWindow::FL_WINDOW_KILLME)
			{
				// Destroy the window
				DestroyActiveWindow();
			}
			else
			{
				m_pActiveWindow->think();

				if(!m_isBlurActive)
				{
					m_isBlurActive = true;
					m_blurFadeTime = engineTime;
				}
			}
		}
	}
	else if(m_isBlurActive)
	{
		m_isBlurActive = false;
		m_blurFadeTime = engineTime;
	}

	// Manage blurring
	if(m_blurFadeTime)
	{
		if((m_blurFadeTime+BACKGROUND_BLUR_TIME) <= engineTime)
		{
			// Reset blur
			m_blurFadeTime = 0;
			cl_efxapi.pfnSetGaussianBlur(m_isBlurActive, 1.0);
		}
		else
		{
			Float bluralpha = 0.0;
			if(m_isBlurActive)
				bluralpha = (engineTime - m_blurFadeTime)/BACKGROUND_BLUR_TIME;
			else
				bluralpha = 1.0 - ((engineTime - m_blurFadeTime)/BACKGROUND_BLUR_TIME);

			cl_efxapi.pfnSetGaussianBlur(true, bluralpha);
		}
	}
	else if(m_isBlurActive)
	{
		cl_efxapi.pfnSetGaussianBlur(true, 1.0);
	}
}

//=============================================
//
//
//=============================================
bool CGameUIManager::Draw( void )
{
	if(!m_pActiveWindow)
		return true;

	if(!gHUDDraw.SetupDraw())
		return false;

	// Draw the window
	bool result = m_pActiveWindow->draw();

	gHUDDraw.FinishDraw();

	if(!result)
		gHUDDraw.ManageErrorMessage();

	return result;
}

//=============================================
//
//
//=============================================
bool CGameUIManager::MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll )
{
	if(!m_pActiveWindow)
		return false;

	// Get mouse position
	Int32 mouseX, mouseY;
	cl_engfuncs.pfnGetMousePosition(mouseX, mouseY);

	if(m_pActiveWindow->mouseWheelEvent(mouseX, mouseY, button, keyDown, scroll))
		return true;

	return false;
}

//=============================================
//
//
//=============================================
bool CGameUIManager::MouseButtonEvent( Int32 button, bool keyDown )
{
	if(!m_pActiveWindow)
		return false;

	// Get mouse position
	Int32 mouseX, mouseY;
	cl_engfuncs.pfnGetMousePosition(mouseX, mouseY);

	if(m_pActiveWindow->mouseButtonEvent(mouseX, mouseY, button, keyDown))
		return true;

	return false;
}

//=============================================
//
//
//=============================================
bool CGameUIManager::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!m_pActiveWindow)
		return false;

	if(m_pActiveWindow->keyEvent(button, mod, keyDown))
		return true;

	return false;
}

//=============================================
//
//
//=============================================
bool CGameUIManager::HasActiveWindows( void ) const
{
	return (m_pActiveWindow != nullptr) ? true : false;
}

//=============================================
//
//
//=============================================
const CGameUIWindow* CGameUIManager::GetActiveWindow( void )
{
	return m_pActiveWindow;
}