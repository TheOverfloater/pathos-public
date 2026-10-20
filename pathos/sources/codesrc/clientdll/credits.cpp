/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/
// Code by valina354

#include "includes.h"
#include "clientdll.h"
#include "fontset.h"
#include "credits.h"

// Default path to credits file
const Char CCredits::CREDITS_FILE_PATH[] = "scripts/credits.txt";
// Credits font schema name
const Char CCredits::CREDITS_TEXTSCHEMA_NAME[] = "credits";
// Default color for text
const color32_t CCredits::CREDITS_TEXT_COLOR = color32_t(255, 255, 255, 255);

// Global instance
CCredits gCredits;

//=============================================
// @brief
//
//=============================================
CCredits::CCredits( void ):
	m_isActive(false),
	m_scrollSpeed(45.0f),
	m_currentY(0),
	m_lastTime(0),
	m_totalHeight(0),
	m_screenWidth(0),
	m_screenHeight(0),
	m_pFontSet(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CCredits::~CCredits( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CCredits::Init( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void CCredits::Shutdown( void )
{
	ClearGame();
}

//=============================================
// @brief
//
//=============================================
bool CCredits::InitGL( void )
{
	cl_renderfuncs.pfnGetScreenSize(m_screenWidth, m_screenHeight);

	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(CREDITS_TEXTSCHEMA_NAME, m_screenHeight);
	if (!m_pFontSet)
		m_pFontSet = cl_renderfuncs.pfnGetDefaultFontSet();

	if (!m_pFontSet)
	{
		cl_engfuncs.pfnCon_Printf("%s - Failed to get default font set.\n", __FUNCTION__);
		return false;
	}

	RecomputeDimensions();
	return true;
}

//=============================================
// @brief
//
//=============================================
void CCredits::ClearGL( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CCredits::InitGame( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void CCredits::ClearGame( void )
{
	m_isActive = false;
	m_creditsLines.clear();
	m_totalHeight = 0;
	m_currentY = 0;
	m_lastTime = 0;
}

//=============================================
// @brief
//
//=============================================
bool CCredits::LoadCreditsFile( void )
{
	m_creditsLines.clear();
	m_totalHeight = 0;

	Uint32 fileSize = 0;
	const byte* pfile = cl_filefuncs.pfnLoadFile(CREDITS_FILE_PATH, &fileSize);
	if (!pfile)
	{
		cl_engfuncs.pfnCon_Printf("%s - Could not load '%s'.\n", __FUNCTION__, CREDITS_FILE_PATH);
		return false;
	}

	const Char* pScan = reinterpret_cast<const Char*>(pfile);
	Char lineBuffer[1024];

	while (pScan && *pScan)
	{
		pScan = Common::ReadLine(pScan, lineBuffer);

		creditline_t line;
		line.text = lineBuffer;

		if (!m_pFontSet)
			m_pFontSet = cl_renderfuncs.pfnGetDefaultFontSet();

		if (!line.text.empty() && m_pFontSet)
		{
			cl_renderfuncs.pfnGetStringSize(m_pFontSet, line.text.c_str(), &line.width, &line.height, nullptr);
		}
		else if (m_pFontSet)
		{
			line.width = 0;
			line.height = m_pFontSet->fontsize;
		}

		m_totalHeight += line.height + 4;
		m_creditsLines.push_back(line);
	}

	cl_filefuncs.pfnFreeFile(pfile);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CCredits::RecomputeDimensions( void )
{
	if (m_creditsLines.empty() || !m_pFontSet)
		return;

	m_totalHeight = 0;
	for (Uint32 i = 0; i < m_creditsLines.size(); i++)
	{
		creditline_t& line = m_creditsLines[i];
		if (!line.text.empty())
			cl_renderfuncs.pfnGetStringSize(m_pFontSet, line.text.c_str(), &line.width, &line.height, nullptr);
		else
			line.width = 0;

		line.height = m_pFontSet->fontsize;
		m_totalHeight += line.height + 4;
	}
}

//=============================================
// @brief
//
//=============================================
void CCredits::StartCredits( Float scrollSpeed )
{
	if (!LoadCreditsFile())
		return;

	cl_renderfuncs.pfnGetScreenSize(m_screenWidth, m_screenHeight);

	m_scrollSpeed = (scrollSpeed > 0) ? scrollSpeed : 45.0f;
	m_currentY = m_screenHeight;
	m_lastTime = cl_engfuncs.pfnGetClientTime();
	m_isActive = true;
}

//=============================================
// @brief
//
//=============================================
void CCredits::Think( void )
{
	if (!m_isActive)
		return;

	Double curTime = cl_engfuncs.pfnGetClientTime();
	Double deltaTime = curTime - m_lastTime;
	m_lastTime = curTime;

	if (deltaTime <= 0)
		return;

	if (deltaTime > 0.1)
		deltaTime = 0.1;

	m_currentY -= m_scrollSpeed * deltaTime;

	if (m_currentY + m_totalHeight < 0)
		m_isActive = false;
}

//=============================================
// @brief
//
//=============================================
bool CCredits::Draw( void )
{
	if (!m_isActive || m_creditsLines.empty())
		return true;

	if (!cl_renderfuncs.pfnBeginTextRendering(m_pFontSet))
	{
		cl_engfuncs.pfnErrorPopup("Shader error: %s.", cl_renderfuncs.pfnGetStringDrawError());
		return false;
	}

	Double currentLineY = m_currentY;
	for (Uint32 i = 0; i < m_creditsLines.size(); i++)
	{
		const creditline_t& line = m_creditsLines[i];

		if (currentLineY + static_cast<Double>(line.height) >= 0 && currentLineY <= static_cast<Double>(m_screenHeight))
		{
			if (!line.text.empty())
			{
				Int32 xPosition = (static_cast<Int32>(m_screenWidth) - static_cast<Int32>(line.width)) / 2;
				Int32 yPosition = static_cast<Int32>(currentLineY);

				if (!cl_renderfuncs.pfnDrawStringBox(0, 0, 0, 0, 0, 0, false, CREDITS_TEXT_COLOR, xPosition, yPosition, line.text.c_str(), m_pFontSet, 0, m_pFontSet->fontsize, 0))
				{
					cl_renderfuncs.pfnFinishTextRendering();
					return false;
				}
			}
		}

		currentLineY += line.height + 4;
	}

	cl_renderfuncs.pfnFinishTextRendering();
	return true;
}