/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CSTRING_INLINE_HPP
#define CSTRING_INLINE_HPP

//=============================================
// @brief Assignment operator
//
// @param str Reference to CString to copy
// @return Reference to CString object
//=============================================
inline CString& CString::operator=(const CString& str)
{
	if(this != &str)
	{
		const Char* psrc = str.c_str();
		if(psrc && m_pString && !qstrcmp(psrc, m_pString))
			return *this;

		if(m_pString)
		{
			if(m_pString != EMPTY_STRING)
			{
				if(!(m_flags & fl_str_nopooling))
					CStringPool::Instance()->RemoveString(m_pPoolCacheEntry);
				else
					delete[] m_pString;
			}

			m_pString = nullptr;
			m_pPoolCacheEntry = nullptr;
		}

		if(!psrc)
		{
			m_pString = EMPTY_STRING;
			m_stringLength = 0;

			return (*this);
		}
		else
		{
			setdata(psrc);
		}
	}

	return *this;
}

//=============================================
// @brief Equals operator
//
// @param pstr Pointer to string to compare
// @return TRUE if strings match, FALSE otherwise
//=============================================
inline bool CString::operator==(const Char* pstr) const
{
	if(!pstr)
	{
		if(m_pString != EMPTY_STRING)
			return false;
		else
			return true;
	}
	else
		return qstrcmp(pstr, m_pString) == 0;
}

//=============================================
// @brief Equals operator
//
// @param str Reference to CString to compare
// @return TRUE if strings match, FALSE otherwise
//=============================================
inline bool CString::operator==(const CString& str) const
{
	if(str.empty())
	{
		if(m_pString != EMPTY_STRING)
			return false;
		else
			return true;
	}
	else
		return qstrcmp(str, m_pString) == 0;
}


//=============================================
// @brief Less than operator
//
// @param pstr Pointer to string to compare
// @return TRUE if string is lower in order, FALSE otherwise
//=============================================
inline bool CString::operator<(const Char* pstr) const
{
	return qstrcmp(pstr, m_pString) < 0 ? true : false;
}

//=============================================
// @brief Less than operator
//
// @param str Reference to CString to compare
// @return TRUE if string is lower in order, FALSE otherwise
//=============================================
inline bool CString::operator<(const CString& str) const
{
	return qstrcmp(str, m_pString) < 0 ? true : false;
}

//=============================================
// @brief Greater than operator
//
// @param pstr Pointer to string to compare
// @return TRUE if string is greater in order, FALSE otherwise
//=============================================
inline bool CString::operator>(const Char* pstr) const
{
	return qstrcmp(pstr, m_pString) > 0 ? true : false;
}

//=============================================
// @brief Greater than operator
//
// @param str Reference to CString to compare
// @return TRUE if string is greater in order, FALSE otherwise
//=============================================
inline bool CString::operator>(const CString& str) const
{
	return qstrcmp(str, m_pString) > 0 ? true : false;
}

//=============================================
// @brief Byte shift operator, appends other string to this string
//
// @param pstr Pointer string
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(const Char* pstr)
{
	Append(pstr);
	return *this;
}

//=============================================
// @brief Byte shift operator, appends other string to this string
//
// @param str Reference to CString string
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(const CString& str)
{
	assert(&str != this);
	Append(str.c_str());
	return *this;
}

//=============================================
// @brief Byte shift operator, appends a single character to this string
//
// @param c Character to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(Char c)
{
	Append(c);
	return *this;
}

//=============================================
// @brief Byte shift operator, appends an integer as a string to this string
//
// @param i Integer value to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(Int32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Byte shift operator, appends unsigned int to this string
//
// @param i Integer value to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(Uint32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Byte shift operator, appends a float to this string
//
// @param f Float value to append as string
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(Float f)
{
	Append(f);
	return *this;
}

//=============================================
// @brief Byte shift operator, appends a Double value to this string
//
// @param d Double value to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator<<(Double d)
{
	Append(d);
	return *this;
}

//=============================================
// @brief Plus equal operator, appends another string to this string
//
// @param pstr Pointer to string to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator+=(const Char* pstr)
{
	Append(pstr);
	return *this;
}

//=============================================
// @brief Plus equal operator, appends another string to this string
//
// @param str Reference to other string
// @return Reference to this CString object
//=============================================
inline CString& CString::operator+=(const CString& str)
{
	Append(str.c_str());
	return *this;
}

