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
#include "uiconsolewindow.h"

#include "console.h"
#include "window.h"

#include "r_basicdraw.h"
#include "r_main.h"
#include "system.h"
#include "r_common.h"

// Current instance of the window
CUIConsoleWindow* CUIConsoleWindow::m_pInstance = nullptr;

// Console window relative width
static const Uint32 CONSOLE_RELATIVE_SCREEN_WIDTH = 1920;
// Console window relative height
static const Uint32 CONSOLE_RELATIVE_SCREEN_HEIGHT = 1080;

// Window description file
const Char CUIConsoleWindow::WINDOW_DESC_FILE[] = "consolewindow.txt";
// Window description file
const Char CUIConsoleWindow::WINDOW_OBJ_NAME[] = "ConsoleWindow";
// Execute button object name
const Char CUIConsoleWindow::EXECUTE_BUTTON_OBJ_NAME[] = "ExecuteButton";
// Input tab object name
const Char CUIConsoleWindow::INPUT_TAB_OBJ_NAME[] = "InputTab";
// History tab object name
const Char CUIConsoleWindow::HISTORY_TAB_OBJ_NAME[] = "HistoryTab";

//=============================================
// @brief Constructor
//
//=============================================
CUIConsoleWindow::CUIConsoleWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIWindow( (UIW_FL_MENUWINDOW|UIW_FL_CONSOLEWINDOW), flags, width, height, originx, originy ),
	m_pHistoryTab(nullptr),
	m_pInputTab(nullptr),
	m_inputWasSaved(false),
	m_tabbingFilterSet(false),
	m_prevResolutionWidth(0),
	m_prevResolutionHeight(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIConsoleWindow::~CUIConsoleWindow( void )
{
	// Probably destroyed by the UI manager
	if(m_pInstance)
		m_pInstance = nullptr;
}

//=============================================
// @brief Creates an instance of the console window
//
//=============================================
CUIConsoleWindow* CUIConsoleWindow::CreateInstance( void )
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

	Uint32 relativeWidth = R_GetRelativeX(pWindowObject->getWidth(), CONSOLE_RELATIVE_SCREEN_WIDTH, gWindow.GetWidth());
	Uint32 relativeHeight = R_GetRelativeX(pWindowObject->getHeight(), CONSOLE_RELATIVE_SCREEN_HEIGHT, gWindow.GetHeight());

	// Position to right of the screen
	Int32 xOrigin = gWindow.GetWidth() - relativeWidth - 20;
	Int32 yOrigin = 40;

	// Create the main tab object
	CUIConsoleWindow* pConsoleWindow = new CUIConsoleWindow(pWindowObject->getFlags(), pWindowObject->getWidth(), pWindowObject->getHeight(), xOrigin, yOrigin);
	if(!pConsoleWindow->init(pWinDesc, pWindowObject))
	{
		Con_EPrintf("Failed to initialize console window.\n");
		delete pConsoleWindow;
		return nullptr;
	}

	Int32 xAdjust = 0;
	if(relativeWidth != pWindowObject->getWidth())
		xAdjust = relativeWidth - pWindowObject->getWidth();

	Int32 yAdjust = 0;
	if(relativeHeight != pWindowObject->getHeight())
		yAdjust = relativeHeight - pWindowObject->getHeight();

	if(xAdjust || yAdjust)
		pConsoleWindow->adjustSize(xAdjust, yAdjust);

	// Set pointer
	m_pInstance = pConsoleWindow;
	// Add to UI manager to handle
	gUIManager.AddWindow(pConsoleWindow);

	return m_pInstance;
}

