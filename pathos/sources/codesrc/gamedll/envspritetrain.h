/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVSPRITETRAIN_H
#define ENVSPRITETRAIN_H

#include "functrain.h"

//=============================================
//
//=============================================
class CEnvSpriteTrain : public CFuncTrain
{
public:
	// env_elight renderfx value
	static const Int32 ELIGHT_RENDERFX_VALUE;

public:
	explicit CEnvSpriteTrain( edict_t* pedict );
	virtual ~CEnvSpriteTrain( void );

public:
	virtual void Precache( void ) override;
	virtual void SetSpawnProperties( void ) override;
	virtual Vector GetDestinationVector( const Vector& destOrigin ) override;
};
#endif //ENVSPRITETRAIN_H