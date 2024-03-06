/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSETSEQUENCE_H
#define ENVSETSEQUENCE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSetSequence : public CPointEntity
{
public:
	explicit CEnvSetSequence( edict_t* pedict );
	virtual ~CEnvSetSequence( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void EXPORTFN TriggerThink( void );

private:
	// Trigger target
	string_t m_seqDoneTarget;
};
#endif //ENVSETSEQUENCE_H