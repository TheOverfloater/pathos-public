/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCWALLTOGGLE_H
#define FUNCWALLTOGGLE_H

//=============================================
//
//=============================================
class CFuncWallToggle : public CBaseEntity
{
public:
	enum
	{
		FL_START_OFF		= (1<<0),
		FL_NOT_SOLID		= (1<<1),
		FL_NODE_IGNORE		= (1<<2),
		FL_TAKE_ANGLES		= (1<<3),
		FL_ALWAYS_INVISIBLE	= (1<<4)
	};

public:
	explicit CFuncWallToggle( edict_t* pedict );
	virtual ~CFuncWallToggle( void );

public:
	virtual bool Spawn( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }	
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool IsFuncWallToggleEntity( void ) const override { return true; }

	void TurnOff( void );
	void TurnOn( void );
	bool IsOn( void ) const;
};
#endif //FUNCWALLTOGGLE_H