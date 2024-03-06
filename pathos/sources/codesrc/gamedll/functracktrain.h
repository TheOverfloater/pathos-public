/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRACKTRAIN_H
#define FUNCTRACKTRAIN_H

#include "baseentity.h"
#include "pathtrack.h"

//=============================================
//
//=============================================
class CFuncTrackTrain : public CBaseEntity
{
public:
	// Train starting pitch
	static const Int32 START_PITCH;
	// Train starting pitch
	static const Int32 START_MAX_PITCH;
	// Train starting pitch
	static const Int32 TRAIN_MAX_SPEED;
	// Default speed
	static const Float DEFAULT_SPEED;
	// Number of train default move sounds
	static const Uint32 NB_DEFAULT_MOVE_SOUNDS;
	// Default train move sounds
	static const Char* DEFAULT_MOVE_SOUNDS[];
	// Default train stop sound
	static const Char DEFAULT_STOP_SOUND[];
	// Start train stop sound
	static const Char DEFAULT_START_SOUND[];

public:
	enum
	{
		FL_NO_PITCH		= (1<<0),
		FL_NO_CONTROL	= (1<<1),
		FL_FORWARD_ONLY	= (1<<2),
		FL_NOT_SOLID	= (1<<3)
	};

public:
	explicit CFuncTrackTrain( edict_t* pedict );
	virtual ~CFuncTrackTrain( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void CallBlocked( CBaseEntity* pBlocker ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION) | FL_ENTITY_DIRECTIONAL_USE; }
	virtual void OnOverrideEntity( void ) override;
	virtual bool IsFuncTrackTrainEntity( void ) const override { return true; }
	virtual void SetNextThink( Double thinkTime, bool alwaysThink ) override;

public:
	void EXPORTFN Next( void );
	void EXPORTFN Find( void );
	void EXPORTFN RestorePath( void );
	void EXPORTFN DeadEnd( void );

	void SetTrack( CPathTrack* pTrack );
	void SetControls( CBaseEntity* pControls );
	bool IsOnControls( CBaseEntity* pEntity );
	void SetIsOnTrackChange( bool isOnTrackChange );

	void StopSound( void );
	void UpdateSound( void );

	CPathTrack* FindPathTrack( const Char* pstrPathTrackName );
	void SetPathTrack( CPathTrack* pPathTrack );

public:
	CPathTrack* GetPath( void );
	Float GetLength( void ) const;

private:
	CPathTrack* m_pPath;

	Float m_length;
	Float m_height;
	Float m_speed;
	Float m_direction;
	Float m_startSpeed;
	Float m_blockDamage;

	Vector m_controlMins;
	Vector m_controlMaxs;

	bool m_onTrackChange;
	bool m_soundPlaying;
	Int32 m_sounds;

	Float m_volume;
	Float m_bank;
	Float m_oldSpeed;
	Int32 m_lastPitch;

	string_t m_moveSound;
	string_t m_stopSound;
	string_t m_startSound;

	string_t m_pathTrackName;
};
#endif //FUNCTRACKTRAIN_H