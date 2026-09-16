/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCBRUSHCOPY_H
#define FUNCBRUSHCOPY_H

//=============================================
//
//=============================================
class CFuncBrushCopy : public CBaseEntity
{
public:
	enum
	{
		FL_TAKE_ANGLES = (1<<0)
	};
public:
	explicit CFuncBrushCopy( edict_t* pedict );
	virtual ~CFuncBrushCopy( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual void InitEntity( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual bool CanEntityBeParented( void ) const override { return true; }
	virtual bool IsBrushModel( void ) const override;
};
#endif //FUNCBRUSHCOPY_H