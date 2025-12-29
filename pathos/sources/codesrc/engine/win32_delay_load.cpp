/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

/*
===============================================
Description:
Windows specific code to delay load the correct SDL2 DLL based on the architecture being used.
Other libraries (like OpenAL) are handled by SDL itself so it's unnecessary to add them here.
===============================================
*/

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#include <Windows.h>

#include <delayimp.h>

// Base path for libraries based on the architecture
#ifdef _64BUILD
#define ARCH_LIBRARY_PATH "x64"
#else
#define ARCH_LIBRARY_PATH "x86"
#endif

// Utility function to show error message boxes.
// Pass "nullptr" to "szProcedure" if SDL2 is being loaded and said loading fails.
// Or if it's already been loaded and we're trying to access a corrupted/unknown function or whatever,
// pass its name in "szProcedure".
static void ShowLibraryError(const char *szProcedure)
{
	char szBuffer[512];
	if (szProcedure)
		wsprintfA(szBuffer, "The procedure \"%s\" is missing from the \"SDL2.dll\" library.\n\nThe library itself could be obsolete, corrupted or too new.", szProcedure);
	else
		wsprintfA(szBuffer, "Failed to load the \"SDL2.dll\" library.\n\nEither it's missing or it could not be loaded.");

	MessageBoxA(nullptr, szBuffer, "Fatal Error", MB_OK | MB_ICONHAND);
}

// The actual delay load mechanism.
// Since there is no need to deal with Unicode, we can use the ANSI version of WinAPI directly.
static FARPROC WINAPI DelayLoadNotifyHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	if (dliNotify != dliNotePreLoadLibrary || _stricmp(pdli->szDll, "SDL2.dll") != 0)
		return nullptr;

	HMODULE hSDL2 = LoadLibraryA(ARCH_LIBRARY_PATH "\\SDL2.dll");
	if (!hSDL2)
		return nullptr;

	return reinterpret_cast<FARPROC>(hSDL2);
}

// The failure mechanism.
static FARPROC WINAPI DelayLoadFailureHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
	if (_stricmp(pdli->szDll, "SDL2.dll") != 0)
		return nullptr;

	switch (dliNotify)
	{
	case dliFailLoadLib:
		// Failed to load the library (missing or corrupted)
		ShowLibraryError(nullptr);
		break;
	case dliFailGetProc:
		// Trying to access a function but said function does not exist or corrupted or something
		ShowLibraryError(pdli->dlp.szProcName ? pdli->dlp.szProcName : "Unknown");
		break;
	default:
		// No-op
		break;
	}

	// Close the program immediately with an error exit code
	ExitProcess(1);
}

// Tell Windows to use our mechanisms.
extern "C"
{
	const PfnDliHook __pfnDliNotifyHook2 = DelayLoadNotifyHook;
	const PfnDliHook __pfnDliFailureHook2 = DelayLoadFailureHook;
}
