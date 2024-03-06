/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_bsp.h"
#include "r_common.h"
#include "r_decals.h"
#include "com_math.h"
#include "file.h"
#include "common.h"
#include "system.h"
#include "brushmodel.h"
#include "enginestate.h"
#include "cl_main.h"
#include "r_main.h"
#include "trace_shared.h"
#include "cl_utils.h"
#include "cl_pmove.h"
#include "textures_shared.h"
#include "texturemanager.h"
#include "decallist.h"
#include "console.h"
#include "r_basicdraw.h"
#include "cl_msg.h"

// Distance which decals trace against
const Float CDecalManager::DECAL_CHECK_DIST = 1;
// Decal texture folder path
const Char CDecalManager::DECAL_TEXTURE_PATH[] = "decals/";

// Class object
CDecalManager gDecals;

//====================================
//
//====================================
CDecalManager::CDecalManager( void ):
	m_pCvarShowBogusDecals(nullptr),
	m_spawnClientDecalMsgId(0)
{
}

//====================================
//
//====================================
CDecalManager::~CDecalManager( void )
{
}

//====================================
//
//====================================
void CDecalManager::Init( void )
{
	m_pCvarShowBogusDecals = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_show_bogus_decals", "0", "Toggles rendering of bogus decal positions");

	m_spawnClientDecalMsgId = CL_RegisterClientUserMessage("SpawnClientDecal", -1);
}

//====================================
//
//====================================
bool CDecalManager::InitGame( void )
{
	Uint32 filesize = 0;
	const Char* pfile = reinterpret_cast<const Char*>(FL_LoadFile(DECAL_LIST_FILE_PATH, &filesize));
	if(!pfile)
	{
		Con_Printf("%s - Could not load decal list file '%s'.\n", __FUNCTION__, DECAL_LIST_FILE_PATH);
		return false;
	}

	if(!m_decalList.LoadDecalList(pfile, filesize))
	{
		Con_Printf("%s - Failed to load decal list: %s.\n", __FUNCTION__, DECAL_LIST_FILE_PATH);
		FL_FreeFile(pfile);
		return false;
	}

	FL_FreeFile(pfile);

	if(!cls.netinfo.decalcache.empty())
	{
		for(Uint32 i = 0; i < cls.netinfo.decalcache.size(); i++)
		{
			const decalcache_t& cache = cls.netinfo.decalcache[i];

			switch(cache.type)
			{
			case DECAL_CACHE_SINGLE:
				PrecacheTexture(cache.name.c_str());
				break;
			case DECAL_CACHE_GROUP:
				PrecacheGroup(cache.name.c_str());
				break;
			}
		}

		cls.netinfo.decalcache.clear();
	}

	return true;
}

//====================================
//
//====================================
void CDecalManager::ClearGame( void )
{
	if(!m_cachedDecalsList.empty())
		m_cachedDecalsList.clear();

	m_decalList.Clear();

	if(!m_bogusDecalsList.empty())
		m_bogusDecalsList.clear();
}

//====================================
//
//====================================
bool CDecalManager::DrawBogusDecals( void )
{
	if(m_pCvarShowBogusDecals->GetValue() < 1)
		return true;

	if(m_bogusDecalsList.empty())
		return true;

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);

	CBasicDraw* pDraw = CBasicDraw::GetInstance();

	if(!pDraw->Enable() || !pDraw->DisableTexture())
	{
		Sys_ErrorPopup("Shader error: %s.\n", pDraw->GetShaderError());
		return false;
	}

	pDraw->SetProjection(rns.view.projection.GetMatrix());
	pDraw->SetModelview(rns.view.modelview.GetMatrix());

	const Float axisLength = 32.0f;

	glLineWidth(2.0f);

	m_bogusDecalsList.begin();
	while(!m_bogusDecalsList.end())
	{
		bool nodepthtest = false;
		if(m_pCvarShowBogusDecals->GetValue() >= 2)
		{
			// Always disable
			nodepthtest = true;
		}
		else
		{
			// Only if distance is less than 512
			Vector position = m_bogusDecalsList.get();
			Float distance = (rns.view.v_origin - position).Length();
			if(distance < 512)
				nodepthtest = true;
		}

		if(nodepthtest)
			glDisable(GL_DEPTH_TEST);
		else
			glEnable(GL_DEPTH_TEST);

		pDraw->Begin(GL_LINES);

		for(Uint32 i = 0; i < 3; i++)
		{
			Vector direction;
			switch(i)
			{
			case 0:
				pDraw->Color4f(1.0, 0.0, 0.0, 1.0);
				direction.x = 1.0;
				break;
			case 1:
				pDraw->Color4f(0.0, 1.0, 0.0, 1.0);
				direction.y = 1.0;
				break;
			case 2:
				pDraw->Color4f(0.0, 0.0, 1.0, 1.0);
				direction.z = 1.0;
				break;
			}

			Vector start = m_bogusDecalsList.get() + direction * axisLength;
			Vector end = m_bogusDecalsList.get() - direction * axisLength;

			pDraw->Vertex3fv(start);
			pDraw->Vertex3fv(end);
		}

		pDraw->End();

		m_bogusDecalsList.next();
	}
	pDraw->Disable();

	glDisable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	return true;
}

