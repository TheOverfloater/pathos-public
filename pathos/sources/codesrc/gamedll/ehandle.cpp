/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ehandle.h"
#include "baseentity.h"

//=============================================
// @brief
//
//=============================================
CEntityHandle::CEntityHandle( void ):
	m_pEdict(nullptr),
	m_identifier(0)
{
}

//=============================================
// @brief
//
//=============================================
CEntityHandle::CEntityHandle( edict_t* pentity ):
	m_pEdict(pentity),
	m_identifier(pentity->identifier)
{
}

//=============================================
// @brief
//
//=============================================
CEntityHandle::CEntityHandle( CBaseEntity* pentity ):
	m_pEdict(pentity->m_pEdict),
	m_identifier(pentity->m_pEdict->identifier)
{
}

//=============================================
// @brief
//
//=============================================
edict_t* CEntityHandle::get( void )
{
	if(m_pEdict && m_identifier == m_pEdict->identifier)
	{
		// Entity is still valid
		return m_pEdict;
	}
	else
	{
		reset();
		return nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
edict_t* CEntityHandle::set( edict_t* pedict )
{
	if(!pedict)
	{
		reset();
		return nullptr;
	}

	m_pEdict = pedict;
	m_identifier = pedict->identifier;
	return m_pEdict;
}

//=============================================
// @brief
//
//=============================================
void CEntityHandle::reset( void )
{
	m_pEdict = nullptr;
	m_identifier = 0;
}

//=============================================
// @brief
//
//=============================================
CEntityHandle::operator bool() const
{
	return (m_pEdict != nullptr 
		&& !m_pEdict->free 
		&& m_pEdict->identifier == m_identifier) ? true : false;
}

//=============================================
// @brief
//
//=============================================
CEntityHandle::operator CBaseEntity*()
{
	if(m_pEdict && !m_pEdict->free && m_identifier == m_pEdict->identifier)
		return CBaseEntity::GetClass(m_pEdict);
	else
		return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool CEntityHandle::operator != (const CEntityHandle& entity) const
{
	if(entity.m_pEdict == m_pEdict && entity.m_identifier == m_identifier && !entity.m_pEdict->free)
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
bool CEntityHandle::operator != (const CBaseEntity* pEntity) const
{
	const edict_t* pedict = pEntity->GetEdict();
	if(pedict == m_pEdict && pedict->identifier == m_identifier && !pedict->free)
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
bool CEntityHandle::operator == (const CEntityHandle& entity) const
{
	if(entity.m_pEdict == m_pEdict && entity.m_identifier == m_identifier && !entity.m_pEdict->free)
		return true;
	else
		return false;
}


//=============================================
// @brief
//
//=============================================
bool CEntityHandle::operator == (const CBaseEntity* pEntity) const
{
	const edict_t* pedict = nullptr;
	if(pEntity)
		pedict = pEntity->GetEdict();

	if(pedict == m_pEdict && pedict->identifier == m_identifier && !pedict->free)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CEntityHandle::operator=(CBaseEntity* pEntity)
{
	if(!pEntity)
	{
		m_pEdict = nullptr;
		m_identifier = 0;
	}
	else
	{
		m_pEdict = pEntity->m_pEdict;
		m_identifier = m_pEdict->identifier;
	}

	return pEntity;
}

//=============================================
// @brief
//
//=============================================
edict_t* CEntityHandle::operator=(edict_t* pEntity)
{
	m_pEdict = pEntity;
	if(m_pEdict)
		m_identifier = m_pEdict->identifier;
	else
		m_identifier = 0;

	return pEntity;
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CEntityHandle::operator->() const
{
	assert(m_pEdict != nullptr);
	return CBaseEntity::GetClass(m_pEdict);
}

