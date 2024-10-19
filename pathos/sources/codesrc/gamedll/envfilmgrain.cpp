#include "includes.h"
#include "gd_includes.h"
#include "envfilmgrain.h"
#include "player.h"
// Link the entity to its class
LINK_ENTITY_TO_CLASS(env_filmgrain, CEnvFilmGrain);
//=============================================
// @brief
//=============================================
CEnvFilmGrain::CEnvFilmGrain(edict_t* pedict) :
    CPointEntity(pedict),
    m_isActive(false),
    m_grainStrength(0.05f)
{
}
//=============================================
// @brief
//=============================================
CEnvFilmGrain::~CEnvFilmGrain(void)
{
}
//=============================================
// @brief
//=============================================
void CEnvFilmGrain::DeclareSaveFields(void)
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvFilmGrain, m_isActive, EFIELD_BOOLEAN));
    DeclareSaveField(DEFINE_DATA_FIELD(CEnvFilmGrain, m_grainStrength, EFIELD_FLOAT));
}
//=============================================
// @brief
//=============================================
bool CEnvFilmGrain::KeyValue(const keyvalue_t& kv)
{
    if (!qstrcmp(kv.keyname, "strength"))
    {
        m_grainStrength = SDL_atof(kv.value);
        return true;
    }
    else
        return CPointEntity::KeyValue(kv);
}
//=============================================
// @brief
//=============================================
bool CEnvFilmGrain::Spawn(void)
{
    if (!CPointEntity::Spawn())
        return false;
    if (HasSpawnFlag(FL_START_ON))
        m_isActive = true;
    return true;
}
//=============================================
// @brief
//=============================================
void CEnvFilmGrain::SendInitMessage(const CBaseEntity* pPlayer)
{
    if (pPlayer && !m_isActive)
        return;
    if (pPlayer)
        gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.filmgrain, nullptr, pPlayer->GetEdict());
    else
        gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.filmgrain, nullptr, nullptr);
    gd_engfuncs.pfnMsgWriteByte(m_isActive);
    if (m_isActive)
    {
        gd_engfuncs.pfnMsgWriteFloat(m_grainStrength);
    }
    gd_engfuncs.pfnUserMessageEnd();
}
//=============================================
// @brief
//=============================================
void CEnvFilmGrain::CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value)
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