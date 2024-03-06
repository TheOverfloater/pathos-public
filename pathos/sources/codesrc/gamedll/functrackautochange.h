/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRACKAUTOCHANGE_H
#define FUNCTRACKAUTOCHANGE_H

#include "functrackchange.h"

//=============================================
//
//=============================================
class CFuncTrackAutoChange : public CFuncTrackChange
{
public:
	explicit CFuncTrackAutoChange( edict_t* pedict );
	virtual ~CFuncTrackAutoChange( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void UpdateAutoTargets( togglestate_t state ) override;
};
#endif //FUNCTRACKAUTOCHANGE_H