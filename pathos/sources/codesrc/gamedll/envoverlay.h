/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVOVERLAY
#define ENVOVERLAY

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvOverlay : public CPointEntity
{
public:
    enum setmode_t
    {
        MODE_SET = 0,
        MODE_REMOVE
    };

public:
    explicit CEnvOverlay( edict_t* pedict );
    virtual ~CEnvOverlay(void );

public:
    virtual void DeclareSaveFields(void) override;
    virtual bool Spawn( void ) override;
    virtual bool KeyValue(const keyvalue_t& kv) override;
    virtual void CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value) override;

private:
    Int32 m_setMode;
    Int32 m_layerIndex;
    string_t m_textureName;
    Int32 m_renderMode;
    Int32 m_effect;
    Float m_effectSpeed;
    Float m_effectMinAlpha;
    Float m_fadeTime;
};
#endif // ENVFILMGRAIN