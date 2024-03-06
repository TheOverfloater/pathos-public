/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>

#include "includes.h"
#include "commands.h"
#include "common.h"
#include "file.h"
#include "console.h"
#include "system.h"
#include "cl_main.h"
#include "networking.h"
#include "sv_main.h"
#include "enginestate.h"
#include "sv_msg.h"

// Class definition
CCommandManager gCommands;

//=============================================
// @brief Default constructor
//
//=============================================
CCommandManager::CCommandManager( void ):
	m_pCommandBuffer(nullptr),
	m_cmdBufSize(CMDBUF_ALLOC_SIZE),
	m_cmdBufUsageLength(0),
	m_numArgs(0),
	m_invokerPlayerIndex(-1),
	m_breakExecution(false)
{
	m_pCommandBuffer = new Char[m_cmdBufSize];
	m_pCommandBuffer[0] = '\0';
}

//=============================================
// @brief Default constructor
//
//=============================================
CCommandManager::CCommandManager( const CCommandManager& src ):
	m_cmdDefinitionsArray(src.m_cmdDefinitionsArray),
	m_pCommandBuffer(nullptr),
	m_cmdBufSize(src.m_cmdBufSize),
	m_cmdBufUsageLength(src.m_cmdBufUsageLength),
	m_numArgs(src.m_numArgs),
	m_invokerPlayerIndex(src.m_invokerPlayerIndex),
	m_breakExecution(src.m_breakExecution)
{
	for(Uint32 i = 0; i < m_numArgs; i++)
		m_commandArgs[i] = src.m_commandArgs[i];

	m_pCommandBuffer = new Char[m_cmdBufSize];
	memcpy(m_pCommandBuffer, src.m_pCommandBuffer, sizeof(byte)*m_cmdBufSize);
}

//=============================================
// @brief Destructor
//
//=============================================
CCommandManager::~CCommandManager( void )
{
	if(m_pCommandBuffer)
	{
		delete m_pCommandBuffer;
		m_pCommandBuffer = nullptr;
	}
}

//=============================================
// @brief Default constructor
//
//=============================================
CCommandManager& CCommandManager::operator= ( const CCommandManager& src )
{
	if(&src == this)
		return *this;

	m_cmdDefinitionsArray = src.m_cmdDefinitionsArray;
	m_cmdBufSize = src.m_cmdBufSize;
	m_cmdBufUsageLength = src.m_cmdBufUsageLength;
	m_numArgs = src.m_numArgs;
	m_invokerPlayerIndex = src.m_invokerPlayerIndex;
	m_breakExecution = src.m_breakExecution;

	for(Uint32 i = 0; i < m_numArgs; i++)
		m_commandArgs[i] = src.m_commandArgs[i];

	if(m_pCommandBuffer)
	{
		delete[] m_pCommandBuffer;
		m_pCommandBuffer = nullptr;
	}

	m_pCommandBuffer = new Char[m_cmdBufSize];
	memcpy(m_pCommandBuffer, src.m_pCommandBuffer, sizeof(byte)*m_cmdBufSize);
	return *this;
}

//=============================================
// @brief Intializes the class
//
//=============================================
void CCommandManager::Init( void )
{
}

//=============================================
// @brief Clears the command buffer
//
//=============================================
void CCommandManager::ClearCommands( void )
{
	m_pCommandBuffer[0] = '\0';
	m_cmdBufUsageLength = 0;
}

//=============================================
// @brief Creates a new command definition
//
// @param name Command name
// @param pfn Ponter to function to execute
// @param description Optional description for the command
// @param flags Flags of the command
//=============================================
void CCommandManager::CreateCommand( const Char* name, cmdfunc_t pfn, const Char* description, Int32 flags )
{
	// Make sure it's not already added
	if(CommandExists(name))
	{
		Con_Printf("Error: Command %s already declared.\n", name);
		return;
	}

	// Add it to the array
	cmd_definition_t newCmd;
	newCmd.name = name;
	newCmd.pfn = pfn;
	newCmd.flags = flags;

	m_cmdDefinitionsArray.push_back(newCmd);

	// Add to the console list
	if(!(flags & CMD_FL_HIDDEN))
		gConsole.AddInput(INPUT_CMD, name, description);
}

