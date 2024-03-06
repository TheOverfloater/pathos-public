//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           CompilerOptions.cpp
// last modified:  May 04 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>

#include "config.h"
#include "common.h"
#include "file.h"

// Object definition
CConfig gConfig;

// Configuration file name
const Char CConfig::CONFIG_FILE_NAME[] = "hlmv.cfg";

//=============================================
// @brief Constructor
//
//=============================================
CConfig::CConfig( void )
{
	CString path;
	path << mx::getApplicationPath() << PATH_SLASH_CHAR << CONFIG_FILE_NAME;

	const byte* pFile = FL_LoadFile(path.c_str(), nullptr);
	if(!pFile)
		return;

	Char token1[MAX_PARSE_LENGTH];
	Char token2[MAX_PARSE_LENGTH];

	Uint32 lineNumber = 0;
	CString line;
	const Char* plstr = reinterpret_cast<const Char*>(pFile);
	while(plstr)
	{
		plstr = Common::ReadLine(plstr, line);
		lineNumber++;
		if(line.empty())
			continue;

		// Parse the option name
		const Char* pstr = Common::Parse(line.c_str(), token1);
		if(!pstr || !qstrlen(token1))
			continue;

		// Parse in the value
		qstrcpy(token2, pstr);
		if(!qstrlen(token2))
			continue;
		
		CString optionName;
		const Char* pstrColon = strstr(token1, ":");
		if(pstrColon)
			optionName.assign(token1, (pstrColon-token1));
		else
			optionName = token1;

		SetOption(optionName.c_str(), token2);
	}

	FL_FreeFile(pFile);
}

//=============================================
// @brief Destructor
//
//=============================================
CConfig::~CConfig( void )
{
}

//=============================================
// @brief Saves options to a config file
//
//=============================================
void CConfig::SaveOptions( void )
{
	if(m_optionsArray.empty())
		return;

	CString path;
	path << mx::getApplicationPath() << PATH_SLASH_CHAR << CONFIG_FILE_NAME;

	FILE* pFile = fopen(path.c_str(), "w");
	if(!pFile)
		return;

	for(Uint32 i = 0; i < m_optionsArray.size(); i++)
		fprintf(pFile, "%s: %s\n", m_optionsArray[i].name.c_str(), m_optionsArray[i].value.c_str());

	fclose(pFile);
}

//=============================================
// @brief Returns the value for an option
//
//=============================================
const Char* CConfig::GetOptionValue( const Char* pstrName )
{
	for(Uint32 i = 0; i < m_optionsArray.size(); i++)
	{
		if(!qstrcmp(m_optionsArray[i].name, pstrName))
			return m_optionsArray[i].value.c_str();
	}

	return nullptr;
}

//=============================================
// @brief Sets an option's value
//
//=============================================
void CConfig::SetOption( const Char* pstrName, const Char* pstrValue )
{
	for(Uint32 i = 0; i < m_optionsArray.size(); i++)
	{
		if(!qstrcmp(m_optionsArray[i].name, pstrName))
		{
			m_optionsArray[i].value = pstrValue;
			return;
		}
	}

	option_t newOption;
	newOption.name = pstrName;
	newOption.value = pstrValue;
	m_optionsArray.push_back(newOption);
}

//=============================================
// @brief Sets an option's value
//
//=============================================
void CConfig::EraseOption( const Char* pstrName )
{
	for(Uint32 i = 0; i < m_optionsArray.size(); i++)
	{
		if(!qstrcmp(m_optionsArray[i].name, pstrName))
		{
			m_optionsArray.erase(i);
			return;
		}
	}
}