/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "saverestore.h"
#include "file.h"
#include "edictmanager.h"
#include "sv_main.h"
#include "system.h"
#include "console.h"
#include "com_math.h"
#include "vid.h"
#include "tga.h"
#include "sv_entities.h"
#include "enginestate.h"
#include "edictmanager.h"
#include "r_common.h"
#include "stb_dxt.h"
#include "filewriterthread.h"

// Note:
// When I looked at the ReHLDS code for Half-Life's save-restore system,
// because I had no idea how to write mine, I stopped working on Pathos for
// two months because I was so discouraged. Later on, this implementation
// popped into my mind while showering. I really like how this came out.

// Optimized string array alloc size
const Uint32 CSaveRestore::STRING_ARRAY_ALLOC_SIZE = 512;
// Allocation size for buffers
const Uint32 CSaveRestore::BUFFER_ALLOC_SIZE = 1024;
#ifdef _64BUILD
// Save directory path
const Char CSaveRestore::SAVE_DIR_PATH[] = "save_x64/";
#else
// Save directory path
const Char CSaveRestore::SAVE_DIR_PATH[] = "save_x86/";
#endif
// Save file version
const Uint32 CSaveRestore::SAVE_FILE_VERSION = 2;

// Object declaration
CSaveRestore gSaveRestore;

