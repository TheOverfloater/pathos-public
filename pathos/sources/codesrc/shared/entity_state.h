/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENTITY_STATE_H
#define ENTITY_STATE_H

#include "constants.h"

enum movetype_t
{
	MOVETYPE_NONE = 0,
	MOVETYPE_WALK,
	MOVETYPE_STEP,
	MOVETYPE_PUSHSTEP,
	MOVETYPE_FLY,
	MOVETYPE_TOSS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_BOUNCE,
	MOVETYPE_FOLLOW
};

enum rendermode_t
{
	RENDER_NORMAL = 0,
	RENDER_TRANSCOLOR,
	RENDER_TRANSTEXTURE,
	RENDER_TRANSGLOW,
	RENDER_TRANSALPHA,
	RENDER_TRANSADDITIVE
};

enum solid_t
{
	SOLID_NOT = 0,
	SOLID_TRIGGER,
	SOLID_BBOX,
	SOLID_SLIDEBOX,
	SOLID_BSP
};

enum effects_t
{
	EF_NONE				= 0,
	EF_NODRAW			= (1<<0),
	EF_COLLISION		= (1<<1),
	EF_UNUSED1			= (1<<2),
	EF_CLIENTENT		= (1<<3),
	EF_ALTLIGHTORIGIN	= (1<<4),
	EF_STATICENTITY		= (1<<5),
	EF_DIMLIGHT			= (1<<6),
	EF_SHOULDERLIGHT	= (1<<7),
	EF_VBM_SLERP		= (1<<8),
	EF_FASTINTERP		= (1<<9),
	EF_NOLERP			= (1<<10),
	EF_INVLIGHT			= (1<<11),
	EF_NOELIGHTTRACE	= (1<<12),
	EF_LADDER			= (1<<13),
	EF_VIEWONLY			= (1<<14),
	EF_NOVIS			= (1<<15),
	EF_CONVEYOR			= (1<<16),
	EF_ALWAYS_SEND		= (1<<17),
	EF_TRACKANGLES		= (1<<18),
	EF_MUZZLEFLASH		= (1<<19),
	EF_UPDATEMODEL		= (1<<20),
	EF_QUAKEBUG_FIX		= (1<<21),
	EF_NOINTERP			= (1<<22),
	EF_SET_SEQTIME		= (1<<23)
};

enum waterlevel_t
{
	WATERLEVEL_NONE = 0,
	WATERLEVEL_LOW,
	WATERLEVEL_MID,
	WATERLEVEL_FULL
};


static const Uint64 FL_NONE				= 0;
static const Uint64 FL_KILLME			= (1ULL<<0);
static const Uint64 FL_BASEVELOCITY		= (1ULL<<1);
static const Uint64 FL_ONGROUND			= (1ULL<<2);
static const Uint64 FL_CONVEYOR			= (1ULL<<3);
static const Uint64 FL_ALWAYSTHINK		= (1ULL<<4);
static const Uint64 FL_NPC_CLIP			= (1ULL<<5);
static const Uint64 FL_WORLDBRUSH		= (1ULL<<6);
static const Uint64 FL_NPC				= (1ULL<<7);
static const Uint64 FL_CLIENT			= (1ULL<<8);
static const Uint64 FL_FLOAT			= (1ULL<<9);
static const Uint64 FL_FLY				= (1ULL<<10);
static const Uint64 FL_SWIM				= (1ULL<<11);
static const Uint64 FL_INWATER			= (1ULL<<12);
static const Uint64 FL_WATERJUMP		= (1ULL<<13);
static const Uint64 FL_DORMANT			= (1ULL<<14);
static const Uint64 FL_DUCKING			= (1ULL<<15);
static const Uint64 FL_FROZEN			= (1ULL<<16);
static const Uint64 FL_UNUSED1			= (1ULL<<17);
static const Uint64 FL_ON_LADDER		= (1ULL<<18);
static const Uint64 FL_DEAD				= (1ULL<<19);
static const Uint64 FL_ON_BIKE			= (1ULL<<20);
static const Uint64 FL_NO_SPRINT		= (1ULL<<21);
static const Uint64 FL_SLOWMOVE			= (1ULL<<22);
static const Uint64 FL_GODMODE			= (1ULL<<23);
static const Uint64 FL_NOTARGET			= (1ULL<<24);
static const Uint64 FL_PARENTED			= (1ULL<<25);
static const Uint64 FL_INITIALIZE		= (1ULL<<26);
static const Uint64 FL_REMOVE_ON_SPAWN	= (1ULL<<27);
static const Uint64 FL_PMOVE_IGNORE		= (1ULL<<28);
static const Uint64 FL_PARTIALGROUND	= (1ULL<<29);
static const Uint64 FL_GRAPH_ENTITY		= (1ULL<<30);
static const Uint64 FL_PARALYZED		= (1ULL<<31);
static const Uint64 FL_NO_HITBOX_TRACE	= (1ULL<<32);

