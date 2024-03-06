/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef GAMETEXT_H
#define GAMETEXT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameText : public CPointEntity
{
public:
	enum
	{
		FL_ALL_PLAYERS = (1<<0)
	};

public:
	explicit CGameText( edict_t* pedict );
	virtual ~CGameText( void );

public:
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Int32 m_channel;
	Float m_positionX;
	Float m_positionY;
	Int32 m_effect;
	color32_t m_color1;
	color32_t m_color2;
	Float m_fadeInTime;
	Float m_fadeOutTime;
	Float m_holdTime;
	Float m_fxTime;
};
#endif //GAMETEXT_H