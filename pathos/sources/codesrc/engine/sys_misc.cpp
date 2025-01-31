/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "enginestate.h"
#include "window.h"
#include "common.h"
#include "file.h"
#include "cvar.h"
#include "cl_main.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_text.h"
#include "system.h"
#include "r_menu.h"

#include "uielements.h"
#include "uimanager.h"
#include "config.h"
#include "textschemas.h"

// Default font schema name
static const Char DEFAULT_FONT_SCHEMA_NAME[] = "DefaultFont";

//=============================================
// @brief Returns the optimal frame limit
//
// @return The frame limit to be used for frame capping
//=============================================
bool Sys_LoadDefaultFont( const Char* pstr )
{
	const font_set_t* pDefaultSet = gTextSchemas.GetSchemaFontSet(DEFAULT_FONT_SCHEMA_NAME);
	if(!pDefaultSet)
		pDefaultSet = gText.LoadFont(DEFAULT_FONTSET_NAME, DEFAULT_FONT_SIZE);

	if(!pDefaultSet)
	{
		Sys_ErrorPopup("%s - Failed to load default font set.\n", __FUNCTION__);
		return false;
	}
	
	gText.SetDefaultFont(pDefaultSet);
	return true;
}


//=============================================
// @brief Loads the gameinfo file
//
// @return Success status
//=============================================
bool Sys_LoadGameInfo( CArray<CString>* argsArray )
{
	const byte* pfile = FL_LoadFile(GAMEINFO_FILENAME);
	if(!pfile)
	{
		Sys_ErrorPopup("Failed to load %s.\n", GAMEINFO_FILENAME);
		return false;
	}

	// Parse the fields
	CString token;
	CString value;
	CString line;
	const Char *pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		// Read line by line
		pstr = Common::ReadLine(pstr, line);
		if(line.empty())
			continue;

		if(!qstrncmp(line, "//", 2))
			continue;

		// Read in the field name
		const Char* ppstr = Common::Parse(line.c_str(), token);
		if(!ppstr)
		{
			Con_EPrintf("%s - Missing value token in %s.\n", __FUNCTION__, GAMEINFO_FILENAME);
			break;
		}

		if(!qstrcmp(token, "$launchargs"))
		{
			while(ppstr)
			{
				// Arguments might be in quotes
				CString argtoken;
				ppstr = Common::Parse(ppstr, value);

				const Char* _ppstr = value.c_str();
				while(_ppstr)
				{
					_ppstr = Common::Parse(_ppstr, argtoken);
					if(argtoken.empty())
						break;

					argsArray->push_back(argtoken);
				}
			}
		}
		else
		{
			// Read the value in
			ppstr = Common::Parse(ppstr, value);

			// Match it with a field name
			if(!qstrcmp(token, "$title"))
			{
				// Set game title
				ens.gametitle = value;
			}
			else if(!qstrcmp(token, "$startmap"))
			{
				// Set start map
				ens.startmap = value;
			}
			else if(!qstrcmp(token, "$stencilbits"))
			{
				if(!Common::IsNumber(value))
				{
					Con_Printf("%s - %s value '%s' is not a valid number", __FUNCTION__, token.c_str(), value.c_str());
					continue;
				}

				if(qstrcmp(value, "0") && qstrcmp(value, "1"))
				{
					Con_Printf("%s - %s value is not valid, only 0 or 1 allowed.", __FUNCTION__, token.c_str(), value.c_str());
					continue;
				}

				gConfig.SetValue("Display", "StencilBits", value.c_str());
			}
			else if(!qstrcmp(token, "$max_edicts"))
			{
				if(!Common::IsNumber(value))
				{
					Con_Printf("%s - %s value '%s' is not a valid number", __FUNCTION__, token.c_str(), value.c_str());
					continue;
				}

				if(ens.arg_max_edicts != 0)
				{
					Con_Printf("%s - %s setting ignored, as the '-num_edicts' launch argument was set.\n", __FUNCTION__, token.c_str());
					continue;
				}

				Int32 integervalue = SDL_atoi(value.c_str());
				if(integervalue <= 0 || integervalue > MAX_SERVER_ENTITIES)
				{
					Con_Printf("%s - %s setting %d invalid, only 0 to %d allowed.\n", __FUNCTION__, token.c_str(), MAX_SERVER_ENTITIES);
					continue;
				}

				ens.arg_max_edicts = integervalue;
			}
			else
			{
				Con_Printf("%s - Unknown field %s in %s.\n", __FUNCTION__, token.c_str(), GAMEINFO_FILENAME);
			}
		}
	}
	
	// Free the file
	FL_FreeFile(pfile);

	// Verify that the required values were set
	if(ens.gametitle.empty())
	{
		Sys_ErrorPopup("$title not set in %s.\n", GAMEINFO_FILENAME);
		return false;
	}

	if(ens.startmap.empty())
	{
		Sys_ErrorPopup("$startmap not set in %s.\n", GAMEINFO_FILENAME);
		return false;
	}

	return true;
}

//=============================================
// @brief Returns the launch arg count
//
// @return Launch args count
//=============================================
Uint32 Sys_LaunchArgc( void )
{
	return ens.launchargs.size();
}

//=============================================
// @brief Returns the launch arg count
//
// @return Launch args count
//=============================================
const Char* Sys_LaunchArgv( Uint32 index )
{
	assert(index < ens.launchargs.size());
	return ens.launchargs[index].c_str();
}

//=============================================
// @brief Returns the launch arg count
//
// @return Launch args count
//=============================================
Int32 Sys_CheckLaunchArgs( const Char* pstrArg )
{
	if(ens.launchargs.empty())
		return NO_POSITION;

	for(Uint32 i = 0; i < ens.launchargs.size(); i++)
	{
		if(!qstrcmp(pstrArg, ens.launchargs[i]))
			return i;
	}

	return NO_POSITION;
}

//=============================================
// @brief
//
// @return
//=============================================
bool Sys_IsGameControlActive( void )
{
	if(gMenu.IsActive())
		return false;

	if(gUIManager.HasActiveWindows())
		return false;
	
	if(cls.dllfuncs.pfnIsInputOverridden())
		return false;

	return true;
}

//=============================================
// @brief
//
// @return
//=============================================
void Sys_AddTempFile( const Char* pstrFilepath, rs_level_t level )
{
	ens.tempfileslist.begin();
	while(!ens.tempfileslist.end())
	{
		tempfile_t& tmp = ens.tempfileslist.get();
		if(!qstrcmp(tmp.filepath, pstrFilepath))
			return;

		ens.tempfileslist.next();
	}

	tempfile_t newfile;
	newfile.filepath = pstrFilepath;
	newfile.level = level;

	ens.tempfileslist.add(newfile);
}

//=============================================
// @brief
//
// @return
//=============================================
void Sys_DeleteTempFiles( rs_level_t level )
{
	if(ens.tempfileslist.empty())
		return;

	ens.tempfileslist.begin();
	while(!ens.tempfileslist.end())
	{
		tempfile_t& tmp = ens.tempfileslist.get();
		if(tmp.level <= level)
		{
			if(!FL_DeleteFile(tmp.filepath.c_str()))
				Con_EPrintf("%s - Failed to delete temporary file '%s'.\n", __FUNCTION__, tmp.filepath.c_str());

			ens.tempfileslist.remove(ens.tempfileslist.get_link());
			ens.tempfileslist.next();
			continue;
		}

		ens.tempfileslist.next();
	}
}