//====================================
//
//====================================
void CDecalManager::FindLeaf( cached_decal_t *pdecal )
{
	if(!ens.pworld)
		return;

	Vector voffset;
	const mleaf_t *pleaf = nullptr;
	for(Int32 i = 0; i < 3; i++)
	{
		Math::VectorClear(voffset);
		voffset[i] = 1;

		pleaf = Mod_PointInLeaf(pdecal->origin+voffset, (*ens.pworld));
		if(pleaf->contents != CONTENTS_SOLID)
			break;

		pleaf = Mod_PointInLeaf(pdecal->origin-voffset, (*ens.pworld));
		if(pleaf->contents != CONTENTS_SOLID)
			break;
	}

	if(pleaf && pleaf->contents != CONTENTS_SOLID)
		pdecal->leafnum = pleaf-ens.pworld->pleafs-1;
	else
		pdecal->leafnum = -1;
}

//====================================
//
//====================================
void CDecalManager::AddCached( decalgroupentry_t* pentry, const Char *pstrname, const Vector& origin, const Vector& normal, Int32 flags, entindex_t entityindex, Float life, Float fadetime, Float growthtime )
{
	if(flags & FL_DECAL_BOGUS)
	{
		Con_Printf("Bogus decal placement for decal with entry %s at %.0f %.0f %.0f.\n", pstrname, origin.x, origin.y, origin.z);
		m_bogusDecalsList.add(origin);
		return;
	}

	cached_decal_t newdecal;

	Math::VectorCopy(origin, newdecal.origin);
	Math::VectorCopy(normal, newdecal.normal);
	newdecal.flags = flags;
	newdecal.pentry = pentry;
	newdecal.name = pstrname;
	newdecal.entityindex = entityindex;
	newdecal.life = life;
	newdecal.fadetime = fadetime;
	newdecal.growthtime = growthtime;

	m_cachedDecalsList.add(newdecal);
}

