/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCROTBUTTON_H
#define FUNCROTBUTTON_H

#include "funcbutton.h"

//=============================================
//
//=============================================
class CFuncRotButton : public CFuncButton
{
public:
	enum
	{
		FL_NOT_SOLID		= (1<<0),
		FL_ROTATE_REVERSE	= (1<<1),
		FL_ROTATE_Z			= (1<<6),
		FL_ROTATE_X			= (1<<7)
	};
public:
	explicit CFuncRotButton( edict_t* pedict );
	virtual ~CFuncRotButton( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual void BeginPressedMovement( void ) override;
	virtual void BeginReturnMovement( void ) override;
	virtual void SetReturnBackSparking( void ) override {};
};
#endif //FUNCROTBUTTON_H