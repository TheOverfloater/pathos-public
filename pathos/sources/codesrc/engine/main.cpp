/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>

#ifdef USE_VLD
#include <vld.h>
#endif

#include "includes.h"
#include "system.h"

extern "C" {
		__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
		__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
	}

//===============================================
// _tmain
//
//===============================================
int _tmain(Int32 argc, Char* argv[])
{
	// Store arguments in an array
	CArray<CString>* argsArray;
	argsArray = new CArray<CString>();

	for(Int32 i = 0; i < argc; i++)
		argsArray->push_back(reinterpret_cast<Char*>(argv[i]));

	// Run main loop
	return Sys_Main(argsArray);
}