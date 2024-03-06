/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCNPCCLIP_H
#define FUNCNPCCLIP_H

//=============================================
//
//=============================================
class CFuncNPCClip : public CBaseEntity
{
public:
	enum
	{
		FL_START_OFF = (1<<0)
	};
public:
	explicit CFuncNPCClip( edict_t* pedict );
	virtual ~CFuncNPCClip( void );

public:
	virtual bool Spawn( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }	
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool IsFuncNPCClipEntity( void ) const override { return true; }
};
#endif //FUNCNPCCLIP_H