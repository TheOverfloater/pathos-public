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
CEnvChromatic::CEnvChromatic(edict_t* pedict) :
    CPointEntity(pedict),
    m_isActive(false),
    m_chromaticStrength(0.01f) // Default chromatic aberration strength
{
}
//=============================================
// @brief
//
//=============================================
CEnvChromatic::~CEnvChromatic(void)
{
}
//=============================================
// @brief
//
//=============================================
void CEnvChromatic::DeclareSaveFields(void)
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvChromatic, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvChromatic, m_chromaticStrength, EFIELD_FLOAT));
}
//=============================================
// @brief
//
//=============================================
bool CEnvChromatic::KeyValue(const keyvalue_t& kv)
{
    if (!qstrcmp(kv.keyname, "strength"))
    {
        m_chromaticStrength = SDL_atof(kv.value);
        return true;
    }
    else
        return CPointEntity::KeyValue(kv);
}
//=============================================
// @brief
//
//=============================================
bool CEnvChromatic::Spawn(void)
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
void CEnvChromatic::SendInitMessage(const CBaseEntity* pPlayer)
{
    if (pPlayer && !m_isActive)
        return;
    if (pPlayer)
        gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.chromatic, nullptr, pPlayer->GetEdict()); // Assuming g_usermsgs.chromatic is defined
    else
        gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.chromatic, nullptr, nullptr);
    gd_engfuncs.pfnMsgWriteByte(m_isActive);
    if (m_isActive)
    {
        gd_engfuncs.pfnMsgWriteFloat(m_chromaticStrength);
    }
    gd_engfuncs.pfnUserMessageEnd();
}
//=============================================
// @brief
//
//=============================================
void CEnvChromatic::CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value)
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