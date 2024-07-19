/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"

// Empty string character
Char CString::EMPTY_STRING[] = "\0";

//=============================================
// @brief Default constructor
//
// @param pstr Pointer to string
//=============================================
CString::CString():
	m_pString(nullptr),
	m_stringLength(0)
{
	m_pString = EMPTY_STRING;
}

//=============================================
// @brief Default constructor
//
// @param pstr Pointer to string
//=============================================
CString::CString( const Char* pstr ):
	m_pString(nullptr),
	m_stringLength(0)
{
	if(pstr)
	{
		const Uint32 strlength = qstrlen(pstr);
		if(strlength)
		{
			m_pString = new Char[strlength+1];
			qstrcpy(m_pString, pstr);
			m_stringLength = strlength;
		}
	}

	if(!m_pString)
		m_pString = EMPTY_STRING;
}

//=============================================
// @brief Copy constructor
//
// @param str Reference to CString to copy
//=============================================
CString::CString( const CString& str ):
	m_pString(nullptr),
	m_stringLength(0)
{
	const Char* psrc = str.c_str();
	if(psrc)
	{
		const Uint32 length = str.length();
		if(length)
		{
			m_pString = new Char[length+1];
			qstrcpy(m_pString, psrc);
			m_stringLength = length;
		}
	}

	if(!m_pString)
		m_pString = EMPTY_STRING;
}

//=============================================
// @brief Default constructor
//
// @param pstr Pointer to string
//=============================================
CString::CString( const Char* pstr, Uint32 length ):
	m_pString(nullptr),
	m_stringLength(length)
{
	if(pstr && length)
	{
		m_pString = new Char[length+1];
		qstrncpy(m_pString, pstr, length);
		m_pString[length] = '\0';
		m_stringLength = length;
	}
	else
	{
		m_pString = EMPTY_STRING;
	}
}

//=============================================
// @brief Destructor
//
//=============================================
CString::~CString()
{
	if(m_pString && m_pString != EMPTY_STRING)
		delete[] m_pString;
}
