#include "qrad.h"

#pragma warning(disable: 4018)

static dplane_t backplanes[MAX_MAP_PLANES];

#ifdef HLRAD_HuntForWorld_EDGE_FIX
dleaf_t*		PointInLeaf_Worst_r(int nodenum, const vec3_t point)
{
	vec_t			dist;
	dnode_t*		node;
	dplane_t*		plane;

	while (nodenum >= 0)
	{
		node = &g_dnodes[nodenum];
		plane = &g_dplanes[node->planenum];
		dist = DotProduct(point, plane->normal) - plane->dist;
		if (dist > HUNT_WALL_EPSILON)
		{
			nodenum = node->children[0];
		}
		else if (dist < -HUNT_WALL_EPSILON)
		{
			nodenum = node->children[1];
		}
		else
		{
			dleaf_t* result[2];
			result[0] = PointInLeaf_Worst_r(node->children[0], point);
			result[1] = PointInLeaf_Worst_r(node->children[1], point);
			if (result[0] == g_dleafs || result[0]->contents == CONTENTS_SOLID)
				return result[0];
			if (result[1] == g_dleafs || result[1]->contents == CONTENTS_SOLID)
				return result[1];
			if (result[0]->contents == CONTENTS_SKY)
				return result[0];
			if (result[1]->contents == CONTENTS_SKY)
				return result[1];
#ifdef HLRAD_WATERBLOCKLIGHT
			if (result[0]->contents == result[1]->contents)
				return result[0];
			return g_dleafs;
#else
			return result[0];
#endif
		}
	}

	return &g_dleafs[-nodenum - 1];
}
dleaf_t*		PointInLeaf_Worst(const vec3_t point)
{
	return PointInLeaf_Worst_r(0, point);
}
#endif
dleaf_t*        PointInLeaf(const vec3_t point)
{
    int             nodenum;
    vec_t           dist;
    dnode_t*        node;
    dplane_t*       plane;

    nodenum = 0;
    while (nodenum >= 0)
    {
        node = &g_dnodes[nodenum];
        plane = &g_dplanes[node->planenum];
        dist = DotProduct(point, plane->normal) - plane->dist;
        if (dist >= 0.0)
        {
            nodenum = node->children[0];
        }
        else
        {
            nodenum = node->children[1];
        }
    }

    return &g_dleafs[-nodenum - 1];
}

/*
 * ==============
 * PatchPlaneDist
 * Fixes up patch planes for brush models with an origin brush
 * ==============
 */
vec_t           PatchPlaneDist(const patch_t* const patch)
{
    const dplane_t* plane = getPlaneFromFaceNumber(patch->faceNumber);

    return plane->dist + DotProduct(g_face_offset[patch->faceNumber], plane->normal);
}

void            MakeBackplanes()
{
    int             i;

    for (i = 0; i < g_numplanes; i++)
    {
        backplanes[i].dist = -g_dplanes[i].dist;
        VectorSubtract(vec3_origin, g_dplanes[i].normal, backplanes[i].normal);
    }
}

const dplane_t* getPlaneFromFace(const dface_t* const face)
{
    if (!face)
    {
        Error("getPlaneFromFace() face was NULL\n");
    }

    if (face->side)
    {
        return &backplanes[face->planenum];
    }
    else
    {
        return &g_dplanes[face->planenum];
    }
}

const dplane_t* getPlaneFromFaceNumber(const unsigned int faceNumber)
{
    dface_t*        face = &g_dfaces[faceNumber];

    if (face->side)
    {
        return &backplanes[face->planenum];
    }
    else
    {
        return &g_dplanes[face->planenum];
    }
}

// Returns plane adjusted for face offset (for origin brushes, primarily used in the opaque code)
void getAdjustedPlaneFromFaceNumber(unsigned int faceNumber, dplane_t* plane)
{
    dface_t*        face = &g_dfaces[faceNumber];
    const vec_t*    face_offset = g_face_offset[faceNumber];

    plane->type = (planetypes)0;
    
    if (face->side)
    {
        vec_t dist;

        VectorCopy(backplanes[face->planenum].normal, plane->normal);
        dist = DotProduct(plane->normal, face_offset);
        plane->dist = backplanes[face->planenum].dist + dist;
    }
    else
    {
        vec_t dist;

        VectorCopy(g_dplanes[face->planenum].normal, plane->normal);
        dist = DotProduct(plane->normal, face_offset);
        plane->dist = g_dplanes[face->planenum].dist + dist;
    }
}

