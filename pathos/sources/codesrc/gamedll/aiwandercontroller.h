/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AIWANDERCONTROLLER_H
#define AIWANDERCONTROLLER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAIWanderController : public CPointEntity
{
private:
	enum mode_t
	{
		WC_TOGGLE = 0,
		WC_ENABLE,
		WC_DISABLE
	};

public:
	explicit CAIWanderController( edict_t* pedict );
	virtual ~CAIWanderController( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Int32 m_mode;
};
#endif //AISETENEMY_H