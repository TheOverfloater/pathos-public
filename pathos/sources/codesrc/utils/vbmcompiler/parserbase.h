/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PARSER_BASE_H
#define PARSER_BASE_H

#include "includes.h"
#include "compiler_types.h"
#include "studiocompiler.h"

/*
=======================
CParserBase

=======================
*/
class CParserBase
{
public:
	CParserBase( void );
	~CParserBase( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) = 0;
	// Clears the parser
	virtual void Clear( void );

public:
	// Opens a file for reading
	bool OpenScriptFile( const Char* pstrFilename );
	// Closes the loaded file
	void CloseScriptFile( void );

protected:
	// Checks if we're at the end of the script
	bool IsScriptEnd( void );
	// TRUE if we have a token available
	bool IsTokenAvailable( void ) { return m_readTokenBuffer.empty() ? false : true; }

	// Reads next token in
	bool ReadNextToken( bool ignoreLineEnd = false );
	// Returns the current token
	const Char* GetCurrentToken( void );

	// Read in a string and pass it back
	bool ReadString( const Char*& pstrResult, bool ignoreLineEnd = false );
	// Read in a string and pass it back
	bool ReadString( CString& result, bool ignoreLineEnd = false );
	// Read in an integer value and pass it back
	bool ReadInt32( Int32& result, bool ignoreLineEnd = false );
	// Read in a float value and pass it back
	bool ReadFloat( Float& result, bool ignoreLineEnd = false );

protected:
	// File name
	CString m_scriptFileName;
	// File loaded for parsing
	const Char* m_pQcScript;
	// Script file size
	Uint32 m_scriptLength;

	// Current parsing position;
	const Char* m_pReadPointer;
	// Currently parsed token buffer
	CString m_readTokenBuffer;
	// TRUe if reached end
	bool m_reachedEnd;

	// TRUE if this is the first value read
	bool m_isFirst;
};
#endif // QCPARSER_H