//=============================================
// @brief Increases the size of the command buffer on demand
//
//=============================================
void CCommandManager::IncreaseBufferSize( void )
{
	void* pnew = Common::ResizeArray(m_pCommandBuffer, sizeof(Char), m_cmdBufUsageLength, CMDBUF_ALLOC_SIZE);
	m_pCommandBuffer = reinterpret_cast<Char*>(pnew);

	Con_DPrintf("Increasing command buffer size by %d bytes.\n", (Int32)CMDBUF_ALLOC_SIZE);
	m_cmdBufSize += CMDBUF_ALLOC_SIZE;
}

//=============================================
// @brief Adds a command to be executed
//
// @param pstrCommand Full command string
//=============================================
void CCommandManager::AddCommand( const Char* pstrCommand )
{
	Uint32 cmdLength = qstrlen(pstrCommand);

	// See if we need to resize the buffer
	if(m_cmdBufSize < (m_cmdBufUsageLength+cmdLength+2))
		IncreaseBufferSize();

	// Add the new command
	Char *pdest = m_pCommandBuffer + m_cmdBufUsageLength;
	memcpy(pdest, pstrCommand, cmdLength);
	
	// Add a newline and a terminator
	pdest += cmdLength;
	*pdest++ = '\n';
	*pdest++ = '\0';

	m_cmdBufUsageLength += cmdLength+1;
}

//=============================================
// @brief Inserts a command right after the current one
//
// @param pstrCommand Full command string
//=============================================
void CCommandManager::InsertCommand( const Char* pstrCommand )
{
	Char *ptemp = nullptr;
	Uint64 tmpsize = 0;

	// Move current buffer into temp array
	if(m_cmdBufUsageLength)
	{
		ptemp = new Char[m_cmdBufUsageLength];
		tmpsize = m_cmdBufUsageLength;

		memcpy(ptemp, m_pCommandBuffer, sizeof(Char)*tmpsize);

		m_pCommandBuffer[0] = '\0';
		m_cmdBufUsageLength = 0;
	}

	// Add the command
	AddCommand(pstrCommand);

	// Add the original contents
	if(ptemp)
	{
		// Increase buffer size if needed
		if((m_cmdBufUsageLength+tmpsize) > m_cmdBufSize)
			IncreaseBufferSize();

		// Copy over the contents
		Char* pdest = m_pCommandBuffer + m_cmdBufUsageLength;
		memcpy(pdest, ptemp, sizeof(Char)*tmpsize);
		delete[] ptemp;
	}
}

//=============================================
// @brief Executes all the cached commands
//
//=============================================
void CCommandManager::ExecuteCommands( void )
{
	if(!m_cmdBufUsageLength)
		return;

	// Set invoker to local
	m_invokerPlayerIndex = 0;

	Uint32 cmdLength = 0;
	CString szbuffer;
	while(m_cmdBufUsageLength)
	{
		// Parse a single command from the buffer
		bool inQuote = false;

		Char* pstrBegin = m_pCommandBuffer;
		Char* pstr = pstrBegin;
		while(pstr)
		{
			if(*pstr == '\"')
			{
				if(inQuote)
					inQuote = false;
				else
					inQuote = true;
			}

			// newline always breaks
			if(!inQuote && *pstr == ';' || *pstr == '\n')
			{
				pstr++;
				break;
			}

			cmdLength++;
			pstr++;
		}

		szbuffer.assign(pstrBegin, cmdLength);
		cmdLength = 0;

		// Skip any spaces before continuing
		while(pstr && SDL_isspace(*pstr) && *pstr)
			pstr++;

		Uint64 bufPos = (pstr - m_pCommandBuffer);
		if(bufPos == m_cmdBufUsageLength)
		{
			// No more commands to execute
			m_cmdBufUsageLength = 0;
			m_pCommandBuffer[0] = '\0';
		}
		else
		{
			m_cmdBufUsageLength -= bufPos;
			
			// Move the contents
			Char* pdst = m_pCommandBuffer;
			Char* psrc = pstr;
			while(*psrc)
			{
				*pdst = *psrc;
				psrc++; pdst++;
			}

			*pdst = '\0';
		}

		// Execute this command
		ExecuteCommand(szbuffer.c_str(), false);

		// See if we need to skip to the next frame
		if(m_breakExecution)
		{
			m_breakExecution = false;
			break;
		}
	}
}

