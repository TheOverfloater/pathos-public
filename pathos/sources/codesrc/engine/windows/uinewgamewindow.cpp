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
#include "uinewgamewindow.h"

#include "window.h"
#include "commands.h"

#include "r_basicdraw.h"
#include "r_main.h"
#include "system.h"
#include "sv_main.h"
#include "cvar.h"

// Current instance of the window
CUINewGameWindow* CUINewGameWindow::m_pInstance = nullptr;

// Window description file
const Char CUINewGameWindow::WINDOW_DESC_FILE[] = "newgamewindow.txt";
// Window description file
const Char CUINewGameWindow::WINDOW_OBJ_NAME[] = "NewGameWindow";
// Cancel button object name
const Char CUINewGameWindow::CANCEL_BUTTON_OBJ_NAME[] = "CancelButton";
// 'Easy' button object name
const Char CUINewGameWindow::NEW_GAME_EASY_BUTTON_OBJ_NAME[] = "NewGameEasyButton";
// 'Normal' button object name
const Char CUINewGameWindow::NEW_GAME_NORMAL_BUTTON_OBJ_NAME[] = "NewGameNormalButton";
// 'Hard' button object name
const Char CUINewGameWindow::NEW_GAME_HARD_BUTTON_OBJ_NAME[] = "NewGameHardButton";

//=============================================
// @brief Constructor
//
//=============================================
CUINewGameWindow::CUINewGameWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIWindow(UIW_FL_MENUWINDOW, flags, width, height, originx, originy)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUINewGameWindow::~CUINewGameWindow( void )
{
	// Probably destroyed by the UI manager
	if(m_pInstance)
		m_pInstance = nullptr;
}

