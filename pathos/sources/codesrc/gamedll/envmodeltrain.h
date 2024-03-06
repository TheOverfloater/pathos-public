/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVMODELTRAIN_H
#define ENVMODELTRAIN_H

#include "envspritetrain.h"

//=============================================
//
//=============================================
class CEnvModelTrain : public CEnvSpriteTrain
{
public:
	explicit CEnvModelTrain( edict_t* pedict );
	virtual ~CEnvModelTrain( void );

public:
	virtual bool TrainSetModel( void ) override;
};
#endif //ENVMODELTRAIN_H