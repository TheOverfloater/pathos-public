#include "includes.h"
#include "gd_includes.h"
#include "envblackwhite.h"
#include "player.h"
// Link the entity to its class
LINK_ENTITY_TO_CLASS(env_blackwhite, CEnvBlackWhite);
//=============================================
// @brief
//
//=============================================
CEnvBlackWhite::CEnvBlackWhite(edict_t* pedict) :
    CPointEntity(pedict),
    m_isActive(false),
    m_blackwhiteStrength(1.0f)
{
}
//=============================================
// @brief
//
//=============================================
CEnvBlackWhite::~CEnvBlackWhite(void)
{
}
//=============================================
// @brief
//
//=============================================
void CEnvBlackWhite::DeclareSaveFields(void)
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackWhite, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackWhite, m_blackwhiteStrength, EFIELD_FLOAT));
}
//=============================================
// @brief
//
//=============================================
bool CEnvBlackWhite::KeyValue(const keyvalue_t& kv)
{
    if (!qstrcmp(kv.keyname, "strength"))
    {
        m_blackwhiteStrength = SDL_atof(kv.value);
        return true;
    }
    else
        return CPointEntity::KeyValue(kv);
}
//=============================================
// @brief
//
//=============================================
bool CEnvBlackWhite::Spawn(void)
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
void CEnvBlackWhite::SendInitMessage(const CBaseEntity* pPlayer)
{
    if (pPlayer && !m_isActive)
        return;
    if (pPlayer)
        gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.blackwhite, nullptr, pPlayer->GetEdict());
    else
        gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.blackwhite, nullptr, nullptr);
    gd_engfuncs.pfnMsgWriteByte(m_isActive);
    if (m_isActive)
    {
        gd_engfuncs.pfnMsgWriteFloat(m_blackwhiteStrength);
    }
    gd_engfuncs.pfnUserMessageEnd();
}
//=============================================
// @brief
//
//=============================================
void CEnvBlackWhite::CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value)
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