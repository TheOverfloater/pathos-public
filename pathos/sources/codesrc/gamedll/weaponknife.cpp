/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "weaponknife.h"
#include "player.h"
#include "buttonbits.h"

// Sequence names for weapon
const Char* CWeaponKnife::m_sequenceNames[CWeaponKnife::NUM_WEAPON_ANIMATIONS] = 
{
	"idle1",
	"idle2",
	"swing_start",
	"swing_left",
	"swing_left_hit_near",
	"swing_left_hit_far",
	"swing_right",
	"swing_right_hit_near",
	"swing_right_hit_far",
	"return_left",
	"return_right",
	"draw",
	"holster"
};

// Weapon view model
const Char CWeaponKnife::WEAPON_VIEWMODEL[] = "models/v_knife.mdl";
// Weapon weight
const Int32 CWeaponKnife::WEAPON_WEIGHT = 5;
// Weapon slot
const Int32 CWeaponKnife::WEAPON_SLOT = 0;
// Weapon slot position
const Int32 CWeaponKnife::WEAPON_SLOT_POSITION = 0;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(weapon_knife, CWeaponKnife);

//=============================================
// @brief
//
//=============================================
CWeaponKnife::CWeaponKnife( edict_t* pedict ):
	CPlayerWeapon(pedict),
	m_impactTime(0),
	m_timeNext(0),
	m_attackState(STATE_NONE),
	m_knifeHit(0)
{
}

//=============================================
// @brief
//
//=============================================
CWeaponKnife::~CWeaponKnife( void )
{
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_KNIFE);

	m_clip = WEAPON_NO_CLIP;
	m_weaponId = WEAPON_KNIFE;
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::Precache( void )
{
	CPlayerWeapon::Precache();

	Int32 modelIndex = gd_engfuncs.pfnPrecacheModel(WEAPON_VIEWMODEL);
	if(modelIndex != NO_PRECACHE)
		PrecacheModelSounds(modelIndex);

	gd_engfuncs.pfnPrecacheSound("weapons/knife_hit1.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_hit2.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_hit3.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_wall1.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_wall2.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_wall3.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_swipe1.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/knife_swipe2.wav");
}

