/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVPOSSKY_H
#define ENVPOSSKY_H

#include "functrain.h"

//=============================================
//
//=============================================
class CEnvPosSky : public CFuncTrain
{
public:
	// Distance between PVS updates
	static const Float PVS_UPDATE_DIST;

public:
	explicit CEnvPosSky( edict_t* pedict );
	virtual ~CEnvPosSky( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool TrainSetModel( void ) override;
	virtual void SetSpawnProperties( void ) override;
	virtual Vector GetDestinationVector( const Vector& destOrigin ) override;
	virtual void SetPVSData( void ) override;

public:
	const byte* GetPVSData( void );

public:
	static bool CheckSkyboxVisibility( const edict_t* pedict );
	static void ClearSkyboxEntity( void );

private:
	byte*	m_pPVSData;
	Vector	m_lastPVSOrigin;

private:
	static CEnvPosSky* g_pSkyEntity;
};
#endif //ENVPOSSKY_H