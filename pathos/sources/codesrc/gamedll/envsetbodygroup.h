/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSETBODYGROUP_H
#define ENVSETBODYGROUP_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSetBodyGroup : public CPointEntity
{
public:
	explicit CEnvSetBodyGroup( edict_t* pedict );
	virtual ~CEnvSetBodyGroup( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;

private:
	// Bodygroup name
	string_t m_bodyGroupName;
	// Submodel name
	string_t m_submodelName;
};
#endif //ENVSETBODYGROUP_H