//====================================
//
//====================================
void CDecalManager::CreateCached( void )
{
	if(!ens.pworld)
		return;

	if(m_cachedDecalsList.empty())
		return;

	if(!rns.view.pviewleaf)
		return;

	if(rns.view.pviewleaf->contents == CONTENTS_SOLID)
		return;

	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return;

	const Vector voffsets[] = { Vector(0, 0, DECAL_CHECK_DIST), 
		Vector(0, DECAL_CHECK_DIST, 0), 
		Vector(DECAL_CHECK_DIST, 0, 0), 
		Vector(0, 0, -DECAL_CHECK_DIST), 
		Vector(0, -DECAL_CHECK_DIST, 0), 
		Vector(-DECAL_CHECK_DIST, 0, 0) };

	trace_t trace;

	m_cachedDecalsList.begin();
	while(!m_cachedDecalsList.end())
	{
		cached_decal_t& cachedecal = m_cachedDecalsList.get();

		// Load entry if it wasn't available yet
		if(!cachedecal.pentry)
		{
			decalgroupentry_t *pentry = nullptr;
			if(!(cachedecal.flags & (FL_DECAL_PERSISTENT|FL_DECAL_SPECIFIC_TEXTURE)))
				pentry = m_decalList.GetRandom(cachedecal.name.c_str());
			else
				pentry = m_decalList.GetByName(cachedecal.name.c_str());

			if(!pentry)
			{
				CString msg;
				msg << "Couldn't find ";
				if(!(cachedecal.flags & (FL_DECAL_PERSISTENT|FL_DECAL_SPECIFIC_TEXTURE)))
					msg << "decal group ";
				else
					msg << "decal texture ";

				msg << cachedecal.name << "\n";
				Con_Printf(msg.c_str());

				m_bogusDecalsList.add(cachedecal.origin);

				m_cachedDecalsList.remove(m_cachedDecalsList.get_link());
				m_cachedDecalsList.next();
				continue;
			}

			cachedecal.pentry = pentry;
		}

		// Late leaf detection
		if(!cachedecal.leafnum)
			FindLeaf(&cachedecal);

		// See if it's in the pvs yet
		if(cachedecal.leafnum != -1)
		{
			if (!(rns.pvisbuffer[cachedecal.leafnum >> 3] & (1 << (cachedecal.leafnum&7) )))
			{
				m_cachedDecalsList.next();
				continue;
			}
		}

		if(cachedecal.entityindex != NO_ENTITY_INDEX)
		{
			cl_entity_t* pentity = CL_GetEntityByIndex(cachedecal.entityindex);
			if(!pentity)
			{
				m_cachedDecalsList.next();
				continue;
			}

			if(pentity->curstate.msg_num != pplayer->curstate.msg_num)
			{
				m_cachedDecalsList.next();
				continue;
			}
		}

		if(rns.fog.settings.active)
		{
			if((cachedecal.origin-Vector(rns.view.params.v_origin)).Length() > rns.fog.settings.end)
			{
				m_cachedDecalsList.next();
				continue;
			}
		}

		bool addtobogus = false;
		bool shouldspawn = true;
		if(!(cachedecal.flags & FL_DECAL_HAS_NORMAL))
		{
			for(Int32 j = 0; j < 3; j++)
			{
				Vector vstart = cachedecal.origin + voffsets[j];
				Vector vend = cachedecal.origin + voffsets[j+3];

				CL_PlayerTrace(vend, vstart, FL_TRACE_NO_MODELS, HULL_POINT, NO_ENTITY_INDEX, trace);
				if(trace.fraction != 1 && trace.fraction != 0)
					break;
			}

			if(trace.fraction == 1.0 || trace.fraction == 0.0)
			{
				for(Int32 j = 0; j < 3; j++)
				{
					Vector vstart = cachedecal.origin + voffsets[j+3];
					Vector vend = cachedecal.origin + voffsets[j];

					CL_PlayerTrace(vend, vstart, FL_TRACE_NO_MODELS, HULL_POINT, NO_ENTITY_INDEX, trace);
					if(trace.fraction != 1 && trace.fraction != 0)
						break;
				}
			}

			if(trace.fraction == 1.0 || trace.fraction == 0.0)
			{
				if((cachedecal.flags & FL_DECAL_CL_ENTITY_MANAGER) && !(cachedecal.flags & FL_DECAL_SERVER))
				{
					// Send to server to find entity to spawn on
					SendDecalToServer(cachedecal);
				}
				else
				{
					Con_Printf("Bogus decal placement for decal with entry %s at %.0f %.0f %.0f.\n", cachedecal.name.c_str(), cachedecal.origin.x, cachedecal.origin.y, cachedecal.origin.z);
					addtobogus = true;
				}

				// Do not try to spawn this decal
				shouldspawn = false;
			}
			else
			{
				Math::VectorCopy(trace.plane.normal, cachedecal.normal);
				Math::VectorCopy(trace.endpos, cachedecal.origin);
			}
		}

		// Only spawn if it's a valid decal
		if(shouldspawn)
		{
			gBSPRenderer.CreateDecal(cachedecal.origin, cachedecal.normal, cachedecal.pentry, cachedecal.flags, cachedecal.life, cachedecal.fadetime, cachedecal.growthtime);

			if(cachedecal.flags & FL_DECAL_VBM)
				cls.dllfuncs.pfnDecalExternalEntities(cachedecal.origin, cachedecal.normal, cachedecal.pentry, cachedecal.flags);
		}
		else if(addtobogus)
		{
			// Add to bogus list
			m_bogusDecalsList.add(cachedecal.origin);
		}

		// Remove it
		m_cachedDecalsList.remove(m_cachedDecalsList.get_link());
		m_cachedDecalsList.next();
	}
}

//====================================
//
//====================================
void CDecalManager::LoadTexture( decalgroupentry_t *pentry )
{
	if(pentry->ptexture)
		return;

	CString filepath;
	filepath << DECAL_TEXTURE_PATH << pentry->name << ".dds";

	GLint colorgray[] = {128, 128, 128, 0};

	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	pentry->ptexture = pTextureManager->LoadTexture(filepath.c_str(), RS_GAME_LEVEL, (TX_FL_BORDER|TX_FL_CLAMP_S|TX_FL_CLAMP_T|TX_FL_NOMIPMAPS), colorgray);

	if(!pentry->ptexture)
	{
		// Just set a dummy texture
		pentry->ptexture = pTextureManager->GetDummyTexture();
	}
}

