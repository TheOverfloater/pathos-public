/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "sys_print.h"
#include "enginestate.h"
#include "console.h"
#include "r_main.h"
#include "system.h"

// Object definition
CSysPrintInterface gPrintInterface;

// For thread safety for Printf
static std::mutex m_conPrintfMutex;
// For thread safety for DPrintf
static std::mutex m_conDPrintfMutex;
// For thread safety VPrintf
static std::mutex m_conVPrintfMutex;
// For thread safety EPrintf
static std::mutex m_conEPrintfMutex;

// To protect against infinite loops
static bool g_conPrintfSemaphore = false;
// To protect against infinite loops
static bool g_conDPrintfSemaphore = false;
// To protect against infinite loops
static bool g_conVPrintfSemaphore = false;
// To protect against infinite loops
static bool g_conEPrintfSemaphore = false;

//====================================
//
//====================================
CSysPrintInterface::CSysPrintInterface( void )
{
}

//====================================
//
//====================================
CSysPrintInterface::~CSysPrintInterface( void ) 
{
	Clear();
	ClearGame();
}

//====================================
//
//====================================
void CSysPrintInterface::Clear( void )
{
	if(!m_applicationInstancePrintsSet.empty())
		m_applicationInstancePrintsSet.clear();
}

//====================================
//
//====================================
void CSysPrintInterface::ClearGame( void )
{
	if(!m_gameInstancePrintsSet.empty())
		m_gameInstancePrintsSet.clear();
}

//=============================================
// @brief Formats a string and prints it to the console
//
// @param pstring String to be printed
//=============================================
void CSysPrintInterface::Printf( const Char *pstring )
{
	CString msg(pstring);

	Int32 flags = PRINT_FL_NONE;
	CheckForFlags(msg, flags);
	if(!ShouldPrintMessage(msg.c_str(), flags))
		return;

	if(ens.plogfile)
		ens.plogfile->Write(msg.c_str());

	gConsole.AddTextHistory(msg.c_str());

	// Redraw if loading
	if(ens.isloading && rns.basicsinitialized)
		VID_DrawLoadingScreen();

#ifdef _CONSOLE
	printf(msg.c_str());
#endif
}

//=============================================
// @brief Formats a string and prints it to the console in debug mode only
//
// @param pstring String to be printed
//=============================================
void CSysPrintInterface::DPrintf( const Char *pstring )
{
	CString msg;
	msg << "DEBUG: " << pstring;

	Int32 flags = PRINT_FL_NONE;
	CheckForFlags(msg, flags);
	if(!ShouldPrintMessage(msg.c_str(), flags))
		return;

	if(ens.plogfile)
		ens.plogfile->Write(msg.c_str());

	if(g_pCvarDeveloper->GetValue() >= DEV_MODE_ON)
	{
		gConsole.AddTextHistory(msg.c_str());

		// Redraw if loading
		if(ens.isloading && rns.basicsinitialized)
			VID_DrawLoadingScreen();
	}

#ifdef _CONSOLE
	printf(msg.c_str());
#endif
}

//=============================================
// @brief Formats a string and prints it to the console in verbose mode only
//
// @param pstring String to be printed
//=============================================
void CSysPrintInterface::VPrintf( const Char *pstring )
{
	CString msg;
	msg << "VERBOSE: " << pstring;

	Int32 flags = PRINT_FL_NONE;
	CheckForFlags(msg, flags);
	if(!ShouldPrintMessage(msg.c_str(), flags))
		return;

	if(ens.plogfile)
		ens.plogfile->Write(msg.c_str());

	if(g_pCvarDeveloper->GetValue() >= DEV_MODE_ON)
	{
		gConsole.AddTextHistory(msg.c_str());

		// Redraw if loading
		if(ens.isloading && rns.basicsinitialized)
			VID_DrawLoadingScreen();
	}

#ifdef _CONSOLE
	printf(msg.c_str());
#endif
}

//=============================================
// @brief Formats a string and prints it to the console as an error
//
// @param pstring String to be printed
//=============================================
void CSysPrintInterface::EPrintf( const Char *pstring )
{
	CString msg;
	msg << "ERROR: " << pstring;

	Int32 flags = PRINT_FL_NONE;
	CheckForFlags(msg, flags);
	if(!ShouldPrintMessage(msg.c_str(), flags))
		return;

	if(ens.plogfile)
		ens.plogfile->Write(msg.c_str());

	gConsole.AddTextHistory(msg.c_str());

	// Redraw if loading
	if(ens.isloading && rns.basicsinitialized)
		VID_DrawLoadingScreen();

#ifdef _CONSOLE
	printf(msg.c_str());
#endif
}

