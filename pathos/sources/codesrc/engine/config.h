/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CONFIG_H
#define CONFIG_H

enum cf_status_t
{
	CONF_ERR_NONE = 0,
	CONF_ERR_FIELD_MISSING,
	CONF_ERR_GROUP_MISSING,
	CONF_ERR_TYPE_MISMATCH
};

enum field_type_t
{
	CONF_FIELD_UNDEFINED = -1,
	CONF_FIELD_INT,
	CONF_FIELD_FLOAT,
	CONF_FIELD_STRING,
	CONF_FIELD_KEYBIND
};

enum group_type_t
{
	CONF_GRP_UNDEFINED = -1,
	CONF_GRP_SYSTEM,
	CONF_GRP_USERCONFIG
};

struct conf_field_t
{
	conf_field_t():
		value_int(0),
		value_fl(0),
		archive(false),
		type(CONF_FIELD_UNDEFINED)
	{}

	CString name;

	Int32 value_int;
	Float value_fl;
	CString value_str;

	bool archive;
	field_type_t type;
};

struct conf_group_t
{
	conf_group_t():
		type(CONF_GRP_UNDEFINED),
		updatefile(false)
	{}

	CString name;
	CString filename;
	group_type_t type;
	bool updatefile;

	CArray<conf_field_t> fields;
};

/*
=================================
-Class: CConfig
-Description:

=================================
*/
class CConfig
{
public:
	// System config filename
	static const Char SYSTEM_CONFIG_FILENAME[];
	// System config filename
	static const Char USER_CONFIG_FILENAME[];
	// Name of the group that holds keybinds
	static const Char USER_CONFIG_GRP_NAME[];

public:
	CConfig( void );
	~CConfig( void );

	void Init( void );

	void PostThink( void );
	const cf_status_t GetStatus( void ) const;

public:
	conf_group_t* FindGroup( const Char* name );
	conf_group_t* CreateGroup( const Char* name, const Char* pstrFilename, group_type_t type );
	conf_field_t* CreateField( conf_group_t* pgroup, const Char* name, field_type_t type );

	static bool DeleteField( conf_group_t* pgroup, const Char* name );
	bool DeleteField( const Char* grpName, const Char* name );

	bool SetValue( conf_group_t* pgroup, const Char* field, Int32 value, bool archive = false );
	bool SetValue( const Char* grpName, const Char* field, Int32 value, bool archive = false );
	bool SetValue( conf_group_t* pgroup, const Char* field, Float value, bool archive = false );
	bool SetValue( const Char* grpName, const Char* field, Float value, bool archive = false );
	bool SetValue( conf_group_t* pgroup, const Char* field, const Char* value, bool archive = false, field_type_t type = CONF_FIELD_STRING );
	bool SetValue( const Char* grpName, const Char* field, const Char* value, bool archive = false, field_type_t type = CONF_FIELD_STRING );

	const Int32 GetInt( conf_group_t* pgroup, const Char* field );
	const Float GetFloat( conf_group_t* pgroup, const Char* field );
	const Char* GetString( conf_group_t* pgroup, const Char* field );

public:
	void CmdExecuteScript( void );

private:
	void WriteConfigFile( conf_group_t* pgroup, const Char* pstrFilename ) const;
	void ReadSystemConfigFile( void );
	static void ReadUserConfigFile( const Char* pstrFilename );

private:
	static void FlagRewrite( conf_group_t* pgroup );
	void ResetStatus( void );

private:
	conf_field_t* FindField( conf_group_t* pgroup, const Char* field );

private:
	// Error status of config class
	cf_status_t m_configStatus;

	// Available config groups
	CArray<conf_group_t*> m_configGroups;
};
extern CConfig gConfig;
#endif