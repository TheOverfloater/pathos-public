#include "qrad.h"
#include <assert.h>
#include "../../common/datatypes.h"
#include "../../common/aldformat.h"

#pragma warning(disable: 4018)
#pragma warning(disable: 4101)

// buz
#define LIGHTFLAG_NOT_NORMAL	2
#define LIGHTFLAG_NOT_RENDERER	4
//#define LIGHTFLAG_NO_BUMP		8

edgeshare_t     g_edgeshare[MAX_MAP_EDGES];
vec3_t          g_face_centroids[MAX_MAP_EDGES]; // BUG: should this be [MAX_MAP_FACES]?
bool            g_sky_lighting_fix = DEFAULT_SKY_LIGHTING_FIX;
#ifdef HLRAD_SMOOTH_TEXNORMAL
vec3_t          g_face_texnormals[MAX_MAP_FACES];
#endif

#define TEXTURE_STEP   16.0

#ifdef HLRAD_SMOOTH_TEXNORMAL
bool GetIntertexnormal (int facenum1, int facenum2, vec_t *out)
{
	vec3_t normal;
	const dplane_t *p1 = getPlaneFromFaceNumber (facenum1);
	const dplane_t *p2 = getPlaneFromFaceNumber (facenum2);
	VectorAdd (g_face_texnormals[facenum1], g_face_texnormals[facenum2], normal);
	if (!VectorNormalize (normal)
		|| DotProduct (normal, p1->normal) <= NORMAL_EPSILON
		|| DotProduct (normal, p2->normal) <= NORMAL_EPSILON
		)
	{
		return false;
	}
	if (out)
	{
		VectorCopy (normal, out);
	}
	return true;
}
#endif
// =====================================================================================
//  PairEdges
// =====================================================================================
#ifdef HLRAD_SMOOTH_FACELIST
typedef struct
{
	int numclipplanes;
	dplane_t *clipplanes;
}
intersecttest_t;
bool TestFaceIntersect (intersecttest_t *t, int facenum)
{
	dface_t *f2 = &g_dfaces[facenum];
	Winding *w = new Winding (*f2);
	int k;
	for (k = 0; k < w->m_NumPoints; k++)
	{
		VectorAdd (w->m_Points[k], g_face_offset[facenum], w->m_Points[k]);
	}
	for (k = 0; k < t->numclipplanes; k++)
	{
		if (!w->Clip (t->clipplanes[k], false
#ifdef ZHLT_WINDING_EPSILON
			, ON_EPSILON*4
#endif
			))
		{
			break;
		}
	}
	bool intersect = w->m_NumPoints > 0;
	delete w;
	return intersect;
}
intersecttest_t *CreateIntersectTest (const dplane_t *p, int facenum)
{
	dface_t *f = &g_dfaces[facenum];
	intersecttest_t *t;
	t = (intersecttest_t *)malloc (sizeof (intersecttest_t));
	hlassume (t != NULL, assume_NoMemory);
	t->clipplanes = (dplane_t *)malloc (f->numedges * sizeof (dplane_t));
	hlassume (t->clipplanes != NULL, assume_NoMemory);
	t->numclipplanes = 0;
	int j;
	for (j = 0; j < f->numedges; j++)
	{
		// should we use winding instead?
		int edgenum = g_dsurfedges[f->firstedge + j];
		{
			vec3_t v0, v1;
			vec3_t dir, normal;
			if (edgenum < 0)
			{
				VectorCopy (g_dvertexes[g_dedges[-edgenum].v[1]].point, v0);
				VectorCopy (g_dvertexes[g_dedges[-edgenum].v[0]].point, v1);
			}
			else
			{
				VectorCopy (g_dvertexes[g_dedges[edgenum].v[0]].point, v0);
				VectorCopy (g_dvertexes[g_dedges[edgenum].v[1]].point, v1);
			}
			VectorAdd (v0, g_face_offset[facenum], v0);
			VectorAdd (v1, g_face_offset[facenum], v1);
			VectorSubtract (v1, v0, dir);
			CrossProduct (dir, p->normal, normal); // facing inward
			if (!VectorNormalize (normal))
			{
				continue;
			}
			VectorCopy (normal, t->clipplanes[t->numclipplanes].normal);
			t->clipplanes[t->numclipplanes].dist = DotProduct (v0, normal);
			t->numclipplanes++;
		}
	}
	return t;
}
void FreeIntersectTest (intersecttest_t *t)
{
	free (t->clipplanes);
	free (t);
}
#endif
#ifdef HLRAD_GetPhongNormal_VL
void AddFaceForVertexNormal_printerror (const int edgeabs, const int edgeend, dface_t *const f)
{
	if (DEVELOPER_LEVEL_WARNING <= g_developer)
	{
		int i, e;
		Log ("AddFaceForVertexNormal - bad face:\n");
		Log (" edgeabs=%d edgeend=%d\n", edgeabs, edgeend);
		for (i = 0; i < f->numedges; i++)
		{
			e = g_dsurfedges[f->firstedge + i];
			edgeshare_t *es = &g_edgeshare[abs(e)];
			int v0 = g_dedges[abs(e)].v[0], v1 = g_dedges[abs(e)].v[1];
			Log (" e=%d v0=%d(%f,%f,%f) v1=%d(%f,%f,%f) share0=%d share1=%d\n", e,
				v0, g_dvertexes[v0].point[0], g_dvertexes[v0].point[1], g_dvertexes[v0].point[2],
				v1, g_dvertexes[v1].point[0], g_dvertexes[v1].point[1], g_dvertexes[v1].point[2],
				(es->faces[0]==NULL? -1: es->faces[0]-g_dfaces), (es->faces[1]==NULL? -1: es->faces[1]-g_dfaces));
		}
	}
}
int AddFaceForVertexNormal (const int edgeabs, int &edgeabsnext, const int edgeend, int &edgeendnext, dface_t *const f, dface_t *&fnext, vec_t &angle, vec3_t &normal)
// Must guarantee these faces will form a loop or a chain, otherwise will result in endless loop.
//
//   e[end]/enext[endnext]
//  *
//  |\
//  |a\ fnext
//  |  \
//  | f \
//  |    \
//  e   enext
//
{
	VectorCopy(getPlaneFromFace(f)->normal, normal);
	int vnum = g_dedges[edgeabs].v[edgeend];
	int iedge, iedgenext, edge, edgenext;
	int i, e, count1, count2;
	vec_t dot;
	for (count1 = count2 = 0, i = 0; i < f->numedges; i++)
	{
		e = g_dsurfedges[f->firstedge + i];
		if (g_dedges[abs(e)].v[0] == g_dedges[abs(e)].v[1])
			continue;
		if (abs(e) == edgeabs)
		{
			iedge = i;
			edge = e;
			count1 ++;
		}
		else if (g_dedges[abs(e)].v[0] == vnum || g_dedges[abs(e)].v[1] == vnum)
		{
			iedgenext = i;
			edgenext = e;
			count2 ++;
		}
	}
	if (count1 != 1 || count2 != 1)
	{
		AddFaceForVertexNormal_printerror (edgeabs, edgeend, f);
		return -1;
	}
	int vnum11, vnum12, vnum21, vnum22;
	vec3_t vec1, vec2;
	vnum11 = g_dedges[abs(edge)].v[edge>0?0:1];
	vnum12 = g_dedges[abs(edge)].v[edge>0?1:0];
	vnum21 = g_dedges[abs(edgenext)].v[edgenext>0?0:1];
	vnum22 = g_dedges[abs(edgenext)].v[edgenext>0?1:0];
	if (vnum == vnum12 && vnum == vnum21 && vnum != vnum11 && vnum != vnum22)
	{
		VectorSubtract(g_dvertexes[vnum11].point, g_dvertexes[vnum].point, vec1);
		VectorSubtract(g_dvertexes[vnum22].point, g_dvertexes[vnum].point, vec2);
		edgeabsnext = abs(edgenext);
		edgeendnext = edgenext>0?0:1;
	}
	else if (vnum == vnum11 && vnum == vnum22 && vnum != vnum12 && vnum != vnum21)
	{
		VectorSubtract(g_dvertexes[vnum12].point, g_dvertexes[vnum].point, vec1);
		VectorSubtract(g_dvertexes[vnum21].point, g_dvertexes[vnum].point, vec2);
		edgeabsnext = abs(edgenext);
		edgeendnext = edgenext>0?1:0;
	}
	else
	{
		AddFaceForVertexNormal_printerror (edgeabs, edgeend, f);
		return -1;
	}
	VectorNormalize(vec1);
	VectorNormalize(vec2);
	dot = DotProduct(vec1,vec2);
	dot = dot>1? 1: dot<-1? -1: dot;
	angle = acos(dot);
	edgeshare_t *es = &g_edgeshare[edgeabsnext];
	if (!(es->faces[0] && es->faces[1]))
		return 1;
	if (es->faces[0] == f && es->faces[1] != f)
		fnext = es->faces[1];
	else if (es->faces[1] == f && es->faces[0] != f)
		fnext = es->faces[0];
	else
	{
		AddFaceForVertexNormal_printerror (edgeabs, edgeend, f);
		return -1;
	}
	return 0;
}
#endif
void            PairEdges()
{
    int             i, j, k;
    dface_t*        f;
    edgeshare_t*    e;

    memset(&g_edgeshare, 0, sizeof(g_edgeshare));

    f = g_dfaces;
    for (i = 0; i < g_numfaces; i++, f++)
    {
#ifdef HLRAD_SMOOTH_TEXNORMAL
		{
			const dplane_t *fp = getPlaneFromFace (f);
			vec3_t texnormal;
			const texinfo_t *tex = &g_texinfo[f->texinfo];
			CrossProduct (tex->vecs[1], tex->vecs[0], texnormal);
			VectorNormalize (texnormal);
			if (DotProduct (texnormal, fp->normal) < 0)
			{
				VectorSubtract (vec3_origin, texnormal, texnormal);
			}
			VectorCopy (texnormal, g_face_texnormals[i]);
		}
#endif
#ifdef HLRAD_EDGESHARE_NOSPECIAL
		if (g_texinfo[f->texinfo].flags & TEX_SPECIAL)
		{
			// special textures don't have lightmaps
			continue;
		}
#endif
        for (j = 0; j < f->numedges; j++)
        {
            k = g_dsurfedges[f->firstedge + j];
            if (k < 0)
            {
                e = &g_edgeshare[-k];

                hlassert(e->faces[1] == NULL);
                e->faces[1] = f;
            }
            else
            {
                e = &g_edgeshare[k];

                hlassert(e->faces[0] == NULL);
                e->faces[0] = f;
            }

            if (e->faces[0] && e->faces[1]) {
				// determine if coplanar
				if (e->faces[0]->planenum == e->faces[1]->planenum
#ifdef HLRAD_PairEdges_FACESIDE_FIX
					&& e->faces[0]->side == e->faces[1]->side
#endif
					) {
						e->coplanar = true;
				} else {
                    // see if they fall into a "smoothing group" based on angle of the normals
                    vec3_t          normals[2];

                    VectorCopy(getPlaneFromFace(e->faces[0])->normal, normals[0]);
                    VectorCopy(getPlaneFromFace(e->faces[1])->normal, normals[1]);

                    e->cos_normals_angle = DotProduct(normals[0], normals[1]);

#ifdef HLRAD_CUSTOMSMOOTH
					vec_t smoothvalue;
					int m0 = g_texinfo[e->faces[0]->texinfo].miptex;
					int m1 = g_texinfo[e->faces[1]->texinfo].miptex;
					smoothvalue = qmax (g_smoothvalues[m0], g_smoothvalues[m1]);
					if (m0 != m1)
					{
						smoothvalue = qmax (smoothvalue, g_smoothing_threshold_2);
					}
					if (smoothvalue >= 1.0 - NORMAL_EPSILON)
					{
						smoothvalue = 2.0;
					}
#endif
                    if (e->cos_normals_angle > (1.0 - NORMAL_EPSILON))
                    {
                        e->coplanar = true;
                    }
#ifndef HLRAD_CUSTOMSMOOTH
                    else if (g_smoothing_threshold > 0.0)
#endif
                    {
#ifdef HLRAD_CUSTOMSMOOTH
                        if (e->cos_normals_angle >= qmax (smoothvalue - NORMAL_EPSILON, NORMAL_EPSILON))
#else
                        if (e->cos_normals_angle >= g_smoothing_threshold)
#endif
                        {
                            VectorAdd(normals[0], normals[1], e->interface_normal);
                            VectorNormalize(e->interface_normal);
                        }
                    }
                }
#ifdef HLRAD_TRANSLUCENT
				if (!VectorCompare (g_translucenttextures[g_texinfo[e->faces[0]->texinfo].miptex], g_translucenttextures[g_texinfo[e->faces[1]->texinfo].miptex]))
				{
					e->coplanar = false;
					VectorClear (e->interface_normal);
				}
#endif
#ifdef HLRAD_GetPhongNormal_VL
				if (e->coplanar)
					VectorCopy(getPlaneFromFace(e->faces[0])->normal, e->interface_normal);
				if (!VectorCompare(e->interface_normal, vec3_origin))
					e->smooth = true;
#ifdef HLRAD_SMOOTH_TEXNORMAL
				if (!GetIntertexnormal (e->faces[0] - g_dfaces, e->faces[1] - g_dfaces))
				{
					e->coplanar = false;
					VectorClear (e->interface_normal);
					e->smooth = false;
				}
#endif
#endif
            }
        }
    }
#ifdef HLRAD_GetPhongNormal_VL
	{
		int edgeabs, edgeabsnext;
		int edgeend, edgeendnext;
		int d;
		dface_t *f, *fcurrent, *fnext;
		vec_t angle, angles;
		vec3_t normal, normals;
		vec3_t edgenormal;
		int r, count;
		for (edgeabs = 0; edgeabs < MAX_MAP_EDGES; edgeabs++)
		{
			e = &g_edgeshare[edgeabs];
			if (!e->smooth)
				continue;
			VectorCopy(e->interface_normal, edgenormal);
			if (g_dedges[edgeabs].v[0] == g_dedges[edgeabs].v[1])
			{
				vec3_t errorpos;
				VectorCopy (g_dvertexes[g_dedges[edgeabs].v[0]].point, errorpos);
				VectorAdd (errorpos, g_face_offset[e->faces[0] - g_dfaces], errorpos);
				Developer (DEVELOPER_LEVEL_WARNING, "PairEdges: invalid edge at (%f,%f,%f)", errorpos[0], errorpos[1], errorpos[2]);
				VectorCopy(edgenormal, e->vertex_normal[0]);
				VectorCopy(edgenormal, e->vertex_normal[1]);
			}
			else
			{
				const dplane_t *p0 = getPlaneFromFace (e->faces[0]);
				const dplane_t *p1 = getPlaneFromFace (e->faces[1]);
#ifdef HLRAD_SMOOTH_FACELIST
				intersecttest_t *test0 = CreateIntersectTest (p0, e->faces[0] - g_dfaces);
				intersecttest_t *test1 = CreateIntersectTest (p1, e->faces[1] - g_dfaces);
#endif
				for (edgeend = 0; edgeend < 2; edgeend++)
				{
					vec3_t errorpos;
					VectorCopy (g_dvertexes[g_dedges[edgeabs].v[edgeend]].point, errorpos);
					VectorAdd (errorpos, g_face_offset[e->faces[0] - g_dfaces], errorpos);
					angles = 0;
					VectorClear (normals);

					for (d = 0; d < 2; d++)
					{
						f = e->faces[d];
						count = 0, fnext = f, edgeabsnext = edgeabs, edgeendnext = edgeend;
						while (1)
						{
							fcurrent = fnext;
							r = AddFaceForVertexNormal (edgeabsnext, edgeabsnext, edgeendnext, edgeendnext, fcurrent, fnext, angle, normal);
							count++;
							if (r == -1)
							{
								Developer (DEVELOPER_LEVEL_WARNING, "PairEdges: face edges mislink at (%f,%f,%f)", errorpos[0], errorpos[1], errorpos[2]);
								break;
							}
							if (count >= 100)
							{
								Developer (DEVELOPER_LEVEL_WARNING, "PairEdges: faces mislink at (%f,%f,%f)", errorpos[0], errorpos[1], errorpos[2]);
								break;
							}
							if (DotProduct (normal, p0->normal) <= NORMAL_EPSILON || DotProduct(normal, p1->normal) <= NORMAL_EPSILON)
								break;
	#ifdef HLRAD_CUSTOMSMOOTH
							vec_t smoothvalue;
							int m0 = g_texinfo[f->texinfo].miptex;
							int m1 = g_texinfo[fcurrent->texinfo].miptex;
							smoothvalue = qmax (g_smoothvalues[m0], g_smoothvalues[m1]);
							if (m0 != m1)
							{
								smoothvalue = qmax (smoothvalue, g_smoothing_threshold_2);
							}
							if (smoothvalue >= 1.0 - NORMAL_EPSILON)
							{
								smoothvalue = 2.0;
							}
							if (DotProduct (edgenormal, normal) < qmax (smoothvalue - NORMAL_EPSILON, NORMAL_EPSILON))
	#else
							if (DotProduct (edgenormal, normal) + NORMAL_EPSILON < g_smoothing_threshold)
	#endif
								break;
	#ifdef HLRAD_SMOOTH_TEXNORMAL
							if (!GetIntertexnormal (fcurrent - g_dfaces, e->faces[0] - g_dfaces) || !GetIntertexnormal (fcurrent - g_dfaces, e->faces[1] - g_dfaces))
								break;
	#endif
	#ifdef HLRAD_SMOOTH_FACELIST
							if (fcurrent != e->faces[0] && fcurrent != e->faces[1] &&
								(TestFaceIntersect (test0, fcurrent - g_dfaces) || TestFaceIntersect (test1, fcurrent - g_dfaces)))
							{
								Developer (DEVELOPER_LEVEL_WARNING, "Overlapping faces around corner (%f,%f,%f)\n", errorpos[0], errorpos[1], errorpos[2]);
								break;
							}
	#endif
							angles += angle;
							VectorMA(normals, angle, normal, normals);
	#ifdef HLRAD_SMOOTH_FACELIST
							{
								bool in = false;
								if (fcurrent == e->faces[0] || fcurrent == e->faces[1])
								{
									in = true;
								}
								for (facelist_t *l = e->vertex_facelist[edgeend]; l; l = l->next)
								{
									if (fcurrent == l->face)
									{
										in = true;
									}
								}
								if (!in)
								{
									facelist_t *l = (facelist_t *)malloc (sizeof (facelist_t));
									hlassume (l != NULL, assume_NoMemory);
									l->face = fcurrent;
									l->next = e->vertex_facelist[edgeend];
									e->vertex_facelist[edgeend] = l;
								}
							}
	#endif
							if (r != 0 || fnext == f)
								break;
						}
					}

					if (angles < NORMAL_EPSILON)
					{
						VectorCopy(edgenormal, e->vertex_normal[edgeend]);
						Developer (DEVELOPER_LEVEL_WARNING, "PairEdges: no valid faces at (%f,%f,%f)", errorpos[0], errorpos[1], errorpos[2]);
					}
					else
					{
						VectorNormalize(normals);
						VectorCopy(normals, e->vertex_normal[edgeend]);
					}
				}
#ifdef HLRAD_SMOOTH_FACELIST
				FreeIntersectTest (test0);
				FreeIntersectTest (test1);
#endif
			}
			if (e->coplanar)
			{
				if (!VectorCompare (e->vertex_normal[0], e->interface_normal) || !VectorCompare (e->vertex_normal[1], e->interface_normal))
					e->coplanar = false;
			}
		}
	}
#endif
}

#define	MAX_SINGLEMAP	(18*18*4)

typedef struct
{
    vec_t*          light;
    vec_t           facedist;
    vec3_t          facenormal;
#ifdef HLRAD_TRANSLUCENT
	bool			translucent_b;
	vec3_t			translucent_v;
#endif

    int             numsurfpt;
    vec3_t          surfpt[MAX_SINGLEMAP];
#ifdef HLRAD_CalcPoints_NEW
	bool			lightoutside[MAX_SINGLEMAP];
#endif

    vec3_t          texorg;
    vec3_t          worldtotex[2];                         // s = (world - texorg) . worldtotex[0]
    vec3_t          textoworld[2];                         // world = texorg + s * textoworld[0]
#ifdef HLRAD_CalcPoints_NEW
	vec3_t			texnormal;
#endif

    vec_t           exactmins[2], exactmaxs[2];

    int             texmins[2], texsize[2];
    int             lightstyles[256];
    int             surfnum;
    dface_t*        face;
#ifdef HLRAD_BLUR
	int				lmcache_density; // shared by both s and t direction
	int				lmcache_offset; // shared by both s and t direction
#ifdef HLRAD_AUTOCORING
	vec3_t			(*lmcache)[ALLSTYLES]; // lm = lightmap // don't forget to free!
#ifdef ZHLT_XASH
	vec3_t			(*lmcache_direction)[ALLSTYLES];
#endif
#else
	vec3_t			(*lmcache)[MAXLIGHTMAPS];
#endif
	int				lmcachewidth;
	int				lmcacheheight;
#endif
}
lightinfo_t;
#ifdef HLRAD_MDL_LIGHT_HACK
#ifndef HLRAD_MDL_LIGHT_HACK_NEW
typedef struct
{
	vec3_t			texorg;
	vec3_t			offset;
	vec3_t			textoworld[2];
	vec3_t			worldtotex[2];
	int				texmins[2], texsize[2];
}
facesampleinfo_t;
static facesampleinfo_t facesampleinfo[MAX_MAP_FACES];
#endif
#endif

// =====================================================================================
//  TextureNameFromFace
// =====================================================================================
static const char* TextureNameFromFace(const dface_t* const f)
{
    texinfo_t*      tx;
    miptex_t*       mt;
    int             ofs;

    //
    // check for light emited by texture
    //
    tx = &g_texinfo[f->texinfo];

    ofs = ((dmiptexlump_t*)g_dtexdata)->dataofs[tx->miptex];
    mt = (miptex_t*)((byte*) g_dtexdata + ofs);

	return mt->name;
}

