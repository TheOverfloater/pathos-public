/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "skytexturesets.h"

// Max skybox sets
const Uint32 CSkyTextureSets::MAX_SKYBOX_TEXTURE_SETS = 32;

CSkyTextureSets gSkyTextureSets;

//=============================================
// @brief
//
//=============================================
CSkyTextureSets::CSkyTextureSets( void )
{
}

//=============================================
// @brief
//
//=============================================
CSkyTextureSets::~CSkyTextureSets( void )
{
}

//=============================================
// @brief
//
//=============================================
void CSkyTextureSets::Reset( void )
{
	if(!m_skyTextureNamesArray.empty())
		m_skyTextureNamesArray.clear();
}

//=============================================
// @brief
//
//=============================================
Int32 CSkyTextureSets::RegisterSkyTextureSet( const Char* pstrSkyTextureName )
{
	for(Int32 i = 0; i < (Int32)m_skyTextureNamesArray.size(); i++)
	{
		if(!qstrcmp(m_skyTextureNamesArray[i], pstrSkyTextureName))
			return i;
	}

	if(m_skyTextureNamesArray.size() >= MAX_SKYBOX_TEXTURE_SETS)
	{
		gd_engfuncs.pfnCon_Printf("%s - Exceeded MAX_SKYBOX_TEXTURE_SETS(%d).\n", __FUNCTION__, MAX_SKYBOX_TEXTURE_SETS);
		return NO_POSITION;
	}

	Int32 index = m_skyTextureNamesArray.size();
	m_skyTextureNamesArray.push_back(pstrSkyTextureName);
	return index;
}

//=============================================
// @brief
//
//=============================================
void CSkyTextureSets::RegisterSets( edict_t* pPlayer )
{
	if(m_skyTextureNamesArray.empty())
		return;

	for(Uint32 i = 0; i < m_skyTextureNamesArray.size(); i++)
	{
		if(pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.addskytextureset, nullptr, pPlayer);
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.addskytextureset, nullptr, nullptr);

		gd_engfuncs.pfnMsgWriteString(m_skyTextureNamesArray[i].c_str());
		gd_engfuncs.pfnMsgWriteByte((Int32)i);
		gd_engfuncs.pfnUserMessageEnd();
	}
}