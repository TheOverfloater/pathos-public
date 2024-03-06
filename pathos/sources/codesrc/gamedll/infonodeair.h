/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef INFONODEAIR_H
#define INFONODEAIR_H

#include "infonode.h"

//=============================================
//
//=============================================
class CInfoNodeAir : public CInfoNode
{
public:
	explicit CInfoNodeAir( edict_t* pedict );
	virtual ~CInfoNodeAir( void );

public:
	virtual bool IsAirNode( void ) override;
};
#endif //INFONODEAIR_H