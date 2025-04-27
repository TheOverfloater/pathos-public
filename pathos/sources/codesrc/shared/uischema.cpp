/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "uischema.h"

#include "r_common.h"
#include "textures_shared.h"

//=============================================
// @brief Constructor
//
//=============================================
CUISchemaManager::CUISchemaManager( const file_interface_t& fileInterface, pfnGetDummyTexture_t pfnGetDummyTexture, pfnLoadTexture_t pfnLoadTexture ):
	m_fileInterface( fileInterface ),
	m_pfnGetDummyTexture(pfnGetDummyTexture),
	m_pfnLoadTexture(pfnLoadTexture)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUISchemaManager::~CUISchemaManager( void )
{
	Clear();
}

//=============================================
// @brief
//
//=============================================
void CUISchemaManager::Clear( void )
{
	if(m_tabSchemeArray.empty())
		return;

	for(Uint32 i = 0; i < m_tabSchemeArray.size(); i++)
		delete m_tabSchemeArray[i];

	m_tabSchemeArray.clear();
}

//=============================================
// @brief
//
//=============================================
ui_schemeinfo_t* CUISchemaManager::LoadSchemaFile( const Char* pstrFilename )
{
	// Try to find it in the cache first
	for(Uint32 i = 0; i < m_tabSchemeArray.size(); i++)
	{
		if(!qstrcmp(m_tabSchemeArray[i]->schemeName, pstrFilename))
			return m_tabSchemeArray[i];
	}

	// Load in the file
	CString scriptPath;
	scriptPath << "scripts/ui/schemas/" << pstrFilename;

	Uint32 fileSize = 0;
	const Char* pfile = reinterpret_cast<const Char*>(m_fileInterface.pfnLoadFile(scriptPath.c_str(), &fileSize));
	if(!pfile)
	{
		m_errorString << "Failed to load UI schema script '" << scriptPath.c_str() << "'";
		return nullptr;
	}

	// Allocate new object
	ui_schemeinfo_t* pNew = new ui_schemeinfo_t;
	pNew->schemeName = pstrFilename;

	// Parse the contents
	CString token;
	CString line;

	const Char* pstr = pfile;
	while(pstr && *pstr && (pstr - pfile) < fileSize)
	{
		// Read in the first token
		CString objName;
		pstr = Common::Parse(pstr, objName);
		if(!pstr || objName.empty())
			break;

		// Scheme object we'll be processing
		ui_schemeobject_t newObject;
		newObject.typeName = objName;

		// Next token should be an opening bracket
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			m_errorString << "Unexpected EOF on '" << scriptPath.c_str() << "'";
			m_fileInterface.pfnFreeFile(pfile);

			delete pNew;
			return nullptr;
		}

		// Make sure the script is valid
		if(qstrcmp(token, "{"))
		{
			m_errorString << "{ token expected '" << scriptPath.c_str() << "', got '" << token.c_str() << "' instead.";
			m_fileInterface.pfnFreeFile(pfile);

			delete pNew;
			return nullptr;
		}

		// Read in the fields, line by line
		while(pstr && *pstr && (pstr - pfile) < fileSize)
		{
			// Skip whitespaces
			while(*pstr && SDL_isspace(*pstr))
				pstr++;

			// Read in the entire line
			pstr = Common::ReadLine(pstr, line);
			if(line.empty())
				continue;

			// Read in the first token
			const Char* pstrl = Common::Parse(line.c_str(), token);
			if(token.empty())
			{
				m_errorString << "Unexpected EOF on '" << scriptPath.c_str() << "'";
				m_fileInterface.pfnFreeFile(pfile);

				delete pNew;
				return nullptr;
			}

			// Exit the loop
			if(!qstrcmp(token, "}"))
				break;
			
			if(!pstrl)
			{
				m_errorString << "Unexpected EOF on '" << scriptPath.c_str() << "'";
				m_fileInterface.pfnFreeFile(pfile);

				delete pNew;
				return nullptr;
			}

			// Read in the value
			CString value;
			pstrl = Common::Parse(pstrl, value);
			if(value.empty())
			{
				m_errorString << "Unexpected EOF on '" << scriptPath.c_str() << "'";
				m_fileInterface.pfnFreeFile(pfile);

				delete pNew;
				return nullptr;
			}

			// Determine field type
			CString textureName;
			if(!qstrcmp(token, "$default") 
				|| !qstrcmp(token, "$focus")
				|| !qstrcmp(token, "$clicked")
				|| !qstrcmp(token, "$disabled"))
			{
				// If it's a texture resource, load it in
				CString texturePath;
				texturePath << "ui/" << value;

				// Load it in
				en_texture_t* ptexture = m_pfnLoadTexture(texturePath.c_str(), RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS, nullptr);
				if(!ptexture)
					ptexture = m_pfnGetDummyTexture();

				// Assign it to the right place
				if(!qstrcmp(token, "$default"))
					newObject.defaultTexture = ptexture;
				else if(!qstrcmp(token, "$focus"))
					newObject.focusTexture = ptexture;
				else if(!qstrcmp(token, "$clicked"))
					newObject.clickTexture = ptexture;
				else if(!qstrcmp(token, "$disabled"))
					newObject.disabledTexture = ptexture;

				if(!newObject.width)
					newObject.width = ptexture->width;
				if(!newObject.height)
					newObject.height = ptexture->height;
			}
			else if(!qstrcmp(token, "$width"))
				newObject.width = SDL_atoi(value.c_str());
			else if(!qstrcmp(token, "$height"))
				newObject.height = SDL_atoi(value.c_str());
			else
			{
				CString str;
				str << "Unknown field '" << token.c_str() << "' in '" << scriptPath.c_str() << "'";
				m_warningStringArray.push_back(str);
				break;
			}
		}

		// Add it to the object
		pNew->tabObjects.push_back(newObject);
	}

	// Add this scheme object to the array
	m_tabSchemeArray.push_back(pNew);
	m_fileInterface.pfnFreeFile(pfile);

	return pNew;
}

//=============================================
// @brief
//
//=============================================
Uint32 CUISchemaManager::GetNbWarnings( void ) const
{
	return m_warningStringArray.size();
}

//=============================================
// @brief
//
//=============================================
CString CUISchemaManager::GetWarning( Uint32 index ) const
{
	if(index > m_warningStringArray.size())
		return CString();
	else
		return m_warningStringArray[index];
}

//=============================================
// @brief
//
//=============================================
void CUISchemaManager::ClearWarnings( void )
{
	if(m_warningStringArray.empty())
		return;

	m_warningStringArray.clear();
}

//=============================================
// @brief
//
//=============================================
const CString& CUISchemaManager::GetErrorString( void ) const
{
	return m_errorString;
}