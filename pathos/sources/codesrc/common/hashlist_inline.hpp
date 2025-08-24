/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef HASHLIST_INLINE_HPP
#define HASHLIST_INLINE_HPP

//=============================================
// @brief Take byte data buffer and store it's MD5 hash
//
// @param pdata Pointer to byte buffer
// @param size Size of byte buffer
// @return FALSE if hash was already present, FALSE otherwise
//=============================================
inline bool CHashList::addhash( const byte* pdata, Uint32 size ) 
{
	// Get the MD5 hash of the data
	CString hash = CMD5(pdata, size).HexDigest();

	// See if it's already present
	for(Uint32 i = 0; i < m_hashArray.size(); i++)
	{
		if(!qstrcmp(m_hashArray[i], hash))
			return false;
	}

	m_hashArray.push_back(hash);
	return true;
}

//=============================================
// @brief Clear thehash list of any entries.
//
//=============================================
inline void CHashList::clear( void ) 
{
	if(!m_hashArray.empty())
		return;

	m_hashArray.clear();
}
#endif //HASHLIST_INLINE_HPP
