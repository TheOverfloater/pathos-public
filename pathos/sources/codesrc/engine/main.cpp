/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Required on Windows because SDL2 redirects "main" (and all Windows variants)
// to its own before coming back here.
#include <SDL.h>

#ifdef USE_VLD
#include <vld.h>
#endif

#include "includes.h"
#include "system.h"

// Hint Windows to prefer discrete AMD/NVIDIA GPUs over integrated ones (mostly
// for laptops).
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0x00000001;
}

//===============================================
// main
// SDL2's Windows FAQ says the standard "main" is prefered over any Windows version.
//===============================================
int main(Int32 argc, Char* argv[])
{
	// Store arguments in an array
	CArray<CString>* argsArray;
	argsArray = new CArray<CString>();

	for (Int32 i = 0; i < argc; i++)
		argsArray->push_back(argv[i]);

	// Run main loop
	return Sys_Main(argsArray);
}
