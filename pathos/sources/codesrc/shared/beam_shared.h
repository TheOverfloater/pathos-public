/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BEAM_SHARED_H
#define BEAM_SHARED_H

struct cl_entity_t;

enum beam_flags_t
{
	FL_BEAM_NONE			= 0,
	FL_BEAM_STARTENTITY		= (1<<0),
	FL_BEAM_ENDENTITY		= (1<<1),
	FL_BEAM_FADEIN			= (1<<2),
	FL_BEAM_FADEOUT			= (1<<3),
	FL_BEAM_SINENOISE		= (1<<4),
	FL_BEAM_SOLID			= (1<<5),
	FL_BEAM_SHADEIN			= (1<<6),
	FL_BEAM_SHADEOUT		= (1<<7),
	FL_BEAM_START_VISIBLE	= (1<<8),
	FL_BEAM_END_VISIBLE		= (1<<9),
	FL_BEAM_IS_ACTIVE		= (1<<10),
	FL_BEAM_FOREVER			= (1<<11),
	FL_BEAM_NO_FADE			= (1<<12),
	FL_BEAM_TESLA			= (1<<13),
	FL_BEAM_VARIABLE_NOISE	= (1<<14),
	FL_BEAM_VARIABLE_DIR	= (1<<15),
	FL_BEAM_GLOW_COLOR		= (1<<16),
	FL_BEAM_KILLED			= (1<<17)
};

enum beam_msgtype_t
{
	BEAM_MSG_BEAMPOINTS = 0,
	BEAM_MSG_BEAMENTPOINT,
	BEAM_MSG_BEAMENTS,
	BEAM_MSG_BEAMSPRITE,
	BEAM_MSG_BEAMTORUS,
	BEAM_MSG_BEAMDISK,
	BEAM_MSG_BEAMCYLINDER,
	BEAM_MSG_BEAMFOLLOW,
	BEAM_MSG_BEAMRING,
	BEAM_MSG_KILLENTITYBEAMS
};

enum beam_types_t
{
	BEAM_POINTS = 0,
	BEAM_TORUS,
	BEAM_DISK,
	BEAM_CYLINDER,
	BEAM_FOLLOW,
	BEAM_RING,
	BEAM_TESLA,
	BEAM_VAPORTRAIL,

	NB_BEAM_TYPES
};

struct beam_position_t
{
	beam_position_t():
		life(0),
		acceleration(0),
		pnext(nullptr),
		pprev(nullptr)
		{
		}

	Vector position;
	Vector velocity;
	Float life;
	Float acceleration;
	Vector winddirection;

	beam_position_t* pnext;
	beam_position_t* pprev;
};

struct beamsegment_t
{
	beamsegment_t():
		tcy(0),
		brightness(0)
		{}

	Vector center;
	Vector coord1;
	Vector coord2;
	Float tcy;
	Float brightness;
};

struct beam_t
{
	beam_t():
		type(0),
		flags(0),
		index(0),
		transparency(0),
		frequency(0),
		spawntime(0),
		die(0),
		width(0),
		amplitude(0),
		speed(0),
		noisespeed(0),
		brightness(0),
		colorfadetime(0),
		colorfadedelay(0),
		framerate(0),
		frame(0),
		framecount(0),
		numsegments(0),
		startentity_index(NO_ENTITY_INDEX),
		attachment1(NO_ATTACHMENT_INDEX),
		endentity_index(NO_ENTITY_INDEX),
		attachment2(NO_ATTACHMENT_INDEX),
		modelindex1(NO_PRECACHE),
		modelindex2(NO_PRECACHE),
		startentidentifier(0),
		endentidentifier(0),
		pserverentity(nullptr),
		ppositions(nullptr),
		pnext(nullptr),
		pprev(nullptr)
	{
	}

	Int32 type;
	Int32 flags;
	Int32 index;
	Vector source;
	Vector target;
	Vector delta;

	Float transparency;
	Float frequency;
	Double spawntime;
	Double die;
	Float width;
	Float amplitude;
	Float speed;
	Float noisespeed;

	Vector color1;
	Float brightness;

	Vector color2;
	Float colorfadetime;
	Float colorfadedelay;

	Double framerate;
	Float frame;
	Int32 framecount;

	Uint32 numsegments;

	entindex_t startentity_index;
	Int32 attachment1;
	entindex_t endentity_index;
	Int32 attachment2;
	Int32 modelindex1;
	Int32 modelindex2;

	Uint32 startentidentifier;
	Uint32 endentidentifier;

	cl_entity_t* pserverentity;

	beam_position_t* ppositions;
	CArray<beamsegment_t> drawsegments;

	beam_t* pnext;
	beam_t* pprev;
};
#endif //BEAM_SHARED_H