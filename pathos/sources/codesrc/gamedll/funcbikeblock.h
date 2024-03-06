/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCBIKEBLOCK
#define FUNCBIKEBLOCK

#include "pointentity.h"

//=============================================
//
//=============================================
class CFuncBikeBlock : public CBaseEntity
{
public:
	explicit CFuncBikeBlock( edict_t* pedict );
	virtual ~CFuncBikeBlock( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool Restore( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }	
	virtual void SetBikeBlock( bool enable ) override;
	virtual bool IsBikeBlocker( void ) override { return true; }
	virtual void InitEntity( void ) override;
};

#endif //FUNCBIKEBLOCK