/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "huddraw.h"
#include "r_interface.h"
#include "clientdll.h"
#include "fontset.h"
#include "hud.h"

#include "gameuiobjectiveswindow.h"
#include "gameuiwindows_shared.h"
#include "snd_shared.h"
#include "gameui_shared.h"

// Object x inset for objectives window
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_TAB_X_INSET = 30;
// Object y inset for objectives window
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_TAB_Y_INSET = 60;
// Object y spacing for objectives window
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_TAB_Y_SPACING = 20;
// Object x spacing for objectives window
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_TAB_X_SPACING = 80;
// Text inset for objectives window
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_TAB_TEXT_INSET = 10;
// Default text color
const color32_t CGameUIObjectivesWindow::OBJECTIVESWINDOW_TEXT_COLOR = color32_t(255, 255, 255, 255);
// Height of the title surface
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_TITLE_SURFACE_HEIGHT = 50;
// Height of the button surface
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_BUTTON_SURFACE_HEIGHT = 350;
// Height of the info surface
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_INFO_SURFACE_HEIGHT = 200;
// Height of the exit button surface
const Uint32 CGameUIObjectivesWindow::OBJECTIVESWINDOW_EXIT_BUTTON_SURFACE_HEIGHT = 100;
// Number of objective buttons
const Uint32 CGameUIObjectivesWindow::MAX_NB_OBJECTIVE_BUTTONS = 32;
// Script subfolder name
const Char CGameUIObjectivesWindow::OBJECTIVESWINDOW_SCRIPT_SUBFOLDER_NAME[] = "objectives";
// Base script name
const Char CGameUIObjectivesWindow::OBJECTIVESWINDOW_SCRIPT_NAME[] = "defaults.txt";
// Color of highlighted buttons for this window
const color32_t CGameUIObjectivesWindow::OBJECTIVESWINDOW_BUTTON_NEW_COLOR = color32_t(0, 255, 0, 100);
// Title text default schema set name
const Char CGameUIObjectivesWindow::OBJECTIVESWINDOW_TITLE_TEXTSCHEMA_NAME[] = "objectivestitle";
// Text default font schema name
const Char CGameUIObjectivesWindow::OBJECTIVESWINDOW_TEXTSCHEMA_NAME[] = "objectivestext";

//====================================
//
//====================================
CGameUIObjectivesWindow::CGameUIObjectivesWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIWindow(flags, originX, originY, width, height),
	m_pWindowTitleText(nullptr),
	m_pDefaultDescription(nullptr),
	m_pExitButton(nullptr),
	m_pCurrentObjective(nullptr),
	m_newObjectivesBitflags(0),
	m_usedEdgeThickness(0),
	m_buttonXPos(0),
	m_buttonYPos(0),
	m_buttonWidth(0),
	m_buttonHeight(0),
	m_infoSurfaceHeight(0),
	m_textInset(0),
	m_tabYSpacing(0),
	m_tabWidth(0),
	m_pButtonsTab(nullptr),
	m_pInfoTab(nullptr),
	m_pFontSet(nullptr)
{
}

//====================================
//
//====================================
CGameUIObjectivesWindow::~CGameUIObjectivesWindow( void )
{
}