enum entstate_bits_t
{
	U_NONE			= 0,
	U_ORIGIN		= (1<<0),
	U_ANGLES		= (1<<1),
	U_VELOCITY		= (1<<2),
	U_AVELOCITY		= (1<<3),
	U_PUNCHANGLES	= (1<<4),
	U_VIEWANGLES	= (1<<5),
	U_VIEWOFFSET	= (1<<6),
	U_POSITIONS		= (1<<7),
	U_MINSMAXS		= (1<<8),
	U_BASICS1		= (1<<9),
	U_BASICS2		= (1<<10),
	U_BASICS3		= (1<<11),
	U_BASICS4		= (1<<12),
	U_ANIMINFO		= (1<<13),
	U_CONTROLLERS	= (1<<14),
	U_BLENDING		= (1<<15),
	U_RENDERINFO	= (1<<16),
	U_ENTSINFO		= (1<<17),
	U_BASEVELOCITY	= (1<<18),
	U_PLAYERINFO	= (1<<19),
	U_IUSER			= (1<<20),
	U_VUSER1		= (1<<21),
	U_VUSER2		= (1<<22),
	U_VUSER3		= (1<<23),
	U_VUSER4		= (1<<24),
	U_FUSER			= (1<<25),
	U_PARENTING		= (1<<26),
	U_NEW_TO_PACKET	= (1<<27)
};

struct entity_state_t
{
	entity_state_t():
		entindex(0),
		msg_num(0),
		msg_time(0),
		fallvelocity(0),
		fixangles(false),
		addavelocity(false),
		idealpitch(0),
		pitchspeed(0),
		idealyaw(0),
		yawspeed(0),
		speed(0),
		stamina(0),
		modelindex(0),
		ltime(0),
		nextthink(0),
		movetype(MOVETYPE_NONE),
		solid(SOLID_NOT),
		skin(0),
		body(0),
		effects(0),
		gravity(0),
		friction(0),
		sequence(0),
		frame(0),
		animtime(0),
		framerate(0),
		scale(0),
		rendertype(RT_NORMAL),
		rendermode(RENDER_NORMAL),
		renderamt(0),
		renderfx(0),
		numsegments(0),
		health(0),
		maxhealth(0),
		takedamage(TAKEDAMAGE_NO),
		armorvalue(0),
		deadstate(0),
		frags(0),
		weapons(0),
		weaponanim(0),
		oldbuttons(0),
		buttons(0),
		impulse(0),
		dmginflictor(NO_ENTITY_INDEX),
		enemy(NO_ENTITY_INDEX),
		aiment(NO_ENTITY_INDEX),
		owner(NO_ENTITY_INDEX),
		groundent(NO_ENTITY_INDEX),
		spawnflags(0),
		flags(0),
		waterlevel(WATERLEVEL_NONE),
		watertype(0),
		waterjumptime(0),
		maxspeed(0),
		fov(0),
		induck(false),
		timestepsound(0),
		swimtime(0),
		ducktime(0),
		stepleft(false),
		dmgtaken(0),
		planezcap(0),
		groupinfo(0),
		forcehull(HULL_AUTO),
		iuser1(0),
		iuser2(0),
		iuser3(0),
		iuser4(0),
		iuser5(0),
		iuser6(0),
		iuser7(0),
		iuser8(0),
		fuser1(0),
		fuser2(0),
		fuser3(0),
		fuser4(0),
		parent(NO_ENTITY_INDEX)
	{
		memset(controllers, 0, sizeof(controllers));
		memset(blending, 0, sizeof(blending));
	}

	entindex_t entindex;
	Uint64 msg_num;
	Double msg_time;

	Vector movedir;
	Vector origin;
	Vector prevorigin;
	Vector angles;

	Vector velocity;
	Vector avelocity;
	Vector basevelocity;
	Float fallvelocity;

	Vector punchangles;
	Vector punchamount;
	Vector viewangles;
	Vector view_offset;

	Vector endpos;
	Vector startpos;

	bool fixangles;
	bool addavelocity;

	Float idealpitch;
	Float pitchspeed;

	Float idealyaw;
	Float yawspeed;

	Float speed;
	Float stamina;

	Int32 modelindex;

	Vector absmin;
	Vector absmax;
	Vector mins;
	Vector maxs;
	Vector size;

	Double ltime;
	Double nextthink;

	Int32 movetype;
	Int32 solid;

	Int32 skin;
	Int64 body;
	Int64 effects;

	Float gravity;
	Float friction;

	Int32 sequence;
	
	Float frame;
	Double animtime;
	Float framerate;

	Float controllers[MAX_CONTROLLERS];
	Float blending[MAX_BLENDING];

	Float scale;

	Int32 rendertype;

	Int32 rendermode;
	Float renderamt;
	Vector rendercolor;
	Int32 renderfx;
	Vector lightorigin;
	Uint32 numsegments;

	Float health;
	Float maxhealth;
	Int32 takedamage;
	Float armorvalue;
	Int32 deadstate;

	Int32 frags;
	Int64 weapons;
	Int32 weaponanim;

	Int32 oldbuttons;
	Int32 buttons;
	Int32 impulse;

	entindex_t dmginflictor;
	entindex_t enemy;
	entindex_t aiment;
	entindex_t owner;
	entindex_t groundent;

	Int64 spawnflags;
	Uint64 flags;

	Int32 waterlevel;
	Int32 watertype;
	Float waterjumptime;

	Float maxspeed;
	Float fov;

	bool induck;
	Int32 timestepsound;
	Int32 swimtime;
	Float ducktime;
	bool stepleft;
	Float dmgtaken;
	Float planezcap;

	Int32 groupinfo;

	Int32 forcehull;

	Int32 iuser1;
	Int32 iuser2;
	Int32 iuser3;
	Int32 iuser4;
	Int32 iuser5;
	Int32 iuser6;
	Int32 iuser7;
	Int32 iuser8;

	Float fuser1;
	Float fuser2;
	Float fuser3;
	Float fuser4;

	Vector vuser1;
	Vector vuser2;
	Vector vuser3;
	Vector vuser4;

	Int32 parent;
	Vector parentoffset;
};

#endif //ENTITY_STATE_H