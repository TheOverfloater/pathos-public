/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>

#include "includes.h"
#include "common.h"
#include "file_interface.h"
#include "logfile.h"
#include "constants.h"

// Buffer size
const Uint32 CLogFile::LOG_BUFFER_SIZE = 16384;
// Size of buffer for message prints
const Uint32 CLogFile::PRINT_MSG_BUFFER_SIZE = 16384;

//=============================================
// @brief Constructor
//
// @param pstrPath Log file path
//=============================================
CLogFile::CLogFile( const Char* pstrPath, pfnConPrintf_t pfnConPrintf, file_interface_t& fileInterface, bool deletePrevious, bool timeStamps ):
	m_sLogPath(pstrPath),
	m_nbLinesWritten(0),
	m_useTimeStamps(timeStamps),
	m_deletePreviousLog(deletePrevious),
	m_pBuffer(nullptr),
	m_logBufferLoad(0),
	m_writeSemaphore(false),
	m_pfnConPrintf(pfnConPrintf),
	m_fileInterface(fileInterface)
{
	m_pBuffer = new Char[LOG_BUFFER_SIZE];
}

//=============================================
// @brief Destructor
//
//=============================================
CLogFile::~CLogFile()
{
	if(m_pBuffer)
		delete[] m_pBuffer;
}

//=============================================
// @brief Checks the file for writing and appends the start header
//
// @param pstrString String to write
//=============================================
bool CLogFile::Init()
{
	if(m_deletePreviousLog 
		&& m_fileInterface.pfnFileExists(m_sLogPath.c_str()))
	{
		if(!m_fileInterface.pfnDeleteFile(m_sLogPath.c_str()))
			m_pfnConPrintf("Failed to delete previous log file '%s'.\n", m_sLogPath.c_str());
		else
			m_pfnConPrintf("Deleted previous log file '%s'.\n", m_sLogPath.c_str());
	}

	// Clean
	m_logBufferLoad = 0;

	CString strOut;
	strOut << "Log started: " << Common::GetDate() << NEWLINE;
	if(!Write(strOut.c_str()))
		return false;

	CString sPath;
	if(!qstrstr(m_sLogPath.c_str(), ":"))
		sPath = SDL_GetBasePath() + m_sLogPath;
	else
		sPath = m_sLogPath;

	strOut.clear();
	strOut << "Log file: " << sPath.c_str() << NEWLINE;

	// Attempt to write to the file
	return Write(strOut.c_str());
}

//=============================================
// @brief Checks the file for writing and appends the start header
//
// @param pstrString String to write
//=============================================
bool CLogFile::Close()
{
	bool result = true;
	if(m_logBufferLoad > 0)
	{
		// Write to file
		const byte* pdata = reinterpret_cast<const byte*>(m_pBuffer);
		result = m_fileInterface.pfnWriteLogFile(pdata, m_logBufferLoad, m_sLogPath.c_str(), true);
		if(!result)
		{
			// DO NOT call CON_EPrintf here!
			m_pfnConPrintf("Failed to write to log file %s.\n", m_sLogPath.c_str());
		}

		m_logBufferLoad = 0;
	}

	return result;
}

//=============================================
// @brief Write a line to the log file
//
// @param pstrString String to write
//=============================================
bool CLogFile::Write( const Char* pstrString )
{
	if(m_writeSemaphore)
		return false;

	m_writeSemaphore = true;

	bool result = true;

	if(m_useTimeStamps)
	{
		CString strOut;
		CString dateStr = Common::GetDate();

		Uint32 newlineCount = 0;
		bool isMultiLine = false;

		const Char* pstr = pstrString;
		while(*pstr)
		{
			if(*pstr == '\n' || *pstr == '\r')
			{
				if(newlineCount)
				{
					isMultiLine = true;
					break;
				}

				if((pstr[0] == '\n' || pstr[0] == '\r')
					&& (pstr[1] == '\n' || pstr[1] == '\r'))
					pstr += 2;
				else
					pstr++;

				newlineCount++;
			}
			else
			{
				pstr++;
			}
		}

		if(!isMultiLine)
		{
			// Only a single line
			strOut << dateStr << " - " << pstrString;
			if(!qstrlen(pstrString))
				strOut << NEWLINE;

			result = WriteInternal(strOut.c_str());
		}
		else
		{
			Int32 begin = 0;
			pstr = pstrString;
			while(*pstr && result)
			{
				if(*pstr == '\n' || *pstr == '\r')
				{
					const Char* pstrbegin = pstrString+begin;
					Uint32 count = (pstr - pstrbegin);
					if(count)
					{
						strOut.assign(pstrbegin, count);
						strOut.insert(0, dateStr.c_str());
						strOut.insert(dateStr.length(), " - ");
						strOut << NEWLINE;

						result = WriteInternal(strOut.c_str());
						strOut.clear();
					}
					else
					{
						// Empty line
						strOut << dateStr << NEWLINE;
						result = WriteInternal(strOut.c_str());
					}

					if((pstr[0] == '\n' || pstr[0] == '\r')
						&& (pstr[1] == '\n' || pstr[1] == '\r'))
						pstr += 2;
					else
						pstr++;

					begin = pstr - pstrString;
				}
				else
				{
					// Go to next character
					pstr++;
				}
			}

			if(pstr - (pstrString+begin) > 0)
			{
				const Char* pstrbegin = pstrString+begin;
				Uint32 count = (pstr - pstrbegin) - 1;

				strOut.assign(pstrbegin, count);
				strOut.insert(0, dateStr.c_str());

				result = WriteInternal(strOut.c_str());
				strOut.clear();
			}
		}
	}
	else
	{
		// Do not worry about timestamps
		result = WriteInternal(pstrString);
	}

	m_writeSemaphore = false;

	return result;
}

//=============================================
// @brief Write a line to the log file
//
// @param pstrString String to write
//=============================================
bool CLogFile::WriteInternal( const Char* pstrString )
{
	// Start from beginning
	Uint32 remainsize = qstrlen(pstrString);
	const Char* psrc = pstrString;

	bool result = true;
	while(remainsize > 0 && result)
	{
		Char* pdest = m_pBuffer + m_logBufferLoad;

		Uint32 sizetowrite;
		if(m_logBufferLoad + remainsize >= LOG_BUFFER_SIZE)
			sizetowrite = LOG_BUFFER_SIZE - m_logBufferLoad;
		else
			sizetowrite = remainsize;

		memcpy(pdest, psrc, sizeof(Char)*sizetowrite);
		remainsize -= sizetowrite;
		m_logBufferLoad += sizetowrite;
		psrc += sizetowrite;

		if(m_logBufferLoad == LOG_BUFFER_SIZE)
		{
			// Write to file
			const byte* pdata = reinterpret_cast<const byte*>(m_pBuffer);
			result = m_fileInterface.pfnWriteLogFile(pdata, m_logBufferLoad, m_sLogPath.c_str(), true);
			if(!result)
			{
				// DO NOT call CON_EPrintf here!
				m_pfnConPrintf("Failed to write to log file %s.\n", m_sLogPath.c_str());
				break;
			}

			// Clean buffer
			m_logBufferLoad = 0;
		}
	}

	return result;
}

//=============================================
// @brief Write a formatted string to the log file
//
// @param pstrString String to write
//=============================================
bool CLogFile::Printf( const Char *fmt, ... )
{
	va_list	vArgPtr;
	Char cMsg[PRINT_MSG_BUFFER_SIZE];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	return Write(cMsg);
}