//====================================
//
//====================================
void CGameUIObjectivesWindow::init( void )
{
	// Init basic window elements
	Uint32 verticalbarheight, middlebarwidth, barThickness;
	CGameUIWindow::initBackground(verticalbarheight, middlebarwidth, barThickness);

	Uint32 tabTopInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_TOP_INSET);
	Uint32 hBarYOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_H_BAR_Y_ORIGIN);
	Uint32 tabSideInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_SIDE_INSET);
	Uint32 mainTabMaxWidth = gHUDDraw.ScaleX(GAMEUIWINDOW_MAIN_TAB_MAX_WIDTH);
	Uint32 titleSurfaceHeight = gHUDDraw.ScaleY(OBJECTIVESWINDOW_TITLE_SURFACE_HEIGHT);
	Uint32 buttonSurfaceHeight = gHUDDraw.ScaleY(OBJECTIVESWINDOW_BUTTON_SURFACE_HEIGHT);
	Uint32 exitButtonSurfaceHeight = gHUDDraw.ScaleY(OBJECTIVESWINDOW_EXIT_BUTTON_SURFACE_HEIGHT);
	Uint32 tabXSpacing = gHUDDraw.ScaleX(OBJECTIVESWINDOW_TAB_X_SPACING);

	m_usedEdgeThickness = gHUDDraw.ScaleX(GAMEUIWINDOW_EDGE_THICKNESS);
	m_infoSurfaceHeight = gHUDDraw.ScaleY(OBJECTIVESWINDOW_INFO_SURFACE_HEIGHT);
	m_tabYSpacing = gHUDDraw.ScaleY(OBJECTIVESWINDOW_TAB_Y_SPACING);
	m_textInset = gHUDDraw.ScaleY(OBJECTIVESWINDOW_TAB_TEXT_INSET);

	//
	// Create the title text object
	//
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	const font_set_t* pTitleFont = cl_engfuncs.pfnGetResolutionSchemaFontSet(OBJECTIVESWINDOW_TITLE_TEXTSCHEMA_NAME, screenHeight);
	if(!pTitleFont)
		pTitleFont = gGameUIManager.GetDefaultFontSet();

	//
	// Create the tab objects
	//
	m_tabWidth = middlebarwidth - barThickness*2 - tabSideInset*2;
	if(m_tabWidth > mainTabMaxWidth)
		m_tabWidth = mainTabMaxWidth;

	Int32 tabOriginX = m_width / 2.0 - m_tabWidth / 2.0;
	Int32 tabOriginY = hBarYOrigin + barThickness + tabTopInset;

	// Create the title tab
	CGameUISurface* pTitleTab = new CGameUISurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		titleSurfaceHeight);
	pTitleTab->setParent(this);

	// Add the title for the window
	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(OBJECTIVESWINDOW_TEXTSCHEMA_NAME, screenHeight);
	if (!m_pFontSet)
		m_pFontSet = gGameUIManager.GetDefaultFontSet();

	// Create the buttons tab
	tabOriginY += titleSurfaceHeight;
	m_pButtonsTab = new CGameUIScrollableSurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		m_pFontSet,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		buttonSurfaceHeight,
		m_tabYSpacing);
	m_pButtonsTab->setParent(this);

	// Create the info tab
	tabOriginY += buttonSurfaceHeight;
	m_pInfoTab = new CGameUISurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		m_infoSurfaceHeight);
	m_pInfoTab->setParent(this);

	// Create the exit button tab
	tabOriginY += m_infoSurfaceHeight;
	CGameUISurface* pExitButtonTab = new CGameUISurface(CGameUIObject::FL_NONE,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		m_tabWidth,
		exitButtonSurfaceHeight);
	pExitButtonTab->setParent(this);

	m_pWindowTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH|CGameUIObject::FL_ALIGN_CV,
		OBJECTIVESWINDOW_TEXT_COLOR,
		m_pFontSet,
		0,
		0);
	m_pWindowTitleText->setParent(pTitleTab);

	// Create the buttons
	Uint32 buttonWidth = m_buttonWidth = m_tabWidth - 2*tabXSpacing;
	Uint32 buttonHeight = m_buttonHeight = (buttonSurfaceHeight - m_tabYSpacing * 7)/6;

	Int32 buttonXPos = m_buttonXPos = tabXSpacing;
	Int32 buttonYPos = m_buttonYPos = m_tabYSpacing;

	// Create default description
	m_pDefaultDescription = new CGameUIText(CGameUIObject::FL_NONE,
		OBJECTIVESWINDOW_TEXT_COLOR,
		m_pFontSet,
		"",
		0,
		0,
		m_tabWidth,
		m_infoSurfaceHeight,
		m_textInset);
	m_pDefaultDescription->setParent(m_pInfoTab);
	m_pDefaultDescription->setVisible(true);

	// Create the exit button
	buttonYPos = m_tabYSpacing;
	buttonHeight = exitButtonSurfaceHeight - m_tabYSpacing * 2;

	CGameUIObjectivesWindowCallbackEvent* pClearEvent = new CGameUIObjectivesWindowCallbackEvent(this,  OBJ_BUTTON_EXIT);
	CGameUIButton* pClearButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pClearEvent,
		SDLK_DELETE,
		m_usedEdgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		buttonXPos,
		buttonYPos,
		buttonWidth,
		buttonHeight);
	pClearButton->setParent(pExitButtonTab);
	pClearButton->setText("Exit");
}

