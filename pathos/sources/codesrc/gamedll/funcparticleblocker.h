/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCPARTICLEBLOCKER_H
#define FUNCPARTICLEBLOCKER_H

//=============================================
//
//=============================================
class CFuncParticleBlocker : public CBaseEntity
{
public:
	explicit CFuncParticleBlocker( edict_t* pedict );
	virtual ~CFuncParticleBlocker( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //FUNCWALL_H