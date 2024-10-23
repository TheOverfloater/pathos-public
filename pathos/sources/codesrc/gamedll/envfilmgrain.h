/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Code by valina354

#ifndef ENVFILMGRAIN
#define ENVFILMGRAIN

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvFilmGrain : public CPointEntity
{
public:
    enum
    {
        FL_START_ON = (1 << 1)
    };

public:
    explicit CEnvFilmGrain( edict_t* pedict );
    virtual ~CEnvFilmGrain(void );

public:
    virtual bool Spawn(void) override;
    virtual void DeclareSaveFields(void) override;
    virtual bool KeyValue(const keyvalue_t& kv) override;
    virtual void CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value) override;
    virtual void SendInitMessage(const CBaseEntity* pPlayer) override;

private:
    bool m_isActive;
    Float m_grainStrength;
};
#endif // ENVFILMGRAIN