// =====================================================================================
//  CalcFaceExtents
//      Fills in s->texmins[] and s->texsize[]
//      also sets exactmins[] and exactmaxs[]
// =====================================================================================
#ifdef HLRAD_MAXEXTENT
bool g_warnedextent = false;
#endif
static void     CalcFaceExtents(lightinfo_t* l)
{
    const int       facenum = l->surfnum;
    dface_t*        s;
    float           mins[2], maxs[2], val; //vec_t           mins[2], maxs[2], val; //vluzacn
    int             i, j, e;
    dvertex_t*      v;
    texinfo_t*      tex;

    s = l->face;

    mins[0] = mins[1] = 999999;
    maxs[0] = maxs[1] = -99999; // a little small, but same with Goldsrc. --vluzacn

    tex = &g_texinfo[s->texinfo];

    for (i = 0; i < s->numedges; i++)
    {
        e = g_dsurfedges[s->firstedge + i];
        if (e >= 0)
        {
            v = g_dvertexes + g_dedges[e].v[0];
        }
        else
        {
            v = g_dvertexes + g_dedges[-e].v[1];
        }

        for (j = 0; j < 2; j++)
        {
            val = v->point[0] * tex->vecs[j][0] +
                v->point[1] * tex->vecs[j][1] + v->point[2] * tex->vecs[j][2] + tex->vecs[j][3];
            if (val < mins[j])
            {
                mins[j] = val;
            }
            if (val > maxs[j])
            {
                maxs[j] = val;
            }
        }
    }

    for (i = 0; i < 2; i++)
    {
        l->exactmins[i] = mins[i];
        l->exactmaxs[i] = maxs[i];

        mins[i] = floor(mins[i] / 16.0);
        maxs[i] = ceil(maxs[i] / 16.0);

		l->texmins[i] = mins[i];
        l->texsize[i] = maxs[i] - mins[i];
	}

	if (!(tex->flags & TEX_SPECIAL))
	{
#ifdef HLRAD_MAXEXTENT
		if (l->texsize[0] < 0 || l->texsize[1] < 0 || l->texsize[0] > 17 || l->texsize[1] > 17)
#else
		if ((l->texsize[0] > 16) || (l->texsize[1] > 16)
			|| l->texsize[0] < 0 || l->texsize[1] < 0 //--vluzacn
			)
#endif
		{
			ThreadLock();
			PrintOnce("\nfor Face %d (texture %s) at ", s - g_dfaces, TextureNameFromFace(s));

			for (i = 0; i < s->numedges; i++)
			{
				e = g_dsurfedges[s->firstedge + i];
				if (e >= 0)
                {
					v = g_dvertexes + g_dedges[e].v[0];
                }
				else
                {
					v = g_dvertexes + g_dedges[-e].v[1];
                }
#ifdef HLRAD_OVERWRITEVERTEX_FIX
				vec3_t pos;
				VectorAdd (v->point, g_face_offset[facenum], pos);
				Log ("(%4.3f %4.3f %4.3f) ", pos[0], pos[1], pos[2]);
#else
                VectorAdd(v->point, g_face_offset[facenum], v->point);
				Log("(%4.3f %4.3f %4.3f) ", v->point[0], v->point[1], v->point[2]);
#endif
			}
			Log("\n");

			Error( "Bad surface extents (%d x %d)\nCheck the file ZHLTProblems.html for a detailed explanation of this problem", l->texsize[0], l->texsize[1]);
		}
#ifdef HLRAD_MAXEXTENT
		if (l->texsize[0] > 16 || l->texsize[1] > 16)
		{
			if (!g_warnedextent)
			{ // only warn once
				ThreadLock ();
				if (!g_warnedextent)
				{
					g_warnedextent = true;
					Warning ("\nfor Face %d (texture %s) at ", s - g_dfaces, TextureNameFromFace(s));
					for (i = 0; i < s->numedges; i++)
					{
						e = g_dsurfedges[s->firstedge + i];
						if (e >= 0)
						{
							v = g_dvertexes + g_dedges[e].v[0];
						}
						else
						{
							v = g_dvertexes + g_dedges[-e].v[1];
						}
#ifdef HLRAD_OVERWRITEVERTEX_FIX
						vec3_t pos;
						VectorAdd (v->point, g_face_offset[facenum], pos);
						Log ("(%4.3f %4.3f %4.3f) ", pos[0], pos[1], pos[2]);
#else
						VectorAdd(v->point, g_face_offset[facenum], v->point);
						Log("(%4.3f %4.3f %4.3f) ", v->point[0], v->point[1], v->point[2]);
#endif
					}
					Log("\n");
					Warning ("Surface extents (%d x %d) exceeded (%d x %d)\nThis map will not work in 'Software' video mode or HLDS.", l->texsize[0], l->texsize[1], 16, 16);
				}
				ThreadUnlock ();
			}
		}
#endif
	}
#ifdef HLRAD_BLUR
	// allocate sample light cache
	{
		if (g_extra
	#ifdef HLRAD_FASTMODE
			&& !g_fastmode
	#endif
			)
		{
			l->lmcache_density = 3;
		}
		else
		{
			l->lmcache_density = 1;
		}
		int side = (int)ceil ((0.5 * g_blur * l->lmcache_density - 0.5) * (1 - NORMAL_EPSILON));
		l->lmcache_offset = side;
		l->lmcachewidth = l->texsize[0] * l->lmcache_density + 1 + 2 * side;
		l->lmcacheheight = l->texsize[1] * l->lmcache_density + 1 + 2 * side;
	#ifdef HLRAD_AUTOCORING
		l->lmcache = (vec3_t (*)[ALLSTYLES])malloc (l->lmcachewidth * l->lmcacheheight * sizeof (vec3_t [ALLSTYLES]));
#ifdef ZHLT_XASH
		l->lmcache_direction = (vec3_t (*)[ALLSTYLES])malloc (l->lmcachewidth * l->lmcacheheight * sizeof (vec3_t [ALLSTYLES]));
#endif
	#else
		l->lmcache = (vec3_t (*)[MAXLIGHTMAPS])malloc (l->lmcachewidth * l->lmcacheheight * sizeof (vec3_t [MAXLIGHTMAPS]));
	#endif
		hlassume (l->lmcache != NULL, assume_NoMemory);
#ifdef ZHLT_XASH
		hlassume (l->lmcache_direction != NULL, assume_NoMemory);
#endif
	}
#endif
}

// =====================================================================================
//  CalcFaceVectors
//      Fills in texorg, worldtotex. and textoworld
// =====================================================================================
static void     CalcFaceVectors(lightinfo_t* l)
{
    texinfo_t*      tex;
    int             i, j;
    vec3_t          texnormal;
    vec_t           distscale;
    vec_t           dist, len;

    tex = &g_texinfo[l->face->texinfo];

    // convert from float to double
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 3; j++)
        {
            l->worldtotex[i][j] = tex->vecs[i][j];
        }
    }

    // calculate a normal to the texture axis.  points can be moved along this
    // without changing their S/T
    CrossProduct(tex->vecs[1], tex->vecs[0], texnormal);
    VectorNormalize(texnormal);

    // flip it towards plane normal
    distscale = DotProduct(texnormal, l->facenormal);

    if (distscale < 0)
    {
        distscale = -distscale;
        VectorSubtract(vec3_origin, texnormal, texnormal);
    }

    // distscale is the ratio of the distance along the texture normal to
    // the distance along the plane normal
    distscale = 1.0 / distscale;

#ifdef ZHLT_FREETEXTUREAXIS
	for (i = 0; i < 2; i++)
	{
		CrossProduct (l->worldtotex[!i], l->facenormal, l->textoworld[i]);
		len = DotProduct (l->textoworld[i], l->worldtotex[i]);
		VectorScale (l->textoworld[i], 1 / len, l->textoworld[i]);
	}
#else
    for (i = 0; i < 2; i++)
    {
        len = (float)VectorLength(l->worldtotex[i]);
        dist = DotProduct(l->worldtotex[i], l->facenormal);
        dist *= distscale;
        VectorMA(l->worldtotex[i], -dist, texnormal, l->textoworld[i]);
        VectorScale(l->textoworld[i], (1 / len) * (1 / len), l->textoworld[i]);
    }
#endif

    // calculate texorg on the texture plane
    for (i = 0; i < 3; i++)
    {
        l->texorg[i] = -tex->vecs[0][3] * l->textoworld[0][i] - tex->vecs[1][3] * l->textoworld[1][i];
    }

    // project back to the face plane
    dist = DotProduct(l->texorg, l->facenormal) - l->facedist - DEFAULT_HUNT_OFFSET;
    dist *= distscale;
    VectorMA(l->texorg, -dist, texnormal, l->texorg);
#ifdef HLRAD_CalcPoints_NEW
	VectorCopy (texnormal, l->texnormal);
#endif

}

// =====================================================================================
//  SetSurfFromST
// =====================================================================================
static void     SetSurfFromST(const lightinfo_t* const l, vec_t* surf, const vec_t s, const vec_t t)
{
    const int       facenum = l->surfnum;
    int             j;

    for (j = 0; j < 3; j++)
    {
        surf[j] = l->texorg[j] + l->textoworld[0][j] * s + l->textoworld[1][j] * t;
    }

    // Adjust for origin-based models
    VectorAdd(surf, g_face_offset[facenum], surf);
}

#ifndef HLRAD_CalcPoints_NEW
// =====================================================================================
//  FindSurfaceMidpoint
// =====================================================================================
static dleaf_t* FindSurfaceMidpoint(const lightinfo_t* const l, vec_t* midpoint)
{
    int             s, t;
    int             w, h;
    vec_t           starts, startt;
    vec_t           us, ut;

    vec3_t          broken_midpoint;
    vec3_t          surface_midpoint;
    int             inside_point_count;

    dleaf_t*        last_valid_leaf = NULL;
    dleaf_t*        leaf_mid;

    const int       facenum = l->surfnum;
    const dface_t*  f = g_dfaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);

    const vec_t*    face_delta = g_face_offset[facenum];

    h = l->texsize[1] + 1;
    w = l->texsize[0] + 1;
    starts = (float)l->texmins[0] * 16;
    startt = (float)l->texmins[1] * 16;

    // General case
    inside_point_count = 0;
    VectorClear(surface_midpoint);
    for (t = 0; t < h; t++)
    {
        for (s = 0; s < w; s++)
        {
            us = starts + s * TEXTURE_STEP;
            ut = startt + t * TEXTURE_STEP;

            SetSurfFromST(l, midpoint, us, ut);
            if ((leaf_mid = PointInLeaf(midpoint)) != g_dleafs)
            {
                if ((leaf_mid->contents != CONTENTS_SKY) && (leaf_mid->contents != CONTENTS_SOLID))
                {
                    last_valid_leaf = leaf_mid;
                    inside_point_count++;
                    VectorAdd(surface_midpoint, midpoint, surface_midpoint);
                }
            }
        }
    }

    if (inside_point_count > 1)
    {
        vec_t           tmp = 1.0 / inside_point_count;

        VectorScale(surface_midpoint, tmp, midpoint);

        //Verbose("Trying general at (%4.3f %4.3f %4.3f) %d\n", surface_midpoint[0], surface_midpoint[1], surface_midpoint[2], inside_point_count);
        if (
            (leaf_mid =
             HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET)))
        {
            //Verbose("general method succeeded at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
            return leaf_mid;
        }
        //Verbose("Tried general , failed at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
    }
    else if (inside_point_count == 1)
    {
        //Verbose("Returning single point from general\n");
        VectorCopy(surface_midpoint, midpoint);
        return last_valid_leaf;
    }
    else
    {
        //Verbose("general failed (no points)\n");
    }

    // Try harder
    inside_point_count = 0;
    VectorClear(surface_midpoint);
    for (t = 0; t < h; t++)
    {
        for (s = 0; s < w; s++)
        {
            us = starts + s * TEXTURE_STEP;
            ut = startt + t * TEXTURE_STEP;

            SetSurfFromST(l, midpoint, us, ut);
            leaf_mid =
                HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);
            if (leaf_mid != g_dleafs)
            {
                last_valid_leaf = leaf_mid;
                inside_point_count++;
                VectorAdd(surface_midpoint, midpoint, surface_midpoint);
            }
        }
    }

    if (inside_point_count > 1)
    {
        vec_t           tmp = 1.0 / inside_point_count;

        VectorScale(surface_midpoint, tmp, midpoint);

        if (
            (leaf_mid =
             HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET)))
        {
            //Verbose("best method succeeded at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
            return leaf_mid;
        }
        //Verbose("Tried best, failed at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
    }
    else if (inside_point_count == 1)
    {
        //Verbose("Returning single point from best\n");
        VectorCopy(surface_midpoint, midpoint);
        return last_valid_leaf;
    }
    else
    {
        //Verbose("best failed (no points)\n");
    }

    // Original broken code
    {
        vec_t           mids = (l->exactmaxs[0] + l->exactmins[0]) / 2;
        vec_t           midt = (l->exactmaxs[1] + l->exactmins[1]) / 2;

        SetSurfFromST(l, midpoint, mids, midt);

        if ((leaf_mid = PointInLeaf(midpoint)) != g_dleafs)
        {
            if ((leaf_mid->contents != CONTENTS_SKY) && (leaf_mid->contents != CONTENTS_SOLID))
            {
                return leaf_mid;
            }
        }

        VectorCopy(midpoint, broken_midpoint);
        //Verbose("Tried original method, failed at (%4.3f %4.3f %4.3f)\n", midpoint[0], midpoint[1], midpoint[2]);
    }

    VectorCopy(broken_midpoint, midpoint);
    return HuntForWorld(midpoint, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);
}

// =====================================================================================
//  SimpleNudge
//      Return vec_t in point only valid when function returns true
//      Use negative scales to push away from center instead
// =====================================================================================
static bool     SimpleNudge(vec_t* const point, const lightinfo_t* const l, vec_t* const s, vec_t* const t, const vec_t delta)
{
    const int       facenum = l->surfnum;
    const dface_t*  f = g_dfaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);
    const vec_t*    face_delta = g_face_offset[facenum];
    const int       h = l->texsize[1] + 1;
    const int       w = l->texsize[0] + 1;
    const vec_t     half_w = (vec_t)(w - 1) / 2.0;
    const vec_t     half_h = (vec_t)(h - 1) / 2.0;
    const vec_t     s_vec = *s;
    const vec_t     t_vec = *t;
    vec_t           s1;
    vec_t           t1;

    if (s_vec > half_w)
    {
        s1 = s_vec - delta;
    }
    else
    {
        s1 = s_vec + delta;
    }

    SetSurfFromST(l, point, s1, t_vec);
    if (HuntForWorld(point, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET))
    {
        *s = s1;
        return true;
    }

    if (t_vec > half_h)
    {
        t1 = t_vec - delta;
    }
    else
    {
        t1 = t_vec + delta;
    }

    SetSurfFromST(l, point, s_vec, t1);
    if (HuntForWorld(point, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET))
    {
        *t = t1;
        return true;
    }

    return false;
}
#endif

typedef enum
{
    LightOutside,                                          // Not lit
    LightShifted,                                          // used HuntForWorld on 100% dark face
    LightShiftedInside,                                    // moved to neighbhor on 2nd cleanup pass
    LightNormal,                                           // Normally lit with no movement
    LightPulledInside,                                     // Pulled inside by bleed code adjustments
    LightSimpleNudge,                                      // A simple nudge 1/3 or 2/3 towards center along S or T axist
#ifndef HLRAD_NUDGE_VL
    LightSimpleNudgeEmbedded                               // A nudge either 1 full unit in each of S and T axis, or 1/3 or 2/3 AWAY from center
#endif
}
light_flag_t;

// =====================================================================================
//  CalcPoints
//      For each texture aligned grid point, back project onto the plane
//      to get the world xyz value of the sample point
// =====================================================================================
#ifdef HLRAD_CalcPoints_NEW
static int		PointInFace(const lightinfo_t *l, const vec_t* point)
{
	int facenum = l->surfnum;
	const dface_t* f = &g_dfaces[facenum];
	Winding *w;
	dplane_t plane;
	VectorCopy (l->texnormal, plane.normal);
	const dplane_t *p = &plane;
	vec3_t new_point;
	VectorSubtract (point, g_face_offset[facenum], new_point);
	w = new Winding (*f);
	if (point_in_winding (*w, *p, new_point))
	{
		delete w;
		return facenum;
	}
	delete w;

	int j;
	for (j = 0; j < f->numedges; j++)
	{
		int e;
		edgeshare_t *es;
		dface_t* f2;
		e = g_dsurfedges[f->firstedge + j];
		es = &g_edgeshare[abs(e)];
		if (!es->smooth)
			continue;
		f2 = es->faces[!(e<0)];
		const dplane_t *p2 = getPlaneFromFace (f2);
		if (DotProduct (p->normal, p2->normal) < NORMAL_EPSILON)
			continue;
		w = new Winding (*f2);
		if (point_in_winding (*w, *p, new_point))
		{
			delete w;
			return f2 - g_dfaces;
		}
		delete w;
	}
#ifdef HLRAD_SMOOTH_FACELIST
	for (j = 0; j < f->numedges; j++)
	{
		int e;
		edgeshare_t *es;
		dface_t* f2;
		e = g_dsurfedges[f->firstedge + j];
		es = &g_edgeshare[abs(e)];
		if (!es->smooth)
			continue;
		for (int edgeend = 0; edgeend < 2; edgeend++)
		{
			for (facelist_t *l = es->vertex_facelist[edgeend]; l; l = l->next)
			{
				f2 = l->face;
				const dplane_t *p2 = getPlaneFromFace (f2);
				if (DotProduct (p->normal, p2->normal) < NORMAL_EPSILON)
					continue;
				w = new Winding (*f2);
				if (point_in_winding (*w, *p, new_point))
				{
					delete w;
					return f2 - g_dfaces;
				}
				delete w;
			}
		}
	}
#endif
	return facenum;
}
static void		SetSTFromSurf(const lightinfo_t* const l, const vec_t* surf, vec_t& s, vec_t& t)
{
    const int       facenum = l->surfnum;
    int             j;

	s = t = 0;
	for (j = 0; j < 3; j++)
	{
		s += (surf[j] - g_face_offset[facenum][j] - l->texorg[j]) * l->worldtotex[0][j];
		t += (surf[j] - g_face_offset[facenum][j] - l->texorg[j]) * l->worldtotex[1][j];
	}
}
static light_flag_t SetSampleFromST(vec_t* const point, const lightinfo_t* const l, const vec_t original_s, const vec_t original_t, eModelLightmodes lightmode)
{
	light_flag_t	LuxelFlag = LightOutside;
	int				huntsize = 3;
	vec_t			huntscale = 0.2;
	vec_t			width = DEFAULT_EDGE_WIDTH;

	int				facenum = l->surfnum;
	const vec_t*	face_delta = g_face_offset[facenum];

	const dface_t*	f = &g_dfaces[facenum];
	const dplane_t*	p = getPlaneFromFace (f);
	Winding			*wd = new Winding (*f);
	{
		int				j;
		for (j = 0; j < wd->m_NumPoints; j++)
		{
			VectorAdd (wd->m_Points[j], face_delta, wd->m_Points[j]);
		}
	}

	const vec_t*	face_centroid = g_face_centroids[facenum];
	vec_t			mids, midt;
	SetSTFromSurf (l, face_centroid, mids, midt);

	vec3_t			surf_original;
	dleaf_t*		leaf_original;
	SetSurfFromST (l, surf_original, original_s, original_t);
	leaf_original = HuntForWorld (surf_original, face_delta, p, 1, 0.0, DEFAULT_HUNT_OFFSET);

	int				facenum_tosnap = PointInFace (l, surf_original);
	const dface_t*	f_tosnap = &g_dfaces[facenum_tosnap];
	const dplane_t*	p_tosnap = getPlaneFromFace (f_tosnap);
#ifdef HLRAD_SMOOTH_TEXNORMAL
	vec3_t			snapdir;
	if (!GetIntertexnormal (facenum, facenum_tosnap, snapdir))
	{
		facenum_tosnap = facenum;
		f_tosnap = f;
		p_tosnap = p;
	}
#endif

	vec3_t			surf_direct;
	dleaf_t*		leaf_direct;
	VectorCopy (surf_original, surf_direct);
	{
		vec_t dist;
		vec_t scale;
#ifdef HLRAD_SMOOTH_TEXNORMAL
		scale = DotProduct (snapdir, p_tosnap->normal);
#else
		scale = DotProduct (l->texnormal, p_tosnap->normal);
#endif
		dist = DotProduct (surf_direct, p_tosnap->normal) - DotProduct (face_delta, p_tosnap->normal) - p_tosnap->dist - DEFAULT_HUNT_OFFSET;
#ifdef HLRAD_SMOOTH_TEXNORMAL
		VectorMA (surf_direct, - dist / scale, snapdir, surf_direct);
#else
		VectorMA (surf_direct, - dist / scale, l->texnormal, surf_direct);
#endif
	}
	leaf_direct = HuntForWorld (surf_direct, face_delta, p_tosnap, huntsize, huntscale, DEFAULT_HUNT_OFFSET);

	if (LuxelFlag == LightOutside)
	{
		if (leaf_direct && point_in_winding_noedge (*wd, *p, surf_direct, width))
		{
			LuxelFlag = LightNormal;
			VectorCopy (surf_direct, point);
		}
	}

	if (LuxelFlag == LightOutside)
	{
		bool	blocked_direct;
		bool	blocked_inwinding;
		bool	blocked_inwinding_noedge;
		vec3_t	surf_inwinding;
		vec3_t	surf_inwinding_noedge;
		dleaf_t*leaf_inwinding;
		dleaf_t*leaf_inwinding_noedge;
#ifdef HLRAD_HULLU
		vec3_t transparency = { 1.0, 1.0, 1.0 };
#endif
#ifdef HLRAD_OPAQUE_STYLE
		int opaquestyle;
#endif
		{
			blocked_direct = (leaf_direct == NULL);
			if (!point_in_winding (*wd, *p, surf_original))
			{
				VectorCopy (surf_original, surf_inwinding);
				snap_to_winding (*wd, *p, surf_inwinding);
				leaf_inwinding = HuntForWorld (surf_inwinding, face_delta, p, huntsize, huntscale, DEFAULT_HUNT_OFFSET);
				if ( blocked_direct
					|| !leaf_inwinding
					|| TestLine (surf_direct, surf_inwinding) != CONTENTS_EMPTY
					|| TestSegmentAgainstOpaqueList (surf_direct, surf_inwinding
	#ifdef HLRAD_HULLU
						, transparency
	#endif
	#ifdef HLRAD_OPAQUE_STYLE
						, opaquestyle
	#endif
						) == true
	#ifdef HLRAD_OPAQUE_STYLE
					|| opaquestyle != -1
	#endif
	#ifdef HLRAD_TRANSLUCENT
					|| l->translucent_b
	#endif
					)
				{
					blocked_direct = true;
				}
			}
			else
			{
				VectorCopy (surf_original, surf_inwinding);
				leaf_inwinding = leaf_original;
			}
			blocked_inwinding = (leaf_inwinding == NULL);
			if (!point_in_winding_noedge (*wd, *p, surf_inwinding, width))
			{
				VectorCopy (surf_inwinding, surf_inwinding_noedge);
				snap_to_winding_noedge (*wd, *p, surf_inwinding_noedge, width);
				leaf_inwinding_noedge = HuntForWorld (surf_inwinding_noedge, face_delta, p, huntsize, huntscale, DEFAULT_HUNT_OFFSET);
				if ( blocked_inwinding
					|| !leaf_inwinding_noedge
					|| TestLine (surf_inwinding, surf_inwinding_noedge) != CONTENTS_EMPTY
					|| TestSegmentAgainstOpaqueList (surf_inwinding, surf_inwinding_noedge
	#ifdef HLRAD_HULLU
						, transparency
	#endif
	#ifdef HLRAD_OPAQUE_STYLE
						, opaquestyle
	#endif
						) == true
	#ifdef HLRAD_OPAQUE_STYLE
					|| opaquestyle != -1
	#endif
					)
				{
					blocked_inwinding = true;
				}
			}
			else
			{
				VectorCopy (surf_inwinding, surf_inwinding_noedge);
				leaf_inwinding_noedge = leaf_inwinding;
			}
			blocked_inwinding_noedge = (leaf_inwinding_noedge == NULL);
			if (blocked_inwinding_noedge == true)
			{
				blocked_inwinding = true;
			}
			if (blocked_inwinding == true)
			{
				blocked_direct = true;
			}
		}
		if (!blocked_direct)
		{
			LuxelFlag = LightNormal;
			VectorCopy (surf_direct, point);
		}
		else if (!blocked_inwinding)
		{
			LuxelFlag = LightPulledInside;
			VectorCopy (surf_inwinding, point);
		}
		else if (!blocked_inwinding_noedge)
		{
			LuxelFlag = LightPulledInside;
			VectorCopy (surf_inwinding_noedge, point);
		}
	}

	if (LuxelFlag == LightOutside)
	{
		// this part is very slow
		const int numnudges = 13;
		vec_t nudgelist[numnudges][2] = {{0,0},{0.6,0},{0,0.6},{-0.6,0},{0,-0.6},{1.1,1.1},{1.1,-1.1},{-1.1,1.1},{-1.1,-1.1},{1.6,0},{0,1.6},{-1.6,0},{0,-1.6}};
		vec_t nudgescale_s, nudgescale_t;
		nudgescale_s = original_s <= mids? TEXTURE_STEP: -TEXTURE_STEP;
		nudgescale_t = original_t <= midt? TEXTURE_STEP: -TEXTURE_STEP;
		int i;
		for (i = 0; i < numnudges; i++)
		{
			vec_t s1 = original_s + nudgelist[i][0] * nudgescale_s;
			vec_t t1 = original_t + nudgelist[i][1] * nudgescale_t;
			vec3_t surf;
			SetSurfFromST(l, surf, s1, t1);
			if (point_in_winding (*wd, *p, surf) && HuntForWorld (surf, face_delta, p, 2, 0.5, DEFAULT_HUNT_OFFSET) && point_in_winding (*wd, *p, surf))
			{
				LuxelFlag = LightSimpleNudge;
				VectorCopy (surf, point);
				break;
			}
		}
	}

	if (LuxelFlag == LightOutside)
	{
		VectorCopy (surf_original, point);
	}

	delete wd;
	return LuxelFlag;
}
static void		CalcPoints(lightinfo_t* l)
{
	const int       facenum = l->surfnum;
	const dface_t*  f = g_dfaces + facenum;
	const dplane_t* p = getPlaneFromFace	(f);
	const vec_t*    face_delta = g_face_offset[facenum];
	const eModelLightmodes lightmode = g_face_lightmode[facenum];
	const int       h = l->texsize[1] + 1;
	const int       w = l->texsize[0] + 1;
	const vec_t     starts = (l->texmins[0] * 16);
	const vec_t     startt = (l->texmins[1] * 16);
	light_flag_t    LuxelFlags[MAX_SINGLEMAP];
	light_flag_t*   pLuxelFlags;
	vec_t           us, ut;
	vec_t*          surf;
	int             s, t;
	l->numsurfpt = w * h;
	for (t = 0; t < h; t++)
	{
		for (s = 0; s < w; s++)
		{
			surf = l->surfpt[s+w*t];
			pLuxelFlags = &LuxelFlags[s+w*t];
			us = starts + s * TEXTURE_STEP;
			ut = startt + t * TEXTURE_STEP;
			*pLuxelFlags = SetSampleFromST (surf, l, us, ut, lightmode);
		}
	}
#if 1
    {
		int i, n;
		int s_other, t_other;
		light_flag_t* pLuxelFlags_other;
		vec_t* surf_other;
		bool adjusted;
		for (i = 0; i < h + w; i++)
		{
			adjusted = false;
			for (t = 0; t < h; t++)
			{
				for (s = 0; s < w; s++)
				{
					surf = l->surfpt[s+w*t];
					pLuxelFlags = &LuxelFlags[s+w*t];
					if (*pLuxelFlags != LightOutside)
						continue;
					for (n = 0; n < 4; n++)
					{
						switch (n)
						{
						case 0: s_other = s + 1; t_other = t; break;
						case 1: s_other = s - 1; t_other = t; break;
						case 2: s_other = s; t_other = t + 1; break;
						case 3: s_other = s; t_other = t - 1; break;
						}
						if (t_other < 0 || t_other >= h || s_other < 0 || s_other >= w)
							continue;
						surf_other = l->surfpt[s_other+w*t_other];
						pLuxelFlags_other = &LuxelFlags[s_other+w*t_other];
						if (*pLuxelFlags_other != LightOutside)
						{
							*pLuxelFlags = LightShiftedInside;
							VectorCopy (surf_other, surf);
							adjusted = true;
							break;
						}
					}
				}
			}
			if (!adjusted)
				break;
		}
	}
#endif
	for (int i = 0; i < MAX_SINGLEMAP; i++)
	{
		l->lightoutside[i] = (LuxelFlags[i] == LightOutside);
	}
}
#else
static void     CalcPoints(lightinfo_t* l)
{
    const int       facenum = l->surfnum;
    const dface_t*  f = g_dfaces + facenum;
    const dplane_t* p = getPlaneFromFace	(f);

    const vec_t*    face_delta = g_face_offset[facenum];
    const eModelLightmodes lightmode = g_face_lightmode[facenum];

#ifdef HLRAD_NUDGE_VL
	vec_t mids, midt;
	{
		// use winding center instead
		vec3_t surf;
		VectorSubtract (g_face_centroids[facenum], g_face_offset[facenum], surf);
		VectorSubtract (surf, l->texorg, surf);
		mids = DotProduct (surf, l->worldtotex[0]);
		midt = DotProduct (surf, l->worldtotex[1]);
	}
#else
    const vec_t     mids = (l->exactmaxs[0] + l->exactmins[0]) / 2;
    const vec_t     midt = (l->exactmaxs[1] + l->exactmins[1]) / 2;
#endif

    const int       h = l->texsize[1] + 1;
    const int       w = l->texsize[0] + 1;

    const vec_t     starts = (l->texmins[0] * 16);
    const vec_t     startt = (l->texmins[1] * 16);

    light_flag_t    LuxelFlags[MAX_SINGLEMAP];
    light_flag_t*   pLuxelFlags;
    vec_t           us, ut;
    vec_t*          surf;
    vec3_t          surface_midpoint;
    dleaf_t*        leaf_mid;
    dleaf_t*        leaf_surf;
    int             s, t;
    int             i;

    l->numsurfpt = w * h;

    memset(LuxelFlags, 0, sizeof(LuxelFlags));

    leaf_mid = FindSurfaceMidpoint(l, surface_midpoint);
#if 0
    if (!leaf_mid)
    {
        Developer(DEVELOPER_LEVEL_FLUFF, "CalcPoints [face %d] (%4.3f %4.3f %4.3f) midpoint outside world\n",
                  facenum, surface_midpoint[0], surface_midpoint[1], surface_midpoint[2]);
    }
    else
    {
        Developer(DEVELOPER_LEVEL_FLUFF, "FindSurfaceMidpoint [face %d] @ (%4.3f %4.3f %4.3f)\n",
                  facenum, surface_midpoint[0], surface_midpoint[1], surface_midpoint[2]);
    }
#endif

    // First pass, light normally, and pull any faces toward the center for bleed adjustment

    surf = l->surfpt[0];
    pLuxelFlags = LuxelFlags;
    for (t = 0; t < h; t++)
    {
        for (s = 0; s < w; s++, surf += 3, pLuxelFlags++)
        {
            vec_t           original_s = us = starts + s * TEXTURE_STEP;
            vec_t           original_t = ut = startt + t * TEXTURE_STEP;

            SetSurfFromST(l, surf, us, ut);
            leaf_surf = HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);

            if (!leaf_surf)
            {
                // At first try a 1/3 and 2/3 distance to nearest in each S and T axis towards the face midpoint
                if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP * (1.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP * (1.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP * (2.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP * (2.0 / 3.0)))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
#ifdef HLRAD_NUDGE_VL
                else if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
                else if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP))
                {
                    *pLuxelFlags = LightSimpleNudge;
                }
#else
                // Next, if this is a model flagged with the 'Embedded' mode, try away from the facemid too
                else if (lightmode & eModelLightmodeEmbedded)
                {
                    SetSurfFromST(l, surf, us, ut);
                    if (SimpleNudge(surf, l, &us, &ut, TEXTURE_STEP))
                    {
                        *pLuxelFlags = LightSimpleNudgeEmbedded;
                        continue;
                    }
                    if (SimpleNudge(surf, l, &us, &ut, -TEXTURE_STEP))
                    {
                        *pLuxelFlags = LightSimpleNudgeEmbedded;
                        continue;
                    }

                    SetSurfFromST(l, surf, original_s, original_t);
                    *pLuxelFlags = LightOutside;
                    continue;
                }
#endif
            }

#ifndef HLRAD_NUDGE_VL
            if (!(lightmode & eModelLightmodeEmbedded))
#endif
            {
#ifdef HLRAD_NUDGE_VL
				// HLRAD_NUDGE_VL: only pull when light is blocked AND point is outside face.
				vec3_t			surf_nopull;
				vec_t			us_nopull = us, ut_nopull = ut;
				Winding			*wd = new Winding (*f);
				int				j;
				for (j = 0; j < wd->m_NumPoints; j++)
				{
					VectorAdd (wd->m_Points[j], face_delta, wd->m_Points[j]);
				}
#endif
#ifdef HLRAD_SNAPTOWINDING
				bool nudge_succeeded = false;
				SetSurfFromST(l, surf, us, ut);
				leaf_surf = HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);
				if (leaf_surf && point_in_winding_noedge (*wd, *p, surf, 1.0))
				{
					*pLuxelFlags = LightNormal;
					nudge_succeeded = true;
				}
				else
				{
					SetSurfFromST(l, surf, us, ut);
					snap_to_winding (*wd, *p, surf);
					if (lightmode & eModelLightmodeConcave)
					{
						VectorScale (surf, 0.99, surf);
						VectorMA (surf, 0.01, g_face_centroids[facenum], surf);
					}
					leaf_surf = HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET);
					if (leaf_surf)
					{
						*pLuxelFlags = LightPulledInside;
						nudge_succeeded = true;
					}
				}

