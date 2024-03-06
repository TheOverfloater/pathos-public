/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PLATTRAINENTITY_H
#define PLATTRAINENTITY_H

#include "toggleentity.h"

//=============================================
//
//=============================================
class CPlatTrainEntity : public CToggleEntity
{
public:
	// Number of legacy move sounds
	static const Uint32 NB_LEGACY_MOVE_SOUNDS;
	// Legacy move sounds
	static const Char* LEGACY_MOVE_SOUNDS[];
	// Number of legacy stop sounds
	static const Uint32 NB_LEGACY_STOP_SOUNDS;
	// Legacy stop sounds
	static const Char* LEGACY_STOP_SOUNDS[];

public:
	enum
	{
		FL_PLAT_TOGGLE = (1<<0)
	};

public:
	explicit CPlatTrainEntity( edict_t* pedict );
	virtual ~CPlatTrainEntity( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

public:
	virtual bool IsTogglePlat( void );

protected:
	Int32 m_moveSound;
	Int32 m_stopSound;

	Float m_volume;
};
#endif //PLATTRAINENTITY_H