// Will modify the plane with the new dist
void            TranslatePlane(dplane_t* plane, const vec_t* delta)
{
#ifdef HLRAD_MATH_VL
	plane->dist += DotProduct (plane->normal, delta);
#else
    vec3_t          proj;
    vec_t           magnitude;

    ProjectionPoint(delta, plane->normal, proj);
    magnitude = VectorLength(proj);

    if (DotProduct(plane->normal, delta) > 0)              //if zero, magnitude will be zero.
    {
        plane->dist += magnitude;
    }
    else
    {
        plane->dist -= magnitude;
    }
#endif
}

// HuntForWorld will never return CONTENTS_SKY or CONTENTS_SOLID leafs
dleaf_t*        HuntForWorld(vec_t* point, const vec_t* plane_offset, const dplane_t* plane, int hunt_size, vec_t hunt_scale, vec_t hunt_offset)
{
    dleaf_t*        leaf;
    int             x, y, z;
    int             a;

    vec3_t          current_point;
    vec3_t          original_point;

    vec3_t          best_point;
    dleaf_t*        best_leaf = NULL;
    vec_t           best_dist = 99999999.0;

    vec3_t          scales;

    dplane_t        new_plane = *plane;

#ifndef HLRAD_HuntForWorld_FIX
    if (hunt_scale < 0.1)
    {
        hunt_scale = 0.1;
    }
#endif

    scales[0] = 0.0;
    scales[1] = -hunt_scale;
    scales[2] = hunt_scale;

    VectorCopy(point, best_point);
    VectorCopy(point, original_point);

    TranslatePlane(&new_plane, plane_offset);

#ifndef HLRAD_HuntForWorld_FIX
    if (!hunt_size)
    {
        hunt_size = DEFAULT_HUNT_SIZE;
    }
#endif

#ifdef HLRAD_HuntForWorld_FIX
	for (a = 0; a < hunt_size; a++)
#else
    for (a = 1; a < hunt_size; a++)
#endif
    {
        for (x = 0; x < 3; x++)
        {
            current_point[0] = original_point[0] + (scales[x % 3] * a);
            for (y = 0; y < 3; y++)
            {
                current_point[1] = original_point[1] + (scales[y % 3] * a);
                for (z = 0; z < 3; z++)
                {
#ifdef HLRAD_HuntForWorld_FIX
					if (a == 0)
					{
						if (x || y || z)
							continue;
					}
#endif
                    vec3_t          delta;
                    vec_t           dist;

                    current_point[2] = original_point[2] + (scales[z % 3] * a);

                    SnapToPlane(&new_plane, current_point, hunt_offset);
                    VectorSubtract(current_point, original_point, delta);
#ifdef HLRAD_MATH_VL
                    dist = DotProduct(delta, delta);
#else
                    dist = VectorLength(delta);
#endif

#ifdef HLRAD_OPAQUE_BLOCK
					{
						int x;
						for (x = 0; x < g_opaque_face_count; x++)
						{
							if (TestPointOpaque (g_opaque_face_list[x].modelnum, g_opaque_face_list[x].origin, g_opaque_face_list[x].block, current_point))
								break;
						}
						if (x < g_opaque_face_count)
							continue;
					}
#endif
                    if (dist < best_dist)
                    {
#ifdef HLRAD_HuntForWorld_EDGE_FIX
                        if ((leaf = PointInLeaf_Worst(current_point)) != g_dleafs)
#else
                        if ((leaf = PointInLeaf(current_point)) != g_dleafs)
#endif
                        {
                            if ((leaf->contents != CONTENTS_SKY) && (leaf->contents != CONTENTS_SOLID))
                            {
                                if (x || y || z)
                                {
                                    //dist = best_dist;
#ifdef HLRAD_HuntForWorld_FIX
									best_dist = dist;
#endif
                                    best_leaf = leaf;
                                    VectorCopy(current_point, best_point);
                                    continue;
                                }
                                else
                                {
                                    VectorCopy(current_point, point);
                                    return leaf;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (best_leaf)
        {
            break;
        }
    }

    VectorCopy(best_point, point);
    return best_leaf;
}