//=============================================
// @brief
//
//=============================================
bool CWeaponKnife::GetWeaponInfo( weaponinfo_t* pWeapon )
{
	pWeapon->name = gd_engfuncs.pfnGetString(m_pFields->classname);
	pWeapon->ammo.clear();
	pWeapon->maxammo = -1;
	pWeapon->maxclip = WEAPON_NO_CLIP;
	pWeapon->slot = WEAPON_SLOT;
	pWeapon->position = WEAPON_SLOT_POSITION;
	pWeapon->id = WEAPON_KNIFE;
	pWeapon->weight = WEAPON_WEIGHT;
	pWeapon->flags |= FL_WEAPON_NO_AUTO_AIM;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponKnife::Deploy( void )
{
	return DefaultDeploy(WEAPON_VIEWMODEL, m_sequenceNames[KNIFE_DRAW]);
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::Holster( void )
{
	const Char* pstrSequenceName = m_sequenceNames[KNIFE_HOLSTER];
	m_nextThinkTime = g_pGameVars->time + GetSequenceTime(pstrSequenceName);
	SetWeaponAnimation(pstrSequenceName);

	m_timeNext = 0;
	m_impactTime = 0;
	m_attackState = STATE_NONE;
	m_knifeHit = false;

	CPlayerWeapon::Holster();
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::PrimaryAttack( void )
{
	if(m_attackState != STATE_NONE)
		return;

	const Char* pstrSequenceName = m_sequenceNames[KNIFE_SWING_START];
	m_timeNext = g_pGameVars->time + GetSequenceTime(pstrSequenceName);
	m_nextAttackTime = m_timeNext;
	m_attackState = STATE_FIRST_SWING;

	SetWeaponAnimation(pstrSequenceName);
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::Idle( void )
{
	if (m_nextIdleTime > g_pGameVars->time)
		return;

	Uint32 sequenceIndex = 0;
	if(Common::RandomFloat(0.0, 1.0) <= 0.8)
		sequenceIndex = KNIFE_IDLE1;
	else
		sequenceIndex = KNIFE_IDLE2;

	Float sequenceTime = GetSequenceTime(m_sequenceNames[sequenceIndex]);
	m_nextIdleTime = g_pGameVars->time + sequenceTime;

	SetWeaponAnimation( m_sequenceNames[sequenceIndex] );
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::PostThink( void )
{
	// We're attacking
	if(m_attackState != STATE_NONE)
	{
		if(m_knifeHit && (m_impactTime <= g_pGameVars->time))
		{
			trace_t tr;

			Vector forward, right, up;
			Vector viewAngles = m_pPlayer->GetGunAngles(false);
			Math::AngleVectors(viewAngles, &forward, &right, &up);

			Vector vecBaseSrc = m_pPlayer->GetGunPosition();
			Vector vecBaseEnd = vecBaseSrc + forward * 60 - up * 12;

			gMultiDamage.Clear( );

			Vector vecSrc;
			Vector vecEnd;

			for(Int32 i = 0; i < 8; i++)
			{
				if(m_attackState == STATE_SWING_LEFT || m_attackState == STATE_FIRST_SWING)
				{
					vecSrc = vecBaseSrc + right*(-4+i)*2;
					vecEnd = vecBaseEnd + right*(-4+i)*2;

				}
				else if(m_attackState == STATE_SWING_RIGHT)
				{
					vecSrc = vecBaseSrc + right*(4-i)*2;
					vecEnd = vecBaseEnd + right*(4-i)*2;
				}

				Util::TraceLine(vecSrc, vecEnd, false, true, false, true, m_pPlayer->GetEdict(), tr);

				if(!tr.noHit())
				{
					CBaseEntity *pEntity = Util::GetEntityFromTrace(tr);
					if(pEntity && pEntity->GetClassification() != CLASS_HUMAN_FRIENDLY)
					{
						Float knifeDamage = gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgKnife);
						pEntity->TraceAttack(m_pPlayer, knifeDamage, forward, tr, DMG_MELEE );

						CString soundfile;
						if (pEntity->IsNPC() && pEntity->GetClassification() != CLASS_MACHINE)
						{
							switch (Common::RandomLong(0,2))
							{
							case 0: soundfile = "weapons/knife_hit1.wav"; break;
							case 1: soundfile = "weapons/knife_hit2.wav"; break;
							case 2: soundfile = "weapons/knife_hit3.wav"; break;
							}
						}
						else
						{
							switch (Common::RandomLong(0,2))
							{
							case 0: soundfile = "weapons/knife_wall1.wav"; break;
							case 1: soundfile = "weapons/knife_wall2.wav"; break;
							case 2: soundfile = "weapons/knife_wall3.wav"; break;
							}
						}

						// Play sound
						Util::EmitAmbientSound(tr.endpos, soundfile.c_str());

						// Spawn appropriate things
						Util::CreateImpactEffects(tr, vecSrc, false);

						if(pEntity->GetClassification() != CLASS_NONE && pEntity->GetClassification() != CLASS_MACHINE)
						{
							Util::TraceLine(vecBaseSrc, vecBaseEnd, false, true, false, true, m_pPlayer->GetEdict(), tr);
							if(!tr.noHit())
								Util::CreateVBMDecal(tr.endpos, tr.plane.normal, "shot_human", pEntity->GetEdict(), FL_DECAL_NORMAL_PERMISSIVE);
						}
					}

					// We've hit the entity, so don't bother now
					// doing other tracelines
					break;
				}
			}

			// Apply the damage
			gMultiDamage.ApplyDamage(m_pPlayer, m_pPlayer, tr.hitgroup);

			m_knifeHit = false;
		}

		if(m_timeNext <= g_pGameVars->time)
		{
			if((m_attackState == STATE_FIRST_SWING) || (m_pPlayer->GetButtonBits() & IN_ATTACK))
			{
				Swing();
			}
			else
			{
				if(m_attackState == STATE_SWING_LEFT)
				{
					// Get the sequence time
					Double sequenceTime = GetSequenceTime(m_sequenceNames[KNIFE_RETURN_RIGHT]);

					// We were just about to swing to the left
					m_nextAttackTime = m_nextThinkTime = m_timeNext = g_pGameVars->time + sequenceTime;
					m_nextIdleTime = g_pGameVars->time + sequenceTime;
					SetWeaponAnimation(m_sequenceNames[KNIFE_RETURN_RIGHT], NO_BODY_VALUE, NO_SKIN_VALUE, false);
				}
				else if(m_attackState == STATE_SWING_RIGHT)
				{
					// Get the sequence time
					Double sequenceTime = GetSequenceTime(m_sequenceNames[KNIFE_RETURN_LEFT]);

					// We were just about to swing to the right
					m_nextAttackTime = m_nextThinkTime = m_timeNext = g_pGameVars->time + sequenceTime;
					m_nextIdleTime = g_pGameVars->time + sequenceTime;
					SetWeaponAnimation(m_sequenceNames[KNIFE_RETURN_LEFT], NO_BODY_VALUE, NO_SKIN_VALUE, false);
				}

				// Reset everything
				m_attackState = STATE_NONE;
			}
		}

		return;
	}

	if(m_attackState != STATE_NONE)
		return;

	// Let the rest run it's course
	CPlayerWeapon::PostThink();
}

//=============================================
// @brief
//
//=============================================
void CWeaponKnife::Swing( void )
{
	trace_t tr;
	Vector forward, right, up;
	Vector viewAngles = m_pPlayer->GetGunAngles(false);
	Math::AngleVectors(viewAngles, &forward, &right, &up);

	Vector vecBaseSrc = m_pPlayer->GetGunPosition();
	Vector vecBaseEnd = vecBaseSrc + forward * 60 - up * 12;

	Vector vecSrc, vecEnd;
	for(Int32 i = 0; i < 8; i++)
	{
		if(m_attackState == STATE_SWING_LEFT || m_attackState == STATE_FIRST_SWING)
		{
			vecSrc = vecBaseSrc + right*(-4+i)*2;
			vecEnd = vecBaseEnd + right*(-4+i)*2;

		}
		else if(m_attackState == STATE_SWING_RIGHT)
		{
			vecSrc = vecBaseSrc + right*(4-i)*2;
			vecEnd = vecBaseEnd + right*(4-i)*2;
		}

		Util::TraceLine(vecSrc, vecEnd, false, true, false, true, m_pPlayer->GetEdict(), tr);
		if(!tr.noHit())
			break;
	}

	if(m_attackState == STATE_SWING_LEFT || m_attackState == STATE_FIRST_SWING)
	{
		if(tr.noHit())
		{
			m_timeNext = g_pGameVars->time + GetSequenceTime(m_sequenceNames[KNIFE_SWING_LEFT]) + 0.3;
			SetWeaponAnimation(m_sequenceNames[KNIFE_SWING_LEFT], NO_BODY_VALUE, NO_SKIN_VALUE, false);
		}
		else
		{
			if(tr.fraction > 0.5)
			{
				m_timeNext = g_pGameVars->time + GetSequenceTime(m_sequenceNames[KNIFE_SWING_LEFT_HIT_FAR])+0.2;
				SetWeaponAnimation(m_sequenceNames[KNIFE_SWING_LEFT_HIT_FAR], NO_BODY_VALUE, NO_SKIN_VALUE, false);

				m_impactTime = g_pGameVars->time + 0.14;
				m_knifeHit = true;
			}
			else
			{
				m_timeNext = g_pGameVars->time + GetSequenceTime(m_sequenceNames[KNIFE_SWING_LEFT_HIT_NEAR])+0.2;
				SetWeaponAnimation(m_sequenceNames[KNIFE_SWING_LEFT_HIT_NEAR], NO_BODY_VALUE, NO_SKIN_VALUE, false);

				m_impactTime = g_pGameVars->time + 0.14;
				m_knifeHit = true;
			}
		}

		m_pPlayer->ApplyAxisPunch( 0, -Common::RandomFloat( -14, -20 ) );
		m_pPlayer->ApplyAxisPunch( 1, -Common::RandomFloat( -70, -80 ) );
		m_attackState = STATE_SWING_RIGHT;
	}
	else if(m_attackState == STATE_SWING_RIGHT)
	{
		if(tr.noHit())
		{
			m_timeNext = g_pGameVars->time + GetSequenceTime(m_sequenceNames[KNIFE_SWING_RIGHT]) + 0.3;
			SetWeaponAnimation(m_sequenceNames[KNIFE_SWING_RIGHT], NO_BODY_VALUE, NO_SKIN_VALUE, false);
		}
		else
		{
			if(tr.fraction > 0.5)
			{
				m_timeNext = g_pGameVars->time + GetSequenceTime(m_sequenceNames[KNIFE_SWING_RIGHT_HIT_FAR])+0.2;
				SetWeaponAnimation(m_sequenceNames[KNIFE_SWING_RIGHT_HIT_FAR], NO_BODY_VALUE, NO_SKIN_VALUE, false);
				
				m_impactTime = g_pGameVars->time + 0.12;
				m_knifeHit = true;
			}
			else
			{
				m_timeNext = g_pGameVars->time + GetSequenceTime(m_sequenceNames[KNIFE_SWING_RIGHT_HIT_NEAR])+0.2;
				SetWeaponAnimation(m_sequenceNames[KNIFE_SWING_RIGHT_HIT_NEAR], NO_BODY_VALUE, NO_SKIN_VALUE, false);
				
				m_impactTime = g_pGameVars->time + 0.12;
				m_knifeHit = true;
			}
		}

		m_pPlayer->ApplyAxisPunch( 0, -Common::RandomFloat( 14, 20 ) );
		m_pPlayer->ApplyAxisPunch( 1, -Common::RandomFloat( 70, 80 ) );
		m_attackState = STATE_SWING_LEFT;
	}

	CString soundfile;
	switch(Common::RandomLong(0, 1))
	{
		case 0: soundfile = "weapons/knife_swipe1.wav"; break;
		case 1: soundfile = "weapons/knife_swipe2.wav"; break;
	}

	Util::EmitEntitySound(m_pPlayer, soundfile.c_str(), SND_CHAN_WEAPON, VOL_NORM, ATTN_NORM, PITCH_NORM + Common::RandomLong(-5, +10)); 
}
