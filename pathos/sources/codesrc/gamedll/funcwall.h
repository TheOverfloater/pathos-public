/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCWALL_H
#define FUNCWALL_H

//=============================================
//
//=============================================
class CFuncWall : public CBaseEntity
{
public:
	enum
	{
		FL_TAKE_ANGLES = (1<<3)
	};
public:
	explicit CFuncWall( edict_t* pedict );
	virtual ~CFuncWall( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //FUNCWALL_H