#else
				// Pull the sample points towards the facemid if visibility is blocked
				// and the facemid is inside the world
	#ifdef HLRAD_NUDGE_SMALLSTEP
				int             nudge_divisor = 4 * qmax(qmax(w, h), 4);
	#else
				int             nudge_divisor = qmax(qmax(w, h), 4);
	#endif
				int             max_nudge = nudge_divisor + 1;
				bool            nudge_succeeded = false;

				vec_t           nudge_s = (mids - us) / (vec_t)nudge_divisor;
				vec_t           nudge_t = (midt - ut) / (vec_t)nudge_divisor;

				// if a line can be traced from surf to facemid, the point is good
				for (i = 0; i < max_nudge; i++)
				{
					// Make sure we are "in the world"(Not the zero leaf)
	#ifndef HLRAD_NUDGE_VL
					if (leaf_mid)
					{
	#endif
						SetSurfFromST(l, surf, us, ut);
						leaf_surf =
							HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE,
										 DEFAULT_HUNT_OFFSET);

						if (leaf_surf)
						{
	#ifdef HLRAD_NUDGE_VL
							if (point_in_winding_noedge (*wd, *p, surf, 1.0))
							{
	#else
							if (TestLine(surface_midpoint, surf) == CONTENTS_EMPTY)
							{
								if (lightmode & eModelLightmodeConcave)
								{
		#ifdef HLRAD_HULLU
									vec3_t transparency = { 1.0, 1.0, 1.0 };
		#endif
		#ifdef HLRAD_OPAQUE_STYLE
									int opaquestyle;
		#endif
									if (TestSegmentAgainstOpaqueList(surface_midpoint, surf
		#ifdef HLRAD_HULLU
										, transparency
		#endif
		#ifdef HLRAD_OPAQUE_STYLE
										, opaquestyle
		#endif
										)
		#ifdef HLRAD_OPAQUE_STYLE
										|| opaquestyle != -1
		#endif
										)
									{
										Log("SDF::4\n");
										us += nudge_s;
										ut += nudge_t;
										continue;   // Try nudge again, we hit an opaque face
									}
								}
	#endif
								if (i)
								{
									*pLuxelFlags = LightPulledInside;
								}
								else
								{
									*pLuxelFlags = LightNormal;
								}
								nudge_succeeded = true;
								break;
							}
						}
	#ifndef HLRAD_NUDGE_VL
					}
					else
					{
						leaf_surf = PointInLeaf(surf);
						if (leaf_surf != g_dleafs)
						{
							if ((leaf_surf->contents != CONTENTS_SKY) && (leaf_surf->contents != CONTENTS_SOLID))
							{
								*pLuxelFlags = LightNormal;
								nudge_succeeded = true;
								break;
							}
						}
					}
	#endif

					us += nudge_s;
					ut += nudge_t;
				}
#endif /*HLRAD_SNAPTOWINDING*/

                if (!nudge_succeeded)
                {
                    SetSurfFromST(l, surf, original_s, original_t);
                    *pLuxelFlags = LightOutside;
                }
#ifdef HLRAD_NUDGE_VL
				delete wd;
				if (*pLuxelFlags == LightPulledInside)
				{
					SetSurfFromST(l, surf_nopull, us_nopull, ut_nopull);
					leaf_surf =
						HuntForWorld(surf_nopull, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE,
							DEFAULT_HUNT_OFFSET);
					if (leaf_surf)
					{
						if (TestLine(surf, surf_nopull) == CONTENTS_EMPTY)
						{
#ifdef HLRAD_HULLU
							vec3_t transparency = { 1.0, 1.0, 1.0 };
#endif
#ifdef HLRAD_OPAQUE_STYLE
							int opaquestyle;
#endif
							if (!TestSegmentAgainstOpaqueList(surf, surf_nopull
#ifdef HLRAD_HULLU
								, transparency
#endif
#ifdef HLRAD_OPAQUE_STYLE
								, opaquestyle
#endif
								)
#ifdef HLRAD_OPAQUE_STYLE
								&& opaquestyle == -1
#endif
								)
							{
								*pLuxelFlags = LightNormal;
								VectorCopy (surf_nopull, surf);
							}
						}
					}
				}
#endif
            }
        }
    }

    // 2nd Pass, find units that are not lit and try to move them one half or unit worth 
    // in each direction and see if that is lit.
    // This handles 1 x N lightmaps which are all dark everywhere and have no frame of refernece
    // for a good center or directly lit areas
    surf = l->surfpt[0];
    pLuxelFlags = LuxelFlags;
#if 0
    Developer(DEVELOPER_LEVEL_SPAM,
              "w (%d) h (%d) dim (%d) leafmid (%4.3f %4.3f %4.3f) plane normal (%4.3f) (%4.3f) (%4.3f) dist (%f)\n", w,
              h, w * h, surface_midpoint[0], surface_midpoint[1], surface_midpoint[2], p->normal[0], p->normal[1],
              p->normal[2], p->dist);
#endif
    {
        int             total_dark = 0;
        int             total_adjusted = 0;

        for (t = 0; t < h; t++)
        {
            for (s = 0; s < w; s++, surf += 3, pLuxelFlags++)
            {
                if (!*pLuxelFlags)
                {
#if 0
                    Developer(DEVELOPER_LEVEL_FLUFF, "Dark (%d %d) (%4.3f %4.3f %4.3f)\n",
                              s, t, surf[0], surf[1], surf[2]);
#endif
                    total_dark++;
                    if (HuntForWorld(surf, face_delta, p, DEFAULT_HUNT_SIZE, DEFAULT_HUNT_SCALE, DEFAULT_HUNT_OFFSET))
                    {
#if 0
                        Developer(DEVELOPER_LEVEL_FLUFF, "Shifted %d %d to (%4.3f %4.3f %4.3f)\n", s, t, surf[0],
                                  surf[1], surf[2]);
#endif
                        *pLuxelFlags = LightShifted;
                        total_adjusted++;
                    }
                    else if (HuntForWorld(surf, face_delta, p, 101, 0.5, DEFAULT_HUNT_OFFSET))
                    {
#if 0
                        Developer(DEVELOPER_LEVEL_FLUFF, "Shifted %d %d to (%4.3f %4.3f %4.3f)\n", s, t, surf[0],
                                  surf[1], surf[2]);
#endif
                        *pLuxelFlags = LightShifted;
                        total_adjusted++;
                    }
                }
            }
        }
#if 0
        if (total_dark)
        {
            Developer(DEVELOPER_LEVEL_FLUFF, "Pass 2 : %d dark, %d corrected\n", total_dark, total_adjusted);
        }
#endif
    }

    // 3rd Pass, find units that are not lit and move them towards neighbhors who are
    // Currently finds the first lit neighbhor and uses its data
    surf = l->surfpt[0];
    pLuxelFlags = LuxelFlags;
    {
        int             total_dark = 0;
        int             total_adjusted = 0;

        for (t = 0; t < h; t++)
        {
            for (s = 0; s < w; s++, surf += 3, pLuxelFlags++)
            {
                if (!*pLuxelFlags)
                {
                    int             x_min = qmax(0, s - 1);
                    int             x_max = qmin(w, s + 1);
                    int             y_min = qmax(0, t - 1);
                    int             y_max = qmin(t, t + 1);

                    int             x, y;

#if 0
                    Developer(DEVELOPER_LEVEL_FLUFF, "Point outside (%d %d) (%4.3f %4.3f %4.3f)\n",
                              s, t, surf[0], surf[1], surf[2]);
#endif

                    total_dark++;

                    for (x = x_min; x < x_max; x++)
                    {
                        for (y = y_min; y < y_max; y++)
                        {
                            if (*pLuxelFlags >= LightNormal)
                            {
                                dleaf_t*        leaf;
                                vec_t*          other_surf = l->surfpt[0];

                                other_surf += ((y * w) + x) * 3;

                                leaf = PointInLeaf(other_surf);
                                if ((leaf->contents != CONTENTS_SKY && leaf->contents != CONTENTS_SOLID))
                                {
                                    *pLuxelFlags = LightShiftedInside;
#if 0
                                    Developer(DEVELOPER_LEVEL_MESSAGE,
                                              "Nudged (%d %d) (%4.3f %4.3f %4.3f) to (%d %d) (%4.3f %4.3f %4.3f) \n",
                                              s, t, surf[0], surf[1], surf[2], x, y, other_surf[0], other_surf[1],
                                              other_surf[2]);
#endif
                                    VectorCopy(other_surf, surf);
                                    total_adjusted++;
                                    goto found_it;
                                }
                            }
                        }
                    }
                }
              found_it:;
            }
        }
#if 0
        if (total_dark)
        {
            Developer(DEVELOPER_LEVEL_FLUFF, "Pass 2 : %d dark, %d corrected\n", total_dark, total_adjusted);
        }
#endif
    }
}
#endif

//==============================================================

typedef struct
{
    vec3_t          pos;
    vec3_t          light;
#ifdef ZHLT_XASH
	// this increases the maximum (at 100% AllocBlock, 4 light styles) possible usage of memory of all light samples from 100MB to 200MB
	vec3_t			light_direction; // sum of light direction * light contribution (rgb averaged)
	vec3_t			normal; // phong normal
#endif
}
sample_t;

typedef struct
{
    int             numsamples;
    sample_t*       samples[MAXLIGHTMAPS];
}
facelight_t;

static directlight_t* directlights[MAX_MAP_LEAFS];
static facelight_t facelight[MAX_MAP_FACES];
static int      numdlights;

