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

// Work buffer
Char* CString::g_pWorkBuffer = nullptr;
// Work buffer size
Uint32 CString::g_workBufferSize = 0;
// Work buffer mutex
std::mutex CString::g_workBufferMutex;

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
	if(pstr && qstrlen(pstr) > 0)
		setdata(pstr);
	else
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
	if(str.length())
		setdata( str.c_str());
	else
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
		g_workBufferMutex.lock();
		CheckBuffer(length);
		g_pWorkBuffer[0] = '\0';
		qstrncpy(g_pWorkBuffer, pstr, length);

		setdata(g_pWorkBuffer);
		g_workBufferMutex.unlock();
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
// @brief Appends a source string to the current string
//
// @param psrc Pointer to string
//=============================================
void CString::Append(const Char* psrc)
{
	if(psrc)
	{
		const Uint32 srclength = qstrlen(psrc);
		const Uint32 newlength = srclength + m_stringLength;

		g_workBufferMutex.lock();
		CheckBuffer(newlength);
		g_pWorkBuffer[0] = '\0';

		if(m_pString && m_pString != EMPTY_STRING)
			sprintf_s(g_pWorkBuffer, g_workBufferSize, "%s%s", m_pString, psrc);
		else
			qstrcpy_s(g_pWorkBuffer, psrc, g_workBufferSize);

		setdata(g_pWorkBuffer);
		g_workBufferMutex.unlock();
	}
}

//=============================================
// @brief Appends a char to the current string
//
// @param c Character to append
//=============================================
void CString::Append(Char c)
{
	const Uint32 newlength = m_stringLength+1;

	g_workBufferMutex.lock();
	CheckBuffer(newlength);
	g_pWorkBuffer[0] = '\0';

	if(m_pString && m_pString != EMPTY_STRING)
		sprintf_s(g_pWorkBuffer, g_workBufferSize, "%s%c", m_pString, c);
	else
		sprintf_s(g_pWorkBuffer, g_workBufferSize, "%c", c);

	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Appends an integer to the current string
//
// @param i Integer value to append
//=============================================
void CString::Append(Int32 i)
{
	Uint32 logVal = SDL_log10(i);
	Uint32 digitCount = SDL_floor(logVal) + 1;
	Uint32 newlength = m_stringLength + digitCount;

	g_workBufferMutex.lock();
	CheckBuffer(newlength);

	if(m_pString && m_pString != EMPTY_STRING)
		sprintf_s(g_pWorkBuffer, g_workBufferSize, "%s%d", m_pString, i);
	else
		sprintf_s(g_pWorkBuffer, g_workBufferSize, "%d", i);

	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Appends an integer to the current string
//
// @param i Unsigned integer value to append
//=============================================
void CString::Append(Uint32 i)
{
	Uint32 digitCount = SDL_floor(SDL_log10(i)) + 1;
	Uint32 newlength = m_stringLength + digitCount;

	g_workBufferMutex.lock();
	CheckBuffer(newlength);
	g_pWorkBuffer[0] = '\0';

	if(m_pString && m_pString != EMPTY_STRING)
		sprintf_s(g_pWorkBuffer, g_workBufferSize, "%s%d", m_pString, i);
	else
		sprintf_s(g_pWorkBuffer, g_workBufferSize, "%d", i);

	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Appends a float value to the current string
//
// @param f Floating point value to append
//=============================================
void CString::Append(Float f)
{
	Uint32 logVal = SDL_log10(SDL_floor(f));
	Uint32 digitCount = (logVal + 1) + 7;
	Uint32 newlength = m_stringLength + digitCount;

	g_workBufferMutex.lock();
	CheckBuffer(newlength);
	g_pWorkBuffer[0] = '\0';

	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(g_pWorkBuffer, "%s%.6f", m_pString, f);
	else
		sprintf(g_pWorkBuffer, "%.6f", f);

	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Appends a double float value to the current string
//
// @param d Double value to append
//=============================================
void CString::Append(Double d)
{
	Uint32 logVal = SDL_log10(SDL_floor(d));
	Uint32 digitCount = (logVal + 1) + 7;
	Uint32 newlength = m_stringLength + digitCount;

	g_workBufferMutex.lock();
	CheckBuffer(newlength);
	g_pWorkBuffer[0] = '\0';

	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(g_pWorkBuffer, "%s%.6lf", m_pString, d);
	else
		sprintf(g_pWorkBuffer, "%.6lf", d);

	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Sets data for the string
//
//=============================================
void CString::setdata( const Char* pString )
{
	m_stringLength = qstrlen(pString);
	if(!(m_flags & fl_str_nopooling))
	{
		// If we have a new entry, then remove that one
		if(m_pString && m_pString != EMPTY_STRING)
			g_pStringPool->RemoveString(m_pPoolCacheEntry);

		// First seek out an existing string entry, and if not found,
		// only then add a new one
		m_pPoolCacheEntry = g_pStringPool->GetExistingString(pString);
		if(!m_pPoolCacheEntry)
			m_pPoolCacheEntry = g_pStringPool->AddString(pString);

		// Set our string ptr as the cache
		m_pString = m_pPoolCacheEntry->iterator->first.c_str();
	}
	else
	{
		// Delete previous string object if it wasn't pointing to EMPTY_STRING
		if(m_pString && m_pString != EMPTY_STRING)
			delete[] m_pString;

		// Set new one
		if(pString && m_stringLength > 0)
		{
			Char* pstrNew = new Char[m_stringLength+1];
			qstrcpy(pstrNew, pString);
			m_pString = pstrNew;
		}
		else
		{
			m_pString = EMPTY_STRING;
		}
	}
}

//=============================================
// @brief Ensures work buffer is of adequate size
//
//=============================================
void CString::CheckBuffer( Uint32 length )
{
	Uint32 fullLength = length+1;
	if(g_workBufferSize >= fullLength)
		return;

	g_pWorkBuffer = new Char[fullLength];
	memset(g_pWorkBuffer, 0, sizeof(Char)*fullLength);
	g_workBufferSize = fullLength;
}