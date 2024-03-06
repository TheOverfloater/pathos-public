/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef USERMSGS_H
#define USERMSGS_H

#include "net_shared.h"

struct usermsglist_t
{
	usermsglist_t():
		saytext(0),
		setfog(0),
		creategenericdecal(0),
		createvbmdecal(0),
		createparticlesystem(0),
		precacheparticlesystem(0),
		removeparticlesystem(0),
		skyboxparameters(0),
		dynamiclight(0),
		setdaystage(0),
		setspecialfog(0),
		freeentitydata(0),
		addlightstyle(0),
		precacheflexscript(0),
		setentityflexscript(0),
		createtempentity(0),
		hudstamina(0),
		hudhealth(0),
		hudkevlar(0),
		movementnoise(0),
		hudhealthkit(0),
		hudcurrentweapon(0),
		hudweaponlist(0),
		hudammocount(0),
		hudammopickup(0),
		hudweaponpickup(0),
		huditempickup(0),
		hudsetactive(0),
		hudsetusableobject(0),
		hudsetcountdowntimer(0),
		setautoaimvector(0),
		setflashlight(0),
		screenshake(0),
		creategameuiwindow(0),
		setfov(0),
		triggerzoom(0),
		radiomessage(0),
		ladder(0),
		motorbike(0),
		viewcontroller(0),
		viewmodel(0),
		motionblur(0),
		showmessage(0),
		showcustommessage(0),
		screenfade(0),
		setviewentity(0),
		nodedebug(0),
		screentext(0),
		blackhole(0),
		sunflare(0),
		vaportrail(0),
		npcawareness(0),
		newobjective(0),
		addskytextureset(0),
		setskytexture(0)
		{
		}

	Int32 saytext;
	Int32 setfog;
	Int32 creategenericdecal;
	Int32 createvbmdecal;
	Int32 createparticlesystem;
	Int32 precacheparticlesystem;
	Int32 removeparticlesystem;
	Int32 skyboxparameters;
	Int32 dynamiclight;
	Int32 setdaystage;
	Int32 setspecialfog;
	Int32 freeentitydata;
	Int32 addlightstyle;
	Int32 precacheflexscript;
	Int32 setentityflexscript;
	Int32 createtempentity;
	Int32 hudstamina;
	Int32 hudhealth;
	Int32 hudkevlar;
	Int32 movementnoise;
	Int32 hudhealthkit;
	Int32 hudcurrentweapon;
	Int32 hudweaponlist;
	Int32 hudammocount;
	Int32 hudammopickup;
	Int32 hudweaponpickup;
	Int32 huditempickup;
	Int32 hudsetactive;
	Int32 hudsetusableobject;
	Int32 hudsetcountdowntimer;
	Int32 setautoaimvector;
	Int32 setflashlight;
	Int32 screenshake;
	Int32 creategameuiwindow;
	Int32 setfov;
	Int32 triggerzoom;
	Int32 radiomessage;
	Int32 ladder;
	Int32 motorbike;
	Int32 viewcontroller;
	Int32 viewmodel;
	Int32 motionblur;
	Int32 showmessage;
	Int32 showcustommessage;
	Int32 screenfade;
	Int32 setviewentity;
	Int32 nodedebug;
	Int32 screentext;
	Int32 tempbeam;
	Int32 blackhole;
	Int32 sunflare;
	Int32 vaportrail;
	Int32 npcawareness;
	Int32 newobjective;
	Int32 addskytextureset;
	Int32 setskytexture;
}; 
extern usermsglist_t g_usermsgs;

extern void InitClientUserMessages( void );
#endif //USERMSGS_H