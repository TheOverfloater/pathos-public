/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef HASHLIST_H
#define HASHLIST_H

#include "md5.h"

/*
====================
CHashList

====================
*/
class CHashList
{
public:
	CHashList() {};
	~CHashList() {};

public:
	// Adds a data hash to the list, returns true if new, false otherwise
	bool addhash( const byte* pdata, Uint32 size );
	// Clears the hash list
	void clear( void );

private:
	// Array of hashes stored so far
	CArray<CString> m_hashArray;
};
#include "hashlist_inline.hpp"
#endif