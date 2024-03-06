/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVELIGHTTRAIN_H
#define ENVELIGHTTRAIN_H

#include "envspritetrain.h"

//=============================================
//
//=============================================
class CEnvELightTrain : public CEnvSpriteTrain
{
public:
	explicit CEnvELightTrain( edict_t* pedict );
	virtual ~CEnvELightTrain( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool TrainSetModel( void ) override;
	virtual void SetSpawnProperties( void ) override;
};
#endif //ENVELIGHTTRAIN_H