/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCPLATROT_H
#define FUNCPLATROT_H

#include "funcplat.h"

//=============================================
//
//=============================================
class CFuncPlatRot : public CFuncPlat
{
public:
	enum
	{
		FL_ROTATE_Z = (1<<6),
		FL_ROTATE_X = (1<<7)
	};
public:
	explicit CFuncPlatRot( edict_t* pedict );
	virtual ~CFuncPlatRot( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;

public:
	virtual void GoUp( void ) override;
	virtual void GoDown( void ) override;
	virtual void HitTop( void ) override;
	virtual void HitBottom( void ) override;

	void RotateMove( const Vector& destAngle, Double time );
	void SetupRotation( void );

public:
	Vector m_startAngles;
	Vector m_endAngles;
};
#endif //FUNCPLATROT_H