/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRAINCOPY_H
#define FUNCTRAINCOPY_H

#include "functrain.h"

//=============================================
//
//=============================================
class CFuncTrainCopy : public CFuncTrain
{
public:
	explicit CFuncTrainCopy( edict_t* pedict );
	virtual ~CFuncTrainCopy( void );

public:
	virtual bool TrainSetModel( void ) override;
	virtual void InitEntity( void ) override;
	virtual bool IsBrushModel( void ) const override;
};
#endif //FUNCTRAINCOPY_H
