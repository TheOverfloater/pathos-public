/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AIFLAGTOGGLER_H
#define AIFLAGTOGGLER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAIFlagToggler : public CPointEntity
{
private:
	enum flag_mode_t
	{
		FLAGTOGGLER_DISABLE = 0,
		FLAGTOGGLER_ENABLE
	};

	enum toggler_flags_t
	{
		FLAG_WAITTILLSEEN = 0,
		FLAG_GAG,
		FLAG_MONSTERCLIP,
		FLAG_REMEMBER,
		FLAG_PRISONER,
		FLAG_WAITFORSCRIPT,
		FLAG_PREDISASTER,
		FLAG_FADECORPSE,
		FLAG_IMMORTAL,
		FLAG_DONTFALL,
		FLAG_HIDECORPSE,
		FLAG_NOWEAPONDROP,
		FLAG_NOPUSHING
	};

public:
	explicit CAIFlagToggler( edict_t* pedict );
	virtual ~CAIFlagToggler( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	// Mode
	Int32 m_mode;
	// Flag
	Int32 m_flag;
};
#endif //AIFLAGTOGGLER_H