//=============================================
// @brief Default constructor
//
//=============================================
CSaveRestore::CSaveRestore( void ):
	m_numSaveEdicts(0),
	m_numEdictFields(0),
	m_numSaveDataBlocks(0),
	m_numSaveStringBlocks(0),
	m_pEntityDataBuffer(nullptr),
	m_entityDataBufferSize(0),
	m_entityDataBufferUsage(0),
	m_pStringBuffer(0),
	m_stringBufferSize(0),
	m_stringBufferUsage(0),
	m_pGlobalStatesBuffer(nullptr),
	m_nbGlobalStates(0),
	m_numSavedStrings(0),
	m_saveBufferSize(0),
	m_isTransitionSave(false),
	m_timeBase(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CSaveRestore::~CSaveRestore( void )
{
}

//=============================================
// @brief
//
//=============================================
const byte *CSaveRestore::LoadSaveFile( const Char* savePath, Uint32* psize )
{
	// Try loading the file
	const byte* pfile = FL_LoadFile(savePath, psize);
	if(!pfile)
	{
		Con_EPrintf("%s - File '%s' not found.\n", __FUNCTION__, savePath);
		return false;
	}

	// Make sure the header is correct
	if(qstrncmp(reinterpret_cast<const Char*>(pfile), SAVEFILE_HEADER_ID, 4))
	{
		Con_EPrintf("%s - File '%s' is not a valid save file.\n", __FUNCTION__, savePath);
		return false;
	}

	// Get header information and check version
	const save_header_t* pheader = reinterpret_cast<const save_header_t*>(pfile);
	if(pheader->version != SAVE_FILE_VERSION)
	{
		Con_EPrintf("%s - File '%s' has wrong version(%d, %d expected).\n", __FUNCTION__, savePath, pheader->version, SAVE_FILE_VERSION);
		return false;
	}

	return pfile;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::FreeSaveFile( const byte* pfile )
{
	// Only this for now
	FL_FreeFile(pfile);
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::CleanSaveFiles( const Char* pstrLastSave )
{
	// Load save files list
	CString searchPath;
	searchPath << ens.gamedir << PATH_SLASH_CHAR << SAVE_DIR_PATH << "*" << SAVE_FILE_EXTENSION;

	// List of saves that are not referenced by master saves
	CLinkedList<CString> unreferencedSavesList;

	// First add all PSF files to the list
	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CString filePath;
			filePath << CSaveRestore::SAVE_DIR_PATH << findData.cFileName;

			if(qstrcmp(filePath, pstrLastSave))
			{
				savefile_type_t type = GetSaveFileType(filePath.c_str());
				if(type == SAVE_MAPSAVE)
				{
					// Make sure it's not present in current list
					svs.levelinfos.begin();
					while(!svs.levelinfos.end())
					{
						sv_levelinfo_t& info = svs.levelinfos.get();
						if(!qstrcmp(info.mapsavename, filePath))
							break;

						svs.levelinfos.next();
					}

					if(svs.levelinfos.end())
						unreferencedSavesList.radd(filePath);
				}
			}

		} while(FindNextFileA(hFind, &findData));
		
		FindClose(hFind);
	}

	// Now remove all save files which are not referenced
	hFind = FindFirstFileA(searchPath.c_str(), &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CString filePath;
			filePath << CSaveRestore::SAVE_DIR_PATH << findData.cFileName;
			
			const byte* psavefile = LoadSaveFile(filePath.c_str());
			if(psavefile)
			{
				const save_header_t* pheader = reinterpret_cast<const save_header_t*>(psavefile);
				if(pheader->type == SAVE_QUICK || pheader->type == SAVE_REGULAR || pheader->type == SAVE_AUTO)
				{
					for(Int32 i = 0; i < pheader->numlevelinfos; i++)
					{
						const save_levelinfo_t* plevelinfo = reinterpret_cast<const save_levelinfo_t*>(reinterpret_cast<const byte*>(pheader) + pheader->levelinfosdataoffset)+i;
						unreferencedSavesList.remove(plevelinfo->mapsavename);
					}
				}
			}
			FreeSaveFile(psavefile);

		} while(FindNextFileA(hFind, &findData));
		
		FindClose(hFind);
	}

	// Try to delete the files
	if(!unreferencedSavesList.empty())
	{
		unreferencedSavesList.begin();
		while(!unreferencedSavesList.end())
		{
			CString removepath;
			removepath << ens.gamedir << PATH_SLASH_CHAR << unreferencedSavesList.get();

			if(remove(removepath.c_str()) != 0)
				Con_Printf("%s - Failed to delete file '%s' - %s.\n", __FUNCTION__, removepath.c_str(), strerror(errno));

			unreferencedSavesList.next();
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CSaveRestore::GetMostRecentSave( CString* pOutput )
{
	// Load save files list
	CString searchPath;
	searchPath << ens.gamedir << PATH_SLASH_CHAR << SAVE_DIR_PATH << "*" << SAVE_FILE_EXTENSION;

	file_dateinfo_t lastBestDate;
	CString lastBestFilename;

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CString filePath;
			filePath << CSaveRestore::SAVE_DIR_PATH << findData.cFileName;
			
			savefile_type_t type = GetSaveFileType(filePath.c_str());
			if(type != SAVE_TRANSITION && type != SAVE_MAPSAVE && type != SAVE_UNDEFINED)
			{
				SYSTEMTIME sysTime;
				if(!FileTimeToSystemTime(&findData.ftLastWriteTime, &sysTime))
					Con_EPrintf("Failed to get file creation time for '%s'.\n", filePath.c_str());

				file_dateinfo_t fd;
				fd.year = sysTime.wYear;
				fd.month = sysTime.wMonth;
				fd.day = sysTime.wDay;
				fd.hour = sysTime.wHour;
				fd.minute = sysTime.wMinute;
				fd.second = sysTime.wSecond;

				if(FL_CompareFileDates(lastBestDate, fd) == 1)
				{
					lastBestDate = fd;
					lastBestFilename = filePath;
				}
			}
		} while(FindNextFileA(hFind, &findData));
		
		FindClose(hFind);
	}

	if(lastBestFilename.empty())
		return false;

	if(pOutput)
		*pOutput = lastBestFilename;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::LoadGlobalsFromSaveFile( const save_header_t* pheader )
{
	// Restore globals if any
	if(!pheader->numglobals)
		return;

	for(Int32 i = 0; i < pheader->numglobals; i++)
	{
		const save_global_t* pglobal = reinterpret_cast<const save_global_t*>(reinterpret_cast<const byte*>(pheader) + pheader->globaldataoffset)+i;
		svs.dllfuncs.pfnReadGlobalStateData(pglobal->name, pglobal->levelname, static_cast<globalstate_state_t>(pglobal->state));
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::LoadConnectionsFromSaveFile( const save_header_t* pheader )
{
	if(!pheader->numlevelinfos)
		return;

	for(Int32 i = 0; i < pheader->numlevelinfos; i++)
	{
		const save_levelinfo_t* plevelinfo = reinterpret_cast<const save_levelinfo_t*>(reinterpret_cast<const byte*>(pheader) + pheader->levelinfosdataoffset)+i;
			
		for(Int32 j = 0; j < plevelinfo->numconnections; j++)
		{
			const save_level_connection_t* pconnection = reinterpret_cast<const save_level_connection_t*>(reinterpret_cast<const byte*>(pheader) + plevelinfo->connectioninfoindex)+j;
			SV_AddLevelConnection(plevelinfo->mapname, pconnection->othermapname, pconnection->landmarkname, plevelinfo->mapsavename);
		}
	}
}

//=============================================
// @brief
//
//=============================================
savefile_type_t CSaveRestore::GetSaveFileType( const Char* pstrSaveFile )
{
	const byte* pfile = LoadSaveFile(pstrSaveFile);
	if(!pfile)
		return SAVE_UNDEFINED;

	const save_header_t* phdr = reinterpret_cast<const save_header_t*>(pfile);
	savefile_type_t type = phdr->type;
	FreeSaveFile(pfile);

	return type;
}

//=============================================
// @brief
//
//=============================================
bool CSaveRestore::LoadSaveData( const save_header_t* pheader, const Vector* plandmarkoffset )
{
	if(!pheader)
		return false;

	// Besides map state saves, all save types, including
	// transition saves, need to hold the game time, as we
	// restore it from the transition save on level change
	if(pheader->type != SAVE_MAPSAVE)
		svs.gamevars.gametime = pheader->gametime;
	
	// Set any cvars we saved
	if(pheader->numcvars > 0)
	{
		const save_cvar_t* pcvars = reinterpret_cast<const save_cvar_t*>(reinterpret_cast<const byte*>(pheader) + pheader->cvarsoffset);
		for(Uint32 i = 0; i < pheader->numcvars; i++)
		{
			switch(pcvars[i].type)
			{
			default:
			case CVAR_FLOAT:
				{
					Float value = SDL_atof(pcvars[i].value);
					gConsole.CVarSetFloatValue(pcvars[i].name, value);
				}
				break;
			case CVAR_STRING:
				{
					gConsole.CVarSetStringValue(pcvars[i].name, pcvars[i].value);
				}
				break;
			}
		}
	}

	// Prepare the index map
	CArray<entindex_t> saveEntityIndexMapArray;
	saveEntityIndexMapArray.resize(pheader->numentities);

	for(Int32 i = 0; i < pheader->numentities; i++)
		saveEntityIndexMapArray[i] = NO_ENTITY_INDEX;

	// Get pointers to data
	const save_field_t* pfields = reinterpret_cast<const save_field_t*>(reinterpret_cast<const byte*>(pheader) + pheader->entityfieldsoffset);
	const save_block_t* pdatablocks = reinterpret_cast<const save_block_t*>(reinterpret_cast<const byte*>(pheader) + pheader->entitydatablocksoffset);
	const save_block_t* pstringblocks = reinterpret_cast<const save_block_t*>(reinterpret_cast<const byte*>(pheader) + pheader->stringblocksoffset);
	 
	// Get pointers to buffers
	const byte* pdatabuffer = (reinterpret_cast<const byte*>(pheader) + pheader->entitydataoffset);
	const byte* pstringbuffer = (reinterpret_cast<const byte*>(pheader) + pheader->stringdataoffset);

	// Load the entities in
	for(Int32 i = 0; i < pheader->numentities; i++)
	{
		// Get entity information
		const save_edict_info_t* pentity = reinterpret_cast<const save_edict_info_t*>(reinterpret_cast<const byte*>(pheader) + pheader->entitydescoffset) + i;

		// World and player are handled specially
		edict_t* pedict = nullptr;
		if(pentity->isworldspawn)
		{
			// Worldspawn is handled specially
			pedict = gEdicts.GetEdict(WORLDSPAWN_ENTITY_INDEX);
			pedict->free = false;
		}
		else if(pentity->isplayer)
		{
			// Player is handled specially
			pedict = gEdicts.GetEdict(HOST_CLIENT_ENTITY_INDEX);
			pedict->free = false;
		}
		else if(pentity->isglobalentity && pheader->type == SAVE_TRANSITION)
		{
			// Find the global entity on the current level
			pedict = svs.dllfuncs.pfnFindGlobalEntity(pentity->classname, pentity->globalname);
			if(!pedict || pedict->free)
				continue;
		}
		else
		{
			// Allocate an edict for each of these
			pedict = gEdicts.AllocEdict();
		}

		// Allocate entity class data for non-player and non-worldspawn entities
		for(Int32 j = 0; j < pentity->fieldsdata_numfields; j++)
		{
			const save_field_t* pfield = &pfields[pentity->fieldsdata_startindex + j];
			const save_block_t* pfieldnameblock = &pstringblocks[pfield->fieldnameindex];
			CString fieldname(reinterpret_cast<const Char*>(&pstringbuffer[pfieldnameblock->dataoffset]), pfieldnameblock->datasize);

			if(!qstrcmp(fieldname, "classname"))
			{
				const save_block_t* pclassnameblock = &pstringblocks[pfield->blockindex];
				CString classnamevalue(reinterpret_cast<const Char*>(&pstringbuffer[pclassnameblock->dataoffset]), pclassnameblock->datasize);

				if(pentity->isplayer)
				{
					pedict = gEdicts.CreatePlayerEntity(pentity->entityindex-1);
					if(!pedict)
					{
						Con_EPrintf("%s - Failed to initilize class data for '%s'.\n", __FUNCTION__, classnamevalue.c_str());
						return false;
					}
				}
				else if(pentity->isworldspawn)
				{
					if(!SV_InitPrivateData(pedict, classnamevalue.c_str()))
					{
						Con_EPrintf("%s - Failed to initilize class data for '%s'.\n", __FUNCTION__, classnamevalue.c_str());
						return false;
					}
				}
				else if(!pentity->isglobalentity || pheader->type != SAVE_TRANSITION)
				{
					if(!SV_InitPrivateData(pedict, classnamevalue.c_str()))
						Con_EPrintf("%s - Failed to initilize class data for '%s'.\n", __FUNCTION__, classnamevalue.c_str());
				}

				break;
			}
		}

		// Notify about failed entities, but don't fail
		if(!pedict->pprivatedata)
		{
			Con_EPrintf("%s - Failed to allocate entity %d.\n", __FUNCTION__, i);
			gEdicts.FreeEdict(pedict, EDICT_REMOVED_AT_INIT);
			continue;
		}

		// Remember this
		saveEntityIndexMapArray[i] = pedict->entindex;
	}

	// Begin the load-save
	bool isLoadSave = (pheader->type == SAVE_REGULAR || pheader->type == SAVE_QUICK || pheader->type == SAVE_AUTO) ? true : false;
	bool isTransitionSave = (pheader->type == SAVE_TRANSITION) ? true : false;
	bool isTransitionLoad = (pheader->type == SAVE_TRANSITION || pheader->type == SAVE_MAPSAVE) ? true : false;

	// Tell game what we're using to load
	svs.dllfuncs.pfnBeginLoadSave(isLoadSave, isTransitionSave, isTransitionLoad, plandmarkoffset, saveEntityIndexMapArray);

	// Restore data to the entities
	for(Int32 i = 0; i < pheader->numentities; i++)
	{
		if(saveEntityIndexMapArray[i] == NO_ENTITY_INDEX)
			continue;
		
		// Get entity information
		const save_edict_info_t* pentity = reinterpret_cast<const save_edict_info_t*>(reinterpret_cast<const byte*>(pheader) + pheader->entitydescoffset) + i;
		// TRUE if this is a global entity transfer restore
		bool istransferglobal = (pentity->isglobalentity && pheader->type == SAVE_TRANSITION) ? true : false;

		// World and player are handled specially
		edict_t* pedict = gEdicts.GetEdict(saveEntityIndexMapArray[i]);
		if(!pedict)
		{
			Con_EPrintf("%s - Failure while reading entity state data for entity %d.\n", __FUNCTION__, pentity->entityindex);
			return false;
		}

		// Set entity state data
		for(Int32 j = 0; j < pentity->statedata_numfields; j++)
		{
			const save_field_t* pfield = &pfields[pentity->statedata_startindex + j];
			const save_block_t* pfieldnameblock = &pstringblocks[pfield->fieldnameindex];

			CString fieldname(reinterpret_cast<const Char*>(&pstringbuffer[pfieldnameblock->dataoffset]), pfieldnameblock->datasize);

			switch(pfield->datatype)
			{
			case EFIELD_BYTE:
			case EFIELD_CHAR:
				{
					// These types are read all at once
					const save_block_t* pdatablock = &pdatablocks[pfield->blockindex];
					const byte* pblockdata = &pdatabuffer[pdatablock->dataoffset];

					if(!svs.dllfuncs.pfnReadEntityStateData(pedict, fieldname.c_str(), pblockdata, pdatablock->datasize, 0, istransferglobal))
					{
						Con_EPrintf("%s - Failure while reading entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
						return false;
					}
				}
				break;
			case EFIELD_FLOAT:
			case EFIELD_DOUBLE:
			case EFIELD_ENTINDEX:
			case EFIELD_ENTPOINTER:
			case EFIELD_EDICT:
			case EFIELD_ENTSTATE:
			case EFIELD_EHANDLE:
			case EFIELD_VECTOR:
			case EFIELD_COORD:
			case EFIELD_INT16:
			case EFIELD_UINT16:
			case EFIELD_INT32:
			case EFIELD_UINT32:
			case EFIELD_INT64:
			case EFIELD_UINT64:
			case EFIELD_FUNCPTR:
			case EFIELD_BOOLEAN:
			case EFIELD_TIME:
				{
					for(Int32 k = 0; k < pfield->numblocks; k++)
					{
						const save_block_t* pdatablock = &pdatablocks[pfield->blockindex + k];
						const byte* pblockdata = &pdatabuffer[pdatablock->dataoffset];

						// Extract the name of the field
						if(!svs.dllfuncs.pfnReadEntityStateData(pedict, fieldname.c_str(), pblockdata, pdatablock->datasize, k, istransferglobal))
						{
							Con_EPrintf("%s - Failure while reading entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
							return false;
						}
					}
				}
				break;
			default:
				{
					Con_EPrintf("%s - Invalid or unsupported field type %d specified for entity state field '%s'.\n", __FUNCTION__, pentity->entityindex, fieldname.c_str());
					return false;
				}
				break;
			}
		}

		// Set entity field data
		for(Int32 j = 0; j < pentity->fieldsdata_numfields; j++)
		{
			const save_field_t* pfield = &pfields[pentity->fieldsdata_startindex + j];
			const save_block_t* pfieldnameblock = &pstringblocks[pfield->fieldnameindex];

			CString fieldname(reinterpret_cast<const Char*>(&pstringbuffer[pfieldnameblock->dataoffset]), pfieldnameblock->datasize);

			if(pfield->datatype != EFIELD_STRING
				&& pfield->datatype != EFIELD_MODELNAME
				&& pfield->datatype != EFIELD_SOUNDNAME)
			{
				Con_EPrintf("%s - Invalid or unsupported field type %d specified for entity saved field '%s'.\n", __FUNCTION__, pentity->entityindex, fieldname.c_str());
				return false;
			}

			for(Int32 k = 0; k < pfield->numblocks; k++)
			{
				const save_block_t* pdatablock = &pstringblocks[pfield->blockindex + k];
				if(pdatablock->dataoffset == -1 || !pdatablock->datasize)
					continue;

				const byte* pblockdata = &pstringbuffer[pdatablock->dataoffset];

				// Extract the name of the field
				if(!svs.dllfuncs.pfnReadEntityFieldData(pedict, fieldname.c_str(), pblockdata, pdatablock->datasize, k, istransferglobal))
				{
					Con_EPrintf("%s - Failure while reading entity field data for entity %d.\n", __FUNCTION__, pentity->entityindex);
					return false;
				}
			}
		}

		// Tell server to prep the save fields for restoring class data
		svs.dllfuncs.pfnDispatchDeclareSaveFields(pedict);

		for(Int32 j = 0; j < pentity->classdata_numfields; j++)
		{
			const save_field_t* pfield = &pfields[pentity->classdata_startindex + j];
			const save_block_t* pfieldnameblock = &pstringblocks[pfield->fieldnameindex];

			// Extract the name of the field
			CString fieldname(reinterpret_cast<const Char*>(&pstringbuffer[pfieldnameblock->dataoffset]), pfieldnameblock->datasize);

			switch(pfield->datatype)
			{
			case EFIELD_CBITSET:
				{
					// Allow server to prep the data
					if(!svs.dllfuncs.pfnPrepareEntityClassData(pedict, fieldname.c_str(), pfield->numblocks, istransferglobal))
					{
						Con_EPrintf("%s - Failure while preparing entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
						return false;
					}

					// In case of CBitSet, the numblocks variable tells the number of bits specified
					const save_block_t* pdatablock = &pdatablocks[pfield->blockindex];
					const byte* pblockdata = &pdatabuffer[pdatablock->dataoffset];

					if(!svs.dllfuncs.pfnReadEntityClassData(pedict, fieldname.c_str(), pblockdata, pfield->numblocks, 0, istransferglobal))
					{
						Con_EPrintf("%s - Failure while reading entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
						return false;
					}
				}
				break;
			case EFIELD_BYTE:
			case EFIELD_CHAR:
				{
					// These types are read all at once
					const save_block_t* pdatablock = &pdatablocks[pfield->blockindex];
					const byte* pblockdata = &pdatabuffer[pdatablock->dataoffset];

					if(!svs.dllfuncs.pfnReadEntityClassData(pedict, fieldname.c_str(), pblockdata, pdatablock->datasize, 0, istransferglobal))
					{
						Con_EPrintf("%s - Failure while reading entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
						return false;
					}
				}
				break;
			default:
				{
					// Allow server to prep the data
					if(!svs.dllfuncs.pfnPrepareEntityClassData(pedict, fieldname.c_str(), pfield->numblocks, istransferglobal))
					{
						Con_EPrintf("%s - Failure while preparing entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
						return false;
					}

					for(Int32 k = 0; k < pfield->numblocks; k++)
					{
						const save_block_t* pdatablock = nullptr; 
						const byte* pblockdata = nullptr;
						if(pfield->datatype == EFIELD_MODELNAME || pfield->datatype == EFIELD_SOUNDNAME || pfield->datatype == EFIELD_STRING 
							|| pfield->datatype == EFIELD_CARRAY_STRING || pfield->datatype == EFIELD_FUNCPTR)
						{
							pdatablock = &pstringblocks[pfield->blockindex + k];
							if(pdatablock->dataoffset == -1 || !pdatablock->datasize)
								continue;

							pblockdata = &pstringbuffer[pdatablock->dataoffset];
						}
						else
						{
							pdatablock = &pdatablocks[pfield->blockindex + k];
							pblockdata = &pdatabuffer[pdatablock->dataoffset];
						}

						if(!svs.dllfuncs.pfnReadEntityClassData(pedict, fieldname.c_str(), pblockdata, pdatablock->datasize, k, istransferglobal))
						{
							Con_EPrintf("%s - Failure while reading entity class data for entity %d.\n", __FUNCTION__, pentity->entityindex);
							return false;
						}
					}
				}
				break;
			}
		}

		// Fix angles on client
		if(SV_IsHostClient(pentity->entityindex))
			pedict->state.fixangles = true;

		// Tell the game to restore this entity
		if(!svs.dllfuncs.pfnDispatchRestore(pedict, istransferglobal))
		{
			CString msg;
			msg << __FUNCTION__ << " - Entity " << SV_GetString(pedict->fields.classname);
			if(pedict->fields.targetname != NO_STRING_VALUE)
				msg << "(" << SV_GetString(pedict->fields.targetname) << ")";
			msg << " returned error on restore.\n";
			Con_DPrintf(msg.c_str());

			gEdicts.FreeEdict(pedict, EDICT_REMOVED_AT_RESTORE);
		}

		// Do any adjustments post-transition for global entities
		// This is required for brushents without an origin brush
		if(isTransitionLoad && pentity->isglobalentity)
			svs.dllfuncs.pfnAdjustEntityPositions(pedict, pentity->mins);

		// Tell server to release the save fields for restoring
		svs.dllfuncs.pfnDispatchReleaseSaveFields(pedict);
	}

	// Transitions manage this manually
	if(pheader->type != SAVE_TRANSITION && pheader->type != SAVE_MAPSAVE && pheader->numlevelinfos > 0)
	{
		// Restore connections
		LoadConnectionsFromSaveFile(pheader);
	}

	if(isLoadSave)
	{
		// Load globals directly if it's a save-load
		LoadGlobalsFromSaveFile(pheader);
	}

	// Load any decals
	if(pheader->numdecals > 0)
	{
		for(Int32 i = 0; i < pheader->numdecals; i++)
		{
			const save_decalinfo_t* pdecalinfo = reinterpret_cast<const save_decalinfo_t*>(reinterpret_cast<const byte*>(pheader) + pheader->decaldataoffset) + i;

			edict_t* pedict = nullptr;
			if(pdecalinfo->entityindex != NO_ENTITY_INDEX)
			{
				entindex_t entityindex = saveEntityIndexMapArray[pdecalinfo->entityindex];
				pedict = gEdicts.GetEdict(entityindex);
				if(pedict->free)
					continue;
			}

			saveddecal_t newdecal;
			newdecal.decaltexture = pdecalinfo->texturename;
			newdecal.origin = pdecalinfo->origin;
			newdecal.normal = pdecalinfo->normal;
			newdecal.pedict = pedict;
			newdecal.flags = pdecalinfo->flags;

			// Set new edict identifier
			if(newdecal.pedict)
				newdecal.identifier = newdecal.pedict->identifier;

			// Apply transition offsets if required
			if(pheader->type == SAVE_TRANSITION && plandmarkoffset)
				Math::VectorAdd(newdecal.origin, *plandmarkoffset, newdecal.origin);

			svs.saveddecalslist.add(newdecal);
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CSaveRestore::CreateSaveFile( const Char* baseName, savefile_type_t type, const Vector* plandmarkorigin, CString* poutname, bool keepOld )
{
	// Allow game dll to override this
	if(!svs.dllfuncs.pfnCanSaveGame(type))
	{
		Con_DPrintf("%s - Save attempt blocked by game dll.\n", __FUNCTION__);
		return false;
	}

	// Make sure transition saves are valid
	if(type == SAVE_TRANSITION && (plandmarkorigin == nullptr || plandmarkorigin->IsZero()))
	{
		Con_EPrintf("%s - Called with type SAVE_TRANSITION but no landmark set.\n", __FUNCTION__);
		return false;
	}
	
	// Create filename first
	CString filename;
	CString filenamebase;

	// Make sure the save dir is there
	if(!FL_CreateDirectory(SAVE_DIR_PATH))
	{
		Con_EPrintf("%s - Failed to create save directory.\n", __FUNCTION__);
		return false;
	}

	// Craft the base filename
	filenamebase << SAVE_DIR_PATH << baseName;

	if(!keepOld && (type == SAVE_AUTO || type == SAVE_QUICK) || type == SAVE_TRANSITION)
	{
		// Just set the name without the number
		filename << filenamebase << SAVE_FILE_EXTENSION;
	}
	else if(type != SAVE_AUTO && type != SAVE_QUICK)
	{
		// Keep increasing the number till we have a spot available
		Uint32 i = 0;
		while(true)
		{
			// Add a number to the filename
			filename.clear();
			filename << filenamebase << "-" << static_cast<Int32>(i) << SAVE_FILE_EXTENSION;
			if(!FL_FileExists(filename.c_str()))
				break;

			i++;
		}
	}
	else
	{
		// Craft filename with token
		filename.clear();
		filename << filenamebase << "-%number%" << SAVE_FILE_EXTENSION;
	}
	
	// Set out name if specified
	if(poutname)
		(*poutname) = filename;

	if(type == SAVE_TRANSITION)
	{
		// Set for delta
		m_landmarkOrigin = (*plandmarkorigin);
		m_timeBase = svs.time;

		m_isTransitionSave = true;
	}
	else
	{
		// Not using a landmark
		m_landmarkOrigin.Clear();
		m_timeBase = 0;

		m_isTransitionSave = false;

		// Add cvars to save if not transition save
		// Add sky color cvars
		if(!AddSavedCVar(g_psv_skycolor_r))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skycolor_r->GetName());
			return false;
		}

		if(!AddSavedCVar(g_psv_skycolor_g))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skycolor_g->GetName());
			return false;
		}

		if(!AddSavedCVar(g_psv_skycolor_b))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skycolor_b->GetName());
			return false;
		}

		// Add skyvec cvars
		if(!AddSavedCVar(g_psv_skyvec_x))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skyvec_x->GetName());
			return false;
		}

		if(!AddSavedCVar(g_psv_skyvec_y))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skyvec_y->GetName());
			return false;
		}

		if(!AddSavedCVar(g_psv_skyvec_z))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skyvec_z->GetName());
			return false;
		}

		// Add skyname cvar
		if(!AddSavedCVar(g_psv_skyname))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skyname->GetName());
			return false;
		}

		// If not transition or map state save, then save skill
		if(type != SAVE_MAPSAVE && !AddSavedCVar(g_psv_skill))
		{
			Con_EPrintf("%s - Error while trying to save cvar '%s'.\n", __FUNCTION__, g_psv_skill->GetName());
			return false;
		}
	}

	// Set this to default
	m_saveBufferSize = sizeof(save_header_t);

	Uint32 nbEdicts = gEdicts.GetNbEdicts();
	m_saveEdictsBuffer.resize(nbEdicts);

	// Go through each edict and save it
	for(Uint32 i = 0; i < nbEdicts; i++)
	{
		edict_t* pedict = gEdicts.GetEdict(i);
		if(pedict->free)
			continue;

		// Don't save if not required to
		if(!svs.dllfuncs.pfnShouldSaveEntity(pedict))
		{
			CString msg;
			msg << "Entity " << SV_GetString(pedict->fields.classname);
			if(pedict->fields.targetname != NO_STRING_VALUE)
				msg << " (" << SV_GetString(pedict->fields.targetname) << ")";
			msg << " not saved.\n";

			Con_DPrintf(msg.c_str());
			continue;
		}

		// Special handling for transitioning entities
		if(type == SAVE_TRANSITION || type == SAVE_MAPSAVE)
		{
			// See if entity wants to transition to the next level
			bool transitionEntity = svs.dllfuncs.pfnShouldTransitionEntity(pedict);

			// Only save entity if it should transition
			if((type == SAVE_TRANSITION && !transitionEntity || type == SAVE_MAPSAVE && transitionEntity) 
				&& !svs.dllfuncs.pfnIsGlobalTransitioningEntity(pedict)) // Transitioning entities with global names are put into both files
				continue;
		}

		// Add the entry
		save_edict_info_t& saveentity = m_saveEdictsBuffer[m_numSaveEdicts];
		m_numSaveEdicts++;

		saveentity.entityindex = pedict->entindex;
		m_saveBufferSize += sizeof(save_edict_info_t);
	}

	// Add in our connections
	if(type != SAVE_MAPSAVE && !svs.levelinfos.empty())
	{
		Uint32 numconnections = 0;
		svs.levelinfos.begin();
		while(!svs.levelinfos.end())
		{
			numconnections += svs.levelinfos.get().connectionslist.size();
			svs.levelinfos.next();
		}
		
		m_saveBufferSize += sizeof(save_levelinfo_t)*svs.levelinfos.size();
		m_saveBufferSize += sizeof(save_level_connection_t)*numconnections;
	}

	// Add in cvars if present
	if(!m_savedCVarsArray.empty())
		m_saveBufferSize += sizeof(save_cvar_t)*m_savedCVarsArray.size();

	// Allocate save buffers
	m_pEntityDataBuffer = new byte[BUFFER_ALLOC_SIZE];
	m_entityDataBufferSize = BUFFER_ALLOC_SIZE;
	m_entityDataBufferUsage = 0;

	m_pStringBuffer = new byte[BUFFER_ALLOC_SIZE];
	m_stringBufferSize = BUFFER_ALLOC_SIZE;
	m_stringBufferUsage = 0;

	m_edictFieldsBuffer.resize(BUFFER_ALLOC_SIZE);
	m_numEdictFields = 0;

	m_saveDataBlocksBuffer.resize(BUFFER_ALLOC_SIZE);
	m_numSaveDataBlocks = 0;

	m_saveStringBlocksBuffer.resize(BUFFER_ALLOC_SIZE);
	m_numSaveStringBlocks = 0;

	if(!m_savedStringPositionMap.empty())
		m_savedStringPositionMap.clear();

	m_savedStringsArray.resize(STRING_ARRAY_ALLOC_SIZE);
	m_numSavedStrings = 0;

	// Now loop through all entities and get their data
	for(Uint32 i = 0; i < m_numSaveEdicts; i++)
	{
		// Retrieve entity from table
		save_edict_info_t& saveinfo = m_saveEdictsBuffer[i];
		// Get edict
		edict_t* pedict = gEdicts.GetEdict(saveinfo.entityindex);

		SaveEntity(pedict, saveinfo);
	}

	// Write global states if quick, auto or regular save
	Uint32 ngblobalstates = 0;
	if(type != SAVE_MAPSAVE)
	{
		ngblobalstates = svs.dllfuncs.pfnGetNbGlobalStates();
		m_saveBufferSize += ngblobalstates*sizeof(save_global_t);
	}

	// Grab the screen contents
	Uint32 width = 0, height = 0, bpp = 0;
	byte* pscreentexture = nullptr;

	// Save screenshot for regular, quick and auto saves
	if(type == SAVE_REGULAR || type == SAVE_QUICK || type == SAVE_AUTO)
	{
		// Draw but don't swap buffers
		VID_DrawSceneOnly();
		VID_GetScreenContents(width, height, bpp, &pscreentexture);

		R_ResizeTextureToPOT(width, height, pscreentexture);
		R_FlipTexture(width, height, bpp, false, true, pscreentexture);

		Uint32 texsize = (sizeof(byte)*width*height*bpp)/8;
		m_saveBufferSize += texsize;
	}

	// Source decal list
	CLinkedList<saveddecal_t>* psrcdecallist = nullptr;
	if(!svs.saveddecalslist.empty() && (type == SAVE_MAPSAVE || type == SAVE_REGULAR || type == SAVE_QUICK || type == SAVE_AUTO))
		psrcdecallist = &svs.saveddecalslist;
	else if(!svs.levelchangeinfo.transitiondecallist.empty() && type == SAVE_TRANSITION)
		psrcdecallist = &svs.levelchangeinfo.transitiondecallist;

	// List of decals to save
	CLinkedList<save_decalinfo_t> decallist;

	// Save decals for regular, quick and auto saves
	if(psrcdecallist)
	{
		psrcdecallist->begin();
		while(!psrcdecallist->end())
		{
			saveddecal_t& decal = psrcdecallist->get();
			save_decalinfo_t savedecal;

			if(decal.pedict)
			{
				// Make sure entity is still valid if there's one
				if(decal.identifier != decal.pedict->identifier || decal.pedict->free)
				{
					psrcdecallist->next();
					continue;
				}

				// Find entity in table of saved entities
				entindex_t entindex = NO_ENTITY_INDEX;
				for(Uint32 i = 0; i < m_numSaveEdicts; i++)
				{
					if(m_saveEdictsBuffer[i].entityindex == decal.pedict->entindex)
					{
						entindex = static_cast<entindex_t>(i);
						break;
					}
				}
				
				if(entindex == NO_ENTITY_INDEX)
				{
					psrcdecallist->next();
					continue;
				}

				savedecal.entityindex = entindex;
				savedecal.identifier = decal.identifier;
			}
			else
			{
				// Set to default null value
				savedecal.entityindex = NO_ENTITY_INDEX;
			}

			qstrcpy(savedecal.texturename, decal.decaltexture.c_str());
			savedecal.normal = decal.normal;
			savedecal.origin = decal.origin;
			savedecal.flags = decal.flags;
			decallist.add(savedecal);

			psrcdecallist->next();
		}

		m_saveBufferSize += sizeof(save_decalinfo_t)*decallist.size();
	}

	// Setup save file buffer
	byte* pbuffer = new byte[m_saveBufferSize];
	memset(pbuffer, 0, sizeof(byte)*m_saveBufferSize);

	// Set header info
	save_header_t* pheader = reinterpret_cast<save_header_t*>(pbuffer);

	pheader->id = SAVE_HEADER_ENCODED;
	pheader->version = SAVE_FILE_VERSION;
	pheader->type = type;

	// Besides map state saves, all save types, including
	// transition saves, need to hold the game time, as we
	// restore it from the transition save on level change
	if(pheader->type != SAVE_MAPSAVE)
		pheader->gametime = svs.gamevars.gametime;

	// Copy strings
	strcpy(pheader->name, filename.c_str());
	strcpy(pheader->mapname, svs.mapname.c_str());

	// Call game dll to set header
	svs.dllfuncs.pfnGetSaveGameTitle(pheader->saveheader, SAVE_FILE_HEADER_MAX_LENGTH);

	if(type == SAVE_TRANSITION)
	{
		// Set in header
		pheader->landmarkorigin = (*plandmarkorigin);
		pheader->svtime = 0;
	}
	else
	{
		// Clear these in the header
		pheader->landmarkorigin.Clear();
		pheader->svtime = svs.time;
	}

	// Current position in written data
	Uint32 currentdatapos = sizeof(save_header_t);

	// Save the edicts to the file
	pheader->entitydescoffset = currentdatapos;
	pheader->numentities = m_numSaveEdicts;

	byte* pdest = pbuffer + pheader->entitydescoffset;
	currentdatapos += sizeof(save_edict_info_t)*pheader->numentities;

	// Save entities to file
	for(Uint32 i = 0; i < m_numSaveEdicts; i++)
	{
		// Retrieve entity from table
		save_edict_info_t& saveinfo = m_saveEdictsBuffer[i];
		memcpy(pdest, &saveinfo, sizeof(save_edict_info_t));
		pdest += sizeof(save_edict_info_t);
	}

	m_saveEdictsBuffer.clear();
	m_numSaveEdicts = 0;

	// Save entity fields to file
	pheader->entityfieldsoffset = currentdatapos;
	pheader->numentityfields = m_numEdictFields;

	pdest = pbuffer + pheader->entityfieldsoffset;
	currentdatapos += pheader->numentityfields*sizeof(save_field_t);

	// Save fields to the output
	for(Uint32 i = 0; i < m_numEdictFields; i++)
	{
		save_field_t& savefield = m_edictFieldsBuffer[i];
		memcpy(pdest, &savefield, sizeof(save_field_t));
		pdest += sizeof(save_field_t);
	}

	m_edictFieldsBuffer.clear();
	m_numEdictFields = 0;

	// Write entity data blocks to output
	pheader->entitydatablocksoffset = currentdatapos;
	pheader->numentitydatablocks = m_numSaveDataBlocks;

	pdest = pbuffer + pheader->entitydatablocksoffset;
	currentdatapos += sizeof(save_block_t)*pheader->numentitydatablocks;

	// Save entity blocks to output
	for(Uint32 i = 0; i < m_numSaveDataBlocks; i++)
	{
		save_block_t& saveblock = m_saveDataBlocksBuffer[i];
		memcpy(pdest, &saveblock, sizeof(save_block_t));
		pdest += sizeof(save_block_t);
	}

	m_saveDataBlocksBuffer.clear();
	m_numSaveDataBlocks = 0;

	// Write string blocks to output
	pheader->stringblocksoffset = currentdatapos;
	pheader->numstringblocks = m_numSaveStringBlocks;

	pdest = pbuffer + pheader->stringblocksoffset;
	currentdatapos += sizeof(save_block_t)*pheader->numstringblocks;

	// Save string blocks to output
	for(Uint32 i = 0; i < m_numSaveStringBlocks; i++)
	{
		save_block_t& saveblock = m_saveStringBlocksBuffer[i];
		memcpy(pdest, &saveblock, sizeof(save_block_t));
		pdest += sizeof(save_block_t);
	}

	m_saveStringBlocksBuffer.clear();
	m_numSaveStringBlocks = 0;

	if(!m_savedStringPositionMap.empty())
		m_savedStringPositionMap.clear();

	if(m_pStringBuffer)
	{
		// Save string buffer to output
		pheader->stringdataoffset = currentdatapos;
		pheader->stringdatasize = sizeof(byte)*m_stringBufferUsage;
		currentdatapos += pheader->stringdatasize;

		pdest = pbuffer + pheader->stringdataoffset;
		memcpy(pdest, m_pStringBuffer, sizeof(byte)*pheader->stringdatasize);

		delete[] m_pStringBuffer;
		m_pStringBuffer = nullptr;
	}

	m_stringBufferSize = 0;
	m_stringBufferUsage = 0;

	if(!m_savedStringsArray.empty())
	{
		m_savedStringsArray.clear();
		m_numSavedStrings = 0;
	}

	if(m_pEntityDataBuffer)
	{
		// Save entity data buffer to file
		pheader->entitydataoffset = currentdatapos;
		pheader->entitydatasize = sizeof(byte)*m_entityDataBufferUsage;
		currentdatapos += pheader->entitydatasize;

		pdest = pbuffer + pheader->entitydataoffset;
		memcpy(pdest, m_pEntityDataBuffer, sizeof(byte)*pheader->entitydatasize);

		// Clear up everything
		delete[] m_pEntityDataBuffer;
		m_pEntityDataBuffer = nullptr;
	}

	m_entityDataBufferSize = 0;
	m_entityDataBufferUsage = 0;

	// Save connections data
	if(pheader->type != SAVE_MAPSAVE && !svs.levelinfos.empty())
	{
		pheader->levelinfosdataoffset = currentdatapos;
		pheader->numlevelinfos = svs.levelinfos.size();
		currentdatapos += sizeof(save_levelinfo_t)*pheader->numlevelinfos;

		Uint32 levelindex = 0;
		svs.levelinfos.begin();
		while(!svs.levelinfos.end())
		{
			sv_levelinfo_t& psrclevelinfo = svs.levelinfos.get();
			save_levelinfo_t* pdstlevelinfo = reinterpret_cast<save_levelinfo_t*>(pbuffer + pheader->levelinfosdataoffset)+levelindex;

			qstrcpy(pdstlevelinfo->mapname, psrclevelinfo.mapname.c_str());
			qstrcpy(pdstlevelinfo->mapsavename, psrclevelinfo.mapsavename.c_str());

			pdstlevelinfo->connectioninfoindex = currentdatapos;
			pdstlevelinfo->numconnections = psrclevelinfo.connectionslist.size();
			currentdatapos += sizeof(save_level_connection_t)*pdstlevelinfo->numconnections;

			Uint32 connindex = 0;
			psrclevelinfo.connectionslist.begin();
			while(!psrclevelinfo.connectionslist.end())
			{
				sv_level_connection_t& psrcconninfo = psrclevelinfo.connectionslist.get();
				save_level_connection_t* pdstconninfo = reinterpret_cast<save_level_connection_t*>(pbuffer + pdstlevelinfo->connectioninfoindex)+connindex;

				qstrcpy(pdstconninfo->othermapname, psrcconninfo.othermapname.c_str());
				qstrcpy(pdstconninfo->landmarkname, psrcconninfo.landmarkname.c_str());

				psrclevelinfo.connectionslist.next();
				connindex++;
			}

			svs.levelinfos.next();
			levelindex++;
		}
	}
	
	// Write cvars we'll need
	if(!m_savedCVarsArray.empty())
	{
		pheader->cvarsoffset = currentdatapos;
		pheader->numcvars = m_savedCVarsArray.size();

		currentdatapos += sizeof(save_cvar_t)*pheader->numcvars;

		save_cvar_t* pcvars = reinterpret_cast<save_cvar_t*>(reinterpret_cast<byte*>(pheader) + pheader->cvarsoffset);
		for(Uint32 i = 0; i < m_savedCVarsArray.size(); i++)
		{
			saved_cvar_t& srccvar = m_savedCVarsArray[i];
			save_cvar_t& destcvar = pcvars[i];

			qstrcpy_s(destcvar.name, srccvar.name.c_str(), SAVE_FILE_STRING_MAX_LENGTH);
			qstrcpy_s(destcvar.value, srccvar.value.c_str(), SAVE_FILE_STRING_MAX_LENGTH);
			destcvar.type = srccvar.type;
		}

		m_savedCVarsArray.clear();
	}

	// Write global states if quick, auto or regular save
	if(ngblobalstates > 0)
	{
		pheader->globaldataoffset = currentdatapos;
		pheader->numglobals = ngblobalstates;
		currentdatapos += sizeof(save_global_t)*pheader->numglobals;

		// Set buffer ptr
		m_pGlobalStatesBuffer = reinterpret_cast<save_global_t*>(reinterpret_cast<byte*>(pheader) + pheader->globaldataoffset);
		m_nbGlobalStates = ngblobalstates;

		// Tell game dll to save global states
		svs.dllfuncs.pfnSaveGlobalStates();

		// Reset these
		m_pGlobalStatesBuffer = nullptr;
	}

	// Write decals
	if(!decallist.empty())
	{
		pheader->decaldataoffset = currentdatapos;
		pheader->numdecals = decallist.size();
		currentdatapos += sizeof(save_decalinfo_t)*pheader->numdecals;

		Uint32 decalindex = 0;
		decallist.begin();
		while(!decallist.end())
		{
			save_decalinfo_t* pdestdecal = reinterpret_cast<save_decalinfo_t*>(reinterpret_cast<byte*>(pheader) + pheader->decaldataoffset) + decalindex;
			save_decalinfo_t& decalinfo = decallist.get();

			pdestdecal->entityindex = decalinfo.entityindex;
			pdestdecal->identifier = decalinfo.identifier;
			pdestdecal->normal = decalinfo.normal;
			pdestdecal->origin = decalinfo.origin;
			pdestdecal->flags = decalinfo.flags;
			qstrcpy(pdestdecal->texturename, decalinfo.texturename);

			if(pheader->type == SAVE_TRANSITION && plandmarkorigin)
				Math::VectorSubtract(pdestdecal->origin, *plandmarkorigin, pdestdecal->origin);

			decallist.next();
			decalindex++;
		}
	}

	// Save screenshot
	if(pscreentexture)
	{
		pheader->screenshotwidth = width;
		pheader->screenshotheight = height;
		pheader->screenshotbpp = bpp;
		
		// Save screenshot
		Uint32 datasize = sizeof(byte)*width*height*bpp;
		pheader->screenshotoffset = currentdatapos;
		pheader->screenshotdatasize = datasize/8;
		currentdatapos += pheader->screenshotdatasize;

		pdest = pbuffer + pheader->screenshotoffset;
		rygCompress(pdest, pscreentexture, width, height, false);
		delete[] pscreentexture;
	}

	// Make sure the sizes match
	if(m_saveBufferSize != currentdatapos)
	{
		Con_EPrintf("%s - Mismatch between written(%d) and estimated size(%d) for save '%s'.\n", __FUNCTION__, currentdatapos, m_saveBufferSize, filename.c_str());
		delete[] pbuffer;
		return false;
	}

	// Set size
	pheader->filesize = m_saveBufferSize;

	// Try putting it in writer thread queue
	bool threadresult = false;
	bool incremental = (filename.find(0, "%number%") != CString::CSTRING_NO_POSITION) ? true : false;

	// Try saving with file writer thread if quick or auto
	if(type == SAVE_QUICK || type == SAVE_AUTO)
		threadresult = FWT_AddFile(filename.c_str(), pbuffer, m_saveBufferSize, incremental);

	// If not added, just write
	if(!threadresult)
	{
		CString finaloutputname;
		if(incremental)
		{
			Uint32 i = 0;
			while(true)
			{
				finaloutputname = filename;
				Int32 pos = finaloutputname.find(0, "%number%");
				if(pos == CString::CSTRING_NO_POSITION)
				{
					delete[] pbuffer;
					return false;
				}

				CString numstr;
				numstr << i;

				finaloutputname.erase(pos, 8);
				finaloutputname.insert(pos, numstr.c_str());
				if(!FL_FileExists(finaloutputname.c_str()))
					break;

				i++;
			}
		}
		else
		{
			// No incremental token
			finaloutputname = filename;
		}

		// Save the output
		bool result = FL_WriteFile(pbuffer, m_saveBufferSize, finaloutputname.c_str());
		if(!result)
		{
			Con_EPrintf("%s - Failed to write save file '%s'.\n", __FUNCTION__, finaloutputname.c_str());
			delete[] pbuffer;
			return false;
		}

		// Opportunistically clean saves folder
		if(type != SAVE_QUICK && type != SAVE_AUTO)
			CleanSaveFiles(filename.c_str());
	}

	// Delete buffer data
	delete[] pbuffer;

	return true;
}

