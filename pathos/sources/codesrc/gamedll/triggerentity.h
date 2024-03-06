/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERENTITY_H
#define TRIGGERENTITY_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CTriggerEntity : public CDelayEntity
{
public:
	explicit CTriggerEntity( edict_t* pedict );
	virtual ~CTriggerEntity( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual Int32 GetEntityFlags( void ) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	virtual bool IsMasterTriggered( void );
	virtual bool CheckFilterFlags( CBaseEntity* pOther, bool noclients, bool allownpcs, bool allowpushables );

protected:
	string_t m_filterEntityName;
	string_t m_masterEntityName;
	string_t m_triggerMessage;

	CEntityHandle m_activator;
};
#endif //TRIGGERENTITY_H