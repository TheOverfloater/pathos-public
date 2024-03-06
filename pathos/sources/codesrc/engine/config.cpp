/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>

#include "includes.h"
#include "system.h"
#include "config.h"
#include "file.h"
#include "commands.h"
#include "input.h"
#include "enginestate.h"

// System config filename
const Char CConfig::SYSTEM_CONFIG_FILENAME[] = "system.cfg";
// System config filename
const Char CConfig::USER_CONFIG_FILENAME[] = "config.cfg";
// Name of the group that holds keybinds
const Char CConfig::USER_CONFIG_GRP_NAME[] = "UserConfig";

//
// Console commands
//
void Cmd_ExecScript( void ) { gConfig.CmdExecuteScript(); }

// Class definition
CConfig gConfig;

//=============================================
// Class: CConfig
// Function: CConfig
//=============================================
CConfig::CConfig( void ):
	m_configStatus(CONF_ERR_NONE)
{
}

//=============================================
// Class: CConfig
// Function: CConfig
//=============================================
CConfig::~CConfig( void )
{
	for(Uint32 i = 0; i < m_configGroups.size(); i++)
		delete m_configGroups[i];
}

//=============================================
// Class: CConfig
// Function: Init
//=============================================
void CConfig::Init( void )
{
	ReadSystemConfigFile();

	// Make sure the basic userconfig group is present
	gConfig.CreateGroup(USER_CONFIG_GRP_NAME, USER_CONFIG_FILENAME, CONF_GRP_USERCONFIG);

	// Create any commands
	gCommands.CreateCommand("exec", Cmd_ExecScript, "Executes a config file");
}

//=============================================
// Class: CConfig
// Function: PostThink
//=============================================
void CConfig::PostThink( void )
{
	for(Uint32 i = 0; i < m_configGroups.size(); i++)
	{
		if(!m_configGroups[i]->updatefile)
			continue;

		WriteConfigFile(m_configGroups[i], m_configGroups[i]->filename.c_str());
	}
}

//=============================================
// Class: CConfig
// Function: FlagRewrite
//=============================================
void CConfig::FlagRewrite( conf_group_t* pgroup )
{
	pgroup->updatefile = true;
}

//=============================================
// Class: CConfig
// Function: GetStatus
//=============================================
const cf_status_t CConfig::GetStatus( void ) const
{
	return m_configStatus;
}

//=============================================
// Class: CConfig
// Function: ResetStatus
//=============================================
void CConfig::ResetStatus( void )
{
	m_configStatus = CONF_ERR_NONE;
}

//=============================================
// Class: CConfig
// Function: SetValue
//=============================================
bool CConfig::SetValue( conf_group_t* pgroup, const Char* field, Int32 value, bool archive )
{
	ResetStatus();

	// Find the field entry
	conf_field_t *confField = FindField(pgroup, field);
	if(!confField)
	{
		confField = CreateField(pgroup, field, CONF_FIELD_INT);
		ResetStatus();
	}

	if(confField->type != CONF_FIELD_INT)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return false;
	}

	// Only set if it's different
	if(confField->value_int == value)
		return true;

	confField->value_int = value;
	confField->archive = archive;
	confField->type = CONF_FIELD_INT;

	if(!pgroup->updatefile)
		pgroup->updatefile = archive;

	return true;
}

//=============================================
// Class: CConfig
// Function: SetValue
//=============================================
bool CConfig::SetValue( const Char* grpName, const Char* field, Int32 value, bool archive )
{
	ResetStatus();
	conf_group_t* pgrp = FindGroup(grpName);
	if(!pgrp)
	{
		m_configStatus = CONF_ERR_GROUP_MISSING;
		return false;
	}

	return SetValue(pgrp, field, value, archive);
}

//=============================================
// Class: CConfig
// Function: SetValue
//=============================================
bool CConfig::SetValue( conf_group_t* pgroup, const Char* field, Float value, bool archive )
{
	ResetStatus();

	// Find the field entry
	conf_field_t *confField = FindField(pgroup, field);
	if(!confField)
	{
		ResetStatus();
		confField = CreateField(pgroup, field, CONF_FIELD_FLOAT);
	}

	if(confField->type != CONF_FIELD_FLOAT)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return false;
	}

	// Only set if it's different
	if(confField->value_fl == value)
		return true;

	confField->value_fl = value;
	confField->archive = archive;
	confField->type = CONF_FIELD_FLOAT;

	if(!pgroup->updatefile)
		pgroup->updatefile = archive;

	return true;
}

