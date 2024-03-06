/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BASEENTITY_INLINE_H
#define BASEENTITY_INLINE_H

#include "util.h"
#include "gamevars.h"

//=============================================
// @brief
//
//=============================================
inline entindex_t CBaseEntity::GetEntityIndex( void ) const
{
	return m_pState->entindex;
}

//=============================================
// @brief
//
//=============================================
inline const edict_t* CBaseEntity::GetEdict( void ) const
{
	return m_pEdict;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetOrigin( void ) const
{
	return m_pEdict->state.origin;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetAngles( void ) const
{
	return m_pEdict->state.angles;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetOrigin( const Vector& origin )
{
	m_pState->origin = origin;
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetAngles( const Vector& angles )
{
	m_pState->angles = angles;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetVelocity( void ) const
{
	return m_pState->velocity;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetVelocity( const Vector& velocity )
{
	m_pState->velocity = velocity;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetAngularVelocity( void ) const
{
	return m_pState->avelocity;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetAngularVelocity( const Vector& velocity )
{
	m_pState->avelocity = velocity;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetBaseVelocity( void ) const
{
	return m_pState->basevelocity;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetBaseVelocity( const Vector& basevelocity )
{
	m_pState->basevelocity = basevelocity;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetFlags( Uint64 flagbits )
{
	m_pState->flags |= flagbits;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::RemoveFlags( Uint64 flagbits )
{
	m_pState->flags &= ~flagbits;
}

//=============================================
// @brief
//
//=============================================
inline Int64 CBaseEntity::GetFlags( void ) const
{
	return m_pState->flags;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::HasTargetName( void ) const
{
	return (m_pFields->targetname == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetTargetName( void ) const
{
	if(m_pFields->targetname == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->targetname);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetTargetName( const Char* pstrtargetname )
{
	if(!pstrtargetname || !qstrlen(pstrtargetname))
		m_pFields->targetname = NO_STRING_VALUE;
	else
		m_pFields->targetname = gd_engfuncs.pfnAllocString(pstrtargetname);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetTargetName( const string_t targetname )
{
	m_pFields->targetname = targetname;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::HasModelName( void ) const
{
	return (m_pFields->modelname == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetModelName( void ) const
{
	if(m_pFields->modelname == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->modelname);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetModelName( const Char* pstrmodelname )
{
	if(!pstrmodelname || !qstrlen(pstrmodelname))
		m_pFields->modelname = NO_STRING_VALUE;
	else
		m_pFields->modelname = gd_engfuncs.pfnAllocString(pstrmodelname);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetModelName( const string_t modelname )
{
	m_pFields->modelname = modelname;
}

//=============================================
// @brief
//
//=============================================
inline Int32 CBaseEntity::GetModelIndex( void ) const
{
	return m_pState->modelindex;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::HasTarget( void ) const
{
	return (m_pFields->target == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetTarget( void ) const
{
	if(m_pFields->target == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->target);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetTarget( const Char* pstrtarget )
{
	if(!pstrtarget || !qstrlen(pstrtarget))
		m_pFields->target = NO_STRING_VALUE;
	else
		m_pFields->target = gd_engfuncs.pfnAllocString(pstrtarget);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetTarget( const string_t target )
{
	m_pFields->target = target;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetClassName( void ) const
{
	if(m_pFields->classname == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->classname);
}

//=============================================
// @brief
//
//=============================================
inline takedamage_t CBaseEntity::GetTakeDamage( void ) const
{
	return (takedamage_t)m_pState->takedamage;
}

//=============================================
// @brief
//
//=============================================
inline solid_t CBaseEntity::GetSolidity( void ) const
{
	return (solid_t)m_pState->solid;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetSolidity( solid_t solid )
{
	m_pState->solid = solid;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetAbsMins( void ) const
{
	return m_pState->absmin;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetAbsMaxs( void ) const
{
	return m_pState->absmax;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetMinsMaxs( const Vector& mins, const Vector& maxs )
{
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetMins( void ) const
{
	return m_pState->mins;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetMaxs( void ) const
{
	return m_pState->maxs;
}


//=============================================
// @brief
//
//=============================================
inline Int32 CBaseEntity::GetButtonBits( void ) const
{
	return m_pState->buttons;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetButtonBits( Int32 bitflags )
{
	m_pState->buttons |= bitflags;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::ClearButtonBits( Int32 bitflags )
{
	m_pState->buttons &= ~bitflags;
}

//=============================================
// @brief
//
//=============================================
inline Vector CBaseEntity::GetViewOffset( bool fromNavigable ) const
{
	if(!fromNavigable)
	{
		return m_pState->view_offset;
	}
	else
	{
		Vector checkPosition = m_pState->origin + m_pState->view_offset;
		checkPosition = checkPosition - GetNavigablePosition();
		return checkPosition;
	}
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetViewOffset( const Vector& offset )
{
	m_pState->view_offset = offset;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetViewAngles( void ) const
{
	return m_pState->viewangles;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetViewAngles( const Vector& angles )
{
	m_pState->viewangles = angles;
	if(IsPlayer())
		m_pState->fixangles = true;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetAiment( CBaseEntity* pEntity )
{
	if(!pEntity)
		m_pState->aiment = NO_ENTITY_INDEX;
	else
		m_pState->aiment = pEntity->GetEntityIndex();
}

//=============================================
// @brief
//
//=============================================
inline CBaseEntity* CBaseEntity::GetAiment( void ) const
{
	if(m_pState->aiment == NO_ENTITY_INDEX)
		return nullptr;

	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(m_pState->aiment);
	if(Util::IsNullEntity(pedict))
	{
		m_pState->aiment = NO_ENTITY_INDEX;
		return nullptr;
	}
	
	CBaseEntity* paiment = CBaseEntity::GetClass(pedict);
	return paiment;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetOwner( CBaseEntity* pEntity )
{
	if(!pEntity)
		m_pState->owner = NO_ENTITY_INDEX;
	else
		m_pState->owner = pEntity->GetEntityIndex();
}

//=============================================
// @brief
//
//=============================================
inline CBaseEntity* CBaseEntity::GetOwner( void ) const
{
	if(m_pState->owner == NO_ENTITY_INDEX)
		return nullptr;

	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(m_pState->owner);
	if(Util::IsNullEntity(pedict))
	{
		m_pState->owner = NO_ENTITY_INDEX;
		return nullptr;
	}
	
	CBaseEntity* powner = CBaseEntity::GetClass(pedict);
	return powner;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetGroundEntity( CBaseEntity* pEntity )
{
	if(!pEntity)
		m_pState->groundent = NO_ENTITY_INDEX;
	else
		m_pState->groundent = pEntity->GetEntityIndex();
}

//=============================================
// @brief
//
//=============================================
inline CBaseEntity* CBaseEntity::GetGroundEntity( void ) const
{
	if(m_pState->groundent == NO_ENTITY_INDEX)
		return nullptr;

	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(m_pState->groundent);
	if(Util::IsNullEntity(pedict))
	{
		m_pState->groundent = NO_ENTITY_INDEX;
		return nullptr;
	}
	
	CBaseEntity* pgroundent = CBaseEntity::GetClass(pedict);
	return pgroundent;
}

//=============================================
// @brief
//
//=============================================
inline waterlevel_t CBaseEntity::GetWaterLevel( void ) const
{
	return (waterlevel_t)m_pState->waterlevel;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetWaterLevel( waterlevel_t level )
{
	m_pState->waterlevel = level;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetSize( void ) const
{
	return m_pState->size;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetSize( const Vector& size )
{
	gd_engfuncs.pfnSetSize(m_pEdict, size);
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetPunchAngle( void ) const
{
	return m_pState->punchangles;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetPunchAngle( const Vector& value )
{
	m_pState->punchangles = value;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetPunchAmount( void ) const
{
	return m_pState->punchamount;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetPunchAmount( const Vector& value )
{
	m_pState->punchamount = value;
}

//=============================================
// @brief
//
//=============================================
inline Int64 CBaseEntity::GetSpawnFlags( void ) const
{
	return m_pState->spawnflags;
}

//=============================================
// @brief
//
//=============================================
bool CBaseEntity::HasSpawnFlag( Int64 bit ) const
{ 
	return (m_pState->spawnflags & bit) ? true : false; 
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetSpawnFlag( Int64 flag )
{
	m_pState->spawnflags |= flag;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::RemoveSpawnFlag( Int64 flag )
{
	m_pState->spawnflags &= ~flag;
}

//=============================================
// @brief
//
//=============================================
inline const Vector& CBaseEntity::GetRenderColor( void ) const
{
	return m_pState->rendercolor;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetRenderColor( const Vector& color )
{
	m_pState->rendercolor.x = clamp(color.x, 0, 255);
	m_pState->rendercolor.y = clamp(color.y, 0, 255);
	m_pState->rendercolor.z = clamp(color.z, 0, 255);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetRenderColor( Int32 r, Int32 g, Int32 b )
{
	m_pState->rendercolor.x = clamp(r, 0, 255);
	m_pState->rendercolor.y = clamp(g, 0, 255);
	m_pState->rendercolor.z = clamp(b, 0, 255);
}

//=============================================
// @brief
//
//=============================================
inline rendermode_t CBaseEntity::GetRenderMode( void ) const
{
	return (rendermode_t)m_pState->rendermode;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetRenderMode( rendermode_t rendermode )
{
	m_pState->rendermode = rendermode;
}

//=============================================
// @brief
//
//=============================================
inline const Float CBaseEntity::GetRenderAmount( void ) const
{
	return m_pState->renderamt;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetRenderAmount( Float amount )
{
	m_pState->renderamt = clamp(amount, 0, 255);
}

//=============================================
// @brief
//
//=============================================
inline const Int32 CBaseEntity::GetRenderFx( void ) const
{
	return m_pState->renderfx;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetRenderFx( Int32 renderfx )
{
	m_pState->renderfx = renderfx;
}
 
//=============================================
// @brief Gets the render type
//
//=============================================
inline const rendertype_t CBaseEntity::GetRenderType( void ) const
{
	return (rendertype_t)m_pState->rendertype;
}

//=============================================
// @brief
//
//=============================================
inline Int64 CBaseEntity::GetEffectFlags( void ) const
{
	return m_pState->effects;
}

//=============================================
// @brief
//
//=============================================
bool CBaseEntity::HasEffectFlag( Int64 bit ) const
{ 
	return (m_pState->effects & bit) ? true : false; 
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetEffectFlag( Int64 flag )
{
	m_pState->effects |= flag;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::RemoveEffectFlag( Int64 flag )
{
	m_pState->effects &= ~flag;
}

//=============================================
// @brief
//
//=============================================
inline Float CBaseEntity::GetSpeed( void ) const
{
	return m_pState->speed;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetSpeed( Float speed )
{
	m_pState->speed = speed;
}

//=============================================
// @brief
//
//=============================================
Float CBaseEntity::GetFallingVelocity( void ) const
{
	return m_pState->fallvelocity;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::DropToFloor( void )
{
	return gd_engfuncs.pfnDropToFloor(m_pEdict);
}

//=============================================
// @brief
//
//=============================================
inline CBaseEntity* CBaseEntity::GetParent( void ) const
{
	if(m_pState->parent == NO_ENTITY_INDEX)
		return nullptr;

	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(m_pState->parent);
	if(Util::IsNullEntity(pedict))
	{
		m_pState->groundent = NO_ENTITY_INDEX;
		return nullptr;
	}
	
	CBaseEntity* pparent = CBaseEntity::GetClass(pedict);
	return pparent;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetParent( CBaseEntity* pEntity )
{
	if(pEntity == nullptr)
	{
		m_pState->parent = NO_ENTITY_INDEX;
		m_pState->flags &= ~FL_PARENTED;
	}
	else
	{
		m_pState->parent = pEntity->GetEntityIndex();
		m_pState->flags |= FL_PARENTED;
	}
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::IsParented( void ) const
{
	return (m_pState->parent == NO_ENTITY_INDEX) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline Int32 CBaseEntity::GetSkin( void ) const
{
	return m_pState->skin;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetSkin( Int32 value )
{
	m_pState->skin = value;
}

//=============================================
// @brief
//
//=============================================
inline Int64 CBaseEntity::GetBody( void ) const
{
	return m_pState->body;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetBody( Int64 value )
{
	m_pState->body = value;
}

//=============================================
// @brief
//
//=============================================
inline Float CBaseEntity::GetHealth( void ) const
{
	return m_pState->health;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetHealth( Float value )
{
	m_pState->health = value;
}

//=============================================
// @brief Returns the max health value
//
//=============================================
inline Float CBaseEntity::GetMaxHealth( void ) const
{
	return m_pState->maxhealth;
}

//=============================================
// @brief Sets the max health value
//
//=============================================
inline void CBaseEntity::SetMaxHealth( Float value )
{
	m_pState->maxhealth = value;
}

//=============================================
// @brief
//
//=============================================
inline Float CBaseEntity::GetArmorValue( void ) const
{
	return m_pState->armorvalue;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetArmorValue( Float value )
{
	m_pState->armorvalue = value;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::HasNetname( void ) const
{
	return (m_pFields->netname == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetNetname( void ) const
{
	if(m_pFields->netname == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->netname);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetNetname( const Char* pstrValue )
{
	m_pFields->netname = gd_engfuncs.pfnAllocString(pstrValue);
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::HasMessage( void ) const
{
	return (m_pFields->message == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetMessage( void ) const
{
	if(m_pFields->message == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->message);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetMessage( const Char* pstrValue )
{
	m_pFields->message = gd_engfuncs.pfnAllocString(pstrValue);
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::HasGlobalName( void ) const
{
	return (m_pFields->globalname == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CBaseEntity::GetGlobalName( void ) const
{
	if(m_pFields->globalname == NO_STRING_VALUE)
		return "";
	else
		return gd_engfuncs.pfnGetString(m_pFields->globalname);
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetGlobalName( const Char* pstrValue )
{
	m_pFields->globalname = gd_engfuncs.pfnAllocString(pstrValue);
}

//=============================================
// @brief
//
//=============================================
inline const Float CBaseEntity::GetScale( void ) const
{
	return m_pState->scale;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetScale( Float scale )
{
	m_pState->scale = scale;
}

//=============================================
// @brief
//
//=============================================
inline movetype_t CBaseEntity::GetMoveType( void ) const
{
	return (movetype_t)m_pState->movetype;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetMoveType( movetype_t movetype )
{
	m_pState->movetype = movetype;
}

//=============================================
// @brief
//
//=============================================
inline Double CBaseEntity::GetNextThinkTime( void ) const
{
	return m_pState->nextthink;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetNextThinkTime( Double thinktime )
{
	m_pState->nextthink = thinktime;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetNextThink( Double delay )
{
	if(!delay)
		m_pState->nextthink = 0;
	else
		m_pState->nextthink = g_pGameVars->time + delay;
}

//=============================================
// @brief
//
//=============================================
inline Double CBaseEntity::GetLocalTime( void ) const
{
	return m_pState->ltime;
}

//=============================================
// @brief
//
//=============================================
inline Float CBaseEntity::GetGravity( void ) const
{
	return m_pState->gravity;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetGravity( Float gravity )
{
	m_pState->gravity = gravity;
}

//=============================================
// @brief
//
//=============================================
inline Float CBaseEntity::GetFriction( void ) const
{
	return m_pState->friction;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetFriction( Float friction )
{
	m_pState->friction = friction;
}

//=============================================
// @brief
//
//=============================================
inline Float CBaseEntity::GetPlaneZCap( void ) const
{
	return m_pState->planezcap;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetPlaneZCap( Float planeZCap )
{
	m_pState->planezcap = planeZCap;
}

//=============================================
// @brief
//
//=============================================
inline deathstate_t CBaseEntity::GetDeadState( void ) const
{
	return (deathstate_t)m_pState->deadstate;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetDeadState( deathstate_t state )
{
	m_pState->deadstate = state;
}

//=============================================
// @brief Returns the weapons value
//
//=============================================
inline Int64 CBaseEntity::GetWeapons( void ) const
{
	return m_pState->weapons;
}

//=============================================
// @brief Sets the weapons value
//
//=============================================
inline void CBaseEntity::SetWeapons( Int64 weapons )
{
	m_pState->weapons = weapons;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::SetModel( const Char* pstrModelName, bool setbounds )
{
	if(gd_engfuncs.pfnSetModel(m_pEdict, pstrModelName, setbounds))
	{
		PostModelSet();
		return true;
	}
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
inline bool CBaseEntity::SetModel( string_t modelNameString, bool setbounds )
{
	if(modelNameString == NO_STRING_VALUE)
		return false;

	if(gd_engfuncs.pfnSetModel(m_pEdict, gd_engfuncs.pfnGetString(modelNameString), setbounds))
	{
		PostModelSet();
		return true;
	}
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
inline const Float CBaseEntity::GetIdealYaw( void ) const
{
	return m_pState->idealyaw;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetIdealYaw( Float idealyaw )
{
	m_pState->idealyaw = idealyaw;
}

//=============================================
// @brief
//
//=============================================
inline const Float CBaseEntity::GetFramerate( void ) const
{
	return m_pState->framerate;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetFramerate( Float framerate )
{
	m_pState->framerate = framerate;
}

//=============================================
// @brief
//
//=============================================
inline const Float CBaseEntity::GetFrame( void ) const
{
	return m_pState->frame;
}

//=============================================
// @brief
//
//=============================================
inline void CBaseEntity::SetFrame( Float frame )
{
	m_pState->frame = frame;
}

// 
//=============================================
// @brief Gets the animation time
//
//=============================================
inline const Double CBaseEntity::GetAnimationTime( void ) const
{
	return m_pState->animtime;
}

//=============================================
// @brief Sets the animation time
//
//=============================================
inline void CBaseEntity::SetAnimationTime( Double animtime )
{
	m_pState->animtime = animtime;
}

//=============================================
// @brief Tells if the entity is a visible entity
//
//=============================================
inline bool CBaseEntity::IsVisible( void ) const
{
	return (!m_pState->modelindex  || (m_pState->effects & EF_NODRAW)) ? false : true;
}

#endif //BASEENTITY_INLINE_H