#ifndef HLRAD_REFLECTIVITY
#define	DIRECT_SCALE	0.1f
#endif
extern bool g_nightmode;
extern bool g_daylightreturnmode;
// =====================================================================================
//  CreateDirectLights
// =====================================================================================
void            CreateDirectLights()
{
    unsigned        i;
    patch_t*        p;
    directlight_t*  dl;
    dleaf_t*        leaf;
    int             leafnum;
    entity_t*       e;
    entity_t*       e2;
    const char*     name;
    const char*     target;
    float           angle;
    vec3_t          dest;

#ifndef HLRAD_CUSTOMTEXLIGHT
    // AJM: coplaner lighting
    vec3_t          temp_normal;
#endif

    numdlights = 0;
#ifdef HLRAD_STYLEREPORT
	int styleused[ALLSTYLES];
	memset (styleused, 0, ALLSTYLES * sizeof(styleused[0]));
	styleused[0] = true;
	int numstyles = 1;
#endif

    //
    // surfaces
    //
    for (i = 0, p = g_patches; i < g_num_patches; i++, p++)
    {
#ifdef ZHLT_TEXLIGHT
#ifdef HLRAD_STYLEREPORT
		if (p->emitstyle >= 0 && p->emitstyle < ALLSTYLES)
		{
			if (styleused[p->emitstyle] == false)
			{
				styleused[p->emitstyle] = true;
				numstyles++;
			}
		}
#endif
        if (
	#ifdef HLRAD_REFLECTIVITY
			DotProduct (p->baselight, p->texturereflectivity) / 3
	#else
			VectorAvg(p->baselight)
	#endif
	#ifdef HLRAD_TEXLIGHTTHRESHOLD_FIX
			> 0.0
	#else
			>= g_dlight_threshold
	#endif
	#ifdef HLRAD_CUSTOMTEXLIGHT
			&& !(g_face_texlights[p->faceNumber]
				&& *ValueForKey (g_face_texlights[p->faceNumber], "_scale")
				&& FloatForKey (g_face_texlights[p->faceNumber], "_scale") <= 0)
	#endif
			) //LRC
#else
        if (
	#ifdef HLRAD_REFLECTIVITY
			DotProduct (p->totallight, p->texturereflectivity) / 3
	#else
			VectorAvg(p->totallight)
	#endif
			>= g_dlight_threshold
			)
#endif
        {
            numdlights++;
            dl = (directlight_t*)calloc(1, sizeof(directlight_t));
#ifdef HLRAD_HLASSUMENOMEMORY
			hlassume (dl != NULL, assume_NoMemory);
#endif

            VectorCopy(p->origin, dl->origin);

            leaf = PointInLeaf(dl->origin);
            leafnum = leaf - g_dleafs;

			if(g_face_texlights[p->faceNumber])
				dl->pentity = g_face_texlights[p->faceNumber];
			else
				dl->pentity = NULL;

            dl->next = directlights[leafnum];
            directlights[leafnum] = dl;
#ifdef ZHLT_TEXLIGHT
            dl->style = p->emitstyle; //LRC
#endif
#ifdef HLRAD_GatherPatchLight
			dl->topatch = false;
	#ifdef HLRAD_TEXLIGHTTHRESHOLD_FIX
			if (!p->emitmode)
			{
				dl->topatch = true;
			}
	#endif
#ifdef HLRAD_FASTMODE
			if (g_fastmode)
			{
				dl->topatch = true;
			}
#endif
#endif
#ifdef HLRAD_TEXLIGHT_SPOTS_FIX
			dl->patch_area = p->area;
	#ifdef HLRAD_ACCURATEBOUNCE_TEXLIGHT
			dl->patch_emitter_range = p->emitter_range;
			dl->patch = p;
	#endif
#endif
#ifdef HLRAD_CUSTOMTEXLIGHT
			dl->stopdot = 0.0;
			dl->stopdot2 = 0.0;
			if (g_face_texlights[p->faceNumber])
			{
				if (*ValueForKey (g_face_texlights[p->faceNumber], "_cone"))
				{
					dl->stopdot = FloatForKey (g_face_texlights[p->faceNumber], "_cone");
					dl->stopdot = dl->stopdot >= 90? 0: (float)cos (dl->stopdot / 180 * Q_PI);
				}
				if (*ValueForKey (g_face_texlights[p->faceNumber], "_cone2"))
				{
					dl->stopdot2 = FloatForKey (g_face_texlights[p->faceNumber], "_cone2");
					dl->stopdot2 = dl->stopdot2 >= 90? 0: (float)cos (dl->stopdot2 / 180 * Q_PI);
				}
				if (dl->stopdot2 > dl->stopdot)
					dl->stopdot2 = dl->stopdot;
			}
#endif

            dl->type = emit_surface;
            VectorCopy(getPlaneFromFaceNumber(p->faceNumber)->normal, dl->normal);
#ifdef ZHLT_TEXLIGHT
            VectorCopy(p->baselight, dl->intensity); //LRC
#else
            VectorCopy(p->totallight, dl->intensity);
#endif
#ifdef HLRAD_CUSTOMTEXLIGHT
			if (g_face_texlights[p->faceNumber])
			{
				if (*ValueForKey (g_face_texlights[p->faceNumber], "_scale"))
				{
					vec_t scale = FloatForKey (g_face_texlights[p->faceNumber], "_scale");
					VectorScale (dl->intensity, scale, dl->intensity);
				}
			}
#endif
            VectorScale(dl->intensity, p->area, dl->intensity);
#ifdef HLRAD_ACCURATEBOUNCE_REDUCEAREA
			VectorScale (dl->intensity, p->exposure, dl->intensity);
#endif
#ifdef HLRAD_REFLECTIVITY
			VectorScale (dl->intensity, 1.0 / Q_PI, dl->intensity);
			VectorMultiply (dl->intensity, p->texturereflectivity, dl->intensity);
#else
            VectorScale(dl->intensity, DIRECT_SCALE, dl->intensity);
#endif
        
#ifdef HLRAD_WATERBACKFACE_FIX
			dface_t *f = &g_dfaces[p->faceNumber];
			if (g_face_entity[p->faceNumber] - g_entities != 0 && !strncasecmp (GetTextureByNumber (f->texinfo), "!", 1))
			{
				directlight_t *dl2;
				numdlights++;
				dl2 = (directlight_t *)calloc (1, sizeof (directlight_t));
				hlassume (dl2 != NULL, assume_NoMemory);
				*dl2 = *dl;
				VectorMA (dl->origin, -2, dl->normal, dl2->origin);
				VectorSubtract (vec3_origin, dl->normal, dl2->normal);
				leaf = PointInLeaf (dl2->origin);
				leafnum = leaf - g_dleafs;
				dl2->next = directlights[leafnum];
				directlights[leafnum] = dl2;
			}
#endif
#ifndef HLRAD_CUSTOMTEXLIGHT // no softlight hack
        	// --------------------------------------------------------------
	        // Changes by Adam Foster - afoster@compsoc.man.ac.uk
	        // mazemaster's l33t backwards lighting (I still haven't a clue
	        // what it's supposed to be for) :-)
#ifdef HLRAD_WHOME

	        if (g_softlight_hack[0] || g_softlight_hack[1] || g_softlight_hack[2]) 
            {
		        numdlights++;
		        dl = (directlight_t *) calloc(1, sizeof(directlight_t));
#ifdef HLRAD_HLASSUMENOMEMORY
				hlassume (dl != NULL, assume_NoMemory);
#endif

		        VectorCopy(p->origin, dl->origin);

		        leaf = PointInLeaf(dl->origin);
		        leafnum = leaf - g_dleafs;

		        dl->next = directlights[leafnum];
		        directlights[leafnum] = dl;

#ifdef HLRAD_GatherPatchLight
				dl->topatch = false;
	#ifdef HLRAD_TEXLIGHTTHRESHOLD_FIX
				if (!p->emitmode)
				{
					dl->topatch = true;
				}
	#endif
#ifdef HLRAD_FASTMODE
				if (g_fastmode)
				{
					dl->topatch = true;
				}
#endif
#endif
#ifdef HLRAD_TEXLIGHT_SPOTS_FIX
				dl->patch_area = p->area;
	#ifdef HLRAD_ACCURATEBOUNCE_TEXLIGHT
				dl->patch_emitter_range = p->emitter_range;
				dl->patch = p;
	#endif
#endif
		        dl->type = emit_surface;
		        VectorCopy(getPlaneFromFaceNumber(p->faceNumber)->normal, dl->normal);
		        VectorScale(dl->normal, g_softlight_hack_distance, temp_normal);
		        VectorAdd(dl->origin, temp_normal, dl->origin);
		        VectorScale(dl->normal, -1, dl->normal);

#ifdef ZHLT_TEXLIGHT
                VectorCopy(p->baselight, dl->intensity); //LRC
#else
		        VectorCopy(p->totallight, dl->intensity);
#endif
		        VectorScale(dl->intensity, p->area, dl->intensity);
#ifdef HLRAD_ACCURATEBOUNCE_REDUCEAREA
				VectorScale (dl->intensity, p->exposure, dl->intensity);
#endif
#ifdef HLRAD_REFLECTIVITY
				VectorScale (dl->intensity, 1.0 / Q_PI, dl->intensity);
				VectorMultiply (dl->intensity, p->texturereflectivity, dl->intensity);
#else
		        VectorScale(dl->intensity, DIRECT_SCALE, dl->intensity);
#endif

		        dl->intensity[0] *= g_softlight_hack[0];
		        dl->intensity[1] *= g_softlight_hack[1];
		        dl->intensity[2] *= g_softlight_hack[2];
	        }

#endif
	        // --------------------------------------------------------------
#endif
        }

#ifdef ZHLT_TEXLIGHT
        //LRC        VectorClear(p->totallight[0]);
#else
        VectorClear(p->totallight);
#endif
    }

    //
    // entities
    //
    for (i = 0; i < (unsigned)g_numentities; i++)
    {
        const char*     pLight;
        double          r, g, b, scaler;
        float           l1;
        int             argCnt;

        e = &g_entities[i];
        name = ValueForKey(e, "classname");

		if(g_nightmode)
		{
			if (strncmp(name, "light", 5) && strncmp(name, "night_light", 11))
				continue;

			if (!strcmp(name, "light_environment"))
			{
				pLight = ValueForKey(e, "_light");
				if(pLight)
				{
					int ir, ig, ib;
					argCnt = sscanf(pLight, "%d %d %d", &ir, &ig, &ib);
					Log("Discarded light_environment with color %d %d %d due to -nightmode.\n", ir, ig, ib);
				}

				continue;
			}
		}
		else if(g_daylightreturnmode)
		{
			if (strncmp(name, "light", 5))
				continue;

			if (!strcmp(name, "light_environment"))
			{
				int value = IntForKey(e, "daylightreturn");
				if(value != 1)
				{
					pLight = ValueForKey(e, "_light");
					if(pLight)
					{
						int ir, ig, ib;
						argCnt = sscanf(pLight, "%d %d %d", &ir, &ig, &ib);
						Log("Discarded light_environment with color %d %d %d due to -daylightreturnmode.\n", ir, ig, ib);
					}

					continue;
				}
			}
		}
		else
		{
			if (strncmp(name, "light", 5))
				continue;

			if (!strcmp(name, "light_environment") && IntForKey(e, "daylightreturn") == 1)
			{
				pLight = ValueForKey(e, "_light");
				if(pLight)
				{
					int ir, ig, ib;
					argCnt = sscanf(pLight, "%d %d %d", &ir, &ig, &ib);
					Log("Discarded light_environment with color %d %d %d due to being set to only work in -daylightreturnmode.\n", ir, ig, ib);
				}

				continue;
			}
		}

		int style = IntForKey (e, "style");
#ifdef ZHLT_TEXLIGHT
		if (style < 0)
		{
			style = -style;
		}
#endif
		if (g_bumpmaps && style)
		{
			const char* pstrname = ValueForKey(e, "targetname");
			if(pstrname)
				Log("Warning: light %s with style %d is deleted\n", pstrname, style);

			continue;
		}

#ifdef HLRAD_CUSTOMTEXLIGHT
		if (!strcmp (name, "light_surface"))
		{
			continue;
		}
#endif

        numdlights++;
        dl = (directlight_t*)calloc(1, sizeof(directlight_t));
#ifdef HLRAD_HLASSUMENOMEMORY
		hlassume (dl != NULL, assume_NoMemory);
#endif

        GetVectorForKey(e, "origin", dl->origin);

        leaf = PointInLeaf(dl->origin);
        leafnum = leaf - g_dleafs;

        dl->next = directlights[leafnum];
        directlights[leafnum] = dl;

        dl->style = IntForKey(e, "style");
#ifdef ZHLT_TEXLIGHT
        if (dl->style < 0) 
            dl->style = -dl->style; //LRC
#endif

#ifdef HLRAD_STYLEREPORT
		if (dl->style >= 0 && dl->style < ALLSTYLES)
		{
			if (styleused[dl->style] == false)
			{
				styleused[dl->style] = true;
				numstyles++;
			}
		}
#endif
#ifdef HLRAD_GatherPatchLight
		dl->topatch = false;
		if (IntForKey (e, "_fast") == 1)
		{
			dl->topatch = true;
		}
#ifdef HLRAD_FASTMODE
		if (g_fastmode)
		{
			dl->topatch = true;
		}
#endif
#endif
        pLight = ValueForKey(e, "_light");
        // scanf into doubles, then assign, so it is vec_t size independent
        r = g = b = scaler = 0;
        argCnt = sscanf(pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler);
        dl->intensity[0] = (float)r;
        if (argCnt == 1)
        {
            // The R,G,B values are all equal.
            dl->intensity[1] = dl->intensity[2] = (float)r;
        }
        else if (argCnt == 3 || argCnt == 4)
        {
            // Save the other two G,B values.
            dl->intensity[1] = (float)g;
            dl->intensity[2] = (float)b;

            // Did we also get an "intensity" scaler value too?
            if (argCnt == 4)
            {
                // Scale the normalized 0-255 R,G,B values by the intensity scaler
                dl->intensity[0] = dl->intensity[0] / 255 * (float)scaler;
                dl->intensity[1] = dl->intensity[1] / 255 * (float)scaler;
                dl->intensity[2] = dl->intensity[2] / 255 * (float)scaler;
            }
        }
        else
        {
            Log("light at (%f,%f,%f) has bad or missing '_light' value : '%s'\n",
                dl->origin[0], dl->origin[1], dl->origin[2], pLight);
            continue;
        }

        dl->fade = FloatForKey(e, "_fade");
        if (dl->fade == 0.0)
        {
            dl->fade = g_fade;
        }

#ifndef HLRAD_ARG_MISC
        dl->falloff = IntForKey(e, "_falloff");
        if (dl->falloff == 0)
        {
            dl->falloff = g_falloff;
        }
#endif

        target = ValueForKey(e, "target");

        if (!strcmp(name, "light_spot") || !strcmp(name, "night_light_spot") || !strcmp(name, "light_environment") || target[0])
        {
            if (!VectorAvg(dl->intensity))
            {
#ifndef HLRAD_ALLOWZEROBRIGHTNESS
                VectorFill(dl->intensity, 500);
#endif
            }
            dl->type = emit_spotlight;
            dl->stopdot = FloatForKey(e, "_cone");
            if (!dl->stopdot)
            {
                dl->stopdot = 10;
            }
            dl->stopdot2 = FloatForKey(e, "_cone2");
            if (!dl->stopdot2)
            {
                dl->stopdot2 = dl->stopdot;
            }
            if (dl->stopdot2 < dl->stopdot)
            {
                dl->stopdot2 = dl->stopdot;
            }
            dl->stopdot2 = (float)cos(dl->stopdot2 / 180 * Q_PI);
            dl->stopdot = (float)cos(dl->stopdot / 180 * Q_PI);

            if (!FindTargetEntity(target)) //--vluzacn
            {
                Warning("light at (%i %i %i) has missing target",
                        (int)dl->origin[0], (int)dl->origin[1], (int)dl->origin[2]);
				target = "";
            }
            if (target[0])
            {                                              // point towards target
                e2 = FindTargetEntity(target);
                if (!e2)
                {
                    Warning("light at (%i %i %i) has missing target",
                            (int)dl->origin[0], (int)dl->origin[1], (int)dl->origin[2]);
                }
                else
                {
                    GetVectorForKey(e2, "origin", dest);
                    VectorSubtract(dest, dl->origin, dl->normal);
                    VectorNormalize(dl->normal);
                }
            }
            else
            {                                              // point down angle
                vec3_t          vAngles;

                GetVectorForKey(e, "angles", vAngles);

                angle = (float)FloatForKey(e, "angle");
                if (angle == ANGLE_UP)
                {
                    dl->normal[0] = dl->normal[1] = 0;
                    dl->normal[2] = 1;
                }
                else if (angle == ANGLE_DOWN)
                {
                    dl->normal[0] = dl->normal[1] = 0;
                    dl->normal[2] = -1;
                }
                else
                {
                    // if we don't have a specific "angle" use the "angles" YAW
                    if (!angle)
                    {
                        angle = vAngles[1];
                    }

                    dl->normal[2] = 0;
                    dl->normal[0] = (float)cos(angle / 180 * Q_PI);
                    dl->normal[1] = (float)sin(angle / 180 * Q_PI);
                }

                angle = FloatForKey(e, "pitch");
                if (!angle)
                {
                    // if we don't have a specific "pitch" use the "angles" PITCH
                    angle = vAngles[0];
                }

                dl->normal[2] = (float)sin(angle / 180 * Q_PI);
                dl->normal[0] *= (float)cos(angle / 180 * Q_PI);
                dl->normal[1] *= (float)cos(angle / 180 * Q_PI);
            }

            if (FloatForKey(e, "_sky") || !strcmp(name, "light_environment"))
            {
				// -----------------------------------------------------------------------------------
				// Changes by Adam Foster - afoster@compsoc.man.ac.uk
				// diffuse lighting hack - most of the following code nicked from earlier
				// need to get diffuse intensity from new _diffuse_light key
				//
				// What does _sky do for spotlights, anyway?
				// -----------------------------------------------------------------------------------
#ifdef HLRAD_WHOME
				pLight = ValueForKey(e, "_diffuse_light");
        		r = g = b = scaler = 0;
        		argCnt = sscanf(pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler);
        		dl->diffuse_intensity[0] = (float)r;
        		if (argCnt == 1)
        		{
            		// The R,G,B values are all equal.
            		dl->diffuse_intensity[1] = dl->diffuse_intensity[2] = (float)r;
        		}
        		else if (argCnt == 3 || argCnt == 4)
        		{
            		// Save the other two G,B values.
            		dl->diffuse_intensity[1] = (float)g;
            		dl->diffuse_intensity[2] = (float)b;

            		// Did we also get an "intensity" scaler value too?
      		    	if (argCnt == 4)
     	       		{
                		// Scale the normalized 0-255 R,G,B values by the intensity scaler
                		dl->diffuse_intensity[0] = dl->diffuse_intensity[0] / 255 * (float)scaler;
                		dl->diffuse_intensity[1] = dl->diffuse_intensity[1] / 255 * (float)scaler;
                		dl->diffuse_intensity[2] = dl->diffuse_intensity[2] / 255 * (float)scaler;
            		}
        		}
				else
        		{
					// backwards compatibility with maps without _diffuse_light

					dl->diffuse_intensity[0] = dl->intensity[0];
					dl->diffuse_intensity[1] = dl->intensity[1];
					dl->diffuse_intensity[2] = dl->intensity[2];
        		}
#endif
				// -----------------------------------------------------------------------------------
#ifdef HLRAD_SUNDIFFUSE
				pLight = ValueForKey(e, "_diffuse_light2");
        		r = g = b = scaler = 0;
        		argCnt = sscanf(pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler);
        		dl->diffuse_intensity2[0] = (float)r;
        		if (argCnt == 1)
        		{
            		// The R,G,B values are all equal.
            		dl->diffuse_intensity2[1] = dl->diffuse_intensity2[2] = (float)r;
        		}
        		else if (argCnt == 3 || argCnt == 4)
        		{
            		// Save the other two G,B values.
            		dl->diffuse_intensity2[1] = (float)g;
            		dl->diffuse_intensity2[2] = (float)b;

            		// Did we also get an "intensity" scaler value too?
      		    	if (argCnt == 4)
     	       		{
                		// Scale the normalized 0-255 R,G,B values by the intensity scaler
                		dl->diffuse_intensity2[0] = dl->diffuse_intensity2[0] / 255 * (float)scaler;
                		dl->diffuse_intensity2[1] = dl->diffuse_intensity2[1] / 255 * (float)scaler;
                		dl->diffuse_intensity2[2] = dl->diffuse_intensity2[2] / 255 * (float)scaler;
            		}
        		}
				else
        		{
					dl->diffuse_intensity2[0] = dl->diffuse_intensity[0];
					dl->diffuse_intensity2[1] = dl->diffuse_intensity[1];
					dl->diffuse_intensity2[2] = dl->diffuse_intensity[2];
        		}
#endif

                dl->type = emit_skylight;
                dl->stopdot2 = FloatForKey(e, "_sky");     // hack stopdot2 to a sky key number
#ifdef HLRAD_SUNSPREAD
				dl->sunspreadangle = FloatForKey (e, "_spread");
				if (!g_allow_spread)
				{
					dl->sunspreadangle = 0;
				}
				if (dl->sunspreadangle < 0.0 || dl->sunspreadangle > 180)
				{
					Error ("Invalid spread angle '%s'. Please use a number between 0 and 180.\n", ValueForKey (e, "_spread"));
				}
				if (dl->sunspreadangle > 0.0)
				{
					int i;
					vec_t testangle = dl->sunspreadangle;
					if (dl->sunspreadangle < SUNSPREAD_THRESHOLD)
					{
						testangle = SUNSPREAD_THRESHOLD; // We will later centralize all the normals we have collected.
					}
					{
						vec_t totalweight = 0;
						int count;
						vec_t testdot = cos (testangle * (Q_PI / 180.0));
						for (count = 0, i = 0; i < g_numskynormals[SUNSPREAD_SKYLEVEL]; i++)
						{
							vec3_t &testnormal = g_skynormals[SUNSPREAD_SKYLEVEL][i];
							vec_t dot = DotProduct (dl->normal, testnormal);
							if (dot >= testdot - NORMAL_EPSILON)
							{
								totalweight += qmax (0, dot - testdot) * g_skynormalsizes[SUNSPREAD_SKYLEVEL][i]; // This is not the right formula when dl->sunspreadangle < SUNSPREAD_THRESHOLD, but it gives almost the same result as the right one.
								count++;
							}
						}
						if (count <= 10 || totalweight <= NORMAL_EPSILON)
						{
							Error ("collect spread normals: internal error: can not collect enough normals.");
						}
						dl->numsunnormals = count;
						dl->sunnormals = (vec3_t *)malloc (count * sizeof (vec3_t));
						dl->sunnormalweights = (vec_t *)malloc (count * sizeof (vec_t));
						hlassume (dl->sunnormals != NULL, assume_NoMemory);
						hlassume (dl->sunnormalweights != NULL, assume_NoMemory);
						for (count = 0, i = 0; i < g_numskynormals[SUNSPREAD_SKYLEVEL]; i++)
						{
							vec3_t &testnormal = g_skynormals[SUNSPREAD_SKYLEVEL][i];
							vec_t dot = DotProduct (dl->normal, testnormal);
							if (dot >= testdot - NORMAL_EPSILON)
							{
								if (count >= dl->numsunnormals)
								{
									Error ("collect spread normals: internal error.");
								}
								VectorCopy (testnormal, dl->sunnormals[count]);
								dl->sunnormalweights[count] = qmax (0, dot - testdot) * g_skynormalsizes[SUNSPREAD_SKYLEVEL][i] / totalweight;
								count++;
							}
						}
						if (count != dl->numsunnormals)
						{
							Error ("collect spread normals: internal error.");
						}
					}
					if (dl->sunspreadangle < SUNSPREAD_THRESHOLD)
					{
						for (i = 0; i < dl->numsunnormals; i++)
						{
							vec3_t tmp;
							VectorScale (dl->sunnormals[i], 1 / DotProduct (dl->sunnormals[i], dl->normal), tmp);
							VectorSubtract (tmp, dl->normal, tmp);
							VectorMA (dl->normal, dl->sunspreadangle / SUNSPREAD_THRESHOLD, tmp, dl->sunnormals[i]);
							VectorNormalize (dl->sunnormals[i]);
						}
					}
				}
				else
				{
					dl->numsunnormals = 1;
					dl->sunnormals = (vec3_t *)malloc (sizeof (vec3_t));
					dl->sunnormalweights = (vec_t *)malloc (sizeof (vec_t));
					hlassume (dl->sunnormals != NULL, assume_NoMemory);
					hlassume (dl->sunnormalweights != NULL, assume_NoMemory);
					VectorCopy (dl->normal, dl->sunnormals[0]);
					dl->sunnormalweights[0] = 1.0;
				}
#endif
            }
        }
        else
        {
            if (!VectorAvg(dl->intensity))
			{
#ifndef HLRAD_ALLOWZEROBRIGHTNESS
                VectorFill(dl->intensity, 300);
#endif
			}
            dl->type = emit_point;
        }

        if (dl->type != emit_skylight)
        {
			//why? --vluzacn
            l1 = qmax(dl->intensity[0], qmax(dl->intensity[1], dl->intensity[2]));
            l1 = l1 * l1 / 10;

            dl->intensity[0] *= l1;
            dl->intensity[1] *= l1;
            dl->intensity[2] *= l1;
        }
    }

#ifndef HLRAD_ALLOWZEROBRIGHTNESS
    hlassume(numdlights, assume_NoLights);
#endif
#ifdef HLRAD_GatherPatchLight
	int countnormallights = 0, countfastlights = 0;
	{
		int l;
	#ifdef HLRAD_VIS_FIX
		for (l = 0; l < 1 + g_dmodels[0].visleafs; l++)
	#else
	    for (l = 0; l < g_numleafs; l++)
	#endif
		{
			for (dl = directlights[l]; dl; dl = dl->next)
			{
				switch (dl->type)
				{
				case emit_surface:
				case emit_point:
				case emit_spotlight:
					if (!VectorCompare (dl->intensity, vec3_origin))
					{
						if (dl->topatch)
						{
							countfastlights++;
						}
						else
						{
							countnormallights++;
						}
					}
					break;
				case emit_skylight:
					if (!VectorCompare (dl->intensity, vec3_origin))
					{
						if (dl->topatch)
						{
							countfastlights++;
#ifdef HLRAD_SUNSPREAD
							if (dl->sunspreadangle > 0.0)
							{
								countfastlights--;
								countfastlights += dl->numsunnormals;
							}
#endif
						}
						else
						{
							countnormallights++;
#ifdef HLRAD_SUNSPREAD
							if (dl->sunspreadangle > 0.0)
							{
								countnormallights--;
								countnormallights += dl->numsunnormals;
							}
#endif
						}
					}
			#ifdef HLRAD_WHOME
					if (g_indirect_sun > 0 && !VectorCompare (dl->diffuse_intensity, vec3_origin))
			#else
					if (g_indirect_sun > 0 && !VectorCompare (dl->intensity, vec3_origin))
			#endif
					{
			#ifdef HLRAD_SOFTSKY
						if (g_softsky)
						{
							countfastlights += g_numskynormals[SKYLEVEL_SOFTSKYON];
						}
						else
						{
			#ifdef HLRAD_FASTMODE
							countfastlights += g_numskynormals[SKYLEVEL_SOFTSKYOFF];
			#else
							countnormallights += g_numskynormals[SKYLEVEL_SOFTSKYOFF];
			#endif
						}
			#else
						countnormallights += 162; //NUMVERTEXNORMALS
			#endif
					}
					break;
				default:
					hlassume(false, assume_BadLightType);
					break;
				}
			}
		}
	}
    Log("%i direct lights and %i fast direct lights\n", countnormallights, countfastlights);
#else
    Log("%i direct lights\n", numdlights);
#endif
#ifdef HLRAD_STYLEREPORT
	Log("%i light styles\n", numstyles);
#endif
#ifdef HLRAD_SKYFIX_FIX
	// move all emit_skylight to leaf 0 (the solid leaf)
	if (g_sky_lighting_fix)
	{
		directlight_t *skylights = NULL;
		int l;
	#ifdef HLRAD_VIS_FIX
		for (l = 0; l < 1 + g_dmodels[0].visleafs; l++)
	#else
	    for (l = 0; l < g_numleafs; l++)
	#endif
		{
			directlight_t **pdl;
			for (dl = directlights[l], pdl = &directlights[l]; dl; dl = *pdl)
			{
				if (dl->type == emit_skylight)
				{
					*pdl = dl->next;
					dl->next = skylights;
					skylights = dl;
				}
				else
				{
					pdl = &dl->next;
				}
			}
		}
        while ((dl = directlights[0]) != NULL)
        {
			// since they are in leaf 0, they won't emit a light anyway
            directlights[0] = dl->next;
            free(dl);
        }
		directlights[0] = skylights;
	}
#endif
#ifdef ZHLT_ENTITY_INFOSUNLIGHT
#ifdef HLRAD_MULTISKYLIGHT
	if (g_sky_lighting_fix)
	{
		int countlightenvironment = 0;
		int countinfosunlight = 0;
		for (int i = 0; i < g_numentities; i++)
		{
			entity_t *e = &g_entities[i];
			const char *classname = ValueForKey (e, "classname");
			if (!strcmp (classname, "light_environment"))
			{
				if(!g_nightmode)
				{
					if(g_daylightreturnmode && IntForKey(e, "daylightreturn") == 1 || !g_daylightreturnmode && IntForKey(e, "daylightreturn") != 1)
						countlightenvironment++;
				}
			}
			if (!strcmp (classname, "info_sunlight"))
			{
				countinfosunlight++;
			}
		}
		if (countlightenvironment > 1 && countinfosunlight == 0)
		{
			// because the map is lit by more than one light_environments, but the game can only recognize one of them when setting sv_skycolor and sv_skyvec.
			Warning ("More than one light_environments are in use. Add entity info_sunlight to clarify the sunlight's brightness for in-game model(.mdl) rendering.");
		}
	}
#endif
#endif
}

