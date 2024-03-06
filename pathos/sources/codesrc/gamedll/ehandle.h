/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef EHANDLE_H
#define EHANDLE_H

/*
=======================
CEntityHandle

=======================
*/
class CEntityHandle
{
public:
	CEntityHandle( void );
	explicit CEntityHandle( edict_t* pentity );
	CEntityHandle( CBaseEntity* pentity );

public:
	// Returns a pointer to the edict
	edict_t* get( void );
	// Sets the pointer
	edict_t* set( edict_t* pedict );
	// Resets the object
	void reset( void );

	// Boolean check operator
	operator bool() const;

	// Entity pointer return operator
	operator CBaseEntity*();
	// Not equals operator
	bool operator != (const CEntityHandle& entity) const;
	// Not equals operator
	bool operator != (const CBaseEntity* pEntity) const;
	// Equals operator
	bool operator == (const CEntityHandle& entity) const;
	// Equals operator
	bool operator == (const CBaseEntity* pEntity) const;
	// Assignment operator
	CBaseEntity* operator=(CBaseEntity* pEntity);
	// Assignment operator
	edict_t* operator=(edict_t* pEntity);
	// Accessor operator
	CBaseEntity* operator->() const;

private:
	// Pointer to edict
	edict_t* m_pEdict;
	// Edict identifier
	Uint32 m_identifier;
};
#endif //EHANDLE_H