//=============================================
// @brief Plus equal operator, appends a single character to this string
//
// @param c Character to append to the string
// @return Reference to this CString object
//=============================================
inline CString& CString::operator+=(Char c)
{
	Append(c);
	return *this;
}

//=============================================
// @brief Plus equal operator, appends an integer value to this string
//
// @param i Integer value to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator+=(Int32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Plus equal operator, appends an unsigned int value to this string
//
// @param i Integer value to append
// @return Reference to this CString object
//=============================================
inline CString& CString::operator+=(Uint32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Plus operator, appends a string to this string
//
// @param pstr Pointer string
// @return Resulting string object
//=============================================
inline CString CString::operator+(const Char* pstr)
{
	CString temp(*this);
	temp << pstr;
	return temp;
}

//=============================================
// @brief Plus operator, appends a CString to this string
//
// @param str Reference to CString object
// @return Resulting string object
//=============================================
inline CString CString::operator+(const CString& str)
{
	assert(&str != this);

	CString temp(*this);
	temp << str.c_str();
	return temp;
}

//=============================================
// @brief Plus operator, appends a single character to this string
//
// @param c Character to append
// @return Resulting string object
//=============================================
inline CString CString::operator+(Char c)
{
	CString temp(*this);
	temp << c;
	return temp;
}

//=============================================
// @brief Plus operator, appends a single integer value to this string
//
// @param i Integer value to append
// @return Resulting string object
//=============================================
inline CString CString::operator+(Int32 i)
{
	CString temp(*this);
	temp << i;
	return temp;
}

//=============================================
// @brief Plus operator, appends a single unsigned integer value to this string
//
// @param i Integer value to append
// @return Resulting string object
//=============================================
inline CString CString::operator+(Uint32 i)
{
	CString temp(*this);
	temp << i;
	return temp;
}

//=============================================
// @brief Plus operator, appends a single float value to this string
//
// @param f Float value to append
// @return Resulting string object
//=============================================
inline CString CString::operator+(Float f)
{
	CString temp(*this);
	temp << f;
	return temp;
}

//=============================================
// @brief Plus operator, appends a single double value to this string
//
// @param d Double value to append
// @return Resulting string object
//=============================================
inline CString CString::operator+(Double d)
{
	CString temp(*this);
	temp << d;
	return temp;
}

//=============================================
// @brief Array operator
//
// @param n Index in string
// @return String character at n position
//=============================================
inline Char CString::operator[] (Uint32 n) const
{
	assert(m_pString != nullptr);
	assert(n < m_stringLength);

	if(n < m_stringLength)
		return m_pString[n];
	else
		return '\0';
}

//=============================================
// @brief Returns the length of the string, not counting '/0' terminator char
//
// @return Length of the string
//=============================================
inline Uint32 CString::length( void ) const
{
	return m_stringLength;
}

//=============================================
// @brief Tells if the string is empty
//
// @return TRUE if string is empty, FALSE otherwise
//=============================================
inline bool CString::empty( void ) const
{
	return (!m_stringLength || !m_pString || m_pString == EMPTY_STRING);
}

//=============================================
// @brief Returns a const pointer to the raw string data
//
// @return Pointer to raw string data
//=============================================
inline const Char* CString::c_str( void ) const
{
	return m_pString;
}

//=============================================
// @brief Clears the string
//
//=============================================
inline void CString::clear( void )
{
	if(m_stringLength == 0 && m_pString && m_pString == EMPTY_STRING)
		return;

	m_stringLength = 0;
	if(m_pString && m_pString != EMPTY_STRING)
	{
		if(!(m_flags & fl_str_nopooling))
			g_pStringPool->RemoveString(m_pPoolCacheEntry);
		else
			delete[] m_pString;
	}

	m_pString = EMPTY_STRING;
	m_pPoolCacheEntry = nullptr;
}

//=============================================
// @brief Assigns the contents of this string
//
//=============================================
inline void CString::assign( const Char* pstr, Uint32 num )
{
	g_workBufferMutex.lock();
	CheckBuffer(num);
	g_pWorkBuffer[0] = '\0';

	qstrncpy(g_pWorkBuffer, pstr, num);
	g_pWorkBuffer[num] = '\0';

	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Adds two strings together
//
// @param lhs Left hand string
// @param rhs Right hand string
// @return End result string object
//=============================================
inline CString operator + ( const CString& lhs, const CString& rhs )
{
	assert(&lhs != &rhs);

	CString strOut(lhs);
	strOut.Append(rhs.c_str());

	return strOut;
}

//=============================================
// @brief Finds occurrence of a substring in the string
//
// @param offset Index of first character to start from
// @param psubstr Substring to find in this string
// @return Position at which the substring was located
//=============================================
inline Int32 CString::find( Uint32 offset, const Char* psubstr )
{
	if(empty())
		return CSTRING_NO_POSITION;

	assert(offset < m_stringLength);
	const Char* pstrbegin = m_pString + offset;

	const Uint32 substrlength = qstrlen(psubstr);
	const Char* pstr = pstrbegin;
	while(*pstr != '\0')
	{
		if(pstr[0] == psubstr[0])
		{
			if(!qstrncmp(pstr, psubstr, substrlength))
				break;
		}

		pstr++;
	}

	if(*pstr == '\0')
		return CSTRING_NO_POSITION;

	return (pstr - m_pString);
}

//=============================================
// @brief Erases part of a string
//
// @param begin Index of starting character to erase
// @param numremove Number of characters to erase
//=============================================
inline void CString::erase( Uint32 begin, Uint32 numremove )
{
	assert(begin < m_stringLength && (begin+numremove) <= m_stringLength);

	g_workBufferMutex.lock();
	const Uint32 newsize = m_stringLength - numremove;
	CheckBuffer(newsize);
	g_pWorkBuffer[0] = '\0';

	Uint32 srcindex = 0;
	Uint32 dstindex = 0;
	while(srcindex < begin)
	{
		g_pWorkBuffer[dstindex] = m_pString[srcindex];
		dstindex++; srcindex++;
	}

	// Copy trailing if present
	const Uint32 end = begin + numremove;
	if(end < m_stringLength)
	{
		srcindex = end;
		while(srcindex < m_stringLength)
		{
			g_pWorkBuffer[dstindex] = m_pString[srcindex];
			dstindex++; srcindex++;
		}
	}

	// Terminate string
	g_pWorkBuffer[dstindex] = '\0';
	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}
//=============================================
// @brief Inserts a string into this string
//
// @param begin Index of starting character to start at
// @param pstrsubstr Substring to insert into this string
//=============================================
inline void CString::insert( Uint32 begin, const Char* pstrsubstr )
{
	assert(begin <= m_stringLength);

	g_workBufferMutex.lock();
	Uint32 substrlength = qstrlen(pstrsubstr);
	Uint32 newlength = m_stringLength+substrlength;

	CheckBuffer(newlength);
	g_pWorkBuffer[0] = '\0';

	memcpy(g_pWorkBuffer, m_pString, begin);

	// Add in substring and the rest
	memcpy(g_pWorkBuffer+begin, pstrsubstr, substrlength);
	memcpy(g_pWorkBuffer+begin+substrlength, m_pString+begin, m_stringLength-begin);

	// Set new contents
	g_pWorkBuffer[newlength] = '\0';
	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Converts the string to lowercase chars
//
//=============================================
inline void CString::tolower( void )
{
	if(m_pString == EMPTY_STRING)
		return;

	Uint32 i = 0;
	for(; i < m_stringLength; i++)
	{
		if(m_pString[i] != ::tolower(m_pString[i]))
			break;
	}

	if(i == m_stringLength)
		return;

	g_workBufferMutex.lock();
	CheckBuffer(m_stringLength);

	Char* pInsert = g_pWorkBuffer;
	for(const Char* pTemp = m_pString; *pTemp; pTemp++, pInsert++) 
		(*pInsert) = ::tolower(*pTemp);

	g_pWorkBuffer[m_stringLength] = '\0';
	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}

//=============================================
// @brief Replaces slashes with PATH_SLASH_CHAR
//
//=============================================
inline void CString::replaceslashes( void )
{
	if(m_pString == EMPTY_STRING)
		return;

	Uint32 i = 0;
	for(; i < m_stringLength; i++)
	{
		if(m_pString[i] == '\\')
			break;
	}

	if(i == m_stringLength)
		return;

	g_workBufferMutex.lock();
	CheckBuffer(m_stringLength);

	Char* pInsert = g_pWorkBuffer;
	for(const Char* pTemp = m_pString; *pTemp; pTemp++, pInsert++)
	{
		if(*pTemp == '\\')
			(*pInsert) = PATH_SLASH_CHAR;
		else
			(*pInsert) = *pTemp;
	}

	g_pWorkBuffer[m_stringLength] = '\0';
	setdata(g_pWorkBuffer);
	g_workBufferMutex.unlock();
}
#endif // CSTRING_INLINE_HPP