/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "baseentity.h"

class CEnvCubemapOBB : public CBaseEntity
{
public:
	explicit CEnvCubemapOBB(edict_t* pedict) : CBaseEntity(pedict) {}

	virtual bool Spawn(void) override
	{
		m_pState->solid = SOLID_NOT;
		m_pState->movetype = MOVETYPE_NONE;
		m_pState->effects |= EF_NODRAW;

		return SetModel(m_pFields->modelname);
	}
};

LINK_ENTITY_TO_CLASS(env_cubemap_obb, CEnvCubemapOBB);