//=============================================
// @brief Destroys the current instance of the console window
//
//=============================================
void CUIConsoleWindow::DestroyInstance( void )
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
CUIConsoleWindow* CUIConsoleWindow::GetInstance( void )
{
	return m_pInstance;
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUIConsoleWindow::init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject )
{
	// Save current resolution
	m_prevResolutionWidth = gWindow.GetWidth();
	m_prevResolutionHeight = gWindow.GetHeight();

	const Char* pstrWindowSchemaFile = pWindowObject->getSchema().c_str();
	if(!CUIWindow::init(pstrWindowSchemaFile))
	{
		Con_EPrintf("Failed to initialize schema '%s' for console window.\n", pstrWindowSchemaFile);
		return false;
	}

	// Set the window's properties
	setTitle(pWindowObject->getTitle().c_str(), pWindowObject->getFont(), pWindowObject->getTitleXInset(), pWindowObject->getTitleYInset());
	setAlpha(pWindowObject->getAlpha());

	// Create the dragger
	if(pWindowObject->hasDragger())
	{
		CUIDragger* pDragger = new CUIDragger(UIEL_FL_FIXED_H, m_width-40, 30, 0, 0);
		pDragger->setParent(this);
	}

	// Create the resizer
	if(pWindowObject->isResizable())
	{
		CUIResizer* pResizer = new CUIResizer((UIEL_FL_ALIGN_B|UIEL_FL_ALIGN_R), 32, 32, 0, 0);
		pResizer->setParent(this);
	}

	// Create the execute button
	const ui_objectinfo_t* pExecuteButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, EXECUTE_BUTTON_OBJ_NAME);
	if(!pExecuteButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, EXECUTE_BUTTON_OBJ_NAME);
		return false;
	}

	CUIConsoleExecuteCallback* pAction = new CUIConsoleExecuteCallback(this);
	CUIButton* pExecuteButton = new CUIButton(pExecuteButtonObjectInfo->getFlags(), 
		pExecuteButtonObjectInfo->getText().c_str(), 
		pExecuteButtonObjectInfo->getFont(), pAction, 
		pExecuteButtonObjectInfo->getWidth(), 
		pExecuteButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pExecuteButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pExecuteButtonObjectInfo->getYOrigin());

	pExecuteButton->setParent(this);
	
	if(!pExecuteButton->init(pExecuteButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to set schema '%s' for the console window.\n", pExecuteButtonObjectInfo->getSchema().c_str());
		return false;
	}

	pExecuteButton->setAlpha(pExecuteButtonObjectInfo->getAlpha());

	// Create the input tab
	const ui_objectinfo_t* pInputTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, INPUT_TAB_OBJ_NAME);
	if(!pInputTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, INPUT_TAB_OBJ_NAME);
		return false;
	}

	CUIConsoleKeyEventCallback* pCallback = new CUIConsoleKeyEventCallback(this);
	m_pInputTab = new CUITextInputTab(pInputTabObjectInfo->getFlags(), 
		pCallback, 
		pInputTabObjectInfo->getFont(), 
		pInputTabObjectInfo->getWidth(), 
		pInputTabObjectInfo->getHeight(), 
		pWindowObject->getYInset() + pInputTabObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pInputTabObjectInfo->getYOrigin());

	m_pInputTab->setParent(this);

	if(!m_pInputTab->init(pInputTabObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to set schema '%s' for the console window.\n", pInputTabObjectInfo->getSchema().c_str());
		return false;
	}

	m_pInputTab->setAlpha(pInputTabObjectInfo->getAlpha());
	m_pInputTab->setTextInset(pInputTabObjectInfo->getTextInset());

	// Set by default
	setWindowInputFocusObject(m_pInputTab);

	// Create the text history tab
	const ui_objectinfo_t* pHistoryTabObjectInfo = pWinDesc->getObject(UI_OBJECT_TAB, HISTORY_TAB_OBJ_NAME);
	if(!pHistoryTabObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, HISTORY_TAB_OBJ_NAME);
		return false;
	}

	m_pHistoryTab = new CUITextTab(pHistoryTabObjectInfo->getFlags(), 
		pHistoryTabObjectInfo->getFont(), 
		pHistoryTabObjectInfo->getWidth(), 
		pHistoryTabObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pHistoryTabObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pHistoryTabObjectInfo->getYOrigin());
	m_pHistoryTab->setParent(this);
	
	if(!m_pHistoryTab->init(pHistoryTabObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to set schema '%s' for the console window.\n", pHistoryTabObjectInfo->getSchema().c_str());
		return false;
	}

	m_pHistoryTab->setAlpha(pHistoryTabObjectInfo->getAlpha());
	m_pHistoryTab->setTextInset(pHistoryTabObjectInfo->getTextInset());

	// Get current text from the console
	const Char* pstrHistory = gConsole.GetConsoleHistory();
	m_pHistoryTab->setText(pstrHistory);

	return true;
}

//=============================================
// @brief Executes the string as a command
//
//=============================================
void CUIConsoleWindow::Execute( void )
{
	// Send it to the console and clear
	gConsole.ProcessInput(m_pInputTab->getText());
	m_pInputTab->clearText();

	if(m_inputWasSaved)
	{
		m_inputWasSaved = false;
		m_savedInput.clear();
	}

	ResetTabFilter();
}

//=============================================
// @brief Retrieves the best input choice from the console list
//
//=============================================
void CUIConsoleWindow::GetNextBestInputChoice( void )
{
	const Char* pstrInput = m_pInputTab->getText();
	if(!m_tabbingFilterSet)
	{
		m_tabbingFilter = pstrInput;
		m_tabbingFilterSet = true;
	}

	const Char* pstrBestChoice = gConsole.GetBestInputChoice(m_tabbingFilter.c_str());
	if(!pstrBestChoice)
		return;

	CString bestChoice(pstrBestChoice);
	bestChoice += ' ';
	m_pInputTab->setText(bestChoice.c_str());
}

