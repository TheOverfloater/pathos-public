/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PLATTRIGGER_H
#define PLATTRIGGER_H

#include "baseentity.h"

class CFuncPlat;

//=============================================
//
//=============================================
class CPlatTrigger : public CBaseEntity
{
public:
	explicit CPlatTrigger( edict_t* pedict );
	virtual ~CPlatTrigger( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION); }
	virtual void CallTouch( CBaseEntity* pOther ) override;

public:
	void SetPlatform( class CFuncPlat* pPlatform );

public:
	static void SpawnPlatTrigger( CFuncPlat* pPlatform );

private:
	CFuncPlat* m_pPlatform;
};
#endif //PLATTRIGGER_H