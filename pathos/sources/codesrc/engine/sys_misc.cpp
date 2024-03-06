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

//=============================================
// @brief Returns the optimal frame limit
//
// @return The frame limit to be used for frame capping
//=============================================
bool Sys_LoadDefaultFont( const Char* pstr )
{
	const font_set_t* pDefaultSet = nullptr;
	if(pstr && qstrlen(pstr))
		pDefaultSet = gText.LoadFont(pstr, DEFAULT_FONT_SIZE);

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
	const byte* pfile = FL_LoadFile("gameinfo.cfg");
	if(!pfile)
	{
		Con_EPrintf("Failed to load gameinfo.cfg.\n");
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
			Con_EPrintf("Missing value token in gameinfo.cfg.\n");
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
					Con_Printf("$stencilbits value '%s' is not a valid number", value.c_str());
					continue;
				}

				if(qstrcmp(value, "0") && qstrcmp(value, "1"))
				{
					Con_Printf("$stencilbits value '%s' is not valid, only 0 or 1 allowed.", value.c_str());
					continue;
				}

				gConfig.SetValue("Display", "StencilBits", value.c_str());
			}
			else
			{
				Con_Printf("Unknown field %s in gameinfo.cfg.\n", token.c_str());
			}
		}
	}
	
	// Free the file
	FL_FreeFile(pfile);

	// Verify that the required values were set
	if(ens.gametitle.empty())
	{
		Sys_ErrorPopup("$title not set in gameinfo.cfg.\n");
		return false;
	}

	if(ens.startmap.empty())
	{
		Sys_ErrorPopup("$startmap not set in gameinfo.cfg.\n");
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
		return -1;

	for(Uint32 i = 0; i < ens.launchargs.size(); i++)
	{
		if(!qstrcmp(pstrArg, ens.launchargs[i]))
			return i;
	}

	return -1;
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