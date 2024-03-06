/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CRC32_H
#define CRC32_H

//
// I'd like to say thanks to NTLDS from StackOverflow 
// for the original version of this code
//

#define CRC32_POLYNOMAL_TAB_SIZE 256

/*
=======================
CCRC32Hash

=======================
*/
class CCRC32Hash
{
public:
	// CRC polynomal tab
	static Uint32 CRC32_POLYNOMAL_TAB[CRC32_POLYNOMAL_TAB_SIZE];

public:
	CCRC32Hash();
	CCRC32Hash( const byte* pbuffer, Uint32 datasize );
	~CCRC32Hash();

public:
	void GenerateHash( const byte* pbuffer, Uint32 datasize );
	Uint32 GetHashResult( void ) const;

private:
	static Uint32 UPDC32( byte octet, Uint32 crc );

private:
	// Hash result if any
	Uint32 m_crc32Result;
};
#endif //CRC32_H