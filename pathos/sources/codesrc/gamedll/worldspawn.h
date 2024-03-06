/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WORLDSPAWN_H
#define WORLDSPAWN_H

//=============================================
//
//=============================================
class CWorldSpawn : public CBaseEntity
{
public:
	explicit CWorldSpawn( edict_t* pedict );
	virtual ~CWorldSpawn();

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool Restore( void ) override;
	virtual void InitEntity( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual bool IsWorldSpawn( void ) override { return true; }

protected:
	string_t m_skyboxName;
	string_t m_chapterTitle;
};

#endif //LIGHTENVIRONMENT_H