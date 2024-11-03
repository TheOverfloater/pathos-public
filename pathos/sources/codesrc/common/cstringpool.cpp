/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cstringpool.h"

// Current instance of the string pool, needs to be allocated on stack
CStringPool CStringPool::g_poolInstance;

//=============================================
// @brief Default constructor
//
//=============================================
CStringPool::CStringPool( void )
{
	m_stringMap.clear();
}

//=============================================
// @brief Destructor
//
//=============================================
CStringPool::~CStringPool( void )
{
	if(m_stringMap.empty())
		return;

	CacheStringMapIteratorType_t it = m_stringMap.begin();
	while(it != m_stringMap.end())
	{
		delete it->second;
		it++;
	}

	m_stringMap.clear();
}
