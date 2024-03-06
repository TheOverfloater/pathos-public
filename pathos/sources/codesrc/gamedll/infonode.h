/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef INFONODE_H
#define INFONODE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CInfoNode : public CPointEntity
{
public:
	// spawnflags
	enum
	{
		SF_DISABLE_OPTIMIZATION = (1<<0)
	};

public:
	explicit CInfoNode( edict_t* pedict );
	virtual ~CInfoNode( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;

protected:
	virtual bool IsAirNode( void );

private:
	Int32 m_hintType;
	Int32 m_activity;
	Float m_range;
};
#endif //INFONODE_H