/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "saytext.h"
#include "clientdll.h"
#include "fontset.h"
#include "input.h"

// Maximum text lifetime
const Float CSayText::SAYTEXT_LIFETIME = 15;
// Fade duration for each line
const Float CSayText::SAYTEXT_FADETIME = 1.0;
// Screen text text schema name
const Char CSayText::TEXTSCHEME_FILENAME[] = "saytext";

// Object definition
CSayText gSayText;

//====================================
//
//====================================
void Cmd_MessageMode( void )
{
	gSayText.BeginInputMode(false);
}

//====================================
//
//====================================
void Cmd_TeamMessageMode( void )
{
	gSayText.BeginInputMode(true);
}

//====================================
//
//====================================
CSayText::CSayText( void ):
	m_pFont(nullptr),
	m_isInInputMode(false),
	m_isTeamOnly(false)
{
}

//====================================
//
//====================================
CSayText::~CSayText( void )
{
}

//====================================
//
//====================================
bool CSayText::Init( void )
{
	cl_engfuncs.pfnCreateCommand("messagemode", Cmd_MessageMode, "Say a text message to other players");
	cl_engfuncs.pfnCreateCommand("teammessagemode", Cmd_TeamMessageMode, "Say a text message to other team members only");

	return true;
}

//====================================
//
//====================================
void CSayText::Shutdown( void )
{
	ClearGame();
}

//====================================
//
//====================================
bool CSayText::InitGL( void )
{
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	m_pFont = cl_engfuncs.pfnGetResolutionSchemaFontSet(TEXTSCHEME_FILENAME, screenHeight);
	if(!m_pFont)
		m_pFont = cl_renderfuncs.pfnLoadFontSet("timesnewroman.ttf", 24);

	if(!m_pFont)
		return false;

	return true;
}

//====================================
//
//====================================
void CSayText::ClearGL( void )
{
}

//====================================
//
//====================================
bool CSayText::InitGame( void )
{
	return true;
}

//====================================
//
//====================================
void CSayText::ClearGame( void )
{
	if(!m_sayTextList.empty())
		m_sayTextList.clear();

	m_isInInputMode = false;
}

//====================================
//
//====================================
void CSayText::AddText( const Char* pstrPlayerName, const Char* pstrText )
{
	if(!pstrPlayerName || pstrPlayerName[0] == '\0' || !pstrText || pstrText[0] == '\0')
		return;

	saytextinfo_t newtext;
	newtext.playername = pstrPlayerName;
	newtext.message = pstrText;
	newtext.die = cl_engfuncs.pfnGetClientTime() + SAYTEXT_LIFETIME;

	m_sayTextList.add(newtext);
}

