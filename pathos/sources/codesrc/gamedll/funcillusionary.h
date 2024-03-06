/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCILLUSIONARY_H
#define FUNCILLUSIONARY_H

//=============================================
//
//=============================================
class CFuncIllusionary : public CBaseEntity
{
public:
	enum
	{
		FL_TAKE_ANGLES = (1<<3)
	};
public:
	explicit CFuncIllusionary( edict_t* pedict );
	virtual ~CFuncIllusionary( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual bool IsFuncIllusionaryEntity( void ) const override { return true; }
};
#endif //FUNCWALL_H