/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AMBIENTGENERIC_H
#define AMBIENTGENERIC_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAmbientGeneric : public CPointEntity
{
public:
	enum
	{
		FL_RADIUS_EVERYWHERE	= (1<<0),
		FL_RADIUS_SMALL			= (1<<1),
		FL_RADIUS_MEDIUM		= (1<<2),
		FL_RADIUS_LARGE			= (1<<3),
		FL_START_SILENT			= (1<<4),
		FL_NOT_LOOPING			= (1<<5),
		FL_RADIUS_XXL			= (1<<6),
		FL_RADIUS_XL			= (1<<7),
		FL_RADIUS_XS			= (1<<8),
		FL_REVERBLESS			= (1<<9),
		FL_NO_OCCLUSION			= (1<<10),
		FL_DIM_OTHERS			= (1<<11),
		FL_MUTE_OTHERS			= (1<<12)
	};
public:
	explicit CAmbientGeneric( edict_t* pedict );
	virtual ~CAmbientGeneric( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void InitEntity( void ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
	void EXPORTFN TurnOffThink( void );

	void PlaySound( void );
	void StopSound( void );
	void ApplyTurnOnEffects( void );
	void ApplyTurnOffEffects( void );

private:
	bool m_isActive;
	bool m_isLooping;

	Int32 m_sndFlags;
	Float m_soundRadius;
	Float m_attenuation;

	Float m_volume;
	Float m_startVolume;
	Int32 m_pitch;
	Int32 m_startPitch;

	Float m_volumeFadeInTime;
	Float m_volumeFadeOutTime;

	Float m_pitchFadeInTime;
	Float m_pitchFadeOutTime;

	Double m_beginTime;
	Double m_turnoffBeginTime;
	Double m_turnoffEndTime;

	string_t m_emitterEntityName;
	CEntityHandle m_emitterEntity;
};
#endif //AMBIENTGENERIC_H