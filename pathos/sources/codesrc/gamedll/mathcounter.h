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

    enum operation_t
    {
        ADD = 0,
        SUBTRACT = 1,
        DIVIDE = 2,
        MULTIPLY = 3,

        // Must be last
        NB_OPERATIONS
    };

    enum modes_t
    {
        UM_OFF = 0,
        UM_ON,
        UM_TOGGLE,

        NB_USEMODES
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
    void IncrementValue( operation_t operation, Int32 increment );
    void TriggerTarget( void );

private:
    Int32 m_value;
    Int32 m_incrementValues[NB_USEMODES]; // Final values to be saved
    Int32 m_maxValue;
    Int32 m_minValue;
    Int32 m_originalStartValue;

    Uint32 m_operationToggleTrigger;
    Uint32 m_operationOnTrigger;
    Uint32 m_operationOffTrigger;

    CEntityHandle m_activator;
};

#endif // MATHCOUNTER