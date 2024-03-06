/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DLLEXPORTS_H
#define DLLEXPORTS_H

struct sectionheader_t
{
	sectionheader_t():
		VirtualSize(0),
		VirtualAddress(0),
		SizeOfRawData(0),
		PointerToRawData(0),
		PointerToRelocations(0),
		PointerToLineNumbers(0),
		NumberOfRelocations(0),
		NumberOfLineNumbers(0),
		Characteristics(0)
		{
			memset(Name, 0, sizeof(Name));
		}

    byte Name[8];
    Uint32 VirtualSize;
    Uint32 VirtualAddress;
    Uint32 SizeOfRawData;
    Uint32 PointerToRawData;
    Uint32 PointerToRelocations;
    Uint32 PointerToLineNumbers;
    Uint16 NumberOfRelocations;
    Uint16 NumberOfLineNumbers;
    Uint32 Characteristics;
};

bool EnumExportedFunctions ( const Char *szFilename, void (*pfnCallBack)(Char*) );
#endif //DLLEXPORTS_H