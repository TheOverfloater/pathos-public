/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cvar.h"

//=============================================
// @brief Default constructor
//
//=============================================
CCVar::CCVar( cvar_type_t type, Int32 flags, const Char* pstrName, pfnCVarCallback_t pfnCallback, pfnCon_EPrintf_t pfnCon_EPrintf ):
	m_type(type),
	m_flags(flags),
	m_name(pstrName),
	m_pfnCallbackFn(pfnCallback),
	m_pfnCon_EPrintf(pfnCon_EPrintf)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CCVar::~CCVar()
{
}

//=============================================
// @brief Default constructor
//
//=============================================
CStringCVar::CStringCVar( Int32 flags, const Char* pstrName, pfnCVarCallback_t pfnCallback, pfnCon_EPrintf_t pfnCon_EPrintf ):
	CCVar(CVAR_STRING, flags, pstrName, pfnCallback, pfnCon_EPrintf)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CStringCVar::~CStringCVar()
{
}

//=============================================
// @brief Sets the value of the cvar
//
//=============================================
bool CStringCVar::SetValue( const Char* pstrValue )
{
	m_value = pstrValue;
	if(m_pfnCallbackFn)
		m_pfnCallbackFn(this);

	return true;
}

//=============================================
// @brief Gets the value of the cvar
//
//=============================================
const Char* CStringCVar::GetStrValue( void ) const
{
	return m_value.c_str();
}

//=============================================
// @brief Sets the value of the cvar
//
//=============================================
bool CStringCVar::SetValue( Float value )
{
	m_pfnCon_EPrintf("Tried to set float value on string cvar %s.\n", m_name.c_str());
	return false;
}

//=============================================
// @brief Gets the value of the cvar
//
//=============================================
Float CStringCVar::GetValue( void ) const
{
	m_pfnCon_EPrintf("Tried to get float value on string cvar %s.\n", m_name.c_str());
	return 0;
}

//=============================================
// @brief Default constructor
//
//=============================================
CFloatCVar::CFloatCVar( Int32 flags, const Char* pstrName, pfnCVarCallback_t pfnCallback, pfnCon_EPrintf_t pfnCon_EPrintf ):
	CCVar(CVAR_FLOAT, flags, pstrName, pfnCallback, pfnCon_EPrintf),
	m_value(0.0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CFloatCVar::~CFloatCVar()
{
}

//=============================================
// @brief Sets the value of the cvar
//
//=============================================
bool CFloatCVar::SetValue( const Char* pstrValue )
{
	m_pfnCon_EPrintf("Tried to set string value on float cvar %s.\n", m_name.c_str());
	return false;
}

//=============================================
// @brief Gets the value of the cvar
//
//=============================================
const Char* CFloatCVar::GetStrValue( void ) const
{
	m_pfnCon_EPrintf("Tried to get string value on float cvar %s.\n", m_name.c_str());
	return nullptr;
}

//=============================================
// @brief Sets the value of the cvar
//
//=============================================
bool CFloatCVar::SetValue( Float value )
{
	m_value = value;

	if(m_pfnCallbackFn)
		m_pfnCallbackFn(this);

	return true;
}

//=============================================
// @brief Gets the value of the cvar
//
//=============================================
Float CFloatCVar::GetValue( void ) const
{
	return m_value;
}

