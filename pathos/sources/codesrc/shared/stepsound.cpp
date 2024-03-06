/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "file_interface.h"
#include "stepsound.h"
#include "trace.h"
#include "common.h"

// Default footstep type name
const Char CStepSound::DEFAULT_FOOTSTEP_TYPE[] = "default";
// Default material type name
const Char CStepSound::DEFAULT_MATERIAL_TYPE[] = "default";
// Concrete material type name
const Char CStepSound::CONCRETE_MATERIAL_TYPE[] = "concrete";

//=============================================
//
//=============================================
CStepSound::CStepSound( void ):
	m_pDefaultFootstepInfo(nullptr)
{
}

//=============================================
//
//=============================================
CStepSound::~CStepSound( void )
{
}

//=============================================
//
//=============================================
bool CStepSound::Init( const Char* pstrFile, Uint32 filesize )
{
	if(!pstrFile || !filesize)
		return false;

	// Clear errors
	m_strInfoString.clear();
	// Clear array
	m_footStepInfoArray.clear();

	// Parse in the script's contents
	CString token;
	CString materialtypename;
	const Char* pstr = pstrFile;
	while(pstr && (pstr-pstrFile) < filesize)
	{
		// Read in the name token
		pstr = Common::Parse(pstr, materialtypename);
		if(materialtypename.empty() || !pstr)
		{
			m_strInfoString << "Unexpected end of file";
			m_footStepInfoArray.clear();
			return false;
		}

		// Next is a bracket
		pstr = Common::Parse(pstr, token);
		if(token.empty() || !pstr)
		{
			m_strInfoString << "Unexpected end of file";
			m_footStepInfoArray.clear();
			return false;
		}

		if(qstrcmp(token, "{"))
		{
			m_strInfoString << "Expected '{', got '" << token << "' instead";
			m_footStepInfoArray.clear();
			return false;
		}

		footstepinfo_t newInfo;
		newInfo.materialname = materialtypename;

		while(true)
		{
			// Read next token
			pstr = Common::Parse(pstr, token);
			if(token.empty())
			{
				m_strInfoString << "Unexpected end of file";
				m_footStepInfoArray.clear();
				return false;
			}

			if(!qstrcmp(token, "}"))
				break;

			CArray<CString> *pdestarray = nullptr;
			if(!qstrcmp(token, "right"))
				pdestarray = &newInfo.rightarray;
			else if(!qstrcmp(token, "left"))
				pdestarray = &newInfo.leftarray;
			else if(!qstrcmp(token, "type"))
			{
				pstr = Common::Parse(pstr, token);
				if(token.empty() || !pstr)
				{
					m_strInfoString << "Unexpected end of file";
					m_footStepInfoArray.clear();
					return false;
				}

				newInfo.type = token;
				continue;
			}
			else
			{
				m_strInfoString << "Unknown foot '" << token << "' specified";
				m_footStepInfoArray.clear();
				return false;
			}

			// Next is a bracket
			pstr = Common::Parse(pstr, token);
			if(token.empty() || !pstr)
			{
				m_strInfoString << "Unexpected end of file";
				m_footStepInfoArray.clear();
				return false;
			}

			if(qstrcmp(token, "{"))
			{
				m_strInfoString << "Expected '{', got '" << token << "' instead in";
				m_footStepInfoArray.clear();
				return false;
			}

			while(true)
			{
				// Read next token
				pstr = Common::Parse(pstr, token);
				if(token.empty() || !pstr)
				{
					m_strInfoString << "Unexpected end of file";
					m_footStepInfoArray.clear();
					return false;
				}

				if(!qstrcmp(token, "}"))
					break;

				// Add the sound to the list
				pdestarray->push_back(token);
			}
		}

		// Add it to the list
		m_footStepInfoArray.push_back(newInfo);
	}

	// Find the default type
	for(Uint32 i = 0; i < m_footStepInfoArray.size(); i++)
	{
		footstepinfo_t& info = m_footStepInfoArray[i];
		if(!qstrcmp(info.materialname, DEFAULT_FOOTSTEP_TYPE))
		{
			m_pDefaultFootstepInfo = &info;
			break;
		}
	}

	if(!m_pDefaultFootstepInfo)
	{
		m_strInfoString << "No default type set";
		m_footStepInfoArray.clear();
		return false;
	}

	// Warn about duplicates
	for(Uint32 i = 0; i < m_footStepInfoArray.size(); i++)
	{
		footstepinfo_t& info1 = m_footStepInfoArray[i];

		for(Uint32 j = 0; j < m_footStepInfoArray.size(); j++)
		{
			if(i == j)
				continue;

			footstepinfo_t& info2 = m_footStepInfoArray[j];
			if(!qstrcmp(info1.materialname, info2.materialname)
				&& !qstrcmp(info1.type, info2.type))
			{
				m_strInfoString << "Duplicate material entry '" << info1.materialname
					<< "' with type '" << info1.type << "' found in footstep script.\n";
			}
		}
	}

	return true;
}

