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
			delete[] m_pString;
			m_pString = nullptr;
		}

		if(!psrc)
		{
			m_pString = new Char;
			(*m_pString) = '\0';
			m_stringLength = 0;

			return (*this);
		}

		Uint32 strlength = str.length();
		m_pString = new Char[strlength+1];
		qstrcpy(m_pString, psrc);
		m_stringLength = strlength;
	}

	return *this;
}

//=============================================
// @brief Comparison operator
//
// @param pstr Pointer to CString to compare
//=============================================
inline bool CString::operator==(const Char* pstr)
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
inline bool CString::operator==(const CString& str)
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
		Uint32 srclength = qstrlen(psrc);
		Uint32 newlength = srclength + m_stringLength;

		Char* pstrNew = new Char[newlength+1];
		if(m_pString)
			sprintf(pstrNew, "%s%s", m_pString, psrc);
		else
			qstrcpy(pstrNew, psrc);

		delete m_pString;
		m_pString = pstrNew;
		m_stringLength = newlength;
	}
}

//=============================================
// @brief Appends a char to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Char c)
{
	Uint32 newlength = m_stringLength+1;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString)
		sprintf(pstrNew, "%s%c", m_pString, c);
	else
		sprintf(pstrNew, "%c", c);

	delete[] m_pString;
	m_pString = pstrNew;
	m_stringLength = newlength;
}

//=============================================
// @brief Appends an integer to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Int32 i)
{
	static Char convBuffer[64];
	sprintf(convBuffer, "%d", i);
	Int32 strlength = qstrlen(convBuffer);

	Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	delete[] m_pString;
	m_pString = pstrNew;
	m_stringLength = newlength;
}

//=============================================
// @brief Appends an integer to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Uint32 i)
{
	static Char convBuffer[64];
	sprintf(convBuffer, "%d", (Int32)i);
	Int32 strlength = qstrlen(convBuffer);

	Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	delete[] m_pString;
	m_pString = pstrNew;
	m_stringLength = newlength;
}

//=============================================
// @brief Appends a float value to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Float f)
{
	static Char convBuffer[64];
	sprintf(convBuffer, "%f", f);

	Int32 strlength = qstrlen(convBuffer);
	Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	delete[] m_pString;
	m_pString = pstrNew;
	m_stringLength = newlength;
}

//=============================================
// @brief Appends a float value to the current string
//
// @param pstr Pointer string
//=============================================
inline void CString::Append(Double d)
{
	static Char convBuffer[64];
	sprintf(convBuffer, "%9.9lf", d);

	Int32 strlength = qstrlen(convBuffer);
	Uint32 newlength = m_stringLength+strlength;

	Char* pstrNew = new Char[newlength+1];
	if(m_pString)
		sprintf(pstrNew, "%s%s", m_pString, convBuffer);
	else
		sprintf(pstrNew, "%s", convBuffer);

	delete[] m_pString;
	m_pString = pstrNew;
	m_stringLength = newlength;
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
	return (!m_stringLength || !m_pString || m_pString[0] == '\0');
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
	if(m_stringLength == 0 && m_pString && m_pString[0] == '\0')
		return;

	m_stringLength = 0;
	if(m_pString)
		delete[] m_pString;

	m_pString = new Char;
	*m_pString = '\0';
}

//=============================================
// @brief Assigns a string to use
//
//=============================================
inline void CString::assign( const Char* pstr, Uint32 num )
{
	if(m_pString)
		delete[] m_pString;

	m_pString = new Char[num+1];
	qstrncpy(m_pString, pstr, num);
	m_pString[num] = '\0';

	m_stringLength = num;
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
		return -1;

	assert(offset < m_stringLength);
	const Char* pstrbegin = m_pString + offset;

	Uint32 substrlength = qstrlen(psubstr);
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
		return -1;

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

	Uint32 newsize = m_stringLength - numremove;
	Char* pstrnew = new Char[newsize+1];

	Uint32 srcindex = 0;
	Uint32 dstindex = 0;
	while(srcindex < begin)
	{
		pstrnew[dstindex] = m_pString[srcindex];
		dstindex++; srcindex++;
	}

	// Copy trailing if present
	Uint32 end = begin + numremove;
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

	delete[] m_pString;
	m_pString = pstrnew;
	m_stringLength = dstindex;
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
	m_stringLength = m_stringLength+substrlength;
	pstrnew[m_stringLength] = '\0';

	delete[] m_pString;
	m_pString = pstrnew;
}

//=============================================
// @brief Converts the string to lowercase chars
//
//=============================================
inline void CString::tolower( void ) const
{
	for(Char* ptemp = m_pString; *ptemp; ptemp++) 
		(*ptemp) = ::tolower(*ptemp);
}

//=============================================
// @brief Replaces slashes with PATH_SLASH_CHAR
//
//=============================================
inline void CString::replaceslashes( void )
{
	for(Char* ptemp = m_pString; *ptemp; ptemp++)
	{
		if(*ptemp == '\\')
			*ptemp = PATH_SLASH_CHAR;
	}
}

#endif // CSTRING_INLINE_HPP