//=============================================
// @brief Resets the tabbing filtering string
//
//=============================================
void CUIConsoleWindow::ResetTabFilter( void )
{
	if(!m_tabbingFilterSet)
		return;

	m_tabbingFilter.clear();
	m_tabbingFilterSet = false;
}

//=============================================
// @brief Sets the history tab contents
//
//=============================================
void CUIConsoleWindow::SetHistoryText( const Char* pstrText )
{
	m_pHistoryTab->setText(pstrText);
}

//=============================================
// @brief Saves the currently present input
//
//=============================================
void CUIConsoleWindow::SaveInputText( void )
{
	if(m_inputWasSaved)
		return;

	m_savedInput = m_pInputTab->getText();
	m_inputWasSaved = true;
}

//=============================================
// @brief Restores the input text from the container
//
//=============================================
bool CUIConsoleWindow::RestoreInputText( void )
{
	if(!m_inputWasSaved)
		return false;

	m_pInputTab->setText(m_savedInput.c_str());
	m_savedInput.clear();
	m_inputWasSaved = false;
	return true;
}

//=============================================
// @brief Clears the input text from the container
//
//=============================================
void CUIConsoleWindow::ClearInputText( void )
{
	m_pInputTab->setText(nullptr);
}

//=============================================
// @brief Sets the input text for the container
//
//=============================================
void CUIConsoleWindow::SetInputText( const Char* pstrText )
{
	m_pInputTab->setText(pstrText);
}

//=============================================
// @brief Destructor
//
//=============================================
void CUIConsoleWindow::onGLInitialization( void )
{
	Uint32 referenceWidth = m_prevResolutionWidth;
	m_prevResolutionWidth = gWindow.GetWidth();

	Uint32 referenceHeight = m_prevResolutionHeight;
	m_prevResolutionHeight = gWindow.GetHeight();

	Uint32 newWidth = R_GetRelativeX(m_width, referenceWidth, gWindow.GetWidth());
	Uint32 newHeight = R_GetRelativeY(m_height, referenceHeight, gWindow.GetHeight());

	Int32 widthAdd = newWidth - m_width;
	Int32 heightAdd = newHeight - m_height;

	adjustSize(widthAdd, heightAdd);
}

//=============================================
// @brief Destructor
//
//=============================================
void CUIConsoleExecuteCallback::PerformAction( Float param )
{
	m_pWindow->Execute();
}

//=============================================
// @brief Destructor
//
//=============================================
bool CUIConsoleKeyEventCallback::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	// Don't process releases
	if(!keyDown)
		return true;

	// Get SDL Keycode
	SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)button);
	if(sdlKeycode >= SDLK_SPACE && sdlKeycode <= SDLK_z || button == SDL_SCANCODE_BACKSPACE)
	{
		m_pWindow->ResetTabFilter();
		return true;
	}

	// Handle special keys
	switch(sdlKeycode)
	{
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			{
				// Clear this
				m_lastButton = 0;
				m_pWindow->Execute();
				return true;
			}
			break;
		case SDLK_UP:
			{
				// Step twice if it's not the same key
				if(m_lastButton != 0 && m_lastButton != button)
					gConsole.HistoryStepForward();

				// Reset if we're at the end
				if(gConsole.IsAtHistoryEnd())
					gConsole.ResetHistoryIterator();

				const Char* pstrInput = gConsole.GetCurrentInputHistory();
				if(pstrInput)
				{
					// Try to save anything
					m_pWindow->SaveInputText();
					m_pWindow->SetInputText(pstrInput);
					
					// Advance forward
					gConsole.HistoryStepForward();
					m_lastButton = button;

					m_pWindow->ResetTabFilter();
				}
			}
			break;
		case SDLK_DOWN:
			{
				// Step twice if it's not the same key
				if(m_lastButton != 0 && m_lastButton != button)
					gConsole.HistoryStepBack();

				const Char* pstrInput = gConsole.GetCurrentInputHistory();
				if(pstrInput)
				{
					// Try to save anything
					m_pWindow->SaveInputText();
					m_pWindow->SetInputText(pstrInput);
					
					// Advance forward
					gConsole.HistoryStepBack();
					m_lastButton = button;

					m_pWindow->ResetTabFilter();
				}
				else
				{
					// Try to restore anything
					if(!m_pWindow->RestoreInputText())
						m_pWindow->ClearInputText();

					// Reset this
					m_lastButton = 0;
				}
			}
			break;
		case SDLK_TAB:
			{
				// Grab from the ordered link list
				m_pWindow->GetNextBestInputChoice();
			}
			break;
	}

	return true;
}