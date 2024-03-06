/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVSHOOTER_H
#define ENVSHOOTER_H

#include "gibshooter.h"

//=============================================
//
//=============================================
class CEnvShooter : public CGibShooter
{
public:
	enum shootersnds_t
	{
		SND_NONE = -1,
		SND_GLASS = 0,
		SND_WOOD,
		SND_METAL,
		SND_FLESH,
		SND_ROCKS
	};

public:
	explicit CEnvShooter( edict_t* pedict );
	virtual ~CEnvShooter( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;

public:
	virtual CGib* CreateGib( void ) override;

public:
	string_t m_modelName;
};
#endif //ENVSHOOTER_H