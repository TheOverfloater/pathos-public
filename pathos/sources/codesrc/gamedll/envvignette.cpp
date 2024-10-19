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
CEnvVignette::CEnvVignette(edict_t* pedict) :
    CPointEntity(pedict),
    m_isActive(false),
    m_vignetteStrength(0.5f),
    m_vignetteRadius(0.5f)
{
}
//=============================================
// @brief
//
//=============================================
CEnvVignette::~CEnvVignette(void)
{
}
//=============================================
// @brief
//
//=============================================
void CEnvVignette::DeclareSaveFields(void)
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_vignetteStrength, EFIELD_FLOAT));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvVignette, m_vignetteRadius, EFIELD_FLOAT));
}
//=============================================
// @brief
//
//=============================================
bool CEnvVignette::KeyValue(const keyvalue_t& kv)
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
    else
        return CPointEntity::KeyValue(kv);
}
//=============================================
// @brief
//
//=============================================
bool CEnvVignette::Spawn(void)
{
    if (!CPointEntity::Spawn())
        return false;
    if (HasSpawnFlag(FL_START_ON))
        m_isActive = true;
    return true;
}
//=============================================
// @brief
//
//=============================================
void CEnvVignette::SendInitMessage(const CBaseEntity* pPlayer)
{
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
void CEnvVignette::CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value)
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