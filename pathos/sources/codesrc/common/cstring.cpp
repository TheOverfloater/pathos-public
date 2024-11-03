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
// Pointer to string pool
CStringPool* CString::g_pStringPool = CStringPool::Instance();

//=============================================
// @brief Default constructor
//
// @param pstr Pointer to string
//=============================================
CString::CString():
	m_pString(nullptr),
	m_stringLength(0),
	m_pPoolCacheEntry(nullptr),
	m_flags(fl_str_none)
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
	m_stringLength(0),
	m_pPoolCacheEntry(nullptr),
	m_flags(fl_str_none)
{
	if(pstr)
	{
		const Uint32 strlength = qstrlen(pstr);
		if(strlength)
		{
			Char* pString = new Char[strlength+1];
			qstrcpy(pString, pstr);
			setdata(pString, strlength);
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
	m_stringLength(0),
	m_pPoolCacheEntry(nullptr),
	m_flags(fl_str_none)
{
	const Char* psrc = str.c_str();
	if(psrc)
	{
		const Uint32 length = str.length();
		if(length)
		{
			Char* pString = new Char[length+1];
			qstrcpy(pString, psrc);
			setdata(pString, length);
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
	m_stringLength(length),
	m_pPoolCacheEntry(nullptr),
	m_flags(fl_str_none)
{
	if(pstr && length)
	{
		Char* pString = new Char[length+1];
		qstrncpy(pString, pstr, length);
		pString[length] = '\0';
		setdata(pString, length);
	}
	else
	{
		m_pString = EMPTY_STRING;
	}
}

//=============================================
// @brief Constructor with flags
//
//=============================================
CString::CString( byte flags ):
	m_pString(nullptr),
	m_stringLength(0),
	m_pPoolCacheEntry(nullptr),
	m_flags(flags)
{
	m_pString = EMPTY_STRING;
}
//=============================================
// @brief Destructor
//
//=============================================
CString::~CString()
{
	if(m_pString && m_pString != EMPTY_STRING)
	{
		if(!(m_flags & fl_str_nopooling))
			g_pStringPool->RemoveString(m_pPoolCacheEntry);
		else
			delete[] m_pString;
	}
}

//=============================================
// @brief Sets data for the string
//
//=============================================
void CString::setdata( const Char* pString, Uint32 length )
{
	if(!(m_flags & fl_str_nopooling))
	{
		if(m_pString && m_pString != EMPTY_STRING)
			g_pStringPool->RemoveString(m_pPoolCacheEntry);

		m_pPoolCacheEntry = g_pStringPool->AddString(pString);
		m_pString = m_pPoolCacheEntry->iterator->first.c_str();
	}
	else
	{
		if(m_pString && m_pString != EMPTY_STRING)
			delete[] m_pString;

		m_pString = pString;
	}

	m_stringLength = length;
}
