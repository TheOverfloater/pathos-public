/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Code by valina354

#ifndef ENVCHROMATIC
#define ENVCHROMATIC

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvChromatic : public CPointEntity
{
public:
    enum
    {
        FL_START_ON         = (1 << 0),
        FL_GLOBAL_EFFECT    = (1 << 1)
    };
    enum globaltriggermode_t
    {
       GTM_TOGGLE = 0,
       GTM_ON,
       GTM_OFF
    };

public:
    explicit CEnvChromatic( edict_t* pedict );
    virtual ~CEnvChromatic( void );

public:
    virtual bool Spawn( void ) override;
    virtual void DeclareSaveFields( void ) override;
    virtual bool KeyValue( const keyvalue_t& kv ) override;
    virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
    virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
    bool m_isActive;
    Float m_chromaticStrength;
    Int32 m_globalTriggerMode;
};
#endif //ENVCHROMATIC