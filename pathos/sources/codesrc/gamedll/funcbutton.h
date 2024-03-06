/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCBUTTONH
#define FUNCBUTTONH

#include "toggleentity.h"

//=============================================
//
//=============================================
class CFuncButton : public CToggleEntity
{
public:
	// Locked sound wait time
	static const Float LOCKED_SOUND_WAIT_TIME;
	// Default button speed
	static const Float DEFAULT_SPEED;
	// Default button wait time
	static const Float DEFAULT_WAIT_TIME;
	// Default button lip value
	static const Float DEFAULT_LIP_VALUE;
	// Legacy button sound count
	static const Uint32 NUM_LEGACY_BUTTON_SOUNDS;
	// Legacy button sounds
	static const Char* LEGACY_BUTTON_SOUNDS[];

public:
	enum
	{
		FL_TOGGLE				= (1<<5),
		FL_SPARK_WHEN_OFF		= (1<<6),
		FL_TOUCH_ONLY			= (1<<8),
		FL_MOVE					= (1<<9),
		FL_NODRAW				= (1<<10),
		FL_DISABLED_NO_RETICLE	= (1<<11)
	};

public:
	enum buttoncode_t
	{
		BUTTON_CODE_NONE = 0,
		BUTTON_CODE_ACTIVATE,
		BUTTON_CODE_RETURN
	};

public:
	explicit CFuncButton( edict_t* pedict );
	virtual ~CFuncButton( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual Int32 GetEntityFlags( void ) override { return ((CToggleEntity::GetFlags() & ~FL_ENTITY_TRANSITION) | FL_ENTITY_PLAYER_USABLE); }
	virtual bool IsFuncButtonEntity( void ) const override { return true; }
	virtual void SetPairedButtonDelay( Float delayTime ) override;
	virtual usableobject_type_t GetUsableObjectType( void ) override;

public:
	void EXPORTFN ButtonUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );
	void EXPORTFN ButtonTouch( CBaseEntity* pOther );
	void EXPORTFN SparkThink( void );
	void EXPORTFN TriggerAndWait( void );
	void EXPORTFN ReturnThink( void );
	void EXPORTFN ReturnBack( void );

public:
	virtual buttoncode_t GetResponseToTouch( void );
	virtual void ButtonActivate( void );
	virtual void SetDelayOnPairs( void );

public:
	virtual void SetSpawnProperties( void );
	virtual void BeginPressedMovement( void );
	virtual void BeginReturnMovement( void );
	virtual void SetReturnBackSparking( void );

protected:
	bool m_stayPushed;

	string_t m_changeTargetName;
	string_t m_lockedTriggerTarget;
	string_t m_pairButtonName;

	Int16 m_legacyUseSound;
	Int16 m_legacyLockedSound;
	Int16 m_legacyUnlockedSound;

	string_t m_useSoundFile;

	Double m_nextLockSoundTime;
	Double m_nextUsableTime;
};
#endif //FUNCBUTTONH