/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCCLIPECONOMY_H
#define FUNCCLIPECONOMY_H

//=============================================
//
//=============================================
class CFuncClipEconomy : public CBaseEntity
{
public:
	enum
	{
		FL_TAKE_ANGLES = (1<<0)
	};
public:
	explicit CFuncClipEconomy( edict_t* pedict );
	virtual ~CFuncClipEconomy( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //FUNCCLIPECONOMY_H