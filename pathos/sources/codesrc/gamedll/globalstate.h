/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include "globalstate_shared.h"

struct globalstate_t
{
	globalstate_t():
		state(GLOBAL_OFF)
		{}

	CString globalstate;
	CString mapname;
	globalstate_state_t state;
};

//=============================================
//
//=============================================
class CGlobalState
{
public:
	CGlobalState( void );
	virtual ~CGlobalState( void );

public:
	void Reset( void );
	void Clear( void );
	void Dump( void );

	void SetGlobalState( const Char* pstrglobalstate, globalstate_state_t state );
	void DeleteGlobalState( const Char* pstrglobalstate );
	const globalstate_t* GetGlobalState( const Char* pstrglobalstate );
	globalstate_state_t GetState( const Char* pstrglobalstate );
	bool IsGlobalStatePresent( const Char* pstrglobalstate );
	void UpdateGlobalStateMapName( const Char* pstrglobalstate );

public:
	Uint32 GetNbGlobalStates( void );
	void SaveGlobalStates( void );
	void ReadGlobalStateData( const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state );

public:
	CLinkedList<globalstate_t> m_globalStatesList;
};

extern CGlobalState gGlobalStates;

extern Uint32 GetNbGlobalStates( void );
extern void SaveGlobalStates( void );
extern void ReadGlobalStateData( const Char* pstrglobalname, const Char* pstrlevelname, globalstate_state_t state );
#endif //GLOBALSTATE_H