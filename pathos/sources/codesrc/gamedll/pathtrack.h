/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PATHTRACK_H
#define PATHTRACK_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CPathTrack : public CPointEntity
{
public:
	enum
	{
		FL_DISABLED			= (1<<0),
		FL_FIRE_ONCE		= (1<<1),
		FL_ALT_REVERSE		= (1<<2),
		FL_DISABLE_TRAIN	= (1<<3),
		FL_ALTERNATE		= (1<<4)
	};
public:
	explicit CPathTrack( edict_t* pedict );
	virtual ~CPathTrack( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void InitEntity( void ) override;
	virtual bool IsPathTrackEntity( void ) const override { return true; }

public:
	void SetPrevious( CPathTrack* pPrevious );
	void Link( void );
	
	CPathTrack* GetValidPath( CPathTrack* pPath, bool testflag );
	static void Project( CPathTrack* pStart, CPathTrack* pEnd, Vector& origin, Float dist );
	
public:
	CPathTrack* GetLookAhead( Vector& origin, Float dist, bool test );
	CPathTrack* GetNearest( const Vector& origin );

	CPathTrack* GetNext( void );
	CPathTrack* GetPrevious( void );

private:
	Float m_length;
	string_t m_alternateName;

	CPathTrack* m_pNext;
	CPathTrack* m_pPrevious;
	CPathTrack* m_pAlternatePath;
};
#endif //PATHTRACK_H