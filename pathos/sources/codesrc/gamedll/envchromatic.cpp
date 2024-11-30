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
#include "envchromatic.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_chromatic, CEnvChromatic);

//=============================================
// @brief
//
//=============================================
CEnvChromatic::CEnvChromatic( edict_t* pedict ) :
    CPointEntity(pedict),
    m_isActive(false),
    m_chromaticStrength(0.01f), // Default chromatic aberration strength
    m_globalTriggerMode(GTM_TOGGLE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvChromatic::~CEnvChromatic( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvChromatic::DeclareSaveFields( void )
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvChromatic, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvChromatic, m_chromaticStrength, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvChromatic, m_globalTriggerMode, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CEnvChromatic::KeyValue( const keyvalue_t& kv )
{
    if (!qstrcmp(kv.keyname, "strength"))
    {
        m_chromaticStrength = SDL_atof(kv.value);
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
bool CEnvChromatic::Spawn( void )
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
void CEnvChromatic::SendInitMessage( const CBaseEntity* pPlayer )
{
    if(HasSpawnFlag(FL_GLOBAL_EFFECT))
        return;

    if (pPlayer && !m_isActive)
        return;

    if (pPlayer)
        gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.chromatic, nullptr, pPlayer->GetEdict()); // Assuming g_usermsgs.chromatic is defined
    else
        gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.chromatic, nullptr, nullptr);

    gd_engfuncs.pfnMsgWriteByte(m_isActive);
    if (m_isActive)
        gd_engfuncs.pfnMsgWriteFloat(m_chromaticStrength);
    gd_engfuncs.pfnUserMessageEnd();
}
//=============================================
// @brief
//
//=============================================
void CEnvChromatic::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
    if(HasSpawnFlag(FL_GLOBAL_EFFECT))
    {
        CBaseEntity* pPlayer = Util::GetHostPlayer();
        if(pPlayer)
        {
            bool prevState = pPlayer->IsChromaticAberrationActive();

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
                    pPlayer->SetChromaticAberration(false, 0);
                else
                    pPlayer->SetChromaticAberration(true, m_chromaticStrength);
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