/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DLIGHT_H
#define DLIGHT_H

class CDynamicLightManager;
struct shadowmap_t;

struct cl_dlight_t
{
friend class CDynamicLightManager;

public:
	cl_dlight_t():
		radius(0),
		spawntime(0),
		die(0),
		decay(0),
		decay_delay(0),
		cone_size(0),
		key(0),
		subkey(0),
		lastframe(0),
		lightstyle(0),
		textureindex(0),
		followentity(false),
		attachment(NO_POSITION),
		isstatic(false),
		noshadow(false),
		nomaincull(false),
		pshadowmap(nullptr),
		psmcubemap(nullptr),
		psceneinfo(nullptr),
		psceneinfo_nonstatic(nullptr),
		pfrustum(nullptr)
	{
	}
	const shadowmap_t* getProjShadowMap( void ) const
	{
		return pshadowmap;
	}
	const shadowmap_t* getCubeShadowMap( void ) const
	{
		return psmcubemap;
	}
	bool isStatic( void ) const
	{
		return isstatic;
	}
	bool noShadow( void ) const
	{
		return noshadow;
	}
	void setDontCull( void )
	{
		nomaincull = true;
	}

public:
	Vector	origin;
	Vector	color;
	Vector	angles;

	Float	radius;
	Double	spawntime;
	Double	die;
	Float	decay;
	Float	decay_delay;
	Float	cone_size;

	Int32		key;
	Int32		subkey;
	Int32		lastframe;
	Uint32		lightstyle;

	Uint32 textureindex;

	bool	followentity;
	Int32	attachment;

private:
	bool	isstatic;
	bool	noshadow;
	bool	nomaincull;

	struct shadowmap_t *pshadowmap;
	struct shadowmap_t *psmcubemap;

	struct dlight_sceneinfo_t* psceneinfo;
	struct dlight_sceneinfo_t* psceneinfo_nonstatic;

	Vector prevorigin;
	Vector prevangles;

public:
	class CFrustum* pfrustum;
};
#endif //DLIGHT_H