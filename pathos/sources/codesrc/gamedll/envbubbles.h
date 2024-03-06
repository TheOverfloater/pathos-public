/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVBUBBLES_H
#define ENVBUBBLES_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvBubbles : public CBaseEntity
{
public:
	// Bubble sprite path
	static const Char BUBBLE_SPRITE_PATH[];
	// Maximum frequency
	static const Int32 MAX_FREQUENCY;

public:
	enum
	{
		FL_START_OFF = (1<<0)
	};

public:
	explicit CEnvBubbles( edict_t* pedict );
	virtual ~CEnvBubbles( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }	

public:
	void EXPORTFN BubblingThink( void );

public:
	Int32 m_density;
	Int32 m_frequency;
	Int32 m_bubbleSprite;
	Float m_current;
	bool m_isActive;
};
#endif //ENVBUBBLES_H