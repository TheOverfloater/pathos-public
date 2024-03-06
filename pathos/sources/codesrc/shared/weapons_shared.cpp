/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "weapons_shared.h"

bullet_typemapping_t BULLET_TYPE_MAP[] = 
{
	DEFINE_BULLET_TYPE(BULLET_NONE),
	DEFINE_BULLET_TYPE(BULLET_PLAYER_GLOCK),
	DEFINE_BULLET_TYPE(BULLET_PLAYER_KNIFE),
	DEFINE_BULLET_TYPE(BULLET_NPC_9MM),
	DEFINE_BULLET_TYPE(BULLET_NPC_BUCKSHOT),
	DEFINE_BULLET_TYPE(BULLET_NPC_SIG552),
	DEFINE_BULLET_TYPE(BULLET_NPC_M249),
	DEFINE_BULLET_TYPE(BULLET_NPC_TRG42),
	0, ""
};

//=============================================
// @brief
//
//=============================================
Vector Weapon_GetConeSize( Int32 coneindex, Vector leanoffset, Vector velocity, Vector punchangles )
{
	Vector cone;
	switch(coneindex)
	{
	case 1: 
		cone = VECTOR_CONE_1DEGREES; 
		break;
	case 2: 
		cone = VECTOR_CONE_2DEGREES; 
		break;
	case 3: 
		cone = VECTOR_CONE_3DEGREES; 
		break;
	case 4: 
		cone = VECTOR_CONE_4DEGREES; 
		break;
	case 5: 
		cone = VECTOR_CONE_5DEGREES; 
		break;
	case 6: 
		cone = VECTOR_CONE_6DEGREES; 
		break;
	case 7: 
		cone = VECTOR_CONE_7DEGREES; 
		break;
	case 8: 
		cone = VECTOR_CONE_8DEGREES; 
		break;
	case 9: 
		cone = VECTOR_CONE_10DEGREES; 
		break;
	case 10: 
		cone = VECTOR_CONE_15DEGREES; 
		break;
	case 11: 
		cone = VECTOR_CONE_20DEGREES; 
		break;
	}

	// Add velocity-induced inaccuracy
	Float spreadadd;
	spreadadd = (velocity.Length()*0.1);

	// Add in leaning
	spreadadd += leanoffset.Length()*0.8; // Make leaning far less accurate

	// Add in punchangles
	spreadadd += punchangles.Length()*2;

	for(Uint32 i = 0; i < 3; i++)
		cone[i] += spreadadd / 500.0f;

	return cone;
}