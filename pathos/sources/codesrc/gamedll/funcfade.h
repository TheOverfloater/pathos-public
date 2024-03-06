/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCFADE_H
#define FUNCFADE_H

#include "funcwall.h"

//=============================================
//
//=============================================
class CFuncFade : public CFuncWall
{
public:
	explicit CFuncFade( edict_t* pedict );
	virtual ~CFuncFade( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual void DeclareSaveFields( void ) override;

	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	void EXPORTFN Think( void );

protected:
	// Duration of fade
	Float m_fadeDuration;
	// Target alpha
	Float m_targetAlpha;
	// Fade begin time
	Double m_fadeBeginTime;
	// Start alpha value
	Float m_startAlpha;
};
#endif //FUNCWALL_H