/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "decallist.h"

//====================================
//
//====================================
CDecalList::CDecalList( void )
{
}

//====================================
//
//====================================
CDecalList::~CDecalList( void )
{
}

//====================================
//
//====================================
bool CDecalList::LoadDecalList( const Char* pfile, Uint32 isize )
{
	// Clear this
	if(!m_errorString.empty())
		m_errorString.clear();

	Char token[MAX_PARSE_LENGTH];
	const Char* pscan = pfile;
	while( pscan && (pscan - pfile) < isize )
	{
		pscan = Common::Parse(pscan, token);
		if(!pscan)
			break;

		m_decalGroupsArray.resize(m_decalGroupsArray.size()+1);
		decalgroup_t *pgroup = &m_decalGroupsArray[m_decalGroupsArray.size()-1];

		// Set name
		pgroup->name = token;

		// Read in the bracket
		pscan = Common::Parse(pscan, token);
		if(!pscan)
		{
			m_errorString << __FUNCTION__ << " - Unexpected EOF in decal list file.";
			return false;
		}

		if(qstrcmp(token, "{"))
		{
			m_errorString << __FUNCTION__ << " - Expected a {, got " << token << " instead.";
			return false;
		}

		pscan = Common::Parse(pscan, token);
		if(!pscan)
		{
			m_errorString << __FUNCTION__ << " - Unexpected EOF in decal list file.";
			return false;
		}

		while(true)
		{
			if(!qstrcmp(token, "}"))
				break;

			pgroup->entries.resize(pgroup->entries.size()+1);
			decalgroupentry_t *pentry = &pgroup->entries[pgroup->entries.size()-1];

			pentry->name = token;
			pentry->pgroup = nullptr;

			pscan = Common::Parse(pscan, token);
			if(!pscan)
			{
				m_errorString << __FUNCTION__ << " - Unexpected EOF in decal list file.";
				return false;
			}

			pentry->xsize = SDL_atoi(token)/2;

			pscan = Common::Parse(pscan, token);
			if(!pscan)
			{
				m_errorString << __FUNCTION__ << " - Unexpected EOF in decal list file.";
				return false;
			}

			pentry->ysize = SDL_atoi(token)/2;

			// parse for next
			pscan = Common::Parse(pscan, token);
		}
	}

	// Set group pts for the entries
	for(Uint32 i = 0; i < m_decalGroupsArray.size(); i++)
	{
		decalgroup_t *pgroup = &m_decalGroupsArray[i];
		for(Uint32 j = 0; j < pgroup->entries.size(); j++)
			pgroup->entries[j].pgroup = pgroup;
	}

	return true;
}

//====================================
//
//====================================
void CDecalList::Clear( void )
{
	if(m_decalGroupsArray.empty())
		return;

	m_decalGroupsArray.clear();
}

//====================================
//
//====================================
decalgroupentry_t *CDecalList::GetRandom( const Char *szgroupname )
{
	for(Uint32 i = 0; i < m_decalGroupsArray.size(); i++)
	{
		if(!qstrcmp(m_decalGroupsArray[i].name, szgroupname))
		{
			decalgroup_t *pgroup = &m_decalGroupsArray[i];
			Int32 index = Common::RandomLong(0, pgroup->entries.size()-1);

			decalgroupentry_t *pentry = &pgroup->entries[index];
			return pentry;
		}
	}

	return nullptr;
}

//====================================
//
//====================================
decalgroupentry_t *CDecalList::GetByName( const Char *szdecalname )
{
	for(Uint32 i = 0; i < m_decalGroupsArray.size(); i++)
	{
		decalgroup_t *pgroup = &m_decalGroupsArray[i];
		for(Uint32 j = 0; j < pgroup->entries.size(); j++)
		{
			if(!qstrcmp(pgroup->entries[j].name, szdecalname))
			{
				decalgroupentry_t *pentry = &pgroup->entries[j];
				return pentry;
			}
		}
	}

	return nullptr;
}

//====================================
//
//====================================
decalgroup_t* CDecalList::GetGroup( const char *szgroupname )
{
	for(Uint32 i = 0; i < m_decalGroupsArray.size(); i++)
	{
		decalgroup_t *pgroup = &m_decalGroupsArray[i];
		if(!qstrcmp(pgroup->name, szgroupname))
			return pgroup;
	}

	return nullptr;
}

//====================================
//
//====================================
decalgroup_t* CDecalList::GetGroup( Uint32 index )
{
	if(index >= m_decalGroupsArray.size())
		return nullptr;
	else
		return &m_decalGroupsArray[index];
}

//====================================
//
//====================================
Uint32 CDecalList::GetNbGroups( void ) const
{
	return m_decalGroupsArray.size();
}