// =====================================================================================
//  DeleteDirectLights
// =====================================================================================
void            DeleteDirectLights()
{
    int             l;
    directlight_t*  dl;

#ifdef HLRAD_VIS_FIX
	for (l = 0; l < 1 + g_dmodels[0].visleafs; l++)
#else
    for (l = 0; l < g_numleafs; l++)
#endif
    {
        dl = directlights[l];
        while (dl)
        {
            directlights[l] = dl->next;
            free(dl);
            dl = directlights[l];
        }
    }

    // AJM: todo: strip light entities out at this point
	// vluzacn: hlvis and hlrad must not modify entity data, because the following procedures are supposed to produce the same bsp file:
	//  1> hlcsg -> hlbsp -> hlvis -> hlrad  (a normal compile)
	//  2) hlcsg -> hlbsp -> hlvis -> hlrad -> hlcsg -onlyents
	//  3) hlcsg -> hlbsp -> hlvis -> hlrad -> hlcsg -onlyents -> hlrad
}

// =====================================================================================
//  GatherSampleLight
// =====================================================================================
enum {
	BUMP_BASELIGHT_STYLE	= 61,
	BUMP_ADDLIGHT_STYLE		= 62,
	BUMP_LIGHTVECS_STYLE	= 63,

	DEFAULT_MAP				= 0,
	BUMP_BASELIGHT_MAP		= 1,
	BUMP_ADDLIGHT_MAP		= 3,
	BUMP_LIGHTVECS_MAP		= 2,
};

#ifndef HLRAD_SOFTSKY
#define NUMVERTEXNORMALS	162
double          r_avertexnormals[NUMVERTEXNORMALS][3] = {
//#include "../common/anorms.h"
	#include "anorms.h" //--vluzacn
};
#endif
#ifdef HLRAD_SOFTSKY
int		g_numskynormals[SKYLEVELMAX+1];
vec3_t	*g_skynormals[SKYLEVELMAX+1];
vec_t	*g_skynormalsizes[SKYLEVELMAX+1];
typedef double point_t[3];
typedef struct {int point[2]; bool divided; int child[2];} edge_t;
typedef struct {int edge[3]; int dir[3];} triangle_t;
void CopyToSkynormals (int skylevel, int numpoints, point_t *points, int numedges, edge_t *edges, int numtriangles, triangle_t *triangles)
{
	hlassume (numpoints == (1 << (2 * skylevel)) + 2, assume_first);
	hlassume (numedges == (1 << (2 * skylevel)) * 4 - 4 , assume_first);
	hlassume (numtriangles == (1 << (2 * skylevel)) * 2, assume_first);
	g_numskynormals[skylevel] = numpoints;
	g_skynormals[skylevel] = (vec3_t *)malloc (numpoints * sizeof (vec3_t));
	g_skynormalsizes[skylevel] = (vec_t *)malloc (numpoints * sizeof (vec_t));
	hlassume (g_skynormals[skylevel] != NULL, assume_NoMemory);
	hlassume (g_skynormalsizes[skylevel] != NULL, assume_NoMemory);
	int j, k;
	for (j = 0; j < numpoints; j++)
	{
		VectorCopy (points[j], g_skynormals[skylevel][j]);
		g_skynormalsizes[skylevel][j] = 0;
	}
	double totalsize = 0;
	for (j = 0; j < numtriangles; j++)
	{
		int pt[3];
		for (k = 0; k < 3; k++)
		{
			pt[k] = edges[triangles[j].edge[k]].point[triangles[j].dir[k]];
		}
		double currentsize;
		double tmp[3];
		CrossProduct (points[pt[0]], points[pt[1]], tmp);
		currentsize = DotProduct (tmp, points[pt[2]]);
		hlassume (currentsize > 0, assume_first);
		g_skynormalsizes[skylevel][pt[0]] += currentsize / 3.0;
		g_skynormalsizes[skylevel][pt[1]] += currentsize / 3.0;
		g_skynormalsizes[skylevel][pt[2]] += currentsize / 3.0;
		totalsize += currentsize;
	}
	for (j = 0; j < numpoints; j++)
	{
		g_skynormalsizes[skylevel][j] /= totalsize;
	}
#if 0
	printf ("g_numskynormals[%i]=%i\n", skylevel, g_numskynormals[skylevel]);
	for (j = 0; j < numpoints; j += (numpoints / 20 + 1))
	{
		printf ("g_skynormals[%i][%i]=%1.3f,%1.3f,%1.3f g_skynormalsizes[%i][%i]=%f\n",
			skylevel, j, g_skynormals[skylevel][j][0], g_skynormals[skylevel][j][1], g_skynormals[skylevel][j][2],
			skylevel, j, g_skynormalsizes[skylevel][j]);
	}
#endif
}
void BuildDiffuseNormals ()
{
	int i, j, k;
	g_numskynormals[0] = 0;
	g_skynormals[0] = NULL; //don't use this
	g_skynormalsizes[0] = NULL;
	int numpoints = 6;
	point_t *points = (point_t *)malloc (((1 << (2 * SKYLEVELMAX)) + 2) * sizeof (point_t));
	hlassume (points != NULL, assume_NoMemory);
	points[0][0] = 1, points[0][1] = 0, points[0][2] = 0;
	points[1][0] = -1,points[1][1] = 0, points[1][2] = 0;
	points[2][0] = 0, points[2][1] = 1, points[2][2] = 0;
	points[3][0] = 0, points[3][1] = -1,points[3][2] = 0;
	points[4][0] = 0, points[4][1] = 0, points[4][2] = 1;
	points[5][0] = 0, points[5][1] = 0, points[5][2] = -1;
	int numedges = 12;
	edge_t *edges = (edge_t *)malloc (((1 << (2 * SKYLEVELMAX)) * 4 - 4) * sizeof (edge_t));
	hlassume (edges != NULL, assume_NoMemory);
	edges[0].point[0] = 0, edges[0].point[1] = 2, edges[0].divided = false;
	edges[1].point[0] = 2, edges[1].point[1] = 1, edges[1].divided = false;
	edges[2].point[0] = 1, edges[2].point[1] = 3, edges[2].divided = false;
	edges[3].point[0] = 3, edges[3].point[1] = 0, edges[3].divided = false;
	edges[4].point[0] = 2, edges[4].point[1] = 4, edges[4].divided = false;
	edges[5].point[0] = 4, edges[5].point[1] = 3, edges[5].divided = false;
	edges[6].point[0] = 3, edges[6].point[1] = 5, edges[6].divided = false;
	edges[7].point[0] = 5, edges[7].point[1] = 2, edges[7].divided = false;
	edges[8].point[0] = 4, edges[8].point[1] = 0, edges[8].divided = false;
	edges[9].point[0] = 0, edges[9].point[1] = 5, edges[9].divided = false;
	edges[10].point[0] = 5, edges[10].point[1] = 1, edges[10].divided = false;
	edges[11].point[0] = 1, edges[11].point[1] = 4, edges[11].divided = false;
	int numtriangles = 8;
	triangle_t *triangles = (triangle_t *)malloc (((1 << (2 * SKYLEVELMAX)) * 2) * sizeof (triangle_t));
	hlassume (triangles != NULL, assume_NoMemory);
	triangles[0].edge[0] = 0, triangles[0].dir[0] = 0, triangles[0].edge[1] = 4, triangles[0].dir[1] = 0, triangles[0].edge[2] = 8, triangles[0].dir[2] = 0;
	triangles[1].edge[0] = 1, triangles[1].dir[0] = 0, triangles[1].edge[1] = 11, triangles[1].dir[1] = 0, triangles[1].edge[2] = 4, triangles[1].dir[2] = 1;
	triangles[2].edge[0] = 2, triangles[2].dir[0] = 0, triangles[2].edge[1] = 5, triangles[2].dir[1] = 1, triangles[2].edge[2] = 11, triangles[2].dir[2] = 1;
	triangles[3].edge[0] = 3, triangles[3].dir[0] = 0, triangles[3].edge[1] = 8, triangles[3].dir[1] = 1, triangles[3].edge[2] = 5, triangles[3].dir[2] = 0;
	triangles[4].edge[0] = 0, triangles[4].dir[0] = 1, triangles[4].edge[1] = 9, triangles[4].dir[1] = 0, triangles[4].edge[2] = 7, triangles[4].dir[2] = 0;
	triangles[5].edge[0] = 1, triangles[5].dir[0] = 1, triangles[5].edge[1] = 7, triangles[5].dir[1] = 1, triangles[5].edge[2] = 10, triangles[5].dir[2] = 0;
	triangles[6].edge[0] = 2, triangles[6].dir[0] = 1, triangles[6].edge[1] = 10, triangles[6].dir[1] = 1, triangles[6].edge[2] = 6, triangles[6].dir[2] = 1;
	triangles[7].edge[0] = 3, triangles[7].dir[0] = 1, triangles[7].edge[1] = 6, triangles[7].dir[1] = 0, triangles[7].edge[2] = 9, triangles[7].dir[2] = 1;
	CopyToSkynormals (1, numpoints, points, numedges, edges, numtriangles, triangles);
	for (i = 1; i < SKYLEVELMAX; i++)
	{
		int oldnumedges = numedges;
		for (j = 0; j < oldnumedges; j++)
		{
			if (!edges[j].divided)
			{
				hlassume (numpoints < (1 << (2 * SKYLEVELMAX)) + 2, assume_first);
				point_t mid;
				double len;
				VectorAdd (points[edges[j].point[0]], points[edges[j].point[1]], mid);
				len = sqrt (DotProduct (mid, mid));
				hlassume (len > 0.2, assume_first);
				VectorScale (mid, 1 / len, mid);
				int p2 = numpoints;
				VectorCopy (mid, points[numpoints]);
				numpoints++;
				hlassume (numedges < (1 << (2 * SKYLEVELMAX)) * 4 - 4, assume_first);
				edges[j].child[0] = numedges;
				edges[numedges].divided = false;
				edges[numedges].point[0] = edges[j].point[0];
				edges[numedges].point[1] = p2;
				numedges++;
				hlassume (numedges < (1 << (2 * SKYLEVELMAX)) * 4 - 4, assume_first);
				edges[j].child[1] = numedges;
				edges[numedges].divided = false;
				edges[numedges].point[0] = p2;
				edges[numedges].point[1] = edges[j].point[1];
				numedges++;
				edges[j].divided = true;
			}
		}
		int oldnumtriangles = numtriangles;
		for (j = 0; j < oldnumtriangles; j++)
		{
			int mid[3];
			for (k = 0; k < 3; k++)
			{
				hlassume (numtriangles < (1 << (2 * SKYLEVELMAX)) * 2, assume_first);
				mid[k] = edges[edges[triangles[j].edge[k]].child[0]].point[1];
				triangles[numtriangles].edge[0] = edges[triangles[j].edge[k]].child[1 - triangles[j].dir[k]];
				triangles[numtriangles].dir[0] = triangles[j].dir[k];
				triangles[numtriangles].edge[1] = edges[triangles[j].edge[(k+1)%3]].child[triangles[j].dir[(k+1)%3]];
				triangles[numtriangles].dir[1] = triangles[j].dir[(k+1)%3];
				triangles[numtriangles].edge[2] = numedges + k;
				triangles[numtriangles].dir[2] = 1;
				numtriangles++;
			}
			for (k = 0; k < 3; k++)
			{
				hlassume (numedges < (1 << (2 * SKYLEVELMAX)) * 4 - 4, assume_first);
				triangles[j].edge[k] = numedges;
				triangles[j].dir[k] = 0;
				edges[numedges].divided = false;
				edges[numedges].point[0] = mid[k];
				edges[numedges].point[1] = mid[(k+1)%3];
				numedges++;
			}
		}
		CopyToSkynormals (i + 1, numpoints, points, numedges, edges, numtriangles, triangles);
	}
	free (points);
	free (edges);
	free (triangles);
}
#endif

static void AddLight( directlight_t* l, vec3_t& direction, const vec3_t pos, const byte* const pvs, const vec3_t normal, vec3_t* sample, byte* styles, int step, bool bumppass )
{
	vec3_t			add_one_ambient, add_one_diffuse;
	vec3_t			add, add_diffuse, add_ambient;
    vec3_t          delta, delta_bump;
    float           dot, dot2;
    float           dist;
    float           ratio, ratio_bump;
    int             style_index;
	int				step_match;
	bool			sky_used = false;
	vec3_t			testline_origin;

	VectorClear (add);
	VectorClear (add_diffuse);
	VectorClear (add_ambient);

	// skylights work fundamentally differently than normal lights
	if (l->type == emit_skylight)
	{
		if (!g_sky_lighting_fix)
		{
			if (sky_used)
				return;

			sky_used = true;
		}

		VectorClear (add);

		do // add sun light
		{
			// check step
			step_match = (int)l->topatch;
			if (step != step_match)
				continue;

			// check intensity
			if (!(l->intensity[0] || l->intensity[1] || l->intensity[2]))
				continue;
				
			// loop over the normals
			for (int j = 0; j < l->numsunnormals; j++)
			{
				// make sure the angle is okay
				dot = -DotProduct (normal, l->sunnormals[j]);
				if (dot <= NORMAL_EPSILON) //ON_EPSILON / 10 //--vluzacn
					continue;

				// search back to see if we can hit a sky brush
				VectorScale(l->sunnormals[j], -BOGUS_RANGE, delta);
				VectorAdd(pos, delta, delta);

				vec3_t skyhit;
				VectorCopy (delta, skyhit);

				if (TestLine(pos, delta) != CONTENTS_SKY)
					continue; // occluded

				vec3_t transparency;
				if (TestSegmentAgainstOpaqueList(pos, skyhit, transparency))
					continue;

				vec3_t add_one;
				// Only add default light in non- bump pass
				if(!bumppass)
				{
					VectorScale (l->intensity, dot * l->sunnormalweights[j], add_one);
					VectorMultiply(add_one, transparency, add_one);
				
					// add to the contribution of this light
					VectorAdd (add, add_one, add);
				}

				if(g_bumpmaps)
				{
					// Inverse the light for the sun and copy it
					VectorCopy(delta, delta_bump);
					VectorNormalize(delta_bump);

					if(bumppass)
					{
						// Contribution comes from correlation between
						// the gathered light vector and the light direction
						float dotn = qmax(0, DotProduct(direction, delta_bump));

						// Calculate the lighting for ambient and diffuse
						VectorScale(l->intensity, dotn*l->sunnormalweights[j], add_one_diffuse);
						VectorScale(l->intensity, (1 - dotn)*l->sunnormalweights[j], add_one_ambient);	

						// Multiply by the transparency
						VectorMultiply(add_one_diffuse, transparency, add_one_diffuse);
						VectorMultiply(add_one_ambient, transparency, add_one_ambient);

						// Add it to the collection
						VectorAdd(add_diffuse, add_one_diffuse, add_diffuse);
						VectorAdd(add_ambient, add_one_ambient, add_ambient);
					}
					else
					{
						// Add lightdir to the collected direction based on strength
						float maxlight = VectorAvg(add_one);
						VectorScale(delta_bump, maxlight, delta_bump);
						VectorAdd(direction, delta_bump, direction);
					}
				}
			} // (loop over the normals)
		}
		while (0);

		do // add sky light
		{
			// check step
			step_match = 0;
			if (g_softsky)
				step_match = 1;

			if (g_fastmode)
				step_match = 1;

			if (step != step_match)
				continue;

			// check intensity
			if (g_indirect_sun <= 0.0 || VectorCompare ( l->diffuse_intensity, vec3_origin) && VectorCompare (l->diffuse_intensity2, vec3_origin))
				continue;

			vec3_t sky_intensity;

			// loop over the normals
			vec3_t *skynormals = g_skynormals[g_softsky?SKYLEVEL_SOFTSKYON:SKYLEVEL_SOFTSKYOFF];
			vec_t *skyweights = g_skynormalsizes[g_softsky?SKYLEVEL_SOFTSKYON:SKYLEVEL_SOFTSKYOFF];
			
			for (int j = 0; j < g_numskynormals[g_softsky?SKYLEVEL_SOFTSKYON:SKYLEVEL_SOFTSKYOFF]; j++)
			{
				// make sure the angle is okay
				dot = -DotProduct (normal, skynormals[j]);
				if (dot <= NORMAL_EPSILON) //ON_EPSILON / 10 //--vluzacn
					continue;

				// search back to see if we can hit a sky brush
				VectorScale (skynormals[j], -BOGUS_RANGE, delta);
				VectorAdd(pos, delta, delta);

				vec3_t skyhit;
				VectorCopy (delta, skyhit);

				if (TestLine(pos, delta, skyhit) != CONTENTS_SKY)
					continue;                                  // occluded

				vec3_t transparency;
				if (TestSegmentAgainstOpaqueList(pos, skyhit, transparency))
					continue;

				vec_t factor = qmin (qmax (0.0, (1 - DotProduct (l->normal, skynormals[j])) / 2), 1.0); // how far this piece of sky has deviated from the sun
				VectorScale (l->diffuse_intensity, 1 - factor, sky_intensity);
				VectorMA (sky_intensity, factor, l->diffuse_intensity2, sky_intensity);
				VectorScale (sky_intensity, skyweights[j] * g_indirect_sun / 2, sky_intensity);

				vec3_t add_one;
				VectorScale(sky_intensity, dot, add_one);
				VectorMultiply(add_one, transparency, add_one);

				// Don't add it twice
				if(!bumppass) {
					VectorAdd(add, add_one, add); 
				}
				else{
					VectorAdd(add_ambient, add_one, add_ambient); 
				}

			} // (loop over the normals)

		}
		while (0);
	}
	else // not emit_skylight
	{
		step_match = (int)l->topatch;
		if (step != step_match)
			return;

		if (!(l->intensity[0] || l->intensity[1] || l->intensity[2]))
			return;

		float denominator;
		VectorCopy (l->origin, testline_origin);
		VectorSubtract(l->origin, pos, delta);

		if (l->type == emit_surface)
		{
			// move emitter back to its plane
			VectorMA (delta, -PATCH_HUNT_OFFSET, l->normal, delta);
		}

		dist = VectorNormalize(delta);
		dot = DotProduct(delta, normal);
		if (dist < 1.0)
			dist = 1.0;

		denominator = dist * dist * l->fade;

		switch (l->type)
		{
			case emit_point:
			{
				if (dot <= NORMAL_EPSILON)
				{
					return;
				}
				vec_t denominator = dist * dist * l->fade;

				ratio = dot / denominator;
				ratio_bump = 1.0 / denominator;

				// Only add default light in non- bump pass
				if(!bumppass)
					VectorScale(l->intensity, ratio, add);

				break;
			}

			case emit_surface:
			{
				bool light_behind_surface = false;
				if (dot <= NORMAL_EPSILON)
					light_behind_surface = true;

				dot2 = -DotProduct(delta, l->normal);
				if (dot2 * dist <= MINIMUM_PATCH_DISTANCE)
					return;

				vec_t range = l->patch_emitter_range;
				if (l->stopdot > 0.0) // stopdot2 > 0.0 or stopdot > 0.0
				{
					vec_t range_scale;
					range_scale = 1 - l->stopdot2 * l->stopdot2;
					range_scale = 1 / sqrt (qmax (NORMAL_EPSILON, range_scale));
					// range_scale = 1 / sin (cone2)
					range_scale = qmin (range_scale, 2); // restrict this to 2, because skylevel has limit.
					range *= range_scale; // because smaller cones are more likely to create the ugly grid effect.

					if (dot2 <= l->stopdot2 + NORMAL_EPSILON)
					{
						if (dist >= range) // use the old method, which will merely give 0 in this case
						{
							return;
						}
						ratio = 0.0;
						ratio_bump = 0.0;
					}
					else if (dot2 <= l->stopdot)
					{
						ratio = dot * dot2 * (dot2 - l->stopdot2) / (dist * dist * (l->stopdot - l->stopdot2));
						ratio_bump = dot2 * (dot2 - l->stopdot2) / (dist * dist * (l->stopdot - l->stopdot2));
					}
					else
					{
						ratio = dot * dot2 / (dist * dist);
						ratio_bump = dot2 / (dist * dist);
					}
				}
				else
				{
					ratio = dot * dot2 / (dist * dist);
					ratio_bump = dot2 / (dist * dist);
				}
				
				// analogous to the one in MakeScales
				// 0.4f is tested to be able to fully eliminate bright spots
				if (ratio * l->patch_area > 0.4f)
				{
					ratio = 0.4f / l->patch_area;
					ratio_bump = ratio;
				}

				if (dist < range - ON_EPSILON)
				{ // do things slow
					if (light_behind_surface)
					{
						dot = 0.0;
						ratio = 0.0;
						ratio_bump = 0.0;
					}

					GetAlternateOrigin (pos, normal, l->patch, testline_origin);
					
					vec_t sightarea;
					int skylevel = l->patch->emitter_skylevel;
					if (l->stopdot > 0.0) // stopdot2 > 0.0 or stopdot > 0.0
					{
						const vec_t *emitnormal = getPlaneFromFaceNumber (l->patch->faceNumber)->normal;
						if (l->stopdot2 >= 0.8) // about 37deg
							skylevel += 1; // because the range is larger

						sightarea = CalcSightArea_SpotLight (pos, normal, l->patch->winding, emitnormal, l->stopdot, l->stopdot2, skylevel); // because we have doubled the range
					}
					else
					{
						sightarea = CalcSightArea (pos, normal, l->patch->winding, skylevel);
					}

					vec_t frac = dist / range;
					frac = (frac - 0.5) * 2; // make a smooth transition between the two methods
					frac = qmax (0, qmin (frac, 1));

					vec_t ratio2 = (sightarea / l->patch_area); // because l->patch->area has been multiplied into l->intensity
					ratio = frac * ratio + (1 - frac) * ratio2;
					ratio_bump = frac * ratio_bump + (1 - frac) * ratio2;
				}
				else
				{
					if (light_behind_surface)
						return;
				}

				// Only add default light in non- bump pass
				if(!bumppass)
					VectorScale(l->intensity, ratio, add);

				break;
			}

			case emit_spotlight:
			{
				if (dot <= NORMAL_EPSILON)
					return;

				dot2 = -DotProduct(delta, l->normal);
				if (dot2 <= l->stopdot2)
					return;

				// Variable power falloff (1 = inverse linear, 2 = inverse square
				vec_t denominator = dist * l->fade;
				denominator *= dist;

				ratio = dot * dot2 / denominator;
				ratio_bump = dot2 / denominator;

				if (dot2 <= l->stopdot)
				{
					float spotdotfactor = (dot2 - l->stopdot2) / (l->stopdot - l->stopdot2);
					ratio *= spotdotfactor;
					ratio_bump *= spotdotfactor;
				}

				// Only add default light in non- bump pass
				if(!bumppass)
					VectorScale(l->intensity, ratio, add);

				break;
			}

			default:
			{
				hlassume(false, assume_BadLightType);
				break;
			}
		}

		vec3_t transparency = {1.0,1.0,1.0}; 
		if (TestLine(pos, testline_origin) != CONTENTS_EMPTY)
			return;// occluded

		// Don't test from light_environment entities to face, the special sky code occludes correctly
		if (TestSegmentAgainstOpaqueList(pos, testline_origin, transparency))
			return;

		// Only add default light in non- bump pass
		if(!bumppass)
			VectorMultiply(add, transparency, add);

		// Special calculations
		if(g_bumpmaps)
		{
			// Inverse the light for the sun and copy it
			VectorCopy(delta, delta_bump);
			VectorNormalize(delta_bump);

			if(bumppass)
			{
				// Contribution comes from correlation between
				// the gathered light vector and the light direction
				float dotn = pow(qmax(0, DotProduct(direction, delta_bump)), 16);
				ratio_bump = qmax(0, ratio_bump);

				// Calculate the lighting for ambient and diffuse
				VectorScale(l->intensity, dotn*ratio_bump, add_one_diffuse);
				VectorScale(l->intensity, (1 - dotn)*ratio, add_one_ambient);	

				// Multiply by the transparency
				VectorMultiply(add_one_diffuse, transparency, add_one_diffuse);
				VectorMultiply(add_one_ambient, transparency, add_one_ambient);

				// Add it to the collection
				VectorAdd(add_diffuse, add_one_diffuse, add_diffuse);
				VectorAdd(add_ambient, add_one_ambient, add_ambient);
			}
			else
			{
				// Add lightdir to the collected direction based on strength
				float maxlight = VectorAvg(add);
				VectorScale(delta_bump, maxlight, delta_bump);
				VectorAdd(direction, delta_bump, direction);
			}
		}

	} // end emit_skylight

	// No lightstyle lookup with bump maps
	if(!g_bumpmaps)
	{
		for (style_index = 0; style_index < MAXLIGHTMAPS; style_index++)
		{
			if (styles[style_index] == l->style || styles[style_index] == 255)
				break;
		}

		if (style_index == MAXLIGHTMAPS)
		{
			Warning("Too many direct light styles on a face(%f,%f,%f)", pos[0], pos[1], pos[2]);
			return;
		}

		if (styles[style_index] == 255)
		{
			styles[style_index] = l->style;
		}

		VectorAdd(sample[style_index], add, sample[style_index]);
	}
	else
	{
		// Just copy the values to the desired styles

		if(bumppass)
		{
			// Add it to the sample
			VectorAdd(sample[BUMP_ADDLIGHT_MAP], add_diffuse, sample[BUMP_ADDLIGHT_MAP]);
			VectorAdd(sample[BUMP_BASELIGHT_MAP], add_ambient, sample[BUMP_BASELIGHT_MAP]);
		}
		else
		{
			// Add it to the default lightmap
			VectorAdd(sample[DEFAULT_MAP], add, sample[DEFAULT_MAP]);
		}
	}
}

