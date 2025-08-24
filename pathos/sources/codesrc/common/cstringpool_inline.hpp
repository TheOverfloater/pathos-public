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
// @brief Find an existing string, and if found, increase 
// it's refcount before returning the cache entry
//
// @param pstrString String to find
// @return Cache entry if present, or nullptr
//=============================================
inline CStringPool::cachestring_t* CStringPool::GetExistingString( const Char* pstrString )
{
	// Seek it out in the map first
	CacheStringMapIteratorType_t it = m_stringMap.find(pstrString);
	if(it != m_stringMap.end())
	{
		cachestring_t* pcache = it->second;
		m_mutex.lock();
		pcache->increment();
		m_mutex.unlock();
		return pcache;
	}
	else
	{
		return nullptr;
	}
}

//=============================================
// @brief Add new string to the pool or increments 
// an existing one's refcount
//
// @param pstrString String to find
// @return Cache entry if present, or nullptr
//=============================================
inline CStringPool::cachestring_t* CStringPool::AddString( const Char* pstrString )
{
	cachestring_t* pnew = new cachestring_t;
	pnew->increment();

	m_mutex.lock();
	pnew->iterator = m_stringMap.insert(std::pair<std::string, cachestring_t*>(pstrString, pnew)).first;
	m_mutex.unlock();

	return pnew;
}

//=============================================
// @brief Remove a string from the string cache
//
// @param pCacheEntry String entry to remove
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
// @retrun Current instance of the string pool
//=============================================
inline CStringPool* CStringPool::Instance( void )
{
	return &g_poolInstance;
}
#endif //CSTRINGPOOL_INLINE_HPP