/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

//
// studiomdl.c: generates a studio .mdl file from a .qc script
// models/<scriptname>.mdl.
//

#ifndef MAIN_H
#define MAIN_H

#include "datatypes.h"
#include "vector.h"
#include "constants.h"

enum particle_lightflags_t
{
	PARTICLE_LIGHTCHECK_NONE		= 0,
	PARTICLE_LIGHTCHECK_NORMAL		= (1<<0),
	PARTICLE_LIGHTCHECK_SCOLOR		= (1<<1),
	PARTICLE_LIGHTCHECK_MIXP		= (1<<2),
	PARTICLE_LIGHTCHECK_INTENSITY	= (1<<3),
	PARTICLE_LIGHTCHECK_ONLYONCE	= (1<<4)
};
enum prt_system_shape_e
{
	shape_point = 0,
	shape_box,
	shape_playerplane
};

enum prt_alignment_e
{
	align_tiled = 0,
	align_parallel,
	align_normal,
	align_tracer
};

enum prt_rendermode_e
{
	render_additive = 0,
	render_alpha,
	render_texture,
	render_distort
};

enum prt_collision_e
{
	collide_none = 0,
	collide_die,
	collide_bounce,
	collide_decal,
	collide_stuck,
	collide_new_system
};

enum prt_windtype_e
{
	wind_none = 0,
	wind_linear,
	wind_sine
};

struct system_definition_t
{
	system_definition_t():
		shapetype(0),
		randomdir(0),
		collide_exp(0),
		collide_bmodels(0),
		nofog(0),
		globs(0),
		globsize(0),
		numglobparticles(0),
		attachment(0),
		radius(0),
		minvel(0),
		maxvel(0),
		maxofs(0),
		skyheight(0),
		spawntime(0),
		fadeintime(0),
		softoffbegintime(0),
		fadeoutdelay(0),
		velocitydamp(0),
		stuckdie(0),
		tracerdist(0),
		maxheight(0),
		windx(0),
		windy(0),
		windvar(0),
		windmult(0),
		minwindmult(0),
		windmultvar(0),
		windtype(0),
		attachflags(PARTICLE_ATTACH_NONE),
		maxlife(0),
		maxlifevar(0),
		systemsize(0),
		transitiondelay(0),
		transitiontime(0),
		transitionvar(0),
		rotationvar(0),
		rotationvel(0),
		rotationdamp(0),
		rotationdampdelay(0),
		rotxvar(0),
		rotxvel(0),
		rotxdamp(0),
		rotxdampdelay(0),
		rotyvar(0),
		rotyvel(0),
		rotydamp(0),
		rotydampdelay(0),
		scale(0),
		scalevar(0),
		scaledampdelay(0),
		scaledampfactor(0),
		veldampdelay(0),
		gravity(0),
		particlefreq(0),
		impactdamp(0),
		mainalpha(0),
		minlight(0),
		maxlight(0),
		startparticles(0),
		maxparticles(0),
		maxparticlevar(0),
		skybox(0),
		overbright(0),
		lightcheck(0),
		collision(0),
		colwater(0),
		displaytype(0),
		rendermode(0),
		nocull(0),
		spawnchance(0),
		numspawns(0),
		softoff(0),
		softofftime(0),
		fadedistfar(0),
		fadedistnear(0),
		numframes(0),
		framesizex(0),
		framesizey(0),
		framerate(0)
		{
		}

	Int16 shapetype;
	Int16 randomdir;
	Int16 collide_exp;
	Int16 collide_bmodels;
	Int16 nofog;

	Int16 globs;
	Int16 globsize;
	Int16 numglobparticles;

	byte attachment;

	Vector origin;
	Vector dir;
	Float radius;

	Float minvel;
	Float maxvel;
	Float maxofs;

	Float skyheight;

	Double spawntime;
	Double softoffbegintime;

	Float fadeintime;
	Float fadeoutdelay;
	Float velocitydamp;
	Float stuckdie;
	Float tracerdist;

	Float maxheight;

	Float windx;
	Float windy;
	Float windvar;
	Float windmult;
	Float minwindmult;
	Float windmultvar;
	Int16 windtype;

	Int16 attachflags;

	Float maxlife;
	Float maxlifevar;
	Float systemsize;

	Vector primarycolor;
	Vector secondarycolor;
	Float transitiondelay;
	Float transitiontime;
	Float transitionvar;
	
	Float rotationvar;
	Float rotationvel;
	Float rotationdamp;
	Float rotationdampdelay;

	Float rotxvar;
	Float rotxvel;
	Float rotxdamp;
	Float rotxdampdelay;

	Float rotyvar;
	Float rotyvel;
	Float rotydamp;
	Float rotydampdelay;

	Float scale;
	Float scalevar;
	Float scaledampdelay;
	Float scaledampfactor;
	Float veldampdelay;
	Float gravity;
	Float particlefreq;
	Float impactdamp;
	Float mainalpha;
	Float minlight;
	Float maxlight;

	Uint16 startparticles;
	Int16 maxparticles;
	Uint16 maxparticlevar;

	Int16 skybox;
	Int16 overbright;
	Int16 lightcheck;
	Int16 collision;
	Int16 colwater;
	Int16 displaytype;
	Int16 rendermode;
	Int16 nocull;
	Int16 spawnchance;
	Uint64 numspawns;
	Int16 softoff;
	Float softofftime;

	Int16 fadedistfar;
	Int16 fadedistnear;

	Int16 numframes;
	Int16 framesizex;
	Int16 framesizey;
	Int16 framerate;

	CString create;
	CString deathcreate;
	CString watercreate;

	CString texturepath;
};

extern bool CreateDirectory( const Char* dirPath );
#endif