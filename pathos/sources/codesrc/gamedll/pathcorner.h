/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PATHCORNER_H
#define PATHCORNER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CPathCorner : public CPointEntity
{
public:
	enum
	{
		FL_WAIT_FOR_TRIGGER		= (1<<0),
		FL_TELEPORT				= (1<<1),
		FL_FIRE_ONCE			= (1<<2),
		FL_SET_ZERO_AVELOCITY	= (1<<3)
	};
public:
	explicit CPathCorner( edict_t* pedict );
	virtual ~CPathCorner( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual Float GetDelay( void ) override;
	virtual bool IsPathCornerEntity( void ) const override { return true; }

public:
	Float m_waitTime;
};
#endif //PATHCORNER_H