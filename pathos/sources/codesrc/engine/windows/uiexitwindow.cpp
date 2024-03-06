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
#include "uiexitwindow.h"

#include "window.h"
#include "commands.h"

#include "r_basicdraw.h"
#include "r_main.h"
#include "system.h"

// Current instance of the window
CUIExitWindow* CUIExitWindow::m_pInstance = nullptr;

// Window description file
const Char CUIExitWindow::WINDOW_DESC_FILE[] = "exitwindow.txt";
// Window description file
const Char CUIExitWindow::WINDOW_OBJ_NAME[] = "ExitWindow";
// Cancel button object name
const Char CUIExitWindow::CANCEL_BUTTON_OBJ_NAME[] = "CancelButton";
// Save and Quit button object name
const Char CUIExitWindow::SAVE_AND_QUIT_BUTTON_OBJ_NAME[] = "SaveAndQuitButton";
// Save and Quit button object name
const Char CUIExitWindow::QUIT_BUTTON_OBJ_NAME[] = "QuitButton";
// Quit game text object name
const Char CUIExitWindow::QUIT_TEXT_OBJ_NAME[] = "QuitText";
// In-game quit game text object name
const Char CUIExitWindow::INGAME_QUIT_TEXT_OBJ_NAME[] = "InGameQuitText";

//=============================================
// @brief Constructor
//
//=============================================
CUIExitWindow::CUIExitWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIWindow(UIW_FL_MENUWINDOW, flags, width, height, originx, originy)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIExitWindow::~CUIExitWindow( void )
{
	// Probably destroyed by the UI manager
	if(m_pInstance)
		m_pInstance = nullptr;
}

//=============================================
// @brief Creates an instance of the console window
//
//=============================================
CUIExitWindow* CUIExitWindow::CreateInstance( bool isIngame )
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
	CUIExitWindow* pWindow = new CUIExitWindow(pWindowObject->getFlags(), pWindowObject->getWidth(), pWindowObject->getHeight(), xOrigin, yOrigin);
	if(!pWindow->init(pWinDesc, pWindowObject, isIngame))
	{
		Con_EPrintf("Failed to initialize exit window.\n");
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
void CUIExitWindow::DestroyInstance( void )
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
CUIExitWindow* CUIExitWindow::GetInstance( void )
{
	return m_pInstance;
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUIExitWindow::init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject, bool isIngame )
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

	CUIExitWindowEvent* pEvent = new CUIExitWindowEvent(this, EXIT_BTN_CANCEL);
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
		Con_EPrintf("Failed to initialize button object for exit UI window.\n");
		return false;
	}

	// Create the "save and quit" button
	if(isIngame)
	{
		const ui_objectinfo_t* pSaveAndQuitButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, SAVE_AND_QUIT_BUTTON_OBJ_NAME);
		if(!pSaveAndQuitButtonObjectInfo)
		{
			Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SAVE_AND_QUIT_BUTTON_OBJ_NAME);
			return false;
		}

		pEvent = new CUIExitWindowEvent(this, EXIT_BTN_SAVE_AND_QUIT);
		CUIButton* pSaveAndQuitBtn = new CUIButton(pSaveAndQuitButtonObjectInfo->getFlags(), 
			pSaveAndQuitButtonObjectInfo->getText().c_str(), 
			pSaveAndQuitButtonObjectInfo->getFont(), pEvent, 
			pSaveAndQuitButtonObjectInfo->getWidth(), 
			pSaveAndQuitButtonObjectInfo->getHeight(), 
			pWindowObject->getXInset() + pSaveAndQuitButtonObjectInfo->getXOrigin(), 
			pWindowObject->getYInset() + pSaveAndQuitButtonObjectInfo->getYOrigin());
		pSaveAndQuitBtn->setParent(this);
	
		if(!pSaveAndQuitBtn->init(pSaveAndQuitButtonObjectInfo->getSchema().c_str()))
		{
			Con_EPrintf("Failed to initialize button object for exit UI window.\n");
			return false;
		}
	}

	// Get quit button description
	const ui_objectinfo_t* pQuitButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, QUIT_BUTTON_OBJ_NAME);
	if(!pQuitButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, QUIT_BUTTON_OBJ_NAME);
		return false;
	}

	// Create the "Quit" button
	pEvent = new CUIExitWindowEvent(this, EXIT_BTN_QUIT);
	CUIButton* pQuitBtn = new CUIButton(pQuitButtonObjectInfo->getFlags(), 
		pQuitButtonObjectInfo->getText().c_str(), 
		pQuitButtonObjectInfo->getFont(), pEvent, 
		pQuitButtonObjectInfo->getWidth(), 
		pQuitButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pQuitButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pQuitButtonObjectInfo->getYOrigin());
	pQuitBtn->setParent(this);
	
	if(!pQuitBtn->init(pQuitButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for exit UI window.\n");
		return false;
	}

	// Get text object
	CString objName = isIngame ? INGAME_QUIT_TEXT_OBJ_NAME : QUIT_TEXT_OBJ_NAME;
	const ui_objectinfo_t* pQuitTextObjectInfo = pWinDesc->getObject(UI_OBJECT_TEXT, objName.c_str());
	if(!pQuitTextObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, objName.c_str());
		return false;
	}

	// Set appropriate text
	CUIText* pText = new CUIText(pQuitTextObjectInfo->getFlags(), 
		pQuitTextObjectInfo->getFont(), 
		pQuitTextObjectInfo->getText().c_str(), 
		pWindowObject->getXInset() + pQuitTextObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pQuitTextObjectInfo->getYOrigin());
	pText->setParent(this);

	return true;
}

//=============================================
// @brief Manages a button event
//
//=============================================
void CUIExitWindow::buttonPressed( Uint32 buttonId )
{
	switch(buttonId)
	{
	case EXIT_BTN_CANCEL:
		CUIExitWindow::DestroyInstance();
		break;
	case EXIT_BTN_SAVE_AND_QUIT:
		{
			gCommands.AddCommand("save");
			ens.exit = true;
		}
		break;
	case EXIT_BTN_QUIT:
		{
			ens.exit = true;
		}
		break;
	}
}