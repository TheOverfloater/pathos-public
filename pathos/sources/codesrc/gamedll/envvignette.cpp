/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Code by valina354

#include "includes.h"
#include "gd_includes.h"
#include "envvignette.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_vignette, CEnvVignette);

//=============================================
// @brief
//
//=============================================
CEnvVignette::CEnvVignette( edict_t* pedict ) :
    CPointEntity(pedict),
    m_isActive(false),
    m_vignetteStrength(0.5f),
    m_vignetteRadius(0.5f),
    m_globalTriggerMode(GTM_TOGGLE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvVignette::~CEnvVignette( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvVignette::DeclareSaveFields( void )
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();

    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_vignetteStrength, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_vignetteRadius, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_globalTriggerMode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CEnvVignette::KeyValue( const keyvalue_t& kv )
{
    if (!qstrcmp(kv.keyname, "strength"))
    {
        m_vignetteStrength = SDL_atof(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "radius"))
    {
        m_vignetteRadius = SDL_atof(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "globaltriggermode"))
    {
        Int32 value = SDL_atoi(kv.value);
        switch(value)
        {
        case 0:
            m_globalTriggerMode = GTM_TOGGLE;
            break;
        case 1:
            m_globalTriggerMode = GTM_ON;
            break;
        case 2:
            m_globalTriggerMode = GTM_OFF;
            break;
        default:
            Util::EntityConPrintf(m_pEdict, "Keyvalue '%s' has invalid value '%s'.\n", kv.keyname, kv.value);
            m_globalTriggerMode = GTM_TOGGLE;
            break;
        }

        return true;
    }
    else
        return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CEnvVignette::Spawn( void )
{
    if (!CPointEntity::Spawn())
        return false;

    if (!HasSpawnFlag(FL_GLOBAL_EFFECT) && HasSpawnFlag(FL_START_ON))
        m_isActive = true;

    return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvVignette::SendInitMessage( const CBaseEntity* pPlayer )
{
    if(HasSpawnFlag(FL_GLOBAL_EFFECT))
        return;

    if (pPlayer && !m_isActive)
        return;

    if (pPlayer)
        gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.vignette, nullptr, pPlayer->GetEdict());
    else
        gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.vignette, nullptr, nullptr);

    gd_engfuncs.pfnMsgWriteByte(m_isActive);
    if (m_isActive)
    {
        gd_engfuncs.pfnMsgWriteFloat(m_vignetteStrength);
        gd_engfuncs.pfnMsgWriteFloat(m_vignetteRadius);
    }
    gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CEnvVignette::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
    if(HasSpawnFlag(FL_GLOBAL_EFFECT))
    {
        CBaseEntity* pPlayer = Util::GetHostPlayer();
        if(pPlayer)
        {
            bool prevState = pPlayer->IsVignetteEffectActive();

            bool desiredState = false;
            switch (m_globalTriggerMode)
            {
            case GTM_ON:
                desiredState = true;
                break;
            case GTM_OFF:
                desiredState = false;
                break;
            case GTM_TOGGLE:
                desiredState = !prevState;
                break;
            }

            if(desiredState != prevState)
            {
                if(!desiredState)
                    pPlayer->SetVignetteEffect(false, 0, 0);
                else
                    pPlayer->SetVignetteEffect(true, m_vignetteRadius, m_vignetteStrength);
            }
        }
    }
    else
    {
        bool prevstate = m_isActive;
        switch (useMode)
        {
        case USE_ON:
            m_isActive = true;
            break;
        case USE_OFF:
            m_isActive = false;
            break;
        case USE_TOGGLE:
            m_isActive = !m_isActive;
            break;
        }

        if (m_isActive != prevstate)
            SendInitMessage(nullptr);
    }
}