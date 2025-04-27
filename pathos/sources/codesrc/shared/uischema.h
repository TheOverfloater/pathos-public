/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UISCHEMA_H
#define UISCHEMA_H

#include "file_interface.h"

struct en_texture_t;

struct ui_schemeobject_t
{
	ui_schemeobject_t():
		width(0),
		height(0),
		defaultTexture(nullptr),
		focusTexture(nullptr),
		clickTexture(nullptr),
		disabledTexture(nullptr)
	{}

	CString typeName;

	Uint32 width;
	Uint32 height;

	en_texture_t* defaultTexture;
	en_texture_t* focusTexture;
	en_texture_t* clickTexture;
	en_texture_t* disabledTexture;
};

struct ui_schemeinfo_t
{
	inline const ui_schemeobject_t* getObject( const Char* pstrName ) const
	{
		for(Uint32 i = 0; i < tabObjects.size(); i++)
		{
			if(!qstrcmp(tabObjects[i].typeName, pstrName))
				return &tabObjects[i];
		}

		return nullptr;
	}

	CString	schemeName;
	CArray<ui_schemeobject_t> tabObjects;
};

/*
=================================
CUISchemaManager

=================================
*/
class CUISchemaManager
{
public:
	typedef en_texture_t* (*pfnGetDummyTexture_t)( void );
	typedef en_texture_t* (*pfnLoadTexture_t)( const Char* pstrFilename, rs_level_t level, Int32 flags, const GLint* pborder );

public:
	CUISchemaManager( const file_interface_t& fileInterface, pfnGetDummyTexture_t pfnGetDummyTexture, pfnLoadTexture_t pfnLoadTexture );
	~CUISchemaManager( void );

public:
	// Clears all data
	void Clear( void );
	// Loads in a schema file
	ui_schemeinfo_t* LoadSchemaFile( const Char* pstrFilename );

	// Get nb of warnings
	Uint32 GetNbWarnings( void ) const;
	// Get warning
	CString GetWarning( Uint32 index ) const;
	// Clear warnings
	void ClearWarnings( void );

	// Get error MSG if any
	const CString& GetErrorString( void ) const;

private:
	// Array of schemas loaded
	CArray<ui_schemeinfo_t*> m_tabSchemeArray;
	// Warnings array
	CArray<CString> m_warningStringArray;
	// Error string
	CString m_errorString;

	// Get dummy texture fn
	pfnGetDummyTexture_t m_pfnGetDummyTexture;
	// Load texture fn ptr
	pfnLoadTexture_t m_pfnLoadTexture;

	// File interface
	const file_interface_t& m_fileInterface;
};
#endif //UISCHEMA_H