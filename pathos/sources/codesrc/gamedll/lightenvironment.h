/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LIGHTENVIRONMENT_H
#define LIGHTENVIRONMENT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CLightEnvironment : public CPointEntity
{
public:
	explicit CLightEnvironment( edict_t* pedict );
	virtual ~CLightEnvironment( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;

public:
	// Sunlight direction
	Vector m_sunlightDirection;
	// Sunlight color
	color24_t m_sunlightColor;
	// Sunlight intensity
	Int32 m_sunlightIntensity;
};

#endif //LIGHTENVIRONMENT_H