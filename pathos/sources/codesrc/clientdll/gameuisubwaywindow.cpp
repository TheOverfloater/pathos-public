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

#include "gameuisubwaywindow.h"
#include "gameuiwindows_shared.h"
#include "snd_shared.h"
#include "gameui_shared.h"

// Object x inset for login window
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_TAB_X_INSET = 30;
// Object y inset for login window
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_TAB_Y_INSET = 60;
// Object y spacing for login window
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_TAB_Y_SPACING = 30;
// Object x spacing for login window
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_TAB_X_SPACING = 80;
// Text inset for subway window
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_TAB_TEXT_INSET = 10;
// Default text color
const color32_t CGameUISubwayWindow::SUBWAYWINDOW_TEXT_COLOR = color32_t(255, 255, 255, 255);
// Height of the title surface
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_TITLE_SURFACE_HEIGHT = 100;
// Height of the button surface
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_BUTTON_SURFACE_HEIGHT = 300;
// Height of the info surface
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_INFO_SURFACE_HEIGHT = 200;
// Height of the exit button surface
const Uint32 CGameUISubwayWindow::SUBWAYWINDOW_EXIT_BUTTON_SURFACE_HEIGHT = 100;
// Number of destination buttons
const Uint32 CGameUISubwayWindow::NB_DESTINATION_BUTTONS = 4;
// Title text default schema set name
const Char CGameUISubwayWindow::SUBWAYWINDOW_TITLE_TEXTSCHEMA_NAME[] = "subwaytitle";
// Text default font schema name
const Char CGameUISubwayWindow::SUBWAYWINDOW_TEXTSCHEMA_NAME[] = "subwaytext";

//====================================
//
//====================================
CGameUISubwayWindow::CGameUISubwayWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIWindow(flags, originX, originY, width, height),
	m_pWindowTitleText(nullptr),
	m_pDefaultDescription(nullptr),
	m_pExitButton(nullptr),
	m_subwayFlags(0)
{
}

//====================================
//
//====================================
CGameUISubwayWindow::~CGameUISubwayWindow( void )
{
}

