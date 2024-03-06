/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_DECALS_H
#define R_DECALS_H

#include "decallist.h"
#include "constants.h"

struct cached_decal_t
{
	cached_decal_t():
		pentry(nullptr),
		flags(0),
		leafnum(0),
		entityindex(NO_ENTITY_INDEX),
		life(0),
		fadetime(0),
		growthtime(0)
		{}

	CString name;
	decalgroupentry_t* pentry;

	Vector	origin;
	Vector	normal;

	Int32 flags;
	Int32 leafnum;
	entindex_t entityindex;

	Float life;
	Float fadetime;
	Float growthtime;
};

/*
=================================
CDecalManager

=================================
*/
class CDecalManager
{
public:
	// Distance which decals trace against
	static const Float DECAL_CHECK_DIST;
	// Decal texture folder path
	static const Char DECAL_TEXTURE_PATH[];

public:
	CDecalManager( void );
	~CDecalManager( void );

public:
	// Initializes the class
	void Init( void );

	// Initializes the class
	bool InitGame( void );
	// Clears the class
	void ClearGame( void );

	// Adds a cached decal
	void AddCached( decalgroupentry_t* pentry, const Char *pstrname, const Vector& origin, const Vector& normal, Int32 flags, entindex_t entityindex = NO_ENTITY_INDEX, Float life = 0, Float fadetime = 0, Float growthtime = 0 );
	// Creates cached decals
	void CreateCached( void );

	// Draws bogus decals
	bool DrawBogusDecals( void );

public:
	// Loads a decal texture
	static void LoadTexture( decalgroupentry_t *pentry );
	// Precaches a group's textures
	void PrecacheGroup( const Char *szgroupname );
	// Precaches a texture by name
	void PrecacheTexture( const Char *sztexturename );

	// Returns the decal list
	CDecalList& GetDecalList( void ) { return m_decalList; }

private:
	// Finds a leaf for a decal
	static void FindLeaf( cached_decal_t *pdecal );
	// Sends a decal to the server
	void SendDecalToServer( cached_decal_t& decal );

private:
	// List of cached decals
	CLinkedList<cached_decal_t> m_cachedDecalsList;

	// Array of bogus decal positions
	CLinkedList<Vector> m_bogusDecalsList;
	// Cvar for drawing bogus decals
	CCVar* m_pCvarShowBogusDecals;

	// Decal list object
	CDecalList m_decalList;

	// Spawn client decal usermsg
	Uint32 m_spawnClientDecalMsgId;
};
extern CDecalManager gDecals;

// Utility function for cutting polygons
extern Uint32 Decal_ClipPolygon( const Vector *arrIn, Int32 numpoints, const Vector& normal, const Vector& planepoint, Vector *arrOut );
#endif