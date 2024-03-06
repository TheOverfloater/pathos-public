/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "pathtrack.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(path_track, CPathTrack);

//=============================================
// @brief
//
//=============================================
CPathTrack::CPathTrack( edict_t* pedict ):
	CPointEntity(pedict),
	m_length(0),
	m_alternateName(NO_STRING_VALUE),
	m_pNext(nullptr),
	m_pPrevious(nullptr),
	m_pAlternatePath(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CPathTrack::~CPathTrack( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPathTrack::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CPathTrack, m_length, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPathTrack, m_alternateName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CPathTrack, m_pNext, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPathTrack, m_pPrevious, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPathTrack, m_pAlternatePath, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CPathTrack::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "altpath"))
	{
		m_alternateName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CPathTrack::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(m_pFields->target != NO_STRING_VALUE && 
		!qstrcicmp(gd_engfuncs.pfnGetString(m_pFields->targetname),
		gd_engfuncs.pfnGetString(m_pFields->target)))
	{
		Util::EntityConPrintf(m_pEdict, "Entity is targeting itself.\n");
		Util::RemoveEntity(this);
		return false;
	}

	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPathTrack::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool isOn;
	if(m_pAlternatePath)
	{
		isOn = HasSpawnFlag(FL_ALTERNATE) ? false : true;
		if(ShouldToggle(useMode, isOn))
		{
			if(isOn)
				SetSpawnFlag(FL_ALTERNATE);
			else
				RemoveSpawnFlag(FL_ALTERNATE);
		}
	}
	else
	{
		isOn = HasSpawnFlag(FL_DISABLED) ? false : true;
		if(ShouldToggle(useMode, isOn))
		{
			if(isOn)
				SetSpawnFlag(FL_DISABLED);
			else
				RemoveSpawnFlag(FL_DISABLED);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPathTrack::InitEntity( void )
{
	// Link up this entity
	Link();
}

//=============================================
// @brief
//
//=============================================
void CPathTrack::SetPrevious( CPathTrack* pPrevious )
{
	if(pPrevious && qstrcmp(pPrevious->GetTargetName(), gd_engfuncs.pfnGetString(m_alternateName)))
		m_pPrevious = pPrevious;
}

//=============================================
// @brief
//
//=============================================
void CPathTrack::Link( void )
{
	edict_t* pTargetEntity;
	if(m_pFields->target != NO_STRING_VALUE)
	{
		pTargetEntity = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
		if(!Util::IsNullEntity(pTargetEntity) && !qstrcmp(gd_engfuncs.pfnGetString(pTargetEntity->fields.classname), gd_engfuncs.pfnGetString(m_pFields->classname)))
		{
			m_pNext = reinterpret_cast<CPathTrack*>(CBaseEntity::GetClass(pTargetEntity));
			if(m_pNext)
				m_pNext->SetPrevious(this);
		}
		else
		{
			Util::EntityConPrintf(m_pEdict, "Dead end link: %s.\n", gd_engfuncs.pfnGetString(m_pFields->target));
		}
	}

	if(m_alternateName != NO_STRING_VALUE)
	{
		pTargetEntity = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_alternateName));
		if(!Util::IsNullEntity(pTargetEntity) && !qstrcmp(gd_engfuncs.pfnGetString(pTargetEntity->fields.classname), gd_engfuncs.pfnGetString(m_pFields->classname)))
		{
			m_pAlternatePath = reinterpret_cast<CPathTrack*>(CBaseEntity::GetClass(pTargetEntity));
			if(m_pAlternatePath)
				m_pAlternatePath->SetPrevious(this);
		}
		else
		{
			Util::EntityConPrintf(m_pEdict, "Dead end link: %s.\n", gd_engfuncs.pfnGetString(m_alternateName));
		}
	}
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CPathTrack::GetValidPath( CPathTrack* pPath, bool testflag )
{
	if(!pPath)
		return nullptr;

	if(testflag && pPath->HasSpawnFlag(FL_DISABLED))
		return nullptr;

	return pPath;
}

//=============================================
// @brief
//
//=============================================
void CPathTrack::Project( CPathTrack* pStart, CPathTrack* pEnd, Vector& origin, Float dist )
{
	if(!pStart || !pEnd)
		return;

	Vector direction = (pEnd->GetOrigin()-pStart->GetOrigin());
	direction.Normalize();

	origin = pEnd->GetOrigin() + direction * dist;
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CPathTrack::GetLookAhead( Vector& origin, Float dist, bool test )
{
	CPathTrack* pcurrent = this;
	Float originaldist = dist;
	Vector currentpos = origin;

	Float _dist = dist;
	if(_dist < 0)
	{
		_dist = -_dist;
		while(_dist > 0)
		{
			Vector direction = pcurrent->GetOrigin() - currentpos;
			Float length = direction.Length();
			if(!length)
			{
				if(!GetValidPath(pcurrent->GetPrevious(), test))
				{
					if(!test)
						Project(pcurrent->GetNext(), pcurrent, origin, _dist);

					return nullptr;
				}
			}
			else if(length < _dist)
			{
				origin = currentpos + (direction*(_dist/length));
				return pcurrent;
			}
			else
			{
				_dist -= length;
				currentpos = pcurrent->GetOrigin();
				origin = currentpos;

				if(!GetValidPath(pcurrent->GetPrevious(), test))
					return nullptr;

				pcurrent = pcurrent->GetPrevious();
			}
		}

		origin = currentpos;
		return pcurrent;
	}
	else
	{
		while(_dist > 0)
		{
			if(!GetValidPath(pcurrent->GetNext(), test))
			{
				if(!test)
					Project(pcurrent->GetPrevious(), pcurrent, origin, _dist);

				return nullptr;
			}

			Vector direction = pcurrent->GetNext()->GetOrigin() - currentpos;
			Float length = direction.Length();

			if(!length && !GetValidPath(pcurrent->GetNext()->GetNext(), test))
			{
				if(_dist == originaldist)
					return nullptr;
				else
					return pcurrent;
			}

			if(length > _dist)
			{
				origin = currentpos + (direction*(_dist/length));
				return pcurrent;
			}
			else
			{
				_dist -= length;
				currentpos = pcurrent->GetNext()->GetOrigin();
				pcurrent = pcurrent->GetNext();
				origin = currentpos;
			}
		}
		origin = currentpos;
	}

	return pcurrent;
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CPathTrack::GetNearest( const Vector& origin )
{
	Vector delta = origin - m_pState->origin;
	delta.z = 0;

	Float mindist = delta.Length();
	CPathTrack* pnearest = this;
	CPathTrack* ppath = GetNext();

	Uint32 deadcount = 0;
	while(ppath && ppath != this)
	{
		// Avoid infinite loop
		deadcount++;
		if(deadcount > 512)
		{
			Util::EntityConPrintf(m_pEdict, "Bad sequence of path_tracks from %s.\n", gd_engfuncs.pfnGetString(m_pFields->targetname));
			return nullptr;
		}

		delta = origin - ppath->GetOrigin();
		delta.z = 0;

		Float dist = delta.Length();
		if(dist < mindist)
		{
			mindist = dist;
			pnearest = ppath;
		}

		ppath = ppath->GetNext();
	}

	return pnearest;
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CPathTrack::GetNext( void )
{
	if(m_pAlternatePath && HasSpawnFlag(FL_ALTERNATE)
		&& !HasSpawnFlag(FL_ALT_REVERSE))
		return m_pAlternatePath;
	
	return m_pNext;
}

//=============================================
// @brief
//
//=============================================
CPathTrack* CPathTrack::GetPrevious( void )
{
	if(m_pAlternatePath && HasSpawnFlag(FL_ALTERNATE)
		&& HasSpawnFlag(FL_ALT_REVERSE))
		return m_pAlternatePath;
	
	return m_pPrevious;
}