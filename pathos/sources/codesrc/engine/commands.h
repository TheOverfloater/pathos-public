/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COMMANDS_H
#define COMMANDS_H

#include "cmd_shared.h"

// Maximum character length of a command
#define MAX_COMMAND_LENGTH	128
// Allocation size of the command buffer
#define CMDBUF_ALLOC_SIZE	16384
// Maximum arguments for a command
#define MAX_CMD_ARGS		16

enum cmd_flags_t
{
	CMD_FL_NONE					= 0,
	CMD_FL_SERVERCOMMAND		= (1<<0), // Command can only be executed by SV_ProcessStringCommand
	CMD_FL_CLIENTCOMMAND		= (1<<1), // Command only affects local client
	CMD_FL_CL_RELEVANT			= (1<<2), // This is command that sets a player's state on the server
	CMD_FL_HIDDEN				= (1<<3), // Doesn't show up in the console by tabbing, or in cmdlist
	CMD_FL_HOST_ONLY			= (1<<4), // Can only be invoked by the host player
	CMD_FL_CHEAT				= (1<<5)
};

/*
=======================
CCommandManager

=======================
*/
class CCommandManager
{
public:
	struct cmd_definition_t
	{
		cmd_definition_t():
			flags(CMD_FL_NONE),
			pfn(nullptr)
		{}

		CString name;
		CString description;

		Uint64 flags;

		cmdfunc_t pfn;
	};

public:
	CCommandManager( void );
	CCommandManager( const CCommandManager& src );
	~CCommandManager( void );

private:
	CCommandManager& operator=( const CCommandManager& src );

public:
	// Initializes the class
	void Init( void );
	// Clears the command buffer
	void ClearCommands( void );

public:
	// Creates a new command definition
	void CreateCommand( const Char* name, cmdfunc_t pfn, const Char* description = nullptr, Int32 flags = CMD_FL_NONE );
	// Adds a command to the buffer for execution
	void AddCommand( const Char* pstrCommand );
	// Inserts a command right after the current one
	void InsertCommand( const Char* pstrCommand );

	// Executes all cached commands
	void ExecuteCommands( void );
	// Executes a command
	bool ExecuteCommand( const Char* pstrCommand, bool isServerCall );
	// Caches a command into the argument arrays
	void CacheCommand( const Char* pstrCommand );

	// Tells if a command already exists with the specified name
	bool CommandExists( const Char* pstrName ) const;
	// Retreives flags for a command name
	Int32 GetCommandFlags( const Char* pstrName ) const;
	// Retrieves the invoking player's index
	Int32 GetInvokerPlayerIndex( void ) const;
	// Retrieves the invoking player's index
	void SetInvokerPlayerIndex( Int32 index );

	// Forwards command to server
	bool ForwardToServer( void ) const;

public:
	// Returns the argument count
	Uint32 Cmd_Argc( void ) const { return m_numArgs; }
	// Returns the argument at a given index
	const Char* Cmd_Argv( Uint32 index ) const;

private:
	// Increases the buffer size
	void IncreaseBufferSize( void );

public:
	// Array of command definitions
	CArray<cmd_definition_t> m_cmdDefinitionsArray;
	// Command buffer
	Char* m_pCommandBuffer;
	// Current load of the command buffer
	Uint32 m_cmdBufSize;
	// Current load of the command buffer
	Uint64 m_cmdBufUsageLength;

	// Current command arguments
	CString m_commandArgs[MAX_CMD_ARGS];
	// Number of arguments
	Uint32 m_numArgs;

	// Invoker player index
	Int32 m_invokerPlayerIndex;

	// Tells if we should break command execution
	bool m_breakExecution;
};
extern CCommandManager gCommands;
#endif 