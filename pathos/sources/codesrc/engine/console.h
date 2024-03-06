/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "cvar.h"

enum input_type_t
{
	INPUT_CVAR,
	INPUT_CMD
};

struct con_input_t
{
	CString name;
	CString description;
	input_type_t type;
};

// Datatype for the input list
typedef CLinkedList<con_input_t> ConList_t;

/*
=================================
CUIConsoleExecuteCallback

=================================
*/
class CConsole
{
public:
	// How much to increase the console's buffer by each time
	static const Uint32 CONSOLE_BUFFER_CHUNKSIZE;
	// Command name for the toggleconsole command
	static const Char TOGGLECONSOLE_CMD_NAME[];
	// Max remembered last prints
	static const Uint32 MAX_DEBUG_PRINTS;
	// Print lifetime
	static const Float DEBUG_PRINT_LIFETIME;

	// Print box reference offset for x axis
	static const Uint32 PRINT_BOX_REF_X_OFFSET;
	// Print box reference offset for y axis
	static const Uint32 PRINT_BOX_REF_Y_OFFSET;
	// Print box text inset
	static const Uint32 PRINT_BOX_INSET;

	// Console reference width
	static const Uint32 CONSOLE_REF_RESOLUTION_WIDTH;
	// Print box reference height
	static const Uint32 CONSOLE_REF_RESOLUTION_HEIGHT;

	// Print box reference width
	static const Uint32 PRINT_BOX_REF_WIDTH;
	// Print box reference height
	static const Uint32 PRINT_BOX_REF_HEIGHT;
	// Print box reference bar thickness
	static const Uint32 PRINT_BOX_REF_BAR_THICKNESS;

	// Print box bar color
	static const color32_t PRINT_BOX_BAR_COLOR;
	// Print box background color
	static const color32_t PRINT_BOX_BACKGROUND_COLOR;

public:
	// Debug display prints
	struct debug_print_t
	{
		CString text;
		Double time;
	};

public:
	CConsole( void );
	CConsole( CConsole& src );
	~CConsole( void );

private:
	// Assignment operator
	CConsole& operator=( CConsole& src );

public:
	// Initializes the console
	void Init( void );

	// Adds text to the history
	void AddTextHistory( const Char* pstrText );
	// Returns a pointer to the console buffer
	const Char* GetConsoleHistory( void ) const { return m_pConsoleBuffer; }

	// Processes a command input
	void ProcessInput( const Char* pstrText );
	// Gets the current input from the history
	const Char* GetCurrentInputHistory( void );
	// Advances along the text history
	void HistoryStepForward( void );
	// Backtraces along the text history
	void HistoryStepBack( void );
	// Tells if we've reached the end of the history
	inline bool IsAtHistoryEnd( void ) const { return m_inputHistoryList.end(); }
	// Resets the history iterator
	inline void ResetHistoryIterator( void ) { m_inputHistoryList.begin(); }

	// Spawns the console
	static void ShowConsole( void );
	// Hides the console
	static void HideConsole( void );
	// Tells if the console is active
	static bool IsConsoleVisible( void );
	// Toggles the console
	static void ToggleConsole( void );

	// Retrieves a cvar by name
	CCVar* GetCVar( const Char* pstrName );
	// Creates a console variable
	CCVar* CreateCVar( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription = nullptr, pfnCVarCallback_t pfnCallback = nullptr );

	// Adds an input to the inputs list
	void AddInput( input_type_t type, const Char* name, const Char* description );
	// Attemps to process a command from the cmd buffer
	bool ProcessCommand( void );

	// Sets a float value on a cvar
	bool CVarSetFloatValue( const Char* pstrName, Float value );
	// Sets a string value on a cvar
	bool CVarSetStringValue( const Char* pstrName, const Char* pstrValue );

	// Sets the input list tabbing filter
	const Char* GetBestInputChoice( const Char* pstrFilterString, bool retry = true );
	// Resets the tabbing function
	void ResetTabbing( void );

	// Performs think functions
	void Think( void );
	// Draws debug strings
	bool Draw( void );

	// Updates text history, if it wasn't updated already
	void UpdateTextHistory( void );

public:
	// Clears the console history
	void CmdClearHistory( void );
	// Dumps a list of either input types to the console or to a file
	void CmdInputList( input_type_t type );
	// Dumps the contents of the console to a file
	void CmdConDump( void ) const;

public:
	// Buffer for all console text
	Char* m_pConsoleBuffer;
	// Current buffer size
	Uint32 m_bufferSize;
	// Current buffer position
	Uint32 m_bufferPosition;

	// Linked list of input commands
	CLinkedList<CString> m_inputHistoryList;
	// TRUE if we reached the end
	bool m_reachedHistoryEnd;

	// TRUE if history text is up to date
	bool m_isHistoryUpdated;

private:
	// List of console variables
	CLinkedList<CCVar*> m_cvarList;
	// Alphabetically ordered list of all console inputs
	ConList_t m_inputList;

	// The input filter string for tabbing
	CString m_listFilterString;

	// Debug screen prints
	CLinkedList<debug_print_t> m_debugPrintList;

	// Cvar for toggling print box
	CCVar* m_pCvarDrawHistoryBox;

	// Saved commands for GL dependent cvars
	CArray<CString> m_glDependentCVarCommandsArray;
};
extern CConsole gConsole;
#endif //CONSOLE_H