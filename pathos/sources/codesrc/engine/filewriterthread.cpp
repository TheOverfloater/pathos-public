/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "filewriterthread.h"
#include "file.h"
#include "system.h"

// Data object for file writer thread
writerthread_t g_fileThreadData;

// File writer thread function
extern DWORD WINAPI FileWriterThread( LPVOID lpParam );

//=============================================
// @brief
//
//=============================================
void FWT_Init( void )
{
	InitializeCriticalSection(&g_fileThreadData.criticalsection);
	InitializeConditionVariable(&g_fileThreadData.condition);

	g_fileThreadData.exitevent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if(!g_fileThreadData.exitevent)
	{
		Con_Printf("Failed to initialize file writer thread.\n");
		return;
	}

	DWORD threadId;
	g_fileThreadData.threadhandle = CreateThread(nullptr, 0, FileWriterThread, &g_fileThreadData, 0, &threadId);

	// Mark file writer as available
	g_fileThreadData.available = true;
}

//=============================================
// @brief
//
//=============================================
void FWT_Shutdown( void )
{
	if(!g_fileThreadData.available)
		return;

	SetEvent(g_fileThreadData.exitevent);
	WakeConditionVariable(&g_fileThreadData.condition);

	WaitForSingleObject(g_fileThreadData.threadhandle, INFINITE);

	CloseHandle(g_fileThreadData.threadhandle);
	CloseHandle(g_fileThreadData.exitevent);

	DeleteCriticalSection(&g_fileThreadData.criticalsection);
}

//=============================================
// @brief
//
//=============================================
bool FWT_AddFile( const Char* pstrFilename, const byte* pData, Uint32 dataSize, bool incremental, bool prompt, bool append )
{
	if(!g_fileThreadData.available)
		return false;

	if(incremental && CString(pstrFilename).find(0, "%number%") == -1)
	{
		Con_Printf("%s - Incremental file's path '%s' missing '%number%' token.\n", __FUNCTION__, pstrFilename);
		return false;
	}

	threadfile_t* pnew = new threadfile_t();

	// Copy data
	pnew->pdata = new byte[dataSize];
	memcpy(pnew->pdata, pData, sizeof(byte)*dataSize);

	pnew->datasize = dataSize;
	pnew->incremental = incremental;
	pnew->prompt = prompt;
	pnew->append = append;
	pnew->filename = pstrFilename;

	// Add to list
	EnterCriticalSection(&g_fileThreadData.criticalsection);
	g_fileThreadData.fileslist.radd(pnew);
	LeaveCriticalSection(&g_fileThreadData.criticalsection);

	// Wake writer thread
	WakeConditionVariable(&g_fileThreadData.condition);

	return true;
}

//=============================================
// @brief
//
//=============================================
DWORD WINAPI FileWriterThread( LPVOID lpParam )
{
	writerthread_t* pThreadData = (writerthread_t*)lpParam;

	while(!pThreadData->exit)
	{
		if(WaitForSingleObject(pThreadData->exitevent, 0) == WAIT_OBJECT_0)
		{
			ExitThread(0);
			return 0;
		}

		// Enter critical section
		EnterCriticalSection(&pThreadData->criticalsection);

		if(!pThreadData->fileslist.empty())
		{
			pThreadData->fileslist.begin();
			while(!pThreadData->fileslist.end())
			{
				// Get file pointer and remove from list
				threadfile_t* pfile = pThreadData->fileslist.get();
				pThreadData->fileslist.remove(pThreadData->fileslist.get_link());

				CString filename;
				if(!pfile->incremental)
				{
					// Just use supplied filename
					filename = pfile->filename;
				}
				else
				{
					Uint32 i = 0;
					while(true)
					{
						// Build new name
						filename = pfile->filename;
						
						// Get position of token
						Int32 pos = filename.find(0, "%number%");
						if(pos == -1)
						{
							filename.clear();
							FWT_Con_Printf(pThreadData, "%s - Filename '%s' missing '%number%' token, file not writen.\n");
							break;
						}
						else
						{
							CString numstr;
							numstr << i;

							filename.erase(pos, 8);
							filename.insert(pos, numstr.c_str());

							if(!FL_FileExists(filename.c_str()))
								break;

							i++;
						}
					}
				}

				if(!filename.empty())
				{
					// Write file to disk
					if(!FL_WriteFile(pfile->pdata, pfile->datasize, filename.c_str(), pfile->append))
						FWT_Con_Printf(pThreadData, "%s - Failed to write file '%s'.\n", __FUNCTION__, filename.c_str());
					else if(pfile->prompt)
						FWT_Con_Printf(pThreadData, "Wrote '%s'.\n", filename.c_str());
				}

				// Delete file object
				delete pfile;

				// Move onto next file
				pThreadData->fileslist.next();
			}
		}

		LeaveCriticalSection(&pThreadData->criticalsection);

		// Wait until next opportunity
		if(!SleepConditionVariableCS(&pThreadData->condition, &pThreadData->criticalsection, INFINITE))
			FWT_Con_Printf(pThreadData, "File writer thread couldn't sleep.\n");
	}

	return 0;
}

//=============================================
// @brief Prints a formatted string to the console buffer for thread
//
// @param pThreadData Thread data pointer
// @param fmt String describing the format
// @param ... Additional format input parameters
//=============================================
void FWT_Con_Printf( writerthread_t* pThreadData, const Char *fmt, ... )
{
	va_list	vArgPtr;
	Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	// Enter critical section
	EnterCriticalSection(&pThreadData->criticalsection);
	pThreadData->consoleprints.radd(cMsg);
	LeaveCriticalSection(&pThreadData->criticalsection);
}

//=============================================
// @brief Returns the elements to print to console in an array
//
// @param fmt String describing the format
// @param ... Additional format input parameters
//=============================================
void FWT_GetConsolePrints( CArray<CString>& destArray )
{
	if(!g_fileThreadData.available)
		return;

	// Enter critical section
	EnterCriticalSection(&g_fileThreadData.criticalsection);

	if(g_fileThreadData.consoleprints.empty())
	{
		LeaveCriticalSection(&g_fileThreadData.criticalsection);
		return;
	}

	destArray.reserve(g_fileThreadData.consoleprints.size());

	// Add elements from linked list to output array
	g_fileThreadData.consoleprints.begin();
	while(!g_fileThreadData.consoleprints.end())
	{
		destArray.push_back(g_fileThreadData.consoleprints.get());
		g_fileThreadData.consoleprints.next();
	}
	g_fileThreadData.consoleprints.clear();

	// Leave critical section
	LeaveCriticalSection(&g_fileThreadData.criticalsection);
}