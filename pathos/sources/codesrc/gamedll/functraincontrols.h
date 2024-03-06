/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRAINCONTROLS_H
#define FUNCTRAINCONTROLS_H

#include "baseentity.h"

//=============================================
//
//=============================================
class CFuncTrainControls : public CBaseEntity
{
public:
	explicit CFuncTrainControls( edict_t* pedict );
	virtual ~CFuncTrainControls( void );

public:
	virtual bool Spawn( void ) override;
	virtual void InitEntity( void ) override;
};
#endif //FUNCTRAINCONTROLS_H