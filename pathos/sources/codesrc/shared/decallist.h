/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DECALLIST_H
#define DECALLIST_H

struct en_texture_t;

struct decalgroupentry_t
{
	decalgroupentry_t():
		ptexture(nullptr),
		xsize(0),
		ysize(0),
		pgroup(nullptr)
		{}

	CString name;
	en_texture_t *ptexture;

	Uint32 xsize;
	Uint32 ysize;
	struct decalgroup_t *pgroup;
};

struct decalgroup_t
{
	CString name;
	CArray<decalgroupentry_t> entries;
};

/*
=================================
CDecalList

=================================
*/
class CDecalList
{
public:
	CDecalList( void );
	~CDecalList( void );

public:
	// Loads the decal list
	bool LoadDecalList( const Char* pfile, Uint32 isize );
	// Clears the decal list
	void Clear( void );

public:
	// Returns a decal by it's name, irrespective of groups
	decalgroupentry_t *GetByName( const char *szdecalname );
	// Returns a random decal from a decal group
	decalgroupentry_t *GetRandom( const char *szgroupname );

	// Returns the group array
	decalgroup_t* GetGroup( const char *szgroupname );
	// Returns a group by it's index
	decalgroup_t* GetGroup( Uint32 index );
	// Returns the number of groups
	Uint32 GetNbGroups( void ) const;

private:
	// Array of decal groups
	CArray<decalgroup_t> m_decalGroupsArray;

	// Error string
	CString m_errorString;
};

#endif //DECAL_SHARED_H