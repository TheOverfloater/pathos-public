/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERAUTOSAVE_H
#define TRIGGERAUTOSAVE_H

#include "triggerentity.h"

//=============================================
//
//=============================================
class CTriggerAutoSave : public CTriggerEntity
{
public:
	enum
	{
		FL_ALL_DAY_STAGES	= (1<<0)
	};
	enum
	{
		AS_STATE_DEFAULT = 0,
		AS_STATE_NIGHT,
		AS_STATE_DAYLIGHT_RETURN,
		AS_STAGE_EXHAUSTED
	};

public:
	explicit CTriggerAutoSave( edict_t* pedict );
	virtual ~CTriggerAutoSave( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;

public:
	void PerformSave( CBaseEntity* pPlayer );
	void EXPORTFN SaveTouch( CBaseEntity* pOther );

private:
	Int32 m_triggerState;
};
#endif //TRIGGERAUTOSAVE_H