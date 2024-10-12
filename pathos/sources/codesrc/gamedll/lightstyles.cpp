/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "lightstyles.h"

// start lightstyle index for dynlights
const Uint32 CLightStyles::CUSTOM_LIGHTSTYLE_START_INDEX = 255;
// Default framerate value
const Float CLightStyles::DEFAULT_LIGHTSTYLE_FRAMERATE = 10;

// Class object definition
CLightStyles gSVLightStyles;

//=============================================
// @brief
//
//=============================================
CLightStyles::CLightStyles( void ):
	m_nextLightStyleIndex(0)
{
}

//=============================================
// @brief
//
//=============================================
CLightStyles::~CLightStyles( void )
{
}

//=============================================
// @brief
//
//=============================================
void CLightStyles::ResetStyles( void )
{
	if(!m_lightStylesArray.empty())
		m_lightStylesArray.clear();

	m_nextLightStyleIndex = CUSTOM_LIGHTSTYLE_START_INDEX;
}

//=============================================
// @brief
//
//=============================================
Int32 CLightStyles::AddCustomLightStyle( const Char* pstrpattern, bool interpolate, Float framerate )
{
	for(Uint32 i = 0; i < m_lightStylesArray.size(); i++)
	{
		lightstyle_t& style = m_lightStylesArray[i];

		if(!qstrcmp(style.pattern, pstrpattern)
			&& style.interpolate == interpolate
			&& style.framerate == framerate)
		{
			// Already exists
			return style.styleindex;
		}
	}

	lightstyle_t newstyle;
	newstyle.pattern = pstrpattern;
	newstyle.interpolate = interpolate;
	newstyle.framerate = framerate;
	newstyle.styleindex = m_nextLightStyleIndex;
	m_nextLightStyleIndex++;

	if(m_lightStyleValuesArray.size() <= newstyle.styleindex)
		m_lightStyleValuesArray.resize(newstyle.styleindex+1);

	if(g_gameInitializationDone)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.addcustomlightstyle, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(newstyle.interpolate);
		gd_engfuncs.pfnMsgWriteInt16(newstyle.styleindex);
		gd_engfuncs.pfnMsgWriteString(newstyle.pattern.c_str());
		gd_engfuncs.pfnMsgWriteSmallFloat(newstyle.framerate);
		gd_engfuncs.pfnUserMessageEnd();
	}

	m_lightStylesArray.push_back(newstyle);
	return newstyle.styleindex;
}

//=============================================
// @brief
//
//=============================================
void CLightStyles::SetLightStyle( const Char* pstrpattern, bool interpolate, Float framerate, Uint32 styleindex )
{
	if(styleindex >= CUSTOM_LIGHTSTYLE_START_INDEX)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called with invalid index %d, all indexes above %d are reserved for dynamic light lightystyles.\n", __FUNCTION__, styleindex, CUSTOM_LIGHTSTYLE_START_INDEX);
		return;
	}

	lightstyle_t* pstyle = nullptr;
	for(Uint32 i = 0; i < m_lightStylesArray.size(); i++)
	{
		lightstyle_t& style = m_lightStylesArray[i];
		if(style.styleindex == styleindex)
		{
			pstyle = &style;
			break;
		}
	}

	if(!pstyle)
	{
		Uint32 prevSize = m_lightStylesArray.size();
		m_lightStylesArray.resize(prevSize+1);
		pstyle = &m_lightStylesArray[prevSize];
	}

	pstyle->pattern = pstrpattern;
	pstyle->interpolate = interpolate;
	pstyle->framerate = framerate;
	pstyle->styleindex = styleindex;

	if(m_lightStyleValuesArray.size() <= pstyle->styleindex)
		m_lightStyleValuesArray.resize(pstyle->styleindex+1);

	if(!pstyle->framerate)
		pstyle->framerate = DEFAULT_LIGHTSTYLE_FRAMERATE;

	if(g_gameInitializationDone)
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setlightstyle, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(pstyle->interpolate);
		gd_engfuncs.pfnMsgWriteInt16(pstyle->styleindex);
		gd_engfuncs.pfnMsgWriteString(pstyle->pattern.c_str());
		gd_engfuncs.pfnMsgWriteSmallFloat(pstyle->framerate);
		gd_engfuncs.pfnUserMessageEnd();
	}
}

//=============================================
// @brief
//
//=============================================
void CLightStyles::Think( void )
{
	for(Uint32 i = 0; i < m_lightStylesArray.size(); i++)
	{
		lightstyle_t& style = m_lightStylesArray[i];
		if(!style.pattern.length())
		{
			m_lightStyleValuesArray[style.styleindex] = 1.0;
			continue;
		}

		if(style.interpolate)
		{
			const Float frame = (g_pGameVars->time*style.framerate);
			const Float interp = frame - floor(frame);

			Int32 i1 = static_cast<Int32>(frame) % style.pattern.length();
			Int32 i2 = (static_cast<Int32>(frame) + 1) % style.pattern.length();

			const Int32 v1 = (style.pattern[i1] - 'a')*22;
			const Int32 v2 = (style.pattern[i2] - 'a')*22;

			Float value = ((static_cast<Float>(v1))*(1.0-interp)) + ((static_cast<Float>(v2))*interp);
			m_lightStyleValuesArray[style.styleindex] = value / 256.0f;
		}
		else
		{
			const Float frame = (g_pGameVars->time*style.framerate);
			Int32 i1 = static_cast<Int32>(frame) % style.pattern.length();
			const Int32 v = (style.pattern[i1] - 'a')*22;

			m_lightStyleValuesArray[style.styleindex] = static_cast<Float>(v)/256.0f;
		}
	}
}

//=============================================
// @brief
//
//=============================================
CArray<Float>* CLightStyles::GetLightStyleValuesArray( void )
{
	return &m_lightStyleValuesArray;
}

//=============================================
// @brief
//
//=============================================
void CLightStyles::SendLightStylesToClient( edict_t* pPlayer )
{
	if(m_lightStylesArray.empty())
		return;

	for(Uint32 i = 0; i < m_lightStylesArray.size(); i++)
	{
		lightstyle_t& style = m_lightStylesArray[i];

		Int32 msgId;
		if(style.styleindex < CUSTOM_LIGHTSTYLE_START_INDEX)
			msgId = g_usermsgs.setlightstyle;
		else
			msgId = g_usermsgs.addcustomlightstyle;

		if(pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, msgId, nullptr, pPlayer);
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, msgId, nullptr, nullptr);

		gd_engfuncs.pfnMsgWriteByte(style.interpolate);
		gd_engfuncs.pfnMsgWriteInt16(style.styleindex);
		gd_engfuncs.pfnMsgWriteString(style.pattern.c_str());
		gd_engfuncs.pfnMsgWriteSmallFloat(style.framerate);
		gd_engfuncs.pfnUserMessageEnd();
	}
}
