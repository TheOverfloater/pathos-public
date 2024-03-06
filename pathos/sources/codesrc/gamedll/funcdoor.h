/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCDOOR_H
#define FUNCDOOR_H

#include "toggleentity.h"

//=============================================
//
//=============================================
class CFuncDoor : public CToggleEntity
{
public:
	// Maximum slave doors
	static const Uint32 MAX_SLAVE_DOORS = 64;
	// Maximum related doors
	static const Uint32 MAX_RELATED_DOORS = 64;
	// Default speed for doors
	static const Float DEFAULT_SPEED;
	// Number of legacy door sounds
	static const Uint32 NUM_LEGACY_MOVE_SOUNDS;
	// Legacy move sounds
	static const Char* LEGACY_MOVE_SOUNDS[];
	// Number of legacy door sounds
	static const Uint32 NUM_LEGACY_STOP_SOUNDS;
	// Legacy move sounds
	static const Char* LEGACY_STOP_SOUNDS[];
	// Wait time between locked sounds
	static const Float LOCKED_SOUND_DELAY;

public:
	enum
	{
		FL_START_OPEN			= (1<<0),
		FL_NOT_SOLID			= (1<<3),
		FL_ONE_WAY				= (1<<4),
		FL_NO_AUTO_RETURN		= (1<<5),
		FL_USE_ONLY				= (1<<8),
		FL_NO_NPCS				= (1<<9),
		FL_TOUCH_OPENS			= (1<<10),
		FL_NO_PROXIMITY_CHECKS	= (1<<11),
		FL_NODRAW				= (1<<12)
	};

public:
	explicit CFuncDoor( edict_t* pedict );
	virtual ~CFuncDoor( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual Int32 GetEntityFlags( void ) override;
	virtual void InitEntity( void ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void CallBlocked( CBaseEntity* pOther ) override;

	virtual bool IsFuncDoorEntity( void ) const override { return true; }
	virtual void SetParentDoor( CFuncDoor* pParent ) override;
	virtual void ChildEntityRemoved( CBaseEntity* pEntity ) override;
	virtual void SetForcedClose( void ) override;

	virtual togglestate_t GetToggleState( void ) const override { return (togglestate_t)m_toggleState; }
	virtual void SetToggleState( togglestate_t state, bool reverse ) override;
	virtual usableobject_type_t GetUsableObjectType( void ) override;

public:
	virtual void SetSpawnProperties( void );
	virtual void SetMovementVectors( void );
	virtual void DoorBeginMoveUp( void );
	virtual void DoorBeginMoveDown( void );
	virtual bool ShouldAutoCloseDoor( void );
	virtual void RealignRelatedDoor( CFuncDoor* pDoor );

	bool DoorActivate( void );

public:
	void EXPORTFN GoUp( void );
	void EXPORTFN GoDown( void );
	void EXPORTFN HitTop( void );
	void EXPORTFN HitBottom( void );
	void EXPORTFN DoorTouch( CBaseEntity* pOther );

protected:
	Int16 m_legacyMoveSound;
	Int16 m_legacyStopSound;

	Int16 m_legacyLockedSound;
	Int16 m_legacyUnlockedSound;

	bool m_forcedToClose;
	bool m_isBlocked;
	bool m_isSilent;

	Vector m_activatorOrigin;
	Double m_nextLockedSoundTime;

	CFuncDoor* m_pSlaveDoors[MAX_SLAVE_DOORS];
	Uint32 m_numSlaveDoors;

	CFuncDoor* m_pRelatedDoors[MAX_RELATED_DOORS];
	Uint32 m_numRelatedDoors;
};
#endif //DOORENTITY_H
