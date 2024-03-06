/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUCNWATER_H
#define FUCNWATER_H

#include "funcdoor.h"

//=============================================
//
//=============================================
class CFuncWater : public CFuncDoor
{
public:
	explicit CFuncWater( edict_t* pedict );
	virtual ~CFuncWater( void );

public:
	virtual bool KeyValue( const keyvalue_t& kv ) override;

public:
	virtual void SetSpawnProperties( void ) override;
};
#endif //FUCNWATER_H