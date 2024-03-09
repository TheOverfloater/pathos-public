/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "fontset.h"
#include "r_glsl.h"
#include "r_vbo.h"
#include "matrix.h"
#include "textures_shared.h"
#include "r_common.h"
#include "screentext.h"
#include "clientdll.h"

// Screen text color
const color32_t CScreenText::SCREENTEXT_COLOR = color32_t(255, 255, 255, 255);
// Screen text text schema name
const Char CScreenText::SCREENTEXT_TEXTSCHEME_FILENAME[] = "screentext";

// Object definition
CScreenText gScreenText;

//=============================================
// @brief
//
//=============================================
CScreenText::CScreenText( void ):
	m_pFontSet(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CScreenText::~CScreenText( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CScreenText::Init( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void CScreenText::Shutdown( void )
{
	ClearGame();
}

//=============================================
// @brief
//
//=============================================
bool CScreenText::InitGL( void )
{
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(SCREENTEXT_TEXTSCHEME_FILENAME, screenHeight);
	if(!m_pFontSet)
		m_pFontSet = cl_renderfuncs.pfnGetDefaultFontSet();

	if(!m_pFontSet)
	{
		cl_engfuncs.pfnCon_Printf("%s - Failed to get default font set.\n", __FUNCTION__);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CScreenText::ClearGL( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CScreenText::InitGame( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void CScreenText::ClearGame( void )
{
	if(!m_messagesList.empty())
		m_messagesList.clear();
}

//=============================================
// @brief
//
//=============================================
bool CScreenText::Draw( void )
{
	if(m_messagesList.empty())
		return true;

	if(!cl_renderfuncs.pfnBeginTextRendering(m_pFontSet))
	{
		cl_engfuncs.pfnErrorPopup("Shader error: %s.", cl_renderfuncs.pfnGetStringDrawError());
		return false;
	}

	Uint32 scrwidth, scrheight;
	cl_renderfuncs.pfnGetScreenSize(scrwidth, scrheight);

	m_messagesList.begin();
	while(!m_messagesList.end())
	{
		textmsg_t& msg = m_messagesList.get();
		if(msg.life < cl_engfuncs.pfnGetClientTime())
		{
			m_messagesList.remove(m_messagesList.get_link());
			continue;
		}

		Int32 xPosition, yPosition;
		if(msg.xcoord == -1)
			xPosition = 0;
		else
			xPosition = msg.xcoord;
		
		if(msg.ycoord == -1)
			yPosition = 0;
		else
			yPosition = msg.ycoord;

		cl_renderfuncs.pfnSetStringRectangle(xPosition, yPosition, scrwidth - xPosition, scrheight - yPosition, 0, 0);

		Uint32 width, height;
		cl_renderfuncs.pfnGetStringSize(m_pFontSet, msg.text.c_str(), &width, &height, nullptr);
		cl_renderfuncs.pfnSetStringRectangle(0, 0, 0, 0, 0, 0);
		
		if(msg.xcoord == -1)
			xPosition = scrwidth * 0.5f - width * 0.5; 

		if(msg.ycoord == -1)
			yPosition = scrheight * 0.5f - height * 0.5; 

		if(!cl_renderfuncs.pfnDrawStringBox(0, 0, 0, 0, 0, 0, false, SCREENTEXT_COLOR, xPosition, yPosition, msg.text.c_str(), m_pFontSet, 0, m_pFontSet->fontsize, 0))
			return false;

		m_messagesList.next();
	}

	cl_renderfuncs.pfnFinishTextRendering(m_pFontSet);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CScreenText::AddText( const Char* pstrText, Int32 xcoord, Int32 ycoord, Float lifetime )
{
	textmsg_t newmsg;
	newmsg.xcoord = xcoord;
	newmsg.ycoord = ycoord;
	newmsg.text = pstrText;
	newmsg.life = cl_engfuncs.pfnGetClientTime() + lifetime;

	m_messagesList.add(newmsg);
}