//====================================
//
//====================================
void CGameUISubwayWindow::init( void )
{
	// Init basic window elements
	Uint32 verticalbarheight, middlebarwidth, barThickness;
	CGameUIWindow::initBackground(verticalbarheight, middlebarwidth, barThickness);

	Uint32 tabTopInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_TOP_INSET);
	Uint32 hBarYOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_H_BAR_Y_ORIGIN);
	Uint32 tabSideInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_SIDE_INSET);
	Uint32 mainTabMaxWidth = gHUDDraw.ScaleX(GAMEUIWINDOW_MAIN_TAB_MAX_WIDTH);
	Uint32 edgeThickness = gHUDDraw.ScaleX(GAMEUIWINDOW_EDGE_THICKNESS);
	Uint32 titleSurfaceHeight = gHUDDraw.ScaleY(SUBWAYWINDOW_TITLE_SURFACE_HEIGHT);
	Uint32 buttonSurfaceHeight = gHUDDraw.ScaleY(SUBWAYWINDOW_BUTTON_SURFACE_HEIGHT);
	Uint32 infoSurfaceHeight = gHUDDraw.ScaleY(SUBWAYWINDOW_INFO_SURFACE_HEIGHT);
	Uint32 exitButtonSurfaceHeight = gHUDDraw.ScaleY(SUBWAYWINDOW_EXIT_BUTTON_SURFACE_HEIGHT);
	Uint32 tabXSpacing = gHUDDraw.ScaleX(SUBWAYWINDOW_TAB_X_SPACING);
	Uint32 tabYSpacing = gHUDDraw.ScaleY(SUBWAYWINDOW_TAB_Y_SPACING);
	Uint32 textInset = gHUDDraw.ScaleY(SUBWAYWINDOW_TAB_TEXT_INSET);

	//
	// Create the title text object
	//
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	const font_set_t* pTitleFont = cl_engfuncs.pfnGetResolutionSchemaFontSet(SUBWAYWINDOW_TITLE_TEXTSCHEMA_NAME, screenHeight);
	if(!pTitleFont)
		pTitleFont = gGameUIManager.GetDefaultFontSet();

	Uint32 textYOrigin = hBarYOrigin + tabTopInset/2.0f;
	CGameUIText *pTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH, GAMEUIWINDOW_DEFAULT_TEXT_COLOR, pTitleFont, 0, textYOrigin);
	pTitleText->setParent(this);

	//
	// Create the tab objects
	//
	Uint32 tabWidth = middlebarwidth - barThickness*2 - tabSideInset*2;
	if(tabWidth > mainTabMaxWidth)
		tabWidth = mainTabMaxWidth;

	Int32 tabOriginX = m_width / 2.0 - tabWidth / 2.0;
	Int32 tabOriginY = hBarYOrigin + barThickness + tabTopInset;

	// Create the title tab
	CGameUISurface* pTitleTab = new CGameUISurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		tabWidth,
		titleSurfaceHeight);
	pTitleTab->setParent(this);

	// Create the buttons tab
	tabOriginY += titleSurfaceHeight;
	CGameUISurface* pButtonsTab = new CGameUISurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		tabWidth,
		buttonSurfaceHeight);
	pButtonsTab->setParent(this);

	// Create the info tab
	tabOriginY += buttonSurfaceHeight;
	CGameUISurface* pInfoTab = new CGameUISurface(CGameUIObject::FL_NO_BOTTOM_BORDER,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		tabWidth,
		infoSurfaceHeight);
	pInfoTab->setParent(this);

	// Create the exit button tab
	tabOriginY += infoSurfaceHeight;
	CGameUISurface* pExitButtonTab = new CGameUISurface(CGameUIObject::FL_NONE,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		tabOriginX,
		tabOriginY,
		tabWidth,
		exitButtonSurfaceHeight);
	pExitButtonTab->setParent(this);

	// Add the title for the window
	const font_set_t* pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(SUBWAYWINDOW_TEXTSCHEMA_NAME, screenHeight);
	if(!pFontSet)
		pFontSet = gGameUIManager.GetDefaultFontSet();

	m_pWindowTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH|CGameUIObject::FL_ALIGN_CV,
		SUBWAYWINDOW_TEXT_COLOR,
		pFontSet,
		0,
		0);
	m_pWindowTitleText->setParent(pTitleTab);

	// Create the buttons
	Uint32 buttonWidth = tabWidth - 2*tabXSpacing;
	Uint32 buttonHeight = (buttonSurfaceHeight - tabYSpacing * 5)/4;

	Int32 buttonXPos = tabXSpacing;
	Int32 buttonYPos = tabYSpacing;

	// Allocate buttons
	m_buttonsArray.resize(NB_DESTINATION_BUTTONS);

	for(Uint32 i = 0; i < NB_DESTINATION_BUTTONS; i++)
	{
		CGameUISubwayWindowCallbackEvent* pEvent = new CGameUISubwayWindowCallbackEvent(this, SUBWAY_BUTTON_1+i);
		m_buttonsArray[i].pButton = new CGameUIButton(CGameUIObject::FL_NONE,
			pEvent, 
			edgeThickness, 
			GAMEUIWINDOW_MAIN_TAB_COLOR,
			GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
			GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
			buttonXPos,
			buttonYPos,
			buttonWidth,
			buttonHeight);
		m_buttonsArray[i].pButton->setParent(pButtonsTab);

		m_buttonsArray[i].pDescription = new CGameUIText(CGameUIObject::FL_NONE,
			SUBWAYWINDOW_TEXT_COLOR,
			pFontSet,
			"",
			0,
			0,
			tabWidth,
			infoSurfaceHeight,
			textInset);
		m_buttonsArray[i].pDescription->setParent(pInfoTab);
		m_buttonsArray[i].pDescription->setVisible(false);

		buttonYPos += buttonHeight + tabYSpacing;
	}

	// Create default description
	m_pDefaultDescription = new CGameUIText(CGameUIObject::FL_NONE,
		SUBWAYWINDOW_TEXT_COLOR,
		pFontSet,
		"",
		0,
		0,
		tabWidth,
		infoSurfaceHeight,
		textInset);
	m_pDefaultDescription->setParent(pInfoTab);
	m_pDefaultDescription->setVisible(true);

	// Create the exit button
	buttonYPos = tabYSpacing;
	buttonHeight = exitButtonSurfaceHeight - tabYSpacing * 2;

	CGameUISubwayWindowCallbackEvent* pClearEvent = new CGameUISubwayWindowCallbackEvent(this, SUBWAY_BUTTON_CANCEL);
	CGameUIButton* pClearButton = new CGameUIButton(CGameUIObject::FL_NONE, 
		pClearEvent,
		SDLK_DELETE,
		edgeThickness, 
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
void CGameUISubwayWindow::think( void )
{
	bool hasVisible = false;
	for(Uint32 i = 0; i < m_buttonsArray.size(); i++)
	{
		if(m_buttonsArray[i].pButton->isMouseOver())
		{
			if(!m_buttonsArray[i].pDescription->isVisible())
				m_buttonsArray[i].pDescription->setVisible(true);

			hasVisible = true;
		}
		else
		{
			if(m_buttonsArray[i].pDescription->isVisible())
				m_buttonsArray[i].pDescription->setVisible(false);
		}
	}

	if(!hasVisible && !m_pDefaultDescription->isVisible())
		m_pDefaultDescription->setVisible(true);
	else if(hasVisible && m_pDefaultDescription->isVisible())
		m_pDefaultDescription->setVisible(false);

	// Call base class to manage think functions
	CGameUIWindow::think();
}

//====================================
//
//====================================
bool CGameUISubwayWindow::initData( const Char* pstrScriptFile, Int32 flags )
{
	const byte* pfile = cl_filefuncs.pfnLoadFile(pstrScriptFile, nullptr);
	if(!pfile)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Failed to load '%s'.\n", __FUNCTION__, pstrScriptFile);
		return false;
	}

	CArray<subwaybuttonschema_t> buttonschemas;
	buttonschemas.resize(NB_SUBWAY_BUTTON_TYPES);

	CString token;
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		// Read the token in
		pstr = Common::Parse(pstr, token);

		if(!qstrcmp(token, "$title"))
		{
			// Read in the token
			pstr = Common::Parse(pstr, token);
			m_pWindowTitleText->setText(token.c_str());
			continue;
		}
		else if(!qstrcmp(token, "$button"))
		{
			// Read in the destination
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			subwaybuttonschematype_t type;
			if(!qstrcmp(token, "destination1"))
				type = SUBWAYWINDOW_BUTTON_DESTINATION_1;
			else if(!qstrcmp(token, "destination2"))
				type = SUBWAYWINDOW_BUTTON_DESTINATION_2;
			else if(!qstrcmp(token, "destination3"))
				type = SUBWAYWINDOW_BUTTON_DESTINATION_3;
			else if(!qstrcmp(token, "destination4"))
				type = SUBWAYWINDOW_BUTTON_DESTINATION_4;
			else if(!qstrcmp(token, "unavailable"))
				type = SUBWAYWINDOW_BUTTON_UNAVAILABLE;
			else if(!qstrcmp(token, "disabled"))
				type = SUBWAYWINDOW_BUTTON_DISABLED;
			else
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unknown $button type '%s'.\n", __FUNCTION__, token.c_str());
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			// Next token should be an openig bracket
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			if(qstrcmp(token, "{"))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Expected '{', got %s instead in definition in '%s'.\n", __FUNCTION__, token.c_str(), pstrScriptFile);
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			// Get ptr to schema
			subwaybuttonschema_t& schema = buttonschemas[type];

			while(true)
			{
				// Read the next token
				pstr = Common::Parse(pstr, token);
				if(!pstr)
					break;
				// End of definition
				if(!qstrcmp(token, "}"))
					break;

				if(!qstrcmp(token, "$buttontext"))
				{
					// Read in the button text
					pstr = Common::Parse(pstr, token);
					if(!pstr)
					{
						cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
						cl_filefuncs.pfnFreeFile(pfile);
						return false;
					}

					schema.buttontext = token;
				}
				else if(!qstrcmp(token, "$destinationid"))
				{
					// Read in the button text
					pstr = Common::Parse(pstr, token);
					if(!pstr)
					{
						cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
						cl_filefuncs.pfnFreeFile(pfile);
						return false;
					}

					schema.destinationid = token;
				}
				else if(!qstrcmp(token, "$description"))
				{
					// Next token should be an openig bracket
					pstr = Common::Parse(pstr, token);
					if(!pstr)
					{
						cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
						cl_filefuncs.pfnFreeFile(pfile);
						return false;
					}

					if(qstrcmp(token, "{"))
					{
						cl_engfuncs.pfnCon_EPrintf("%s - Expected '{', got %s instead in definition in '%s'.\n", __FUNCTION__, token.c_str(), pstrScriptFile);
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
						cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
						cl_filefuncs.pfnFreeFile(pfile);
						return false;
					}

					// Assign contents
					Uint32 length = pstrend - pstr;
					schema.description.assign(pstr, length);

					// Erase any tabulations
					while(true)
					{
						Uint32 tabpos = schema.description.find(0, "\t");
						if(tabpos == -1)
							break;

						schema.description.erase(tabpos, 1);
					}

					// Set pointer
					pstr = pstrend+1;
				}
				else
				{
					cl_engfuncs.pfnCon_EPrintf("%s - Unknown token '$s' in '%s'.\n", __FUNCTION__, token.c_str(), pstrScriptFile);
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}
			}
		}
		else if(!qstrcmp(token, "$defaultdescription"))
		{
			// Next token should be an openig bracket
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			if(qstrcmp(token, "{"))
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Expected '{', got %s instead in definition in '%s'.\n", __FUNCTION__, token.c_str(), pstrScriptFile);
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
				cl_engfuncs.pfnCon_EPrintf("%s - Unexpected EOF while reading $button definition in '%s'.\n", __FUNCTION__, pstrScriptFile);
				cl_filefuncs.pfnFreeFile(pfile);
				return false;
			}

			// Assign contents
			Uint32 length = pstrend - pstr;
			CString defaultdescription;
			defaultdescription.assign(pstr, length);

			// Erase any tabulations
			while(true)
			{
				Uint32 tabpos = defaultdescription.find(0, "\t");
				if(tabpos == -1)
					break;

				defaultdescription.erase(tabpos, 1);
			}

			m_pDefaultDescription->setText(defaultdescription.c_str());
			
			// Set pointer
			pstr = pstrend+1;
		}
		else
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Unknown token '$s' in '%s'.\n", __FUNCTION__, token.c_str(), pstrScriptFile);
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}
	}
	cl_filefuncs.pfnFreeFile(pfile);

	// Assign the button descriptions
	for(Uint32 i = 0; i < NB_DESTINATION_BUTTONS; i++)
	{
		bool isAvailable = (flags & (1<<i)) ? true : false;
		Int32 j = 0;
		if(i == 2)
			j = 3;
		else if(i == 3)
			j = 2;
		else
			j = i;

		subwaybuttonschema_t* pschema = nullptr;
		if(flags & FL_SUBWAY_DISABLED)
			pschema = &buttonschemas[SUBWAYWINDOW_BUTTON_DISABLED];
		else if(!isAvailable)
			pschema = &buttonschemas[SUBWAYWINDOW_BUTTON_UNAVAILABLE];
		else
			pschema = &buttonschemas[SUBWAYWINDOW_BUTTON_DESTINATION_1+j];

		m_buttonsArray[j].pButton->setText(pschema->buttontext.c_str());
		m_buttonsArray[j].pDescription->setText(pschema->description.c_str());
		m_buttonsArray[j].destinationid = pschema->destinationid;
	}

	// Set this information
	m_scriptFilePath = pstrScriptFile;
	m_subwayFlags = flags;

	return true;
}