static void GatherSampleLight( const vec3_t pos, const byte* const pvs, const vec3_t normal, vec3_t* sample, byte* styles, int step )
{
    int i;
    directlight_t* l;
    vec3_t direction;

	// Clear this
	VectorClear(direction);

	//
	// First step - Calculate regular lighting
	// For Bump maps - Get combined light vector to all lights hitting this surface
	//
    for (i = 0; i < 1 + g_dmodels[0].visleafs; i++)
    {
        l = directlights[i];
        if (l)
		{
            if (i == 0? g_sky_lighting_fix: pvs[(i - 1) >> 3] & (1 << ((i - 1) & 7)))
            {
                for (; l; l = l->next)
					AddLight(l, direction, pos, pvs, normal, sample, styles, step, false);
            }
        }
    }

	//
	// Second step - Calculate bump map lighting using the gathered light directions
	//
	if(g_bumpmaps)
	{
		// Normalize the lightdir
		VectorNormalize(direction);

		for (i = 0; i < 1 + g_dmodels[0].visleafs; i++)
		{
			l = directlights[i];
			if (l)
			{
				if (i == 0? g_sky_lighting_fix: pvs[(i - 1) >> 3] & (1 << ((i - 1) & 7)))
				{
					for (; l; l = l->next)
						AddLight(l, direction, pos, pvs, normal, sample, styles, step, true);
				}
			}
		}

		// Add the light vector to the sample
		VectorAdd(sample[BUMP_LIGHTVECS_MAP], direction, sample[BUMP_LIGHTVECS_MAP]);
	}
}

// =====================================================================================
//  AddSampleToPatch
//      Take the sample's collected light and add it back into the apropriate patch for the radiosity pass.
// =====================================================================================
#ifdef HLRAD_ACCURATEBOUNCE_SAMPLELIGHT
static void AddSamplesToPatches (const sample_t **samples, const unsigned char *styles, int facenum, const lightinfo_t *l)
{
#ifndef HLRAD_GatherPatchLight
    if (g_numbounce == 0)
    {
        return;
    }
#endif
	patch_t *patch;
	int i, j, m, k;
	int numtexwindings;
	Winding **texwindings;

	numtexwindings = 0;
	for (patch = g_face_patches[facenum]; patch; patch = patch->next)
	{
		numtexwindings++;
	}
	texwindings = (Winding **)malloc (numtexwindings * sizeof (Winding *));
	hlassume (texwindings != NULL, assume_NoMemory);

	// translate world winding into winding in s,t plane
	for (j = 0, patch = g_face_patches[facenum]; j < numtexwindings; j++, patch = patch->next)
	{
		Winding *w = new Winding (patch->winding->m_NumPoints);
		for (int x = 0; x < w->m_NumPoints; x++)
		{
			vec_t s, t;
			SetSTFromSurf (l, patch->winding->m_Points[x], s, t);
			w->m_Points[x][0] = s;
			w->m_Points[x][1] = t;
			w->m_Points[x][2] = 0.0;
		}
		w->RemoveColinearPoints ();
		texwindings[j] = w;
	}
	
	for (i = 0; i < l->numsurfpt; i++)
	{
		// prepare clip planes
		vec_t s_vec, t_vec;
		s_vec = (l->texmins[0] + (i % (l->texsize[0] + 1))) * TEXTURE_STEP;
		t_vec = (l->texmins[1] + (i / (l->texsize[0] + 1))) * TEXTURE_STEP;

		dplane_t clipplanes[4];
		VectorClear (clipplanes[0].normal);
		clipplanes[0].normal[0] = 1;
		clipplanes[0].dist = s_vec - 0.5 * TEXTURE_STEP;
		VectorClear (clipplanes[1].normal);
		clipplanes[1].normal[0] = -1;
		clipplanes[1].dist = -(s_vec + 0.5 * TEXTURE_STEP);
		VectorClear (clipplanes[2].normal);
		clipplanes[2].normal[1] = 1;
		clipplanes[2].dist = t_vec - 0.5 * TEXTURE_STEP;
		VectorClear (clipplanes[3].normal);
		clipplanes[3].normal[1] = -1;
		clipplanes[3].dist = -(t_vec + 0.5 * TEXTURE_STEP);

		// clip each patch
		for (j = 0, patch = g_face_patches[facenum]; j < numtexwindings; j++, patch = patch->next)
		{
			Winding *w = new Winding (*texwindings[j]);
			for (k = 0; k < 4; k++)
			{
				if (w->m_NumPoints)
				{
					w->Clip (clipplanes[k], false);
				}
			}
			if (w->m_NumPoints)
			{
				// add sample to patch
				vec_t area = w->getArea () / (TEXTURE_STEP * TEXTURE_STEP);
				patch->samples += area;
				for (m = 0; m < ALLSTYLES && styles[m] != 255; m++)
				{
					int style = styles[m];
					const sample_t *s = &samples[m][i];
					for (k = 0; k < ALLSTYLES && patch->totalstyle_all[k] != 255; k++)
					{
						if (patch->totalstyle_all[k] == style)
						{
							break;
						}
					}
					if (k == ALLSTYLES)
					{
			#ifdef HLRAD_READABLE_EXCEEDSTYLEWARNING
						if (++stylewarningcount >= stylewarningnext)
						{
							stylewarningnext = stylewarningcount * 2;
							Warning("Too many direct light styles on a face(?,?,?)\n");
							Warning(" total %d warnings for too many styles", stylewarningcount);
						}
			#else
						Warning("Too many direct light styles on a face(?,?,?)\n");
			#endif
					}
					else
					{
						if (patch->totalstyle_all[k] == 255)
						{
							patch->totalstyle_all[k] = style;
						}
						VectorMA (patch->samplelight_all[k], area, s->light, patch->samplelight_all[k]);
			#ifdef ZHLT_XASH
						VectorMA (patch->samplelight_all_direction[k], area, s->light_direction, patch->samplelight_all_direction[k]);
			#endif
					}
				}
			}
			delete w;
		}
	}

	for (j = 0; j < numtexwindings; j++)
	{
		delete texwindings[j];
	}
	free (texwindings);
}
#else
#ifdef ZHLT_TEXLIGHT
static void     AddSampleToPatch(const sample_t* const s, const int facenum, int style) //LRC
#else
static void     AddSampleToPatch(const sample_t* const s, const int facenum)
#endif
{
    patch_t*        patch;
    BoundingBox     bounds;
    int             i;

#ifndef HLRAD_GatherPatchLight
    if (g_numbounce == 0)
    {
        return;
    }
#endif

    for (patch = g_face_patches[facenum]; patch; patch = patch->next)
    {
        // see if the point is in this patch (roughly)
        patch->winding->getBounds(bounds);
        for (i = 0; i < 3; i++)
        {
            if (bounds.m_Mins[i] > s->pos[i] + 16)
            {
                goto nextpatch;
            }
            if (bounds.m_Maxs[i] < s->pos[i] - 16)
            {
                goto nextpatch;
            }
        }
#ifdef HLRAD_AUTOCORING
		if (style == 0)
		{
			patch->samples++;
		}
#endif

        // add the sample to the patch
#ifdef ZHLT_TEXLIGHT
        //LRC:
	#ifdef HLRAD_AUTOCORING
		for (i = 0; i < ALLSTYLES && patch->totalstyle_all[i] != 255; i++)
		{
			if (patch->totalstyle_all[i] == style)
				break;
		}
		if (i == ALLSTYLES) // shouldn't happen
	#else
		for (i = 0; i < MAXLIGHTMAPS && patch->totalstyle[i] != 255; i++)
		{
			if (patch->totalstyle[i] == style)
				break;
		}
		if (i == MAXLIGHTMAPS)
	#endif
		{
#ifdef HLRAD_READABLE_EXCEEDSTYLEWARNING
			if (++stylewarningcount >= stylewarningnext)
			{
				stylewarningnext = stylewarningcount * 2;
				Warning("Too many direct light styles on a face(?,?,?)\n");
				Warning(" total %d warnings for too many styles", stylewarningcount);
			}
#else
			Warning("Too many direct light styles on a face(?,?,?)\n");
#endif
		}
		else
		{
	#ifdef HLRAD_AUTOCORING
			if (patch->totalstyle_all[i] == 255)
			{
				patch->totalstyle_all[i] = style;
			}
			VectorAdd(patch->samplelight_all[i], s->light, patch->samplelight_all[i]);
		#ifdef ZHLT_XASH
			VectorAdd (patch->samplelight_all_direction[i], s->light_direction, patch->samplelight_all_direction[i]);
		#endif
	#else
			if (patch->totalstyle[i] == 255)
			{
				patch->totalstyle[i] = style;
			}

	        patch->samples[i]++;
			VectorAdd(patch->samplelight[i], s->light, patch->samplelight[i]);
	#endif
		}
        //LRC (ends)
#else
        patch->samples++;
        VectorAdd(patch->samplelight, s->light, patch->samplelight);
#endif
        //return;

      nextpatch:;
    }

    // don't worry if some samples don't find a patch
}
#endif

// =====================================================================================
//  GetPhongNormal
// =====================================================================================
void            GetPhongNormal(int facenum, vec3_t spot, vec3_t phongnormal)
{
    int             j;
#ifdef HLRAD_GetPhongNormal_VL
	int				s; // split every edge into two parts
#endif
    const dface_t*  f = g_dfaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);
    vec3_t          facenormal;

    VectorCopy(p->normal, facenormal);
    VectorCopy(facenormal, phongnormal);

#ifndef HLRAD_CUSTOMSMOOTH
    if (g_smoothing_threshold > 0.0)
#endif
    {
        // Calculate modified point normal for surface
        // Use the edge normals iff they are defined.  Bend the surface towards the edge normal(s)
        // Crude first attempt: find nearest edge normal and do a simple interpolation with facenormal.
        // Second attempt: find edge points+center that bound the point and do a three-point triangulation(baricentric)
        // Better third attempt: generate the point normals for all vertices and do baricentric triangulation.

        for (j = 0; j < f->numedges; j++)
        {
            vec3_t          p1;
            vec3_t          p2;
            vec3_t          v1;
            vec3_t          v2;
            vec3_t          vspot;
            unsigned        prev_edge;
            unsigned        next_edge;
            int             e;
            int             e1;
            int             e2;
            edgeshare_t*    es;
            edgeshare_t*    es1;
            edgeshare_t*    es2;
            float           a1;
            float           a2;
            float           aa;
            float           bb;
            float           ab;

            if (j)
            {
#ifdef HLRAD_NEGATIVEDIVIDEND_MISCFIX
                prev_edge = f->firstedge + ((j + f->numedges - 1) % f->numedges);
#else
                prev_edge = f->firstedge + ((j - 1) % f->numedges);
#endif
            }
            else
            {
                prev_edge = f->firstedge + f->numedges - 1;
            }

            if ((j + 1) != f->numedges)
            {
                next_edge = f->firstedge + ((j + 1) % f->numedges);
            }
            else
            {
                next_edge = f->firstedge;
            }

            e = g_dsurfedges[f->firstedge + j];
            e1 = g_dsurfedges[prev_edge];
            e2 = g_dsurfedges[next_edge];

            es = &g_edgeshare[abs(e)];
            es1 = &g_edgeshare[abs(e1)];
            es2 = &g_edgeshare[abs(e2)];

#ifdef HLRAD_GetPhongNormal_VL
			if ((es->coplanar || !es->smooth) && (es1->coplanar || !es1->smooth) && (es2->coplanar || !es2->smooth))
#else
            if (
                (es->coplanar && es1->coplanar && es2->coplanar)
                ||
                (VectorCompare(es->interface_normal, vec3_origin) &&
                 VectorCompare(es1->interface_normal, vec3_origin) &&
                 VectorCompare(es2->interface_normal, vec3_origin)))
#endif
            {
                continue;
            }

            if (e > 0)
            {
                VectorCopy(g_dvertexes[g_dedges[e].v[0]].point, p1);
                VectorCopy(g_dvertexes[g_dedges[e].v[1]].point, p2);
            }
            else
            {
                VectorCopy(g_dvertexes[g_dedges[-e].v[1]].point, p1);
                VectorCopy(g_dvertexes[g_dedges[-e].v[0]].point, p2);
            }

            // Adjust for origin-based models
            VectorAdd(p1, g_face_offset[facenum], p1);
            VectorAdd(p2, g_face_offset[facenum], p2);
#ifdef HLRAD_GetPhongNormal_VL
		for (s = 0; s < 2; s++)
		{
			vec3_t s1, s2;
			if (s == 0)
			{
				VectorCopy(p1, s1);
			}
			else
			{
				VectorCopy(p2, s1);
			}

			VectorAdd(p1,p2,s2); // edge center
			VectorScale(s2,0.5,s2);

            VectorSubtract(s1, g_face_centroids[facenum], v1);
            VectorSubtract(s2, g_face_centroids[facenum], v2);
#else

            // Build vectors from the middle of the face to the edge vertexes and the sample pos.
            VectorSubtract(p1, g_face_centroids[facenum], v1);
            VectorSubtract(p2, g_face_centroids[facenum], v2);
#endif
            VectorSubtract(spot, g_face_centroids[facenum], vspot);

            aa = DotProduct(v1, v1);
            bb = DotProduct(v2, v2);
            ab = DotProduct(v1, v2);
            a1 = (bb * DotProduct(v1, vspot) - ab * DotProduct(vspot, v2)) / (aa * bb - ab * ab);
            a2 = (DotProduct(vspot, v2) - a1 * ab) / bb;

            // Test center to sample vector for inclusion between center to vertex vectors (Use dot product of vectors)
#ifdef HLRAD_GetPhongNormal_VL
            if (a1 >= -ON_EPSILON && a2 >= -ON_EPSILON)
#else
            if (a1 >= 0.0 && a2 >= 0.0)
#endif
            {
                // calculate distance from edge to pos
                vec3_t          n1, n2;
                vec3_t          temp;

#ifdef HLRAD_GetPhongNormal_VL
				/*
				if (s == 0)
				{VectorCopy(es1->interface_normal, n1);}
				else
				{VectorCopy(es2->interface_normal, n1);}
				if (VectorCompare(n1, vec3_origin))
				{VectorCopy(facenormal, n1);}
				if (VectorCompare(es->interface_normal, vec3_origin))
				{VectorAdd(n1, facenormal, n1);}
				else
				{VectorAdd(n1, es->interface_normal, n1);}
				VectorSubtract(n1, facenormal, n1);
				VectorNormalize(n1);
				*/
				if (es->smooth)
					if (s == 0)
					{VectorCopy(es->vertex_normal[e>0?0:1], n1);}
					else
					{VectorCopy(es->vertex_normal[e>0?1:0], n1);}
				else if (s == 0 && es1->smooth)
				{VectorCopy(es1->vertex_normal[e1>0?1:0], n1);}
				else if (s == 1 && es2->smooth)
				{VectorCopy(es2->vertex_normal[e2>0?0:1], n1);}
				else
				{VectorCopy(facenormal, n1);}

				if (es->smooth)
				{VectorCopy(es->interface_normal, n2);}
				else
				{VectorCopy(facenormal, n2);}
#else
                VectorAdd(es->interface_normal, es1->interface_normal, n1)

                if (VectorCompare(n1, vec3_origin))
                {
                    VectorCopy(facenormal, n1);
                }
                VectorNormalize(n1);

                VectorAdd(es->interface_normal, es2->interface_normal, n2);

                if (VectorCompare(n2, vec3_origin))
                {
                    VectorCopy(facenormal, n2);
                }
                VectorNormalize(n2);
#endif

                // Interpolate between the center and edge normals based on sample position
                VectorScale(facenormal, 1.0 - a1 - a2, phongnormal);
                VectorScale(n1, a1, temp);
                VectorAdd(phongnormal, temp, phongnormal);
                VectorScale(n2, a2, temp);
                VectorAdd(phongnormal, temp, phongnormal);
                VectorNormalize(phongnormal);
                break;
            }
#ifdef HLRAD_GetPhongNormal_VL
		} // s=0,1
#endif
        }
    }
}

const vec3_t    s_circuscolors[] = {
    {100000.0,  100000.0,   100000.0},                              // white
    {100000.0,  0.0,        0.0     },                              // red
    {0.0,       100000.0,   0.0     },                              // green
    {0.0,       0.0,        100000.0},                              // blue
    {0.0,       100000.0,   100000.0},                              // cyan
    {100000.0,  0.0,        100000.0},                              // magenta
    {100000.0,  100000.0,   0.0     }                               // yellow
};

// =====================================================================================
//  BuildFacelights
// =====================================================================================
void CalcLightmap (lightinfo_t *l, byte *styles)
{
	int facenum;
	int i, j;
	byte pvs[(MAX_MAP_LEAFS + 7) / 8];
	int lastoffset;
	byte pvs2[(MAX_MAP_LEAFS + 7) / 8];
	int lastoffset2;

	facenum = l->surfnum;
	memset (l->lmcache, 0, l->lmcachewidth * l->lmcacheheight * sizeof (vec3_t [MAXLIGHTMAPS]));

	// for each sample whose light we need to calculate
	for (i = 0; i < l->lmcachewidth * l->lmcacheheight; i++)
	{
		vec_t s, t;
		vec_t s_vec, t_vec;
		int nearest_s, nearest_t;
		vec3_t spot;
		vec3_t pointnormal;
		bool blocked;
		vec3_t spot2;
		vec3_t pointnormal2;
		vec3_t *sampled;

		// prepare input parameter and output parameter
		{
			s = ((i % l->lmcachewidth) - l->lmcache_offset) / (vec_t)l->lmcache_density;
			t = ((i / l->lmcachewidth) - l->lmcache_offset) / (vec_t)l->lmcache_density;
			s_vec = l->texmins[0] * 16 + s * TEXTURE_STEP;
			t_vec = l->texmins[1] * 16 + t * TEXTURE_STEP;
			nearest_s = qmax (0, qmin (floor (s + 0.5), l->texsize[0]));
			nearest_t = qmax (0, qmin (floor (t + 0.5), l->texsize[1]));
			sampled = l->lmcache[i];
		}
		// find world's position for the sample
		{
			if (i == (nearest_s * l->lmcache_density + l->lmcache_offset)
				+ l->lmcachewidth * (nearest_t * l->lmcache_density + l->lmcache_offset)) // almost always true when compiled with no '-extra'
			{
				j = nearest_s + (l->texsize[0] + 1) * nearest_t;
				VectorCopy (l->surfpt[j], spot);
				blocked = l->lightoutside[j];
			}
			else
			{
				blocked = false;
				if (SetSampleFromST (spot, l, s_vec, t_vec, g_face_lightmode[facenum]) == LightOutside)
				{
					j = nearest_s + (l->texsize[0] + 1) * nearest_t;
					if (l->lightoutside[j])
					{
						blocked = true;
					}
					else
					{
						VectorCopy(l->surfpt[j], spot);
					}
				}
			}

			if (l->translucent_b)
			{
				vec3_t delta;
				VectorSubtract (g_face_centroids[facenum], spot, delta);
				VectorNormalize (delta);
				VectorMA (spot, 0.2, delta, spot2);
				VectorMA (spot2, -(g_translucentdepth + 2*DEFAULT_HUNT_OFFSET), l->facenormal, spot2);
			}
		}
		// calculate normal for the sample
		{
			vec3_t pos_original;
			SetSurfFromST (l, pos_original, s_vec, t_vec);
			{
				// adjust sample's offset to 0
				vec_t scale;
				scale = DotProduct (l->texnormal, l->facenormal);
				VectorMA (pos_original, - DEFAULT_HUNT_OFFSET / scale, l->texnormal, pos_original);
			}
			GetPhongNormal(facenum, pos_original, pointnormal);
			VectorSubtract (vec3_origin, pointnormal, pointnormal2);
		}
		// calculate visibility for the sample
		{
			if (!g_visdatasize)
			{
				if (i == 0)
				{
					memset(pvs, 255, (g_numleafs + 7) / 8);
				}
			}
			else
			{
				dleaf_t *leaf = PointInLeaf(spot);
				int thisoffset = leaf->visofs;
				if (i == 0 || thisoffset != lastoffset)
				{
					if (thisoffset == -1)
					{
						memset (pvs, 0, (g_numleafs + 7) / 8);
					}
					else
					{
						DecompressVis(&g_dvisdata[leaf->visofs], pvs, sizeof(pvs));
					}
				}
				lastoffset = thisoffset;
			}

			if (l->translucent_b)
			{
				if (!g_visdatasize)
				{
					if (i == 0)
					{
						memset(pvs2, 255, (g_numleafs + 7) / 8);
					}
				}
				else
				{
					dleaf_t *leaf2 = PointInLeaf(spot2);
					int thisoffset2 = leaf2->visofs;
					if (i == 0 || thisoffset2 != lastoffset2)
					{
						if (thisoffset2 == -1)
						{
							memset(pvs2, 0, (g_numleafs + 7) / 8);
						}
						else
						{
							DecompressVis(&g_dvisdata[leaf2->visofs], pvs2, sizeof(pvs2));
						}
					}
					lastoffset2 = thisoffset2;
				}
			}
		}
		// gather light
		{
			if (!blocked)
				GatherSampleLight(spot, pvs, pointnormal, sampled, styles, 0);

			if (l->translucent_b)
			{
				vec3_t sampled2[MAXLIGHTMAPS];
				memset (sampled2, 0, MAXLIGHTMAPS * sizeof (vec3_t));

				if (!blocked)
				{
					GatherSampleLight(spot2, pvs2, pointnormal2, sampled2
						, styles
						, 0
						);
				}

				for (j = 0; j < MAXLIGHTMAPS && styles[j] != 255; j++)
				{
					for (int x = 0; x < 3; x++)
					{
						sampled[j][x] = (1.0 - l->translucent_v[x]) * sampled[j][x] + l->translucent_v[x] * sampled2[j][x];
					}
				}
			}
		}
	}
}

