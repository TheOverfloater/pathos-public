/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERGRAVITY_H
#define TRIGGERGRAVITY_H

#include "triggerentity.h"

//=============================================
//
//=============================================
class CTriggerGravity : public CTriggerEntity
{
public:
	explicit CTriggerGravity( edict_t* pedict );
	virtual ~CTriggerGravity( void );

public:
	virtual void CallTouch( CBaseEntity* pOther ) override;
};
#endif //TRIGGERGRAVITY_H