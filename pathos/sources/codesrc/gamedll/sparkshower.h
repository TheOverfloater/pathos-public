/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef SPARKSHOWER_H
#define SPARKSHOWER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CSparkShower : public CBaseEntity
{
public:
	explicit CSparkShower( edict_t* pedict );
	virtual ~CSparkShower( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;
	virtual Int32 GetEntityFlags( void ) override { return FL_ENTITY_DONT_SAVE; }

public:
	void EXPORTFN SparkThink( void );
};
#endif //SPARKSHOWER_H