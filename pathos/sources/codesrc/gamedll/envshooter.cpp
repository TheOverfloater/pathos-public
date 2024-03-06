/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envshooter.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_shooter, CEnvShooter);

//=============================================
// @brief
//
//=============================================
CEnvShooter::CEnvShooter( edict_t* pedict ):
	CGibShooter(pedict),
	m_modelName(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvShooter::~CEnvShooter( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvShooter::DeclareSaveFields( void )
{
	// Call base class to do it first
	CGibShooter::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvShooter, m_modelName, EFIELD_STRING));
}

//=============================================
// @brief
//
//=============================================
bool CEnvShooter::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "shootmodel"))
	{
		m_modelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "shootsounds"))
	{
		Int32 noise = SDL_atoi(kv.value);
		switch(noise)
		{
		case SND_GLASS:
			m_gibMaterial = MAT_GLASS;
			break;
		case SND_WOOD:
			m_gibMaterial = MAT_WOOD;
			break;
		case SND_METAL:
			m_gibMaterial = MAT_METAL;
			break;
		case SND_FLESH:
			m_gibMaterial = MAT_FLESH;
			break;
		case SND_ROCKS:
			m_gibMaterial = MAT_ROCKS;
			break;
		case SND_NONE:
		default:
			m_gibMaterial = MAT_NONE;
			break;
		}
		return true;
	}
	else
		return CGibShooter::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvShooter::Precache( void )
{
	if(m_gibMaterial != MAT_NONE)
		Util::PrecacheDebrisSounds((breakmaterials_t)m_gibMaterial);

	if(m_modelName == NO_STRING_VALUE)
		return;

	const Char* pstrModelname = gd_engfuncs.pfnGetString(m_modelName);
	m_gibModelIndex = gd_engfuncs.pfnPrecacheModel(pstrModelname);

	if(m_gibModelIndex != NO_PRECACHE)
		m_nbBodyVariations = gd_engfuncs.pfnGetModelFrameCount(m_gibModelIndex);
}

//=============================================
// @brief
//
//=============================================
bool CEnvShooter::Spawn( void )
{
	if(!CGibShooter::Spawn())
		return false;

	if(m_modelName == NO_STRING_VALUE || m_gibModelIndex == NO_PRECACHE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
CGib* CEnvShooter::CreateGib( void )
{
	CGib* pGib = reinterpret_cast<CGib*>(CBaseEntity::CreateEntity("gib", nullptr));
	if(!pGib)
		return nullptr;

	if(!pGib->InitGib(gd_engfuncs.pfnGetString(m_modelName)))
	{
		Util::RemoveEntity(pGib);
		return nullptr;
	}

	if(m_nbBodyVariations > CGib::GIB_SKULL)
	{
		Uint32 randomGibsBegin = CGib::GIB_SKULL+1;
		pGib->SetBody(Common::RandomLong(randomGibsBegin, CGib::NB_GIBS-randomGibsBegin));
	}

	pGib->SetBloodColor(BLOOD_NONE);
	pGib->SetMaterial(m_gibMaterial);

	pGib->SetRenderMode((rendermode_t)m_pState->rendermode);
	pGib->SetRenderAmount(m_pState->renderamt);
	pGib->SetRenderColor(m_pState->rendercolor);
	pGib->SetRenderFx(m_pState->renderfx);
	pGib->SetScale(m_pState->scale);
	pGib->SetSkin(m_pState->skin);

	return pGib;
}