//====================================
//
//====================================
void CGameUISubwayWindow::getInformation( CString& scriptfile, Int32& flags ) const
{
	scriptfile = m_scriptFilePath;
	flags = m_subwayFlags;
}

//====================================
//
//====================================
void CGameUISubwayWindow::ManageEvent( subwaybuttonevent_t event )
{
	switch(event)
	{
		case SUBWAY_BUTTON_1:
		case SUBWAY_BUTTON_2:
		case SUBWAY_BUTTON_3:
		case SUBWAY_BUTTON_4:
			{
				subwaybutton_t& button = m_buttonsArray[event];
				if(button.destinationid.empty())
				{
					cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_FAIL_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
					return;
				}

				// Send message to server
				Uint32 msgid = g_pGUIManager->GetServerUIMessageId();
				if(!msgid)
				{
					cl_engfuncs.pfnCon_Printf("%s - Message 'GameUIMessage' not registered on client.\n", __FUNCTION__);
					return;
				}

				// Just tell them which window sent it
				cl_engfuncs.pfnClientUserMessageBegin(msgid);
					cl_engfuncs.pfnMsgWriteByte(GAMEUIEVENT_SUBWAY_SELECTION);
					cl_engfuncs.pfnMsgWriteString(button.destinationid.c_str());
				cl_engfuncs.pfnClientUserMessageEnd();

				cl_engfuncs.pfnPlayAmbientSound(0, ZERO_VECTOR, SND_CHAN_AUTO, GAMEUI_OK_SOUND, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_2D, 0);
				m_windowFlags |= CGameUIWindow::FL_WINDOW_KILLME;
			}
			break;
		case SUBWAY_BUTTON_CANCEL:
			m_windowFlags |= CGameUIWindow::FL_WINDOW_KILLME;
			break;
	}
}

//====================================
//
//====================================
void CGameUISubwayWindowCallbackEvent::PerformAction( Float param )
{
	if(!m_pSubwayWindow)
		return;

	m_pSubwayWindow->ManageEvent(m_eventType);
}