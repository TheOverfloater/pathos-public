/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "cl_main.h"
#include "r_main.h"
#include "r_common.h"
#include "trace.h"
#include "trace_shared.h"
#include "cl_pmove.h"
#include "system.h"
#include "com_math.h"
#include "cl_utils.h"
#include "r_glqueries.h"

// Glow query increment count
static const Uint32 GLOW_QUERY_INCR_COUNT = 64;

//====================================
//
//====================================
void R_InitQueryObjects( void )
{
	// Allocate query objects
	if(!rns.objects.glowqueryarray.empty())
		return;

	rns.objects.glowqueryarray.resize(GLOW_QUERY_INCR_COUNT);
}

//====================================
//
//====================================
void R_ClearQueryObjects( void )
{
	if(rns.objects.glowqueryarray.empty())
		return;

	for(Uint32 i = 0; i < rns.objects.glowqueryarray.size(); i++)
	{
		glowquery_t* pQuery = &rns.objects.glowqueryarray[i];
		if(!pQuery->queries.empty())
		{
			gGLExtF.glDeleteQueriesARB(pQuery->queries.size(), &pQuery->queries[0]);

			for(Uint32 j = 0; j < pQuery->queries.size(); j++)
				pQuery->queries[j] = 0;
		}
	}

	rns.objects.glowqueryarray.clear();
}

//====================================
//
//====================================
glowquery_t* R_AllocQueryObject( Int32 key, Uint32 numqueries, Uint32 renderpassidx, querytype_t type )
{
	glowquery_t* pQuery = nullptr;

	// See if it's already used
	for(Uint32 i = 0; i < rns.objects.glowqueryarray.size(); i++)
	{
		if(rns.objects.glowqueryarray[i].key == key 
			&& rns.objects.glowqueryarray[i].type == type
			&& rns.objects.glowqueryarray[i].renderpassidx == renderpassidx)
		{
			pQuery = &rns.objects.glowqueryarray[i];
			break;
		}
	}

	if(!pQuery)
	{
		// Look up an unused slot
		for(Uint32 i = 0; i < rns.objects.glowqueryarray.size(); i++)
		{
			if(rns.objects.glowqueryarray[i].key == NO_POSITION)
			{
				pQuery = &rns.objects.glowqueryarray[i];
				break;
			}
		}

		// Extend the array if none are available
		if(!pQuery)
		{
			Uint32 previousSize = rns.objects.glowqueryarray.size();
			rns.objects.glowqueryarray.resize(previousSize + GLOW_QUERY_INCR_COUNT);

			pQuery = &rns.objects.glowqueryarray[previousSize];
		}
	}

	pQuery->key = key;
	pQuery->type = type;
	pQuery->renderpassidx = renderpassidx;

	if(pQuery->queries.size() != numqueries)
	{
		if(!pQuery->queries.empty())
			gGLExtF.glDeleteQueriesARB(pQuery->queries.size(), &pQuery->queries[0]);

		pQuery->queries.resize(numqueries);
		gGLExtF.glGenQueriesARB(numqueries, &pQuery->queries[0]);
	}

	return pQuery;
}

