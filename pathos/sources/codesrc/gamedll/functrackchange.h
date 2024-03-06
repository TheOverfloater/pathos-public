/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRACKCHANGE_H
#define FUNCTRACKCHANGE_H

#include "funcplatrot.h"

class CFuncTrackTrain;
class CPathTrack;

//=============================================
//
//=============================================
class CFuncTrackChange : public CFuncPlatRot
{
public:
	// Blocked alarm sound
	static const Char BLOCKED_ALARM_SND[];

public:
	enum
	{
		FL_AUTO_ACTIVATE_TRAIN	= (1<<0),
		FL_RELINK_TRACK			= (1<<1),
		FL_ROTATE_MOVE			= (1<<2),
		FL_START_AT_BOTTOM		= (1<<3),
		FL_ROTATE_ONLY			= (1<<4),
		FL_ROTATE_X_AXIS		= (1<<6),
		FL_ROTATE_Y_AXIS		= (1<<7)
	};
	enum traincode_t
	{
		TRAIN_SAFE = 0,
		TRAIN_BLOCKING,
		TRAIN_FOLLOWING
	};

public:
	explicit CFuncTrackChange( edict_t* pedict );
	virtual ~CFuncTrackChange( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void OnOverrideEntity( void ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;
	virtual void InitEntity( void ) override;

public:
	virtual void GoUp( void ) override;
	virtual void GoDown( void ) override;

	traincode_t EvaluateTrain( CPathTrack* pCurrent );
	void UpdateTrain( const Vector& destAngles );

	virtual void HitBottom( void ) override;
	virtual void HitTop( void ) override;
	virtual void UpdateAutoTargets( togglestate_t state );
	virtual bool IsTogglePlat( void ) override;

	void DisableUse( void );
	void EnableUse( void );
	bool IsUseEnabled( void ) const;

	CPathTrack* FindPathTrack( const Char* pstrTargetname );

protected:
	CPathTrack* m_pTopTrack;
	CPathTrack* m_pBottomTrack;
	CFuncTrackTrain* m_pTrain;

	string_t m_topTrackName;
	string_t m_bottomTrackName;
	
	string_t m_trainName;

	Int32 m_code;
	Int32 m_targetState;
	bool m_isUsable;
};
#endif //FUNCTRACKCHANGE_H