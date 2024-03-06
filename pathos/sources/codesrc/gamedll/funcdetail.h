/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCDETAIL_H
#define FUNCDETAIL_H

//=============================================
//
//=============================================
class CFuncDetail : public CBaseEntity
{
public:
	enum
	{
		FL_TAKE_ANGLES = (1<<0)
	};
public:
	explicit CFuncDetail( edict_t* pedict );
	virtual ~CFuncDetail( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //FUNCWALL_H