/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envoverlay.h"
#include "player.h"

// Link the entity to its class
LINK_ENTITY_TO_CLASS(env_overlay, CEnvOverlay);

//=============================================
// @brief
//=============================================
CEnvOverlay::CEnvOverlay( edict_t* pedict ) :
    CPointEntity(pedict),
    m_setMode(0),
    m_layerIndex(0),
    m_textureName(NO_STRING_VALUE),
    m_renderMode(OVERLAY_RENDER_NORMAL),
    m_effect(OVERLAY_EFFECT_NONE),
    m_effectSpeed(0),
    m_effectMinAlpha(0),
    m_fadeTime(0)
{
}

//=============================================
// @brief
//=============================================
CEnvOverlay::~CEnvOverlay( void )
{
}

//=============================================
// @brief
//=============================================
void CEnvOverlay::DeclareSaveFields( void )
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();

    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_setMode, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_layerIndex, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_textureName, EFIELD_STRING));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_renderMode, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_effect, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_effectSpeed, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_effectMinAlpha, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvOverlay, m_fadeTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//=============================================
bool CEnvOverlay::Spawn( void )
{
    if (!CPointEntity::Spawn())
        return false;

    if(m_setMode != MODE_REMOVE && m_textureName == NO_STRING_VALUE)
    {
        Util::EntityConPrintf(m_pEdict, "No overlay texture was set.\n");
        Util::RemoveEntity(m_pEdict);
        return false;
    }

    if(m_effectSpeed == 0)
        m_effectSpeed = 1.0;

    if(m_effectMinAlpha < 0)
        m_effectMinAlpha = 0;
    else if(m_effectMinAlpha > 1.0)
        m_effectMinAlpha = 1.0;

    return true;
}

//=============================================
// @brief
//=============================================
bool CEnvOverlay::KeyValue( const keyvalue_t& kv )
{
    if (!qstrcmp(kv.keyname, "setmode"))
    {
        Int32 value = SDL_atoi(kv.value);
        switch(value)
        {
        case 0:
            m_setMode = MODE_SET;
            break;
        case 1:
            m_setMode = MODE_REMOVE;
            break;
        default:
            Util::EntityConPrintf(m_pEdict, "Invalid value '%s' for keyvalue '%s'.\n", kv.value, kv.keyname);
            m_setMode = MODE_SET;
            break;
        }

        return true;
    }
    else if (!qstrcmp(kv.keyname, "layerindex"))
    {
        m_layerIndex = SDL_atoi(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "texturename"))
    {
        m_textureName = gd_engfuncs.pfnAllocString(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "layer_rendermode"))
    {
        Int32 value = SDL_atoi(kv.value);
        switch(value)
        {
        case 0:
            m_renderMode = OVERLAY_RENDER_NORMAL;
            break;
        case 1:
            m_renderMode = OVERLAY_RENDER_ADDITIVE;
            break;
        case 2:
            m_renderMode = OVERLAY_RENDER_ALPHATEST;
            break;
        case 3:
            m_renderMode = OVERLAY_RENDER_ALPHABLEND;
            break;
        default:
            Util::EntityConPrintf(m_pEdict, "Invalid value '%s' for keyvalue '%s'.\n", kv.value, kv.keyname);
            m_renderMode = OVERLAY_RENDER_NORMAL;
            break;
        }

        return true;
    }
    else if (!qstrcmp(kv.keyname, "layer_effect"))
    {
        Int32 value = SDL_atoi(kv.value);
        switch(value)
        {
        case 0:
            m_effect = OVERLAY_EFFECT_NONE;
            break;
        case 1:
            m_effect = OVERLAY_EFFECT_PULSATE;
            break;
        default:
            Util::EntityConPrintf(m_pEdict, "Invalid value '%s' for keyvalue '%s'.\n", kv.value, kv.keyname);
            m_effect = OVERLAY_EFFECT_NONE;
            break;
        }
        return true;
    }
    else if (!qstrcmp(kv.keyname, "effectspeed"))
    {
        m_effectSpeed = SDL_atof(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "effectminalpha"))
    {
        m_effectMinAlpha = SDL_atof(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "fadetime"))
    {
        m_fadeTime = SDL_atof(kv.value);
        return true;
    }
    else
        return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//=============================================
void CEnvOverlay::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
    CBaseEntity* pPlayer = Util::GetHostPlayer();
    if(!pPlayer)
        return;

    switch(m_setMode)
    {
    case MODE_SET:
        {
            pPlayer->SetScreenOverlay(m_layerIndex, gd_engfuncs.pfnGetString(m_textureName), static_cast<overlay_rendermode_t>(m_renderMode),
                m_pState->rendercolor, m_pState->renderamt, static_cast<overlay_effect_t>(m_effect), m_effectSpeed, m_effectMinAlpha, m_fadeTime);
        }
        break;
    case MODE_REMOVE:
        {
            pPlayer->ClearOverlay(m_layerIndex, m_fadeTime);
        }
        break;
    }
}