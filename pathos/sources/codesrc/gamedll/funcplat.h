/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCPLAT_H
#define FUNCPLAT_H

#include "plattrainentity.h"

//=============================================
//
//=============================================
class CFuncPlat : public CPlatTrainEntity
{
public:
	// Default T length
	static const Float DEFAULT_T_LENGTH;
	// Default T width
	static const Float DEFAULT_T_WIDTH;
	// Default speed
	static const Float DEFAULT_SPEED;
	// Default volume
	static const Float DEFAULT_VOLUME;
	// Return delay time
	static const Float RETURN_DELAY_TIME;

public:
	explicit CFuncPlat( edict_t* pedict );
	virtual ~CFuncPlat( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallBlocked( CBaseEntity* pBlocker ) override;

public:
	bool Setup( void );

	void EXPORTFN PlatUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );
	void EXPORTFN CallGoDown( void );
	void EXPORTFN CallHitTop( void );
	void EXPORTFN CallHitBottom( void );

	virtual void GoUp( void );
	virtual void GoDown( void );
	virtual void HitTop( void );
	virtual void HitBottom( void );
};
#endif //FUNCPLAT_H