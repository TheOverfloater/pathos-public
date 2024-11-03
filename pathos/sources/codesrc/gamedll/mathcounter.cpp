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
#include "mathcounter.h"

// Default values
const Int32 CMathCounter::DEFAULT_START_VALUE = 0;
const Int32 CMathCounter::DEFAULT_INCREMENT = 1;
const Int32 CMathCounter::DEFAULT_MAX_VALUE = 100;
const Int32 CMathCounter::DEFAULT_MIN_VALUE = 0;

// Link the entity to its class
LINK_ENTITY_TO_CLASS(math_counter, CMathCounter);

//=============================================
// @brief Constructor
//=============================================
CMathCounter::CMathCounter( edict_t* pedict ) :
    CPointEntity(pedict),
    m_value(DEFAULT_START_VALUE),
    m_maxValue(DEFAULT_MAX_VALUE),
    m_minValue(DEFAULT_MIN_VALUE),
    m_originalStartValue(m_value),
    m_operationToggleTrigger(ADD),
    m_operationOnTrigger(ADD),
    m_operationOffTrigger(ADD)
{
    for(Uint32 i = 0; i < NB_USEMODES; i++)
        m_incrementValues[i] = DEFAULT_START_VALUE;
}

//=============================================
// @brief Destructor
//=============================================
CMathCounter::~CMathCounter( void )
{
}

//=============================================
// @brief Declare save fields
//=============================================
void CMathCounter::DeclareSaveFields( void )
{
    // Call base class to do it first
    CPointEntity::DeclareSaveFields();

    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_value, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_originalStartValue, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_maxValue, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_minValue, EFIELD_INT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_operationToggleTrigger, EFIELD_UINT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_operationOnTrigger, EFIELD_UINT32));
    DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_operationOffTrigger, EFIELD_UINT32));

    for(Uint32 i = 0; i < NB_USEMODES; i++)
        DeclareSaveField(DEFINE_DATA_FIELD(CMathCounter, m_incrementValues[i], EFIELD_INT32));
}

//=============================================
// @brief Handle key-value pairs
//=============================================
bool CMathCounter::KeyValue( const keyvalue_t& kv )
{
    if (!qstrcmp(kv.keyname, "startvalue"))
    {
        m_value = SDL_atoi(kv.value);
        m_originalStartValue = m_value;
        return true;
    }
    // USE_OFF
    if(!qstrcmp(kv.keyname, "increment_off"))
    {
        m_incrementValues[UM_OFF] = SDL_atoi(kv.value);
        return true;
    }
    // USE_ON
    else if(!qstrcmp(kv.keyname, "increment_on"))
    {
        m_incrementValues[UM_ON] = SDL_atoi(kv.value);
        return true;
    }
    // USE_TOGGLE
    else if(!qstrcmp(kv.keyname, "increment_toggle"))
    {
        m_incrementValues[UM_TOGGLE] = SDL_atoi(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "maxvalue"))
    {
        m_maxValue = SDL_atoi(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "minvalue"))
    {
        m_minValue = SDL_atoi(kv.value);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "operation_toggle"))
    {
        Int32 op = SDL_atoi(kv.value);
        if (op >= ADD && op <= NB_OPERATIONS)
            m_operationToggleTrigger = static_cast<operation_t>(op);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "operation_on"))
    {
        Int32 op = SDL_atoi(kv.value);
        if (op >= ADD && op <= NB_OPERATIONS)
            m_operationOnTrigger = static_cast<operation_t>(op);
        return true;
    }
    else if (!qstrcmp(kv.keyname, "operation_off"))
    {
        Int32 op = SDL_atoi(kv.value);
        if (op >= ADD && op <= NB_OPERATIONS)
            m_operationOffTrigger = static_cast<operation_t>(op);
        return true;
    }
    else
        return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief Spawn the entity
//=============================================
bool CMathCounter::Spawn( void )
{
    if (!CPointEntity::Spawn())
        return false;

    if (m_value < m_minValue)
        m_value = m_minValue;
    else if (m_value > m_maxValue)
        m_value = m_maxValue;

    return true;
}

//=============================================
// @brief Increase or modify the counter value
//=============================================
void CMathCounter::IncrementValue( operation_t operation, Int32 increment )
{
    switch (operation)
    {
    case ADD:
        m_value += increment;
    break;
    case SUBTRACT:
        m_value -= increment;
    break;
    case DIVIDE:
        if (increment != 0)
            m_value /= increment;
    break;
    case MULTIPLY:
        m_value *= increment;
    break;
    }

    if (HasSpawnFlag(FL_TRIGGER_ON_MAX) && m_value > m_maxValue)
    {
        UseTargets(m_activator, USE_TOGGLE, 0);

        if (HasSpawnFlag(FL_RESET_ON_USE))
            m_value = m_originalStartValue;
    }
    else if (HasSpawnFlag(FL_TRIGGER_ON_MIN) && m_value < m_minValue)
    {
        if (HasSpawnFlag(FL_RESET_ON_USE))
            m_value = m_originalStartValue;

        UseTargets(m_activator, USE_TOGGLE, 0);
    }
}

//=============================================
// @brief Call the use function
//=============================================
void CMathCounter::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
    m_activator = pActivator;

    Int32 increment;
    Uint32 operation;
    switch (useMode)
    {
    case USE_ON:
        operation = m_operationOnTrigger;
        increment = m_incrementValues[UM_ON];
        break;
    case USE_OFF:
        operation = m_operationOffTrigger;
        increment = m_incrementValues[UM_OFF];
        break;
    case USE_TOGGLE:
    default:
        operation = m_operationToggleTrigger;
        increment = m_incrementValues[UM_TOGGLE];
        break;
    }

    IncrementValue(static_cast<operation_t>(operation), increment);
}