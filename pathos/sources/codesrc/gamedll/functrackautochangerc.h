/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRACKAUTOCHANGERC_H
#define FUNCTRACKAUTOCHANGERC_H

#include "functrackautochange.h"

//=============================================
//
//=============================================
class CFuncTrackAutoChangeRC : public CFuncTrackAutoChange
{
public:
	// Default damage dealt by this entity
	static const Float DEFAULT_DAMAGE_DEALT;

public:
	explicit CFuncTrackAutoChangeRC( edict_t* pedict );
	virtual ~CFuncTrackAutoChangeRC( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void CallBlocked( CBaseEntity* pBlocker ) override;
};
#endif //FUNCTRACKAUTOCHANGERC_H