//=============================================
// Class: CConfig
// Function: SetValue
//=============================================
bool CConfig::SetValue( const Char* grpName, const Char* field, Float value, bool archive )
{
	ResetStatus();
	conf_group_t* pgrp = FindGroup(grpName);
	if(!pgrp)
	{
		m_configStatus = CONF_ERR_GROUP_MISSING;
		return false;
	}

	return SetValue(pgrp, field, value, archive);
}

//=============================================
// Class: CConfig
// Function: SetValue
//=============================================
bool CConfig::SetValue( conf_group_t* pgroup, const Char* field, const Char* value, bool archive, field_type_t type )
{
	ResetStatus();

	if(type != CONF_FIELD_KEYBIND && type != CONF_FIELD_STRING)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return false;
	}

	// Find the field entry
	conf_field_t *confField = FindField(pgroup, field);
	if(!confField)
	{
		ResetStatus();
		confField = CreateField(pgroup, field, type);
	}

	if(type == CONF_FIELD_STRING && confField->type != CONF_FIELD_STRING
		|| type == CONF_FIELD_KEYBIND && confField->type != CONF_FIELD_KEYBIND)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return false;
	}

	// Only set if it's different
	if(!qstrcmp(confField->value_str, value))
		return true;

	confField->value_str = value;
	confField->archive = archive;
	confField->type = type;

	if(!pgroup->updatefile)
		pgroup->updatefile = archive;

	return true;
}

//=============================================
// Class: CConfig
// Function: SetValue
//=============================================
bool CConfig::SetValue( const Char* grpName, const Char* field, const Char* value, bool archive, field_type_t type )
{
	ResetStatus();
	conf_group_t* pgrp = FindGroup(grpName);
	if(!pgrp)
	{
		m_configStatus = CONF_ERR_GROUP_MISSING;
		return false;
	}

	return SetValue(pgrp, field, value, archive, type);
}

//=============================================
// Class: CConfig
// Function: DeleteField
//=============================================
bool CConfig::DeleteField( conf_group_t* pgroup, const Char* name )
{
	for(Uint32 i = 0; i < pgroup->fields.size(); i++)
	{
		if(!qstrcicmp(pgroup->fields[i].name, name))
		{
			pgroup->fields.erase(i);
			FlagRewrite(pgroup);
			return true;
		}
	}

	return false;
}

//=============================================
// Class: CConfig
// Function: DeleteField
//=============================================
bool CConfig::DeleteField( const Char* grpName, const Char* name )
{
	ResetStatus();
	conf_group_t* pgrp = FindGroup(grpName);
	if(!pgrp)
	{
		m_configStatus = CONF_ERR_GROUP_MISSING;
		return false;
	}

	return DeleteField(pgrp, name);
}

//=============================================
// Class: CConfig
// Function: GetInt
//=============================================
const Int32 CConfig::GetInt( conf_group_t* pgroup, const Char* field )
{
	ResetStatus();

	// Find the field entry
	conf_field_t *confField = FindField(pgroup, field);
	if(!confField)
		return 0;

	if(confField->type != CONF_FIELD_INT)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return 0;
	}

	return confField->value_int;
}

//=============================================
// Class: CConfig
// Function: GetFloat
//=============================================
const Float CConfig::GetFloat( conf_group_t* pgroup, const Char* field )
{
	ResetStatus();

	// Find the field entry
	conf_field_t *confField = FindField(pgroup, field);
	if(!confField)
		return 0;

	if(confField->type != CONF_FIELD_FLOAT)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return 0;
	}

	return confField->value_fl;
}

//=============================================
// Class: CConfig
// Function: GetString
//=============================================
const Char* CConfig::GetString( conf_group_t* pgroup, const Char* field )
{
	ResetStatus();

	// Find the field entry
	conf_field_t *confField = FindField(pgroup, field);
	if(!confField)
		return nullptr;

	if(confField->type != CONF_FIELD_STRING)
	{
		m_configStatus = CONF_ERR_TYPE_MISMATCH;
		return 0;
	}

	return confField->value_str.c_str();
}

//=============================================
// Class: CConfig
// Function: FindGroup
//=============================================
conf_group_t* CConfig::FindGroup( const Char* name )
{
	for(Uint32 i = 0; i < m_configGroups.size(); i++)
	{
		if(!qstrcicmp(name, m_configGroups[i]->name))
			return m_configGroups[i];
	}

	return nullptr;
}