//=============================================
//
//=============================================
void CStepSound::GetPrecacheList( CArray<CString>& outList )
{
	if(m_footStepInfoArray.empty())
		return;

	for(Uint32 i = 0; i < m_footStepInfoArray.size(); i++)
	{
		footstepinfo_t& info = m_footStepInfoArray[i];

		for(Uint32 j = 0; j < info.leftarray.size(); j++)
			outList.push_back(info.leftarray[j]);

		for(Uint32 j = 0; j < info.rightarray.size(); j++)
			outList.push_back(info.rightarray[j]);
	}
}

//=============================================
//
//=============================================
bool CStepSound::CheckType( const Char* pstrType, const Char* pstrCheckType )
{
	if(!pstrCheckType || !qstrlen(pstrCheckType) || !qstrcmp(pstrCheckType, DEFAULT_FOOTSTEP_TYPE))
	{
		if(!qstrcmp(pstrType, DEFAULT_FOOTSTEP_TYPE))
			return true;
		else
			return false;
	}
	else if(!qstrcmp(pstrType, pstrCheckType))
		return true;
	else
		return false;
}

//=============================================
//
//=============================================
bool CStepSound::CheckMaterial( const Char* pstrMaterial, const Char* pstrCheckMaterial )
{
	if(!pstrCheckMaterial || !qstrlen(pstrCheckMaterial) 
		|| !qstrcmp(pstrCheckMaterial, DEFAULT_MATERIAL_TYPE) 
		|| !qstrcmp(pstrCheckMaterial, CONCRETE_MATERIAL_TYPE))
	{
		if(!qstrcmp(pstrCheckMaterial, DEFAULT_FOOTSTEP_TYPE) || !qstrcmp(pstrCheckMaterial, CONCRETE_MATERIAL_TYPE))
			return true;
		else
			return false;
	}
	else if(!qstrcmp(pstrMaterial, pstrCheckMaterial))
		return true;
	else
		return false;
}

//=============================================
//
//=============================================
const CArray<CString>* CStepSound::GetFootSoundList( foot_t foot, const Char* pmaterialname, const Char* psteptype )
{
	if(m_footStepInfoArray.empty())
		return nullptr;

	// Try to retreive the material relevant sound
	footstepinfo_t* pinfo = nullptr;
	for(Uint32 i = 0; i < m_footStepInfoArray.size(); i++)
	{
		footstepinfo_t& info = m_footStepInfoArray[i];
		if(CheckMaterial(info.materialname.c_str(), pmaterialname) && CheckType(info.type.c_str(), psteptype))
		{
			pinfo = &info;
			break;
		}
	}

	// Try without steptype
	if(!pinfo)
	{
		for(Uint32 i = 0; i < m_footStepInfoArray.size(); i++)
		{
			footstepinfo_t& info = m_footStepInfoArray[i];
			if(CheckMaterial(info.materialname.c_str(), pmaterialname) && CheckType(info.type.c_str(), DEFAULT_FOOTSTEP_TYPE))
			{
				pinfo = &info;
				break;
			}
		}
	}

	// Use default otherwise
	if(!pinfo)
		pinfo = m_pDefaultFootstepInfo;

	switch(foot)
	{
	case FOOT_LEFT:
		return &pinfo->leftarray;
		break;
	case FOOT_RIGHT:
	default:
		return &pinfo->rightarray;
		break;
	}
}

//=============================================
//
//=============================================
bool CStepSound::IsMaterialTypeValid( const Char* pmaterialname )
{
	if(m_footStepInfoArray.empty())
		return false;

	// Try to retreive the material relevant sound
	for(Uint32 i = 0; i < m_footStepInfoArray.size(); i++)
	{
		footstepinfo_t& info = m_footStepInfoArray[i];
		if(CheckMaterial(info.materialname.c_str(), pmaterialname))
			return true;
	}

	return false;
}

//=============================================
//
//=============================================
const CStepSound::footstepinfo_t* CStepSound::GetMaterialByIndex( Uint32 index )
{
	if(m_footStepInfoArray.empty())
		return nullptr;

	return &m_footStepInfoArray[index];
}

//=============================================
//
//=============================================
Uint32 CStepSound::GetNbMaterials( void )
{
	return m_footStepInfoArray.size();
}