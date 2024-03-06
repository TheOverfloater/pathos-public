/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PLAYERLOADSAVED_H
#define PLAYERLOADSAVED_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CPlayerLoadSaved : public CPointEntity
{
public:
	explicit CPlayerLoadSaved( edict_t* pedict );
	virtual ~CPlayerLoadSaved( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Float GetDuration( void );
	Float GetHoldTime( void );
	Float GetMessageTime( void );
	Float GetLoadTime( void );

	void SetDuration( Float duration );
	void SetHoldTime( Float holdtime );
	void SetMessageTime( Float messagetime );
	void SetLoadTime( Float loadtime );

public:
	void EXPORTFN MessageThink( void );
	void EXPORTFN LoadThink( void );

public:
	static void ClearSaveGameBlock( void );
	static bool IsBlockingSaving( void );

private:
	Float m_messageTime;
	Float m_loadTime;
	Float m_duration;
	Float m_holdTime;

private:
	static bool m_isBlockingSaveGame;
};
#endif //PLAYERLOADSAVED_H