void BuildFacelights(const int facenum)
{
    dface_t*        f;

    lightinfo_t     l;
    int             i;
    int             j;
    int             k;
    sample_t*       s;
    vec_t*          spot;
    patch_t*        patch;
    const dplane_t* plane;
    byte            pvs[(MAX_MAP_LEAFS + 7) / 8];
    int             thisoffset = -1, lastoffset = -1;
    int             lightmapwidth;
    int             lightmapheight;
    int             size;
	int				ofs;
	int				e_index;
	miptex_t		*mt;
	vec3_t			spot2, normal2;
	vec3_t			delta;
	byte			pvs2[(MAX_MAP_LEAFS + 7) / 8];
	int				thisoffset2 = -1, lastoffset2 = -1;
	float			tbnmatrix[3][3];

    f = &g_dfaces[facenum];

	//
    // some surfaces don't need lightmaps
    //
    f->lightofs = -1;

	if(!g_bumpmaps)
	{
		for (j = 0; j < MAXLIGHTMAPS; j++)
			f->styles[j] = 255;
	}

    if (g_texinfo[f->texinfo].flags & TEX_SPECIAL)
        return;

	// Everyone gets the style zero map.
    f->styles[DEFAULT_MAP] = 0;

	if(g_bumpmaps)
	{
		// Set bumplight styles
		f->styles[BUMP_BASELIGHT_MAP] = BUMP_BASELIGHT_STYLE; 
		f->styles[BUMP_LIGHTVECS_MAP] = BUMP_LIGHTVECS_STYLE;
		f->styles[BUMP_ADDLIGHT_MAP] = BUMP_ADDLIGHT_STYLE;
	}

    memset(&l, 0, sizeof(l));

    l.surfnum = facenum;
    l.face = f;

	VectorCopy (g_translucenttextures[g_texinfo[f->texinfo].miptex], l.translucent_v);
	l.translucent_b = !VectorCompare (l.translucent_v, vec3_origin);

    //
    // rotate plane
    //
    plane = getPlaneFromFace(f);
    VectorCopy(plane->normal, l.facenormal);
    l.facedist = plane->dist;

	if(g_bumpmaps)
	{
		texinfo_t *tx = &g_texinfo[f->texinfo];

		// Build tbn matrix
		VectorCopy(tx->vecs[0], tbnmatrix[0]);
		VectorNormalize(tbnmatrix[0]);

		VectorCopy(tx->vecs[1], tbnmatrix[1]);
		VectorNormalize(tbnmatrix[1]);

		VectorCopy(plane->normal, tbnmatrix[2]);
		VectorNormalize(tbnmatrix[2]);
	}

    CalcFaceVectors(&l);
    CalcFaceExtents(&l);
    CalcPoints(&l);

	CalcLightmap (&l, f->styles);

    lightmapwidth = l.texsize[0] + 1;
    lightmapheight = l.texsize[1] + 1;

    size = lightmapwidth * lightmapheight;
    hlassume(size <= MAX_SINGLEMAP, assume_MAX_SINGLEMAP);

    facelight[facenum].numsamples = l.numsurfpt;

    for (k = 0; k < MAXLIGHTMAPS; k++)
    {
        facelight[facenum].samples[k] = (sample_t*)calloc(l.numsurfpt, sizeof(sample_t));
		hlassume (facelight[facenum].samples[k] != NULL, assume_NoMemory);
    }

    spot = l.surfpt[0];
    for (i = 0; i < l.numsurfpt; i++, spot += 3)
    {
		vec3_t spot_original;
		{
			vec_t s_vec = l.texmins[0] * 16 + (i % lightmapwidth) * TEXTURE_STEP;
			vec_t t_vec = l.texmins[1] * 16 + (i / lightmapwidth) * TEXTURE_STEP;
			SetSurfFromST (&l, spot_original, s_vec, t_vec);
			{
				// adjust sample's offset to 0
				vec_t scale;
				scale = DotProduct (l.texnormal, l.facenormal);
				VectorMA (spot_original, - DEFAULT_HUNT_OFFSET / scale, l.texnormal, spot_original);
			}
		}

        for (k = 0; k < MAXLIGHTMAPS; k++)
        {
            VectorCopy(spot_original, facelight[facenum].samples[k][i].pos);
        }

		int s, t, pos;
		int s_center, t_center, side;
		vec_t sizehalf;
		vec_t weighting, subsamples;
		s_center = (i % lightmapwidth) * l.lmcache_density + l.lmcache_offset;
		t_center = (i / lightmapwidth) * l.lmcache_density + l.lmcache_offset;
		sizehalf = 0.5 * g_blur * l.lmcache_density;
		side = (int)ceil ((sizehalf - 0.5) * (1 - NORMAL_EPSILON));
		subsamples = 0.0;
		for (s = s_center - side; s <= s_center + side; s++)
		{
			for (t = t_center - side; t <= t_center + side; t++)
			{
				if (s < 0 || s >= l.lmcachewidth || t < 0 || t >= l.lmcacheheight)
				{
					// there could be some calculation errors
					continue;
				}
				weighting = (qmin (0.5, sizehalf - (s - s_center)) - qmax (-0.5, -sizehalf - (s - s_center)))
					* (qmin (0.5, sizehalf - (t - t_center)) - qmax (-0.5, -sizehalf - (t - t_center)));
				pos = s + l.lmcachewidth * t;

				for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++)
				{
					VectorMA (facelight[facenum].samples[j][i].light, 
						weighting, 
						l.lmcache[pos][j], 
						facelight[facenum].samples[j][i].light);
				}

				subsamples += weighting;
			}
		}

		if (subsamples > NORMAL_EPSILON)
		{
			for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++)
			{
				VectorScale (facelight[facenum].samples[j][i].light, 1.0 / subsamples, facelight[facenum].samples[j][i].light);

				// Don't do it on the special lightmaps
				if(!g_bumpmaps || j != BUMP_LIGHTVECS_MAP)
					AddSampleToPatch(&facelight[facenum].samples[j][i], facenum, f->styles[j]); //LRC
			}
		}
    } // end of i loop

    // average up the direct light on each patch for radiosity
    {
        for (patch = g_face_patches[facenum]; patch; patch = patch->next)
        {
            //LRC:
			unsigned istyle;
			for (istyle = 0; istyle < MAXLIGHTMAPS && patch->totalstyle[istyle] != 255; istyle++)
			{
				if (patch->samples[istyle])
		        {
		            vec3_t          v;                         // BUGBUG: Use a weighted average instead?

					VectorScale(patch->samplelight[istyle], (1.0f / patch->samples[istyle]), v);
					VectorAdd(patch->totallight[istyle], v, patch->totallight[istyle]);
	                VectorAdd(patch->directlight[istyle], v, patch->directlight[istyle]);
				}
			}
            //LRC (ends)
        }
    }

	for (patch = g_face_patches[facenum]; patch; patch = patch->next)
	{
		// get the PVS for the pos to limit the number of checks
		if (!g_visdatasize)
		{
			memset(pvs, 255, (g_numleafs + 7) / 8);
			lastoffset = -1;
		}
		else
		{
			dleaf_t*        leaf = PointInLeaf(patch->origin);

			thisoffset = leaf->visofs;

			if (patch == g_face_patches[facenum] || thisoffset != lastoffset)
			{
				if (thisoffset == -1)
				{
					memset(pvs, 0, (g_numleafs + 7) / 8);
				}
				else
				{
					DecompressVis(&g_dvisdata[leaf->visofs], pvs, sizeof(pvs));
				}
			}
			lastoffset = thisoffset;
		}

		if (l.translucent_b)
		{
			if (!g_visdatasize)
			{
				memset(pvs2, 255, (g_numleafs + 7) / 8);
				lastoffset2 = -1;
			}
			else
			{
				VectorMA (patch->origin, -(g_translucentdepth+2*PATCH_HUNT_OFFSET), l.facenormal, spot2);
				dleaf_t*        leaf2 = PointInLeaf(spot2);

				thisoffset2 = leaf2->visofs;
				if (l.numsurfpt == 0 || thisoffset2 != lastoffset2)
				{
					if (thisoffset2 == -1)
					{
						memset(pvs2, 0, (g_numleafs + 7) / 8);
					}
					else
					{
						DecompressVis(&g_dvisdata[leaf2->visofs], pvs2, sizeof(pvs2));
					}
				}
				lastoffset2 = thisoffset2;
			}

			vec3_t frontsampled[MAXLIGHTMAPS], backsampled[MAXLIGHTMAPS];
			for (j = 0; j < MAXLIGHTMAPS; j++)
			{
				VectorClear (frontsampled[j]);
				VectorClear (backsampled[j]);
			}
			
			VectorSubtract (vec3_origin, l.facenormal, normal2);
			GatherSampleLight (patch->origin, pvs, l.facenormal, frontsampled, patch->totalstyle, 1);
			GatherSampleLight (spot2, pvs2, normal2, backsampled, patch->totalstyle, 1);

			for (j = 0; j < MAXLIGHTMAPS && (patch->totalstyle[j] != 255); j++)
			{
				for (int x = 0; x < 3; x++)
					patch->totallight[j][x] += (1.0 - l.translucent_v[x]) * frontsampled[j][x] + l.translucent_v[x] * backsampled[j][x];
			}
		}
		else
		{
			GatherSampleLight (patch->origin, pvs, l.facenormal, patch->totallight, patch->totalstyle, 1);
		}
	}

    // add an ambient term if desired
    if (g_ambient[0] || g_ambient[1] || g_ambient[2])
    {
        for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++)
        {
			// Only add it for the base lightmap when bump map is set
            if (!g_bumpmaps || f->styles[j] == 0 || g_bumpmaps && f->styles[j] == BUMP_BASELIGHT_STYLE)
            {
                s = facelight[facenum].samples[j];
                for (i = 0; i < l.numsurfpt; i++, s++)
                    VectorAdd(s->light, g_ambient, s->light);

                break;
            }
        }
    }

    // add circus lighting for finding black lightmaps
    if (g_circus)
    {
        for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++)
        {
            if (f->styles[j] == 0)
            {
                int amt = 7;
                s = facelight[facenum].samples[j];

                while ((l.numsurfpt % amt) == 0)
                    amt--;

                if (amt < 2)
                    amt = 7;

                for (i = 0; i < l.numsurfpt; i++, s++)
                {
                    if ((s->light[0] == 0) && (s->light[1] == 0) && (s->light[2] == 0))
                    {
                        VectorAdd(s->light, s_circuscolors[i % amt], s->light);
                    }
                }
                break;
            }
        }
    }

    // light from dlight_threshold and above is sent out, but the
    // texture itself should still be full bright

    {
        //LRC:
		if (g_face_patches[facenum])
		{
			for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++)
			{
                if (f->styles[j] == g_face_patches[facenum]->emitstyle) //LRC
				{
					break;
				}
			}

			if (j == MAXLIGHTMAPS)
			{
				if (++stylewarningcount >= stylewarningnext)
				{
					stylewarningnext = stylewarningcount * 2;
					Warning("Too many direct light styles on a face(?,?,?)");
					Warning(" total %d warnings for too many styles", stylewarningcount);
				}
			}
			else
			{
				if (f->styles[j] == 255)
				{
					f->styles[j] = g_face_patches[facenum]->emitstyle;
				}

				s = facelight[facenum].samples[j];
				for (i = 0; i < l.numsurfpt; i++, s++)
				{
					VectorAdd(s->light, g_face_patches[facenum]->baselight, s->light);
					
					if(g_bumpmaps && j == DEFAULT_MAP)
					{
						// Add it to the ambient lightmap too if it's the zero lightmap
						sample_t* s2 = facelight[facenum].samples[BUMP_BASELIGHT_MAP];
						VectorAdd(s2->light, g_face_patches[facenum]->baselight, s2->light);
					}
				}
			}
		}
        //LRC (ends)
    }

	if(g_bumpmaps)
	{
		// Get the pointer to the bumpvecs style
        s = facelight[facenum].samples[BUMP_LIGHTVECS_MAP];

        for (i = 0; i < l.numsurfpt; i++, s++)
		{
			if (s->light[0] || s->light[1] || s->light[2])
			{
				vec3_t tmp;
				tmp[0] = DotProduct(tbnmatrix[0], s->light);
				tmp[1] = DotProduct(tbnmatrix[1], s->light);
				tmp[2] = DotProduct(tbnmatrix[2], s->light);
				VectorCopy(tmp, s->light);
			}
			else
			{
				s->light[0] = 0;
				s->light[1] = 0;
				s->light[2] = 0.5;
			}

			s->light[0] = (s->light[0] + 1) * 127;
			s->light[1] = (s->light[1] + 1) * 127;
			s->light[2] = (s->light[2] + 1) * 127;
		}
	}

	free (l.lmcache);
}

// =====================================================================================
//  PrecompLightmapOffsets
// =====================================================================================
void            PrecompLightmapOffsets()
{
    int             facenum;
    dface_t*        f;
    facelight_t*    fl;
    int             lightstyles;

#ifdef ZHLT_TEXLIGHT
    int             i; //LRC
	patch_t*        patch; //LRC
#endif

    g_lightdatasize = 0;
#ifdef ZHLT_XASH
	g_dlitdatasize = 0;
#endif

    for (facenum = 0; facenum < g_numfaces; facenum++)
    {
        f = &g_dfaces[facenum];
        fl = &facelight[facenum];

        if (g_texinfo[f->texinfo].flags & TEX_SPECIAL)
        {
            continue;                                      // non-lit texture
        }

#ifdef ZHLT_TEXLIGHT
        		//LRC - find all the patch lightstyles, and add them to the ones used by this face
#ifdef HLRAD_STYLE_CORING
		for (patch = g_face_patches[facenum]; patch; patch = patch->next)
#else
		patch = g_face_patches[facenum];
		if (patch)
#endif
		{
			for (i = 0; i < MAXLIGHTMAPS && patch->totalstyle[i] != 255; i++)
			{
				for (lightstyles = 0; lightstyles < MAXLIGHTMAPS && f->styles[lightstyles] != 255; lightstyles++)
				{
					if (f->styles[lightstyles] == patch->totalstyle[i])
						break;
				}
				if (lightstyles == MAXLIGHTMAPS)
				{
#ifdef HLRAD_READABLE_EXCEEDSTYLEWARNING
					if (++stylewarningcount >= stylewarningnext)
					{
						stylewarningnext = stylewarningcount * 2;
						Warning("Too many direct light styles on a face(?,?,?)\n");
						Warning(" total %d warnings for too many styles", stylewarningcount);
					}
#else
					Warning("Too many direct light styles on a face(?,?,?)\n");
#endif
				}
				else if (f->styles[lightstyles] == 255)
				{
					f->styles[lightstyles] = patch->totalstyle[i];
//					Log("Face acquires new lightstyle %d at offset %d\n", f->styles[lightstyles], lightstyles);
				}
			}
		}
		//LRC (ends)
#endif

        for (lightstyles = 0; lightstyles < MAXLIGHTMAPS; lightstyles++)
        {
            if (f->styles[lightstyles] == 255)
            {
                break;
            }
        }

        if (!lightstyles)
        {
            continue;
        }

        f->lightofs = g_lightdatasize;
        g_lightdatasize += fl->numsamples * 3 * lightstyles;
		hlassume (g_lightdatasize <= g_max_map_lightdata, assume_MAX_MAP_LIGHTING); //lightdata
#ifdef ZHLT_XASH
		g_dlitdatasize += fl->numsamples * 3 * lightstyles;
		hlassume (g_dlitdatasize < g_max_map_dlitdata, assume_MAX_MAP_LIGHTING);
#endif

    }
}
#ifdef HLRAD_REDUCELIGHTMAP
void ReduceLightmap ()
{
	return;
	byte *oldlightdata = (byte *)malloc (g_lightdatasize);
	hlassume (oldlightdata != NULL, assume_NoMemory);
	memcpy (oldlightdata, g_dlightdata, g_lightdatasize);
#ifdef ZHLT_XASH
	if (g_dlitdatasize != g_lightdatasize)
	{
		Error ("g_dlitdatasize != g_lightdatasize");
	}
	byte *olddlitdata = (byte *)malloc (g_dlitdatasize);
	hlassume (olddlitdata != NULL, assume_NoMemory);
	memcpy (olddlitdata, g_ddlitdata, g_dlitdatasize);
	g_dlitdatasize = 0;
#endif
	g_lightdatasize = 0;

	int facenum;
	for (facenum = 0; facenum < g_numfaces; facenum++)
	{
		dface_t *f = &g_dfaces[facenum];
		facelight_t *fl = &facelight[facenum];
		if (g_texinfo[f->texinfo].flags & TEX_SPECIAL)
		{
			continue;                                      // non-lit texture
		}
#ifdef HLRAD_ENTSTRIPRAD
		// just need to zero the lightmap so that it won't contribute to lightdata size
		if (IntForKey (g_face_entity[facenum], "zhlt_striprad"))
		{
			f->lightofs = g_lightdatasize;
			for (int k = 0; k < MAXLIGHTMAPS; k++)
			{
				f->styles[k] = 255;
			}
			continue;
		}
#endif
#if 0 //debug. --vluzacn
		const char *lightmapcolor = ValueForKey (g_face_entity[facenum], "zhlt_rad");
		if (*lightmapcolor)
		{
			hlassume (MAXLIGHTMAPS == 4, assume_first);
			int styles[4], values[4][3];
			if (sscanf (lightmapcolor, "%d=%d,%d,%d %d=%d,%d,%d %d=%d,%d,%d %d=%d,%d,%d"
					, &styles[0], &values[0][0], &values[0][1], &values[0][2]
					, &styles[1], &values[1][0], &values[1][1], &values[1][2]
					, &styles[2], &values[2][0], &values[2][1], &values[2][2]
					, &styles[3], &values[3][0], &values[3][1], &values[3][2]
				) != 16)
			{
				Error ("Bad value for 'zhlt_rad'.");
			}
			f->lightofs = g_lightdatasize;
			int i, k;
			for (k = 0; k < 4; k++)
			{
				f->styles[k] = 255;
			}
			for (k = 0; k < 4 && styles[k] != 255; k++)
			{
				f->styles[k] = styles[k];
				hlassume (g_lightdatasize + fl->numsamples * 3 <= g_max_map_lightdata, assume_MAX_MAP_LIGHTING);
		#ifdef ZHLT_XASH
				hlassume (g_dlitdatasize + fl->numsamples * 3 <= g_max_map_dlitdata, assume_MAX_MAP_LIGHTING);
		#endif
				for (i = 0; i < fl->numsamples; i++)
				{
					VectorCopy (values[k], (byte *)&g_dlightdata[g_lightdatasize + i * 3]);
		#ifdef ZHLT_XASH
					VectorFill ((byte *)&g_ddlitdata[g_lightdatasize + i * 3], 128);
		#endif
				}
				g_lightdatasize += fl->numsamples * 3;
		#ifdef ZHLT_XASH
				g_dlitdatasize += fl->numsamples * 3;
		#endif
			}
			continue;
		}
#endif
		if (f->lightofs == -1)
		{
			continue;
		}

		int i, k;
		int oldofs;
		unsigned char oldstyles[MAXLIGHTMAPS];
		oldofs = f->lightofs;
		f->lightofs = g_lightdatasize;
		for (k = 0; k < MAXLIGHTMAPS; k++)
		{
			oldstyles[k] = f->styles[k];
			f->styles[k] = 255;
		}
		int numstyles = 0;
		for (k = 0; k < MAXLIGHTMAPS && oldstyles[k] != 255; k++)
		{
			unsigned char maxb = 0;
			for (i = 0; i < fl->numsamples; i++)
			{
				unsigned char *v = &oldlightdata[oldofs + fl->numsamples * 3 * k + i * 3];
				maxb = qmax (maxb, VectorMaximum (v));
			}
			if (maxb <= 0) // black
			{
				continue;
			}
			f->styles[numstyles] = oldstyles[k];
			hlassume (g_lightdatasize + fl->numsamples * 3 * (numstyles + 1) <= g_max_map_lightdata, assume_MAX_MAP_LIGHTING);
			memcpy (&g_dlightdata[f->lightofs + fl->numsamples * 3 * numstyles], &oldlightdata[oldofs + fl->numsamples * 3 * k], fl->numsamples * 3);
#ifdef ZHLT_XASH
			hlassume (g_dlitdatasize + fl->numsamples * 3 * (numstyles + 1) <= g_max_map_dlitdata, assume_MAX_MAP_LIGHTING);
			memcpy (&g_ddlitdata[f->lightofs + fl->numsamples * 3 * numstyles], &olddlitdata[oldofs + fl->numsamples * 3 * k], fl->numsamples * 3);
#endif
			numstyles++;
		}
		g_lightdatasize += fl->numsamples * 3 * numstyles;
#ifdef ZHLT_XASH
		g_dlitdatasize += fl->numsamples * 3 * numstyles;
#endif
	}
	free (oldlightdata);
#ifdef ZHLT_XASH
	free (olddlitdata);
#endif
}
#endif

#ifdef HLRAD_MDL_LIGHT_HACK

// Change the sample light right under a mdl file entity's origin.
// Use this when "mdl" in shadow has incorrect brightness.

const int MLH_MAXFACECOUNT = 16;
const int MLH_MAXSAMPLECOUNT = 4;
const vec_t MLH_LEFT = 0;
const vec_t MLH_RIGHT = 1;

typedef struct
{
	vec3_t origin;
	vec3_t floor;
	struct
	{
		int num;
		struct
		{
			bool exist;
			int seq;
		}
		style[ALLSTYLES];
		struct
		{
			int num;
			vec3_t pos;
			unsigned char* (style[ALLSTYLES]);
		}
		sample[MLH_MAXSAMPLECOUNT];
		int samplecount;
	}
	face[MLH_MAXFACECOUNT];
	int facecount;
} mdllight_t;