//====================================
//
//====================================
void CDecalManager::SendDecalToServer( cached_decal_t& decal )
{
	Int32 flags = (decal.flags | FL_DECAL_SPECIFIC_TEXTURE);

	CL_ClientUserMessageBegin(m_spawnClientDecalMsgId);
	CL_Msg_WriteUint16(flags);
	for(Uint32 i = 0; i < 3; i++)
		CL_Msg_WriteFloat(decal.origin[i]);
	CL_Msg_WriteString(decal.pentry->name.c_str());
	CL_ClientUserMessageEnd();
}

//====================================
//
//====================================
void CDecalManager::PrecacheGroup( const Char *szgroupname )
{
	decalgroup_t *pgroup = m_decalList.GetGroup(szgroupname);
	if(!pgroup)
	{
		Con_Printf("Decal group '%s' not found.\n", szgroupname);
		return;
	}

	for(Uint32 i = 0; i < pgroup->entries.size(); i++)
	{
		decalgroupentry_t *pentry = &pgroup->entries[i];

		if(!pentry->ptexture)
			LoadTexture(pentry);
	}
}

//====================================
//
//====================================
void CDecalManager::PrecacheTexture( const Char *sztexturename )
{
	Uint32 nbGroups = m_decalList.GetNbGroups();
	for(Uint32 i = 0; i < nbGroups; i++)
	{
		decalgroup_t* pgroup = m_decalList.GetGroup(i);
		if(!pgroup)
			continue;

		for(Uint32 j = 0; j < pgroup->entries.size(); j++)
		{
			decalgroupentry_t *pentry = &pgroup->entries[j];

			if(!qstrcmp(pentry->name, sztexturename))
			{
				if(!pentry->ptexture)
					LoadTexture(pentry);
			}
		}
	}
}

//====================================
//
//====================================
void FindIntersectionPoint( const Vector &p1, const Vector &p2, const Vector &normal, const Vector &planepoint, Vector &newpoint )
{
	Vector planevec;
	Vector linevec;
	Float planedist, linedist;

	Math::VectorSubtract( planepoint, p1, planevec );
	Math::VectorSubtract( p2, p1, linevec );

	planedist = Math::DotProduct(normal, planevec);
	linedist = Math::DotProduct(normal, linevec);

	if (linedist != 0)
	{
		Math::VectorMA( p1, planedist/linedist, linevec, newpoint );
		return;
	}

	Math::VectorClear( newpoint );
};

//====================================
//
//====================================
Uint32 Decal_ClipPolygon( const Vector *arrIn, Int32 numpoints, const Vector& normal, const Vector& planepoint, Vector *arrOut )
{
	Int32 cur, prev;
	Int32 first = -1;
	Int32 outCur = 0;
	Float dots[64];

	for (Int32 i = 0; i < numpoints; i++)
	{
		Vector vecDir;
		Math::VectorSubtract( arrIn[i], planepoint, vecDir );
		dots[i] = Math::DotProduct(vecDir, normal);
		
		if (dots[i] > 0) 
			first = i;
	}

	if (first == -1) 
		return 0;

	Math::VectorCopy( arrIn[first], arrOut[outCur] );
	outCur++;

	cur = first + 1;
	if (cur == numpoints) 
		cur = 0;

	while (cur != first)
	{
		if (dots[cur] > 0)
		{
			Math::VectorCopy( arrIn[cur], arrOut[outCur] );
			cur++; outCur++;

			if (cur == numpoints) 
				cur = 0;
		}
		else
			break;
	}

	if (cur == first) 
		return outCur;

	if (dots[cur] < 0)
	{
		Vector newpoint;
		if (cur > 0) 
			prev = cur-1;
		else 
			prev = numpoints - 1;

		FindIntersectionPoint( arrIn[prev], arrIn[cur], normal, planepoint, newpoint );
		Math::VectorCopy( newpoint, arrOut[outCur] );
	}
	else
	{
		Math::VectorCopy( arrIn[cur], arrOut[outCur] );
	}

	outCur++;
	cur++;

	if (cur == numpoints) 
		cur = 0;

	while (dots[cur] < 0)
	{
		cur++;
		if (cur == numpoints) cur = 0;
	}

	if (cur > 0) 
		prev = cur-1;
	else 
		prev = numpoints - 1;

	if (dots[cur] > 0 && dots[prev] < 0)
	{
		Vector newpoint;
		FindIntersectionPoint( arrIn[prev], arrIn[cur], normal, planepoint, newpoint );
		Math::VectorCopy( newpoint, arrOut[outCur] );
		outCur++;
	}

	while (cur != first)
	{
		Math::VectorCopy( arrIn[cur], arrOut[outCur] );
		cur++; outCur++;
		if (cur == numpoints) cur = 0;
	}

	return outCur;
}