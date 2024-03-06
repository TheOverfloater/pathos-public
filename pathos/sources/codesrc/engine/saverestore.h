/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SAVERESTORE_H
#define SAVERESTORE_H

#include "savefile.h"

class CCVar;

struct edict_t;
enum globalstate_state_t;

extern void Save_WriteBool( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteByte( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteChar( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteInt16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteUint16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteInt32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteUint32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteInt64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteUint64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteFloat( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteDouble( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteTime( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteString( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteRawString( const Char* fieldname, const byte* pdata, entfieldtype_t fieldtype );
extern void Save_WriteVector( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteCoord( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteEntindex( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
extern void Save_WriteGlobalState( Uint32 index, const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state );

/*
=======================
CSaveRestore

=======================
*/
class CSaveRestore
{
public:
	// Optimized string array alloc size
	static const Uint32 STRING_ARRAY_ALLOC_SIZE;
	// Allocation size for buffers
	static const Uint32 BUFFER_ALLOC_SIZE;
	// Save directory path
	static const Char SAVE_DIR_PATH[];
	// Save file version
	static const Uint32 SAVE_FILE_VERSION;

public:
	struct saved_string_t
	{
		saved_string_t():
			offset(-1),
			length(0)
			{
			}

		Int32 offset;
		Uint32 length;
	};

public:
	CSaveRestore();
	~CSaveRestore();

public:
	// Creates a save file
	bool CreateSaveFile( const Char* baseName, savefile_type_t type, const Vector* plandmarkorigin, CString* poutname, bool keepOld = false );
	// Loads a save file
	const byte *LoadSaveFile( const Char* savePath, Uint32* psize = nullptr );
	// Frees a save file
	void FreeSaveFile( const byte* pfile );
	// Loads a save file
	bool LoadSaveData( const save_header_t* pheader, const Vector* plandmarkoffset = nullptr );
	// Loads globals from a save file
	void LoadGlobalsFromSaveFile( const save_header_t* pheader );
	// Loads connections from a save file
	void LoadConnectionsFromSaveFile( const save_header_t* pheader );
	// Returns the save file type
	savefile_type_t GetSaveFileType( const Char* pstrSaveFile );

public:
	// Finds the most recent save file
	bool GetMostRecentSave( CString* pOutput );

private:
	// Saves an entity's data
	void SaveEntity( edict_t* pedict, save_edict_info_t& saveinfo );

	// Writes a field entry into the list
	save_field_t* SaveField( const Char* fieldname, entfieldtype_t fieldtype, const Uint32& blocksize );
	// Saves a string to the buffer
	save_block_t& SaveToStringBuffer( const Char* pstrstring );
	// Saves a buffer to the entity data buffer
	save_block_t& SaveToDataBuffer( const byte* pdata, Int32 size );

	// Cleans save files
	void CleanSaveFiles( const Char* pstrLastSave );

public:
	// Writes a integer values to the output
	void WriteBool( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteByte( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteChar( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteInt16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteUint16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteInt32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteUint32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteInt64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteUint64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a float values to the output
	void WriteFloat( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes time values to the output
	void WriteDouble( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes time values to the output
	void WriteTime( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes strings to the output
	void WriteString( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a raw string to the output
	void WriteRawString( const Char* fieldname, const byte* pdata, entfieldtype_t fieldtype );
	// Writes vectors to the output
	void WriteVector( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes coordinate vectors to the output
	void WriteCoord( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );
	// Writes a integer values to the output
	void WriteEntindex( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype );

public:
	// Writes a global state to the save file
	void WriteGlobalState( Uint32 index, const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state );

private:
	// Buffer of entities to save
	CArray<save_edict_info_t> m_saveEdictsBuffer;
	// Number of edicts in array
	Uint32 m_numSaveEdicts;

	// Buffer of fields saved for entities
	CArray<save_field_t> m_edictFieldsBuffer;
	// Number of entity fields
	Uint32 m_numEdictFields;

	// Linked list of data blocks saved
	CArray<save_block_t> m_saveDataBlocksBuffer;
	// Number of saved data blocks
	Uint32 m_numSaveDataBlocks;

	// Linked list of string blocks
	CArray<save_block_t> m_saveStringBlocksBuffer;
	// Number of saved string blocks
	Uint32 m_numSaveStringBlocks;

	// Raw data buffer for entity fields
	byte* m_pEntityDataBuffer;
	// Entity data buffer size
	Uint32 m_entityDataBufferSize;
	// Entity data buffer usage
	Uint32 m_entityDataBufferUsage;

	// Raw data buffer for strings
	byte* m_pStringBuffer;
	// String buffer size
	Uint32 m_stringBufferSize;
	// String buffer usage
	Uint32 m_stringBufferUsage;

	// Buffer of global states to save
	save_global_t* m_pGlobalStatesBuffer;
	// Number of globals expected
	Uint32 m_nbGlobalStates;

	// Server string index into string buffer map
	CArray<Int32> m_serverStringIndexArray;

	// Array of saved string
	CArray<saved_string_t> m_savedStringsArray;
	// Number of elements in array
	Uint32 m_numSavedStrings;

private:
	// Save buffer size
	Uint32 m_saveBufferSize;

	// TRUE if using a landmark
	bool m_isTransitionSave;
	// Landmark origin
	Vector m_landmarkOrigin;

	// Time base we need to get delta from
	Double m_timeBase;
};
extern CSaveRestore gSaveRestore;
#endif // SAVERESTORE_H