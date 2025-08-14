/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "studiocompiler.h"
#include "parserbase.h"
#include "options.h"
#include "main.h"

//===============================================
// @brief Constructor for CParserBase class
//
//===============================================
CParserBase::CParserBase( void ):
	m_pQcScript(nullptr),
	m_scriptLength(0),
	m_pReadPointer(nullptr),
	m_reachedEnd(false),
	m_isFirst(false)
{
}

//===============================================
// @brief Destructor for CParserBase class
//
//===============================================
CParserBase::~CParserBase( void )
{
}

//===============================================
// @brief Opens a file for reading
//
// @param pstrFilename Path to script to load
// @return TRUE if reached file end, FALSE otherwise
//===============================================
bool CParserBase::OpenScriptFile( const Char* pstrFilename )
{
	// Now load the QC file to be read
	Uint32 length = 0;
	const byte* pFilePtr = nullptr;

	if(!OpenFile(pstrFilename, length, pFilePtr))
		return false;

	// Set ptrs
	m_pQcScript = reinterpret_cast<const Char*>(pFilePtr);
	m_scriptLength = length;
	m_pReadPointer = m_pQcScript;
	m_scriptFileName = pstrFilename;

	return true;
}

//===============================================
// @brief Closes file that was opened for reading
//
//===============================================
void CParserBase::CloseScriptFile( void )
{
	m_scriptFileName.clear();

	if(m_pQcScript)
	{
		FreeFile(reinterpret_cast<const byte*>(m_pQcScript));
		m_pQcScript = nullptr;
	}
}

//===============================================
// @brief Clears the parser
//
//===============================================
void CParserBase::Clear( void )
{
	if(m_pQcScript)
	{
		delete[] m_pQcScript;
		m_pQcScript = nullptr;
	}

	m_pReadPointer = nullptr;
	m_scriptLength = 0;
	m_reachedEnd = false;

	if(!m_readTokenBuffer.empty())
		m_readTokenBuffer.clear();
}

//===============================================
// @brief Checks if we're at the end of the script
//
// @return TRUE if reached file end, FALSE otherwise
//===============================================
bool CParserBase::IsScriptEnd( void )
{
	return m_reachedEnd;
}

//===============================================
// @brief Reads next token in
//
// @param ignoreLineEnd If set, parsing will go over newlines
// @return TRUE if parsing was successful, FALSE if it failed
//===============================================
bool CParserBase::ReadNextToken( bool ignoreLineEnd )
{
	if(!m_pReadPointer)
	{
		m_readTokenBuffer.clear();
		m_reachedEnd = true;
		return false;
	}

	if(m_reachedEnd)
	{
		if(!ignoreLineEnd)
			ErrorMsg("Incomplete line encountered.\n");

		m_readTokenBuffer.clear();
		return false;
	}

	// Skip over newline if set
	if(ignoreLineEnd && ((*m_pReadPointer) == '\r' || (*m_pReadPointer) == '\n'))
	{
		// Skip over any of the spaces all the way to the next token
		while(SDL_isspace(*m_pReadPointer))
			m_pReadPointer++;

		// Check again for this
		if(m_pReadPointer[0] == '\0')
			m_reachedEnd = true;
	}

	// Skip any spaces except newline
	while(SDL_isspace(*m_pReadPointer) 
		&& (*m_pReadPointer) != '\n' 
		&& (*m_pReadPointer) != '\r')
		m_pReadPointer++;

	// Parse next token, but don't go over a newline
	m_pReadPointer = Common::Parse(m_pReadPointer, m_readTokenBuffer, "\r\n");
	if(!m_pReadPointer)
	{
		// We've reached the end of the file
		m_reachedEnd = true;
		return m_readTokenBuffer.empty() ? false : true;
	}

	return m_readTokenBuffer.empty() ? false : true;
}

//===============================================
// @brief Returns the current token we read
//
// @return Pointer to current token string
//===============================================
const Char* CParserBase::GetCurrentToken( void )
{
	return m_readTokenBuffer.c_str();
}

//===============================================
// @brief Read in a string and pass it back
//
// @param pstrResult Pointer to variable that will point to the result string
// @param ignoreLineEnd If set, parsing will go over newlines
// @return TRUE if parsed value is valid, FALSE otherwise
//===============================================
bool CParserBase::ReadString( const Char*& pstrResult, bool ignoreLineEnd )
{
	if(!ReadNextToken(ignoreLineEnd))
	{
		pstrResult = nullptr;
		return false;
	}
	else if(!m_readTokenBuffer.empty())
	{
		pstrResult = m_readTokenBuffer.c_str();
		return true;
	}
	else
	{
		pstrResult = nullptr;
		return false;
	}
}

//===============================================
// @brief Read in a string and pass it back
//
// @param pstrResult Pointer to variable that will point to the result string
// @param ignoreLineEnd If set, parsing will go over newlines
// @return TRUE if parsed value is valid, FALSE otherwise
//===============================================
bool CParserBase::ReadString( CString& result, bool ignoreLineEnd )
{
	if(!ReadNextToken(ignoreLineEnd))
		return false;

	if(!m_readTokenBuffer.empty())
	{
		result = m_readTokenBuffer;
		return true;
	}
	else
	{
		result.clear();
		return false;
	}
}

//===============================================
// @brief Read in an integer value and pass it back
//
// @param result Reference to integer that'll store our value
// @param ignoreLineEnd If set, parsing will go over newlines
// @return TRUE if parsed value is valid, FALSE otherwise
//===============================================
bool CParserBase::ReadInt32( Int32& result, bool ignoreLineEnd )
{
	if(!ReadNextToken(ignoreLineEnd))
		return false;

	if(m_readTokenBuffer.empty())
		return false;

	const Char* pstrString = m_readTokenBuffer.c_str();
	if(!Common::IsNumber(pstrString))
	{
		ErrorMsg("Parsed string '%s' is not a numerical value.\n", pstrString);
		return false;
	}

	result = SDL_atoi(pstrString);
	return true;
}

//===============================================
// @brief Read in a float value and pass it back
//
// @param result Reference to float that'll store our value
// @param ignoreLineEnd If set, parsing will go over newlines
// @return TRUE if parsed value is valid, FALSE otherwise
//===============================================
bool CParserBase::ReadFloat( Float& result, bool ignoreLineEnd )
{
	if(!ReadNextToken(ignoreLineEnd))
		return false;

	if(m_readTokenBuffer.empty())
		return false;

	const Char* pstrString = m_readTokenBuffer.c_str();
	if(!Common::IsNumber(pstrString))
	{
		ErrorMsg("Parsed string '%s' is not a numerical value.\n", pstrString);
		return false;
	}

	result = SDL_atof(pstrString);
	return true;
}