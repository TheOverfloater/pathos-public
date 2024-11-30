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
#include "envfilmgrain.h"
#include "player.h"

// Link the entity to its class
LINK_ENTITY_TO_CLASS(env_filmgrain, CEnvFilmGrain);

//=============================================
// @brief
//=============================================
CEnvFilmGrain::CEnvFilmGrain( edict_t* pedict ) :
    CPointEntity(pedict),
    m_isActive(false),
    m_grainStrength(0.05f),
    m_globalTriggerMode(GTM_TOGGLE)
{
}

//=============================================
// @brief
//=============================================
CEnvFilmGrain::~CEnvFilmGrain( void )
{
}

//=============================================
// @brief
//=============================================
void CEnvFilmGrain::DeclareSaveFields( void )
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();

    DeclareSaveField(DEFINE_DATA_FIELD(CEnvFilmGrain, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvFilmGrain, m_grainStrength, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvFilmGrain, m_globalTriggerMode, EFIELD_INT32));
}

//=============================================
// @brief
//=============================================
bool CEnvFilmGrain::KeyValue( const keyvalue_t& kv )
{
    if (!qstrcmp(kv.keyname, "strength"))
    {
        m_grainStrength = SDL_atof(kv.value);
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
//=============================================
bool CEnvFilmGrain::Spawn( void )
{
    if (!CPointEntity::Spawn())
        return false;

    if (!HasSpawnFlag(FL_GLOBAL_EFFECT) && HasSpawnFlag(FL_START_ON))
        m_isActive = true;

    return true;
}

//=============================================
// @brief
//=============================================
void CEnvFilmGrain::SendInitMessage( const CBaseEntity* pPlayer )
{
    if(HasSpawnFlag(FL_GLOBAL_EFFECT))
        return;

    if (pPlayer && !m_isActive)
        return;

    if (pPlayer)
        gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.filmgrain, nullptr, pPlayer->GetEdict());
    else
        gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.filmgrain, nullptr, nullptr);

    gd_engfuncs.pfnMsgWriteByte(m_isActive);
    if (m_isActive)
        gd_engfuncs.pfnMsgWriteFloat(m_grainStrength);

    gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//=============================================
void CEnvFilmGrain::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
    if(HasSpawnFlag(FL_GLOBAL_EFFECT))
    {
        CBaseEntity* pPlayer = Util::GetHostPlayer();
        if(pPlayer)
        {
            bool prevState = pPlayer->IsFilmGrainActive();

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
                    pPlayer->SetFilmGrain(false, 0);
                else
                    pPlayer->SetFilmGrain(true, m_grainStrength);
            }
        }
    }
    else
    {
        bool prevState = m_isActive;
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

        if (m_isActive != prevState)
            SendInitMessage(nullptr);
   }
}