//=============================================
// @brief Add a cvar to be saved
//
//=============================================
bool CSaveRestore::AddSavedCVar( CCVar* pcvar )
{
	const Char* pstrName = pcvar->GetName();
	for(Uint32 i = 0; i < m_savedCVarsArray.size(); i++)
	{
		const saved_cvar_t& check = m_savedCVarsArray[i];
		if(!qstrcmp(check.name, pstrName))
		{
			Con_EPrintf("Cvar '%s' already marked to be saved.\n", pcvar->GetName());
			return false;
		}
	}

	Uint32 length = qstrlen(pstrName);
	if(length >= SAVE_FILE_STRING_MAX_LENGTH)
	{
		Con_EPrintf("Cvar '%s' has a name longer(%d characters) than the max length(%d characters).\n", pcvar->GetName(), length, (SAVE_FILE_STRING_MAX_LENGTH-1));
		return false;
	}

	CString strValue;
	switch(pcvar->GetType())
	{
	case CVAR_STRING:
		strValue = pcvar->GetStrValue();
		break;
	default:
	case CVAR_FLOAT:
		strValue << pcvar->GetValue();
		break;
	}

	if(strValue.length() >= SAVE_FILE_STRING_MAX_LENGTH)
	{
		Con_EPrintf("Cvar '%s' has a value longer(%d characters) than the max length(%d characters).\n", pcvar->GetName(), strValue.length(), (SAVE_FILE_STRING_MAX_LENGTH-1));
		return false;
	}

	saved_cvar_t newCVar;
	newCVar.name = pstrName;
	newCVar.value = strValue;
	newCVar.type = pcvar->GetType();

	m_savedCVarsArray.push_back(newCVar);
	return true;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::SaveEntity( edict_t* pedict, save_edict_info_t& saveinfo )
{
	// Set entity index
	saveinfo.entityindex = pedict->entindex;
	saveinfo.isplayer = SV_IsHostClient(pedict->entindex);
	saveinfo.isworldspawn = SV_IsWorldSpawn(pedict->entindex);
	saveinfo.isglobalentity = svs.dllfuncs.pfnIsGlobalTransitioningEntity(pedict);

	// Save classname
	const Char* pstrClassName = SV_GetString(pedict->fields.classname);
	qstrcpy(saveinfo.classname, pstrClassName);

	// Save globalname if needed
	if(pedict->fields.globalname != NO_STRING_VALUE)
	{
		const Char* pstrGlobalName = SV_GetString(pedict->fields.globalname);
		qstrcpy(saveinfo.globalname, pstrGlobalName);

		// This is a global entity
		saveinfo.isglobalentity = true;

		// Set mins for adjustment
		saveinfo.mins = pedict->state.mins;
	}
	else
	{
		// Not a global entity
		saveinfo.isglobalentity = false;
	}

	// Save entity state data
	saveinfo.statedata_startindex = m_numEdictFields;
	svs.dllfuncs.pfnSaveEntityStateData(pedict, m_isTransitionSave);
	saveinfo.statedata_numfields = m_numEdictFields - saveinfo.statedata_startindex;

	// Save entity fields data
	saveinfo.fieldsdata_startindex = m_numEdictFields;
	svs.dllfuncs.pfnSaveEntityFieldsData(pedict, m_isTransitionSave);
	saveinfo.fieldsdata_numfields = m_numEdictFields - saveinfo.fieldsdata_startindex;

	// Tell server to prep the class-specific save fields for saving data
	svs.dllfuncs.pfnDispatchDeclareSaveFields(pedict);

	// Save entity class data
	saveinfo.classdata_startindex = m_numEdictFields;
	svs.dllfuncs.pfnSaveEntityClassData(pedict, m_isTransitionSave);
	saveinfo.classdata_numfields = m_numEdictFields - saveinfo.classdata_startindex;

	// Tell server to release the class-specific save fields
	svs.dllfuncs.pfnDispatchReleaseSaveFields(pedict);
}

//=============================================
// @brief
//
//=============================================
save_block_t& CSaveRestore::SaveToStringBuffer( const Char* pstrstring )
{
	// Get length
	Uint32 length = qstrlen(pstrstring);

	// Only save string if it has an actual length
	Int32 stringBufferOffset = NO_POSITION;
	if(length || m_isTransitionSave)
	{
		StringPositionMap_t::iterator it = m_savedStringPositionMap.find(pstrstring);
		if(it != m_savedStringPositionMap.end())
		{
			saved_string_t& savedString = m_savedStringsArray[it->second];
			stringBufferOffset = savedString.offset;
		}
		else
		{
			// Expand buffer if required
			if((m_stringBufferUsage + length) >= m_stringBufferSize)
			{
				void* pnewbuffer = Common::ResizeArray(m_pStringBuffer, sizeof(byte), m_stringBufferSize, BUFFER_ALLOC_SIZE);
				m_pStringBuffer = static_cast<byte*>(pnewbuffer);
				m_stringBufferSize += BUFFER_ALLOC_SIZE;
			}

			// Set offset
			stringBufferOffset = m_stringBufferUsage;
			m_stringBufferUsage += sizeof(byte)*length;

			// Save into optimized lookup array
			if(m_numSavedStrings == m_savedStringsArray.size())
				m_savedStringsArray.resize(m_savedStringsArray.size() + STRING_ARRAY_ALLOC_SIZE);

			// Save to fast lookup array too
			Int32 insertPosition = m_numSavedStrings;
			m_numSavedStrings++;

			saved_string_t& savedString = m_savedStringsArray[insertPosition];
			savedString.offset = stringBufferOffset;
			savedString.length = length;

			// Copy to buffer
			byte* pdest = m_pStringBuffer + stringBufferOffset;
			memcpy(pdest, pstrstring, sizeof(byte)*length);
			m_saveBufferSize += sizeof(byte)*length;

			// Add it to the map
			m_savedStringPositionMap.insert(std::pair<CString,Int32>(pstrstring, insertPosition));
		}
	}

	// Extend block buffer if needed
	if(m_numSaveStringBlocks == m_saveStringBlocksBuffer.size())
		m_saveStringBlocksBuffer.resize(m_saveStringBlocksBuffer.size() + BUFFER_ALLOC_SIZE);

	// Save a new block each time
	Int32 index = m_numSaveStringBlocks;
	save_block_t& newblock = m_saveStringBlocksBuffer[m_numSaveStringBlocks];
	m_numSaveStringBlocks++;

	newblock.index = index;
	newblock.dataoffset = stringBufferOffset;
	newblock.datasize = length;

	m_saveBufferSize += sizeof(save_block_t);
	return newblock;
}

//=============================================
// @brief
//
//=============================================
save_block_t& CSaveRestore::SaveToDataBuffer( const byte* pdata, Int32 size )
{
	// Expand buffer if required
	if((m_entityDataBufferUsage + size) >= m_entityDataBufferSize)
	{
		void* pnewbuffer = Common::ResizeArray(m_pEntityDataBuffer, sizeof(byte), m_entityDataBufferSize, BUFFER_ALLOC_SIZE);
		m_pEntityDataBuffer = static_cast<byte*>(pnewbuffer);
		m_entityDataBufferSize += BUFFER_ALLOC_SIZE;
	}

	// Extend buffer if needed
	if(m_numSaveDataBlocks == m_saveDataBlocksBuffer.size())
		m_saveDataBlocksBuffer.resize(m_saveDataBlocksBuffer.size() + BUFFER_ALLOC_SIZE);

	// Save a new one
	Int32 index = m_numSaveDataBlocks;
	save_block_t& newblock = m_saveDataBlocksBuffer[m_numSaveDataBlocks];
	m_numSaveDataBlocks++;

	newblock.index = index;
	newblock.dataoffset = m_entityDataBufferUsage;
	newblock.datasize = sizeof(byte)*size;

	// Copy to buffer
	byte* pdest = m_pEntityDataBuffer + newblock.dataoffset;
	memcpy(pdest, pdata, newblock.datasize);
	m_entityDataBufferUsage += newblock.datasize;

	m_saveBufferSize += newblock.datasize;
	m_saveBufferSize += sizeof(save_block_t);

	return newblock;
}

//=============================================
// @brief Saves a field to the saved strings array
//
// IMPORTANT: blocksize must be a reference, as it can refer to m_numSaveStringBlocks,
// which is updated here when we save with SaveToStringBuffer to the fields list, and
// the field value we are saving is also a string. This way the index is sure to be
// correct on the string we save.
//
//=============================================
save_field_t* CSaveRestore::SaveField( const Char* fieldname, entfieldtype_t fieldtype, const Uint32& blocksize )
{
	// Save the field's name to the string buffer
	const save_block_t& nameblock = SaveToStringBuffer(fieldname);

	// Extend buffer if needed
	if(m_numEdictFields == m_edictFieldsBuffer.size())
		m_edictFieldsBuffer.resize(m_edictFieldsBuffer.size() + BUFFER_ALLOC_SIZE);

	// Save the field
	save_field_t& field = m_edictFieldsBuffer[m_numEdictFields];
	m_numEdictFields++;

	field.blockindex = blocksize;
	field.fieldnameindex = nameblock.index;
	field.datatype = fieldtype;
	field.numblocks = 0;

	m_saveBufferSize += sizeof(save_field_t);
	return &field;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteBool( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		bool value = (*(pdata + offset)) == 1 ? true : false;
		offset += sizeof(bool);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(bool));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteByte( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	// With byte arrays, just write it out in one big chunk
	SaveToDataBuffer(pdata, sizeof(byte)*fieldsize);
	pfield->numblocks = 1;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteBitset( const Char* fieldname, const byte* pdata, Uint32 numberofbits, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	// With byte arrays, just write it out in one big chunk
	Uint32 bitsetByteCount = static_cast<Uint32>(SDL_ceil(static_cast<Float>(numberofbits) / static_cast<Float>(CBitSet::NB_BITS_IN_BYTE)));
	SaveToDataBuffer(pdata, sizeof(byte)*bitsetByteCount);
	pfield->numblocks = numberofbits;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteChar( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	// With byte arrays, just write it out in one big chunk
	SaveToDataBuffer(pdata, sizeof(Char)*fieldsize);
	pfield->numblocks = 1;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteInt16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Int16 value = Common::ByteToInt16(pdata + offset);
		offset += sizeof(Int16);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Int16));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteUint16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Uint16 value = Common::ByteToUint32(pdata + offset);
		offset += sizeof(Uint16);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Uint16));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteInt32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Int32 value = Common::ByteToInt32(pdata + offset);
		offset += sizeof(Int32);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Int32));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteUint32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Uint32 value = Common::ByteToUint32(pdata + offset);
		offset += sizeof(Uint32);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Uint32));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteInt64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Int64 value = Common::ByteToInt64(pdata + offset);
		offset += sizeof(Int64);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Int64));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteUint64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Uint64 value = Common::ByteToUint64(pdata + offset);
		offset += sizeof(Uint64);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Uint64));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteFloat( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Float value = Common::ByteToFloat(pdata + offset);
		offset += sizeof(Float);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Float));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteDouble( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Double value = Common::ByteToDouble(pdata + offset);
		offset += sizeof(Double);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Double));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteTime( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Double value = Common::ByteToDouble(pdata + offset);
		offset += sizeof(Double);

		// Subtract save time
		if(m_isTransitionSave && value)
			value -= m_timeBase;

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Double));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteString( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveStringBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		string_t stringindex = Common::ByteToUint32(pdata + offset);
		const Char* pstr = SV_GetString(stringindex);
		if(!pstr)
			pstr = "";

		offset += sizeof(string_t);

		SaveToStringBuffer(pstr);
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteRawString( const Char* fieldname, const byte* pdata, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveStringBlocks);
	if(!pfield)
		return;

	const Char* pstr = reinterpret_cast<const Char*>(pdata);
	if(!pstr || !qstrlen(pstr))
		SaveToStringBuffer("");
	else
		SaveToStringBuffer(pstr);

	pfield->numblocks++;
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteVector( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Vector value = *(reinterpret_cast<const Vector*>(pdata + offset));
		offset += sizeof(Vector);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Vector));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteCoord( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		Vector value = *(reinterpret_cast<const Vector*>(pdata + offset));
		offset += sizeof(Vector);

		// Subtract landmark if needed
		if(m_isTransitionSave)
			Math::VectorSubtract(value, m_landmarkOrigin, value);

		SaveToDataBuffer(reinterpret_cast<byte*>(&value), sizeof(Vector));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteEntindex( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	save_field_t* pfield = SaveField(fieldname, fieldtype, m_numSaveDataBlocks);
	if(!pfield)
		return;

	Uint32 offset = 0;
	for(Uint32 i = 0; i < fieldsize; i++)
	{
		entindex_t entindex = Common::ByteToInt32(pdata + offset);
		offset += sizeof(Int32);

		if(entindex != NO_ENTITY_INDEX)
		{
			Uint32 j = 0;
			for(; j < m_numSaveEdicts; j++)
			{
				const save_edict_info_t& edict = m_saveEdictsBuffer[j];
				if(edict.entityindex == entindex)
				{
					entindex = j;
					break;
				}
			}

			if(j == m_numSaveEdicts)
			{
				Con_VPrintf("Entity with index %d not found in save list.\n", entindex);
				entindex = -1;
			}
		}

		SaveToDataBuffer(reinterpret_cast<byte*>(&entindex), sizeof(Int32));
		pfield->numblocks++;
	}
}

//=============================================
// @brief
//
//=============================================
void CSaveRestore::WriteGlobalState( Uint32 index, const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state )
{
	if(index >= m_nbGlobalStates)
	{
		Con_EPrintf("Invalid global state index.\n");
		return;
	}

	save_global_t* pglobal = &m_pGlobalStatesBuffer[index];
	qstrcpy(pglobal->name, pstrglobalname);
	qstrcpy(pglobal->levelname, pstrlevelname);
	pglobal->state = state;
}

//=============================================
// @brief
//
//=============================================
void Save_WriteBool( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteBool(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteByte( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteByte(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteBitset( const Char* fieldname, const byte* pdata, Uint32 numberofbits, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteBitset(fieldname, pdata, numberofbits, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteChar( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteChar(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteUint16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteUint16(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteInt16( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteInt16(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteUint32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteUint32(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteInt32( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteInt32(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteUint64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteUint64(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteInt64( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteInt64(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteFloat( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteFloat(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteDouble( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteDouble(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteTime( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteTime(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteString( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteString(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteRawString( const Char* fieldname, const byte* pdata, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteRawString(fieldname, pdata, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteVector( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteVector(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteCoord( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteCoord(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteEntindex( const Char* fieldname, const byte* pdata, Uint32 fieldsize, entfieldtype_t fieldtype )
{
	gSaveRestore.WriteEntindex(fieldname, pdata, fieldsize, fieldtype);
}

//=============================================
// @brief
//
//=============================================
void Save_WriteGlobalState( Uint32 index, const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state )
{
	gSaveRestore.WriteGlobalState(index, pstrglobalname, pstrlevelname, state);
}
