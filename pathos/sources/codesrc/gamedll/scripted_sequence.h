/*
===============================================
Pathos Engine - Created by Andrew "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SCRIPTED_SEQUENCE_H
#define SCRIPTED_SEQUENCE_H

#include "delayentity.h"

class CBaseNPC;

enum script_interrupt_level_t
{
	SCRIPT_INTERRUPT_IDLE = 0,
	SCRIPT_INTERRUPT_BY_NAME,
	SCRIPT_INTERRUPT_AI
};

enum script_moveto_t
{
	SCRIPT_MOVETO_NO = 0,
	SCRIPT_MOVETO_WALK,
	SCRIPT_MOVETO_RUN,
	SCRIPT_MOVETO_INSTANTENOUS = 4,
	SCRIPT_MOVETO_TURN_TO_FACE,
	SCRIPT_MOVETO_WALK_NO_TURN,
	SCRIPT_MOVETO_RUN_NO_TURN,
	SCRIPT_MOVETO_TURN_TO_PLAYER
};

//=============================================
//
//=============================================
class CScriptedSequence : public CDelayEntity
{
public:
	enum
	{
		FL_WAIT_TILL_SEEN		= (1<<0),
		FL_EXIT_WHEN_AGITATED	= (1<<1),
		FL_REPEATABLE			= (1<<2),
		FL_LEAVE_CORPSE			= (1<<3),
		FL_DONT_RETRY			= (1<<4),
		FL_NO_INTERRUPTIONS		= (1<<5),
		FL_OVERRIDE_STATE		= (1<<6),
		FL_NO_SCRIPT_MOVEMENT	= (1<<7),
		FL_IDLE_BY_TRIGGER		= (1<<8),
		FL_TRIGGER_IDLE_FIRST	= (1<<9),
		FL_SEARCH_RADIUS		= (1<<10),
		FL_SOUNDS_CAN_INTERRUPT	= (1<<11)
	};

public:
	CScriptedSequence( edict_t* pedict );
	virtual ~CScriptedSequence( void );

public:
	void DelayedStart( bool state );
	void SetAllowInterrupt( bool allowInterrupt );
	bool StartSequence( CBaseNPC* pNPC, const Char* pstrSequenceName, bool completeOnEmpty );
	void CancelScript( void );

	Int32 GetMoveToSetting( void );
	Int32 GetScriptDelay( void );
	Double GetStartTime( void );
	void SetSequenceDone( CBaseNPC* pNPC );
	void ClearTargetEntity( void );

	bool HasPlaySequence( void );
	const Char* GetPlaySequenceName( void );
	bool HasIdleAnimation( void );
	const Char* GetIdleSequenceName( void );

	Uint64 GetIgnoreConditions( void );
};
#endif //SCRIPTED_SEQUENCE_H