/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENTITYDATA_H
#define ENTITYDATA_H

struct keyvalue_t
{
	keyvalue_t()
	{
		memset(keyname, 0, sizeof(keyname));
		memset(value, 0, sizeof(value));
	}

	Char keyname[MAX_PARSE_LENGTH];
	Char value[MAX_PARSE_LENGTH];
};

struct entitydata_t
{
	~entitydata_t()
	{
		for(Uint32 i = 0; i < values.size(); i++)
			delete values[i];
	}

	CString classname;
	CArray<keyvalue_t*> values;
};

//=============================================
// @brief
//
//=============================================
inline const Char* ValueForKey( const entitydata_t& entity, const Char* pstrKeyName )
{
	for(Uint32 i = 0; i < entity.values.size(); i++)
	{
		if(!qstrcmp(entity.values[i]->keyname, pstrKeyName))
			return entity.values[i]->value;
	}

	return nullptr;
};
#endif //ENTITYDATA_H