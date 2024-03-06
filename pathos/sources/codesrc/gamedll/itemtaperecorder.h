/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ITEMTAPERECORDER_H
#define ITEMTAPERECORDER_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CItemTapeRecorder : public CAnimatingEntity
{
public:
	// Recorder model for benefactor
	static const Char RECORDER_MODEL[];
	// Recorder idle sequence name
	static const Char RECORDER_IDLE_SEQ_NAME[];
	// Recorder play sequence name
	static const Char RECORDER_PLAY_SEQ_NAME[];
	// Recorder use sound name
	static const Char RECORDER_USE_SOUND_NAME[];
	// Recorder stop sound name
	static const Char RECORDER_STOP_SOUND_NAME[];
	// Recorder play sound name
	static const Char RECORDER_PLAY_SOUND_NAME[];

public:
	enum
	{
		SF_START_INVISIBLE = (1<<0)
	};

public:
	explicit CItemTapeRecorder( edict_t* pedict );
	virtual ~CItemTapeRecorder( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual bool Restore( void ) override;

	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CAnimatingEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION)|FL_ENTITY_PLAYER_USABLE; }
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;
	virtual usableobject_type_t GetUsableObjectType( void ) override;

public:
	void EXPORTFN Reset( void );

private:
	// Playback title
	string_t m_playbackTitle;
	// File to play back
	string_t m_soundFileName;
	// Duration of playback
	Float m_duration;
	// Playback begin time
	Double m_playbackBeginTime;
	// TRUE if active
	bool m_isActive;
};
#endif //ITEMTAPERECORDER_H