/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVDLIGHTTRAIN_H
#define ENVDLIGHTTRAIN_H

#include "envspritetrain.h"

//=============================================
//
//=============================================
class CEnvDLightTrain : public CEnvSpriteTrain
{
public:
	explicit CEnvDLightTrain( edict_t* pedict );
	virtual ~CEnvDLightTrain( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool TrainSetModel( void ) override;
	virtual void SetSpawnProperties( void ) override;
};
#endif //ENVELIGHTTRAIN_H