//=============================================
// Class: CConfig
// Function: FindField
//=============================================
conf_field_t* CConfig::FindField( conf_group_t* pgroup, const Char* field )
{
	for(Uint32 i = 0; i < pgroup->fields.size(); i++)
	{
		if(!qstrcicmp(pgroup->fields[i].name, field))
			return &pgroup->fields[i];
	}

	m_configStatus = CONF_ERR_FIELD_MISSING;
	return nullptr;
}

//=============================================
// Class: CConfig
// Function: CreateGroup
//=============================================
conf_group_t* CConfig::CreateGroup( const Char* name, const Char* pstrFilename, group_type_t type )
{
	// No such group, make a new one
	conf_group_t* newGroup = new conf_group_t;
	newGroup->name = name;
	newGroup->type = type;
	newGroup->filename = pstrFilename;
	newGroup->updatefile = false;

	m_configGroups.push_back(newGroup);
	return m_configGroups[m_configGroups.size()-1];
}

//=============================================
// Class: CConfig
// Function: CreateField
//=============================================
conf_field_t* CConfig::CreateField( conf_group_t* pgroup, const Char* name, field_type_t type )
{
	conf_field_t newField;
	newField.name = name;
	newField.type = type;

	pgroup->fields.push_back(newField);
	return &pgroup->fields[pgroup->fields.size()-1];
}

//=============================================
// Class: CConfig
// Function: WriteConfigFile
//=============================================
void CConfig::WriteConfigFile( conf_group_t* pgroup, const Char* pstrFilename ) const
{
	CString ssOutFile;

	if(pgroup->type == CONF_GRP_USERCONFIG)
	{
		// Print the header
		ssOutFile << "// This file is overwritten whenever you change your user settings in the game." << NEWLINE;
		ssOutFile << "// Add custom configurations to the file \"userconfig.cfg\"." << NEWLINE << NEWLINE;

		// Make sure the config is cleared
		ssOutFile << "unbindall" << NEWLINE;
	}

	// Find every config group that uses this file
	for(Uint32 i = 0; i < m_configGroups.size(); i++)
	{
		conf_group_t* confGroup = m_configGroups[i];
		if(qstrcicmp(confGroup->filename, pstrFilename))
			continue;

		for(Uint32 j = 0; j < confGroup->fields.size(); j++)
		{
			conf_field_t* confField = &confGroup->fields[j];

			CString ssOut;
			if(confGroup->type == CONF_GRP_SYSTEM)
			{
				// System config fields are more unique
				switch(confField->type)
				{
					case CONF_FIELD_INT: ssOut << confGroup->name << "." << confField->name << " = " << confField->value_int << NEWLINE; break;
					case CONF_FIELD_FLOAT: ssOut << confGroup->name << "." << confField->name << " = " << confField->value_fl << NEWLINE; break;
					case CONF_FIELD_STRING: ssOut << confGroup->name << "." << confField->name << " = \"" << confField->value_str << "\"" << NEWLINE; break;
					default:
						{
							Con_EPrintf("Invalid system config field type %d for field %s.\n", confField->type, confField->name.c_str());
							continue;
						}
						break;
				}
			}
			else if(confGroup->type == CONF_GRP_USERCONFIG)
			{
				// Userconfig entries are a series of console commands
				switch(confField->type)
				{
					case CONF_FIELD_INT: ssOut << confField->name << " \"" << confField->value_int << "\"" << NEWLINE; break;
					case CONF_FIELD_FLOAT: ssOut << confField->name << " \"" << confField->value_fl << "\"" << NEWLINE; break;
					case CONF_FIELD_STRING: ssOut << confField->name << " \"" << confField->value_str << "\"" << NEWLINE; break;
					case CONF_FIELD_KEYBIND: ssOut << "bind \"" << confField->name << "\" \"" << confField->value_str << "\"" << NEWLINE; break;
					default:
						{
							Con_EPrintf("Invalid system config field type %d for field %s.\n", confField->type, confField->name.c_str());
							continue;
						}
						break;
				}
			}
			ssOutFile << ssOut;
		}

		confGroup->updatefile = false;
	}

	if(pgroup->type == CONF_GRP_SYSTEM)
	{
		if(!FL_WriteFileRoot(reinterpret_cast<const byte*>(ssOutFile.c_str()), ssOutFile.length(), pstrFilename))
			Con_EPrintf("Failed to write config file.\n");
	}
	else
	{
		if(!FL_WriteFile(reinterpret_cast<const byte*>(ssOutFile.c_str()), ssOutFile.length(), pstrFilename))
			Con_EPrintf("Failed to write config file.\n");
	}
}

