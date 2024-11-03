/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CSTRINGPOOL_INLINE_HPP
#define CSTRINGPOOL_INLINE_HPP

//=============================================
// @brief Add new string to the pool or increments an existing one's refcount
//
//=============================================
inline CStringPool::cachestring_t* CStringPool::AddString( const Char* pstrString )
{
	m_mutex.lock();

	// Seek it out in the map first
	CacheStringMapIteratorType_t it = m_stringMap.find(pstrString);
	if(it != m_stringMap.end())
	{
		cachestring_t* pcache = it->second;
		pcache->increment();
		m_mutex.unlock();
		delete[] pstrString;
		return pcache;
	}

	cachestring_t* pnew = new cachestring_t;
	pnew->increment();
	pnew->iterator = m_stringMap.insert(std::pair<std::string, cachestring_t*>(pstrString, pnew)).first;
	delete[] pstrString;

	m_mutex.unlock();
	return pnew;
}

//=============================================
// @brief Add new string to the pool or increments an existing one's refcount
//
//=============================================
inline void CStringPool::RemoveString( cachestring_t* pCacheEntry )
{
	m_mutex.lock();

	// See if decrementing clears this
	if(pCacheEntry->decrement())
	{
		m_mutex.unlock();
		return;
	}

	m_stringMap.erase(pCacheEntry->iterator);
	delete pCacheEntry;

	m_mutex.unlock();
}

//=============================================
// @brief Retrieve the current instance of the pool or create it
//
//=============================================
inline CStringPool* CStringPool::Instance( void )
{
	return &g_poolInstance;
}
#endif //CSTRINGPOOL_INLINE_HPP