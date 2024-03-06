/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUCNDOORROTATING_H
#define FUCNDOORROTATING_H

#include "funcdoor.h"

//=============================================
//
//=============================================
class CFuncDoorRotating : public CFuncDoor
{
public:
	enum
	{
		FL_ROTATE_Y				= 0,
		FL_ROTATE_REVERSE		= (1<<1),
		FL_ROTATE_Z				= (1<<6),
		FL_ROTATE_X				= (1<<7)
	};

public:
	explicit CFuncDoorRotating( edict_t* pedict );
	virtual ~CFuncDoorRotating( void );

	virtual void SetToggleState( togglestate_t state, bool reverse ) override;
	virtual bool IsFuncDoorRotatingEntity( void ) const { return true; }

public:
	virtual void SetSpawnProperties( void ) override;
	virtual void SetMovementVectors( void ) override;
	virtual void DoorBeginMoveUp( void ) override;
	virtual void DoorBeginMoveDown( void ) override;
	virtual void RealignRelatedDoor( CFuncDoor* pDoor ) override;
};
#endif //FUCNDOORROTATING_H