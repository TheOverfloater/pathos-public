#ifndef ENVBLACKWHITE
#define ENVBLACKWHITE
#include "pointentity.h"
//=============================================
//
//=============================================
class CEnvBlackWhite : public CPointEntity
{
public:
    enum
    {
        FL_START_ON = (1 << 1)
    };
public:
    explicit CEnvBlackWhite(edict_t* pedict);
    virtual ~CEnvBlackWhite(void);
public:
    virtual bool Spawn(void) override;
    virtual void DeclareSaveFields(void) override;
    virtual bool KeyValue(const keyvalue_t& kv) override;
    virtual void CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value) override;
    virtual void SendInitMessage(const CBaseEntity* pPlayer) override;
private:
    bool m_isActive;
    Float m_blackwhiteStrength;
};
#endif //ENVBLACKWHITE