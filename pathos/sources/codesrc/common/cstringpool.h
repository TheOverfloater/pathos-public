/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CSTRINGPOOL_H
#define CSTRINGPOOL_H

#include <string>
#include <unordered_map>
#include <mutex>

#include "carray.h"
#include "clinkedlist.h"
#include "crc32.h"

/*
=======================
CStringPool

=======================
*/
class CStringPool
{
public:
	struct cachestring_t
	{
		// Constructor for cachestring_t
		cachestring_t():
			refcount(0)
		{
		}

		// Destructor for cachestring_t
		~cachestring_t()
		{
		}

		// Increments reference counter
		void increment( void )
		{
			refcount++;
		}

		// Decrements reference counter
		bool decrement( void )
		{
			refcount--;
			return (refcount <= 0) ? false : true;
		}

		Uint32 refcount;
		std::unordered_map<std::string, cachestring_t*>::iterator iterator;
	};

	typedef std::unordered_map<std::string, cachestring_t*> CacheStringMapType_t;
	typedef CacheStringMapType_t::iterator CacheStringMapIteratorType_t;

public:
	// Constructor
	CStringPool( void );
	// Destructor
	~CStringPool( void );

public:
	// Find an existing string, and if found, increase it's refcount
	inline cachestring_t* GetExistingString( const Char* pstrString );
	// Add new string to the pool or increments an existing one's refcount
	inline cachestring_t* AddString( const Char* pstrString );
	// Removes an instance of a string used
	inline void RemoveString( cachestring_t* pCacheEntry );

public:
	// Retrieve the current instance of the pool
	inline static CStringPool* Instance( void );

private:
	// Map to individual elements
	CacheStringMapType_t m_stringMap;
	// The mutex used to lock access to a single thread
	std::mutex m_mutex;

private:
	// Current instance of the string pool
	static CStringPool g_poolInstance;
};
#include "cstringpool_inline.hpp"
#endif //CSTRINGPOOL_H