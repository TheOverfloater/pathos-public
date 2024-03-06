/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERMULTIPLE_H
#define TRIGGERMULTIPLE_H

#include "triggeronce.h"

//=============================================
//
//=============================================
class CTriggerMultiple : public CTriggerOnce
{
public:
	explicit CTriggerMultiple( edict_t* pedict );
	virtual ~CTriggerMultiple( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual bool IsTriggerMultipleEntity( void ) const override { return true; }
	virtual void TriggerWait( CBaseEntity* pActivator ) override;
	virtual void PostTrigger( void ) override;
	virtual void SetTriggerWait( void ) override;

public:
	void EXPORTFN RestoreThink( void );

private:
	Float m_waitTime;
};
#endif //TRIGGERONCE_H