//====================================
//
//====================================
void CGameUIObjectivesWindow::think( void )
{
	if(!m_pCurrentObjective)
	{
		bool hasVisible = false;

		for (Uint32 i = 0; i < m_buttonsArray.size(); i++)
		{
			if (!m_buttonsArray[i].pButton->isVisible())
				continue;

			if (m_pButtonsTab->isMouseOverChild(m_buttonsArray[i].pButton))
			{
				if (!m_buttonsArray[i].pDescription->isVisible())
					m_buttonsArray[i].pDescription->setVisible(true);

				hasVisible = true;
			}
			else
			{
				if (m_buttonsArray[i].pDescription->isVisible())
					m_buttonsArray[i].pDescription->setVisible(false);
			}
		}

		if (!hasVisible && !m_pDefaultDescription->isVisible())
			m_pDefaultDescription->setVisible(true);
		else if (hasVisible && m_pDefaultDescription->isVisible())
			m_pDefaultDescription->setVisible(false);

	}

	// Call base class to manage think functions
	CGameUIWindow::think();
}

//====================================
//
//====================================
bool CGameUIObjectivesWindow::initData( const CArray<CString>& objectivesArray, const Char* pstrActiveObjectiveName, Int32& newObjectivesBitflags )
{
	// Load the base script
	CString scriptFilePath;
	scriptFilePath << GAMEUI_SCRIPT_BASE_PATH << PATH_SLASH_CHAR << OBJECTIVESWINDOW_SCRIPT_SUBFOLDER_NAME << PATH_SLASH_CHAR << OBJECTIVESWINDOW_SCRIPT_NAME;

	const byte* pfile = cl_filefuncs.pfnLoadFile(scriptFilePath.c_str(), nullptr);
	if(!pfile)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Failed to load '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
		return false;
	}

	CString windowTitle;
	CString defaultDescription;

	CString token;
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		// Read the token in
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
			break;

		if(!qstrcmp(token, "$title"))
		{
			// Read in the token
			pstr = Common::Parse(pstr, token);
			windowTitle = token;
			continue;
		}
		else if(!qstrcmp(token, "$defaultdescription"))
		{
			// Next token should be an openig bracket
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			if(qstrcmp(token, "{"))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Expected '{', got %s instead in definition in '%s'.\n", __FUNCTION__, token.c_str(), scriptFilePath.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			// Skip whitespaces to first line
			while(*pstr && SDL_isspace(*pstr))
				pstr++;
					
			// Find the ending bracket
			const Char* pstrend = qstrstr(pstr, "}");
			if(!pstrend)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			// Assign contents
			Uint32 length = pstrend - pstr;
			defaultDescription.assign(pstr, length);

			// Erase any tabulations
			while(true)
			{
				Uint32 tabpos = defaultDescription.find(0, "\t");
				if(tabpos == -1)
					break;

				defaultDescription.erase(tabpos, 1);
			}
			
			// Set pointer
			pstr = pstrend+1;
		}
		else
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unknown token '$s' in '%s'.\n", __FUNCTION__, token.c_str(), scriptFilePath.c_str());
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}
	}
	cl_filefuncs.pfnFreeFile(pfile);

	// Check for errors
	if(windowTitle.empty())
	{
		cl_engfuncs.pfnCon_EPrintf("%s - '$title' was not specified in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
		windowTitle = "NULL";
	}

	// Check for errors
	if(defaultDescription.empty())
	{
		cl_engfuncs.pfnCon_EPrintf("%s - '$defaultdescription' was not specified in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
		defaultDescription = "NULL";
	}

	m_pWindowTitleText->setText(windowTitle.c_str());
	m_pDefaultDescription->setText(defaultDescription.c_str());
	m_newObjectivesBitflags = newObjectivesBitflags;

	Int32 buttonXPos = m_buttonXPos;
	Int32 buttonYPos = m_buttonYPos;

	// Track separately to avoid issues if a file is missing/bad
	Uint32 nbAdded = 0;
	for(Uint32 i = 0; i < objectivesArray.size(); i++)
	{
		scriptFilePath.clear();;
		scriptFilePath << GAMEUI_SCRIPT_BASE_PATH << PATH_SLASH_CHAR << "objectives" << PATH_SLASH_CHAR << objectivesArray[i] << ".txt";

		const byte* pobjfile = cl_filefuncs.pfnLoadFile(scriptFilePath.c_str(), nullptr);
		if(!pobjfile)
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Failed to load '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
			continue;
		}

		pstr = reinterpret_cast<const Char*>(pobjfile);

		// Read the token in
		pstr = Common::Parse(pstr, token);

		// First token should be "$title"
		if(qstrcmp(token, "$title"))
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unknown token '$s' in '%s', expected '$title'.\n", __FUNCTION__, token.c_str(), scriptFilePath.c_str());
			cl_filefuncs.pfnFreeFile(pobjfile);
			continue;
		}

		// Read in the objective's title
		pstr = Common::Parse(pstr, token);
		if(!pstr)
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading objective definition in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
			cl_filefuncs.pfnFreeFile(pobjfile);
			continue;
		}

		// Save description
		CString objectiveTitle = token;

		// Next token should be an openig bracket
		pstr = Common::Parse(pstr, token);
		if(!pstr)
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
			cl_filefuncs.pfnFreeFile(pobjfile);
			continue;
		}

		if(qstrcmp(token, "{"))
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Expected '{', got %s instead in definition in '%s'.\n", __FUNCTION__, token.c_str(), scriptFilePath.c_str());
			cl_filefuncs.pfnFreeFile(pobjfile);
			continue;
		}

		// Skip whitespaces to first line
		while(*pstr && SDL_isspace(*pstr))
			pstr++;
					
		// Find the ending bracket
		const Char* pstrend = qstrstr(pstr, "}");
		if(!pstrend)
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, scriptFilePath.c_str());
			cl_filefuncs.pfnFreeFile(pobjfile);
			continue;
		}

		// Assign contents
		CString objectiveDescription;
		Uint32 length = pstrend - pstr;
		objectiveDescription.assign(pstr, length);

		// Erase any tabulations
		while(true)
		{
			Uint32 tabpos = objectiveDescription.find(0, "\t");
			if(tabpos == -1)
				break;

			objectiveDescription.erase(tabpos, 1);
		}

		// Assign button contents and such
		Uint32 addIndex = nbAdded;
		m_buttonsArray.resize(nbAdded + 1);
		nbAdded++;

		button_t& newButton = m_buttonsArray[addIndex];

		CGameUIObjectivesWindowCallbackEvent* pEvent = new CGameUIObjectivesWindowCallbackEvent(this, OBJ_BUTTON_0 + addIndex);
		newButton.pButton = new CGameUIButton(CGameUIObject::FL_NONE,
			pEvent,
			m_usedEdgeThickness,
			GAMEUIWINDOW_MAIN_TAB_COLOR,
			GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
			GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
			buttonXPos,
			buttonYPos,
			m_buttonWidth,
			m_buttonHeight);
		newButton.pButton->setParent(m_pButtonsTab);
		newButton.pButton->setVisible(false);

		newButton.pDescription = new CGameUITextTab(CGameUIObject::FL_NONE,
			m_pFontSet,
			m_textInset,
			m_usedEdgeThickness,
			GAMEUIWINDOW_MAIN_TAB_COLOR,
			GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
			GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
			GAMEUIWINDOW_DEFAULT_TEXT_COLOR,
			0,
			0,
			m_tabWidth,
			m_infoSurfaceHeight);

		newButton.pDescription->setParent(m_pInfoTab);
		newButton.pDescription->setVisible(false);

		// Set button properties
		buttonYPos += m_buttonHeight + m_tabYSpacing;

		newButton.objectiveName = objectivesArray[i];
		newButton.pButton->setText(objectiveTitle.c_str());
		newButton.pDescription->initData((const byte*)objectiveDescription.c_str(), objectiveDescription.length());
		newButton.pButton->setVisible(true);

		// Restore currently selected objective
		if(pstrActiveObjectiveName && !qstrcmp(objectivesArray[i], pstrActiveObjectiveName))
			m_pCurrentObjective = &newButton;
		else if(m_newObjectivesBitflags & (1<<i))
			newButton.pButton->setBgColor(OBJECTIVESWINDOW_BUTTON_NEW_COLOR);

		// Set pointer
		pstr = pstrend+1;

		// There should be nothing else here
		pstr = Common::Parse(pstr, token);
		if(pstr)
		{
			pstr = Common::Parse(pstr, token);
			cl_engfuncs.pfnCon_EPrintf("%s - Unexpected %s after end of definition in '%s'.\n", __FUNCTION__, token.c_str(), scriptFilePath.c_str());
		}

		// Close file
		cl_filefuncs.pfnFreeFile(pobjfile);
	}

	// Clear "new objective" flag
	gHUD.SetNewObjective(false);

	return true;
}

