/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVSPRITE_H
#define ENVSPRITE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSprite : public CBaseEntity
{
public:
	enum
	{
		FL_START_ON		= (1<<0),
		FL_PLAY_ONCE	= (1<<1),
		FL_TEMPORARY	= (1<<2),
		FL_KEEP_ANGLES	= (1<<3),
		FL_KEEP_SERVER	= (1<<4)
	};

public:
	explicit CEnvSprite( edict_t* pedict );
	virtual ~CEnvSprite( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool Restore( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override;
	virtual void PreAddPacket( void ) override;
	virtual void InitEntity( void ) override;

public:
	void EXPORTFN AnimateThink( void );
	void EXPORTFN ExpandThink( void );
	void EXPORTFN AnimateOnceThink( void );

	void Animate( Float frameadd );
	void AnimateAndDie( Float framerate );
	void Expand( Float scalespeed, Float fadespeed );
	void SpriteInit( const Char* pstrSpriteName, const Vector& origin );
	void SetAttachment( CBaseEntity* pEntity, Int32 attachment );
	void SetTransparency( rendermode_t rendermode, Int32 r, Int32 g, Int32 b, Int32 alpha, Int32 renderfx );
	void SetTexture( Int32 spritemodel );

	void TurnOff( void );
	void TurnOn( void );

	Float GetMaxFrame( void ) const;

public:
	static CEnvSprite* CreateSprite( const Char* pstrSpriteName, const Vector& origin, bool animate );

private:
	Double m_lastAnimTime;
	Double m_dieTime;
	Float m_maxFrame;
	Uint32 m_angularFOV;
	string_t m_attachEntity;
	Uint32 m_attachmentIndex;
};
#endif //ENVSPRITE_H