//=============================================
// @brief Executes a single command
//
//=============================================
void CCommandManager::CacheCommand( const Char* pstrCommand )
{
	// Clear previous args
	for(Uint32 i = 0; i < m_numArgs; i++)
		m_commandArgs[i].clear();

	// Reset this
	m_numArgs = 0;

	// Tokenize the string
	CString argument;
	const Char* pstr = pstrCommand;
	while(pstr)
	{
		if(m_numArgs == MAX_CMD_ARGS)
		{
			Con_EPrintf("Too many arguments for command.\n");
			break;
		}

		pstr = Common::Parse(pstr, argument, '\0', true);

		// Add new arg
		m_commandArgs[m_numArgs] = argument;
		m_numArgs++;

		// handle 'say' specially
		if(!qstrcmp(argument, "say") || !qstrcmp(argument, "say_team"))
		{
			m_commandArgs[m_numArgs].assign(pstr, qstrlen(pstr));
			m_numArgs++;
			break;
		}
	}
}

//=============================================
// @brief Executes a single command
//
//=============================================
bool CCommandManager::ExecuteCommand( const Char* pstrCommand, bool isServerCall )
{
	// Process it into the arrays
	CacheCommand(pstrCommand);

	if(!m_numArgs)
		return false;

	// Seek out the command
	cmd_definition_t* pcmd = nullptr;
	for(Uint32 i = 0; i < m_cmdDefinitionsArray.size(); i++)
	{
		if(!qstrcmp(m_cmdDefinitionsArray[i].name, m_commandArgs[0]))
		{
			pcmd = &m_cmdDefinitionsArray[i];
			break;
		}
	}

	// Try and process the command
	if(pcmd)
	{
		// Manage server-only commands
		if(pcmd->flags & (CMD_FL_SERVERCOMMAND|CMD_FL_CL_RELEVANT))
		{
			// Only run these when the game is active
			if(ens.gamestate != GAME_RUNNING)
			{
				Con_Printf("Cannot execute '%s' - no game session running.\n", pcmd->name.c_str());
				return true;
			}

			if(!CL_IsHostClient())
			{
				// Forward to SV_ProcessStringCommand
				ForwardToServer();
				return true;
			}
			else if((pcmd->flags & CMD_FL_HOST_ONLY) && !SV_IsHostClient(m_invokerPlayerIndex))
			{
				if(m_invokerPlayerIndex < 0 || m_invokerPlayerIndex >= (Int32)svs.maxclients)
				{
					Con_Printf("Invalid invoker client %d.\n", m_invokerPlayerIndex);
					return true;
				}

				sv_client_t* pinvoker = &svs.clients[m_invokerPlayerIndex];
				SV_ClientPrintf(pinvoker->pedict, "The command '%s' can only be invoked by the host.\n", pcmd->name.c_str());
				return true;
			}
			else if((pcmd->flags & CMD_FL_CHEAT) && !svs.dllfuncs.pfnAreCheatsEnabled())
			{
				sv_client_t* pinvoker = &svs.clients[m_invokerPlayerIndex];
				SV_ClientPrintf(pinvoker->pedict, "The command '%s' is a cheat command and can only be called with sv_cheats set to 1.\n", pcmd->name.c_str());
				return true;
			}
		}
		else if(pcmd->flags & CMD_FL_CLIENTCOMMAND)
		{
			// Do not let the server run client commands
			if(isServerCall)
			{
				Con_Printf("The command '%s' is a client command and cannot be run on the server.\n", pcmd->name.c_str());
				return true;
			}
		}

		// Call the command to execute
		pcmd->pfn();
		return true;
	}

	// Check cvars via console
	if(gConsole.ProcessCommand())
		return true;

	// Send it to the server
	if(isServerCall)
	{
		// If it was not found, send it for execution by the player code
		return false;
	}
	else if(ens.gamestate == GAME_RUNNING)
	{
		// Just send to the server
		ForwardToServer();
		return true;
	}
	else
	{
		Con_Printf("Unknown command: '%s'.\n", m_commandArgs[0].c_str());
		return false;
	}
}