//=============================================
// Class: CConfig
// Function: ReadSystemConfigFile
//=============================================
void CConfig::ReadSystemConfigFile( void )
{
	Uint32 fileSize = 0;
	const byte *confFile = FL_LoadFileFromRoot(SYSTEM_CONFIG_FILENAME, &fileSize);

	if(!confFile || !fileSize)
		return;

	CString strGroupName;
	CString strFieldName;
	CString strValue;

	Uint32 numLines = 0;
	const Char *pchar = reinterpret_cast<const Char *>(confFile);
	while(pchar)
	{
		// Read the entire line in
		CString line;
		pchar = Common::ReadLine(pchar, line);
		if(line.empty())
			break;

		// Get group
		const Char* curParse = Common::Parse(line.c_str(), strGroupName, ".");
		if(!curParse || strGroupName.empty())
			break;

		// Skip the dot
		curParse++;

		// Get field
		curParse = Common::Parse(curParse, strFieldName);
		if(!curParse || strFieldName.empty())
		{
			Con_EPrintf("Error: Line %d is incomplete in %s.\n", numLines, SYSTEM_CONFIG_FILENAME);
			FL_FreeFile(confFile);
			return;
		}

		// Skip the equal character
		while((SDL_isspace(*curParse) || *curParse == '=') && *curParse != '\0')
			curParse++;

		// Get value
		curParse = Common::Parse(curParse, strValue);
		if(strValue.empty())
		{
			Con_EPrintf("Error: Line %d is incomplete in %s.\n", numLines, SYSTEM_CONFIG_FILENAME);
			FL_FreeFile(confFile);
			return;
		}

		// Default to int
		field_type_t stype = CONF_FIELD_INT;
		for(Uint32 i = 0; i < strValue.length(); i++)
		{
			if(!SDL_isdigit((Uchar)strValue[i]) && (strValue[i] != '.' || stype == CONF_FIELD_FLOAT))
			{
				stype = CONF_FIELD_STRING;
				break;
			}
			else if(strValue[i] == '.')
			{
				// Dot indicates float
				stype = CONF_FIELD_FLOAT;
			}
		}

		// Create the group if needed
		conf_group_t* pgrp = FindGroup(strGroupName.c_str());
		if(!pgrp)
		{
			ResetStatus();
			pgrp = CreateGroup(strGroupName.c_str(), SYSTEM_CONFIG_FILENAME, CONF_GRP_SYSTEM);
		}

		if(stype == CONF_FIELD_INT)
			SetValue(pgrp, strFieldName.c_str(), SDL_atoi(strValue.c_str()), true);
		else if(stype == CONF_FIELD_FLOAT)
			SetValue(pgrp, strFieldName.c_str(), (Float)SDL_atof(strValue.c_str()), true);
		else if(stype == CONF_FIELD_STRING)
			SetValue(pgrp, strFieldName.c_str(), strValue.c_str(), true);

		numLines++;
	}

	FL_FreeFile(confFile);
}

//=============================================
// Class: CConfig
// Function: ReadUserConfigFile
//=============================================
void CConfig::ReadUserConfigFile( const Char* pstrFilename )
{
	Uint32 fileSize = 0;
	const byte *confFile = FL_LoadFile(pstrFilename, &fileSize);
	if(!confFile)
		return;

	const Char *pchar = reinterpret_cast<const Char *>(confFile);
	while(pchar)
	{
		// Read the entire line in
		CString line;
		pchar = Common::ReadLine(pchar, line);
		if(line.empty())
			continue;

		if(!qstrncmp(line, "//", 2))
			continue;

		// Check that it's not an overwritten cvar
		if(!ens.overwrittencvars.empty())
		{
			CString token;
			Common::Parse(line.c_str(), token);

			Uint32 i = 0;
			for(; i < ens.overwrittencvars.size(); i++)
			{
				if(!qstrcmp(ens.overwrittencvars[i], token.c_str()))
					break;
			}

			// Skip if it was overwritten by the launch args
			if(i != ens.overwrittencvars.size())
				continue;
		}

		gCommands.AddCommand(line.c_str());
	}

	// Execute all the commands
	gCommands.ExecuteCommands();

	FL_FreeFile(confFile);
}

//=============================================
// Class: CConfig
// Function: CmdExecuteScript
//=============================================
void CConfig::CmdExecuteScript( void )
{
	if(gCommands.Cmd_Argc() != 2)
	{
		const Char* pstrFnName = gCommands.Cmd_Argv(0);
		Con_Printf("%s usage: <config file name>\n", pstrFnName);
		return;
	}

	ReadUserConfigFile(gCommands.Cmd_Argv(1));
}