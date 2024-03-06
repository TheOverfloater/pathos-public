//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           FileAssociation.h
// last modified:  Apr 28 1999, Mete Ciragan
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

#ifndef CONFIG_H
#define CONFIG_H

#include "includes.h"

/*
=================================
CConfig

=================================
*/
class CConfig
{
public:
	// Configuration file name
	static const Char CONFIG_FILE_NAME[];

public:
	struct option_t
	{
		CString name;
		CString value;
	};

public:
	CConfig( void );
	~CConfig( void );

public:
	// Saves options to a file
	void SaveOptions( void );
	// Gets the value of an option
	const Char* GetOptionValue( const Char* pstrName );
	// Sets the value of an option
	void SetOption( const Char* pstrName, const Char* pstrValue );
	// Erases an option
	void EraseOption( const Char* pstrName );

private:
	// Options array
	CArray<option_t> m_optionsArray;
};
extern CConfig gConfig;
#endif // CONFIG_H