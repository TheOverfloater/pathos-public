/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERTEXTWINDOW_H
#define TRIGGERTEXTWINDOW_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerTextWindow : public CPointEntity
{
public:
	enum
	{
		FL_AUTO_GENERATE_CODE = (1<<0)
	};

public:
	explicit CTriggerTextWindow( edict_t* pedict );
	virtual ~CTriggerTextWindow( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	string_t m_textFilePath;
	string_t m_codeId;
	string_t m_code;
};

#endif //TRIGGERTEXTWINDOW_H