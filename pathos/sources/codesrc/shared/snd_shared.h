/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SND_SHARED_H
#define SND_SHARED_H

enum snd_msg_types
{
	MSG_SNDENGINE_PRECACHE = 0,
	MSG_SNDENGINE_ROOMTYPE,
	MSG_SNDENGINE_OGG,
	MSG_SNDENGINE_KILL_ALL_SOUNDS,
	MSG_SNDENGINE_EMITENTITYSOUND,
	MSG_SNDENGINE_EMITAMBIENTSOUND,
	MSG_SNDENGINE_APPLY_EFFECT,
	MSG_SNDENGINE_KILL_ENTITY_SOUNDS,
};

enum ogg_flags_t
{
	OGG_FL_NONE				= 0,
	OGG_FL_STOP				= (1<<0),
	OGG_FL_STOP_FADE		= (1<<1),
	OGG_FL_LOOP				= (1<<2),
	OGG_FL_FADE_IN			= (1<<3),
	OGG_FL_MENU				= (1<<4)
};

enum snd_effects_t
{
	SND_EF_UNDEFINED = 0,
	SND_EF_CHANGE_VOLUME,
	SND_EF_CHANGE_PITCH,
};

enum snd_flags_t
{
	SND_FL_NONE				= 0,
	SND_FL_STOP				= (1<<0),
	SND_FL_CHANGE_VOLUME	= (1<<1),
	SND_FL_CHANGE_PITCH		= (1<<2),
	SND_FL_SPAWNING			= (1<<3),
	SND_FL_AMBIENT			= (1<<4),
	SND_FL_2D				= (1<<5),
	SND_FL_TEMPENT			= (1<<6),
	SND_FL_RADIO			= (1<<7),
	SND_FL_KILLME			= (1<<8),
	SND_FL_OCCLUSIONLESS	= (1<<9),
	SND_FL_REVERBLESS		= (1<<10),
	SND_FL_MOTORBIKE		= (1<<11),
	SND_FL_MUTEIGNORE		= (1<<12),
	SND_FL_DIALOUGE			= (1<<13),
	SND_FL_RADIUS			= (1<<14),
	SND_FL_DIM_OTHERS		= (1<<15),
	SND_FL_SUB_ONLY_RADIUS	= (1<<16),
	SND_FL_MENU				= (1<<17),
	SND_FL_DIM_OTHERS_LIGHT	= (1<<18),
	SND_FL_HAS_SUBTITLES	= (1<<19)
};

enum snd_channels_t
{
	SND_CHAN_AUTO = 0,
	SND_CHAN_WEAPON,
	SND_CHAN_VOICE,
	SND_CHAN_ITEM,
	SND_CHAN_BODY,
	SND_CHAN_STREAM,
	SND_CHAN_STATIC
};

enum music_channels_t
{
	MUSIC_CHANNEL_ALL = -1,
	MUSIC_CHANNEL_0,
	MUSIC_CHANNEL_1,
	MUSIC_CHANNEL_2,
	MUSIC_CHANNEL_3,
	NB_MUSIC_CHANNELS,

	MUSIC_CHANNEL_MENU
};

// No attenuation
static const Float ATTN_NONE = 0.0;
// Normal attenuation
static const Float ATTN_NORM = 1.0;
// Idle attenuation
static const Float ATTN_IDLE = 2.0;
// Static attenuation
static const Float ATTN_STATIC = 1.25;
// Gunfire attenuation
static const Float ATTN_GUNFIRE = 0.5;
// Medium attenuation
static const Float ATTN_MEDIUM = 0.4;
// Large attenuation
static const Float ATTN_LARGE = 0.25;

// Normal pitch value
static const Int32 PITCH_NORM = 100;
// Minimum pitch value
static const Int32 MIN_PITCH = 50;
// Maximum pitch value
static const Int32 MAX_PITCH = 255;

// Normal sound volume
static const Float VOL_NORM = 1.0;
#endif//SND_SHARED_H