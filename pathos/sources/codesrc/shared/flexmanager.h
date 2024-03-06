/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FLEXMANAGER_H
#define FLEXMANAGER_H

#include "snd_shared.h"
#include "flex_shared.h"
#include "file_interface.h"

struct vbmheader_t;

enum flex_flags_t
{
	FLEX_FLAG_NONE		= 0,
	FLEX_FLAG_LOOP		= (1<<0),
	FLEX_FLAG_STAY		= (1<<1),
	FLEX_FLAG_NO_BLINK	= (1<<2)
};

struct sent_assoc_t
{
	CString sentence;
	CString flexfile;
};

/*
====================
CFlexManager

====================
*/
class CFlexManager
{
public:
	explicit CFlexManager( const file_interface_t& fileInterface );
	~CFlexManager( void );

public:
	// Resets the manager
	void Clear( void );

public:
	// Loads a flex script
	const flexscript_t* LoadScript( const Char* pscriptname );

	// Sets a script for a VBM file
	void SetScript( const vbmheader_t* pvbm, flexstate_t* pstate, Float time, const Char* pscriptname );
	// Updates values for a flex script
	void UpdateValues( Float cur_time, Float health, Int32 mouthopen, flexstate_t* pstate, bool scripting );

	// Gets a controller index for a flex
	static Int32 GetControllerIndex( const flexscript_t* pscript, const Char* name );
	// Gets a fixed flex's controller index
	static Int32 GetFixedFlexIndex( const Char* pstrname );

	// Sets flex mapping based on VBM data
	static void SetFlexMappings( const vbmheader_t* pvbm, flexstate_t* pstate );
	// Sets a controller's name
	static bool SetControllerName( const flexscript_t* pscript, flexcontroller_t* pcontroller, const Char* pstrname );

	// Gets the error message
	const Char* GetError( void ) const { return m_errorString.c_str(); }

public:
	// Loads an association script
	bool LoadAssociationScript( flextypes_t npcType, const Char* pscriptname );

	// Gets an AI script based on type and AI state
	const Char* GetAIScript( flextypes_t npcType, flexaistates_t aiState );
	// Retreives a script for a sentence
	const Char* GetSentenceScript( const Char* pstrSentence );

private:
	// Sets mouth value
	void SetMouth( Int32 mouthopen, flexstate_t* pstate );
	// Calculates blinking
	void Blink( Float cur_time, Float health, flexstate_t* pstate );
	
	// Sets the error string
	void SetError( const Char *fmt, ... );

private:
	// Array of flex scripts tied to npcs and AI states
	CArray<CString> m_flexAIStateScripts[NUM_FLEX_NPC_TYPES][NUM_FLEX_AISTATES];

	// Array of sentence-flex script associations
	CArray<sent_assoc_t> m_sentenceScriptArray;

private:
	// Cache of flex scripts
	CArray<flexscript_t*> m_scriptCache;
	// Current mouth value
	Float m_mouthValue;

	// Error string if any
	CString m_errorString;

private:
	// File access interface
	file_interface_t m_fileInterface;
};
#endif