/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVROTLIGHT_H
#define ENVROTLIGHT_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CEnvRotLight : public CAnimatingEntity
{
public:
	// Rotating light model
	static const Char ENV_ROTLIGHT_MODEL_FILENAME[];

public:
	enum
	{
		FL_NO_SHADOWS	= (1<<0),
		FL_START_ON		= (1<<1)
	};

public:
	explicit CEnvRotLight( edict_t* pedict );
	virtual ~CEnvRotLight( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CAnimatingEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

private:
	bool m_isActive;
};
#endif //ENVROTLIGHT_H