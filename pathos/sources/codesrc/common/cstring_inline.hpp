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
			const Uint32 strlength = str.length();
			Char* pString = new Char[strlength+1];
			qstrcpy(pString, psrc);
			setdata(pString, strlength);
		}
	}

	return *this;
}

//=============================================
// @brief Comparison operator
//
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator==(const Char* pstr) const
{
	if(!pstr)
	{
		if(m_pString)
			return false;
		else
			return true;
	}
	else
		return qstrcmp(pstr, m_pString) == 0;
}

//=============================================
// @brief Comparison operator
//
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator==(const CString& str) const
{
	if(str.empty())
	{
		if(m_pString)
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
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator<(const Char* pstr) const
{
	return qstrcmp(pstr, m_pString) < 0 ? true : false;
}

//=============================================
// @brief Less than operator
//
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator<(const CString& str) const
{
	return qstrcmp(str, m_pString) < 0 ? true : false;
}

//=============================================
// @brief Greater than operator
//
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator>(const Char* pstr) const
{
	return qstrcmp(pstr, m_pString) > 0 ? true : false;
}

//=============================================
// @brief Greater than operator
//
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator>(const CString& str) const
{
	return qstrcmp(str, m_pString) > 0 ? true : false;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(const Char* pstr)
{
	Append(pstr);
	return *this;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(const CString& str)
{
	assert(&str != this);
	Append(str.c_str());
	return *this;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(Char c)
{
	Append(c);
	return *this;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(Int32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(Uint32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(Float f)
{
	Append(f);
	return *this;
}

//=============================================
// @brief Byte shift operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator<<(Double d)
{
	Append(d);
	return *this;
}

//=============================================
// @brief Plus equal operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator+=(const Char* pstr)
{
	Append(pstr);
	return *this;
}

//=============================================
// @brief Plus equal operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator+=(const CString& str)
{
	Append(str.c_str());
	return *this;
}

//=============================================
// @brief Plus equal operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator+=(Char c)
{
	Append(c);
	return *this;
}

//=============================================
// @brief Plus equal operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator+=(Int32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Plus equal operator
//
// @param pstr Pointer string
//=============================================
inline CString& CString::operator+=(Uint32 i)
{
	Append(i);
	return *this;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(const Char* pstr)
{
	CString temp(*this);
	temp << pstr;
	return temp;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(const CString& str)
{
	assert(&str != this);

	CString temp(*this);
	temp << str.c_str();
	return temp;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(Char c)
{
	CString temp(*this);
	temp << c;
	return temp;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(Int32 i)
{
	CString temp(*this);
	temp << i;
	return temp;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(Uint32 i)
{
	CString temp(*this);
	temp << i;
	return temp;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(Float f)
{
	CString temp(*this);
	temp << f;
	return temp;
}

//=============================================
// @brief Plus operator
//
// @param pstr Pointer string
//=============================================
inline CString CString::operator+(Double d)
{
	CString temp(*this);
	temp << d;
	return temp;
}

//=============================================
// @brief Appends a source string to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(const Char* psrc)
{
	if(psrc)
	{
		const Uint32 srclength = qstrlen(psrc);
		const Uint32 newlength = srclength + m_stringLength;

		Char* pstrNew = new Char[newlength+1];
		if(m_pString && m_pString != EMPTY_STRING)
			sprintf(pstrNew, "%s%s", m_pString, psrc);
		else
			qstrcpy(pstrNew, psrc);

		setdata(pstrNew, newlength);
	}
}

//=============================================
// @brief Appends a char to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Char c)
{
	const Uint32 newlength = m_stringLength+1;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(pstrNew, "%s%c", m_pString, c);
	else
		sprintf(pstrNew, "%c", c);

	setdata(pstrNew, newlength);
}

//=============================================
// @brief Appends an integer to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Int32 i)
{
	static Char convBuffer[64];
	sprintf_s(convBuffer, "%d", i);
	const Int32 strlength = qstrlen(convBuffer);

	const Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	setdata(pstrNew, newlength);
}

//=============================================
// @brief Appends an integer to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Uint32 i)
{
	static Char convBuffer[64];
	sprintf_s(convBuffer, "%d", static_cast<Int32>(i));
	const Int32 strlength = qstrlen(convBuffer);

	const Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	setdata(pstrNew, newlength);
}

//=============================================
// @brief Appends a float value to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Float f)
{
	static Char convBuffer[64];
	sprintf_s(convBuffer, "%f", f);

	const Int32 strlength = qstrlen(convBuffer);
	const Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	setdata(pstrNew, newlength);
}

//=============================================
// @brief Appends a float value to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Double d)
{
	static Char convBuffer[64];
	sprintf_s(convBuffer, "%9.9lf", d);

	const Int32 strlength = qstrlen(convBuffer);
	const Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString && m_pString != EMPTY_STRING)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	setdata(pstrNew, newlength);
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
	assert(n < qstrlen(m_pString));
	return m_pString[n];
}

//=============================================
// @brief Returns the size of the string
//
// @param n Index in string
// @return length of the string
//=============================================
inline Uint32 CString::length( void ) const
{
	return m_stringLength;
}

//=============================================
// @brief Returns the size of the string
//
// @param n Index in string
// @return length of the string
//=============================================
inline bool CString::empty( void ) const
{
	return (!m_stringLength || !m_pString || m_pString == EMPTY_STRING);
}

//=============================================
// @brief Returns the size of the string
//
// @param n Index in string
// @return length of the string
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
// @brief Assigns a string to use
//
//=============================================
inline void CString::assign( const Char* pstr, Uint32 num )
{
	Char* pString = new Char[num+1];
	qstrncpy(pString, pstr, num);
	pString[num] = '\0';

	setdata(pString, num);
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
// @param begin index of starting character to erase
// @param end index of last character to erase
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
// @param begin index of starting character to erase
// @param end index of last character to erase
//=============================================
inline void CString::erase( Uint32 begin, Uint32 numremove )
{
	assert(begin < m_stringLength && (begin+numremove) <= m_stringLength);

	const Uint32 newsize = m_stringLength - numremove;
	Char* pstrnew = new Char[newsize+1];

	Uint32 srcindex = 0;
	Uint32 dstindex = 0;
	while(srcindex < begin)
	{
		pstrnew[dstindex] = m_pString[srcindex];
		dstindex++; srcindex++;
	}

	// Copy trailing if present
	const Uint32 end = begin + numremove;
	if(end < m_stringLength)
	{
		srcindex = end;
		while(srcindex < m_stringLength)
		{
			pstrnew[dstindex] = m_pString[srcindex];
			dstindex++; srcindex++;
		}
	}

	// Terminate string
	pstrnew[dstindex] = '\0';
	setdata(pstrnew, dstindex);
}
//=============================================
// @brief Erases part of a string
//
// @param begin index of starting character to erase
// @param end index of last character to erase
//=============================================
inline void CString::insert( Uint32 begin, const Char* pstrsubstr )
{
	assert(begin <= m_stringLength);

	Uint32 substrlength = qstrlen(pstrsubstr);
	Char* pstrnew = new Char[m_stringLength + substrlength + 1];
	memcpy(pstrnew, m_pString, begin);

	// Add in substring and the rest
	memcpy(pstrnew+begin, pstrsubstr, substrlength);
	memcpy(pstrnew+begin+substrlength, m_pString+begin, m_stringLength-begin);

	// Set new size
	Uint32 newlength = m_stringLength+substrlength;
	pstrnew[newlength] = '\0';
	setdata(pstrnew, newlength);
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

	Char* pNew = new Char[m_stringLength+1];
	Char* pInsert = pNew;
	for(const Char* pTemp = m_pString; *pTemp; pTemp++, pInsert++) 
		(*pInsert) = ::tolower(*pTemp);

	pNew[m_stringLength] = '\0';
	setdata(pNew, m_stringLength);
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

	Char* pNew = new Char[m_stringLength+1];
	Char* pInsert = pNew;
	for(const Char* pTemp = m_pString; *pTemp; pTemp++, pInsert++)
	{
		if(*pTemp == '\\')
			(*pInsert) = PATH_SLASH_CHAR;
		else
			(*pInsert) = *pTemp;
	}

	pNew[m_stringLength] = '\0';
	setdata(pNew, m_stringLength);
}
#endif // CSTRING_INLINE_HPP