//====================================
//
//====================================
bool CSayText::DrawSayText( void )
{
	if(m_sayTextList.empty() && !m_isInInputMode)
		return true;

	Uint32 scrwidth, scrheight;
	cl_renderfuncs.pfnGetScreenSize(scrwidth, scrheight);

	Uint32 xcoord = scrwidth * 0.05;
	Uint32 ycoord = scrheight * 0.75;
	Uint32 ycoordbase = ycoord;

	Uint32 maxwidth = scrwidth * 0.4;
	Uint32 maxheight = scrheight * 0.5;
	Uint32 totalheight = 0;

	// Get time
	Double time = cl_engfuncs.pfnGetClientTime();

	// Draw last-to-first
	m_sayTextList.begin();
	while(!m_sayTextList.end())
	{
		saytextinfo_t& text = m_sayTextList.get();

		// Remove old text
		Float alpha = 1.0;
		if((text.die - SAYTEXT_FADETIME) < time)
			alpha = 1.0 - ((time - (text.die - SAYTEXT_FADETIME))/SAYTEXT_FADETIME);

		color32_t colorPlayerName(255, 192, 64, 255*alpha);
		color32_t colorMessage(255, 192, 255, 255*alpha);

		if(text.die < time)
		{
			m_sayTextList.remove(m_sayTextList.get_link());
			m_sayTextList.next();
			continue;
		}

		Uint32 textwidth = 0;
		Uint32 textheight = m_pFont->fontsize;

		CString playernamestring;
		playernamestring << text.playername << ": ";

		Uint32 namestringlength = 0;
		const Char* pstr = playernamestring.c_str();
		while(*pstr)
		{
			Uint32 glyphwidth = m_pFont->glyphs[*pstr].advancex;
			pstr++;

			if(textwidth + glyphwidth > maxwidth)
			{
				textwidth = 0;
				textheight += m_pFont->fontsize;
			}

			textwidth += glyphwidth;
			namestringlength += glyphwidth;
		}

		textwidth = namestringlength;
		pstr = text.message.c_str();
		while(*pstr)
		{
			Uint32 glyphwidth = m_pFont->glyphs[*pstr].advancex;
			pstr++;

			if(textwidth + glyphwidth > maxwidth)
			{
				textwidth = 0;
				textheight += m_pFont->fontsize;
			}

			textwidth += glyphwidth;
		}

		// Shift us upwards
		ycoord -= textheight;
		totalheight += textheight;

		if(totalheight >= maxheight)
		{
			m_sayTextList.remove(m_sayTextList.get_link());
			m_sayTextList.next();
			continue;
		}

		// Draw the text
		if(!cl_renderfuncs.pfnDrawStringBox(0, 0, maxwidth, textheight+m_pFont->fontsize, 0, 0, false, colorPlayerName, xcoord, ycoord, playernamestring.c_str(), m_pFont, 0, 0, 0)
			|| !cl_renderfuncs.pfnDrawStringBox(0, 0, maxwidth, textheight+m_pFont->fontsize, 0, 0, false, colorMessage, xcoord, ycoord, text.message.c_str(), m_pFont, 0, 0, namestringlength))
		{
			cl_engfuncs.pfnErrorPopup("Shader error: %s.\n", cl_renderfuncs.pfnGetStringDrawError());
			return false;
		}

		m_sayTextList.next();
	}

	if(m_isInInputMode)
	{
		ycoord = ycoordbase + m_pFont->fontsize*2;

		CString displaytext;
		displaytext << "Say: " << m_sayText;

		color32_t colorInput(255, 255, 255, 255);

		// Allow 4 rows of text
		if(!cl_renderfuncs.pfnDrawStringBox(xcoord, ycoord, xcoord+maxwidth, ycoord+m_pFont->fontsize*4, 0, 0, false, colorInput, 0, 0, displaytext.c_str(), m_pFont, 0, 0, 0))
		{
			cl_engfuncs.pfnErrorPopup("Shader error: %s.\n", cl_renderfuncs.pfnGetStringDrawError());
			return false;
		}
	}

	return true;
}

//====================================
//
//====================================
void CSayText::BeginInputMode( bool teamonly )
{
	m_isInInputMode = true;
	m_isTeamOnly = teamonly;

	if(!m_sayText.empty())
		m_sayText.clear();

	CL_ResetPressedInputs();
}

//====================================
//
//====================================
bool CSayText::IsInInputMode( void ) const
{
	return m_isInInputMode;
}

//====================================
//
//====================================
void CSayText::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!keyDown)
		return;

	// Get SDL Keycode
	SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)button);
	
	// See if it's a valid text input character
	if(sdlKeycode >= SDLK_SPACE && sdlKeycode <= SDLK_z)
	{
		// Shift if needed
		Char inputChar = (Char)sdlKeycode;
		if(mod & KMOD_SHIFT)
			inputChar = Common::GetShiftedChar(inputChar);

		m_sayText.Append(inputChar);
	}
	else if(sdlKeycode == SDLK_BACKSPACE)
	{
		if(!m_sayText.empty())
			m_sayText.erase(m_sayText.length()-1, 1);
	}
	else if(sdlKeycode == SDLK_RETURN)
	{
		// Avoid spinning bug
		cl_engfuncs.pfnResetMouse();

		// Clear input mode
		m_isInInputMode = false;

		if(m_sayText.empty())
			return;

		CString cmd;
		if(m_isTeamOnly)
			cmd << "say_team ";
		else
			cmd << "say ";

		cmd << m_sayText;
		m_sayText.clear();

		cl_engfuncs.pfnServerCommand(cmd.c_str());
	}
	else if(sdlKeycode == SDLK_ESCAPE)
	{
		// Avoid spinning bug
		cl_engfuncs.pfnResetMouse();

		m_isInInputMode = false;
		if(!m_sayText.empty())
			m_sayText.clear();
	}
}