#ifdef HLRAD_MDL_LIGHT_HACK_NEW
int MLH_AddFace (mdllight_t *ml, int facenum)
{
	dface_t *f = &g_dfaces[facenum];
	int i, j;
	for (i = 0; i < ml->facecount; i++)
	{
		if (ml->face[i].num == facenum)
		{
			return -1;
		}
	}
	if (ml->facecount >= MLH_MAXFACECOUNT)
	{
		return -1;
	}
	i = ml->facecount;
	ml->facecount++;
	ml->face[i].num = facenum;
	ml->face[i].samplecount = 0;
	for (j = 0; j < ALLSTYLES; j++)
	{
		ml->face[i].style[j].exist = false;
	}
	for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++)
	{
		ml->face[i].style[f->styles[j]].exist = true;
		ml->face[i].style[f->styles[j]].seq = j;
	}
	return i;
}
void MLH_AddSample (mdllight_t *ml, int facenum, int w, int h, int s, int t, const vec3_t pos)
{
	dface_t *f = &g_dfaces[facenum];
	int i, j;
	int r = MLH_AddFace (ml, facenum);
	if (r == -1)
	{
		return;
	}
	int size = w * h;
	int num = s + w * t;
	for (i = 0; i < ml->face[r].samplecount; i++)
	{
		if (ml->face[r].sample[i].num == num)
		{
			return;
		}
	}
	if (ml->face[r].samplecount >= MLH_MAXSAMPLECOUNT)
	{
		return;
	}
	i = ml->face[r].samplecount;
	ml->face[r].samplecount++;
	ml->face[r].sample[i].num = num;
	VectorCopy (pos, ml->face[r].sample[i].pos);
	for (j = 0; j < ALLSTYLES; j++)
	{
		if (ml->face[r].style[j].exist)
		{
			ml->face[r].sample[i].style[j] = &g_dlightdata[f->lightofs + (num + size * ml->face[r].style[j].seq) * 3];
		}
	}
}
void MLH_CalcExtents (const dface_t *f, int *texturemins, int *extents)
{
	float mins[2], maxs[2];
	int bmins[2], bmaxs[2];
	texinfo_t *tex;
	tex = &g_texinfo[f->texinfo];
	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;
	int i;
	for (i = 0; i < f->numedges; i++)
	{
		int e;
		dvertex_t *v;
		int j;
		e = g_dsurfedges[f->firstedge + i];
		if (e >= 0)
		{
			v = &g_dvertexes[g_dedges[e].v[0]];
		}
		else
		{
			v = &g_dvertexes[g_dedges[-e].v[1]];
		}
		for (j = 0; j < 2; j++)
		{
			float val = v->point[0] * tex->vecs[j][0] + v->point[1] * tex->vecs[j][1]
				+ v->point[2] * tex->vecs[j][2] + tex->vecs[j][3];
			if (val < mins[j])
			{
				mins[j] = val;
			}
			if (val > maxs[j])
			{
				maxs[j] = val;
			}
		}
	}
	for (i = 0; i < 2; i++)
	{
		bmins[i] = floor (mins[i] / 16);
		bmaxs[i] = ceil (maxs[i] / 16);
		texturemins[i] = bmins[i] * 16;
		extents[i] = (bmaxs[i] - bmins[i]) * 16;
	}
}
void MLH_GetSamples_r (mdllight_t *ml, int nodenum, const float *start, const float *end)
{
	if (nodenum < 0)
		return;
	dnode_t *node = &g_dnodes[nodenum];
	dplane_t *plane;
	float front, back, frac;
	float mid[3];
	int side;
	plane = &g_dplanes[node->planenum];
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	if ((back < 0) == side)
	{
		MLH_GetSamples_r (ml, node->children[side], start, end);
		return;
	}
	frac = front / (front - back);
	mid[0] = start[0] + (end[0] - start[0]) * frac;
	mid[1] = start[1] + (end[1] - start[1]) * frac;
	mid[2] = start[2] + (end[2] - start[2]) * frac;
	MLH_GetSamples_r (ml, node->children[side], start, mid);
	if (ml->facecount > 0)
	{
		return;
	}
	{
		int i;
		for (i = 0; i < node->numfaces; i++)
		{
			dface_t *f = &g_dfaces[node->firstface + i];
			texinfo_t *tex = &g_texinfo[f->texinfo];
			const char *texname = GetTextureByNumber (f->texinfo);
			if (!strncmp (texname, "sky", 3))
			{
				continue;
			}
			if (f->lightofs == -1)
			{
				continue;
			}
			int s = DotProduct (mid, tex->vecs[0]) + tex->vecs[0][3];
			int t = DotProduct (mid, tex->vecs[1]) + tex->vecs[1][3];
			int texturemins[2], extents[2];
			MLH_CalcExtents (f, texturemins, extents);
			if (s < texturemins[0] || t < texturemins[1])
			{
				continue;
			}
			int ds = s - texturemins[0];
			int dt = t - texturemins[1];
			if (ds > extents[0] || dt > extents[1])
			{
				continue;
			}
			ds >>= 4;
			dt >>= 4;
			MLH_AddSample (ml, node->firstface + i, extents[0] / 16 + 1, extents[1] / 16 + 1, ds, dt, mid);
			break;
		}
	}
	if (ml->facecount > 0)
	{
		VectorCopy (mid, ml->floor);
		return;
	}
	MLH_GetSamples_r (ml, node->children[!side], mid, end);
}
void MLH_mdllightCreate (mdllight_t *ml)
{
	// code from Quake
	float p[3];
	float end[3];
	ml->facecount = 0;
	VectorCopy (ml->origin, ml->floor);
	VectorCopy (ml->origin, p);
	VectorCopy (ml->origin, end);
	end[2] -= 2048;
	MLH_GetSamples_r (ml, 0, p, end);
}
#else
void MLH_mdllightCreate (mdllight_t *ml)
{
	int i, j, k;
	vec_t height, minheight = BOGUS_RANGE;
	ml->facecount = 0;
	for (i = 0; i < g_numfaces; ++i)
	{
		if (stricmp (ValueForKey (g_face_entity[i], "classname"), "worldspawn"))
			continue;
		const dface_t *f = &g_dfaces[i];
		const dplane_t *p = getPlaneFromFace(f);
		Winding *w=new Winding (*f);
		for (j = 0;j < w->m_NumPoints; j++)
		{
			VectorAdd(w->m_Points[j], g_face_offset[i], w->m_Points[j]);
		}
		vec3_t delta , sect;
		VectorCopy (ml->origin, delta);
		delta[2] -= BOGUS_RANGE;
		if (intersect_linesegment_plane(p, ml->origin, delta, sect) && point_in_winding (*w, *p, sect))
		{
			height = ml->origin[2] - sect[2];
			if (height >= 0 && height <= minheight)
				minheight = height;
		}
		delete w;
	}
	VectorCopy (ml->origin, ml->floor);
	ml->floor[2] -= minheight;
	for (i = 0; i < g_numfaces; ++i)
	{
		if (stricmp (ValueForKey (g_face_entity[i], "classname"), "worldspawn"))
			continue;
		const dface_t *f = &g_dfaces[i];
		const dplane_t *p = getPlaneFromFace(f);
		Winding *w=new Winding (*f);
		if (g_texinfo[f->texinfo].flags & TEX_SPECIAL)
		{
			continue;                                            // non-lit texture
		}
		for (j = 0;j < w->m_NumPoints; j++)
		{
			VectorAdd(w->m_Points[j], g_face_offset[i], w->m_Points[j]);
		}
		vec3_t delta , sect;
		VectorCopy (ml->origin, delta);
		delta[2] -= BOGUS_RANGE;
		if (intersect_linesegment_plane(p, ml->origin, delta, sect) && VectorCompare (sect, ml->floor))
		{
			bool inlightmap = false;
			{
				vec3_t v;
				facesampleinfo_t *info = &facesampleinfo[i];
				int w = info->texsize[0] + 1;
				int h = info->texsize[1] + 1;
				vec_t vs, vt;
				int s1, s2, t1, t2, s, t;
				VectorCopy (ml->floor, v);
				VectorSubtract (v, info->offset, v);
				VectorSubtract (v, info->texorg, v);
				vs = DotProduct (v, info->worldtotex[0]);
				vt = DotProduct (v, info->worldtotex[1]);
				s1 = (int)floor((vs-MLH_LEFT)/16) - info->texmins[0];
				s2 = (int)floor((vs+MLH_RIGHT)/16) - info->texmins[0];
				t1 = (int)floor((vt-MLH_LEFT)/16) - info->texmins[1];
				t2 = (int)floor((vt+MLH_RIGHT)/16) - info->texmins[1];
				for (s=s1; s<=s2; ++s)
					for (t=t1; t<=t2; ++t)
						if (s>=0 && s<w && t>=0 && t<h)
							inlightmap = true;
			}
			if (inlightmap && ml->facecount < MLH_MAXFACECOUNT)
			{
				ml->face[ml->facecount].num = i;
				ml->facecount++;
			}
		}
		delete w;
	}
	for (i = 0; i < ml->facecount; ++i)
	{
		const dface_t *f = &g_dfaces[ml->face[i].num];
		for (j = 0; j < ALLSTYLES; ++j)
			ml->face[i].style[j].exist = false;
		for (j = 0; j < MAXLIGHTMAPS && f->styles[j] != 255; ++j)
		{
			ml->face[i].style[f->styles[j]].exist = true;
			ml->face[i].style[f->styles[j]].seq = j;
		}
		ml->face[i].samplecount = 0;
		if (j == 0)
			continue;

	    const facelight_t *fl=&facelight[ml->face[i].num];
		{
			vec3_t v;
			facesampleinfo_t *info = &facesampleinfo[ml->face[i].num];
			int w = info->texsize[0] + 1;
			int h = info->texsize[1] + 1;
			vec_t vs, vt;
			int s1, s2, t1, t2, s, t;
			VectorCopy (ml->floor, v);
			VectorSubtract (v, info->offset, v);
			VectorSubtract (v, info->texorg, v);
			vs = DotProduct (v, info->worldtotex[0]);
			vt = DotProduct (v, info->worldtotex[1]);
			s1 = (int)floor((vs-MLH_LEFT)/16) - info->texmins[0];
			s2 = (int)floor((vs+MLH_RIGHT)/16) - info->texmins[0];
			t1 = (int)floor((vt-MLH_LEFT)/16) - info->texmins[1];
			t2 = (int)floor((vt+MLH_RIGHT)/16) - info->texmins[1];
			for (s=s1; s<=s2; ++s)
				for (t=t1; t<=t2; ++t)
					if (s>=0 && s<w && t>=0 && t<h)
						if (ml->face[i].samplecount < MLH_MAXSAMPLECOUNT)
						{
							ml->face[i].sample[ml->face[i].samplecount].num = s + t * w;
							VectorAdd (info->offset, info->texorg, v);
							vs = 16.0 * (s + info->texmins[0]);
							vt = 16.0 * (t + info->texmins[1]);
							VectorMA (v, vs, info->textoworld[0], v);
							VectorMA (v, vt, info->textoworld[1], v);
							VectorCopy (v, ml->face[i].sample[ml->face[i].samplecount].pos);
							ml->face[i].samplecount++;
						}
		}

		for (j = 0; j < ml->face[i].samplecount; ++j)
		{
			for (k = 0; k < ALLSTYLES; ++k)
				if (ml->face[i].style[k].exist)
				{
					ml->face[i].sample[j].style[k] = 
						&g_dlightdata[f->lightofs + ml->face[i].style[k].seq * fl->numsamples * 3 + ml->face[i].sample[j].num * 3];
				}
		}
	}
}
#endif

int MLH_CopyLight (const vec3_t from, const vec3_t to)
{
	int i, j, k, count = 0;
	mdllight_t mlfrom, mlto;
	VectorCopy (from, mlfrom.origin);
	VectorCopy (to, mlto.origin);
	MLH_mdllightCreate (&mlfrom);
	MLH_mdllightCreate (&mlto);
	if (mlfrom.facecount == 0 || mlfrom.face[0].samplecount == 0)
		return -1;
	for (i = 0; i < mlto.facecount; ++i)
		for (j = 0; j < mlto.face[i].samplecount; ++j, ++count)
			for (k = 0; k < ALLSTYLES; ++k)
				if (mlto.face[i].style[k].exist && mlfrom.face[0].style[k].exist)
				{
					VectorCopy (mlfrom.face[0].sample[0].style[k],mlto.face[i].sample[j].style[k]);
					Developer (DEVELOPER_LEVEL_SPAM, "Mdl Light Hack: face (%d) sample (%d) style (%d) position (%f,%f,%f)\n",
						mlto.face[i].num, mlto.face[i].sample[j].num, k, 
						mlto.face[i].sample[j].pos[0], mlto.face[i].sample[j].pos[1], mlto.face[i].sample[j].pos[2]);
				}
	Developer (DEVELOPER_LEVEL_MESSAGE, "Mdl Light Hack: %d sample light copied from (%f,%f,%f) to (%f,%f,%f)\n", 
		count, mlfrom.floor[0], mlfrom.floor[1], mlfrom.floor[2], mlto.floor[0], mlto.floor[1], mlto.floor[2]);
	return count;
}

void MdlLightHack ()
{
	int ient;
	entity_t *ent1, *ent2;
	vec3_t origin1, origin2;
	const char *target;
#ifndef HLRAD_MDL_LIGHT_HACK_NEW
    double start, end;
#endif
	int used = 0, countent = 0, countsample = 0, r;
#ifndef HLRAD_MDL_LIGHT_HACK_NEW
    start = I_FloatTime();
#endif
	for (ient = 0; ient < g_numentities; ++ient)
	{
		ent1 = &g_entities[ient];
		target = ValueForKey (ent1, "zhlt_copylight");
		if (!strcmp (target, ""))
			continue;
		used = 1;
		ent2 = FindTargetEntity (target);
		if (ent2 == NULL)
		{
			Warning ("target entity '%s' not found", target);
			continue;
		}
		GetVectorForKey (ent1, "origin", origin1);
		GetVectorForKey (ent2, "origin", origin2);
		r = MLH_CopyLight (origin2, origin1);
		if (r < 0)
			Warning ("can not copy light from (%f,%f,%f)", origin2[0], origin2[1], origin2[2]);
		else
		{
			countent += 1;
			countsample += r;
		}
	}
#ifndef HLRAD_MDL_LIGHT_HACK_NEW
    end = I_FloatTime();
#endif
	if (used)
#ifdef HLRAD_MDL_LIGHT_HACK_NEW
		Log ("Adjust mdl light: modified %d samples for %d entities\n", countsample, countent);
#else
		Log("Mdl Light Hack: %d entities %d samples (%.2f seconds)\n", countent, countsample, end - start);
#endif
}
#endif /*HLRAD_MDL_LIGHT_HACK*/

// =====================================================================================
//  FinalLightFace
//      Add the indirect lighting on top of the direct lighting and save into final map format
// =====================================================================================
void            FinalLightFace(const int facenum)
{
    int             i, j, k;
    vec3_t          lb, v;
    facelight_t*    fl;
    sample_t*       samp;
    float           minlight;
    int             lightstyles;
    dface_t*        f;
    lerpTriangulation_t* trian = NULL;

	vec3_t			*original_basiclight;
	int				(*final_basiclight)[3];
	int				lbi[3];

    // ------------------------------------------------------------------------
    // Changes by Adam Foster - afoster@compsoc.man.ac.uk
    float           temp_rand;

    // ------------------------------------------------------------------------

    f = &g_dfaces[facenum];
    fl = &facelight[facenum];

    if (g_texinfo[f->texinfo].flags & TEX_SPECIAL)
    {
        return;                                            // non-lit texture
    }

	if (IntForKey (g_face_entity[facenum], "zhlt_striprad"))
	{
		return;
	}

    for (lightstyles = 0; lightstyles < MAXLIGHTMAPS; lightstyles++)
    {
        if (f->styles[lightstyles] == 255)
        {
            break;
        }
    }

    if (!lightstyles)
    {
        return;
    }

    //
    // set up the triangulation
    //
    {
        trian = CreateTriangulation(facenum);
    }

    //
    // sample the triangulation
    //
	minlight = 0;
	if(!g_nightmode || IntForKey(g_face_entity[facenum], "_ignorenight") != 0)
	{
		minlight = FloatForKey(g_face_entity[facenum], "_minlight") * 128;
	}

	original_basiclight = (vec3_t *)calloc (fl->numsamples, sizeof(vec3_t));
	final_basiclight = (int (*)[3])calloc (fl->numsamples, sizeof(int [3]));
	hlassume (original_basiclight != NULL, assume_NoMemory);
	hlassume (final_basiclight != NULL, assume_NoMemory);

    for (k = 0; k < lightstyles; k++)
    {
        samp = fl->samples[k];
        for (j = 0; j < fl->numsamples; j++, samp++)
        {
			if(g_bumpmaps && k == BUMP_LIGHTVECS_MAP)
			{
				// Just copy the light vector straight
				VectorCopy(samp->light, lbi);
			}
			else
			{
				// Should be a VectorCopy, but we scale by 2 to compensate for an earlier lighting flaw
				// Specifically, the directlight contribution was included in the bounced light AND the directlight
				// Since many of the levels were built with this assumption, this "fudge factor" compensates for it.

				// Default direct_scale has been changed from 2 to 1 and default scale has been changed from 1 to 2. --vluzacn
				VectorScale(samp->light, g_direct_scale, lb);

				{
					SampleTriangulation(trian, samp->pos, v, f->styles[k]); //LRC

					if (isPointFinite(v))
					{
						VectorAdd(lb, v, lb);
					}
					else
					{
						Warning("point (%4.3f %4.3f %4.3f) infinite v (%4.3f %4.3f %4.3f)\n",
								samp->pos[0], samp->pos[1], samp->pos[2], v[0], v[1], v[2]);
					}
				}

				if (f->styles[0] != 0)
				{
					Warning ("wrong f->styles[0]");
				}

				VectorCompareMaximum (lb, vec3_origin, lb);
				if(!g_bumpmaps)
				{
					if (k == 0)
					{
						VectorCopy (lb, original_basiclight[j]);
					}
					else
					{
						VectorAdd (lb, original_basiclight[j], lb);
					}
				}

				// ------------------------------------------------------------------------
				// Changes by Adam Foster - afoster@compsoc.man.ac.uk
				// colour lightscale:
				lb[0] *= g_colour_lightscale[0];
				lb[1] *= g_colour_lightscale[1];
				lb[2] *= g_colour_lightscale[2];

				// ------------------------------------------------------------------------

				// clip from the bottom first
				for (i = 0; i < 3; i++)
				{
					if (lb[i] < minlight)
					{
						lb[i] = minlight;
					}
				}

				// ------------------------------------------------------------------------
				// Changes by Adam Foster - afoster@compsoc.man.ac.uk

				// AJM: your code is formatted really wierd, and i cant understand a damn thing. 
				//      so i reformatted it into a somewhat readable "normal" fashion. :P

				if ( g_colour_qgamma[0] != 1.0 ) 
					lb[0] = (float) pow(lb[0] / 256.0f, g_colour_qgamma[0]) * 256.0f;

				if ( g_colour_qgamma[1] != 1.0 ) 
					lb[1] = (float) pow(lb[1] / 256.0f, g_colour_qgamma[1]) * 256.0f;

				if ( g_colour_qgamma[2] != 1.0 ) 
					lb[2] = (float) pow(lb[2] / 256.0f, g_colour_qgamma[2]) * 256.0f;

				// Two different ways of adding noise to the lightmap - colour jitter
				// (red, green and blue channels are independent), and mono jitter
				// (monochromatic noise). For simulating dithering, on the cheap. :)

				// Tends to create seams between adjacent polygons, so not ideal.

				// Got really weird results when it was set to limit values to 256.0f - it
				// was as if r, g or b could wrap, going close to zero.

				// scale the brightness after adjusting gamma
				for (i = 0; i < 3; i++)
				{
					lb[i] *= pow (g_texturebrightness[g_texinfo[f->texinfo].miptex][i], (vec_t)(1 / LIGHTMAPTOTEXTURE_GAMMA));
				}

				for (i = 0; i < 3; ++i)
					if (lb[i] < g_minlight)
						lb[i] = g_minlight;

				// ------------------------------------------------------------------------
				for (i = 0; i < 3; ++i)
				{
					lbi[i] = (int) floor (lb[i] + 0.5);
					if (lbi[i] < 0) lbi[i] = 0;
				}

				if(!g_bumpmaps)
				{
					if (k == 0)
					{
						VectorCopy (lbi, final_basiclight[j]);
					}
					else
					{
						VectorSubtract (lbi, final_basiclight[j], lbi);
					}
				}

				if (k == 0)
				{
					if (g_colour_jitter_hack[0] || g_colour_jitter_hack[1] || g_colour_jitter_hack[2]) 
						for (i = 0; i < 3; i++) 
							lbi[i] += g_colour_jitter_hack[i] * ((float)rand() / RAND_MAX - 0.5);
					if (g_jitter_hack[0] || g_jitter_hack[1] || g_jitter_hack[2]) 
					{
						temp_rand = (float)rand() / RAND_MAX - 0.5;
						for (i = 0; i < 3; i++) 
							lbi[i] += g_jitter_hack[i] * temp_rand;
					}
				}
			}

			for (i = 0; i < 3; ++i)
			{
				if (lbi[i] < 0) lbi[i] = 0;
				if (lbi[i] > 255) lbi[i] = 255;
			}

            {
                unsigned char* colors = &g_dlightdata[f->lightofs + k * fl->numsamples * 3 + j * 3];

                colors[0] = (unsigned char)lbi[0];
                colors[1] = (unsigned char)lbi[1];
                colors[2] = (unsigned char)lbi[2];
            }
        }
    }

	free (original_basiclight);
	free (final_basiclight);
	FreeTriangulation(trian);
}


#ifdef ZHLT_TEXLIGHT
//LRC
vec3_t    totallight_default = { 0, 0, 0 };
#ifdef ZHLT_XASH
vec3_t    totallight_default_direction = { 0, 0, 0 };
#endif

//LRC - utility for getting the right totallight value from a patch
vec3_t* GetTotalLight(patch_t* patch, int style
#ifdef ZHLT_XASH
	, const vec3_t *&direction_out
#endif
	)
{
	int i;
	for (i = 0; i < MAXLIGHTMAPS && patch->totalstyle[i] != 255; i++)
	{
		if (patch->totalstyle[i] == style)
		{
#ifdef ZHLT_XASH
			direction_out = &(patch->totallight_direction[i]);
#endif
			return &(patch->totallight[i]);
		}
	}
#ifdef ZHLT_XASH
	direction_out = &totallight_default_direction;
#endif
	return &totallight_default;
}

extern bool g_bumpmaps;

// =====================================================================================
//
// =====================================================================================
bool ExportALDData( ald_datatype_t type )
{
	aldheader_t* poriginal = nullptr;

	// See if a file already exists
	char szpath[_MAX_PATH];
	strcpy(szpath, g_Mapname);
	strcat(szpath, ".ald");

	FILE* pfin = fopen(szpath, "rb");
	if(pfin)
	{
		fseek(pfin, 0, SEEK_END);
		int filesize = ftell(pfin);
		fseek(pfin, 0, SEEK_SET);

		byte* pbuffer = new byte[filesize];
		fread(pbuffer, filesize, 1, pfin);
		fclose(pfin);

		poriginal = (aldheader_t*)pbuffer;
	}

	// Create new lump
	aldlumptype_t lumptype;
	switch(type)
	{
	case ALD_DATA_DAYLIGHT_RETURN:
		{
			lumptype = g_bumpmaps ? ALD_LUMP_DAYLIGHT_RETURN_DATA_BUMP : ALD_LUMP_DAYLIGHT_RETURN_DATA_NOBUMP;
		}
		break;
	case ALD_DATA_NIGHTSTAGE:
	default:
		{
			lumptype = g_bumpmaps ? ALD_LUMP_NIGHTDATA_BUMP : ALD_LUMP_NIGHTDATA_NOBUMP;
		}
		break;
	}

	// Create file based on original if present
	int newnumlumps = 0;
	int totalsize = 0;

	if(poriginal)
	{
		totalsize = sizeof(aldheader_t);

		for(int i = 0; i < poriginal->numlumps; i++)
		{
			aldlump_t* plump = (aldlump_t*)((byte*)poriginal + poriginal->lumpoffset) + i;
			if(plump->type != lumptype)
			{
				totalsize += plump->lumpsize + sizeof(aldlump_t);
				newnumlumps++;
			}
		}

		totalsize += sizeof(aldlump_t) + sizeof(byte)*g_lightdatasize;
		newnumlumps++;
	}
	else
	{
		// No original file
		totalsize = sizeof(aldheader_t) + sizeof(aldlump_t) + sizeof(byte)*g_lightdatasize;
		newnumlumps = 1;
	}

	// Create buffer
	int fileoffset = 0;
	byte* pfilebuffer = new byte[totalsize];

	aldheader_t* pnewhdr = (aldheader_t*)pfilebuffer;
	fileoffset += sizeof(aldheader_t);

	pnewhdr->header = ALD_HEADER_ENCODED;
	pnewhdr->numlumps = newnumlumps;
	pnewhdr->lumpoffset = fileoffset;

	aldlump_t* pnewlump = nullptr;
	if(poriginal)
	{
		aldlump_t* pnewlumps = (aldlump_t*)(pfilebuffer + fileoffset);
		fileoffset += sizeof(aldlump_t)*newnumlumps;

		int j = 0;
		for(int i = 0; i < poriginal->numlumps; i++)
		{
			aldlump_t* psrclump = (aldlump_t*)((byte*)poriginal + poriginal->lumpoffset) + i;
			if(psrclump->type == lumptype)
				continue;

			pnewlumps[j].type = psrclump->type;
			pnewlumps[j].lumpoffset = fileoffset;
			pnewlumps[j].lumpsize = psrclump->lumpsize;
			fileoffset += psrclump->lumpsize;

			// copy data
			byte* psrclumpdata = ((byte*)poriginal + psrclump->lumpoffset);
			byte* pdestlumpdata = pfilebuffer + pnewlumps[j].lumpoffset;

			memcpy(pdestlumpdata, psrclumpdata, psrclump->lumpsize);

			j++;
		}

		pnewlump = &pnewlumps[j];
		j++;
	}
	else
	{
		pnewlump = (aldlump_t*)(pfilebuffer + fileoffset);
		fileoffset += sizeof(aldlump_t)*newnumlumps;
	}

	pnewlump->type = lumptype;
	pnewlump->lumpoffset = fileoffset;
	pnewlump->lumpsize = g_lightdatasize;

	// Copy data to destination
	byte* pdestdata = pfilebuffer + pnewlump->lumpoffset;
	memcpy(pdestdata, g_dlightdata, g_lightdatasize);
	fileoffset += g_lightdatasize;

	if(fileoffset != totalsize)
	{
		Log("Data size mismatch(%d != %d) when writing ALD data file '%s'.\n", fileoffset, totalsize, szpath);
		delete[] pfilebuffer;
		return false;
	}

	// write the file
	FILE* pf = fopen(szpath, "wb");
	if(!pf)
	{
		Log("Failed to open '%s' for writing.\n", szpath);
		delete[] pfilebuffer;
		return false;
	}

	fwrite(pfilebuffer, sizeof(byte)*totalsize, 1, pf);
	fclose(pf);

	delete[] pfilebuffer;

	Log("%s written(%d bytes).\n", szpath, totalsize);
	return true;
}
#endif
