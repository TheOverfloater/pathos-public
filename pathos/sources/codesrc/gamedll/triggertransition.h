/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERTRANSITION_H
#define TRIGGERTRANSITION_H

#include "baseentity.h"

//=============================================
//
//=============================================
class CTriggerTransition : public CBaseEntity
{
public:
	explicit CTriggerTransition( edict_t* pedict );
	virtual ~CTriggerTransition( void );

public:
	virtual bool Spawn( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //TRIGGERTRANSITION_H