//=============================================
// @brief Creates an instance of the console window
//
//=============================================
CUINewGameWindow* CUINewGameWindow::CreateInstance( bool isIngame )
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
	Int32 yOrigin = gWindow.GetCenterY() - pWindowObject->getHeight()/3;

	// Create the main tab object
	CUINewGameWindow* pWindow = new CUINewGameWindow(pWindowObject->getFlags(), pWindowObject->getWidth(), pWindowObject->getHeight(), xOrigin, yOrigin);
	if(!pWindow->init(pWinDesc, pWindowObject, isIngame))
	{
		Con_EPrintf("Failed to initialize new game window.\n");
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
void CUINewGameWindow::DestroyInstance( void )
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
CUINewGameWindow* CUINewGameWindow::GetInstance( void )
{
	return m_pInstance;
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUINewGameWindow::init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject, bool isIngame )
{
	// Retreive schema for the window object
	const Char* pstrSchemaName = pWindowObject->getSchema().c_str();
	if(!CUIWindow::init(pstrSchemaName))
	{
		Con_EPrintf("Failed to initialize schema '%s' for console window.\n", pstrSchemaName);
		return false;
	}

	// Create the dragger
	if(pWindowObject->dragger)
	{
		CUIDragger* pDragger = new CUIDragger(UIEL_FL_FIXED_H, m_width-40, 30, 0, 0);
		pDragger->setParent(this);
	}

	// Set the window's properties
	setAlpha(pWindowObject->getAlpha());
	setTitle(pWindowObject->getTitle().c_str(), pWindowObject->getFont(), pWindowObject->getXInset(), pWindowObject->getYInset());

	// Create cancel button
	const ui_objectinfo_t* pCancelButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, CANCEL_BUTTON_OBJ_NAME);
	if(!pCancelButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, CANCEL_BUTTON_OBJ_NAME);
		return false;
	}

	CUINewGameWindowButtonEvent* pEvent = new CUINewGameWindowButtonEvent(this, NEWGAME_BTN_CANCEL);
	CUIButton* pCancelButton = new CUIButton(pCancelButtonObjectInfo->getFlags(), 
		pCancelButtonObjectInfo->getText().c_str(), 
		pCancelButtonObjectInfo->getFont(), pEvent, 
		pCancelButtonObjectInfo->getWidth(), 
		pCancelButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pCancelButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pCancelButtonObjectInfo->getYOrigin());

	pCancelButton->setParent(this);
	
	if(!pCancelButton->init(pCancelButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for new game UI window.\n");
		return false;
	}

	// Get 'Easy' button description
	const ui_objectinfo_t* pEasyButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, NEW_GAME_EASY_BUTTON_OBJ_NAME);
	if(!pEasyButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, NEW_GAME_EASY_BUTTON_OBJ_NAME);
		return false;
	}

	// Create the "Easy" button
	pEvent = new CUINewGameWindowButtonEvent(this, NEWGAME_BTN_EASY);
	CUIButton* pEasyButton = new CUIButton(pEasyButtonObjectInfo->getFlags(), 
		pEasyButtonObjectInfo->getText().c_str(), 
		pEasyButtonObjectInfo->getFont(), pEvent, 
		pEasyButtonObjectInfo->getWidth(), 
		pEasyButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pEasyButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pEasyButtonObjectInfo->getYOrigin());
	pEasyButton->setParent(this);
	
	if(!pEasyButton->init(pEasyButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for new game UI window.\n");
		return false;
	}

	// Get 'Normal' button description
	const ui_objectinfo_t* pNormalButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, NEW_GAME_NORMAL_BUTTON_OBJ_NAME);
	if(!pNormalButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, NEW_GAME_NORMAL_BUTTON_OBJ_NAME);
		return false;
	}

	// Create the "Normal" button
	pEvent = new CUINewGameWindowButtonEvent(this, NEWGAME_BTN_NORMAL);
	CUIButton* pNormalButton = new CUIButton(pNormalButtonObjectInfo->getFlags(), 
		pNormalButtonObjectInfo->getText().c_str(), 
		pNormalButtonObjectInfo->getFont(), pEvent, 
		pNormalButtonObjectInfo->getWidth(), 
		pNormalButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pNormalButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pNormalButtonObjectInfo->getYOrigin());
	pNormalButton->setParent(this);
	
	if(!pNormalButton->init(pNormalButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for new game UI window.\n");
		return false;
	}

	// Get 'Hard' button description
	const ui_objectinfo_t* pHardButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, NEW_GAME_HARD_BUTTON_OBJ_NAME);
	if(!pHardButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, NEW_GAME_HARD_BUTTON_OBJ_NAME);
		return false;
	}

	// Create the "Hard" button
	pEvent = new CUINewGameWindowButtonEvent(this, NEWGAME_BTN_HARD);
	CUIButton* pHardButton = new CUIButton(pHardButtonObjectInfo->getFlags(), 
		pHardButtonObjectInfo->getText().c_str(), 
		pHardButtonObjectInfo->getFont(), pEvent, 
		pHardButtonObjectInfo->getWidth(), 
		pHardButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pHardButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pHardButtonObjectInfo->getYOrigin());
	pHardButton->setParent(this);

	if(!pHardButton->init(pHardButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for new game UI window.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief Manages a button event
//
//=============================================
void CUINewGameWindow::buttonPressed( Uint32 buttonId )
{
	switch(buttonId)
	{
	case NEWGAME_BTN_CANCEL:
		CUINewGameWindow::DestroyInstance();
		break;
	case NEWGAME_BTN_EASY:
		{
			// Destroy window instance
			CUINewGameWindow::DestroyInstance();

			// Create command for launching game
			CString cmd;
			cmd << g_psv_skill->GetName() << " " << SKILL_EASY << ";";
			cmd << "map " << ens.startmap.c_str();
			gCommands.AddCommand(cmd.c_str());
		}
		break;
	case NEWGAME_BTN_NORMAL:
		{
			// Destroy window instance
			CUINewGameWindow::DestroyInstance();

			// Create command for launching game
			CString cmd;
			cmd << g_psv_skill->GetName() << " " << SKILL_NORMAL << ";";
			cmd << "map " << ens.startmap.c_str();
			gCommands.AddCommand(cmd.c_str());
		}
		break;
	case NEWGAME_BTN_HARD:
		{
			// Destroy window instance
			CUINewGameWindow::DestroyInstance();

			// Create command for launching game
			CString cmd;
			cmd << g_psv_skill->GetName() << " " << SKILL_HARD << ";";
			cmd << "map " << ens.startmap.c_str();
			gCommands.AddCommand(cmd.c_str());
		}
		break;
	}
}