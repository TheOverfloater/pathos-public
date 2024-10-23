/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

// Code by valina354

#ifndef MATHCOUNTER
#define MATHCOUNTER

#include "pointentity.h"

//=============================================
// Math Counter class
//=============================================
class CMathCounter : public CPointEntity
{
public:
    // Default values
    static const int DEFAULT_START_VALUE;
    static const int DEFAULT_INCREMENT;
    static const int DEFAULT_MAX_VALUE;
    static const int DEFAULT_MIN_VALUE;

    enum Operation
    {
        ADD = 0,
        SUBTRACT = 1,
        DIVIDE = 2,
        MULTIPLY = 3
    };

public:
    explicit CMathCounter( edict_t* pedict );
    virtual ~CMathCounter( void );

public:
    // Spawn flags
    enum
    {
        FL_RESET_ON_USE = (1 << 0),  // Resets counter to start value on use
        FL_TRIGGER_ON_MAX = (1 << 1),  // Triggers target when value exceeds max value
        FL_TRIGGER_ON_MIN = (1 << 2),  // Triggers target when value drops below min value
    };

public:
    virtual bool Spawn( void ) override;
    virtual void DeclareSaveFields( void ) override;
    virtual bool KeyValue( const keyvalue_t& kv ) override;
    virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
    void IncrementValue( void );
    void TriggerTarget( void );

private:
    int m_value;
    int m_increment;
    int m_maxValue;
    int m_minValue;
    CString m_target;
    Operation m_operation;
    CEntityHandle m_activator;
};

#endif // MATHCOUNTER