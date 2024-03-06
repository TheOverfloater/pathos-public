/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ITEMDIARY_H
#define ITEMDIARY_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CItemDiary : public CAnimatingEntity
{
public:
	// Blend time for diary
	static const Float DIARY_BLEND_TIME;
	// Diary model for benefactor
	static const Char DIARY_MODEL_BENEFACTOR[];
	// Diary model for benefactor
	static const Char DIARY_MODEL_RADFORD[];
	// Diary entry sequence name
	static const Char DIARY_ENTRY_SEQ_NAME[];
	// Diary loop sequence name
	static const Char DIARY_LOOP_SEQ_NAME[];
	// Diary exit sequence name
	static const Char DIARY_EXIT_SEQ_NAME[];
	// Diary rest sequence name
	static const Char DIARY_REST_SEQ_NAME[];

public:
	enum diarytype_t
	{
		DIARY_BENEFACTOR = 0,
		DIARY_RADFORD
	};
	enum diarystate_t
	{
		DIARY_OFF = 0,
		DIARY_ENTER,
		DIARY_PLAYBACK,
		DIARY_LEAVE
	};
	enum
	{
		FL_START_DISABLED	= (1<<0),
		FL_STAY_DISABLED	= (1<<1),
		FL_CANNOT_INTERRUPT	= (1<<2)
	};

public:
	explicit CItemDiary( edict_t* pedict );
	virtual ~CItemDiary( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CAnimatingEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION)|FL_ENTITY_PLAYER_USABLE; }
	virtual void AdvanceState( void ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;
	virtual void StopDiaryPlayback( void ) override;
	virtual usableobject_type_t GetUsableObjectType( void ) override;

public:
	void EXPORTFN Exit( void );
	void EXPORTFN Enter( void );

private:
	Float m_duration;
	Int32 m_diaryType;
	Int32 m_state;
	bool m_isDisabled;

	string_t m_soundFile;
	string_t m_playerSound;
	string_t m_entryTrigger;

private:
	class CTriggerCameraModel* m_pCameraModel;
	class CBaseEntity* m_pPlayer;
};
#endif //ITEMDIARY_H