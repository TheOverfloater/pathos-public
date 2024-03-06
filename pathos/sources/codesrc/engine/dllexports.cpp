/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

// Special thanks to user5588894 for this code
// This is used for 32-bit mode, because Detours refuses
// to compile properly with x86

#include "includes.h"
#include "datatypes.h"
#include "dllexports.h"

extern Int32 Rva2Offset( Uint32 rva, Uint32 numSections, sectionheader_t* pSections ) ;

//=============================================
// @brief Initializes the basic systems
// 
// @param pparams Launch parameters
// @return TRUE if successful, FALSE on error
//=============================================
Int32 Rva2Offset( Uint32 rva, Uint32 numSections, sectionheader_t* pSections ) 
{
    Int32 i = 0;
    for (; i < numSections; i++) {
        Uint32 x = pSections[i].VirtualAddress + pSections[i].SizeOfRawData;

        if (x >= rva)
            return pSections[i].PointerToRawData + (rva + pSections[i].SizeOfRawData) - x;
    }
    return -1;
}

//=============================================
// @brief Initializes the basic systems
// 
// @param pparams Launch parameters
// @return TRUE if successful, FALSE on error
//=============================================
bool EnumExportedFunctions ( const Char *szFilename, void (*pfnCallBack)(Char*) ) 
{
    FILE *hFile = fopen (szFilename, "rb");
	if(!hFile)
		return false;

	sectionheader_t *pSections;
	Uint32 numSections = 0;

    if (fgetc (hFile) != 'M' || fgetc (hFile) != 'Z')
	{
		fclose(hFile);
		return false;
	}

    Uint32 e_lfanew = 0;
    Uint32 numberOfRvaAndSizes = 0;
    Uint32 exportVirtualAddress = 0;
    Uint32 exportSize = 0;
    Int32 i = 0;

    fseek(hFile, 0x3C, SEEK_SET);
    fread(&e_lfanew, 4, 1, hFile);
    fseek(hFile, e_lfanew + 6, SEEK_SET);
    fread(&numSections, 2, 1, hFile);
    fseek(hFile, 108, SEEK_CUR);
    fread(&numberOfRvaAndSizes, 4, 1, hFile);

    if (numberOfRvaAndSizes != 16) 
	{
		fclose(hFile);
		return false;
	}

    fread (&exportVirtualAddress, 4, 1, hFile);
    fread (&exportSize, 4, 1, hFile);

    if (exportVirtualAddress <= 0 || exportSize <= 0)
	{
		fclose(hFile);
		return false;
	}
	
    fseek (hFile, 120, SEEK_CUR);

    if (numSections <= 0)
	{
		fclose(hFile);
		return false;
	}

    pSections = reinterpret_cast<sectionheader_t *>(malloc(numSections * sizeof(sectionheader_t)));

    for (i = 0; i < numSections; i++) 
	{
        fread(pSections[i].Name, 8, 1, hFile);
        fread(&pSections[i].VirtualSize, 4, 1, hFile);
        fread(&pSections[i].VirtualAddress, 4, 1, hFile);
        fread(&pSections[i].SizeOfRawData, 4, 1, hFile);
        fread(&pSections[i].PointerToRawData, 4, 1, hFile);
        fread(&pSections[i].PointerToRelocations, 4, 1, hFile);
        fread(&pSections[i].PointerToLineNumbers, 4, 1, hFile);
        fread(&pSections[i].NumberOfRelocations, 2, 1, hFile);
        fread(&pSections[i].NumberOfLineNumbers, 2, 1, hFile);
        fread(&pSections[i].Characteristics, 4, 1, hFile);
    }

    Uint32 numberOfNames = 0;
    Uint32 addressOfNames = 0;

    Int32 offset = Rva2Offset (exportVirtualAddress, numSections, pSections);
    fseek(hFile, offset + 24, SEEK_SET);
    fread(&numberOfNames, 4, 1, hFile);

    fseek(hFile, 4, SEEK_CUR);
    fread(&addressOfNames, 4, 1, hFile);

    Int32 namesOffset = Rva2Offset (addressOfNames, numSections, pSections), pos = 0;
    fseek(hFile, namesOffset, SEEK_SET);

    for (i = 0; i < numberOfNames; i++) 
	{
        Int32 y = 0;
        fread (&y, 4, 1, hFile);
        pos = ftell (hFile);
        fseek (hFile, Rva2Offset (y, numSections, pSections), SEEK_SET);

        Char c = fgetc (hFile);
        int szNameLen = 0;

        while (c != '\0') 
		{
            c = fgetc (hFile);
            szNameLen++;
        }

        fseek (hFile, (-szNameLen)-1, SEEK_CUR);
        Char* szName = reinterpret_cast<Char*>(calloc (szNameLen + 1, 1));
        fread (szName, szNameLen, 1, hFile);

        pfnCallBack (szName);

        fseek (hFile, pos, SEEK_SET);
    }

    fclose (hFile);

	return true;
}