//====================================
//
//====================================
void CGameUIObjectivesWindow::getInformation( CArray<CString>& objectivesArray, CString& activeObjectiveName, Int32& newObjectivesBitflags ) const
{
	if(m_pCurrentObjective)
		activeObjectiveName = m_pCurrentObjective->objectiveName;

	for(Uint32 i = 0; i < m_buttonsArray.size(); i++)
		objectivesArray.push_back(m_buttonsArray[i].objectiveName);

	newObjectivesBitflags = m_newObjectivesBitflags;
}

//====================================
//
//====================================
void CGameUIObjectivesWindow::ManageEvent( objectivesbuttonevent_t event )
{
	if (event >= OBJ_BUTTON_0)
	{
		Int32 buttonIndex = event - OBJ_BUTTON_0;
		button_t& button = m_buttonsArray[buttonIndex];
		if (!button.pButton->isVisible())
			return;

		if (m_pCurrentObjective)
		{
			m_pCurrentObjective->pButton->setHighlighted(false);
			m_pCurrentObjective->pDescription->setVisible(false);
		}

		if (m_pCurrentObjective == &button)
		{
			m_pCurrentObjective = nullptr;
			m_pDefaultDescription->setVisible(true);
		}
		else
		{
			m_pCurrentObjective = &button;
			m_pCurrentObjective->pDescription->setVisible(true);
			m_pCurrentObjective->pButton->setHighlighted(true);

			if (m_newObjectivesBitflags & (1 << buttonIndex))
			{
				m_pCurrentObjective->pButton->setBgColor(GAMEUIWINDOW_MAIN_TAB_BG_COLOR);
				m_newObjectivesBitflags &= ~(1 << buttonIndex);

				Uint32 msgid = g_pGUIManager->GetServerUIMessageId();
				if (!msgid)
				{
					cl_engfuncs.pfnCon_Printf("%s - Message 'GameUIMessage' not registered on client.\n", __FUNCTION__);
					return;
				}

				cl_engfuncs.pfnClientUserMessageBegin(msgid);
				cl_engfuncs.pfnMsgWriteByte(GAMEUIEVENT_READ_OBJECTIVE);
				cl_engfuncs.pfnMsgWriteString(m_pCurrentObjective->objectiveName.c_str());
				cl_engfuncs.pfnClientUserMessageEnd();
			}
		}
	}
	else if(event == OBJ_BUTTON_EXIT)
	{
		m_windowFlags |= CGameUIWindow::FL_WINDOW_KILLME;
	}
	else
	{
		cl_engfuncs.pfnCon_Printf("%s - Unknown button event %d.\n", __FUNCTION__, event);
	}
}

//====================================
//
//====================================
void CGameUIObjectivesWindowCallbackEvent::PerformAction( Float param )
{
	if(!m_pWindow)
		return;

	m_pWindow->ManageEvent(m_eventType);
}