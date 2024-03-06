/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCAMERAMODEL_H
#define TRIGGERCAMERAMODEL_H

#include "animatingentity.h"

class CPlayerEntity;

//=============================================
//
//=============================================
class CTriggerCameraModel : public CAnimatingEntity
{
public:
	// v_sequences model file path
	static const Char V_SEQUENCES_MODELNAME[];

public:
	enum
	{
		FL_FOLLOW_PLAYER	= (1<<0),
		FL_LEAVE_ON_TRIGGER	= (1<<1),
		FL_ADJUST_VIEW		= (1<<2),
		FL_KEEP_VIEWMODEL	= (1<<3),
		FL_KEEP_ANGLES		= (1<<4),
		FL_INTERPOLATE		= (1<<5),
		FL_ALWAYS_DRAW		= (1<<6),
		FL_DONT_UNDUCK		= (1<<7)
	};
	enum cammodel_state_t
	{
		CAMMODEL_STATE_OFF = 0,
		CAMMODEL_STATE_FIRST,
		CAMMODEL_STATE_SECOND,
		CAMMODEL_STATE_LOOP
	};
public:
	explicit CTriggerCameraModel( edict_t* pedict );
	virtual ~CTriggerCameraModel( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;
	virtual Int32 GetEntityFlags( void ) override { return CAnimatingEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual bool ShouldOverrideKeyValue( const Char* pstrKeyValue ) override;

public:
	static CTriggerCameraModel* CreateCameraModel( const CBaseEntity* pOwner, Float bendTime, const Char* pstrRestSequence, const Char* pstrEntrySequence, const Char* pstrLoopSequence, const Char* pstrTriggerSequence );
	static bool IsBlockingSaving( void );
	static void SetBlockingSaving( bool blocking );

public:
	void SetEntrySequence( const Char* pstrseqname );
	void SetLoopSequence( const Char* pstrseqname );
	void SetRestSequence( const Char* pstrseqname );
	void SetTriggerSequence( const Char* pstrseqname );
	void SetBlendTime( Float time );

private:
	void EXPORTFN FinishThink( void );
	void EXPORTFN ResetThink( void );
	void EXPORTFN EnterLoopThink( void );

private:
	bool CheckSequence( string_t& sequence );

private:
	string_t m_loopTarget;

	string_t m_entrySequence;
	string_t m_loopSequence;
	string_t m_restSequence;
	string_t m_triggerSequence;

	Float m_blendTime;

	Int32	m_triggerState;
	Int32	m_fov;

	CPlayerEntity* m_pPlayer;

private:
	// TRUE if we are blocking saving
	static bool g_bIsBlockingSaving;
};

#endif //TRIGGERCAMERAMODEL_H