//====================================
// @brief Processes any flags inside strings
// 
//====================================
void CSysPrintInterface::CheckForFlags( CString& inputstring, Int32& outflags )
{
	// Reset to default
	outflags = PRINT_FL_NONE;

	Int32 offset = 0;
	Uint32 eraseoffset = 0;

	CString output;

	while(offset < inputstring.length())
	{
		// Find next square bracket character
		Int32 bracketpos = inputstring.find(offset, "[");
		if(bracketpos == CString::CSTRING_NO_POSITION)
			break;

		// Find end position
		Int32 endbracketpos = inputstring.find(bracketpos, "]");
		if(endbracketpos == CString::CSTRING_NO_POSITION)
			break;
		
		// Check if it has valid contents
		bool hasValidContents = false;
		const Char* pstr = inputstring.c_str() + bracketpos + 1;
		while(true)
		{
			while((*pstr) && SDL_isspace(*pstr))
				pstr++;

			// End if we reached the bracket
			if((*pstr) == ']')
				break;

			// Manage "flag" token
			if(!qstrncmp(pstr, "flags", 5))
			{
				// Skip ahead to the = token
				pstr += 5;
				if((*pstr) == '=')
				{
					// Mark as having valid contents
					hasValidContents = true;
					pstr++;

					while(true)
					{
						// Skip to whitespace or end bracket
						while(SDL_isspace(*pstr) && (*pstr))
							pstr++;

						// End if we reached the bracket
						if((*pstr) == ']')
							break;

						// Skip till whitespace or end
						Uint32 i = 0;
						while(!SDL_isspace(*(pstr+i)) && (*(pstr+i)) != ']')
							i++;

						CString flag(pstr, i);
						if(!qstrcmp(flag, "onlyonce_instance"))
						{
							// Only once per instance
							outflags |= PRINT_FL_ONLYONCE;
						}
						else if(!qstrcmp(flag, "onlyonce_game"))
						{
							// Only once per game session
							outflags |= PRINT_FL_ONLYONCE_GAME;
						}

						// Skip to the whitespace/end bracket
						pstr += i;
					}
				}
				else
				{
					// Skip to whitespace or end bracket
					while(!SDL_isspace(*pstr) && (*pstr) != ']')
						pstr++;
				}
			}
			else
			{
				// Skip to whitespace or end bracket
				while(!SDL_isspace(*pstr) && (*pstr) != ']')
					pstr++;
			}
		}

		// If contents were valid, remove them from the output
		if(hasValidContents)
		{
			// Fill the output if needed
			if(output.empty())
				output = inputstring;

			Uint32 removecnt = (endbracketpos - bracketpos) + 1;
			output.erase(bracketpos-eraseoffset, removecnt);
			eraseoffset += removecnt;
		}
		
		// Move onto the next offset
		offset = endbracketpos;
	}

	// Set output string
	if(!output.empty())
		inputstring = output;
}

//====================================
// @brief Tells if the message should be printed
//
//====================================
bool CSysPrintInterface::ShouldPrintMessage( const Char* pstrMessage, Int32 flags )
{
	if(flags & (PRINT_FL_ONLYONCE|PRINT_FL_ONLYONCE_GAME))
	{
		m_md5Hasher.Init();
		m_md5Hasher.Update(reinterpret_cast<const byte*>(pstrMessage), qstrlen(pstrMessage));
	
		CString md5hash(CString::fl_str_nopooling);
		md5hash = m_md5Hasher.Finalize().HexDigest();
		
		if(flags & PRINT_FL_ONLYONCE_GAME)
		{
			std::pair<std::set<CString>::iterator, bool> it = m_gameInstancePrintsSet.insert(md5hash);
			if(!it.second)
				return false;
		}
		else if(flags & PRINT_FL_ONLYONCE)
		{
			std::pair<std::set<CString>::iterator, bool> it = m_applicationInstancePrintsSet.insert(md5hash);
			if(!it.second)
				return false;
		}
	}

	return true;
}

//====================================
//
//====================================
void Con_Printf( const Char *fmt, ... )
{
	if(g_conPrintfSemaphore)
		return;

	g_conPrintfSemaphore = true;
	m_conPrintfMutex.lock();

	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	// Send to print interface to complete
	gPrintInterface.Printf(cMsg);

	g_conPrintfSemaphore = false;
	m_conPrintfMutex.unlock();
}

//====================================
//
//====================================
void Con_DPrintf( const Char *fmt, ... )
{
	if(g_pCvarDeveloper->GetValue() < DEV_MODE_ON && !ens.plogfile)
		return;

	if(g_conDPrintfSemaphore)
		return;

	g_conDPrintfSemaphore = true;
	m_conDPrintfMutex.lock();

	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	// Send to print interface to complete
	gPrintInterface.DPrintf(cMsg);

	g_conDPrintfSemaphore = false;
	m_conDPrintfMutex.unlock();
}

//====================================
//
//====================================
void Con_VPrintf( const Char *fmt, ... )
{
	if(g_pCvarDeveloper->GetValue() < DEV_MODE_VERBOSE)
		return;

	if(g_conVPrintfSemaphore)
		return;

	g_conVPrintfSemaphore = true;
	m_conVPrintfMutex.lock();

	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	// Send to print interface to complete
	gPrintInterface.VPrintf(cMsg);

	g_conVPrintfSemaphore = false;
	m_conDPrintfMutex.unlock();
}

//====================================
//
//====================================
void Con_EPrintf( const Char *fmt, ... )
{
	if(g_conEPrintfSemaphore)
		return;

	g_conEPrintfSemaphore = true;
	m_conEPrintfMutex.lock();

	// compile the string result
	va_list	vArgPtr;
	static Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	// Send to print interface to complete
	gPrintInterface.EPrintf(cMsg);

	g_conEPrintfSemaphore = false;
	m_conEPrintfMutex.unlock();
}