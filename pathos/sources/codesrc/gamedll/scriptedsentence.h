/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SCRIPTED_SENTENCE_H
#define SCRIPTED_SENTENCE_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CScriptedSentence : public CDelayEntity
{
public:
	enum
	{
		FL_ONLY_ONCE			= (1<<0),
		FL_ONLY_FOLLOWERS		= (1<<1),
		FL_INTERRUPT_SPEECH		= (1<<2),
		FL_CONCURRENT			= (1<<3),
		FL_SUB_ONLY_IN_RADIUS	= (1<<4)
	};
	enum attenuation_setting_t
	{
		RADIUS_SMALL = 0,
		RADIUS_MEDIUM,
		RADIUS_LARGE,
		RADIUS_NONE,
		RADIUS_X_LARGE
	};

public:
	explicit CScriptedSentence( edict_t* pedict );
	virtual ~CScriptedSentence( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

	virtual void DeclareSaveFields( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION); }
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

protected:
	void EXPORTFN FindThink( void );
	void EXPORTFN DelayThink( void );

	CBaseEntity* FindNPC( void );
	bool IsAcceptableSpeaker( CBaseEntity* pEntity ) const;
	bool StartSentence( CBaseEntity* pTarget );

private:
	string_t	m_sentenceName;
	string_t	m_npcName;
	string_t	m_listenerName;

	Float		m_radius;
	Float		m_repeatRate;
	Float		m_attenuation;
	Int32		m_attenuationSetting;
	Float		m_volume;

	bool		m_isActive;

	Float		m_duration;
	Double		m_beginTime;

	CEntityHandle	m_targetEntity;
	CEntityHandle	m_listenerEntity;
};
#endif //SCRIPTED_SENTENCE_H