//=============================================
// @brief Executes a single command
//
//=============================================
bool CCommandManager::ForwardToServer( void ) const
{
	if(!CL_IsGameActive())
		return false;

	// Send the command to the server
	CNetworking* pNet = CNetworking::GetInstance();
	if(pNet->GetClientState(0) != NETCL_CONNECTED)
		return false;
	
	// Do not include "cmd" part
	Uint32 i;
	if(!qstrcmp(m_commandArgs[0], "cmd"))
		i = 1;
	else
		i = 0;

	CString cmd;
	for(; i < m_numArgs; i++)
	{
		if(i > 0)
			cmd << ' ';

		cmd << m_commandArgs[i];
	}

	// Send it through
	pNet->CLS_MessageBegin(cls_cmdstring);
		pNet->WriteString(cmd.c_str());
	pNet->CLS_MessageEnd();

	return true;
}

//=============================================
// @brief Returns an argument at an index
//
// @param index Index of the argument to retrieve
// @return Command argument string 
//=============================================
const Char* CCommandManager::Cmd_Argv( Uint32 index ) const
{
	if( index >= m_numArgs )
		return "";

	return m_commandArgs[index].c_str(); 
}

//=============================================
// @brief Tells if a command already exists with the specified name
//
// @param index Index of the argument to retrieve
// @return TRUE if it was found, FALSE otherwise 
//=============================================
bool CCommandManager::CommandExists( const Char* pstrName ) const
{
	for(Uint32 i = 0; i < m_cmdDefinitionsArray.size(); i++)
	{
		if(!qstrcmp(m_cmdDefinitionsArray[i].name.c_str(), pstrName))
			return true;
	}

	return false;
}

//=============================================
// @brief Tells if a command already exists with the specified name
//
// @param index Index of the argument to retrieve
// @return TRUE if it was found, FALSE otherwise 
//=============================================
Int32 CCommandManager::GetCommandFlags( const Char* pstrName ) const
{
	for(Uint32 i = 0; i < m_cmdDefinitionsArray.size(); i++)
	{
		if(!qstrcmp(m_cmdDefinitionsArray[i].name.c_str(), pstrName))
			return m_cmdDefinitionsArray[i].flags;
	}

	return CMD_FL_NONE;
}

//=============================================
// @brief Tells if a command already exists with the specified name
//
// @param index Index of the argument to retrieve
// @return TRUE if it was found, FALSE otherwise 
//=============================================
void CCommandManager::SetInvokerPlayerIndex( Int32 index )
{
	m_invokerPlayerIndex = index;
}

//=============================================
// @brief Tells if a command already exists with the specified name
//
// @param index Index of the argument to retrieve
// @return TRUE if it was found, FALSE otherwise 
//=============================================
Int32 CCommandManager::GetInvokerPlayerIndex( void ) const
{
	return m_invokerPlayerIndex;
}