//====================================
//
//====================================
Float R_CalcOcclusionFactor( const Vector& origin, 
	Int32 key, 
	Float width, 
	Float scale, 
	Float glowSpeed, 
	Uint32 numGlowTraces, 
	querytype_t queryType,
	struct glowstate_t& glowState, 
	bool useQueries, 
	bool traceAll, 
	bool checkSky,
	bool checkPortal,
	Float (*viewMatrix)[4], 
	void *pContext, 
	void (*pfnPreDrawFnPtr)( void* pContext ), 
	void (*pfnDrawFnPtr)( void* pContext, const Vector& origin )
	)
{
	// Move origin towards the view a bit
	Vector dirToView = rns.view.v_origin - origin;
	Math::VectorNormalize(dirToView);

	Vector shiftedOrigin;
	Math::VectorMA( origin, numGlowTraces-1, dirToView, shiftedOrigin );

	for(Uint32 j = 0; j < 3; j++)
		viewMatrix[j][3] = shiftedOrigin[j];

	Float flwidth = width * scale * 0.5;

	Vector left, right;
	left[0] = 0; left[1] = -flwidth / 5; left[2] = 0;
	right[0] = 0; right[1] = flwidth / 5; right[2] = 0;

	Vector aleft, aright;
	Math::VectorTransform(left, viewMatrix, aleft);
	Math::VectorTransform(right, viewMatrix, aright);
		
	Vector dist, step;
	Math::VectorSubtract(aright, aleft, dist);
	Math::VectorScale(dist, 1.0 / (numGlowTraces+1), step);

	Float totalfrac = 0;
	Float frac = (Float)1/(Float)numGlowTraces;

	if(useQueries)
	{
		glowquery_t* pObject = R_AllocQueryObject(key, numGlowTraces, rns.renderpassidx, queryType);

		if(!pObject->queried)
		{
			// Handle any pre-render stuff
			if(pfnPreDrawFnPtr)
				(*pfnPreDrawFnPtr)(pContext);

			if(g_pCvarOcclusionQueries->GetValue() <= 1)
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glPointSize(1.0);

			for (Uint32 j = 0; j < numGlowTraces; j++)
			{
				Vector start;
				Math::VectorMA(aleft, j+1, step, start);

				gGLExtF.glBeginQueryARB(GL_ANY_SAMPLES_PASSED, pObject->queries[j]);

				// Draw the object using the function
				(*pfnDrawFnPtr)(pContext, start);

				gGLExtF.glEndQueryARB(GL_ANY_SAMPLES_PASSED);
			}

			if(g_pCvarOcclusionQueries->GetValue() <= 1)
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

			pObject->queried = true;
		}

		Uint32 j = 0;
		for (; j < numGlowTraces; j++)
		{
			GLint queryResult = 0;
			gGLExtF.glGetQueryObjectivARB(pObject->queries[j], GL_QUERY_RESULT_AVAILABLE, &queryResult);
			if(queryResult != GL_TRUE)
				break;
		}

		if(rns.renderpassidx >= glowState.lastfrac.size())
			glowState.lastfrac.resize(rns.renderpassidx+1);

		if(j == numGlowTraces)
		{
			for (j = 0; j < numGlowTraces; j++)
			{
				GLint queryResult = 0;
				gGLExtF.glGetQueryObjectivARB(pObject->queries[j], GL_QUERY_RESULT, &queryResult);
				if(queryResult == GL_TRUE)
					totalfrac += frac;
			}

			// Remember for waiting state
			glowState.lastfrac[rns.renderpassidx] = totalfrac;

			// Release this query
			R_ReleaseQueryObject(pObject);
		}
		else
		{
			// Get last fraction
			totalfrac = glowState.lastfrac[rns.renderpassidx];
		}
	}
	else
	{
		for (Uint32 j = 0; j < numGlowTraces; j++)
		{
			Vector start;
			Math::VectorMA(aleft, j+1, step, start);

			Int32 traceflags = FL_TRACE_NORMAL;
			if(traceAll || checkPortal)
				traceflags |= FL_TRACE_NO_TRANS | FL_TRACE_NO_NPCS;
			else
				traceflags |= FL_TRACE_WORLD_ONLY;

			trace_t tr;
			CL_PlayerTrace( rns.view.v_origin, start, traceflags, HULL_POINT, NO_ENTITY_INDEX, tr);

			if( checkSky && !checkPortal )
			{
				if( tr.fraction == 1.0 )
					totalfrac += frac;
			}
			else if(!checkPortal)
			{
				if(CL_PointContents(tr.endpos, nullptr) == CONTENTS_SKY)
					totalfrac += frac;
			}
			else
			{
				if(tr.hitentity != NO_ENTITY_INDEX)
				{
					cl_entity_t* pEntity = CL_GetEntityByIndex(tr.hitentity);
					if(pEntity && pEntity->curstate.rendertype == RT_PORTALSURFACE)
						totalfrac += frac;
				}

			}
		}
	}

	// Check if we've been away too long & we're totally blocked
	if(rns.mainframe && totalfrac == 0)
	{
		Float flinterval = rns.time - glowState.lastrendertime;
		Float remaintime = glowState.currentalpha / glowSpeed;

		if(flinterval > remaintime)
		{
			glowState.currentalpha = 0;
			return false;
		}
	}

	Float currentalpha;
	if(rns.mainframe)
	{
		Float targetalpha = totalfrac;
		if (glowState.currentalpha > targetalpha)
		{
			glowState.currentalpha -= rns.frametime * glowSpeed;
			if (glowState.currentalpha <= targetalpha)
				glowState.currentalpha = targetalpha;
		}
		else if (glowState.currentalpha < targetalpha)
		{
			glowState.currentalpha += rns.frametime * glowSpeed;
			if (glowState.currentalpha >= targetalpha)
				glowState.currentalpha = targetalpha;
		}

		if(!glowState.currentalpha)
			return 0;

		currentalpha = glowState.currentalpha;
	}
	else
	{
		// Renderpasses do not modify glowstate.currentalpha
		currentalpha = totalfrac;
		if(!currentalpha)
			return 0;
	}

	// Save last time this was visible
	glowState.lastrendertime = rns.time;

	return currentalpha;
}

//====================================
//
//====================================
void R_ReleaseQueryObject( glowquery_t* pQuery )
{
	pQuery->key = NO_POSITION;
	pQuery->queried = false;
	pQuery->renderpassidx = 0;
	pQuery->type = GL_QUERY_UNDEFINED;
}

//====================================
//
//====================================
void R_ReleaseRenderPassQueryObjects( Uint32 renderpassidx, querytype_t type )
{
	// See if it's already used
	for(Uint32 i = 0; i < rns.objects.glowqueryarray.size(); i++)
	{
		glowquery_t& query = rns.objects.glowqueryarray[i];
		if(query.key != NO_POSITION 
			&& query.renderpassidx == renderpassidx 
			&& query.type == type)
		{
			if(rns.objects.glowqueryarray[i].queried)
			{
				Uint32 j = 0;
				for (; j < query.queries.size(); j++)
				{
					GLint queryResult = 0;
					gGLExtF.glGetQueryObjectivARB(query.queries[j], GL_QUERY_RESULT_AVAILABLE, &queryResult);
					if(queryResult != GL_TRUE)
						break;
				}

				if(j != query.queries.size())
					continue;
			}

			R_ReleaseQueryObject(&query);
		}
	}
}