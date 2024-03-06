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

#include "gameuitextwindow.h"
#include "gameuiwindows_shared.h"

// Text inset for the text tab
const Uint32 CGameUITextWindow::TEXTWINDOW_TEXT_TAB_TEXT_INSET = 15;
// Title text default font set name
const Char CGameUITextWindow::TEXTWINDOW_TITLE_DEFAULT_FONT_SET_NAME[] = "timesnewroman.ttf";
// Title text default font size
const Uint32 CGameUITextWindow::TEXTWINDOW_TITLE_DEFAULT_FONT_SIZE = 36;
// Title text low-res font size
const Uint32 CGameUITextWindow::TEXTWINDOW_TITLE_LOWRES_FONT_SIZE = 24;
// Display text lowres font size
const Uint32 CGameUITextWindow::TEXTWINDOW_LOWRES_FONT_SIZE = 12;

//====================================
//
//====================================
CGameUITextWindow::CGameUITextWindow( Int32 flags, Int32 originX, Int32 originY, Uint32 width, Uint32 height ):
	CGameUIWindow(flags, originX, originY, width, height),
	m_pTitleText(nullptr),
	m_pTextTab(nullptr)
{
}

//====================================
//
//====================================
CGameUITextWindow::~CGameUITextWindow( void )
{
}

//====================================
//
//====================================
void CGameUITextWindow::init( void )
{
	// Init basic window elements
	Uint32 verticalbarheight, middlebarwidth, barThickness;
	CGameUIWindow::initBackground(verticalbarheight, middlebarwidth, barThickness);

	Uint32 tabTopInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_TOP_INSET);
	Uint32 hBarYOrigin = gHUDDraw.ScaleY(GAMEUIWINDOW_H_BAR_Y_ORIGIN);
	Uint32 tabSideInset = gHUDDraw.ScaleX(GAMEUIWINDOW_TAB_SIDE_INSET);
	Uint32 mainTabMaxWidth = gHUDDraw.ScaleX(GAMEUIWINDOW_MAIN_TAB_MAX_WIDTH);
	Uint32 mainTabMaxHeight = gHUDDraw.ScaleY(GAMEUIWINDOW_MAIN_TAB_MAX_HEIGHT);
	Uint32 bottomInset = gHUDDraw.ScaleY(GAMEUIWINDOW_TAB_BOTTOM_INSET);
	Uint32 textInset = gHUDDraw.ScaleY(TEXTWINDOW_TEXT_TAB_TEXT_INSET);
	Uint32 edgeThickness = gHUDDraw.ScaleX(GAMEUIWINDOW_EDGE_THICKNESS);
	Uint32 defaultButtonWidth = gHUDDraw.ScaleX(DEFAULT_BUTTON_WIDTH);
	Uint32 defaultButtonHeight = gHUDDraw.ScaleY(DEFAULT_BUTTON_HEIGHT);

	//
	// Create the title text object
	//
	Uint32 titlefontsize;
	if(gHUDDraw.ScaleY(TEXTWINDOW_TITLE_DEFAULT_FONT_SIZE) <= TEXTWINDOW_TITLE_LOWRES_FONT_SIZE)
		titlefontsize = TEXTWINDOW_TITLE_LOWRES_FONT_SIZE;
	else
		titlefontsize = TEXTWINDOW_TITLE_DEFAULT_FONT_SIZE;

	const font_set_t* pTitleFont = cl_renderfuncs.pfnLoadFontSet(TEXTWINDOW_TITLE_DEFAULT_FONT_SET_NAME, titlefontsize);
	Uint32 textYOrigin = hBarYOrigin + tabTopInset/2.0f;
	m_pTitleText = new CGameUIText(CGameUIObject::FL_ALIGN_CH, GAMEUIWINDOW_DEFAULT_TEXT_COLOR, pTitleFont, 0, textYOrigin);
	m_pTitleText->setParent(this);

	//
	// Create the text tab
	//
	Uint32 tabWidth = middlebarwidth - barThickness*2 - tabSideInset*2;
	if(tabWidth > mainTabMaxWidth)
		tabWidth = mainTabMaxWidth;

	Uint32 tabOriginX = m_width / 2.0 - tabWidth / 2.0;

	Uint32 tabHeight = verticalbarheight - tabTopInset - bottomInset;
	if(tabHeight > mainTabMaxHeight)
		tabHeight = mainTabMaxHeight;

	m_pTextTab = new CGameUITextTab(CGameUIObject::FL_NONE, 
		nullptr,
		textInset,
		edgeThickness, 
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		GAMEUIWINDOW_DEFAULT_TEXT_COLOR,
		tabOriginX,
		hBarYOrigin + barThickness + tabTopInset,
		tabWidth,
		tabHeight);
	m_pTextTab->setParent(this);

	//
	// Create the "exit" button
	//

	CGameUITextWindowExitCallbackEvent* pEvent = new CGameUITextWindowExitCallbackEvent(this);

	Int32 buttonYOrigin = hBarYOrigin + barThickness + tabTopInset + tabHeight 
		+ (verticalbarheight-tabHeight-tabTopInset) * 0.5 - DEFAULT_BUTTON_HEIGHT*0.5;

	CGameUIButton* pButton = new CGameUIButton(CGameUIObject::FL_ALIGN_CH, 
		pEvent, 
		SDLK_RETURN,
		edgeThickness,
		GAMEUIWINDOW_MAIN_TAB_COLOR,
		GAMEUIWINDOW_MAIN_TAB_BG_COLOR,
		GAMEUIWINDOW_BUTTON_HIGHLIGHT_COLOR,
		0,
		buttonYOrigin,
		defaultButtonWidth,
		defaultButtonHeight);

	pButton->setText("Exit");
	pButton->setParent(this);
}

