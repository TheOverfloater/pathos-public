/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FILEWRITERTHREAD_H
#define FILEWRITERTHREAD_H

extern void FWT_Init( void );
extern void FWT_Shutdown( void );
extern bool FWT_AddFile( const Char* pstrFilename, const byte* pData, Uint32 dataSize, bool incremental = false, bool prompt = false, bool append = false );
extern void FWT_Con_Printf( struct writerthread_t* pThreadData, const Char *fmt, ... );
extern void FWT_GetConsolePrints( CArray<CString>& destArray );

struct threadfile_t
{
	threadfile_t():
		pdata(nullptr),
		datasize(0),
		incremental(false),
		prompt(false),
		append(false)
		{}
	~threadfile_t()
	{
		if(pdata)
			delete[] pdata;
	}

	CString filename;
	byte* pdata;
	Uint32 datasize;
	bool incremental;
	bool prompt;
	bool append;
};

struct writerthread_t
{
	writerthread_t():
		threadhandle(nullptr),
		exitevent(nullptr),
		exit(false),
		available(false)
		{}

	HANDLE threadhandle;
	HANDLE exitevent;
	CRITICAL_SECTION criticalsection;
	CONDITION_VARIABLE condition;

	bool exit;
	bool available;

	// List of files to write
	CLinkedList<threadfile_t*> fileslist;

	// Cached up console prints
	CLinkedList<CString> consoleprints;
};

#endif //FILEWRITERTHREAD_H