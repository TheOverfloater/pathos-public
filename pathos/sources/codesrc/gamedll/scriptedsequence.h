/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SCRIPTED_SEQUENCE_H
#define SCRIPTED_SEQUENCE_H

#include "delayentity.h"

class CBaseNPC;

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

enum script_loop_t
{
	SCRIPT_LOOP_INACTIVE = 0,
	SCRIPT_LOOP_PLAYING_LOOP,
	SCRIPT_LOOP_PLAYING_EXIT
};

//=============================================
//
//=============================================
class CScriptedSequence : public CDelayEntity
{
public:
	// Script default break conditions
	static const Uint64 SCRIPT_BREAK_CONDITIONS;

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
		FL_SOUNDS_CAN_INTERRUPT	= (1<<11),
		FL_ENEMIES_CAN_INTERRUPT= (1<<12)
	};

public:
	explicit CScriptedSequence( edict_t* pedict );
	virtual ~CScriptedSequence( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION); }
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void InitEntity( void ) override;
	virtual bool IsScriptedSequenceEntity( void ) const override { return true; }
	virtual CBaseEntity* GetTargetEntity( void ) override { return m_targetEntity; }

public:
	bool StartSequence( CBaseEntity* pNPC, const Char* pstrSequenceName, bool completeOnEmpty );
	void CancelScript( void );

	Int32 GetMoveToSetting( void ) const;
	Int32 GetScriptDelay( void ) const;
	bool IsWaitingToBeTriggered( void ) const;
	void SetSequenceDone( CBaseEntity* pNPC );

	void EXPORTFN ScriptedThink( void );
	void PosessEntity( void );

	bool FindNPC( void );
	void DelayStart( bool state );
	bool CanOverrideState( void ) const;
	void ClearTargetEntity( void );

	void SetAllowInterrupt( bool allowInterrupt );
	bool CanInterrupt( void );

	bool PlayAndLoopMatch( void );
	void SetLoopState( script_loop_t loopstate );
	script_loop_t GetLoopState( void ) const;

	static void FixScriptNPCSchedule( CBaseEntity* pNPC );
	static void ScriptEntityCancel( CScriptedSequence* pScripted );

public:
	bool HasPlaySequence( void ) const;
	const Char* GetPlaySequenceName( void );
	bool HasIdleAnimation( void ) const;
	const Char* GetIdleSequenceName( void );
	const Char* GetLoopSequenceName( void );
	const Char* GetExitSequenceName( void );

	Uint64 GetIgnoreConditions( void );
	Uint64 GetScriptBreakingAIConditions( void );
	Uint64 GetInterruptSoundMask( void );
	script_loop_t GetLoopState( void );

private:
	string_t m_idleSequence;
	string_t m_playSequence;
	string_t m_loopSequence;
	string_t m_exitSequence;
	string_t m_npcName;
	Int32 m_lastSequence;
	Int32 m_moveToSetting;

	Float m_radius;
	Float m_repeatRange;
	Int32 m_customSoundMask;

	Int32 m_scriptDelay;
	bool m_waitForReTrigger;

	bool m_isInterruptable;
	Int32 m_playLoop;

	CEntityHandle m_targetEntity;
};
#endif //SCRIPTED_SEQUENCE_H