//====================================
//
//====================================
bool CGameUITextWindow::initData( const Char* pstrtextfilepath, const Char* pstrPassCode )
{
	const byte* pfile = cl_filefuncs.pfnLoadFile(pstrtextfilepath, nullptr);
	if(!pfile)
	{
		cl_engfuncs.pfnCon_Printf("%s - Could not load file '%s'.\n", __FUNCTION__, pstrtextfilepath);
		return false;
	}

	// Title text
	CString titletext;
	// Font set name
	CString titlefontsetname = TEXTWINDOW_TITLE_DEFAULT_FONT_SET_NAME;
	Uint32 titlefontsize;
	if(gHUDDraw.ScaleY(TEXTWINDOW_TITLE_DEFAULT_FONT_SIZE) <= TEXTWINDOW_TITLE_LOWRES_FONT_SIZE)
		titlefontsize = TEXTWINDOW_TITLE_LOWRES_FONT_SIZE;
	else
		titlefontsize = TEXTWINDOW_TITLE_DEFAULT_FONT_SIZE;

	// Remember ideal size
	Uint32 idealtitlefontsize = titlefontsize;

	// Text color
	color32_t titletextcolor = GAMEUIWINDOW_DEFAULT_TEXT_COLOR;

	// Font set name
	CString fontsetname = "default";
	// Font size
	Uint32 fontsize;
	if(gHUDDraw.ScaleY(DEFAULT_FONT_SIZE) <= TEXTWINDOW_LOWRES_FONT_SIZE)
		fontsize = TEXTWINDOW_LOWRES_FONT_SIZE;
	else
		fontsize = DEFAULT_FONT_SIZE;

	// Remember ideal font size
	Uint32 idealfontsize = fontsize;

	// Text color
	color32_t textcolor = GAMEUIWINDOW_DEFAULT_TEXT_COLOR;

	// First get all options
	CString token;
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		// Parse the token
		pstr = Common::Parse(pstr, token);
		if(!pstr)
		{
			cl_engfuncs.pfnCon_Printf("%s - Unexpected end of file in '%s'.\n", __FUNCTION__, pstrtextfilepath);
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}

		if(token[0] == '$')
		{
			// Read in the next parameter
			CString paramvalue;
			pstr = Common::Parse(pstr, paramvalue);

			// Need to handle a parameter
			if(!qstrcmp(token, "$title"))
			{
				// Set title
				titletext = paramvalue;
			}
			else if(!qstrcmp(token, "$fontset"))
			{
				// Set the font set name
				fontsetname = paramvalue;
			}
			else if(!qstrcmp(token, "$titlefontset"))
			{
				// Set the font set name
				titlefontsetname = paramvalue;
			}
			else if(!qstrcmp(token, "$fontsize") || !qstrcmp(token, "$titlefontsize"))
			{
				if(!Common::IsNumber(paramvalue.c_str()))
				{
					cl_engfuncs.pfnCon_Printf("%s - Expected a numerical value for '%s' in '%s', got '%s' instead.\n", __FUNCTION__, token.c_str(), pstrtextfilepath, paramvalue.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				// Set the font size
				if(!qstrcmp(token, "$titlefontsize"))
				{
					titlefontsize = SDL_atoi(paramvalue.c_str());
					if(titlefontsize > MAX_FONT_SIZE)
					{
						cl_engfuncs.pfnCon_DPrintf("%s - Invalid title font size %d specified in '%s'.\n", __FUNCTION__, fontsize, pstrtextfilepath);
						titlefontsize = TEXTWINDOW_TITLE_DEFAULT_FONT_SIZE;
					}
				}
				else
				{
					fontsize = SDL_atoi(paramvalue.c_str());
					if(fontsize > MAX_FONT_SIZE)
					{
						cl_engfuncs.pfnCon_DPrintf("%s - Invalid font size %d specified in '%s'.\n", __FUNCTION__, fontsize, pstrtextfilepath);
						fontsize = DEFAULT_FONT_SIZE;
					}
				}
			}
			else if(!qstrcmp(token, "$color") || !qstrcmp(token, "$titlecolor"))
			{
				if(!Common::IsNumber(paramvalue.c_str()))
				{
					cl_engfuncs.pfnCon_Printf("%s - Expected a numerical value for '%s' in '%s', got '%s' instead.\n", __FUNCTION__, token.c_str(), pstrtextfilepath, paramvalue.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				// Read in the g component
				CString gcomponent;
				pstr = Common::Parse(pstr, gcomponent);
				if(!pstr)
				{
					cl_engfuncs.pfnCon_Printf("%s - Unexpected end of file in '%s'.\n", __FUNCTION__, pstrtextfilepath);
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}
				if(!Common::IsNumber(gcomponent.c_str()))
				{
					cl_engfuncs.pfnCon_Printf("%s - Expected a numerical value for '%s' in '%s', got '%s' instead.\n", __FUNCTION__, token.c_str(), pstrtextfilepath, paramvalue.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				// Read in the b component
				CString bcomponent;
				pstr = Common::Parse(pstr, bcomponent);
				if(!pstr)
				{
					cl_engfuncs.pfnCon_Printf("%s - Unexpected end of file in '%s'.\n", __FUNCTION__, pstrtextfilepath);
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}
				if(!Common::IsNumber(bcomponent.c_str()))
				{
					cl_engfuncs.pfnCon_Printf("%s - Expected a numerical value for '%s' in '%s', got '%s' instead.\n", __FUNCTION__, token.c_str(), pstrtextfilepath, paramvalue.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				// Read in the a component
				CString acomponent;
				pstr = Common::Parse(pstr, acomponent);
				if(!pstr)
				{
					cl_engfuncs.pfnCon_Printf("%s - Unexpected end of file in '%s'.\n", __FUNCTION__, pstrtextfilepath);
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}
				if(!Common::IsNumber(acomponent.c_str()))
				{
					cl_engfuncs.pfnCon_Printf("%s - Expected a numerical value for '%s' in '%s', got '%s' instead.\n", __FUNCTION__, token.c_str(), pstrtextfilepath, paramvalue.c_str());
					cl_filefuncs.pfnFreeFile(pfile);
					return false;
				}

				// Assign the color components
				if(!qstrcmp(token, "$titlecolor"))
				{
					titletextcolor.r = SDL_atoi(paramvalue.c_str());
					titletextcolor.g = SDL_atoi(gcomponent.c_str());
					titletextcolor.b = SDL_atoi(bcomponent.c_str());
					titletextcolor.a = SDL_atoi(acomponent.c_str());
				}
				else
				{
					textcolor.r = SDL_atoi(paramvalue.c_str());
					textcolor.g = SDL_atoi(gcomponent.c_str());
					textcolor.b = SDL_atoi(bcomponent.c_str());
					textcolor.a = SDL_atoi(acomponent.c_str());
				}
			}
			else
			{
				cl_engfuncs.pfnCon_Printf("%s - Unknown option '%s' in '%s'.\n", __FUNCTION__, token.c_str(), pstrtextfilepath);
				continue;
			}
		}
		else if(!qstrcmp(token, "{"))
		{
			// Time to read text contents
			break;
		}
		else
		{
			cl_engfuncs.pfnCon_Printf("%s - Expected '{' or option starting with '$', got '%s' instead in '%s'.\n", __FUNCTION__, token.c_str(), pstrtextfilepath);
			cl_filefuncs.pfnFreeFile(pfile);
			return false;
		}
	}

	// Safeguard twice
	if(qstrcmp(token, "{"))
	{
		cl_engfuncs.pfnCon_Printf("%s - Expected '{', got '%s' instead in '%s'.\n", __FUNCTION__, token.c_str(), pstrtextfilepath);
		cl_filefuncs.pfnFreeFile(pfile);
		return false;
	}

	// Find the ending bracket
	const Char* pstrend = qstrstr(pstr, "}");
	if(!pstrend)
	{
		cl_engfuncs.pfnCon_Printf("%s - Text file '%s' is missing ending bracket '}'.\n", __FUNCTION__, pstrtextfilepath);
		cl_filefuncs.pfnFreeFile(pfile);
		return false;
	}

	CString textcontents;
	Uint32 datasize = (pstrend - pstr);
	textcontents.assign(pstr, datasize);

	// Release the file
	cl_filefuncs.pfnFreeFile(pfile);

	// Replace any passcode tokens
	Int32 tokenoffset = textcontents.find(0, "%passcode%");
	if(tokenoffset != -1)
	{
		textcontents.erase(tokenoffset, 10);
		textcontents.insert(tokenoffset, pstrPassCode);
	}

	// Assign the contents
	m_pTextTab->initData(reinterpret_cast<const byte*>(textcontents.c_str()), textcontents.length());

	// Set color
	m_pTextTab->setTextColor(textcolor);

	// Load font if not default
	const font_set_t* pfontset = nullptr;
	if(qstrcmp(fontsetname, "default") || fontsize != idealfontsize)
	{
		// Make sure to replace the token if it's "default"
		if(!qstrcmp(fontsetname, "default"))
			fontsetname = DEFAULT_FONTSET_NAME;

		if(gHUDDraw.ScaleY(fontsize) <= TEXTWINDOW_LOWRES_FONT_SIZE)
			fontsize = TEXTWINDOW_LOWRES_FONT_SIZE;

		pfontset = cl_renderfuncs.pfnLoadFontSet(fontsetname.c_str(), fontsize);
	}

	// Set font if not null
	if(pfontset)
		m_pTextTab->setFontSet(pfontset);

	// Load font if not default
	const font_set_t* ptitlefontset = nullptr;
	if(qstrcmp(titlefontsetname, TEXTWINDOW_TITLE_DEFAULT_FONT_SET_NAME) || titlefontsize != idealtitlefontsize)
	{
		if(gHUDDraw.ScaleY(titlefontsize) <= TEXTWINDOW_TITLE_LOWRES_FONT_SIZE)
			titlefontsize = TEXTWINDOW_TITLE_LOWRES_FONT_SIZE;

		ptitlefontset = cl_renderfuncs.pfnLoadFontSet(titlefontsetname.c_str(), titlefontsize);
	}

	// Set font if not null
	if(ptitlefontset)
		m_pTitleText->setFontSet(ptitlefontset);

	// Set color
	m_pTitleText->setColor(titletextcolor);

	// Set title if specified
	if(!titletext.empty())
		m_pTitleText->setText(titletext.c_str());

	// Set these
	m_textFilePath = pstrtextfilepath;
	m_passcode = pstrPassCode;

	// Pause the game
	cl_engfuncs.pfnSetPaused(true, true);

	return true;
}

//====================================
//
//====================================
void CGameUITextWindow::getInformation( CString& textfilepath, CString& passcode ) const
{
	textfilepath = m_textFilePath;
	passcode = m_passcode;
}

//====================================
//
//====================================
void CGameUITextWindow::CloseWindow( void )
{
	// Flag for removal
	m_windowFlags |= CGameUIWindow::FL_WINDOW_KILLME;

	// Unpause the game
	cl_engfuncs.pfnSetPaused(false, false);
}

//====================================
//
//====================================
void CGameUITextWindowExitCallbackEvent::PerformAction( Int32 param )
{
	if(!m_pTextWindow)
		return;

	m_pTextWindow->CloseWindow();
}